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
//      DragDropListener.java    -  edu.sdsc.grid.gui.applet.DragDropListener
//
//  CLASS HIERARCHY
//      java.lang.Object
//          |
//          +-.DragDropListener
//
//  PRINCIPAL AUTHOR
//      Alex Wu, SDSC/UCSD
//
//

package edu.sdsc.grid.gui.applet;

import java.awt.dnd.DnDConstants;
import java.awt.datatransfer.DataFlavor;
import java.awt.dnd.DropTargetEvent;
import java.awt.dnd.DropTargetDragEvent;
import java.awt.dnd.DropTargetDropEvent;
import java.awt.dnd.DropTargetListener;

import java.net.URI;
  
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


import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.local.LocalFile;

/**
 * This class handles the drag and drop event from the local filesystem
 */
class DragDropListener implements DropTargetListener {
    static AppletLogger logger = AppletLogger.getInstance();        
    private Account account;
    public static final DataFlavor fileListFlavor = DataFlavor.javaFileListFlavor;
    public static final DataFlavor[] flavors = { fileListFlavor };
    public static final List flavorList = Arrays.asList(flavors);
    private DataFlavor chosenFlavor;
    UploadTableModel model;
    UploadApplet applet;
    
    DragDropListener(UploadTableModel model, UploadApplet a, Account acct) {
        this.model = model;
        applet = a;
        account = acct;
    }
    
    private URI getCurrentRuri() {
        return applet.getRuri();
    }
    
    public void drop (DropTargetDropEvent dtde) {
        Object data = null;
        
        try {
            dtde.acceptDrop(DnDConstants.ACTION_COPY);
            data = dtde.getTransferable().getTransferData(chosenFlavor);
            List dropList = (List) data; // a List of File objects
            List fileList = new Vector();
            File file = null;

            account.parseURI(getCurrentRuri(), true);
 
            for (int i = 0; i < dropList.size(); i++) {
                file = ((File) dropList.get(i));
                UploadItem item = new UploadItem(
                  new LocalFile(file), //local file from from desktop
                  FileFactory.newFile( 
                  account.getDestinationFolder(), file.getName()), 
                  account.getResourceList());
                
                boolean success = DBUtil.getInstance().insert(item);
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
