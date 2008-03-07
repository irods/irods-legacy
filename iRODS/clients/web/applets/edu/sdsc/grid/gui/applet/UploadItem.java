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
//      UploadItem.java    -  edu.sdsc.grid.gui.applet.UploadItem
//
//  CLASS HIERARCHY
//      java.lang.Object
//          |
//          +-.UploadItem
//
//  PRINCIPAL AUTHOR
//      Alex Wu, SDSC/UCSD
//
//

package edu.sdsc.grid.gui.applet;

import java.net.URI;
import java.net.URISyntaxException;

import java.util.List;

import java.io.File;
import java.io.IOException;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
//TODO temp?
import edu.sdsc.grid.io.RemoteFile;
import edu.sdsc.grid.io.RemoteFileSystem;


class UploadItem 
{
  /* Begin static variables
  */

  static String STATUS_QUEUED = "queued";
  static String STATUS_UPLOADED = "uploaded";
  static String STATUS_IN_PROGRESS = "in progress";
  static String STATUS_REQUIRES_AUTHENTICATION = "requires authentication";
  static String STATUS_SERVER_UNAVAILABLE = "server unavailable";
  static String STATUS_FAILED = "failed";

  static String TYPE_FILE = "File";
  static String TYPE_FOLDER = "Folder";

  static String IN_PROGRESS_STATUS = "In Progress";
  static String DONE_STATUS = "Done";
  static String FAILED_STATUS = "Failed";

  /* End static variables
  */

  private GeneralFile source, destination; 
  
  private String selectedResource;
  /**
   * type has value FILE or FOLDER
   */
  private String type;
  /**
   * queued, uploaded, in progress, requires authentication, 
   * server unavailable, failed
   */
  private String status = STATUS_QUEUED; 
  private boolean assigned = false;
  private List resourceList;
  static AppletLogger logger = AppletLogger.getInstance();


  UploadItem(String s, String d, String resource) 
    throws IOException, URISyntaxException
  {
    this.source = FileFactory.newFile(new URI(s));
//TODO only needed for DB, won't connect without password.     
    this.destination = FileFactory.newFile(new URI(d));
    if (resource == null)
      this.selectedResource = Account.defaultResource;
    else
      this.selectedResource = resource;

    resourceList = DBUtil.getInstance().getResourceList(
      ((RemoteFileSystem)destination.getFileSystem()).getHost(), 
      ((RemoteFileSystem)destination.getFileSystem()).getPort());

    // find out if this is a file or folder
    if (source.exists()) {
      if (source.isFile())
        type = TYPE_FILE;
      else
        type = TYPE_FOLDER;
    }
    // else
    // TODO: Handle case when file no longer exists
  }  

  
  UploadItem(GeneralFile source, GeneralFile destination, 
    String resource) 
    throws IOException
  {
    this.source = source;
    this.destination = destination;
    if (resource == null)
      this.selectedResource = Account.defaultResource;
    else
      this.selectedResource = resource;

    resourceList = DBUtil.getInstance().getResourceList(
      ((RemoteFileSystem)destination.getFileSystem()).getHost(), 
      ((RemoteFileSystem)destination.getFileSystem()).getPort());

    // find out if this is a file or folder
    if (source.exists()) {
      if (source.isFile())
        type = TYPE_FILE;
      else
        type = TYPE_FOLDER;
    }
    // else
    // TODO: Handle case when file no longer exists
  }//UploadItem


  UploadItem(GeneralFile source, GeneralFile destination, 
    List resourceList) 
  {
    this.source = source;
    this.destination = destination;
    this.resourceList = resourceList;
    if (resourceList != null && resourceList.size() > 0)
      this.selectedResource = (String) resourceList.get(0);
    else
      this.selectedResource = Account.defaultResource; // should never come here

    // find out if this is a file or folder
    if (source.exists()) {
      if (source.isFile())
        type = TYPE_FILE;
      else
        type = TYPE_FOLDER;
    }
    // else
    // TODO: Handle case when file no longer exists
  }//UploadItem


  public String getSource() {
    return source.toString();
  }

  public String getDestination() {
    return destination.toString();
  }

  public GeneralFile getSourceFile() {
    return source;
  }

  public GeneralFile getDestinationFile() {
    return destination;
  }

  public GeneralFile getDestinationParent() {
    // returns the parent folder of the upload item
  return destination.getParentFile();        
  }

  public String getSelectedResource() {
    return selectedResource;
  }

  public List getResourceList() {
    return resourceList;
  }

  public String getType() {
    return type;
  }

  public String getStatus() {
    return status;
  }

  public void setStatus(String status) {
    this.status = status;
  }

  public boolean isAssigned() {
    return assigned;
  }

  public void setAssigned(boolean assigned) {
    this.assigned = assigned;
  }
}
