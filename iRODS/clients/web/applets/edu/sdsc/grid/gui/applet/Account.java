package edu.sdsc.grid.gui.applet;

import java.util.Map;
import java.util.HashMap;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLEncoder;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import org.json.JSONObject;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSMetaDataSet;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataField;

public class Account {
    private String ruri, host, username, home, destination, destinationFolder;
    private String zone = "tempZone";
    private String defaultResource = "demoResc";
    private int port;
    private String sessionId;
    private Map map = new HashMap();
    private static String SCHEME_DELIMITER = "://";
    private static String TEMP_PASSWORD_SERVICE_URL = null;
    
    // Logger
    static AppletLogger logger = AppletLogger.getInstance();
    
    public Account() {}

    
    // This constructor is used at applet initialization
    public Account(String tempPassworServiceUrl, String ruri, String sessionId) {
        this.ruri = ruri;
        TEMP_PASSWORD_SERVICE_URL = tempPassworServiceUrl;
        parseRuri(ruri, true);
        setSessionId(sessionId);
        //setResourceList(); // giving problems
    }
    
    private void setResourceList() {
        String[] selectFieldNames = {IRODSMetaDataSet.RESOURCE_NAME};
        MetaDataCondition conditions[] = {MetaDataSet.newCondition( "RESC_ZONE_KW", MetaDataCondition.LIKE, "tempZone" )};
        MetaDataSelect selects[] = MetaDataSet.newSelection( selectFieldNames );

        MetaDataRecordList[] rl = null;
        try {
            IRODSAccount irodsAccount = new IRODSAccount(host, port, username, new String(getPassword(ruri)), home, zone, defaultResource);            
            IRODSFileSystem fileSystem = new IRODSFileSystem(irodsAccount);
            rl=((IRODSFileSystem) fileSystem).query(conditions, selects);
            logger.log("rl.length : "+ rl.length);
            for (int i=0; i < rl.length; i++) {
                MetaDataField[] fields = rl[i].getFields();
                for (int k=0; k < fields.length; k++) {
                    logger.log("field name:: " + fields[k].getName());
                }
            }//for
            
        } catch (Exception e) {
            logger.log("Problem querying for resource list. " + e);
            
        }
    }
        
        
    public void parseRuri(String ruri, boolean isInit) {
        // parse out password if any
        // Possible URI formats:
        //   URI_PARAM = irods://user:pass@host:port/destination
        //   URI_PARAM = user:pass@host:port/destination
        //
        //   URI_PARAM = irods://user@host:port/destination
        //   URI_PARAM = user@host:port/destination

        if (ruri == null) return;
        
        username = parseUsernameFromRuri(ruri);
        host = parseHostFromRuri(ruri);
        port = parsePortFromRuri(ruri);
        destination = parseDestinationFromRuri(ruri);
        setPassword(ruri);
        
        // set default values for home and defaultResource
        home = zone + "/" + username;
        
        if (isInit) {
            // destination value during upload process could be the file name
            destinationFolder = destination;
            
            // make sure destinationFolder ends with a forward slash
            // may be a better way
            if (!destinationFolder.endsWith("/"))
                destinationFolder += "/";
            
        }
        
    }//parseRuri
    
    public void setRuri(String ruri) {
        parseRuri(ruri, true);
    }
    
    public void setSessionId(String sessionId) {
        if (sessionId != null && !sessionId.trim().equals(""))
            this.sessionId = sessionId;
    }
    
    public String getSessionId() {
        return sessionId;
    }
    
    public void setHost(String host) {
        this.host = host;
    }
    
    public String getHost() {
        return host;
    }
    
    public void setUsername(String username) {
        this.username = username;
    }

    public String getUsername() {
        return username;
    }
    
    public void setDestination(String destination) {
        this.destination = destination;
    }
    
    public String getDestination() {
        return destination;
    }

    public String getDestinationFolderAsUri() {
        return username + "@" + host + ":" + port + destinationFolder;
    }
    
    public void setZone(String zone) {
        this.zone = zone;
    }

    public String getZone() {
        return zone;
    }
    
    public void setDefaultResource(String defaultResource) {
        this.defaultResource = defaultResource;
    }

