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
//	ClausesPanel.java	-  edu.sdsc.grid.gui.ClausesPanel
//
//  CLASS HIERARCHY
//	javax.swing.JPanel
//	    |
//	    +-edu.sdsc.grid.gui.ClausesPanel
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.gui;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.AdjustmentEvent;
import java.awt.event.AdjustmentListener;
import java.util.Vector;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JScrollBar;
import javax.swing.SwingConstants;
import javax.swing.border.EmptyBorder;


//needed for the testing methods
import edu.sdsc.grid.io.*;
import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.local.*;
import java.awt.*;
import java.awt.event.*;
import java.net.*;
import javax.swing.*;


/**
 *
 *
 * @author Lucas Ammon Gilbert
 * @see SelectionChooser
 * @see QueryPanel
 * @since JARGON1.5
 */
/*public*/ class ClausesPanel
	extends JPanel implements ActionListener
{
//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------
	public static String ADD_BUTTON_TEXT = "+";
	public static String REMOVE_BUTTON_TEXT = "-";



//----------------------------------------------------------------------
//  Fields
//----------------------------------------------------------------------
	private Vector clauses;

	//label
	private JLabel queryLabel;
	private JButton addButton;
	private JButton removeButton;
	private JPanel labelPanel;

	//finishedComponents
	private JPanel finishedComponents;
	private JScrollPane pane;
	private JScrollBar horizontalBar;
	private JScrollBar verticalBar;

	//chooser display
	private SelectionChooser display;

	private GridBagConstraints gridBagConstraints;

	/**
	 * Start a new line every once in a while.
	 */
	private int itemsInRow = 0;

	private int widthOfRow = 3;

//----------------------------------------------------------------------
//  Constructors, Destructors and initialization Methods
//----------------------------------------------------------------------
	public ClausesPanel( SelectionChooser display, String label )
	{
		this.display = display;

		queryLabel = new JLabel(label);
		addButton = new JButton(ADD_BUTTON_TEXT);
		removeButton = new JButton(REMOVE_BUTTON_TEXT);

		init();
	}

	/**
	 * Finalizes the object by explicitly letting go of each of
	 * its internally held values.
	 */
	protected void finalize( )
		throws Throwable
	{
		if (clauses != null)
			clauses = null;

		if (queryLabel != null)
			queryLabel = null;
		if (addButton != null)
			addButton = null;
		if (removeButton != null)
			removeButton = null;


		if (finishedComponents != null)
			finishedComponents = null;


		if (display != null)
			display = null;

		super.finalize();
	}


	protected void init( )
	{
		clauses = new Vector();
		gridBagConstraints = new GridBagConstraints();
		//Width = to the end of the line.
		gridBagConstraints.gridwidth = GridBagConstraints.REMAINDER;
		gridBagConstraints.anchor = GridBagConstraints.WEST;


		setBackground(new Color(0,0,0,0));
		setLayout(new GridBagLayout());


		//the label: and finished clauses
		queryLabel.setHorizontalAlignment(SwingConstants.LEFT);
		queryLabel.setToolTipText("Search conditions of the query should be based on the following items");
		queryLabel.setVerticalAlignment(SwingConstants.TOP);

		display.setBorder(new EmptyBorder(1,1,1,1));

		//finishedComponents panel for the chosen query conditions/selects
		finishedComponents = new JPanel();
		finishedComponents.setBackground(Color.WHITE);
		finishedComponents.setBorder(new EmptyBorder(1,1,1,1));
		finishedComponents.setLayout(new GridBagLayout());
		finishedComponents.setMinimumSize(new Dimension(100, 25));

		removeButton.setVerticalAlignment(SwingConstants.TOP);
		removeButton.setMargin(new Insets(0, 0, 0, 0));
		removeButton.setMaximumSize(new Dimension(20, 20));
		removeButton.setMinimumSize(new Dimension(20, 20));
		removeButton.setPreferredSize(new Dimension(20, 20));
		removeButton.addActionListener(this);

		labelPanel = new JPanel();
		labelPanel.setBackground(Color.WHITE);
		labelPanel.setMinimumSize(new Dimension(100, 1));
		labelPanel.add(queryLabel);
		labelPanel.add(finishedComponents);
		labelPanel.add(removeButton, gridBagConstraints);
		add(labelPanel, gridBagConstraints);

		add(display);

		//add buttons
		addButton.setVerticalAlignment(SwingConstants.TOP);
		addButton.setMargin(new Insets(0, 0, 0, 0));
		addButton.setMaximumSize(new Dimension(20, 20));
		addButton.setMinimumSize(new Dimension(20, 20));
		addButton.setPreferredSize(new Dimension(20, 20));
		addButton.addActionListener(this);
		add(addButton, gridBagConstraints);


		//need to push the display lower.
		JPanel spacer = new JPanel();
		spacer.setMinimumSize(new Dimension(1, 24));
		spacer.setBorder(new EmptyBorder(0,0,0,0));
		spacer.setBackground(new Color(0,0,0,0));
		add(spacer, gridBagConstraints);
	}



//----------------------------------------------------------------------
//  Setters and Getters
//----------------------------------------------------------------------

	/*TODO public*/ Object[] getClauses( )
	{
		if (clauses != null) {
			return clauses.toArray();
		}

		return null;
	}

	/**
	 * Return all the clauses in the clause vector. Cast them to
	 * the same class as <code>type</code>.
	 */
	/*TODO public*/ Object[] getClauses( Object[] type )
	{
		if (clauses != null) {
			return clauses.toArray(type);
		}

		return null;
	}


//----------------------------------------------------------------------
//  Methods
//----------------------------------------------------------------------
	/**
	 * Add the clause from display to the clause vector.
	 */
	private void addClause( )
	{
		clauses.add( display.getClause() );

		if (itemsInRow%widthOfRow == 2) {
			finishedComponents.add( new JLabel( display.getClause().toString()+"," ),
				gridBagConstraints );
		}
		else {
			finishedComponents.add( new JLabel(	display.getClause().toString()+", "));
		}
		itemsInRow++;

/*
display.validate();
display.repaint();
finishedComponents.validate();
finishedComponents.repaint();
		validate();
		repaint();*/
component.validate();
component.repaint();
	}

	/**
	 * Adds these clauses to the clause vector.
	 */
	/*TODO public*/ void addClauses( Object[] addClauses )
	{
		for (int i=0;i<addClauses.length;i++) {
			if (addClauses[i] != null) {
				clauses.add( addClauses[i] );
				if (itemsInRow%widthOfRow == 2) {
					finishedComponents.add( new JLabel( addClauses[i].toString()+"," ),
						gridBagConstraints );
				}
				else {
					finishedComponents.add( new JLabel( addClauses[i].toString()+", " ) );
				}
				itemsInRow++;
			}
		}

display.validate();
display.repaint();
finishedComponents.validate();
finishedComponents.repaint();
		validate();
		repaint();
	}


	/**
	 * Remove the last clause from the clause vector and from the display.
	 */
	private void removeClause( )
	{
		int lastIndex = finishedComponents.getComponentCount()-1;

		//Don't try to delete when nothing exists
		if (lastIndex >= 0) {
			finishedComponents.remove( lastIndex );
			clauses.remove( clauses.size()-1 );

/*
display.validate();
display.repaint();
finishedComponents.validate();
finishedComponents.repaint();
		validate();
		repaint();*/
component.validate();
component.repaint();
		}
	}


	public void actionPerformed( ActionEvent e )
	{
		if (addButton.getText().equals(e.getActionCommand())) {
			addClause();
		}
		if (removeButton.getText().equals(e.getActionCommand())) {
			removeClause();
		}
	}

//TODO HACK
//TODO temp method until I figure out refresh better.
void setContainer( JComponent component ) {
this.component = component;
}


			JComponent component = null;

	/**
	 * Stand alone testing.
	 *
	public static void main( String[] args )
	{
		try {
			frame = new JFrame("JargonTree");

			ClausesPanel pane = new ClausesPanel( new SelectionChooser( MetaDataSet.getMetaDataGroups(true) ),
			"Select:" );
//			pane.setPreferredSize(new Dimension( 800, 600 ));

			frame.addWindowListener(new WindowAdapter() {
				public void windowClosing(WindowEvent we) {
					System.exit(0);
				}
			});
			frame.getContentPane().add(pane, BorderLayout.NORTH);
			frame.pack();
			frame.show();
			frame.validate();

		} catch (Throwable e) {
			e.printStackTrace();
			System.out.println(((SRBException)e).getStandardMessage());
		}
	}
*/
}

