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
//  FILE
//	MetaDataTableDisplay.java	-  edu.sdsc.grid.gui.MetaDataTableDisplay
//
//  CLASS HIERARCHY
//	javax.swing.JTable
//	    |
//	    +-edu.sdsc.grid.gui.MetaDataTableDisplay
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.gui;

import edu.sdsc.grid.io.*;
import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.local.*;

import java.io.*;
import java.util.*;//EventObject
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.event.*;
import javax.swing.border.*;

import java.awt.*;
import java.awt.event.*;


/**
 *
 * @author  iktome
 */
//TODO no, I want to create a table model
class MetaDataTableDisplay extends javax.swing.JTable
{
	private MetaDataTable table;

	private int tableRows;

	/** Creates new form MetaDataDisplay */
	MetaDataTableDisplay( MetaDataTable table )
	{
super();
		this.table = table;

		initComponents();
	}

	private void initComponents()
	{
		setLayout(new BoxLayout(this, BoxLayout.X_AXIS));

	}

	public int getRows( )
	{
		return tableRows;
	}


	public void editingStopped( ChangeEvent e )
	{
	 System.out.println("editingStopped "+e);
	 super.editingStopped(e);
	}

	/**
	 *
	 */
//TODO static
	static JTable createMetaDataTable( MetaDataTable table )
	{
//TODO this should probably be a seperate class, in case people
//just want to display such a table
		int tableRows = table.getRowCount();
		int tableColumns = table.getColumnCount();
		Object[][] temp = new Object[tableRows][tableColumns];
		String value = null;
		Object[][] subData = null;
		int k = 0;
		int maxK = 0;
		for (int ii=0;ii<tableRows;ii++) {
			for (int jj=0;jj<tableColumns;jj++) {
				value = table.getStringValue( ii, jj );
				if ((value != null) && !value.equals( "" )) {
					temp[ii][k] = value;
					k++;
				}
				else {
					temp[ii][k] = "";
				}
			}
			maxK = Math.max(k, maxK);
			k=0;
		}
//System.out.println("maxK "+maxK);
		subData = new Object[tableRows][maxK];
		for (int ii=0;ii<tableRows;ii++) {
			System.arraycopy( temp[ii], 0, subData[ii], 0, maxK );
		}
/*
//System.out.println("subData********** "+subData);
//System.out.println("subData.l********** "+subData.length);
//System.out.println("subData.0.l********** "+subData[0].length);
for (int ii=0;ii<subData.length;ii++) {
	for (int jj=0;jj<subData[0].length;jj++) {
//System.out.println(subData[ii][jj]);
//		if (subData[ii][jj] == null)
//			subData[ii][jj] = "";
	}
}
*/
JTable jTable = new JTable( subData, subData[0] );


Font font;
FontMetrics fontMetrics;
font = jTable.getFont();

if (font==null) {
	font = new Font("SansSerif", Font.PLAIN, 12);
	jTable.setFont(font);
}
fontMetrics = jTable.getFontMetrics(font);

//creates proper column sizes and returns total table width
int columnWidth = 0;

//TODO
//MetaDataDisplay.initColumnSizes(jTable);


//total height = (rows + header + MetaDataTable_Rows ) * font height
int columnHeight = (tableRows+1)* fontMetrics.getHeight();
		jTable.setPreferredScrollableViewportSize(
			new Dimension( columnWidth,	columnHeight*2 ));



jTable.getModel().addTableModelListener(new TableModelListener() {
	public void tableChanged(TableModelEvent e) {
			int row = e.getFirstRow();
			int column = e.getColumn();
			TableModel model = (TableModel)e.getSource();
			String columnName = model.getColumnName(column);
			Object data = model.getValueAt(row, column);


//System.out.println("!!!!!!!!!!!  "+data);
	}
});

return jTable;
	}
}
