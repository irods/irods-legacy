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


public class UploadItem {
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
    
    private String source, destination, selectedResource, type; // type has value FILE or FOLDER
    private String status = STATUS_QUEUED; // queued, uploaded, in progress, requires authentication, server unavailable, failed
    private boolean assigned = false;
    private List resourceList;
    static AppletLogger logger = AppletLogger.getInstance();
    
    public UploadItem(String source, String destination, String selectedResource) {
        this.source = source;
        this.destination = destination;
        this.selectedResource = selectedResource;

        try {
            URI uri = new URI(destination);
            
            resourceList = DBUtil.getInstance().getResourceList(uri.getHost(), uri.getPort());
        } catch (URISyntaxException e) {
        }
        
        // find out if this is a file or folder
        File f = new File(source);
        if (f.exists())
            if (f.isFile())
                type = TYPE_FILE;
            else
                type = TYPE_FOLDER;
        // else
        // TODO: Handle case when file no longer exists
            
    }//UploadItem
    

    public UploadItem(String source, String destination, List resourceList) {
        this.source = source;
        this.destination = destination;
        this.resourceList = resourceList;
        if (resourceList != null && resourceList.size() > 0)
            this.selectedResource = (String) resourceList.get(0);
        else
            this.selectedResource = ""; // should never come here
        
        // find out if this is a file or folder
        File f = new File(source);
        if (f.exists())
            if (f.isFile())
                type = TYPE_FILE;
            else
                type = TYPE_FOLDER;
        // else
        // TODO: Handle case when file no longer exists
            
    }//UploadItem


    public String getSource() {
        return source;
    }

    public String getDestination() {
        return destination;
    }
    
    public String getDestinationParent() {
        // returns the parent folder of the upload item
        int lastIndex = destination.lastIndexOf("/");
        return destination.substring(0, lastIndex);
        
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
