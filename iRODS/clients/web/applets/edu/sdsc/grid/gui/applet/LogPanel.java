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
//      LogPanel.java    -  edu.sdsc.grid.gui.applet.LogPanel
//
//  CLASS HIERARCHY
//      java.lang.Object
//          |
//          +-.JPanel
//                   |
//                   +-.LogPanel
//
//  PRINCIPAL AUTHOR
//      Alex Wu, SDSC/UCSD
//
//

package edu.sdsc.grid.gui.applet;

import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JPanel;

public class LogPanel extends JPanel {
    //private static LogPanel instance = null;
    private JTextArea textArea;
    
    public LogPanel() {
        init();
    }
    /*
    public static LogPanel getInstance() {
        if(instance == null)
            instance = new LogPanel();
        
        return instance;
    }
    */
    /**
     * Creates a scrollable text area and add to this JPanel
     **/
    public void init() {
        //static JTextArea textArea = new JTextArea(15, 48); // row, col
        textArea = new JTextArea(15, 50); // row, col
        textArea.setLineWrap(true);
        textArea.setWrapStyleWord(true);
        textArea.setEditable(false);
        //Create the scroll pane and add the textarea to it.
        JScrollPane textScrollPane = new JScrollPane(textArea);
        this.add(textScrollPane);
    }//init
 
    /**
     * Add a line of string to the text area
     **/
    public void append(String s) {
        try {
            textArea.append(s);
        } catch (Exception e) {
            // may be null if panel is not set up yet
            // ignore exception
        }
    }
}