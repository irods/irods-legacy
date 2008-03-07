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
//      UploadApplet.java    -  edu.sdsc.grid.gui.applet.UploadApplet
//
//  CLASS HIERARCHY
//      java.lang.Object
//          |
//          +-.JApplet
//                   |
//                   +-.UploadApplet
//
//  PRINCIPAL AUTHOR
//      Alex Wu, SDSC/UCSD
//
//


package edu.sdsc.grid.gui.applet;


import java.io.IOException;
import java.io.File;

import java.util.List;
import java.util.ArrayList;

import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.MalformedURLException;

import javax.swing.JApplet;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JCheckBox;
import javax.swing.JRadioButton;
import javax.swing.JTextArea;

import java.awt.Dimension;
import java.awt.Container;
import java.awt.BorderLayout;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import com.sun.java.browser.dom.*;

import javax.swing.JComboBox;
import javax.swing.JTextField;
import javax.swing.ToolTipManager;

import org.w3c.dom.*;
import org.w3c.dom.css.*;
import org.w3c.dom.events.*;
import org.w3c.dom.html.*;
import org.w3c.dom.stylesheets.*;
import org.w3c.dom.views.*;

import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.FileFactory;

/**
 * Main Applet window. Creates and adds other JPanels to the main Container.
 *
 **/
public class UploadApplet extends JApplet implements ActionListener {
    static String USER_HOME = System.getProperty("user.home");
    static String FILE_SEPARATOR = System.getProperty("file.separator");
    static String IRODS_DIR = USER_HOME + FILE_SEPARATOR + ".irods";
    static String UPLOADED_LOG = IRODS_DIR + FILE_SEPARATOR + "uploaded.txt";
    static String QUEUE_LOG = IRODS_DIR + FILE_SEPARATOR + "queue.txt";
    static String APPLET_LOG = IRODS_DIR + FILE_SEPARATOR + "log.txt";
    static String ACTIVE_APPLETS_LOG = IRODS_DIR + FILE_SEPARATOR + "active_applets.txt";
    static String INACTIVE_APPLETS_LOG = IRODS_DIR + FILE_SEPARATOR + "inactive_applets.txt";
    static String RECOVERY_LOCK_DIR = IRODS_DIR + FILE_SEPARATOR + ".lock" + FILE_SEPARATOR;
    static String PROMPTED_FILE = IRODS_DIR + FILE_SEPARATOR + ".prompted";

    
    private Container content;
    private UploadTableModel model;
    private JButton removeButton;
    private JButton uploadButton;
    private JRadioButton overwriteRadio;
    private JRadioButton checksumRadio;
    private JRadioButton fileSizeRadio;
    private JCheckBox checksumCheckBox;
   
    // log console
    public static JTextArea textArea;
    
    /* Upload options set by user */
    private boolean OVERWRITE_FORCED = true; // upload overwrites remote file; default behavior
    private boolean OVERWRITE_IF_CHECKSUM; // overwrite if checksum is different
    private boolean OVERWRITE_IF_FILE_SIZE; // overwrite if file size is different
    private boolean VERIFY_UPLOAD_WITH_CHECKSUM; // verify upload with checksum comparison; default behavior
    
    private String STR_VERIFY_UPLOAD_WITH_CHECKSUM = "VERIFY_UPLOAD_WITH_CHECKSUM";
    private String STR_RADIO_GROUP = "RADIO_GROUP";
    private String STR_REMOVE = "REMOVE";
    private String STR_UPLOAD = "UPLOAD";
    private String STR_REMOVE_TOOLTIP = "Remove selected files from list";
    private String STR_UPLOAD_TOOLTIP = "Start upload";
    
    // Logger
    static AppletLogger logger = AppletLogger.getInstance();
    
    DragDropPanel dragDropPanel;
    OptionsPanel optionsPanel;
    
