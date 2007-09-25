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
//      Manager.java    -  edu.sdsc.grid.gui.applet.Manager
//
//  CLASS HIERARCHY
//      java.lang.Object
//          |
//          +-.Manager
//
//  PRINCIPAL AUTHOR
//      Alex Wu, SDSC/UCSD
//
//

package edu.sdsc.grid.gui.applet;

import java.util.List;

public class Manager implements AppletConstant {
    protected Manager() {}
    
    private static Manager instance;
    private static int appletCount; // how many applet instances are active
    // Logger
    static AppletLogger logger = AppletLogger.getInstance();
    
    public static Manager getInstance() {
        if (instance == null)
            instance = new Manager();
        
        return instance;
    }
    
    public static void registerApplet() {
        appletCount++;
        logger.log("applet count is now: " + appletCount);    
    }
    
    public static void unregisterApplet() {
        if (appletCount > 0) // should never be negative
            appletCount--;
            
        logger.log("applet count is now: " + appletCount);
    }
    
    public static int getAppletCount() {
        return appletCount;
    }
    
    /* Log files methods */
    public static List recoverQueue() {
        // if queue log differs from completed log, then
        // some files were not uploaded
        // once recovered, both log files should be cleared
        // AND the queue log be rewritten
        //if (appletCount != 1) return null;
        
        // have any applet recover queue at the moment
        // will need to create a file lock to determine if
        // queue should be recovered or not
        //
        // a file lock will help in situations
        // where a user opens applets on multiple browser vendors
        
        AppletLogger logger = AppletLogger.getInstance();
        return logger.recoverQueue();
    }
    
}