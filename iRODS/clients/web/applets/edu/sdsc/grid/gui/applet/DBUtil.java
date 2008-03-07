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
//      DBUtil.java    -  edu.sdsc.grid.gui.applet.DBUtil
//
//  CLASS HIERARCHY
//      java.lang.Object
//          |
//          +-.DBUtil
//
//  PRINCIPAL AUTHOR
//      Alex Wu, SDSC/UCSD
//
//

package edu.sdsc.grid.gui.applet;

import java.io.IOException;
import java.net.URISyntaxException;

import java.util.List;
import java.util.ArrayList;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

// This class handles the interaction with the database
// contains SQL statements
public class DBUtil {
    static String DB_FILE = UploadApplet.IRODS_DIR + UploadApplet.FILE_SEPARATOR + "mydb";
    static String DB_LOCK_FILE = DB_FILE + ".lck";
    static String DB_SERVER_URL = "jdbc:hsqldb:hsql://localhost/mydb";
    static String DB_USER = "sa";
    static String DB_PASSWORD = "";
    static String DB_SCRIPT_FILE = UploadApplet.IRODS_DIR + UploadApplet.FILE_SEPARATOR + "db.script";
    
    private static DBUtil instance;
    private static Connection c;
    private static Statement stmt;
    private static AppletLogger logger = AppletLogger.getInstance();
    
    private DBUtil() {
    }
    
    public static DBUtil getInstance() {
        if (instance == null) {
            instance = new DBUtil();
            instance.init();
            
        }//if
        
        return instance;
    }//getInstance
    
