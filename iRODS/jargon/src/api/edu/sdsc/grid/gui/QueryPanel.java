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
//	QueryPanel.java	-  edu.sdsc.grid.gui.QueryPanel
//
//  CLASS HIERARCHY
//	javax.swing.JPanel
//	    |
//	    +-edu.sdsc.grid.gui.QueryPanel
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.gui;

import javax.swing.JLabel;
import java.awt.event.MouseListener;

import java.io.IOException;


//needed for the testing methods
import edu.sdsc.grid.io.*;
import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.local.*;
import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.awt.event.*;
import java.util.*;


/**
 * A gui component for creating and submitting queries to a filesystem.
 * Displays two ClausesPanels, one for conditions and one for
 * selects.
 *
 * @author Lucas Ammon Gilbert
 * @see JargonGui
 * @see ClausesPanel
 * @since JARGON1.5
 */
/*TODO public */class QueryPanel extends JScrollPane implements ActionListener
{
//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------
	/**
	 * The text of the list button.
	 */
	public static String LIST_BUTTON_TEXT = "List";

	/**
	 * The image for the list button.
	 */
	/*public*/ static String LIST_BUTTON_IMG = "list.gif";

	/**
	 * The text of the graph button.
	 */
	public static String GRAPH_BUTTON_TEXT = "Graph";

	/**
	 * The image for the graph button.
	 */
	/*public*/ static String GRAPH_BUTTON_IMG = "graph.gif";

	/**
	 * The text of the graph button.
	 */
	public static String CLEAR_BUTTON_TEXT = "Clear";

	/**
	 * The image for the graph button.
	 */
	/*public*/ static String CLEAR_BUTTON_IMG = "clear.gif";

	/**
	 * The text of the submission button.
	 */
	public static String MORE_BUTTON_TEXT = "More Results";

	/**
	 * The image for the submission button.
	 */
	/*public*/ static String MORE_BUTTON_IMG = "More Results";


	/**
	 * The text if there is no query result to display.
	 */
	public static String NO_RESULT = "No Result";


//----------------------------------------------------------------------
//  Fields
//----------------------------------------------------------------------
	/**
	 * Queries will be addressed to this filesystem
	 */
	private GeneralFileSystem fileSystem;

	/**
	 * The list of MetaDataGroups, for selecting the sublist of MetaDataFields.
	 */
	private MetaDataGroup[] groups;

	/**
	 * The list of MetaDataFields, used to find query attributes.
	 */
	private MetaDataField[] fields;


	/**
	 * The panel for choosing the conditions in the query.
	 */
	private ClausesPanel conditionPanel;

	/**
	 * The panel for choosing the selects in the query.
	 */
	private ClausesPanel selectionPanel;

	/**
	 * Sends the query to the filesystem. And return the results as a JTable.
	 */
	private JButton listButton;


	/**
	 * Sends the query to the filesystem. And return the results as a JargonGraph.
	 */
	private JButton graphButton;


	/**
	 * Clears the old queries.
	 */
	private JButton clearButton;


	/**
	 * The query results.
	 */
	private MetaDataRecordList[] rl;




//For display
	/**
	 * Create a new window for the results.
	 */
	private JFrame frame;

	/**
	 * Just a JPanel so more results can be added without everything
	 * getting messed up.
	 */
	private JPanel panel;

	/**
	 * Layout used by <code>panel</code>.
	 */
	private GridBagConstraints gridBagConstraints;

	/**
	 * A scrollpane to hold the query results.
	 * The result lists can get pretty long.
	 */
	/*TODO private*/ JScrollPane jScrollPane;

	/**
	 * If more results to the query are available,
	 * This is the button that will show further results.
	 */
	private JButton moreButton;


	/**
	 * Even more overarching panels to contain other panels to keep the
	 * layout from getting messed up.
	 */
	private JPanel containingPanel;




//----------------------------------------------------------------------
//  Constructors, Destructors and initialization Methods
//----------------------------------------------------------------------
	/**
	 * Shows the condition and select panels with all the
	 * metadata groups and fields available.
	 *
	 * @param fileSystem Queries will be addressed to this filesystem
	 */
	public QueryPanel( GeneralFileSystem fileSystem )
	{
		this( fileSystem, MetaDataSet.getMetaDataGroups(true) );
	}


	/**
	 * Shows the condition and select panels with the MetaDataGroups available
	 * determined by the <code>groups</code> array.
	 *
	 * @param fileSystem Queries will be addressed to this filesystem
	 */
	public QueryPanel( GeneralFileSystem fileSystem, MetaDataGroup[] groups )
	{
		this.fileSystem = fileSystem;
		this.groups = groups;

		conditionPanel = new ClausesPanel( new ConditionChooser( groups ),
			"Where:" );
		selectionPanel = new ClausesPanel( new SelectionChooser( groups ),
			"Select:" );

		init();
	}


	/**
	 * Will construct the QueryPanel without seperating the fields by group.
	 * The groups list/JComponent will not be displayed.
	 *
	 * @param fileSystem Queries will be addressed to this filesystem
	 */
	public QueryPanel( GeneralFileSystem fileSystem, MetaDataField[] fields )
	{
		this.fileSystem = fileSystem;
		this.fields = fields;

		conditionPanel = new ClausesPanel( new ConditionChooser( fields ),
			"Where:" );
		selectionPanel = new ClausesPanel( new SelectionChooser( fields ),
			"Select:" );

		init();
	}

	/**
	 * Shows the condition and select panels with all the
	 * metadata groups and fields available.
	 *
	 * @param file This file will be a condition of the query and
	 *		queries will be addressed to the filesystem of this file.
	 */
	public QueryPanel( GeneralFile file )
	{
		this( file, MetaDataSet.getMetaDataGroups(true) );
	}

	public QueryPanel( GeneralFile file, MetaDataGroup[] groups )
	{
		this.fileSystem = file.getFileSystem();
		this.groups = groups;

		conditionPanel = new ClausesPanel( new ConditionChooser( groups ),
			"Where:" );
		selectionPanel = new ClausesPanel( new SelectionChooser( groups ),
			"Select:" );

		//include the conditions of the file in the query
		addConditions( file );

		init();
	}


	/**
	 * Will construct the QueryPanel without seperating the fields by group.
	 * The groups list/JComponent will not be displayed.
	 *
	 * @param file This file will be a initial condition of the query and
	 *		queries will be addressed to the filesystem of this file.
	 */
	public QueryPanel( GeneralFile file, MetaDataField[] fields )
	{
		this.fileSystem = file.getFileSystem();
		this.fields = fields;

		conditionPanel = new ClausesPanel( new ConditionChooser( fields ),
			"Where:" );
		selectionPanel = new ClausesPanel( new SelectionChooser( fields ),
			"Select:" );

		//include the conditions of the file in the query
		addConditions( file );

		init();
	}


	/**
	 * I think I like init methods.
	 */
	private void init( )
	{
		containingPanel = new JPanel();
		containingPanel.setLayout(new GridBagLayout());
		containingPanel.setBackground(Color.WHITE);

		gridBagConstraints = new GridBagConstraints();
		//Width = to the end of the line.
		gridBagConstraints.gridwidth = GridBagConstraints.REMAINDER;
		gridBagConstraints.anchor = GridBagConstraints.WEST;
//		setLayout(new GridBagLayout());

		setPreferredSize( new Dimension( 640, 180 ) );
		setMinimumSize( new Dimension( 640, 180 ) );
		setBackground(Color.WHITE);

		//Where ClausesPanel
		containingPanel.add(conditionPanel, gridBagConstraints );

		//Select ClausesPanel
		containingPanel.add(selectionPanel, gridBagConstraints );


		gridBagConstraints.insets = new Insets(0, 30, 0, 0);
		gridBagConstraints.gridwidth = 1;
		listButton = new JButton( new ImageIcon(LIST_BUTTON_IMG) );
		listButton.setActionCommand( LIST_BUTTON_TEXT );
		listButton.setToolTipText( LIST_BUTTON_TEXT );
		listButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
		setListButton( listButton );
		containingPanel.add( listButton, gridBagConstraints );

		gridBagConstraints.gridwidth = GridBagConstraints.RELATIVE;
		graphButton = new JButton( new ImageIcon(GRAPH_BUTTON_IMG) );
		graphButton.setActionCommand( GRAPH_BUTTON_TEXT );
		graphButton.setToolTipText( GRAPH_BUTTON_TEXT );
		graphButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
		setGraphButton( graphButton );
		containingPanel.add( graphButton, gridBagConstraints );

		gridBagConstraints.gridwidth = GridBagConstraints.REMAINDER;
		clearButton = new JButton( new ImageIcon(CLEAR_BUTTON_IMG) );
		clearButton.setActionCommand( CLEAR_BUTTON_TEXT );
		clearButton.setToolTipText( CLEAR_BUTTON_TEXT );
		clearButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
		setClearButton( clearButton );
		containingPanel.add( clearButton, gridBagConstraints );

		gridBagConstraints.insets = new Insets(0, 0, 0, 0);

		panel = new JPanel();
		panel.setLayout(new GridBagLayout());
		panel.setBackground(Color.WHITE);
		containingPanel.add(panel, gridBagConstraints);


setViewportView(containingPanel);

//TODO HACK
conditionPanel.setContainer( this );
selectionPanel.setContainer( this );
	}


//----------------------------------------------------------------------
//  Setters and Getters
//----------------------------------------------------------------------
	/**
	 * Sets the GeneralFileSystem used for queries by this panel.
	 * Allows use of the same query on other filesystems.
	 */
	public void setFileSystem( GeneralFileSystem fileSystem )
	{
		if (fileSystem != null)
			this.fileSystem = fileSystem;
	}


	/*TODO? public*/ void setListButton( JButton listButton )
	{
		this.listButton = listButton;
		this.listButton.addActionListener(this);
	}

	/*TODO? public*/ void setGraphButton( JButton graphButton )
	{
		this.graphButton = graphButton;
		this.graphButton.addActionListener(this);
	}

	/*TODO? public*/ void setClearButton( JButton clearButton )
	{
		this.clearButton = clearButton;
		this.clearButton.addActionListener(this);
	}

	/**
	 * Returns the GeneralFileSystem used for queries by this panel.
	 */
	public GeneralFileSystem getFileSystem( )
	{
		return fileSystem;
	}


	/**
	 * Returns the MetaDataConditions that will currently be used in a query.
	 */
	public MetaDataCondition[] getConditions()
	{
		return (MetaDataCondition[]) conditionPanel.getClauses(
			new MetaDataCondition[0]);
	}

	/**
	 * Returns the MetaDataSelects that will currently be used in a query.
	 */
	public MetaDataSelect[] getSelects()
	{
		return (MetaDataSelect[]) selectionPanel.getClauses(
			new MetaDataSelect[0]);
	}



//----------------------------------------------------------------------
//	Methods
//----------------------------------------------------------------------
	/**
	 * Adds a MetaDataCondition to the current query.
	 */
	public void addCondition( MetaDataCondition condition )
	{
		MetaDataCondition[] conditions = { condition };
		addConditions( conditions );
	}


	/**
	 * Adds these MetaDataConditions to the current query.
	 */
	public void addConditions( MetaDataCondition[] conditions )
	{
		conditionPanel.addClauses( conditions );
	}


	/**
	 * Used internally to include the conditions of the file in the query.
	 *
	 * @param file need the path for the query, but can just give the path
	 * 		because dirs and files require different queries.
	 */
	/*TODO public?*/ void addConditions( GeneralFile file )
	{
		//code block taken from SRBFile.query
		MetaDataCondition iConditions[] = null;
		String fieldName = null;
		int operator = MetaDataCondition.EQUAL;
		String value = null;

		if (file.isDirectory()) {
			iConditions = new MetaDataCondition[1];
			fieldName = SRBMetaDataSet.DIRECTORY_NAME;
			operator = MetaDataCondition.EQUAL;
			value = file.getAbsolutePath();
			iConditions[0] =
				MetaDataSet.newCondition( fieldName, operator, value );
		}
		else {
			iConditions = new MetaDataCondition[3];
			fieldName = SRBMetaDataSet.DIRECTORY_NAME;
			operator = MetaDataCondition.EQUAL;
			value = file.getParent();
			iConditions[0] =
				MetaDataSet.newCondition( fieldName, operator, value );

			fieldName = SRBMetaDataSet.FILE_NAME;
			value = file.getName();
			iConditions[1] =
				MetaDataSet.newCondition( fieldName, operator, value );

			if (file instanceof SRBFile) {
				int replicaNumber = ((SRBFile) file).getReplicaNumber();
				if (replicaNumber >= 0) {
					fieldName = SRBMetaDataSet.FILE_REPLICATION_ENUM;
					value = "" + replicaNumber;
					iConditions[2] =
						MetaDataSet.newCondition( fieldName, operator, value );
				}
			}
			//else last condition = null, will be ignored
		}

		conditionPanel.addClauses( iConditions );
	}



	/**
	 * Adds a MetaDataSelect to the current query.
	 */
	public void addSelect( MetaDataSelect select )
	{
		MetaDataSelect[] selects = { select };
		addSelects( selects );
	}


	/**
	 * Adds these MetaDataSelects to the current query.
	 */
	public void addSelects( MetaDataSelect[] selects )
	{
		selectionPanel.addClauses( selects );
	}


	/**
	 * Submits the query to the <code>fileSystem</code>. Querying a
	 * <code>LocalFileSystem</code> is possible, but unproductive.
	 *
	 * @see MetaDataRecordList
	 * @see GeneralFileSystem.query
	 */
	public MetaDataRecordList[] submitQuery( )
		throws IOException, NullPointerException
	{
		if (fileSystem != null) {
			rl = fileSystem.query( getConditions(), getSelects() );
			return rl;
		}

		throw new NullPointerException( "FileSystem cannot be null" );
	}


	/**
	 * Displays the results of the query in a new <code>MetaDataDisplay</code>
	 * Be sure to submit the query before you try to display it.
	 *
	 * @see displayRecordList( boolean )
	 */
	/*TODO? public*/ void displayList( )
	{
		if (rl == null) {
			panel.add( new JLabel(NO_RESULT), gridBagConstraints);
			containingPanel.validate();
			containingPanel.repaint();
			return;
		}
		else {
			MetaDataDisplay display = new MetaDataDisplay(fileSystem, rl);
			Dimension d = display.getPreferredSize();

panel.add(display.getTableHeader(), gridBagConstraints);
			panel.add(display, gridBagConstraints);

			//if more results in query add the moreButton
			if (!rl[rl.length-1].isQueryComplete()) {
				moreButton = new JButton(MORE_BUTTON_TEXT);
				d = new Dimension( 40, 20 );
				moreButton.setPreferredSize(d);
				moreButton.setMaximumSize(d);
				moreButton.setMinimumSize(d);
				moreButton.addActionListener(this);
				containingPanel.add(moreButton, gridBagConstraints);
			}

			containingPanel.validate();
			containingPanel.repaint();
		}
	}

	/**
	 * Displays the results of the query in a new <code>MetaDataDisplay</code>
	 * Be sure to submit the query before you try to display it.
	 *
	 * @see displayList( boolean )
	 */
	/*TODO? public*/ JPanel getResultsPanel( )
	{
		MetaDataDisplay display = new MetaDataDisplay(fileSystem, rl);
		Dimension d = display.getPreferredSize();

		JPanel container = new JPanel();
		if (rl == null) {
			JLabel label = new JLabel(NO_RESULT);
			container.add( label );
			return container;
		}

		jScrollPane = new JScrollPane(display);

		d.setSize( d.width+MetaDataDisplay.TEXT_PADDING,
			d.height+MetaDataDisplay.fontPixelHeight+MetaDataDisplay.TEXT_PADDING*10 );
		jScrollPane.setPreferredSize(d);
//		container.setPreferredSize(d);

		//for adding a new recordList, make sure each is on its own line.
//		gridBagConstraints = new GridBagConstraints();
		//Width = to the end of the line.
//		gridBagConstraints.gridwidth = GridBagConstraints.REMAINDER;
		container.setLayout(new GridBagLayout());
		container.add(jScrollPane, gridBagConstraints);

		//if more results in query add the moreButton
 		if ((rl != null) && (!rl[rl.length-1].isQueryComplete())) {
 			moreButton = new JButton(MORE_BUTTON_TEXT);
			d = new Dimension( 40, 20 );
			moreButton.setPreferredSize(d);
			moreButton.setMaximumSize(d);
			moreButton.setMinimumSize(d);
			moreButton.addActionListener(this);
			container.add(moreButton, gridBagConstraints);
		}

		return container;
	}


	/**
	 * Displays all the results of the query in a new <code>MetaDataDisplay</code>
	 * Be sure to submit the query before you try to display it.
	 *<P>
	 * Be advised, queries that have a very large result set, displaying all the
	 * results could take a long time depending on connection speed.
	 */
	/*public*/ void displayList( boolean allResults )
		throws IOException
	{
		if (allResults)
			rl = MetaDataRecordList.getAllResults( rl );

		displayList();
	}


	/**
	 * Displays the results of the query in a new <code>JargonGraph</code>
	 * Be sure to submit the query before you try to display it.
	 *
	 * @see displayRecordList( boolean )
	 */
	/*TODO? public*/ void displayGraph( )
	{
//TODO
	}

//----------------------------------------------------------------------
//	Listeners and related
//----------------------------------------------------------------------
	/**
	 * Invoked when an action occurs.
	 */
	public void actionPerformed(ActionEvent e)
	{
		if (listButton.getActionCommand().equals(e.getActionCommand())) {
			try {
				submitQuery();
				displayList();
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}
		}
		else if (graphButton.getActionCommand().equals(e.getActionCommand())) {
			try {
				submitQuery();
				displayGraph();
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}
		}
		else if (clearButton.getActionCommand().equals(e.getActionCommand())) {
			try {
				panel.removeAll();
				panel.validate();
				panel.repaint();
if (false)
throw new IOException();
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}
		}
		else if (moreButton.getText().equals(e.getActionCommand())) {
			try {
				if (rl == null) return;

				//add the next set of results
				MetaDataRecordList[] rl2 = rl[rl.length-1].getMoreResults();

				if (rl2 == null) return;

				MetaDataRecordList[] totalRecordList =
					new MetaDataRecordList[rl.length+rl2.length];
				System.arraycopy(	rl, 0, totalRecordList, 0, rl.length );
				System.arraycopy(	rl2, 0, totalRecordList, rl.length, rl2.length );

				rl = totalRecordList;
/*
				panel
				panel.add(new MetaDataDisplay(rl), gridBagConstraints);
				panel.validate();
*/
//				jScrollPane.setViewportView(new MetaDataDisplay(rl));
				containingPanel.add(new MetaDataDisplay(fileSystem, rl), gridBagConstraints);

				if (rl[rl.length-1].isQueryComplete()) {
					//remove the moreButton
//					frame.getContentPane().remove(
//						frame.getContentPane().getComponentCount()-1);
					containingPanel.remove(
						containingPanel.getComponentCount()-1);
					moreButton.validate();
//					frame.validate();
				}
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}
		}
	}
}
