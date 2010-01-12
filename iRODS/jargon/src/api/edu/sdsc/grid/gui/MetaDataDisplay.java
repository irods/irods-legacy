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
//	MetaDataDisplay.java	-  edu.sdsc.grid.gui.MetaDataDisplay
//
//  CLASS HIERARCHY
//	javax.swing.JPanel
//	    |
//	    +-edu.sdsc.grid.gui.MetaDataDisplay
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.gui;

import edu.sdsc.grid.io.*;
import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.local.*;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.AdjustmentEvent;
import java.awt.event.AdjustmentListener;

import java.io.IOException;
import java.util.EventObject;

import javax.swing.AbstractCellEditor;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.event.CellEditorListener;
import javax.swing.event.ChangeEvent;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;
import javax.swing.table.TableColumn;
import javax.swing.table.TableModel;

/**
 * A gui for the <code>MetaDataRecordList</code> class.
 * 
 * @author Lucas Ammon Gilbert
 * @see JargonTreeCellEditor
 * @see MetaDataTableDisplay
 * @since JARGON1.5
 * @deprecated
 */
@Deprecated
public class MetaDataDisplay extends JTable {
	// -----------------------------------------------------------------------
	// Constants
	// -----------------------------------------------------------------------
	/**
	 * The cells of tables were too small for the text.
	 */
	static final int TEXT_PADDING = 4;

	// -----------------------------------------------------------------------
	// Fields
	// -----------------------------------------------------------------------
	/**
	 * The filesystem being queried.
	 */
	GeneralFileSystem fileSystem;
	// just going to save this file(filesystem?) in case we need it later
	// to do more queries
	private GeneralFile[] queryObj;

	/**
	 * The query results to be displayed.
	 */
	private MetaDataRecordList[] rl;

	// TODO maybe have?
	/**
	 * Holds MetaDataTable to be displayed.
	 */
	private MetaDataTable table;

	// TODO
	// need better name?
	boolean keyValue = true;

	void setKeyValue(boolean keyValuePair) {
		keyValue = keyValuePair;
	}

	boolean getKeyValue() {
		return keyValue;
	}

	// TODO
	static int fontPixelHeight = -1;
	static Font font;
	static FontMetrics fontMetrics;

	// -----------------------------------------------------------------------
	// Constructors, Destructors & init
	// -----------------------------------------------------------------------
	/**
	 * Displays the <code>recordList</code> as the row data for JTable. The
	 * MetaDataField names are used as the column names.
	 */
	public MetaDataDisplay(MetaDataRecordList recordList) {
		rl = new MetaDataRecordList[1];
		rl[0] = recordList;

		initComponents();
	}

	/**
	 * Displays these <code>recordList</code>s as the row data for JTable. The
	 * MetaDataField names are used as the column names.
	 */
	public MetaDataDisplay(MetaDataRecordList recordList[]) {
		rl = recordList;

		initComponents();
	}

	/**
	 * Displays the <code>recordList</code> as the row data for JTable. The
	 * MetaDataField names are used as the column names.<br>
	 * By including a <code>file</code> parameter the displayed
	 * <code>recordList</code> can modify the metadata of that file.
	 * 
	 * @param file
	 *            Changing the displayed <code>recordList</code> will change
	 *            this file's metadata values.
	 */
	public MetaDataDisplay(GeneralFile file, MetaDataRecordList recordList) {
		queryObj = new GeneralFile[1];
		queryObj[0] = file;
		rl = new MetaDataRecordList[1];
		rl[0] = recordList;

		initComponents();
	}

	/**
	 * Displays these <code>recordList</code>s as the row data for JTable. The
	 * the MetaDataField names are used as the column names.
	 */
	public MetaDataDisplay(GeneralFileSystem fileSystem,
			MetaDataRecordList recordList[]) {
		this.fileSystem = fileSystem;
		rl = recordList;
		queryObj = new GeneralFile[rl.length];

		initComponents();
	}

