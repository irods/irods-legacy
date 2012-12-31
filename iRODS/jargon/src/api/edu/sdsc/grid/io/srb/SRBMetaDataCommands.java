//  Copyright (c) 2005, Regents of the University of California
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//    * Redistributions of source code must retain the above copyright notice,
//  this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
//    * Neither the name of the University of California, San Diego (UCSD) nor
//  the names of its contributors may be used to endorse or promote products
//  derived from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//  FILE
//  SRBMetaDataCommands.java  -  edu.sdsc.grid.io.srb.SRBMetaDataCommands
//
//  CLASS HIERARCHY
//  java.lang.Object
//     |
//     +-.SRBMetaDataCommands
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.srb;

import java.io.IOException;
import java.lang.reflect.Array;
import java.util.Vector;

import edu.sdsc.grid.io.Host;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataField;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.MetaDataTable;
import edu.sdsc.grid.io.ResourceMetaData;
import edu.sdsc.grid.io.StandardMetaData;
import edu.sdsc.grid.io.UserMetaData;

/**
 * SRBMetaDataCommands handles the SRB server call srbGetDataDirInfo and all
 * related methods. All the metadata methods were getting to complicated and
 * unwieldy in the SRBCommands class. So they were moved here to keep the code,
 * a tiny bit more, readable.
 * 
 * <P>
 * 
 * @author Lucas Gilbert, San Diego Supercomputer Center
 * @since JARGON1.2
 */
class SRBMetaDataCommands {

	/**
	 * Keep track of the connection.
	 */
	private SRBCommands commands;

	/**
	 * Constructor
	 * <P>
	 * 
	 * @param userInfoDirectory
	 *            the directory to find the user info
	 * @throws IOException
	 *             if the connection to the SRB fails.
	 */
	SRBMetaDataCommands(final SRBCommands commands) throws IOException {
		this.commands = commands;
	}

	/**
	 * Finalizes the object by explicitly letting go of each of its internally
	 * held values.
	 */
	@Override
	protected void finalize() {
		if (commands != null) {
			commands = null;
		}
	}

	// Various methods for handling metadata used by srbGetDataDirInfo(...).
	static String getOperator(final MetaDataCondition condition) {
		int operator = condition.getOperator();
		if (operator == MetaDataCondition.IN) {
			return " " + MetaDataCondition.getOperatorString(operator) + " (";
		} else {
			return " " + MetaDataCondition.getOperatorString(operator) + " ";
		}
	}

	static String getOperator(final int operator) {
		if (operator == MetaDataCondition.IN) {
			return " " + MetaDataCondition.getOperatorString(operator) + " (";
		} else {
			return " " + MetaDataCondition.getOperatorString(operator) + " ";
		}
	}

	static String getEndOperator(final MetaDataCondition condition) {
		if (condition.getOperator() == MetaDataCondition.IN) {
			return ")";
		} else {
			return "";
		}
	}

	static String getEndOperator(final int condition) {
		if (condition == MetaDataCondition.IN) {
			return ")";
		} else {
			return "";
		}
	}

	/**
	 * used to properly group the value with the operator, string values are in
	 * single quotes, ints are not...
	 */
	private String quotes(final int type) {
		switch (type) {
		case MetaDataField.INT:
		case MetaDataField.LONG:
		case MetaDataField.FLOAT:
			return "";
		case MetaDataField.STRING:
		case MetaDataField.DATE:
		default:
			return "'";
		}
	}

	private String fixLIKE(String value) {
		int index = value.indexOf("*");
		while (index >= 0) {
			value = value.substring(0, index) + "%"
					+ value.substring(index + 1);
			index = value.indexOf("*");
		}
		index = value.indexOf("?");
		while (index >= 0) {
			value = value.substring(0, index) + "_"
					+ value.substring(index + 1);
			index = value.indexOf("?");
		}

		// No wildcards used.
		// add % otherwise it would just be the EQUAL operator.
		if ((value.indexOf("%") < 0) && (value.indexOf("_") < 0)) {
			value = "%" + value + "%";
		}

		return value;
	}

