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
//	JargonTreeCellEditor.java	-  edu.sdsc.grid.gui.JargonTreeCellEditor
//
//  CLASS HIERARCHY
//	javax.swing.tree.TreeCellEditor
//	    |
//	    +-edu.sdsc.grid.gui.JargonTreeCellEditor
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.gui;

import java.util.EventObject;
import java.awt.Component;
import java.awt.Dimension;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JScrollPane;
import javax.swing.JTree;
import javax.swing.event.CellEditorListener;
import javax.swing.event.EventListenerList;
import javax.swing.tree.TreeCellEditor;


import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.GeneralMetaData;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.local.LocalFile;
//TODO remove these, make filefactory
import edu.sdsc.grid.io.srb.SRBFile;
import edu.sdsc.grid.io.srb.SRBMetaDataRecordList;
import edu.sdsc.grid.io.srb.SRBMetaDataSet;



/**
 * Creates an editor for a tree node. <p>
 * A custom tree cell editor to compliment the <code>JargonTreeModel</cope>
 * The GeneralFile classes which form the nodes of the tree will be displayed,
 * either as an inline <code>MetaDataDisplay</code>, or a JLabel for filesystems
 * that do not support metadata.
 *
 * @author Lucas Ammon Gilbert
 * @see JargonTreeModel
 * @see JargonTreeCellRenderer
 * @since JARGON1.5
 */
public class JargonTreeCellEditor implements TreeCellEditor
{
	/**
	 * Stores the event listeners.
	 */
	private EventListenerList listenerList = new EventListenerList();

	/**
	 * The value currently being edited.
	 */
	private JComponent currentComponent;


	/**
	 * Default constructor, does nothing.
	 */
	public JargonTreeCellEditor()
	{

	}


	/**
	 * Finalizes the object by explicitly letting go of each of
	 * its internally held values.
	 */
	protected void finalize( )
		throws Throwable
	{
		super.finalize();

		if ( listenerList != null)
			listenerList = null;

		if ( currentComponent != null)
			currentComponent = null;
	}

	/**
	 * Does nothing.
	 */
	public void cancelCellEditing()	{	}

	/**
	 * Returns the value currently being edited.
	 * @return the value currently being edited.
	 */
	public Object getCellEditorValue(){ return currentComponent; }

	/**
	 * Returns true.
	 * @param e an event object.
	 * @return true
	 */
	public boolean isCellEditable(EventObject e){ return true; }

	/**
	 * Returns true.
	 * @param anEvent an event object.
	 * @return true
	 */
	public boolean shouldSelectCell(EventObject anEvent){ return true; }

	/**
	 * Returns true.
	 * @return true
	 */
	public boolean stopCellEditing( ){ return true; }

	/**
	 * Adds a <code>CellEditorListener</code> to the listener list.
	 * @param l the new listener to be added.
	 */
	public void addCellEditorListener(CellEditorListener l) {
		listenerList.add(CellEditorListener.class, l);
	}

	/**
	 * Removes a <code>CellEditorListener</code> from the listener list.
	 * @param l the listener to be removed.
	 */
	public void removeCellEditorListener(CellEditorListener l) {
		listenerList.remove(CellEditorListener.class, l);
	}

	/**
	 * Configures the editor. Creates a JLabel, unless metadata is available,
	 * then uses the <code>MetaDataDisplay</code> class.
	 */
	public Component getTreeCellEditorComponent( JTree tree, Object value,
		boolean isSelected, boolean expanded, boolean leaf,	int row )
	{
		//String and LocalFile don't get editing.
//TODO a metadata check might be better, more general
		if (value instanceof String) {
			if (value.equals(JargonTreeModel.TOP_LEVEL)) {
				currentComponent = new JLabel( (String)value, JargonTreeModel.FOLDER_ICON, 0 );
			}
			else {
				if (leaf) {
					currentComponent = new JLabel( (String)value, JargonTreeModel.FILE_ICON, 0 );
				}
				else {
					currentComponent = new JLabel( (String)value, JargonTreeModel.FOLDER_ICON, 0 );
//				throw new IllegalArgumentException( "Improper node on tree" );
				}
			}
		}
		else if (value instanceof LocalFile) {
			if (leaf) {
				currentComponent = new JLabel( ((LocalFile)value).getName(),
					JargonTreeModel.FILE_ICON, 0 );
			}
			else {
				currentComponent = new JLabel( ((LocalFile)value).getName(),
					JargonTreeModel.FOLDER_ICON, 0 );
			}
		}
		else {//if GeneralFile
//TODO this really isn't the best way.
			JargonTreeModel model = (JargonTreeModel) tree.getModel();
			GeneralFile file = (GeneralFile) value;
			MetaDataRecordList rl[] =  { model.getMetaDataRecordList(file) };

			//create a fake MetaDataRecordList
			if (rl[0] == null) {
				if (leaf) {
//TODO need metadatarecordlist factory
					if (file instanceof SRBFile) {
						rl[0] = new SRBMetaDataRecordList(
							SRBMetaDataSet.getField(GeneralMetaData.FILE_NAME),
							file.getName() );
						rl[0].addRecord(
							SRBMetaDataSet.getField(GeneralMetaData.DIRECTORY_NAME),
							file.getParent() );
					}
				}
				else {
					if (file instanceof SRBFile) {
						rl[0] = new SRBMetaDataRecordList(
							SRBMetaDataSet.getField(GeneralMetaData.DIRECTORY_NAME),
							file.getAbsolutePath() );
					}
				}
			}

			currentComponent = new MetaDataDisplay(file, rl[0]);

			JScrollPane jScrollPane = new JScrollPane(currentComponent);

			Dimension d = currentComponent.getPreferredSize();
			d.setSize(d.width,
				d.height+MetaDataDisplay.fontPixelHeight+MetaDataDisplay.TEXT_PADDING);
			jScrollPane.setPreferredSize(d);

			return jScrollPane;
		}

		return currentComponent;
	}
}