	/**
	 * Displays the <code>table</code> as the row data for JTable.
	 */
	MetaDataDisplay(MetaDataTable table) {
		this.table = table;

		initComponents();
	}

	/**
	 * This method is called from within the constructor to initialize the
	 * display.
	 */
	private void initComponents() {
		// if constructed with full query or query results.
		// display results table
		if (rl != null) {
			String fileName = null, directory = null;
			Object temp = null;
			createResultsTable(rl);

			// extract query objects
			if (fileSystem != null) {
				for (int i = 0; i < rl.length; i++) {
					temp = rl[i].getValue(SRBMetaDataSet.FILE_NAME);
					if (temp != null) {
						fileName = temp.toString();

						temp = rl[i].getValue(SRBMetaDataSet.DIRECTORY_NAME);
						if (temp != null) {
							directory = temp.toString();

							queryObj[i] = FileFactory.newFile(fileSystem,
									directory, fileName);
						}
					}
				}
			}
		} else if (table != null) {
			createMetaDataTable(table);
		}
	}

	// -----------------------------------------------------------------------
	// Methods
	// -----------------------------------------------------------------------
	/**
	 * Returns the recordList array displayed by this object.
	 */
	public MetaDataRecordList[] getRecordLists() {
		return rl;
	}

	/**
	 * This method sets good column sizes for the table. But it doesn't really
	 * try that hard. It only compares the header and TEST_ROWS rows.
	 * 
	 * @return total column widths
	 */
	static int adjustColumnSizes(JTable table) {
		TableModel model = table.getModel();

		TableColumn column = null;
		int columnCount = table.getColumnCount();

		Component comp = null;

		Object value;
		int headerWidth = 0;
		int cellWidth = 0;
		int totalWidth = TEXT_PADDING;

		TableCellRenderer headerRenderer = table.getTableHeader()
				.getDefaultRenderer();

		for (int i = 0; i < columnCount; i++) {
			column = table.getColumnModel().getColumn(i);

			comp = headerRenderer.getTableCellRendererComponent(null, column
					.getHeaderValue(), false, false, 0, 0);

			if (fontMetrics == null) {
				font = comp.getFont();

				if (font == null) {
					font = new Font("SansSerif", Font.PLAIN, 12);
					comp.setFont(font);
				}
				fontMetrics = comp.getFontMetrics(font);
			}

			// header size + some padding for readability.
			headerWidth = comp.getPreferredSize().width + TEXT_PADDING;

			int oldWidth = 0;
			// was using a limited number, but doesn't seem to hurt to do them
			// all
			int testRows = table.getRowCount();
			for (int j = 0; j < testRows; j++) {
				value = table.getValueAt(j, i);
				oldWidth = cellWidth;

				if (value == null) {
					cellWidth = 1;
				} else if (value instanceof String) {
					cellWidth = getCellWidth((String) value.toString());
				} else if (value instanceof Integer) {
					cellWidth = getCellWidth(value.toString());
				} else if (value instanceof Long) {
					cellWidth = getCellWidth(value.toString());
				} else if (value instanceof Float) {
					cellWidth = getCellWidth(value.toString());
				} else if (value instanceof java.util.Date) {
					cellWidth = getCellWidth(value.toString());
				} else if (value instanceof JTable) {
					cellWidth = adjustColumnSizes((JTable) value);
				}

				cellWidth = Math.max(cellWidth, oldWidth);
			}

			// pick the bigger of the header or the widest cell
			cellWidth = Math.max(headerWidth, cellWidth);

			column.setPreferredWidth(cellWidth);

			totalWidth += cellWidth;

			cellWidth = 0;
		}

		return totalWidth;
	}

	/**
	 *
	 */
	private static int getCellWidth(String value) {
		int w = fontMetrics.stringWidth(value);

		if (fontPixelHeight <= 0) {
			fontPixelHeight = fontMetrics.getHeight();
		}
		return w + TEXT_PADDING;
	}

