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
//	SelectionChooser.java	-  edu.sdsc.grid.gui.SelectionChooser
//
//  CLASS HIERARCHY
//	javax.swing.JPanel
//	    |
//	    +-edu.sdsc.grid.gui.SelectionChooser
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.gui;


import edu.sdsc.grid.io.MetaDataField;
import edu.sdsc.grid.io.MetaDataGroup;
import edu.sdsc.grid.io.MetaDataSet;

import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JComboBox;
import javax.swing.JPanel;



/**
 * A gui component for choosing metadata query components. As there are a large
 * number of metadata attributes registered, metadata attributes
 * have been grouped to making finding and choosing one easier. Choosing a
 * group from the <code>ComboBox</code> will display that group's fields
 * in a second <code>ComboBox</code>.
 *
 * @author Lucas Ammon Gilbert
 * @see QueryComponentPanel
 * @since JARGON1.5
 */
public class SelectionChooser extends JPanel
{
//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------



//----------------------------------------------------------------------
//  Fields
//----------------------------------------------------------------------
	/**
	 * The list of MetaDataGroups, for selecting the sublist of MetaDataFields.
	 */
	protected MetaDataGroup[] groups;

	/**
	 * The list of MetaDataFields, used to find query attributes.
	 */
	protected MetaDataField[] fields;

	/**
	 * Holds the groups, or if groups is null, then holds the fields.
	 * Clicking on a group will change the subList to the appropriate fields
	 * in that group.
	 */
	protected JComboBox mainList;

	/**
	 * If the mainList holds groups, this will hold the fields of the
	 * group selected.
	 */
	protected JComboBox subList;

	/**
	 * The part of a query that this panel represents, such as
	 * a MetaDataSelection or MetaDataCondition.
	 */
	protected Object clause;



//----------------------------------------------------------------------
//  Constructors, Destructors and init
//----------------------------------------------------------------------
	/**
	 * Default chooser, displays the selection chooser with all the
	 * metadata groups and fields available.
	 */
	public SelectionChooser( )
	{
		this( MetaDataSet.getMetaDataGroups(true) );
	}

	/**
	 * Displays the selection chooser with the MetaDataGroups available
	 * determined by the <code>groups</code> array.
	 */
	public SelectionChooser( MetaDataGroup[] groups )
	{
		this.groups = groups;

		mainList = new JComboBox(groups);
		add(mainList);

		changeSubList( 0 );

		ActionListener actionListener = new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				changeSubList( mainList.getSelectedIndex() );
			}
		};
		mainList.addActionListener(actionListener);

		init();
	}

	/**
	 * Constructs the SelectionChooser without seperating the fields by group.
	 * The groups <code>JComboBox</code> will not be displayed.
	 */
	public SelectionChooser( MetaDataField[] fields )
	{
		this.fields = fields;

		mainList = new JComboBox(fields);
		mainList.insertItemAt(makeObj( "---" ), 0);
		mainList.setSelectedIndex(0);
		add(mainList);

		init();
	}

	/**
	 * Finalizes the object by explicitly letting go of each of
	 * its internally held values.
	 */
	protected void finalize()
		throws Throwable
	{
		if (groups != null)
			groups = null;
		if (fields != null)
			fields = null;

		if (mainList != null)
			mainList = null;
		if (subList != null)
			subList = null;

		if (clause != null)
			clause = null;

		super.finalize();
	}

	/**
	 * init here!
	 */
	private void init()
	{
//		setMinimumSize(new java.awt.Dimension(600, 80));
//		setPreferredSize(new java.awt.Dimension(600, 80));
		setBackground(Color.WHITE);

		mainList.setBackground(Color.WHITE);
	}



//----------------------------------------------------------------------
//  Methods
//----------------------------------------------------------------------
	/**
	 * Sets the chooser clause, the internal MetaDataSelection.
	 */
	void setClause( Object clause )
	{
		if (clause != null) {
			this.clause = clause;

			//TODO set the lists to look right
		}
	}

	/**
	 * Returns the current clause of this chooser's MetaDataSelection
	 */
	public Object getClause()
	{
		String field = null;
		Object temp;

		if (subList != null) {
			temp = subList.getSelectedItem();
			if (temp == null)
				return null;

			field = temp.toString();
		}
		else {
			temp = mainList.getSelectedItem();
			if (temp == null)
				return null;

			field = temp.toString();
		}

		clause = MetaDataSet.newSelection( field );
		return clause;
	}

	/**
	 * Changes the displayed fields when a new group is chosen.
	 * Only used if the groups are shown.
	 */
	protected void changeSubList( int index )
	{
		if (subList != null) {
			remove(subList);
		}
		subList = new JComboBox(groups[index].getFields(true));
		subList.insertItemAt(makeObj( "---" ), 0);
		subList.setSelectedIndex(0);
		subList.setBackground(Color.WHITE);
		add(subList, 1);
		validate();
		repaint();
	}


	/**
	 * Part of the workaround to add a fake term at index 0 of the comboBoxes.
	 */
	Object makeObj( final String item )
	{
	 return new Object() { public String toString() { return item; } };
	}
}
