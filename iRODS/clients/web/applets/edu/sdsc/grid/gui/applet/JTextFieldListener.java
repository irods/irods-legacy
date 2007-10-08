/*
 * JTextFieldListener.java
 *
 * Created on September 21, 2007, 10:20 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package edu.sdsc.grid.gui.applet;

import java.awt.event.FocusListener;
import java.awt.event.FocusEvent;
import java.awt.Component;
import java.awt.Color;
import javax.swing.JTextField;
import javax.swing.JTable;
import javax.swing.border.EtchedBorder;
import javax.swing.border.EmptyBorder;
import javax.swing.border.LineBorder;

/**
 *
 * @author awu
 */
public class JTextFieldListener implements FocusListener {
    static AppletLogger logger = AppletLogger.getInstance();
    private static EtchedBorder etchedBorder = new EtchedBorder(EtchedBorder.LOWERED);
    private static EmptyBorder emptyBorder = new EmptyBorder(0, 8, 0, 8); // top, left, bottom, right
    //private static LineBorder lineBorder = new LineBorder(Color.lightGray);
    
    public void focusGained(FocusEvent e) {

        JTextField tf = (JTextField) e.getComponent();
        tf.setBorder(etchedBorder);
	tf.selectAll();

    }

    public void focusLost(FocusEvent e) {
        
        JTextField tf = (JTextField) e.getComponent();        
        tf.setBorder(emptyBorder);
        
        Component c = (Component) tf.getParent();
        
        // JTextField is not calling the JTextFieldEditor.removeCellEditor
        // calling this results in JTextFieldEditor.removeCellEditor being called
        if (c instanceof JTable)
            ((JTable) c).removeEditor();
        

    }  
    
  
}