	/**
	 *
	 */
	private void createResultsTable(MetaDataRecordList[] rl) {
		if (rl == null)
			return;

		int definableRows = 0;
		int column = 0;
		int fieldsLength = rl[0].getFieldCount();
		int recordListLength = rl.length;
		String[] fields = new String[fieldsLength];
		Object[][] data = new Object[recordListLength][fieldsLength];

		int tempFieldsLength = 0;

		// Setup for the Object[][]
		for (int i = 0; i < fieldsLength; i++) {
			switch (rl[0].getFieldType(i)) {
			case MetaDataField.TABLE:
				if (keyValue) {
					// NOTE:*********************
					// assumes all results have the same number of def_metadata
					// also, only displays first two values of a row
					MetaDataTable subTable = rl[0].getTableValue(i);
					// have to turn rows into columns of key-values
					tempFieldsLength += fieldsLength + subTable.getRowCount();
					String[] tempFields = new String[tempFieldsLength - 1];
					System.arraycopy(fields, 0, tempFields, 0, fieldsLength);
					for (int j = fieldsLength; j < tempFieldsLength; j++) {
						tempFields[j - 1] = subTable.getStringValue(j
								- fieldsLength, 0);
					}
					fields = tempFields;
				} else {
					fields[i] = rl[0].getFieldName(i);
				}
				break;
			default:
				fields[i] = rl[0].getFieldName(i);
				break;
			}
			;
		}
		for (int i = 0; i < recordListLength; i++) {
			for (int j = 0; j < fieldsLength; j++) {
				if (rl[i].getFieldCount() > j) {
					switch (rl[i].getFieldType(j)) {
					case MetaDataField.TABLE:
						if (keyValue) {
							// NOTE:*********************
							// assumes all results have the same number of
							// def_metadata
							// also, only displays first two values of a row
							MetaDataTable subTable = rl[i].getTableValue(j);
							// have to turn rows into columns of key-values
							// tempFieldsLength += fieldsLength +
							// subTable.getRowCount();
							Object[] tempData = new Object[tempFieldsLength];
							/*
							 * System.out.println( "data.length "+data.length );
							 * System.out.println( "i "+i ); System.out.println(
							 * "data[i], 0, tempData, 0, fieldsLength"+ data[i]+
							 * "   "+tempData+ "   "+fieldsLength);
							 */
							System.arraycopy(data[i], 0, tempData, 0,
									fieldsLength);
							for (int k = fieldsLength; k < tempFieldsLength; k++) {
								tempData[k - 1] = subTable.getStringValue(k
										- fieldsLength, 1);
							}
							data[i] = tempData;
						} else {
							data[i][j] = createMetaDataTable(rl[i]
									.getTableValue(j));
							definableRows = definableRows
									+ rl[i].getTableValue(j).getRowCount();
						}
						break;
					case MetaDataField.STRING:
					default:
						data[i][j] = rl[i].getStringValue(j);
						break;
					}
				}
			}
		}

		// create the table of metadata
		TableModel model = new MetaDataTableModel(data, fields);
		setModel(model);

		// set up the renderer
		if (!keyValue)
			setDefaultRenderer(JTable.class, new MetaDataTableCellRenderer(
					getDefaultRenderer(JTable.class)));

		setDefaultRenderer(String.class, new MetaDataTableCellRenderer(
				getDefaultRenderer(String.class)));

		// has a queryObj so the metadata is modifiable.
		if (queryObj != null) {
			if (!keyValue) {
				// TODO actually this sort of worked for a while, but now it
				// does not.
				setDefaultEditor(JTable.class, new MetaDataTableCellEditor(
						model, getDefaultEditor(JTable.class)));
			}

			setDefaultEditor(String.class, new MetaDataTableCellEditor(model,
					getDefaultEditor(String.class)));
		}

		// don't double count the first row and header
		if (definableRows > 1) {
			definableRows -= 2;

			// creates proper column sizes and returns total table width
			int columnWidth = adjustColumnSizes(this);

			// TODO works in JargonGui but the height is too much in standalone.
			// total height = (rows + header + MetaDataTable_Rows ) * font
			// height
			int columnHeight = (data.length + 1 + definableRows)
					* fontPixelHeight;
			setPreferredScrollableViewportSize(new Dimension(columnWidth,
					columnHeight * 2));

			setRowHeight(columnHeight);
		} else {
			// creates proper column sizes and returns total table width
			int columnWidth = adjustColumnSizes(this);

			// total height = (rows + header + MetaDataTable_Rows ) * font
			// height
			int columnHeight = /* (data.length)* */fontPixelHeight;
			setPreferredScrollableViewportSize(new Dimension(columnWidth,
					columnHeight));

			setRowHeight(columnHeight);
		}
	}

