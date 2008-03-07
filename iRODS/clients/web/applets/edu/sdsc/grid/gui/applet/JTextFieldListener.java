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
//      JTextFieldListener.java    -  edu.sdsc.grid.gui.applet.JTextFieldListener
//
//  CLASS HIERARCHY
//      java.lang.Object
//          |
//          +-.JTextFieldListener
//
//  PRINCIPAL AUTHOR
//      Alex Wu, SDSC/UCSD
//
//

package edu.sdsc.grid.gui.applet;

import java.awt.event.FocusListener;
import java.awt.event.FocusEvent;
import java.awt.Component;

import javax.swing.JTextField;
import javax.swing.JTable;
import javax.swing.border.EtchedBorder;
import javax.swing.border.EmptyBorder;



class JTextFieldListener implements FocusListener {
    static AppletLogger logger = AppletLogger.getInstance();
    private static EtchedBorder etchedBorder = new EtchedBorder(EtchedBorder.LOWERED);
    private static EmptyBorder emptyBorder = new EmptyBorder(0, 8, 0, 8); // top, left, bottom, right

    public void focusGained(FocusEvent e) {
        JTextField tf = (JTextField) e.getComponent();
        tf.setBorder(etchedBorder);
	tf.selectAll();
    }//focusGained

    public void focusLost(FocusEvent e) {
        JTextField tf = (JTextField) e.getComponent();
        tf.setBorder(emptyBorder);

        Component c = (Component) tf.getParent();

        // JTextField is not calling the JTextFieldEditor.removeCellEditor
        // calling this results in JTextFieldEditor.removeCellEditor being called
        if (c instanceof JTable)
            ((JTable) c).removeEditor();

    }//focusLost
}