	private MetaDataTable fixLIKE(final MetaDataTable table,
			final boolean hasLike) {
		int rowCount = table.getRowCount(), columnCount = table
				.getColumnCount();
		int index = -1;
		String value, temp;

		for (int i = 0; i < rowCount; i++) {
			for (int j = 0; j < columnCount; j++) {
				if ((j == 1)
						&& ((table.getOperator(i) == MetaDataCondition.LIKE) || (table
								.getOperator(i) == MetaDataCondition.NOT_LIKE))
						|| ((j == 0) && hasLike)) {
					value = table.getStringValue(i, j);
					temp = value;
					index = value.indexOf("*");
					while (index >= 0) {
						value = value.substring(0, index) + "%"
								+ value.substring(index + 1);
						index = value.indexOf("*");
					}
					index = value.indexOf("?");
					while (index >= 0) {
						value = value.substring(0, index) + "_"
								+ value.substring(index + 1);
						index = value.indexOf("?");
					}

					// No wildcards used.
					// add % otherwise it would just be the EQUAL operator.
					if ((value.indexOf("%") < 0) && (value.indexOf("_") < 0)) {
						value = "%" + value + "%";
					}

					if (!temp.equals(value)) {
						table.setStringValue(i, j, value);
					}
				}
			}
		}

		return table;
	}

	// for the qVal when the conditions contain a table
	// new query protocol
	private String setTableGenQuery(final MetaDataTable table,
			final MetaDataCondition condition) {
		if (table == null) {
			return "";
		}

		String fieldName = condition.getFieldName();
		String operator = getOperator(condition);

		// 1st row, condition already added
		String queryStatement = setTableGenQueryRow(0, table, condition);

		// 2nd row
		if (table.getRowCount() >= 2) {
			queryStatement += " " + operator + " "
					+ setTableGenQueryRow(1, table, condition);

			// 3rd row
			if (table.getRowCount() >= 3) {
				queryStatement += " " + operator + " "
						+ setTableGenQueryRow(2, table, condition);

				// 4th row
				if (table.getRowCount() >= 4) {
					queryStatement += " " + operator + " "
							+ setTableGenQueryRow(3, table, condition);

					// 5th row, users and resources don't have fifth row.
					if (fieldName
							.equals(SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES)
							|| fieldName
									.equals(SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES)) {
						if (table.getRowCount() >= 5) {
							queryStatement += " " + operator + " "
									+ setTableGenQueryRow(4, table, condition);
						}
					}
				}
			}
		}

		return queryStatement;
	}

	private String setTableGenQueryRow(final int row,
			final MetaDataTable table, final MetaDataCondition condition) {
		String queryStatement;
		int operator = table.getOperator(row);
		if (operator == MetaDataCondition.IN
				|| operator == MetaDataCondition.NOT_IN) {
			queryStatement = "'" + table.getStringValue(row, 0) + "'\u0000 "
					+ MetaDataCondition.getOperatorString(operator) + " (";
			String list = table.getStringValue(row, 1);
			String[] tokens = list.split(",");
			int length = tokens.length;
			for (int i = 0; i < length; i++) {
				queryStatement += "'" + tokens[i] + "'";
				if (i < length - 1) {
					queryStatement += ",";
				}
			}
			queryStatement += ")\u0000";
		} else {
			queryStatement = "'" + table.getStringValue(row, 0) + "'\u0000";
			queryStatement += getOperator(table.getOperator(row)) + "'"
					+ table.getStringValue(row, 1) + "'\u0000";
		}
		return queryStatement;
	}

	// To set the ID value early enough in the new query protocol
	private int[] setTableConditionIDs(final MetaDataTable table,
			final String fieldName) {
		// 1st row
		int[] ids = null;
		int size = table.getRowCount();
		if (size < 5) {
			ids = new int[size * 2];
		} else if (fieldName
				.equals(SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES)
				|| fieldName
						.equals(SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES)) {
			// can't be larger than 5, ask raja.
			ids = new int[10];
		}

		// 1st row
		// ids[0] = SRBMetaDataSet.getSRBID( fieldName+"0_0" );
		// ids[1] = SRBMetaDataSet.getSRBID( fieldName+"1_0" );

		// the new first row, because we need the first row for other
		ids[0] = SRBMetaDataSet.getSRBID(fieldName + "0_1");
		ids[1] = SRBMetaDataSet.getSRBID(fieldName + "1_1");

		// 2nd row
		if (ids.length >= 4) {
			ids[2] = SRBMetaDataSet.getSRBID(fieldName + "0_2");
			ids[3] = SRBMetaDataSet.getSRBID(fieldName + "1_2");

			// 3rd row
			if (ids.length >= 6) {
				ids[4] = SRBMetaDataSet.getSRBID(fieldName + "0_3");
				ids[5] = SRBMetaDataSet.getSRBID(fieldName + "1_3");

				// 4th row
				if (ids.length >= 8) {
					ids[6] = SRBMetaDataSet.getSRBID(fieldName + "0_4");
					ids[7] = SRBMetaDataSet.getSRBID(fieldName + "1_4");

					// 5th row
					if (ids.length >= 10) {
						ids[8] = SRBMetaDataSet.getSRBID(fieldName + "0_0");
						ids[9] = SRBMetaDataSet.getSRBID(fieldName + "1_0");
					}

				}
			}
		}

		return ids;
	}