	/**
	 *
	 */
	// TODO static?
	static JTable createMetaDataTable(MetaDataTable table) {
		// TODO this should probably be a seperate class, in case people
		// just want to display such a table
		int tableRows = table.getRowCount();
		int tableColumns = table.getColumnCount();
		Object[][] temp = new Object[tableRows][tableColumns];
		String value = null;
		Object[][] subData = null;
		int k = 0;
		int maxK = 0;
		for (int ii = 0; ii < tableRows; ii++) {
			for (int jj = 0; jj < tableColumns; jj++) {
				value = table.getStringValue(ii, jj);
				if ((value != null) && !value.equals("")) {
					temp[ii][k] = value;
					k++;
				} else {
					temp[ii][k] = "";
				}
			}
			maxK = Math.max(k, maxK);
			k = 0;
		}
		// System.out.println("maxK "+maxK);
		subData = new Object[tableRows][maxK];
		for (int ii = 0; ii < tableRows; ii++) {
			System.arraycopy(temp[ii], 0, subData[ii], 0, maxK);
		}

		JTable jTable = new JTable(subData, subData[0]);

		Font font;
		FontMetrics fontMetrics;
		font = jTable.getFont();

		if (font == null) {
			font = new Font("SansSerif", Font.PLAIN, 12);
			jTable.setFont(font);
		}
		fontMetrics = jTable.getFontMetrics(font);

		// creates proper column sizes and returns total table width
		int columnWidth = 0;

		// TODO
		// MetaDataDisplay.initColumnSizes(jTable);

		// total height = (rows + header + MetaDataTable_Rows ) * font height
		int columnHeight = (tableRows + 1) * fontMetrics.getHeight();
		jTable.setPreferredScrollableViewportSize(new Dimension(columnWidth,
				columnHeight * 2));

		jTable.getModel().addTableModelListener(new TableModelListener() {
			public void tableChanged(TableModelEvent e) {
				int row = e.getFirstRow();
				int column = e.getColumn();
				TableModel model = (TableModel) e.getSource();
				String columnName = model.getColumnName(column);
				Object data = model.getValueAt(row, column);

				// System.out.println("!!!!!!!!!!!  "+data);
			}
		});

		return jTable;
	}