    private static void init() {
        if (! tableExists()) {
            createTables();
        }
        
    }//init
    
    
    // checks to see if the tables have be created
    private static boolean tableExists() {
        beginTransaction();
        String sql = "SELECT count(*) FROM queue";
        ResultSet rs = null;
        
        try {
            rs = stmt.executeQuery(sql);
            if (rs.next()) {
                return true;
            }
            
        } catch (SQLException ex) {
            ex.printStackTrace();
        } finally {
            try {
                if (rs != null)
                    rs.close();
            } catch (SQLException e) {
                // no log
            }
            
            endTransaction();
        }//try-catch-finally

        return false;
        
    }//tableExists
    
    
    // creates the tables
    private static void createTables() {
        beginTransaction();
        
        String sql = "CREATE TABLE queue (" +
                       "source VARCHAR," +
                       "destination VARCHAR," + 
                       "resource VARCHAR," +
                       "status VARCHAR," +
                       "type VARCHAR," +
                       "assigned BOOLEAN," +
                       "PRIMARY KEY(source, destination)" +
                     ")";

        
        try {
            stmt.execute(sql);
            
            sql = "CREATE TABLE resource (" +
                    "host VARCHAR," +
                    "port INTEGER," + 
                    "name VARCHAR," +
                    "PRIMARY KEY (host, port, name)" +
                  ")";
            
            stmt.execute(sql);
        } catch (SQLException ex) {
            ex.printStackTrace();
        } finally {
            endTransaction();
        }//try-catch-finally
    }//createTable
    
    
    // first check to see if the UploadItem already exists, if not then
    // inserts data into the table
    // UploadItem contains information about the file/folder to be uploaded
    // and where it will be uploaded
    public synchronized static boolean insert(UploadItem item) {
/*        beginTransaction();
        boolean rowExist = false;
        String sql = "SELECT COUNT(*) FROM queue WHERE source = ? AND destination = ?";
        ResultSet rs = null; 
        
        try {
            PreparedStatement p = c.prepareStatement(sql);
            p.setString(1, item.getSource());
            p.setString(2, item.getDestination());
            rs = p.executeQuery();
            
            if (rs.next())
                if (rs.getInt(1) > 0)
                    rowExist = true;
            
        } catch (SQLException ex) {
            ex.printStackTrace();
        }//try-catch
        
        sql = "INSERT INTO queue (source, destination, resource, status, type, assigned) " + 
                     "VALUES(?, ?, ?, ?, ?, ?) ";
        
        try {
            if (!rowExist) {
                PreparedStatement p = c.prepareStatement(sql);
                p.setString(1, item.getSource());
                p.setString(2, item.getDestination());
                p.setString(3, item.getSelectedResource());
                p.setString(4, item.getStatus());
                p.setString(5, item.getType());
                p.setBoolean(6, true);
                p.execute();
                stmt.executeQuery("CHECKPOINT");
                return true;
            }
        } catch (SQLException ex) {
            ex.printStackTrace();
            return false;
        } finally {
            try {
                if (rs != null)
                    rs.close();
            } catch (SQLException e) {
                // no log
            }
            
            endTransaction();
        }//try-catch-finally
        
        
        return false;*/return true;
    }//insert
    
    
    // returns a list of files/folders that have not been uploaded (assigned == false)
    // there will be items that have not been uploaded if the applet
    // closes while there are still items not uploaded successfully
    public synchronized static List getUnassigned() {
        beginTransaction();
        List itemList = new ArrayList();
        String sql = "SELECT source, destination, resource FROM queue WHERE assigned = false AND status != '" +  UploadItem.STATUS_UPLOADED + "'";
        ResultSet rs = null;
        
        try {
            rs = stmt.executeQuery(sql); // returns one row ONLY
            while (rs.next()) {
                UploadItem item = new UploadItem(rs.getString(1),
                                      rs.getString(2),
                                      rs.getString(3));
                
                itemList.add(item);
            }
            
        } catch (SQLException ex) {
            ex.printStackTrace();
            logger.log("getUnassigned: " + ex);            
        } catch (URISyntaxException ex) {
            ex.printStackTrace();
            logger.log("URISyntaxException: " + ex);             
        } catch (IOException ex) {
            ex.printStackTrace();
            logger.log("IOException: " + ex);          
        } finally {
            try {
                if (rs != null)
                    rs.close();
            } catch (SQLException e) {
                // no log
            }
            
            endTransaction();
        }//try-catch-finally

        return itemList;
        
    }//getUnassigned 
    
    
    // updates the table and sets the value for assigned to true
    // this is to prevent multiple applets from loading
    // the same items for uploading
    public synchronized static void setAssigned(List itemList) {
        beginTransaction();
        String sql = "UPDATE queue SET assigned = true WHERE source = ? AND destination = ?";

        // TODO: batch sql update
        PreparedStatement pstmt = null;
                    
        try {
            pstmt = c.prepareStatement(sql);
            for (int i=0; i<itemList.size(); i++) {
                UploadItem item = (UploadItem) itemList.get(i);
                pstmt.setString(1, item.getSource());
                pstmt.setString(2, item.getDestination());
                pstmt.execute();
            }//for
        } catch (SQLException ex) {
            ex.printStackTrace();
        } finally {
            try {
                if (pstmt != null)
                    pstmt.close();
            } catch (Exception e) {
                // do nothing
            }
            endTransaction();
        }//try-catch
        
    }//setUnassigned 
    
    
    // remove the items from the database
    public synchronized static void delete(List itemList) {
        beginTransaction();
        String sql = "DELETE FROM queue WHERE source = ? AND destination = ?";

        // TODO: batch sql update
        PreparedStatement pstmt = null;
                    
        try {
            pstmt = c.prepareStatement(sql);
            for (int i=0; i<itemList.size(); i++) {
                UploadItem item = (UploadItem) itemList.get(i);
                pstmt.setString(1, item.getSource());
                pstmt.setString(2, item.getDestination());
                pstmt.execute();
            }//for
        } catch (SQLException ex) {
            ex.printStackTrace();
        } finally {
            try {
                if (pstmt != null)
                    pstmt.close();
            } catch (Exception e) {
                // do nothing
            }
            endTransaction();
        }//try-catch

    }//delete
    
    
    // updates the status for the item
    // status indicates if the item has been uploaded, failed, etc.
    public synchronized static void updateStatus(UploadItem item, String status) {
        beginTransaction();
        String sql = "UPDATE queue SET status = ? WHERE source = ? AND destination = ?";

        // TODO: batch sql update
        PreparedStatement pstmt = null;
                    
        try {
            pstmt = c.prepareStatement(sql);
            pstmt.setString(1, status);
            pstmt.setString(2, item.getSource());
            pstmt.setString(3, item.getDestination());
            pstmt.execute();

        } catch (SQLException ex) {
            ex.printStackTrace();
        } finally {
            try {
                if (pstmt != null)
                    pstmt.close();
            } catch (Exception e) {
                // do nothing
            }
            endTransaction();
        }//try-catch        
    }//updateStatus

    
    public synchronized static void updateAssigned(List itemList, boolean assigned) {
        beginTransaction();
        String sql = "UPDATE queue SET assigned = ?, resource = ? WHERE source = ? AND destination = ?";
        
        PreparedStatement pstmt = null;
                    
        try {
            pstmt = c.prepareStatement(sql);
            
            for (int i=0; i<itemList.size(); i++) {
                
                UploadItem item = (UploadItem) itemList.get(i);
                pstmt.setBoolean(1, assigned);
                pstmt.setString(2, item.getSelectedResource());
                pstmt.setString(3, item.getSource());
                pstmt.setString(4, item.getDestination());
                pstmt.execute();
                
            }//for
            
            
        } catch (SQLException ex) {
            ex.printStackTrace();
        } finally {
            try {
                if (pstmt != null)
                    pstmt.close();
            } catch (Exception e) {
                // do nothing
            }
            endTransaction();
        }//try-catch        
    }//updateAssigned

    
    public synchronized static void removeUploaded() {
        beginTransaction();
        String sql = "DELETE FROM queue WHERE status = '" + UploadItem.STATUS_UPLOADED + "'";
        
        try {
            stmt = c.createStatement();
            stmt.execute(sql);
        } catch (SQLException ex) {
            ex.printStackTrace();
        } finally {
            endTransaction();
        }//try-catch

    }//removeUploaded
    
    
    // adds the irods resource
    public synchronized static void addResource(String host, int port, String resource) {
        beginTransaction();
        
        // TODO: batch sql update
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // was not able to use PreparedStatement
            // got Unsupported sql exception with HSQL jdbc
            String sql = "SELECT COUNT(*) FROM resource WHERE host = '" + host + "' AND port = " + port + " AND name = '" + resource + "'";
            rs = stmt.executeQuery(sql);

            if (rs.next()) {
                //logger.log("rs.getInt(1) : " + rs.getInt(1));
                if (rs.getInt(1) == 0) {
             
                    sql = "INSERT INTO resource (host, port, name) VALUES (?, ?, ?)";
                    pstmt = c.prepareStatement(sql);
                    pstmt.setString(1, host);
                    pstmt.setInt(2, port);
                    pstmt.setString(3, resource);
                    pstmt.execute();
                }
            }
        } catch (SQLException ex) {
            ex.printStackTrace();
            logger.log("addResource. " + ex);
        } finally {
            try {
                if (pstmt != null)
                    pstmt.close();
            } catch (Exception e) {
                logger.log("addResource(). " + e);
            }
            
            try {
                if (rs != null)
                    rs.close();
            } catch (SQLException e) {
                //no log
            }
                    
            endTransaction();
        }//try-catch 
    }

    // returns a list of irods resources
    public synchronized static List getResourceList(String host, int port) {
        beginTransaction();
        String sql = "SELECT name FROM resource WHERE host = ? and port = ?";
        PreparedStatement pstmt = null;
        List resourceList = new ArrayList();
        ResultSet rs = null;
        
        try {
            pstmt = c.prepareStatement(sql);
            pstmt.setString(1, host);
            pstmt.setInt(2, port);
            rs = pstmt.executeQuery();
            
            while (rs.next()) {
                resourceList.add(rs.getString(1));
            }//while
            
        } catch (SQLException ex) {
            ex.printStackTrace();
        } finally {
            try {
                if (pstmt != null)
                    pstmt.close();
            } catch (Exception e) {
                // do nothing
            }
            
            try {
                if (rs != null)
                    rs.close();
            } catch (SQLException e) {
                // no log
            }
            
            endTransaction();
        }//try-catch-finally
        
        return resourceList;
    }//getResourceList
    
    public boolean query() {
        return true;
    }//query
    
    private static void setConnection() {
        try {
            if (c == null) {
            
                // Load the HSQL Database Engine JDBC driver
                // hsqldb.jar should be in the class path or made part of the current jar
                Class.forName("org.hsqldb.jdbcDriver");
                c = DriverManager.getConnection(DB_SERVER_URL, DB_USER, DB_PASSWORD);
        
            }//if
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (SQLException ex) {
            ex.printStackTrace();
        }//try-catch        
    }//setConnection
    
    
    // makes sure the databaser server is running before each transaction
    // this is because multiple applets may be using the same database, and each
    // applet shuts down the database when the applet is destroyed
    private static void beginTransaction() {
        // start db server
        DBServer.getInstance().start();
        
        setConnection();
        
        try {
            stmt = c.createStatement();
        } catch (SQLException ex) {
            ex.printStackTrace();
        }//try-catch
       
    }//beginTransaction
    
    private static void endTransaction() {
        try {
            if (stmt != null) {
                stmt.executeQuery("CHECKPOINT");
                stmt.close();
            }
        } catch (SQLException ex) {
            //ex.printStackTrace();
        }//try-catch
        
        
        try {
            if (c != null) {
                c.close();
                c = null;
            }
        } catch (SQLException ex) {
            logger.log("Closing connection exception. " + ex);
        }//try-catch
        
    }//endTransaction
    
}
