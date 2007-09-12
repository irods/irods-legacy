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
//      AppletConstant.java    -  edu.sdsc.grid.gui.applet.AppletConstant
//
//  CLASS HIERARCHY
//      java.lang.Object
//          |
//          +-.AppletConstant
//
//
//  PRINCIPAL AUTHOR
//      Alex Wu, SDSC/UCSD
//
//

package edu.sdsc.grid.gui.applet;

public interface AppletConstant {
    static String USER_HOME = System.getProperty("user.home");
    static String FILE_SEPARATOR = System.getProperty("file.separator");
    static String IRODS_DIR = USER_HOME + FILE_SEPARATOR + ".irods";
    static String UPLOADED_LOG = IRODS_DIR + FILE_SEPARATOR + "uploaded.txt";
    static String QUEUE_LOG = IRODS_DIR + FILE_SEPARATOR + "queue.txt";
    static String APPLET_LOG = IRODS_DIR + FILE_SEPARATOR + "log.txt";
    
    /* Column index for table */
    static int ICON_COLUMN = 0;
    static int LOCAL_COLUMN = 1;
    static int REMOTE_COLUMN = 2;
    static int FILE_SIZE_COLUMN = 3;
    static int STATUS_COLUMN = 4;
    
    static String DONE_STATUS = "Done";
    static String FAILED_STATUS = "Failed";
    
}
