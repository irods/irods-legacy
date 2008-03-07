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
//      UploadProgressThread.java    -  edu.sdsc.grid.gui.applet.UploadProgressThread
//
//  CLASS HIERARCHY
//      java.lang.Object
//          |
//          +-.Thread
//                   |
//                   +-.UploadProgressThread
//
//  PRINCIPAL AUTHOR
//      Alex Wu, SDSC/UCSD
//
//

package edu.sdsc.grid.gui.applet;

import edu.sdsc.grid.io.GeneralFile;
import javax.swing.JProgressBar;
import javax.swing.JTable;
import java.io.File;


class UploadProgressThread extends Thread {
    GeneralFile remoteFile;
    JProgressBar progressBar;
    JTable table;
    int row;
    long fileSize;
    boolean complete = false;

    UploadProgressThread(File localFile, GeneralFile remoteFile, JProgressBar progressBar, JTable table,  int row) {
        this.fileSize = localFile.length();
        this.remoteFile = remoteFile;
        this.progressBar = progressBar;
        this.table = table;
        this.row = row;
        this.start();
    }

    public void run() {

        try {
            long bytesTransferred = 0;
            int percentage = 0;

            while (!complete) {
                //bytesTransferred = remoteFile.fileCopyStatus();
                bytesTransferred = 0;
                percentage = (int) (((float) bytesTransferred) / ((float) fileSize) * 100);
                progressBar.setValue(percentage);
                this.sleep(3000);
            }

        } catch (InterruptedException ie) {
            System.out.println("thread interrupted. " + ie);
        }
    }//run
}