    public String getDefaultResource() {
        IRODSAccount irodsAccount = new IRODSAccount(host, port, username, new String(getPassword(ruri)), home, zone, defaultResource);            
        defaultResource = irodsAccount.getDefaultStorageResource();
        logger.log("default storage resource is :  " + defaultResource);
        return defaultResource;
    }
    
    public void setPort(int port) {
        this.port = port;
    }

    public int getPort() {
        return port;
    }
    
    public void setHome(String home) {
        this.home = home;
    }

    public String getHome() {
        return home;
    }
    
    public void setPassword(String ruri, char[] password) {
        ruri = scrubSchemeFromRuri(ruri);
        ruri = scrubPasswordDestinationFromRuri(ruri);        
        
        if (password != null) {
            map.put(ruri, password);
        }
            
    }

    public void setPassword(String ruri) {
        if (containsPassword(ruri)) {
            char[] password = parsePasswordFromRuri(ruri);
            
            if (password != null && password.length > 0) {
                map.put(ruri, password);
            }
        }            
    }
    
    public char[] _getPassword(String ruri) {
        ruri = scrubSchemeFromRuri(ruri);
        ruri = scrubPasswordDestinationFromRuri(ruri);

        char[] result = null;
        
        if (map.containsKey((Object) ruri)) {
            result = (char[]) map.get(ruri);
        } else {
            //logger.log("No key in map.");
        }
            
        return result;
    }//getPassword
    
    public char[] getPassword(String ruri) {
        if (_getPassword(ruri) != null)
            return _getPassword(ruri);

        // will need to get temporary password if not found in map
        // use sessionId to get temporary password
        // use session id to get temporary password
        String result = null;
        
        try {
            // temp password service does not accept "irods://user@host:port"
            // but accepts "user@host:port" for authentication
            String sUri = scrubPasswordFromRuri (ruri);
            sUri = scrubSchemeFromRuri(sUri);
            
            URL url = new URL(TEMP_PASSWORD_SERVICE_URL + "?SID=" + sessionId + "&ruri=" + URLEncoder.encode(sUri, "UTF-8") );
            
            // possible returned value from temp password service
            // {"success":false,"error":"Authentication Required"}
            // {"success":true,"temppass":"78d49f10ff73889ba9ee3bbf978c093a"}
            
            JSONObject jsonObject = null;
            URLConnection conn = url.openConnection();
            InputStream istream = conn.getInputStream();
            
            BufferedReader in = new BufferedReader(new InputStreamReader(istream));
            if ((result = in.readLine()) != null) {
                jsonObject = new JSONObject(result);
            }
            
            if (jsonObject.has("success")) {
                // true or false
                if (jsonObject.get("success").toString().equals("false")) {
    
                    // get the error message first
                    if (jsonObject.has("error")) {
                        // a String message
                        logger.log("Returned error message from temporary password service is: " + jsonObject.get("error"));                        
                    }
                    
                    return null;
                }
                
            }
            
            
             
            // should be able to retrieve password at this point    
            if (jsonObject.has("temppass")) {
                // a String message
                result = (String) jsonObject.get("temppass");
            }
            
            
        } catch (java.io.FileNotFoundException fe) {
            logger.log("Could not find temporary password service. " + fe);
            fe.printStackTrace();
        } catch (Exception e) {
            logger.log("Exception getting temporary password. " + e);
            e.printStackTrace();
        }
        
        if (result == null)
            return null;
                    
        return result.toCharArray();
    }
    
    public void removePassword(String ruri) {
        ruri = scrubSchemeFromRuri(ruri);
        ruri = scrubPasswordDestinationFromRuri(ruri);
        map.remove(ruri);
    }
    
    public void clearMap() {
        map.clear();
    }
    
    public String scrubPasswordFromRuri() {
        // returns ruri w/o password
        return username + "@" + host + ":" + port + destination;
    }
    
    public String scrubPasswordFromRuri(String ruri) {

        if (!containsPassword(ruri))
            return ruri;

        // sample uri
        // irods://rods:1580a833c4618453a3bd53fafd81db9f@saltwater.sdsc.edu:1247/tempZone/home/rods/include
        
        String result = null;
        int i = ruri.indexOf(SCHEME_DELIMITER); // '://'
        i = ruri.indexOf(":", i + SCHEME_DELIMITER.length());
        int k = ruri.indexOf("@");
        String password = ruri.substring(i, k); // including the : symbol
        
        //result = ruri.replaceAll(password, ""); // jdk 1.4
        result = ruri.replace(password, ""); // jdk 1.5
        
        return result;
    }
    
