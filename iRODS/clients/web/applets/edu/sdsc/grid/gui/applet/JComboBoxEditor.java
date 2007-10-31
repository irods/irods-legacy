/*
 * JComboBoxEditor.java
 * 
 * Created on Oct 16, 2007, 3:59:30 PM
 * 
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.sdsc.grid.gui.applet;

import java.awt.Component;
import java.awt.event.MouseEvent;
import java.util.EventObject;
import javax.swing.JComboBox;
import javax.swing.JTable;
import javax.swing.event.CellEditorListener;
import javax.swing.table.TableCellEditor;
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
/**
 *
 * @author awu
 */

// TODO: Add listener for selection change
// will need to update DB with new selection
public class JComboBoxEditor implements TableCellEditor, ActionListener {
    static AppletLogger logger = AppletLogger.getInstance();        
    
    public Component getTableCellEditorComponent ( JTable table, Object value, boolean isSelected, int row, int column )  {
        JComboBox box = (JComboBox) value;
        if (isSelected)
            box.setBackground(new Color(196, 210, 227)); // RGB of #c4d2e3; light blue)
        else
            box.setBackground(Color.WHITE);
        
        box.removeActionListener(this);
        box.addActionListener(this);
        
        return  box;
        
    }
    
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