	/**
	 * Removes null values from an array.
	 */
	static final Object[] cleanNulls(final Object[] obj) {
		Vector temp = new Vector(obj.length);
		boolean add = false;
		int i = 0;

		for (i = 0; i < obj.length; i++) {
			if (obj[i] != null) {
				temp.add(obj[i]);
				if (!add) {
					add = true;
				}
			}
		}
		if (!add) {
			return null;
		}

		// needs its own check
		if ((obj.length == 1) && (obj[0] == null)) {
			return null;
		}

		return temp.toArray((Object[]) Array.newInstance(
				temp.get(0).getClass(), 0));
	}

	/**
	 * Removes null and duplicate values from an array.
	 */
	static final Object[] cleanNullsAndDuplicates(final Object[] obj) {
		if (obj == null) {
			return null;
		}

		Vector temp = new Vector(obj.length);
		boolean anyAdd = false;
		int i = 0, j = 0;

		for (i = 0; i < obj.length; i++) {
			if (obj[i] != null) {
				// need to keep them in original order
				// keep the first, remove the rest.
				for (j = i + 1; j < obj.length; j++) {
					if (obj[i].equals(obj[j])) {
						obj[j] = null;
						j = obj.length;
					}
				}

				if (obj[i] != null) {
					temp.add(obj[i]);
					if (!anyAdd) {
						anyAdd = true;
					}
				}
			}
		}
		if (!anyAdd) {
			return null;
		}

		// needs its own check
		if ((obj.length == 1) && (obj[0] == null)) {
			return null;
		}

		return temp.toArray((Object[]) Array.newInstance(
				temp.get(0).getClass(), 0));
	}

	/**
	 * each query style has to be handled differently first the consequent
	 * (value) of the conditional is formed.
	 * 
	 * @param style
	 *            whether the condition is scalar, rangepair, enum, etc
	 * @param the
	 *            WHERE value to be converted
	 * @param need
	 *            to pass along in case the style is MetaDataCondition.TABLE
	 */
	private String addValue(final int style, final MetaDataCondition condition) {
		String qValCondition = "";
		qValCondition += getOperator(condition);
		switch (style) {
		case MetaDataCondition.SCALAR:
			switch (condition.getFieldType()) {
			case MetaDataField.STRING:
			case MetaDataField.DATE:
				qValCondition += "'" + condition.getStringValue() + "'";
				break;
			case MetaDataField.INT:
				qValCondition += condition.getIntValue();
				break;
			case MetaDataField.FLOAT:
				qValCondition += condition.getFloatValue();
				break;
			default:
				throw new IllegalArgumentException(
						"Invalid value type for the metadata attribute: "
								+ condition.getField());
			}
			break;
		// eg.
		case MetaDataCondition.RANGEPAIR:
			// The operator is always BETWEEN or NOT_BETWEEN
			switch (condition.getFieldType()) {
			case MetaDataField.STRING:
			case MetaDataField.DATE:
				qValCondition += "'" + condition.getStringValue(0) + "' and "
						+ "'" + condition.getStringValue(1) + "'";
				break;
			case MetaDataField.INT:
				qValCondition += condition.getIntValue(0) + " and "
						+ condition.getIntValue(1);
				break;
			case MetaDataField.FLOAT:
				qValCondition += condition.getFloatValue(0) + " and "
						+ condition.getFloatValue(1);
				break;
			default:
				throw new IllegalArgumentException(
						"Invalid value type for the metadata attribute: "
								+ condition.getField());
			}
			break;
		case MetaDataCondition.ENUM:
			// The operator is always IN or NOT_IN
			switch (condition.getFieldType()) {
			case MetaDataField.STRING:
			case MetaDataField.DATE:
				for (int i = 0; i < condition.getCount(); i++) {
					qValCondition += "'" + condition.getStringValue(i) + "',";
				}
				break;
			case MetaDataField.INT:
				for (int i = 0; i < condition.getCount(); i++) {
					qValCondition += condition.getIntValue(i) + ",";
				}
				break;
			case MetaDataField.FLOAT:
				for (int i = 0; i < condition.getCount(); i++) {
					qValCondition += condition.getFloatValue(i) + ",";
				}
				break;
			default:
				throw new IllegalArgumentException(
						"Invalid value type for the metadata attribute: "
								+ condition.getField());
			}
			// delete the last ','
			qValCondition = qValCondition.substring(0,
					qValCondition.length() - 1);
			break;
		case MetaDataCondition.TABLE:
			// random notes
			// SRB user defined metadata, API treats as one table.
			// SRB only handles user defined metadata as strings

			MetaDataTable table = condition.getTableValue();
			qValCondition += setTableGenQuery(table, condition);

			if (table.getRowCount() > 5) {

			}
			break;
		default:
			throw new RuntimeException(
					"temp SRBMetaDataCommands.getdatadir,style");
		}
		qValCondition += getEndOperator(condition);
		return qValCondition;
	}

