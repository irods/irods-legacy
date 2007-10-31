/*
 * UploadItem.java
 * 
 * Created on Oct 10, 2007, 10:12:29 AM
 * 
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.sdsc.grid.gui.applet;

import java.util.List;
import java.io.File;
import java.net.URI;
import java.util.ArrayList;
import java.net.URISyntaxException;

/**
 *
 * @author awu
 */
public class UploadItem implements AppletConstant {
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
            
            resourceList = new DBUtil().getResourceList(uri.getHost(), uri.getPort());
        } catch (URISyntaxException e) {
        }
        //resourceList.add(selectedResource);
        
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
