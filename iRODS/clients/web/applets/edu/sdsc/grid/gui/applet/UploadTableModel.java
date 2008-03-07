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
//      UploadTableModel.java    -  edu.sdsc.grid.gui.applet.UploadTableModel
//
//  CLASS HIERARCHY
//      java.lang.Object
//          |
//          +-.DefaultTableModel
//                             |
//                             +-.UploadTableModel
//
//  PRINCIPAL AUTHOR
//      Alex Wu, SDSC/UCSD
//
//

package edu.sdsc.grid.gui.applet;

import java.io.File;
import java.io.IOException;

import java.net.URISyntaxException;

import java.util.List;
import java.util.ArrayList;

import javax.swing.table.DefaultTableModel;
import javax.swing.ImageIcon;
import javax.swing.JProgressBar;
import javax.swing.Icon;
import javax.swing.JComboBox;
import javax.swing.JTextField;
import javax.swing.border.EmptyBorder;

class UploadTableModel extends DefaultTableModel {
    /* Column index for table */
    static int ICON_COLUMN = 0;
    static int SOURCE_COLUMN = 1;
    static int DESTINATION_COLUMN = 2;
    static int RESOURCE_COLUMN = 3;
    static int STATUS_COLUMN = 4;

    
    private int directoryFileCount;
    //private int currentRow; // row just added
                                             
    private static String fileIconPath = "/image/file.png";
    private static String folderIconPath = "/image/folder.png";
    
    private ImageIcon fileIcon;
    private ImageIcon folderIcon;
        
    // Logger
    static AppletLogger logger = AppletLogger.getInstance();
    
    UploadTableModel() {        
        this.addColumn("");// empty for file/folder icon
        this.addColumn("Source"); // local file or folder
        this.addColumn("Destination"); // local file or folder
        this.addColumn("Resource"); // local file or folder
        this.addColumn("Status"); // Progress bar
        
        try {
            fileIcon = new ImageIcon(this.getClass().getResource(fileIconPath));
            folderIcon = new ImageIcon(this.getClass().getResource(folderIconPath));
            
        } catch (Exception e) {
            logger.log("file icon exception. " + e);
        }//try-catch
    }//UpdateTableModel
 
    
    public boolean isCellEditable(int row, int col) {
        if (col == SOURCE_COLUMN || col == DESTINATION_COLUMN || col == RESOURCE_COLUMN)
            return true;
        
        return false;
    }//isCellEditable
    
    
    // implement function below to display Checkbox
    public Class getColumnClass(int c) {
        Object obj = getValueAt(0, c);
        
        if (obj == null){
            return new Object().getClass();
        }else if (c == ICON_COLUMN) {
            // folder or file icon enclosed in JLabel
            return Icon.class;
        } else if (c == STATUS_COLUMN) {
            return JProgressBar.class;
        } else if (c == SOURCE_COLUMN) {
            return JTextField.class;
        } else if (c == DESTINATION_COLUMN) {
            return JTextField.class;  
        } else if (c == RESOURCE_COLUMN) {
            return JComboBox.class;
        } else {
            return this.getValueAt(0, c).getClass();
        }
        
    }//getColumnClass
    
    
    // param: List fileList is a List of UploadItem objects
    public void addFile(List fileList) { 
        int rowCount = this.getRowCount();
        
        UploadItem item = null;
        for (int k=0; k<fileList.size(); k++) {
            item = (UploadItem) fileList.get(k);
            addToTable(item);            
        }//for
        
    }//addFile
    
    public void removeFile(int[] selectedRows)
    {
      List itemList = new ArrayList();
      try {
        for (int k = selectedRows.length - 1; k >= 0; k--) {
          String source = ((JTextField) this.getValueAt(selectedRows[k], SOURCE_COLUMN)).getText();
          String destination = ((JTextField) this.getValueAt(selectedRows[k], DESTINATION_COLUMN)).getText();
          String resource = (String) ((JComboBox) this.getValueAt(selectedRows[k], RESOURCE_COLUMN)).getSelectedItem();

          UploadItem item = new UploadItem(source, destination, resource);
          itemList.add(item);

          this.removeRow(selectedRows[k]);
        }//for
      } catch (IOException e) {
        logger.log("file grid exception. " + e);
      } catch (URISyntaxException e) {
        logger.log("file name/uri exception. " + e);
      }
      DBUtil.getInstance().delete(itemList);        
    }//removeFile

    
    private JTextFieldListener tfListener = new JTextFieldListener();
    private JTextFieldMouseListener tfMouseListener = new JTextFieldMouseListener();
     
    private void addToTable(UploadItem item) {
        JTextField tfSource = new JTextField(item.getSource());
        JTextField tfDestination = new JTextField(item.getDestination());
        
        
        JComboBox comboBox = new JComboBox(item.getResourceList().toArray());
        comboBox.setSelectedItem(item.getSelectedResource());
        
        tfSource.addFocusListener(tfListener);
        tfDestination.addFocusListener(tfListener);
        
        tfSource.setBorder(new EmptyBorder(0, 8, 0, 8));        
        tfDestination.setBorder(new EmptyBorder(0, 8, 0, 8));        
        
        tfSource.setDragEnabled(false);
        tfDestination.setDragEnabled(false);
        
        ImageIcon icon = null;
        if (item.getType().equals(UploadItem.TYPE_FILE))
            icon = fileIcon;
        else
            icon = folderIcon;
        
        this.addRow(new Object[] { icon, tfSource, tfDestination, comboBox, null});
    }//addToTable
    
    
    private void calculateFileCount(File file, int row, String folderName) {
        // recursive function to calculate total file count
        // includes subfolders
        File[] fileList = file.listFiles();
        int len = fileList.length;
        
        for (int i=0; i < len; i++) {
            if (fileList[i].isFile()) {
                directoryFileCount++;
            } else if (fileList[i].isDirectory()) {
                calculateFileCount(fileList[i], row, folderName); // recurse
            }
        }//for
        
    }//calculateFileCount

    
}//UploadTableModel


