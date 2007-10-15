/*
 * UploadItem.java
 * 
 * Created on Oct 10, 2007, 10:12:29 AM
 * 
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.sdsc.grid.gui.applet;

import java.io.File;

/**
 *
 * @author awu
 */
public class UploadItem implements AppletConstant {
    private String source, destination, resource, type; // type has value FILE or FOLDER
    private String status = STATUS_QUEUED; // queued, uploaded, in progress, requires authentication, server unavailable, failed
    private boolean assigned = false;
   
    
    public UploadItem(String source, String destination, String resource) {
        this.source = source;
        this.destination = destination;
        this.resource = resource;
        
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

    public String getResource() {
        return resource;
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