	// -----------------------------------------------------------------------
	// Inner Classes
	// -----------------------------------------------------------------------
	class MetaDataTableCellEditor extends AbstractCellEditor implements
			TableCellEditor, ActionListener {
		private TableCellEditor defaultEditor;
		Object component;
		String originalValue;
		int currentRow = -1, currentColumn = -1;

		/**
		 * 
		 * @param table
		 *            To modify the outer class.
		 */
		public MetaDataTableCellEditor(final TableModel model,
				TableCellEditor editor) {
			super();

			defaultEditor = editor;

			addCellEditorListener(new CellEditorListener() {
				public void editingCanceled(ChangeEvent e) {
				}

				public void editingStopped(ChangeEvent e) {
					MetaDataRecordList[] tempRL = new MetaDataRecordList[1];

					try {

						if (getCellEditorValue() instanceof JTable) {
							JTable table = (JTable) getCellEditorValue();
							int rows = table.getRowCount();
							int columns = table.getColumnCount();

							String[][] temp = new String[rows][columns];
							int[] op = new int[rows];
							Object t = null;
							for (int i = 0; i < rows; i++) {
								for (int j = 0; j < columns; j++) {
									t = table.getValueAt(i, j);
									if (t != null) {
										temp[i][j] = t.toString();
									}
								}
								op[i] = MetaDataCondition.EQUAL;
							}
							MetaDataTable mdt = new MetaDataTable(op, temp);
							tempRL[0] = new SRBMetaDataRecordList(
									rl[currentRow].getField(currentColumn), mdt);

							if (queryObj[currentRow] != null) {
								queryObj[currentRow].modifyMetaData(tempRL[0]);
							}
						} else if (getCellEditorValue() instanceof JTextField) {
							JTextField textField = (JTextField) getCellEditorValue();

							String text = textField.getText();
							if (!text.equals(originalValue)) {
								Object value = rl[currentRow]
										.getValue(currentColumn);
								if (value instanceof MetaDataTable) {
									rl[currentRow].setValue(currentColumn,
											(MetaDataTable) value);
									tempRL[0] = new SRBMetaDataRecordList(
											rl[currentRow]
													.getField(currentColumn),
											(MetaDataTable) value);
								} else {
									rl[currentRow]
											.setValue(currentColumn, text);
									tempRL[0] = new SRBMetaDataRecordList(
											rl[currentRow]
													.getField(currentColumn),
											text);
								}
								// change the local displayed value to the
								// edited value
								model.setValueAt(value, currentRow,
										currentColumn);

								if (queryObj[currentRow] != null) {
									queryObj[currentRow]
											.modifyMetaData(tempRL[0]);
								}
							}
						} else if (getCellEditorValue() instanceof String) {
							System.out.println("getCellEditorValue- String "
									+ getCellEditorValue());
						}

					} catch (SRBException t) {
						t.printStackTrace();
					} catch (Throwable t) {
						t.printStackTrace();
					}
				}
			}); // end of add listener
		}

		/**
		 * Finalizes the object by explicitly letting go of each of its
		 * internally held values.
		 */
		protected void finalize() throws Throwable {
			super.finalize();

			if (defaultEditor != null)
				defaultEditor = null;

			if (component != null)
				component = null;
		}

		/**
		 * @return the component used for drawing the cell. This method is used
		 *         to configure the renderer appropriately before drawing.
		 * 
		 * @param table
		 *            the JTable that is asking the renderer to draw; can be
		 *            null
		 * @param value
		 *            the value of the cell to be rendered. It is up to the
		 *            specific renderer to interpret and draw the value. For
		 *            example, if value is the string "true", it could be
		 *            rendered as a string or it could be rendered as a check
		 *            box that is checked. null is a valid value
		 * @param isSelected
		 *            true if the cell is to be rendered with the selection
		 *            highlighted; otherwise false
		 * @param hasFocus
		 *            if true, render cell appropriately. For example, put a
		 *            special border on the cell, if the cell can be edited,
		 *            render in the color used to indicate editing
		 * @param row
		 *            the row index of the cell being drawn. When drawing the
		 *            header, the value of row is -1
		 * @param column
		 *            the column index of the cell being drawn
		 */
		public Component getTableCellEditorComponent(JTable table,
				Object value, boolean isSelected, int row, int column) {
			component = value;

			currentRow = row;
			currentColumn = column;

			if (value instanceof String) {
				component = new JTextField((String) value);
				originalValue = (String) value;
			} else {
				component = defaultEditor.getTableCellEditorComponent(table,
						value, isSelected, row, column);
			}

			return (Component) component;
		}

		public void actionPerformed(ActionEvent e) {

		}

		public Object getCellEditorValue() {
			return component;
		}

		// TODO what? why does it say I need to override these?
		public void addCellEditorListener(CellEditorListener l) {
			super.addCellEditorListener(l);
		}

		public void cancelCellEditing() {
			super.cancelCellEditing();
		}

		public boolean isCellEditable(EventObject anEvent) {
			return super.isCellEditable(anEvent);
		}

		public void removeCellEditorListener(CellEditorListener l) {
			super.removeCellEditorListener(l);
		}

		public boolean shouldSelectCell(EventObject anEvent) {
			return super.shouldSelectCell(anEvent);
		}

		public boolean stopCellEditing() {
			return super.stopCellEditing();
		}
	}

