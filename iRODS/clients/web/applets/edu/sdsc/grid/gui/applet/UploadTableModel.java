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
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.List;
import javax.swing.table.DefaultTableModel;
import javax.swing.JCheckBox;
import javax.swing.ImageIcon;
import javax.swing.JProgressBar;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.Icon;
import javax.swing.JTextField;
import java.net.MalformedURLException;
import javax.swing.border.EmptyBorder;
import java.awt.FlowLayout;
import javax.swing.JTable;

class UploadTableModel extends DefaultTableModel implements AppletConstant {
    private int directoryFileCount;
    private int currentRow; // row just added
                                             
    private static String fileIconPath = "/image/file.png";
    private static String folderIconPath = "/image/folder.png";
    
    private ImageIcon fileIcon;
    private ImageIcon folderIcon;
        
    // Logger
    static AppletLogger logger = AppletLogger.getInstance();
    
    public UploadTableModel() {        
        this.addColumn("");// empty for file/folder icon
        this.addColumn("Local"); // local file or folder
        this.addColumn("Remote"); // local file or folder
        //this.addColumn("Size"); // size of file; folder will show sum of enclosed file sizes
        this.addColumn("Status"); // Progress bar
        
        try {
            fileIcon = new ImageIcon(this.getClass().getResource(fileIconPath));
            folderIcon = new ImageIcon(this.getClass().getResource(folderIconPath));
            
        } catch (Exception e) {
            logger.log("file icon exception. " + e);
        }//try-catch
    }
 
    public boolean isCellEditable(int row, int col) {
        if (col == SOURCE_COLUMN || col == DESTINATION_COLUMN)
            return true;
        
        return false;
    }
    
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
        } else {
            return this.getValueAt(0, c).getClass();
        }
        
    }
    
    // param file can be a file or folder
    // issue: queue log file may be written to as it is being read
    // may cause a file to be queued in two separate applet instances
    // param: List fileList is a List of String[] where
    // String[0] is the source file path and
    // String[1] is the destination path
    public void addFile(List fileList) { 
        int rowCount = this.getRowCount();
        File file = null;
        String filePath = null;    
        File fileLog = new File(QUEUE_LOG);
        
        // load queue log entries into a List
        List queue = logger.readQueueLog();  // a List of String
        
        for (int k=0; k<fileList.size(); k++) {
            String[] s = (String[]) fileList.get(k);
            file = new File(s[0]); //source
            String destination = s[1];

            //check if file is already in queue log
            boolean fileAdded = false;
            for (int p=0; p < queue.size(); p++) {
                String t = (String) queue.get(p); // from queue log file
                String t_split[] = t.split("\t"); // delimited by a tab; the format of the line read is : <source> <tab> <destination>
            
                // case sensitive
                // may cause a file to be in the table twice if Operating System doesn not handle case sensitivity
                //
                // compare the file source in the queue log and the source of the file dragged-dropped
                try {
                    if (t_split[0].equals(s[0])) {
                        fileAdded = true;
                        break;
                    }
                } catch (ArrayIndexOutOfBoundsException e){
                    // don't log
                }
            }
            
            if (fileAdded)
                continue;
            
            
            if (file.isFile())
                this.addFileToTable(file, destination); // can be a file
            else if (file.isDirectory())
                this.addDirectoryToTable(file, destination);
            
            // add file to text file for recovery in case application crashes or is interrupted unexpectedly    
            FileOutputStream fos = null;
            try {
                fos = new FileOutputStream(fileLog, true);
                fos.write(file.getAbsolutePath().getBytes());
                fos.write("\t".getBytes());
                fos.write(destination.getBytes());
                fos.write("\n".getBytes());
            } catch (IOException e) {
            } finally {
                try {
                    fos.close();
                } catch (IOException e) {
                }
            }//try-catch-finally
            
        }//for
        
    }
    
    public void removeFile(int[] selectedRows) {
        for (int k = selectedRows.length - 1; k >= 0; k--) {

            //this.setValueAt(null, selectedRows[k], SOURCE_COLUMN);
            this.removeRow(selectedRows[k]);
            
        }
        

            
        // need to save upload queue to log file
        // delete queue log and save files currently in table
        File fileLog = new File(QUEUE_LOG);
        fileLog.delete();
        
        fileLog = new File(QUEUE_LOG);
        int rowCount = this.getRowCount();
        
        JTextField tfSource = null;
        JTextField tfDestination = null;
        
        for (int k = 0; k < rowCount; k++) {
            //String filePath = this.getValueAt(k, 1).toString() + "\t" + this.getValueAt(k, 2);                
            tfSource = (JTextField) this.getValueAt(k, SOURCE_COLUMN);
            tfDestination = (JTextField) this.getValueAt(k, DESTINATION_COLUMN);
            String filePath = tfSource.getText() + "\t" + tfDestination.getText();                
            
            FileOutputStream fos = null;
            
            try {
                fos = new FileOutputStream(fileLog, true);
                fos.write(filePath.getBytes());
                fos.write("\n".getBytes());
            } catch (IOException ioe) {
            } finally {
                try {
                    fos.close();
                } catch (IOException ioe) {
                }
            }//try-catch-finally
        }//for
    }
    
    
    private JTextFieldListener tfListener = new JTextFieldListener();
    private JTextFieldMouseListener tfMouseListener = new JTextFieldMouseListener();
     
    private void addFileToTable(File file, String destination) {
        JTextField tfSource = new JTextField(file.getAbsolutePath());
        JTextField tfDestination = new JTextField(destination);
        tfSource.addFocusListener(tfListener);
        tfDestination.addFocusListener(tfListener);

        //tfSource.addMouseListener(tfMouseListener);
        //tfDestination.addMouseListener(tfMouseListener);
        
        tfSource.setBorder(new EmptyBorder(0, 8, 0, 8));        
        tfDestination.setBorder(new EmptyBorder(0, 8, 0, 8));        
        tfSource.setDragEnabled(false);
        tfDestination.setDragEnabled(false);
        
        this.addRow(new Object[] { fileIcon, tfSource, tfDestination, null});
    }
    
    private void addDirectoryToTable(File file, String destination) {
        
        JTextField tfSource = new JTextField(file.getAbsolutePath());
        JTextField tfDestination = new JTextField(destination);

        tfSource.addFocusListener(tfListener);
        tfDestination.addFocusListener(tfListener);

        //tfSource.addMouseListener(tfMouseListener);
        //tfDestination.addMouseListener(tfMouseListener);
        
        tfSource.setBorder(new EmptyBorder(0, 8, 0, 8));        
        tfDestination.setBorder(new EmptyBorder(0, 8, 0, 8));        
        tfSource.setDragEnabled(false);
        tfDestination.setDragEnabled(false);
        
        this.addRow(new Object[] { folderIcon, tfSource, tfDestination, null});
    }
    
    
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
        
    }
    
 

    
    // show content in the folder
    // recursive function
    public void expandDirectory(File file) {
        // add rows under this directory
    }
    
    public void collapseDirectory(File file) {
        // remove rows under this directory
    }
    
}//UploadTableModel


