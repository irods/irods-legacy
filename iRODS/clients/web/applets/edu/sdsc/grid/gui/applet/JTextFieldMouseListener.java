/*
 * JTextFieldMouseListener.java
 *
 * Created on September 21, 2007, 3:14 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package edu.sdsc.grid.gui.applet;

import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
/**
 *
 * @author awu
 */
public class JTextFieldMouseListener  extends MouseAdapter {
    static AppletLogger logger = AppletLogger.getInstance();
    
    public void mouseClicked(MouseEvent e) {
        //logger.log("Mouse clicked.");
        if (e.getClickCount() > 1) {
            //logger.log("at least double click.");
        }
        else
            e.consume();
    }

    
}