	/**
	 * This is a more compact form of srbGetDataDirInfo. Instead of using fixed
	 * array of selval and qval, it uses the genQuery struct.
	 * 
	 * 
	 * @param catType
	 *            The catalog type, 0 = MDAS_CATALOG
	 * @param myMcatZone
	 *            The MCAT zone for this query. Only valid for version 3.x.x and
	 *            above. For version 2.x.x, this should be NULL.
	 * @param genQuery_t
	 *            *myGenQuery - The input query in the form of genQuery_t
	 *            instead of selval and qval of srbGetDataDirInfo.
	 * @param myresult
	 *            The output query result.
	 * @param rowsWanted
	 *            The number of rows of result wanted.
	 * 
	 * @return The query result, use srbGenGetMoreRows() to retrieve more rows.
	 */
	SRBMetaDataRecordList[] srbGenQuery(final int catType,
			final String myMcatZone, MetaDataCondition[] metaDataConditions,
			final MetaDataSelect[] metaDataSelects, int rowsWanted,
			boolean orderBy, final boolean nonDistinct) throws IOException {
		if (SRBCommands.DEBUG > 0) {
			System.err.println("\n srbGenQuery");
		}

		/*
		 * The conversion of the query in the SRB query protocol takes the
		 * following form: int ? = total length of the packed query? zero
		 * works... int selectsLength = a count of the selects to be sent int
		 * conditionsLength = a count of the conditions to be sent
		 * int[selectsLength] selectsIDs = an int ID# for every select
		 * int[conditionsLength] conditionIDs = an int ID# for every condition
		 * int[selectsLength] selectsValues = an int value for every select
		 * String[conditionsLength] conditionValues = a null-term String for
		 * every condition
		 * 
		 * for example: ? 0x0000 selectsLength 0x0001 conditionsLength 0x0002
		 * selectsIDs[0] 0x0012 selectsIDs[0] 0x001D selectsValues[0] 0x0001
		 * selectsValues[0] 0x0001 conditionIDs[0] 0x0002 conditionIDs[1] 0x000F
		 * conditionValue[0] = 'myJARGONMetaDataTestFile'\u0000
		 * conditionValue[1] = '/demozone/home/testuser.sdsc'\u0000
		 */

		int i = 0, j = 0, temp = 0;

		int fieldID = Integer.MIN_VALUE;

		SRBMetaDataRecordList[] recordList = null;
		SRBMetaDataRecordList[] metaDataNumList = null;
		int metaDataNumAt = 2;

		// temporary variable used for packing the conditions and selects
		// and bytes in the correct order in packedQuery[]
		String queryString = "";
		// the byte string sent to the SRB as the query
		byte[] packedQuery = new byte[2700];

		int selectsLength = metaDataSelects.length;

		// make duplicates, to leave original unmodified
		MetaDataCondition[] conditions = null;
		MetaDataSelect[] selects = new MetaDataSelect[selectsLength];

		int[] conditionIDs = null;
		new Vector();
		new Vector();

		int conditionsLength = 0;
		if (metaDataConditions != null) {
			// clean nulls
			metaDataConditions = (MetaDataCondition[]) cleanNullsAndDuplicates(metaDataConditions);
			if (metaDataConditions != null) {
				conditionsLength = metaDataConditions.length;
				for (i = 0; i < conditionsLength; i++) {
					if (metaDataConditions[i].getStyle() == MetaDataCondition.TABLE) {
						// make sure all * characters are changed to %
						// and all ? changed to _
						if ((metaDataConditions[i].getOperator() == MetaDataCondition.LIKE)
								|| (metaDataConditions[i].getOperator() == MetaDataCondition.NOT_LIKE)) {
							metaDataConditions[i] = MetaDataSet.newCondition(
									metaDataConditions[i].getFieldName(),
									metaDataConditions[i].getOperator(),
									fixLIKE(metaDataConditions[i]
											.getTableValue(), true));
						} else {
							// check for internal LIKE
							metaDataConditions[i] = MetaDataSet.newCondition(
									metaDataConditions[i].getFieldName(),
									metaDataConditions[i].getOperator(),
									fixLIKE(metaDataConditions[i]
											.getTableValue(), false));
						}
					} else if ((metaDataConditions[i].getOperator() == MetaDataCondition.LIKE)
							|| (metaDataConditions[i].getOperator() == MetaDataCondition.NOT_LIKE)) {
						metaDataConditions[i] = MetaDataSet
								.newCondition(metaDataConditions[i]
										.getFieldName(), metaDataConditions[i]
										.getOperator(),
										fixLIKE(metaDataConditions[i]
												.getStringValue()));
					}
				}
			}
		}

		if (conditionsLength > 0) {
			conditions = new MetaDataCondition[conditionsLength];
			System.arraycopy(metaDataConditions, 0, conditions, 0,
					conditionsLength);
		}
		System.arraycopy(metaDataSelects, 0, selects, 0, selectsLength);

		if (conditions != null) {
			// remove duplicate/null conditions.
			conditionsLength = conditions.length;

			// Set the proper conditionsLength, when MetaDataTable
			for (i = 0; i < conditions.length; i++) {
				if (SRBCommands.DEBUG > 3) {
					System.err.println("conditions     "
							+ conditions[i].getFieldName() + " "
							+ conditions[i].getStringValue());
				}
				if (conditions[i].getStyle() == MetaDataCondition.TABLE) {
					int rowCount = conditions[i].getTableValue().getRowCount();
					if (rowCount >= 5) {
						// 5 rows max, users and resources don't have fifth row.
						if (conditions[i].getFieldName().equals(
								SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES)
								|| conditions[i]
										.getFieldName()
										.equals(SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES)) {
							conditionsLength += 5 * conditions[i]
									.getTableValue().getColumnCount() - 1;
						} else {
							conditionsLength += 4 * conditions[i]
									.getTableValue().getColumnCount() - 1;
						}
					} else {
						// the table condition is already included, so
						// rows*columns minus one
						conditionsLength += rowCount
								* conditions[i].getTableValue()
										.getColumnCount() - 1;
					}
				}
			}

			if (conditionsLength > 0) {
				temp = 0;
				conditionIDs = new int[conditionsLength];
				for (i = 0; i < conditions.length; i++) {
					conditionIDs[i] = SRBMetaDataSet.getSRBID(conditions[i]
							.getFieldName());
				}
				for (i = 0; i < conditions.length; i++) {
					// if condition exists and it wasn't a double && condition,
					// which gets included with the early condition of the same
					// metadata type
					if ((conditions[i] != null)
							&& (conditionIDs[i] != Integer.MIN_VALUE)) {
						for (j = i + 1; j < conditions.length; j++) {
							// Cases of multiple conditions on same metadata
							// type
							if (conditionIDs[i] == conditionIDs[j]) {
								conditionsLength--;
								conditionIDs[j] = Integer.MIN_VALUE;
							}
						}

						if (conditionIDs[i] == Integer.MIN_VALUE) {
							// skip it
						} else if (conditions[i].getStyle() == MetaDataCondition.TABLE) {
							// conditions use User Definable metadata
							int[] conIDs = setTableConditionIDs(
									conditions[i].getTableValue(),
									conditions[i].getFieldName());

							if (conditions.length < conditionIDs.length + temp) {
								// Because they && the same condition use this
								// stuff

								int[] oldConIDs = new int[conditionIDs.length
										- temp - 1];

								// just setting oldConIDs = conditionIDs,
								// doesn't work
								System.arraycopy(conditionIDs, temp + 1,
										oldConIDs, 0, oldConIDs.length);

								// add all the rows to the main list
								System.arraycopy(conIDs, 0, conditionIDs, temp,
										conIDs.length);

								// don't include first one, already counted.
								temp += conIDs.length;

								// shift the old rows down where necessary
								for (j = temp; j < conditionIDs.length; j++) {
									conditionIDs[j] = oldConIDs[j - temp];
									i++;
								}
							} else {
								// add all the rows to the main list
								System.arraycopy(conIDs, 0, conditionIDs, temp,
										conIDs.length);
								// don't include first one, already counted.
								temp += conIDs.length;
							}
						} else {
							conditionIDs[temp] = SRBMetaDataSet
									.getSRBID(conditions[i].getFieldName());
							temp++;
						}
					} else {
						temp++;
					}
				}
			}
		} else {
			conditionsLength = 0;
		}

		// remove duplicate/null selects.
		selects = (MetaDataSelect[]) cleanNullsAndDuplicates(selects);
		if (selects == null) {
			return null;
		}
		selectsLength = selects.length;
		int[] selectsIDs = new int[selectsLength];
		int[] selectsValues = new int[selectsLength];

		// Create the predefined SRB selVal integer array
		temp = selectsLength;
		for (i = 0; i < temp; i++) {
			fieldID = SRBMetaDataSet.getSRBID(selects[i].getFieldName());
			// = 1; the byte order is reversed.

			if (fieldID > 0) {
				// the normal, non-user definable metadata
				selectsIDs[i] = fieldID;
				selectsValues[i] = selects[i].getOperation();
			} else if ((fieldID < 0) && (fieldID > -4)) {
				// first find out how many rows there will be to get all the
				// definableMD
				// for this groups of files.
				int multipleRows = 0;
				MetaDataSelect[] tempSelects = new MetaDataSelect[3];

				if (fieldID == -1) {
					tempSelects[0] = MetaDataSet
							.newSelection(StandardMetaData.FILE_NAME);
					tempSelects[1] = MetaDataSet
							.newSelection(StandardMetaData.DIRECTORY_NAME);
					tempSelects[2] = MetaDataSet.newSelection(
							SRBMetaDataSet.METADATA_NUM, 9);
					metaDataNumAt = 2;
				} else if (fieldID == -2) {
					tempSelects[0] = MetaDataSet
							.newSelection(StandardMetaData.DIRECTORY_NAME);
					tempSelects[1] = MetaDataSet.newSelection(
							SRBMetaDataSet.METADATA_NUM_DIRECTORY, 9);
					metaDataNumAt = 1;
				} else if (fieldID == -3) {
					tempSelects[0] = MetaDataSet
							.newSelection(UserMetaData.USER_NAME);
					tempSelects[1] = MetaDataSet
							.newSelection(SRBMetaDataSet.USER_DOMAIN);
					tempSelects[2] = MetaDataSet.newSelection(
							SRBMetaDataSet.METADATA_NUM_USER, 9);
					metaDataNumAt = 2;
				} else if (fieldID == -4) {
					tempSelects[0] = MetaDataSet
							.newSelection(ResourceMetaData.RESOURCE_NAME);
					tempSelects[1] = MetaDataSet.newSelection(
							SRBMetaDataSet.METADATA_NUM_RESOURCE, 9);
					metaDataNumAt = 1;
				}
				metaDataNumList = srbGenQuery(catType, myMcatZone, conditions,
						tempSelects, rowsWanted, true, nonDistinct);

				if (metaDataNumList != null) {
					for (j = 0; j < metaDataNumList.length; j++) {
						// if there are no results it returns -1
						if (metaDataNumList[j].getIntValue(metaDataNumAt) >= 0) {
							// the max value is one low, since the count starts
							// at zero
							multipleRows += metaDataNumList[j]
									.getIntValue(metaDataNumAt);
						}
					}

					if (multipleRows + metaDataNumList.length > rowsWanted) {
						// if total number of user defined rows plus
						// the number of results for just system metadata is
						// greater than
						// the rowsWanted, then just do what you can rowsWanted
						// plus enough
						// user defined metadata to finish out the last record.
						rowsWanted = multipleRows;
					}
				}

				// replace selects[i] with full table of definable metadata
				tempSelects = new MetaDataSelect[12];

				if (fieldID == -1) {
					tempSelects[0] = MetaDataSet
							.newSelection(StandardMetaData.FILE_NAME);
					tempSelects[1] = MetaDataSet
							.newSelection(StandardMetaData.DIRECTORY_NAME);
					tempSelects[2] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES0);
					tempSelects[3] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES1);
					tempSelects[4] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES2);
					tempSelects[5] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES3);
					tempSelects[6] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES4);
					tempSelects[7] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES5);
					tempSelects[8] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES6);
					tempSelects[9] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES7);
					tempSelects[10] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES8);
					tempSelects[11] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES9);
				}
				if (fieldID == -2) {
					tempSelects[0] = MetaDataSet
							.newSelection(StandardMetaData.DIRECTORY_NAME);
					tempSelects[1] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES0);
					tempSelects[2] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES1);
					tempSelects[3] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES2);
					tempSelects[4] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES3);
					tempSelects[5] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES4);
					tempSelects[6] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES5);
					tempSelects[7] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES6);
					tempSelects[8] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES7);
					tempSelects[9] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES8);
					tempSelects[10] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES9);
				}
				if (fieldID == -3) {
					tempSelects[0] = MetaDataSet
							.newSelection(UserMetaData.USER_NAME);
					tempSelects[1] = MetaDataSet
							.newSelection(SRBMetaDataSet.USER_DOMAIN);
					tempSelects[2] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_USERS0);
					tempSelects[3] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_USERS1);
					tempSelects[4] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_USERS2);
					tempSelects[5] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_USERS3);
					tempSelects[6] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_USERS4);
					tempSelects[7] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_USERS5);
					tempSelects[8] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_USERS6);
					tempSelects[9] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_USERS7);
					tempSelects[10] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_USERS8);
					tempSelects[11] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_USERS9);
				}
				if (fieldID == -4) {
					tempSelects[0] = MetaDataSet
							.newSelection(ResourceMetaData.RESOURCE_NAME);
					tempSelects[1] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_RESOURCES0);
					tempSelects[2] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_RESOURCES1);
					tempSelects[3] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_RESOURCES2);
					tempSelects[4] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_RESOURCES3);
					tempSelects[5] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_RESOURCES4);
					tempSelects[6] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_RESOURCES5);
					tempSelects[7] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_RESOURCES6);
					tempSelects[8] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_RESOURCES7);
					tempSelects[9] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_RESOURCES8);
					tempSelects[10] = MetaDataSet
							.newSelection(SRBMetaDataSet.DEFINABLE_METADATA_FOR_RESOURCES9);
				}

				selects[i] = null;
				orderBy = true;
				// merge new selects to the begin for orderBy
				selects = MetaDataSet.mergeSelects(tempSelects, selects);
				selects = (MetaDataSelect[]) cleanNullsAndDuplicates(selects);
				if (selects == null) {
					return null;
				}

				// easier to just start over
				i = -1;
				selectsLength = selects.length;
				temp = selectsLength;
				selectsIDs = new int[selectsLength];
				selectsValues = new int[selectsLength];
			} else {
				throw new NullPointerException(
						"Extensible metadata or invalid metadata attribute: "
								+ selects[i].getFieldName()
								+ " not support in this version");
			}
		}

		byte[] correctSizeQuery = null;
		temp = 0;
		// -------------------------------------------------
		// Form the final packed query
		// -------------------------------------------------
		// how many selects, int
		if (nonDistinct) {
			Host.copyInt(selectsLength + 1, packedQuery, temp);
			temp += SRBCommands.INT_LENGTH;
		} else {
			Host.copyInt(selectsLength, packedQuery, temp);
			temp += SRBCommands.INT_LENGTH;
		}

		// how many conditions, int
		if (orderBy) {
			Host.copyInt(conditionsLength + 1, packedQuery, temp);
			temp += SRBCommands.INT_LENGTH;
		} else {
			Host.copyInt(conditionsLength, packedQuery, temp);
			temp += SRBCommands.INT_LENGTH;
		}

		// the id # of the selects, ints
		for (i = 0; i < selectsLength; i++) {
			Host.copyInt(selectsIDs[i], packedQuery, temp);
			temp += SRBCommands.INT_LENGTH;
		}
		if (nonDistinct) {
			Host.copyInt(SRBMetaDataSet.getSRBID(SRBMetaDataSet.NONDISTINCT),
					packedQuery, temp);
			temp += SRBCommands.INT_LENGTH;
		}

		// the values of the selects, ints
		for (i = 0; i < selectsLength; i++) {
			Host.copyInt(selectsValues[i], packedQuery, temp);
			temp += SRBCommands.INT_LENGTH;
		}
		if (nonDistinct) {
			Host.copyInt(1, packedQuery, temp);
			temp += SRBCommands.INT_LENGTH;
		}

		// set the id # of the conditions, ints
		if (conditionsLength > 0) {
			for (i = 0; i < conditionIDs.length; i++) {
				// negative IDs were set with the && double condition on
				// metadata
				if (conditionIDs[i] >= 0) {
					Host.copyInt(conditionIDs[i], packedQuery, temp);
					temp += SRBCommands.INT_LENGTH;
					if (SRBCommands.DEBUG > 4) {
						System.err.println("conditionID " + conditionIDs[i]);
					}
				}
			}
		}

		if (orderBy) {
			Host.copyInt(SRBMetaDataSet.getSRBID(SRBMetaDataSet.ORDERBY),
					packedQuery, temp);
			temp += SRBCommands.INT_LENGTH;
		}

		// the value of the conditions, null-term string
		if (conditionsLength > 0) {
			for (i = 0; i < conditions.length; i++) {
				if (conditions[i] != null) {
					int style = conditions[i].getStyle();
					queryString += addValue(style, conditions[i]);
					System.arraycopy(queryString.getBytes(), 0, packedQuery,
							temp, queryString.getBytes().length);

					if (style == MetaDataCondition.TABLE) {
						temp += queryString.getBytes().length; // length (has an
						// extra null?)
					} else {
						temp += queryString.getBytes().length + 1; // length+null
					}
					queryString = "";

					for (j = i + 1; j < conditions.length; j++) {
						// Cases of multiple conditions on same metadata type
						if ((conditions[j] != null)
								&& (conditions[i].getField()
										.equals(conditions[j].getField()))) {
							// remove the false null value
							temp--;

							// in cases of: qval[i] = "like 'A*' && like 'B*'"
							style = conditions[j].getStyle();
							queryString += " && "
									+ addValue(style, conditions[j]);
							System.arraycopy(queryString.getBytes(), 0,
									packedQuery, temp,
									queryString.getBytes().length);
							conditions[j] = null;
							if (style == MetaDataCondition.TABLE) {
								temp += queryString.getBytes().length; // length
								// (has
								// an
								// extra
								// null?)
							} else {
								temp += queryString.getBytes().length + 1; // length+null
							}
							queryString = "";
						}
					}
				}
			}
		}

		if (orderBy) {
			queryString = SRBMetaDataSet.getSRBName(selects[0].getFieldName());
			for (j = 1; j < selects.length; j++) {
				if (selects[j].getOperation() == 1) {
					queryString += ", "
							+ SRBMetaDataSet.getSRBName(selects[j]
									.getFieldName());
				}
			}
			System.arraycopy(queryString.getBytes(), 0, packedQuery, temp,
					queryString.getBytes().length);
			temp += queryString.getBytes().length + 1; // length+null
		}

		correctSizeQuery = new byte[temp + SRBCommands.INT_LENGTH];
		System.arraycopy(packedQuery, 0, correctSizeQuery,
				SRBCommands.INT_LENGTH, temp);

		// must add total length of packedQuery to beginning
		Host.copyInt(temp, correctSizeQuery, 0);

		if (SRBCommands.DEBUG > 1) {
			System.err.print("\nselects[j].getFieldName() " + selectsLength);
			for (j = 0; j < selectsLength; j++) {
				System.err.print("\n" + selectsIDs[j]);
				System.err.print(" " + selectsValues[j]);
			}

			if (SRBCommands.DEBUG > 4) {
				System.err.print("\ncorrectSizeQuery1.");
				for (i = 0; i < correctSizeQuery.length; i++) {
					if (correctSizeQuery[i] > 32) {
						System.err.print((char) correctSizeQuery[i] + "_");
					} else {
						System.err.print(correctSizeQuery[i] + ".");
					}
				}
				System.err.print("\n\n");
			}
		}

		commands.startSRBCommand(SRBCommands.F_GEN_QUERY, 4);

		commands.sendArg(catType);
		commands.sendArg(myMcatZone);
		commands.sendArg(correctSizeQuery);
		commands.sendArg(rowsWanted);
		commands.flush();

		try {
			commands.commandStatus();
		} catch (SRBException e) {
			if (e.getType() == -3005) {
				// the database contains no objects matching the query.
				return null;
			} else {
				throw e;
			}
		}
		recordList = commands.returnSRBMetaDataRecordList(false, null);
		if (metaDataNumList != null) {
			for (i = 0; i < recordList.length; i++) {
				recordList[i].addMetaDataNums(metaDataNumList, metaDataNumAt);
			}
		}

		return recordList;
	}
}
// End SRBMetaDataCommands-----
