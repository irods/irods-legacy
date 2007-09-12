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

import java.io.File;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.util.Map;
import java.util.HashMap;
import java.util.Vector;
import java.util.List;
import java.util.Iterator;
import java.util.Properties;

import javax.swing.JApplet;
import javax.swing.JScrollPane;
import javax.swing.JButton;
import javax.swing.JTextField;
import javax.swing.JPasswordField;
import javax.swing.JTable;
import javax.swing.JLabel;
import javax.swing.JProgressBar;
import javax.swing.JOptionPane;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JTabbedPane;
import javax.swing.JCheckBox;
import javax.swing.JRadioButton;
import javax.swing.ButtonGroup;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;
import javax.swing.table.TableColumn;
import javax.swing.table.TableColumnModel;
import javax.swing.table.DefaultTableColumnModel;
import javax.swing.BoxLayout;
import javax.swing.SpringLayout;
import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;
import javax.swing.SwingUtilities;
import javax.swing.JTextArea;
import javax.swing.JOptionPane;

import java.net.URI;
import java.net.URISyntaxException;
import java.net.CookieHandler;
import java.net.URL;
import java.net.MalformedURLException;
import java.net.URISyntaxException;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.Container;
import java.awt.Color;
import java.awt.Rectangle;
import java.awt.FlowLayout;
import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.MouseAdapter;
import java.awt.event.ActionListener;
import java.awt.dnd.DnDConstants;
import java.awt.dnd.DropTarget;
import java.awt.event.ActionEvent;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import edu.sdsc.grid.io.srb.SRBException;
import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.srb.SRBFile;
import edu.sdsc.grid.io.srb.SRBFileSystem;
import edu.sdsc.grid.io.srb.SRBAccount;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.local.LocalFile;

import edu.sdsc.grid.io.irods.IRODSFile;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.grid.io.irods.IRODSAccount;

/**
 * Main Applet window. Creates and adds other JPanels to the main Container.
 *
 * @author      Alex Wu, San Diego Supercomputer Center
 * 
 **/

public class UploadApplet extends JApplet implements AppletConstant {
    private Container content;
    private JTable table;
    private UploadTableModel model;
    //private DropTarget dropTarget;
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
    
    //AccountPanel accountPanel;
    DragDropPanel dragDropPanel;
    OptionsPanel optionsPanel;
    //LogPanel logPanel;
    
    static Manager manager = Manager.getInstance();
    
    /**
     * Initiation method.
     * Reads in parameters sent to the applet.
     * Try and set native look and feel.
     * Creates and add panels to the main Container.
     * Add listener for tab focus change event.
     * 
     **/
    public void init() {
        
        // try to load native look and feel
        // not using native look anymore
        // will use look of web client
        /*
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (UnsupportedLookAndFeelException e) {
            logger.log("Can't load native look and feel." + e);
        } catch (ClassNotFoundException ce) {
            logger.log("Could not find class: " + ce);
        } catch (InstantiationException ie) {
            logger.log("Could not instantiate class: " + ie);
        } catch (IllegalAccessException ile) {
            logger.log("Illegal access exc. : " + ile);
        }
        */
        
        
        // Get the base web host url which will be used to get the temporary password
        String documentBase = getDocumentBase().toString();
        //logger.log("documentBase : " + documentBase);
        
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

        } catch (MalformedURLException me) {
            logger.log("Problem while getting URL parts. " + me);
            me.printStackTrace();
        }
        
        
        // Sample URI_PARAM = irods://user:pass@host:port/destination
        String ruri = getParameter("ruri"); // passed to applet in html code
        //logger.log("ruri: " + ruri);
        String sessionId = getParameter("ssid"); // web session id passed to applet in html code
        //logger.log("Session id passed is : " + sessionId);
        
        
        if (ruri == null || ruri.trim().equals("")) {
            // what to do?
        }//if
        
        // go ahead and set the account instance, even if values are null
        // will prompt for user input at end of init method
        //Account account = new Account(host, port, username, password, destination, zone, defaultResource);
        Account account = new Account(TEMP_PASSWORD_SERVICE_URL, ruri, sessionId);
        
        // create GUI
        Color bgColor = new Color(196, 210, 227); // RGB of #c4d2e3; light blue
        content = getContentPane();
        content.setBackground(bgColor);
        content.setLayout(new BorderLayout());
        content.setSize(new Dimension(300, 200));
        
        model = new UploadTableModel();
        dragDropPanel = new DragDropPanel(model, account);
        optionsPanel = new OptionsPanel();
        
        // set background color
        dragDropPanel.setBackground(bgColor);
        optionsPanel.setBackground(bgColor);
        
        JTabbedPane tabbedPane = new JTabbedPane();
        tabbedPane.setBackground(bgColor);
        tabbedPane.add("File Upload Table", dragDropPanel); // DragDrop Panel
        tabbedPane.add("Options", optionsPanel); // UploadOptions Panel
        content.add(tabbedPane);
        manager.registerApplet();

        /**
         * Compare queue and uploaded log files to determine if any files were not uploaded from last session.
         * 1. Check if both log files exist
         * 2. Compare the two files
         * 3. If content matches, then clear content of both files
         * 4. Else, determine which files need to be added to the table for uploading
         * 
         **/
        List queueList = manager.recoverQueue();
        
        if (queueList != null) {
            // open a new tab and ask user if they would like to load files from previous session
            // if yes, load files and prompt user for password for each distinct RURI, if password is not found in Hashmap
            
            model.addFile(queueList);
        }//if
        
        // prompt for password, if needed
        if (account.getPassword(ruri) == null && account.getSessionId() == null) {
            // prompt user for password for the specific RURI excluding the destination path
            // convert user input into char array
            String inputValue = JOptionPane.showInputDialog(content, "Please enter your password for " + account.scrubPasswordFromRuri() + "  .");
            
            if (inputValue != null && !inputValue.trim().equals("")) {
                char[] password = inputValue.toCharArray();
                account.setPassword(ruri, password);
            }
        
        }//if
        
    }//init

    
    public void start() {
    }
    
    public void stop() {
    }
    
    public void destroy() {
        manager.unregisterApplet();
    }
    
    

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
    

}//UploadApplet




