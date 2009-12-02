//	Copyright (c) 2005, Regents of the University of California
//	All rights reserved.
//
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided that the following conditions are
//	met:
//
//	  * Redistributions of source code must retain the above copyright notice,
//	this list of conditions and the following disclaimer.
//	  * Redistributions in binary form must reproduce the above copyright
//	notice, this list of conditions and the following disclaimer in the
//	documentation and/or other materials provided with the distribution.
//	  * Neither the name of the University of California, San Diego (UCSD) nor
//	the names of its contributors may be used to endorse or promote products
//	derived from this software without specific prior written permission.
//
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//	IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//  FILE
//	JargonTreeCellRenderer.java	-  edu.sdsc.grid.gui.JargonTreeCellRenderer
//
//  CLASS HIERARCHY
//	javax.swing.tree.TreeCellRenderer
//	    |
//	    +-edu.sdsc.grid.gui.JargonTreeCellRenderer
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.gui;


import java.awt.Color;
import java.awt.Component;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.tree.TreeCellRenderer;

import javax.swing.*;

import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.local.LocalFile;

/**
 * Displays an entry in a tree. <p>
 * A custom tree cell renderer to compliment the <code>JargonTreeModel</cope>
 * The GeneralFile classes which form the nodes of the tree will be displayed,
 * as inline JLabel, and expanded to a table for filesystems that
 * support metadata.
 *
 * @author Lucas Ammon Gilbert
 * @see JargonTreeModel
 * @see JargonTreeCellEditor
 * @since JARGON1.5
 * @deprecated - GUI code will go away in future releases
 */
@Deprecated
public class JargonTreeCellRenderer implements TreeCellRenderer
{
	/**
	 * Default empty constructor.
	 */
	public JargonTreeCellRenderer( )
	{

	}

	/**
	 * Sets the value of the current tree cell to value. If selected is true,
	 * the cell will be drawn as if selected. If expanded is true the node is
	 * currently expanded and if leaf is true the node represets a leaf and if
	 * hasFocus is true the node currently has focus. tree is the JTree the
	 * receiver is being configured for. Returns the Component that the renderer
	 * uses to draw the value.
	 */
	public Component getTreeCellRendererComponent(
											JTree tree,
											Object value,
											boolean sel,
											boolean expanded,
											boolean leaf,
											int row,
											boolean hasFocus)
	{
		//For multiroot trees, first level just a string
		if (value instanceof String) {
			if (value.equals(JargonTreeModel.TOP_LEVEL)) {
				return new JLabel( value.toString(), JargonTreeModel.FOLDER_ICON, 0 );
			}
			else {
				return new JLabel( (String)value, JargonTreeModel.FOLDER_ICON, 0 );
//				throw new IllegalArgumentException( "Improper node on tree" );
			}
		}
		else if (value instanceof LocalFile) {
			//LocalFile doesn't have metadata
//TODO a metadata check would be better, more general
			if (leaf) {
				return new JLabel( ((LocalFile)value).getName(),
					JargonTreeModel.FILE_ICON, 0 );
			}
			else {
				return new JLabel( ((LocalFile)value).getName(),
					JargonTreeModel.FOLDER_ICON, 0 );
			}
		}

		//So, "value" must be an Object that can have metadata--------------
		JargonTreeModel model = (JargonTreeModel) tree.getModel();
		//Real tree part
		GeneralFile file = (GeneralFile)value;
//TODO temp? eventually add metadata to directories
if (leaf && model.showMetaData()) {

			JPanel panel = new JPanel();
			JLabel label = null;
			JTable table = null;

			Object tableData[][] = null;
			String tableNames[] = null;

			//didn't like the grey box and thick border, BoxLayout seems to work...
			panel.setBackground(new Color(0,0,0,0));
			panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));

			label = new JLabel( "", JargonTreeModel.FILE_ICON, 0 );
			label.setAlignmentX(Component.CENTER_ALIGNMENT);
			panel.add(label);

			table = model.getMetaData( file );
			if (table != null) {
				table.setAlignmentX(Component.LEFT_ALIGNMENT);
				panel.add(table);
			}

			return panel;
		}
		else if (leaf) {
			return new JLabel( file.getName(), JargonTreeModel.FILE_ICON, 0 );
		}
		else {
			return new JLabel( file.getName(), JargonTreeModel.FOLDER_ICON, 0 );
		}
	}
}