    private long id; // applet id for logging purpose
    private URI uri;
    
    
    /**
     * Initiation method.
     * Reads in parameters sent to the applet.
     * Try and set native look and feel.
     * Creates and add panels to the main Container.
     * Add listener for tab focus change event.
     * 
     **/
    public void init() {
        // prompt user for permission to create iRODS directory and log files in the user's home directory
        //
        // 1. check if <iRODS_DIR>/.prompted file exists
        // 2. if yes, create applet panels
        // 3. if no, prompt user for permission
        //     3.a. if user says no, stay on permission screen with a message
        //     3.b. if user says yes
        //          - create <iRODS_DIR>/.prompted file
        //          - create applet panels
        //
        File promptedFile = new File(PROMPTED_FILE);
        if (promptedFile.exists()) {
            createDisplay();
            
            // set the behavior for tool tips
            ToolTipManager toolTipManager = ToolTipManager.sharedInstance();
            toolTipManager.setDismissDelay(60000); // show tip for 1 minute
            toolTipManager.setInitialDelay(0); // show tip immediately
        } else {
            showPrompt();
        }
    }//init
    
    private void showPrompt() {
        JPanel panel = new JPanel(new BorderLayout());
        MyTextArea ta = new MyTextArea("This applet requires some files to be written to your home directory. " +
                                  "These files are used for logging and recovery, and can be viewed by a text editor. " +
                                  "To use this applet, you must click Allow.", 4, 15);
        
        ta.setLineWrap(true);
        ta.setWrapStyleWord(true);
        
        MyButton allowButton = new MyButton("Allow");
        MyButton denyButton = new MyButton("Deny");
        allowButton.setActionCommand("Allow");
        denyButton.setActionCommand("Deny");
        allowButton.addActionListener(this);
        denyButton.addActionListener(this);
        
        MyPanel buttonPanel = new MyPanel();
        buttonPanel.add(allowButton);
        buttonPanel.add(denyButton);
                
        panel.add(ta, BorderLayout.CENTER);
        panel.add(buttonPanel, BorderLayout.PAGE_END);
        
        content = getContentPane();
        content.setBackground(new MyColor());
        content.setLayout(new BorderLayout());
        content.setSize(new Dimension(200, 100));
        content.add(panel);
    }//showPrompt

    public void actionPerformed(ActionEvent e) {
        String action = e.getActionCommand().toUpperCase();
        
        if (action.equals("ALLOW")) {
            //clear components and add table
            content.removeAll();
            createDisplay();
            
            try {
                File promptedFile = new File(PROMPTED_FILE);
                promptedFile.createNewFile();
            } catch (IOException ex) {
                ex.printStackTrace();
            }//try-catch
            
        } else if (action.equals("DENY")) {
            // do nothing
        }//if-else
        
    }//actionPerformed
        
        
    private void createDisplay() {
        // Get the base web host url which will be used to get the temporary password
        String documentBase = getDocumentBase().toString();
        
        // attempt parsing of document base to construct where temporary password service is
        String TEMP_PASSWORD_SERVICE_URL = null;
        
        try {
            URL url = new URL(documentBase);
            TEMP_PASSWORD_SERVICE_URL = url.getProtocol() + "://" + url.getHost();
            
            if (url.getPort() != -1)
                TEMP_PASSWORD_SERVICE_URL += ":" + url.getPort();
            
            // Split the url path
            // then catenate all except the last one, which will be a file reference
            // the result is a folder where the service/getTempPassword.php file is located
            String urlPath = url.getPath();
            String[] p = urlPath.split("/");
            urlPath = "";
            for (int i = 0; i < p.length -1; i++) {
                if (i == 0)
                    urlPath = p[i];
                else
                    urlPath += "/" + p[i];
            }
            
            TEMP_PASSWORD_SERVICE_URL += urlPath + "/services/getTempPassword.php"; // maybe put the string literal outside of class file
//TODO temp!
TEMP_PASSWORD_SERVICE_URL = Account.TEMP_PASSWORD_SERVICE_URL +"/services/getTempPassword.php";
        } catch (MalformedURLException me) {
            logger.log("Problem while getting URL parts. " + me);
            me.printStackTrace();
        }
        
        
        // Sample URI_PARAM = irods://user:pass@host:port/destination
        String ruri = getParameter("ruri"); // passed to applet in html code
        String sessionId = getParameter("ssid"); // web session id passed to applet in html code
        
        if (ruri == null || ruri.trim().equals("")) {
            // TODO what to do?        
        }//if
        //TODO
        try {
          uri = new java.net.URI( ruri );
        } catch( java.net.URISyntaxException e ) {

        }
        
        // go ahead and set the account instance, even if values are null
        // will prompt for user input at end of init method
        Account account = null;
        try {
          account = new Account(TEMP_PASSWORD_SERVICE_URL, uri, sessionId);
        } catch (IOException e) {
          //TODO do something...
          throw new RuntimeException("IO failure");
        }

        
        // create GUI
        content = getContentPane();
        content.setBackground(new MyColor());
        content.setLayout(new BorderLayout());
        content.setSize(new Dimension(300, 400));
        
        model = new UploadTableModel();
        dragDropPanel = new DragDropPanel(this, model, account);
        optionsPanel = new OptionsPanel();
        
        MyTabbedPane tabbedPane = new MyTabbedPane();
        tabbedPane.add("File Upload Table", dragDropPanel); // DragDrop Panel
        tabbedPane.add("Options", optionsPanel); // UploadOptions Panel
        
        content.add(tabbedPane);
        tabbedPane.updateUI();

        /**
         * Compare queue and uploaded log files to determine if any files were not uploaded from last session.
         * 1. Check if both log files exist
         * 2. Compare the two files
         * 3. If content matches, then clear content of both files
         * 4. Else, determine which files need to be added to the table for uploading
         * 
         **/
        
        List queueList = DBUtil.getInstance().getUnassigned();
        if (queueList != null) {
            model.addFile(queueList);
        }
        
        // set queueList to assigned
        DBUtil.getInstance().setAssigned(queueList);
    }//createDisplay
    
