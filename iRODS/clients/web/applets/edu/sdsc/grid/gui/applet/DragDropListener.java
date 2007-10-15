/*
 * DragDropListener.java
 *
 * Created on September 26, 2007, 2:10 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package edu.sdsc.grid.gui.applet;

import java.awt.dnd.DnDConstants;
import java.awt.datatransfer.DataFlavor;
import java.awt.dnd.DropTargetEvent;
import java.awt.dnd.DropTargetDragEvent;
import java.awt.dnd.DropTargetDropEvent;
import java.awt.dnd.DropTargetListener;
import java.util.List;
import java.util.Vector;
import java.util.Arrays;
import java.io.File;
import org.w3c.dom.*;
import org.w3c.dom.css.*;
import org.w3c.dom.events.*;
import org.w3c.dom.html.*;
import org.w3c.dom.stylesheets.*;
import org.w3c.dom.views.*;
import com.sun.java.browser.dom.*;


        
/**
 *
 * @author awu
 */
public class DragDropListener implements DropTargetListener {
    static AppletLogger logger = AppletLogger.getInstance();        
    private Account account;
    public static final DataFlavor fileListFlavor = DataFlavor.javaFileListFlavor;
    public static final DataFlavor[] flavors = { fileListFlavor };
    public static final List flavorList = Arrays.asList(flavors);
    private DataFlavor chosenFlavor;
    UploadTableModel model;
    UploadApplet applet;
    
    public DragDropListener(UploadTableModel model, UploadApplet a, Account acct) {
        this.model = model;
        applet = a;
        account = acct;
    }
    
    private String getCurrentRuri() {
        String ruri = applet.getCurrentRuri();
        return ruri;
    }
    
    public void drop (DropTargetDropEvent dtde) {
        Object data = null;
        
        try {
            dtde.acceptDrop(DnDConstants.ACTION_COPY);
            data = dtde.getTransferable().getTransferData(chosenFlavor);
            List dropList = (List) data; // a List of File objects
            List fileList = new Vector();
            File file = null;
            
            account.parseRuri(getCurrentRuri(), true);

            for (int i = 0; i < dropList.size(); i++) {
                file = (File) dropList.get(i);
                String[] f = new String[2];
                f[0] = file.getAbsolutePath(); // source
                f[1] = "irods://" + account.getDestinationFolderAsUri() +  file.getName(); // destination

                UploadItem item = new UploadItem(f[0], f[1], account.getDefaultResource());
                boolean success = new DBUtil().insert(item);
                if (success)
                    fileList.add(item);
            }//for
            
            model.addFile(fileList);
            
        } catch (Exception e) {
            logger.log("drop error. " + e);
            e.printStackTrace();
        }
        
    }


    public void dragEnter (DropTargetDragEvent dtde) {
        if (isDataFlavorSupported(dtde) == false)
            dtde.rejectDrag();
    }
    
    
    public void dragExit (DropTargetEvent dte) {

    }
    public void dragOver (DropTargetDragEvent dtde) {

    }
    
    public void dropActionChanged(DropTargetDragEvent dtde) {

    }
    
    public boolean isDataFlavorSupported(DropTargetDragEvent dtde) {
        boolean isSupported = false;
        
        for (int i=0; i<flavors.length; i++) {
            if (dtde.isDataFlavorSupported(flavors[i])) {

                isSupported = true;
                chosenFlavor = flavors[i];
                break;
            }
        }
        return isSupported;
    }        
    
    
}
