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
//      JComboBoxEditor.java    -  edu.sdsc.grid.gui.applet.JComboBoxEditor
//
//  CLASS HIERARCHY
//      java.lang.Object
//          |
//          +-.JComboBoxEditor
//
//  PRINCIPAL AUTHOR
//      Alex Wu, SDSC/UCSD
//
//

package edu.sdsc.grid.gui.applet;

import java.awt.Component;
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import java.util.EventObject;

import javax.swing.JComboBox;
import javax.swing.JTable;
import javax.swing.event.CellEditorListener;
import javax.swing.table.TableCellEditor;
/**
 *
 * @author awu
 */

// TODO: Add listener for selection change
// will need to update DB with new selection
class JComboBoxEditor implements TableCellEditor, ActionListener {
    static AppletLogger logger = AppletLogger.getInstance();

    public Component getTableCellEditorComponent ( JTable table, Object value, boolean isSelected, int row, int column )  {
        JComboBox box = (JComboBox) value;
        if (isSelected)
            box.setBackground(new MyColor()); // RGB of #c4d2e3; light blue)
        else
            box.setBackground(Color.WHITE);

        box.removeActionListener(this);
        box.addActionListener(this);

        return  box;

    }//getTableCellEditorComponent

    public void actionPerformed(ActionEvent e) {
        JComboBox cb = (JComboBox)e.getSource();
        String resource = (String)cb.getSelectedItem();

    }

    public void removeCellEditorListener(CellEditorListener l) {

    }


    public void addCellEditorListener(CellEditorListener l) {

    }

    public void cancelCellEditing() {

    }

    public boolean stopCellEditing() {

        return true;

    }

    public boolean shouldSelectCell(EventObject anEvent) {

        return true;
    }

    public boolean isCellEditable(EventObject e) {

        return true;

    }

    public Object getCellEditorValue() {

        return null;
    }
}