	// tabley
	class MetaDataTableCellRenderer implements TableCellRenderer {
		private TableCellRenderer __defaultRenderer;
		private TableCellEditor defaultEditor;

		public MetaDataTableCellRenderer(TableCellRenderer renderer) {
			__defaultRenderer = renderer;
		}

		/**
		 * @return the component used for drawing the cell. This method is used
		 *         to configure the renderer appropriately before drawing.
		 * 
		 * @param table
		 *            the JTable that is asking the renderer to draw; can be
		 *            null
		 * @param value
		 *            the value of the cell to be rendered. It is up to the
		 *            specific renderer to interpret and draw the value. For
		 *            example, if value is the string "true", it could be
		 *            rendered as a string or it could be rendered as a check
		 *            box that is checked. null is a valid value
		 * @param isSelected
		 *            true if the cell is to be rendered with the selection
		 *            highlighted; otherwise false
		 * @param hasFocus
		 *            if true, render cell appropriately. For example, put a
		 *            special border on the cell, if the cell can be edited,
		 *            render in the color used to indicate editing
		 * @param row
		 *            the row index of the cell being drawn. When drawing the
		 *            header, the value of row is -1
		 * @param column
		 *            the column index of the cell being drawn
		 */
		public Component getTableCellRendererComponent(JTable table,
				Object value, boolean isSelected, boolean hasFocus, int row,
				int column) {
			if (value instanceof JComponent) {
				return (JComponent) value;
			}
			return __defaultRenderer.getTableCellRendererComponent(table,
					value, isSelected, hasFocus, row, column);
		}
	}

	// MetaDataTableModel
	class MetaDataTableModel extends AbstractTableModel {
		/* private */Object[][] data;

		private String[] names;

		MetaDataTableModel(Object[][] data, String[] names) {
			this.data = data;
			this.names = names;
			/*
			 * addTableModelListener( new TableModelListener() { public void
			 * tableChanged( TableModelEvent e ) { int row = e.getFirstRow();
			 * int column = e.getColumn(); TableModel model =
			 * (TableModel)e.getSource(); String columnName =
			 * model.getColumnName(column); Object data = model.getValueAt(row,
			 * column);
			 * 
			 * 
			 * System.out.println("~~~~~~~~~tableChanged "+data); } });
			 */
		}

		public String getColumnName(int column) {
			return names[column];
		}

		public int getRowCount() {
			return data.length;
		}

		public int getColumnCount() {
			return names.length;
		}

		public Object getValueAt(int row, int column) {
			while (column >= data[row].length)
				column--;
			return data[row][column];
		}

		public boolean isCellEditable(int row, int column) {
			return true;
		}

		public Class getColumnClass(int column) {
			while (column >= data[0].length)
				column--;
			Object obj = getValueAt(0, column);
			if (obj != null)
				return obj.getClass();
			return String.class;
		}

		public void setValueAt(Object value, int row, int column) {
			if ((row < data.length) && (column < data[row].length)) {
				data[row][column] = value;
			}
		}
	}
	// end MetaDataTableModel
}