    long getId() {
        return id;
    }
    
    public void start() {
    }
    
    public void stop() {
        
    }
    
    public void destroy() {
        DBUtil.getInstance().removeUploaded();
        int rowCount = model.getRowCount();
/*        List itemList = new ArrayList();
        
        for (int k = 0; k < rowCount; k++) {
            String s = ((JTextField) model.getValueAt(k, UploadTableModel.SOURCE_COLUMN)).getText();
            String d = ((JTextField) model.getValueAt(k, UploadTableModel.DESTINATION_COLUMN)).getText();
            String resource = (String) ((JComboBox) model.getValueAt(k, UploadTableModel.RESOURCE_COLUMN)).getSelectedItem();
            
//TODO Shouldn't need to login again just to remove a file from the database...            
            try {
              UploadItem item = new UploadItem(FileFactory.newFile(new URI(s)), 
                FileFactory.newFile(new URI(d)), resource);
              itemList.add(item); 
            } catch (IOException e) {
//TODO ?
              RuntimeException x = new RuntimeException();
              x.initCause(e);
            } catch (URISyntaxException e) {
//TODO ?
              RuntimeException x = new RuntimeException();
              x.initCause(e);              
            }
        }

        DBUtil.getInstance().updateAssigned(itemList, false);
*/        
        DBServer.shutdown();
                
    }//destroy
    
    

    /**
     * Update the upload options as true/false.
     * OVERWRITE_FORCED = if true, upload will overwrite file on remote server
     * OVERWRITE_IF_CHECKSUM = if true, upload will overwrite file on remote server if the local and remote file checksum do NOT match
     * OVERWRITE_IF_FILE_SIZE = if true, upload will overwrite file on remote server if the local and remote file size do NOT match
     * 
     **/
    
    private void updateUploadOption(Object obj) {
        if (obj.getClass() != new JRadioButton().getClass())
            return;
        
        OVERWRITE_FORCED = overwriteRadio.isSelected(); 
        OVERWRITE_IF_CHECKSUM = checksumRadio.isSelected();
        OVERWRITE_IF_FILE_SIZE = fileSizeRadio.isSelected();
    }
    
    
    URI getRuri() 
    {
      return uri;
    }


    void setRuri( URI uri ) 
    {
      this.uri = uri;
    }
    

}//UploadApplet




