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
//	ConditionChooser.java	-  edu.sdsc.grid.gui.ConditionChooser
//
//  CLASS HIERARCHY
//	javax.swing.JPanel
//	    |
//	    +-edu.sdsc.grid.gui.ConditionChooser
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.gui;

import edu.sdsc.grid.io.*;


import java.awt.Color;
import javax.swing.JComboBox;
import javax.swing.JTextField;


/**
 * A gui component for choosing MetaDataConditions.
 * Extends the SelectionChooser by adding operator and value ComboBoxes.
 *
 * @author Lucas Ammon Gilbert
 * @see SelectionChooser
 * @see QueryComponentPanel
 * @since JARGON1.5
 */
public class ConditionChooser extends SelectionChooser
{
//----------------------------------------------------------------------
//  Fields
//----------------------------------------------------------------------
	/**
	 * Display the MetaDataCondition operators
	 */
	JComboBox operatorList;

	//consequent? no... what is the correct term for this?
	/**
	 * Display for MetaDataCondition value
	 */
	JTextField dependent;



//----------------------------------------------------------------------
//  Constructors, Destructors and initialization Methods
//----------------------------------------------------------------------
	/**
	 * Default chooser, displays the condition chooser with all the
	 * metadata groups and fields available.
	 */
	public ConditionChooser( )
	{
		super( );
	}

	/**
	 * Displays the condition chooser with the MetaDataGroups available
	 * determined by the <code>groups</code> array.
	 */
	public ConditionChooser( MetaDataGroup[] groups )
	{
		super( groups );

		init();
	}

	/**
	 * Constructs the ConditionChooser without seperating the fields by group.
	 * The groups <code>JComboBox</code> will not be displayed.
	 */
	public ConditionChooser( MetaDataField[] fields )
	{
		super( fields );

		init();
	}

	/**
	 * Finalizes the object by explicitly letting go of each of
	 * its internally held values.
	 */
	protected void finalize()
		throws Throwable
	{
		if (operatorList != null)
			operatorList = null;
		if (dependent != null)
			dependent = null;

		super.finalize();
	}



	/**
	 * init! Now is the time!
	 */
	private void init()
	{
//		setMinimumSize(new java.awt.Dimension(600, 80));
//		setPreferredSize(new java.awt.Dimension(600, 80));
		setBackground(Color.WHITE);

		mainList.setBackground(Color.WHITE);

		operatorList = new JComboBox( MetaDataCondition.getOperatorStrings() );
		operatorList.setBackground(Color.WHITE);
		add( operatorList );

		dependent = new JTextField(20);
		add( dependent );
	}



//----------------------------------------------------------------------
//  Methods
//----------------------------------------------------------------------
	/**
	 * Returns the current values of this chooser's MetaDataCondition
	 */
	public Object getClause()
	{
		String field = null;
		int operator = 0;
		Object temp;

		//metadata attribute
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

		//operator
//TODO ok technically this will only work if the numbers count up regularily.
		operator = operatorList.getSelectedIndex();

		clause = MetaDataSet.newCondition( field, operator, dependent.getText() );
		return clause;
	}


	/**
	 * Changes the displayed fields when a new group is chosen.
	 */
/*	protected void changeSubList( int index )
	{
		if (subList != null) {
			remove(subList);
		}

		subList = new JComboBox(groups[index].getFields(true));
		subList.insertItemAt(makeObj( "---" ), 0);
		subList.setSelectedIndex(0);
		subList.setBackground(Color.WHITE);

		//has two extra components
		add(subList, 1);

		validate();
		repaint();
	}
*/
}