    private String scrubSchemeFromRuri(String ruri) {
        // scrub out the scheme portion of the RURI
        // could be in different cases, such  as:
        //   irods://
        //   IRODS://
        //   iRods://
        // etc.
        try {            
            int i = ruri.indexOf(SCHEME_DELIMITER);
            if (i != -1) {
                return ruri.substring(i+SCHEME_DELIMITER.length(), ruri.length());
            }
        } catch (StringIndexOutOfBoundsException e) {
            logger.log("Problem scrubbing scheme from ruri. " + e);
        }
        
        return ruri;
    }//scrubSchemeFromRuri
    
    private String scrubPasswordDestinationFromRuri(String ruri) {
        ruri = scrubSchemeFromRuri(ruri);
        
        if (username == null) parseUsernameFromRuri(ruri);            
        if (host == null) parseHostFromRuri(ruri);
        if (port == 0) parsePortFromRuri(ruri);
            
        return username + "@" + host + ":" + port;
    }//scrubPasswordDestinationFromRuri    

    private char[] parsePasswordFromRuri(String ruri) {
        ruri = scrubSchemeFromRuri(ruri);
        
        try {
            int i = ruri.indexOf("@");
            int k = ruri.indexOf(":");
            char[] password = null;
            
            if (k < i) {
                password = ruri.substring(k+1, i).toCharArray();
                
                if (password.length > 0) // make sure password is not an empty
                    return password;
            }
        } catch (StringIndexOutOfBoundsException e) {
            logger.log("Problem parsing password from ruri. " + e);
        }
        return null;
    }//parsePasswordFromRuri
    
    private String parseUsernameFromRuri(String ruri) {
        ruri = scrubSchemeFromRuri(ruri);
        String result = null;
        
        try {        
            int i = ruri.indexOf("@");
            int k = ruri.indexOf(":"); // first, assume there is a password

            if (containsPassword(ruri))
                result = ruri.substring(0, k); // password passed in ruri parameter
            else
                result = ruri.substring(0, i);
                
        } catch (StringIndexOutOfBoundsException e) {
            logger.log("Problem parsing username from ruri. " + e);
        }

        return result;
    }//parseUsernameFromRuri
    
    private String parseHostFromRuri(String ruri) {
        ruri = scrubSchemeFromRuri(ruri);
        String result = null;
        
        try {
            int i = ruri.indexOf("@");
            int k = ruri.indexOf(":", i);
        
            result = ruri.substring(i+1, k);
        } catch (StringIndexOutOfBoundsException e) {
            logger.log("Problem parsing host from ruri. " + e);
        }
        
        return result;
    }

    private int parsePortFromRuri(String ruri) {
        ruri = scrubSchemeFromRuri(ruri);
        int port = 0;
        
        try {
            int i = ruri.indexOf("@");
            i = ruri.indexOf(":", i);
            int k = ruri.indexOf("/", i);
            port = Integer.parseInt(ruri.substring(i+1, k));
        } catch (NumberFormatException e) {
            // port stays at 0
            logger.log("Problem formatting port from ruri. " + e);
        } catch (StringIndexOutOfBoundsException ie) {
            logger.log("Problem parsing port from ruri. " + ie);
        }
        
        return port;
    }//parsePortFromRuri
    
    private String parseDestinationFromRuri(String ruri) {
        ruri = scrubSchemeFromRuri(ruri);
        String result = null;
        
        try {
            int i = ruri.indexOf("/");
            result = ruri.substring(i, ruri.length());
        } catch (StringIndexOutOfBoundsException e) {
            logger.log("Problem parsing destination from ruri. " + e);
        }
        return result;
    }//parseDestinationFromRuri
    
    
    private boolean containsPassword(String ruri) {
        ruri = scrubSchemeFromRuri(ruri);
        try {
            int i = ruri.indexOf("@");
            int k = ruri.indexOf(":");
                
            if (k < i) // password could be an empty, only checks for ruri format <username>:<password>@host....
                return true;
            
        } catch (StringIndexOutOfBoundsException e) {
            logger.log("Problem checking for password in ruri. " + e);
        }
        
        return false;
    }//containsPassword

}