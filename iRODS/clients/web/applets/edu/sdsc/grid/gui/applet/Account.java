//      Copyright (c) 2008, Regents of the University of California
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

package edu.sdsc.grid.gui.applet;

import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;

import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLEncoder;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;

import org.json.JSONObject;

//temp
import edu.sdsc.grid.io.RemoteFileSystem;
//TOD should use remote instead.
import edu.sdsc.grid.io.irods.IRODSFileSystem;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.GeneralFileSystem;
import edu.sdsc.grid.io.GeneralMetaData;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.ResourceMetaData;
import edu.sdsc.grid.io.UserMetaData;

import java.net.URI;
import java.net.URISyntaxException;

/**
 * The account information of a remote filesystem.
 */
class Account 
{
  // Logger
  static AppletLogger logger = AppletLogger.getInstance();

  private URI uri;
  private GeneralFileSystem fileSystem;
  private GeneralFile destination, destinationFolder;

  private String zone = "tempZone"; // assign default value
  static String defaultResource = "demoResc"; // assign default value

  private int port;
  private String sessionId; // web session id passed to this class by UploadApplet


  /**
   * Remote file system resource list
   */      
  private List resourceList = new ArrayList();

  /**
   * Used for parsing uri
   */      
//    private static String USERINFO_DELIMITER = ":";

  /**
   * Used for parsing uri
   */      
  private static String SCHEME_DELIMITER = "://"; 


  /**
   * Web url for requesting a password for the given uri
   */
  static String TEMP_PASSWORD_SERVICE_URL = "http://www.irods.org/web"; 


  /**
   * Used by the applet at initialization
   */
  Account( String tempPasswordServiceUrl, URI uri, String sessionId ) 
    throws IOException
  {
    if (uri == null) {  
      throw new NullPointerException("iRODS URI cannot be null");
    }
    else {
      this.uri = uri;
    }

    TEMP_PASSWORD_SERVICE_URL = tempPasswordServiceUrl;
    parseURI(uri, true);
    setSessionId(sessionId);
//TODO Jargon will figure this out?        setDefaultResource();
    setResourceList(); 
  }//Account

  
  
  /**
   * Queries the irods filesystem for available resources
   * saves resources to a List
   */
  private void setResourceList( ) 
  {        
    try {
      MetaDataRecordList[] rl = fileSystem.query( null, 
        new MetaDataSelect[]{ 
          MetaDataSet.newSelection(ResourceMetaData.RESOURCE_NAME) } );

      for (int i=0; i < rl.length; i++) {
        String rsc = rl[i].getStringValue(0);
        resourceList.add(rsc);

        // add to database
        // applet queries database for a list of resources when
        // reloading files that have not been uploaded
//TODO change database to accept uri's                
        DBUtil.getInstance().addResource(
          ((RemoteFileSystem)fileSystem).getHost(), 
          ((RemoteFileSystem)fileSystem).getPort(), rsc);
      }
    } catch (Exception e) {
        logger.log("Account.class. Problem querying for resource list. " + e);
    } 
  }//setResourceList

  List getResourceList( ) 
  {
    return resourceList;
  }
  
  
  public void parseURI( URI uri, boolean isInit ) 
    throws IOException
  {
    // Possible URI formats:
    //   URI_PARAM = irods://user:pass@host:port/destination
    //   URI_PARAM = irods://user@host:port/destination
    if (uri == null) return;


    if (isInit) {
      // destination value during upload process could be the file name
      destinationFolder = FileFactory.newFile(uri, getPassword(uri));
      fileSystem = destinationFolder.getFileSystem();
    }        
    else {
      fileSystem = FileFactory.newFile(uri, getPassword(uri)).getFileSystem();      
    }
  } 

  
  /**
   * In case of need to change upload destination after applet start
   */
  void setURI( URI uri ) throws IOException
  {
    parseURI(uri, true);
  }

  URI getURI( )
  {
    return uri;
  }

  
  void setSessionId( String sessionId )
  {
      if (sessionId != null && !sessionId.trim().equals(""))
          this.sessionId = sessionId;
  }

  String getSessionId( ) {
      return sessionId;
  }

  
  GeneralFile getDestinationFolder( )
  {
    return destinationFolder;
  }

  
  void setDefaultResource( ) 
  {
    //TODO add getDefaultStorageResource to RemoteFileSystem?
    defaultResource = 
      ((IRODSFileSystem)fileSystem).getDefaultStorageResource();
  }

  String getDefaultResource( ) 
  {        
    return defaultResource;
  }
  

  public String _getPassword( URI uri )
  {
    String result = uri.getUserInfo();
    int index = result.indexOf(":");        
    if (index >= 0) 
    {
      return result.substring(index+1);
    } else {
      //logger.log("No key in passwordMap.");
      return null;
    }
  }

  public String getPassword( URI uri )
  {
    // look first to see if password can be found
    if (_getPassword(uri) != null)
        return _getPassword(uri);

    // will need to get temporary password if not found 
    // use sessionId to get temporary password
    // use session id to get temporary password
    String result = null;

    try {
      // temp password service does not accept "irods://user@host:port"
      // but accepts "user@host:port" for authentication
      String sUri = scrubURI(uri);
//TODO? php uses ruri            
      URL url = new URL(TEMP_PASSWORD_SERVICE_URL + "?SID=" + 
        sessionId + "&ruri=" + URLEncoder.encode(sUri, "UTF-8") );

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

      if (jsonObject.has("success")) 
      {
        // true or false
        if (jsonObject.get("success").toString().equals("false")) 
        {
          // get the error message first
          if (jsonObject.has("error")) 
          {
            // a String message
            logger.log("Returned error message from temporary password service is: " + jsonObject.get("error"));                        
          }
          return null;
        }
      }//if


      // should be able to retrieve password at this point    
      if (jsonObject.has("temppass")) 
      {
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

    return result;
  }
  

  public String scrubURI( URI realURI ) 
  {
    String result = realURI.toString();
    // sample uri
    // irods://rods:1580a833c4618453a3bd53fafd81db9f@saltwater.sdsc.edu:1247/tempZone/home/rods/include

    //remove the uri scheme (TODO for some reason?)
    int i = result.indexOf(SCHEME_DELIMITER);
    try {            
      if (i >= 0) {
        return result.substring(i+SCHEME_DELIMITER.length());
      }
    } catch (StringIndexOutOfBoundsException e) {
      logger.log("Problem scrubbing uri. " + e);
    }

    return result;
  }
}