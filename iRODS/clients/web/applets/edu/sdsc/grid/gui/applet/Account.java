//      Copyright (c) 2005, Regents of the University of California
//      All rights reserved.
//
//      Redistribution and use in source and binary forms, with or without
//      modification, are permitted provided that the following conditions are
//      met:
//
//        * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//        * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//        * Neither the name of the University of California, San Diego (UCSD) nor
//      the names of its contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
//      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//      IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//      PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//      CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//      EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//      PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//      PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//      LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//      NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//      SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//  FILE
//      Account.java    -  edu.sdsc.grid.gui.applet.Account
//
//  CLASS HIERARCHY
//      java.lang.Object
//          |
//          +-.Account
//
//  PRINCIPAL AUTHOR
//      Alex Wu, SDSC/UCSD
//
//

package edu.sdsc.grid.gui.applet;

import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;

import java.net.URL;
import java.net.URLConnection;
import java.net.URLEncoder;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;

import org.json.JSONObject;

import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSMetaDataSet;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import java.net.URI;
import java.net.URISyntaxException;

public class Account {
    // Logger
    static AppletLogger logger = AppletLogger.getInstance();
    
    private String ruri, host, username, home, destination, destinationFolder;
    private String zone = "tempZone"; // assign default value
    private String defaultResource = "demoResc"; // assign default value
    private int port;
    private String sessionId; // web session id passed to this class by UploadApplet
    private List resourceList = new ArrayList(); // irods resource list
    private Map map = new HashMap();
    private static String SCHEME_DELIMITER = "://"; // used for parsing irods uri
    private static String TEMP_PASSWORD_SERVICE_URL = null; // web url for requesting a password for the given uri
    
    public Account() {}
    
    // This constructor is used by the applet at initialization
    public Account(String tempPasswordServiceUrl, String ruri, String sessionId) {
        this.ruri = ruri;
        TEMP_PASSWORD_SERVICE_URL = tempPasswordServiceUrl;
        parseRuri(ruri, true);
        setSessionId(sessionId);
        setDefaultResource();
        setResourceList(); 
    }//Account
    
    // queries the irods filesystem for available resources
    // saves resources to a List
    private void setResourceList() {
        String[] selectFieldNames = {IRODSMetaDataSet.RESOURCE_NAME};
        
        MetaDataCondition[] conditions = {
            MetaDataSet.newCondition( IRODSMetaDataSet.USER_NAME,
            MetaDataCondition.EQUAL, getUsername() )
        };
        
        IRODSFileSystem irodsFs = null;
        MetaDataSelect selects[] = MetaDataSet.newSelection( selectFieldNames );
        MetaDataRecordList[] rl = null;
        
        try {
            IRODSAccount irodsAccount = new IRODSAccount(getHost(), getPort(), getUsername(), new String(getPassword(ruri)), getHome(), getZone(), getDefaultResource());
            irodsFs = new IRODSFileSystem(irodsAccount);
            rl = irodsFs.query(conditions, selects);
            
            for (int i=0; i < rl.length; i++) {
                String rsc = rl[i].getValue(IRODSMetaDataSet.RESOURCE_NAME).toString();
                resourceList.add(rsc);
                
                // add to database
                // applet queries database for a list of resources when
                // reloading files that have not been uploaded
                DBUtil.getInstance().addResource(getHost(), getPort(), rsc);
            }//for
            
        } catch (Exception e) {
            System.out.println("Problem querying for resource list. " + e);
        } finally {
            try {
                if (irodsFs.isConnected())
                    irodsFs.close();
            } catch (IOException ioe) {
                logger.log("Account.class. Problem closing irods connection. " + ioe);
            }
        }
    }//setResourceList
        
        
    public void parseRuri(String ruri, boolean isInit) {
        // Possible URI formats:
        //   URI_PARAM = irods://user:pass@host:port/destination
        //   URI_PARAM = user:pass@host:port/destination
        //
        //   URI_PARAM = irods://user@host:port/destination
        //   URI_PARAM = user@host:port/destination

        if (ruri == null) return;
        
        // using java.net.URI class so make sure ruri is a correct uri format
        if (ruri.indexOf(SCHEME_DELIMITER) == -1)
            ruri = "irods://" + ruri;
        
        URI uri = null;
        try {
            uri = new URI(ruri);
        } catch (URISyntaxException e) {
            logger.log("URI exception. " + e);
        }//try-catch
        
        //username = uri.getUserInfo(); // includes the password; don't use this one
        username = parseUsernameFromRuri(ruri);
        setPassword(ruri); // in case ruri passed has a password
        host = uri.getHost();
        port = uri.getPort();
        destination = uri.getPath();
        
        // set default values for home and defaultResource
        home = zone + "/" + username;
        
        if (isInit) {
            // destination value during upload process could be the file name
            destinationFolder = destination;
            
            // make sure destinationFolder ends with a forward slash
            // may be a better way
            if (!destinationFolder.endsWith("/"))
                destinationFolder += "/";
        }//if
        
    }//parseRuri
    
    public List getResourceList() {
        return resourceList;
    }

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
    
    public void setDefaultResource() {
        // necessary to create IRODSAccount here?
        IRODSAccount irodsAccount = new IRODSAccount(host, port, username, "", home, zone, defaultResource);            
        defaultResource = irodsAccount.getDefaultStorageResource();
    }

    public String getDefaultResource() {
        
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

        // look first to see if password can be found
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
            
            //
            // possible returned value from temp password service
            // {"success":false,"error":"Authentication Required"}
            // {"success":true,"temppass":"78d49f10ff73889ba9ee3bbf978c093a"}
            //
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
            }//if
            
             
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
        }//try-catch
        
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
        //ruri = scrubSchemeFromRuri(ruri);
        URI uri = null;
        try {
            uri = new URI(ruri);
        } catch (URISyntaxException e) {
            logger.log("URI exception. " + e);
        }//try-catch
        
        if (username == null) parseUsernameFromRuri(ruri);            
        if (host == null) uri.getHost();
        if (port == 0) uri.getPort();
        
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