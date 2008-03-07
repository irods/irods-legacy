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
//      DBServer.java    -  edu.sdsc.grid.gui.applet.DBServer
//
//  CLASS HIERARCHY
//      java.lang.Object
//          |
//          +-.DBServer
//
//  PRINCIPAL AUTHOR
//      Alex Wu, SDSC/UCSD
//
//

package edu.sdsc.grid.gui.applet;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;

import org.hsqldb.Server;


// This class handles the starting/stopping of the database server
class DBServer {
    private static DBServer instance;
    private static Server server;
    static AppletLogger logger = AppletLogger.getInstance();

    private DBServer() {
    }

    static DBServer getInstance() {
        if (instance == null)
            instance = new DBServer();

        return instance;
    }//getInstance

    public static void start() {
        if (server == null) {
            PrintWriter writer = null;
            try {
                writer = new PrintWriter(UploadApplet.APPLET_LOG);
            } catch (FileNotFoundException e) {
                // no log
            }//try-catch

            server = new org.hsqldb.Server();
            server.setLogWriter(writer); // write to log file instead
            server.setErrWriter(writer);
            server.putPropertiesFromString("database.0=file:" + DBUtil.DB_FILE + ";dbname.0=mydb");

        }//if

        if (server.getState() == 16) { // server is not running
            try {
                server.start();

            } catch (Exception e) {
                logger.log("server.start() exception : " + e);
            }//try-catch
        }

    }//start

    public static void shutdown() {
        if (server != null) {
            server.shutdown();
            File lockFile = new File(DBUtil.DB_LOCK_FILE);

            // delete lock file so other JVMs can access the database
            if (lockFile.exists())
                lockFile.delete();

        }//if
    }//shutdown
}
