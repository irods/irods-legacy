/*
 * DBUtil.java
 * 
 * Created on Oct 10, 2007, 9:13:18 AM
 * 
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.sdsc.grid.gui.applet;

import java.util.List;
import java.util.ArrayList;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

/**
 *
 * @author awu
 */
public class DBUtil implements AppletConstant {
    private Connection c;
    private Statement stmt;
    private ResultSet rs;
    private static AppletLogger logger = AppletLogger.getInstance();
    
    public DBUtil() {
        init();
    }
    
    private void init() {
        setConnection();
        if (! tableExists()) {
            createTable();
        }
    }
    
    private void setConnection() {
        if (c == null)
            try {
                // Load the HSQL Database Engine JDBC driver
                // hsqldb.jar should be in the class path or made part of the current jar
                Class.forName("org.hsqldb.jdbcDriver");
                c = DriverManager.getConnection(DB_FILE, DB_USER, DB_PASSWORD);


            } catch (ClassNotFoundException e) {
                e.printStackTrace();
            } catch (SQLException ex) {
                ex.printStackTrace();
            }        
    }//setConnection
    
    private boolean tableExists() {
        beginTransaction();
        String sql = "SELECT count(*) FROM queue";
        
        try {
            rs = stmt.executeQuery(sql);
            if (rs.next()) {
                return true;
            }
            
        } catch (SQLException ex) {
            ex.printStackTrace();
        } finally {
            endTransaction();
        }

        return false;
        
    }//tableExists
    
    private void createTable() {
        String sql = "CREATE TABLE queue (" +
                       "source VARCHAR," +
                       "destination VARCHAR," + 
                       "resource VARCHAR," +
                       "status VARCHAR," +
                       "type VARCHAR," +
                       "assigned BOOLEAN," +
                       "PRIMARY KEY(source, destination, resource)" +
                     ")";

        beginTransaction();
        try {
            stmt.execute(sql);
        } catch (SQLException ex) {
            ex.printStackTrace();
        } finally {
            endTransaction();
        }
    }//createTable
    
    public boolean insert(UploadItem item) {
        beginTransaction();
        boolean rowExist = false;
        String sql = "SELECT COUNT(*) FROM queue WHERE source = ? AND destination = ? AND resource = ?";

        try {
            PreparedStatement p = c.prepareStatement(sql);
            p.setString(1, item.getSource());
            p.setString(2, item.getDestination());
            p.setString(3, item.getResource());
            rs = p.executeQuery();
            
            if (rs.next())
                if (rs.getInt(1) > 0)
                    rowExist = true;
            
        } catch (SQLException ex) {
            ex.printStackTrace();
        } 

        
        sql = "INSERT INTO queue (source, destination, resource, status, type, assigned) " + 
                     "VALUES(?, ?, ?, ?, ?, ?) ";
        if (!rowExist)
            try {
                PreparedStatement p = c.prepareStatement(sql);
                p.setString(1, item.getSource());
                p.setString(2, item.getDestination());
                p.setString(3, item.getResource());
                p.setString(4, item.getStatus());
                p.setString(5, item.getType());
                p.setBoolean(6, true);
                p.execute();
                return true;
            } catch (SQLException ex) {
                ex.printStackTrace();
                return false;
            } finally {
                endTransaction();
            }
            
        return false;
    } 
    
    public List getUnassigned() {
        beginTransaction();
        List itemList = new ArrayList();
        String sql = "SELECT source, destination, resource FROM queue WHERE assigned = false AND status != '" +  STATUS_UPLOADED + "'";

        try {
            stmt = c.createStatement();
            rs = stmt.executeQuery(sql);
            
            //public UploadItem(String source, String destination, String resource) {
            UploadItem item = null;
            while (rs.next()) {
                item = new UploadItem(rs.getString(1),
                                      rs.getString(2),
                                      rs.getString(3));
                
                itemList.add(item);
            }
            
        } catch (SQLException ex) {
            ex.printStackTrace();
        } finally {
            endTransaction();
        }

        return itemList;
        
    }//getUnassigned 
    
    
    public void setAssigned(List itemList) {
        beginTransaction();
        String sql = "UPDATE queue SET assigned = true WHERE source = ? AND destination = ? AND resource = ?";

        // TODO: batch sql update
        PreparedStatement pstmt = null;
                    
        try {
            pstmt = c.prepareStatement(sql);
            for (int i=0; i<itemList.size(); i++) {
                UploadItem item = (UploadItem) itemList.get(i);
                pstmt.setString(1, item.getSource());
                pstmt.setString(2, item.getDestination());
                pstmt.setString(3, item.getResource());
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
    
    public void delete(List itemList) {
        beginTransaction();
        String sql = "DELETE FROM queue WHERE source = ? AND destination = ? AND resource = ?";

        // TODO: batch sql update
        PreparedStatement pstmt = null;
                    
        try {
            pstmt = c.prepareStatement(sql);
            for (int i=0; i<itemList.size(); i++) {
                UploadItem item = (UploadItem) itemList.get(i);
                pstmt.setString(1, item.getSource());
                pstmt.setString(2, item.getDestination());
                pstmt.setString(3, item.getResource());
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
    
    public void updateStatus(UploadItem item, String status) {
        beginTransaction();
        String sql = "UPDATE queue SET status = ? WHERE source = ? AND destination = ? AND resource = ?";

        // TODO: batch sql update
        PreparedStatement pstmt = null;
                    
        try {
            pstmt = c.prepareStatement(sql);
            pstmt.setString(1, status);
            pstmt.setString(2, item.getSource());
            pstmt.setString(3, item.getDestination());
            pstmt.setString(4, item.getResource());
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
    }

    public void updateAssigned(List itemList, boolean assigned) {
        beginTransaction();
        String sql = "UPDATE queue SET assigned = ? WHERE source = ? AND destination = ? AND resource = ?";

        // TODO: batch sql update
        PreparedStatement pstmt = null;
                    
        try {
            pstmt = c.prepareStatement(sql);
            
            for (int i=0; i<itemList.size(); i++) {
                UploadItem item = (UploadItem) itemList.get(i);
                pstmt.setBoolean(1, assigned);
                pstmt.setString(2, item.getSource());
                pstmt.setString(3, item.getDestination());
                pstmt.setString(4, item.getResource());
                pstmt.execute();
            }

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

    
    public void removeUploaded() {
        String sql = "DELETE FROM queue WHERE status = '" + STATUS_UPLOADED + "'";
                    
        try {
            stmt = c.createStatement();
            stmt.execute(sql);
        } catch (SQLException ex) {
            ex.printStackTrace();
        } finally {
            endTransaction();
        }//try-catch

    }//removeUploaded
    
    

    
    public boolean query() {
        return true;
    }
    
    private void beginTransaction() {
        setConnection();
        
        try {
            stmt = c.createStatement();
        } catch (SQLException ex) {
            ex.printStackTrace();
        }
       
        
    }
    
    private void endTransaction() {
        try {
            if (stmt != null)
                stmt.close();
        } catch (SQLException ex) {
            ex.printStackTrace();
        }
        
        try {
            if (rs != null)
                rs.close();
        } catch (SQLException ex) {
            ex.printStackTrace();
        }
        
    }
    
}
