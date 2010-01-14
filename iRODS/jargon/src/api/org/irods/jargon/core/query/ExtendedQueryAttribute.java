/**
 * 
 */
package org.irods.jargon.core.query;

import org.irods.jargon.core.exception.JargonException;

/**
 * Immutable value object that contains the mapping between table/column and
 * extended query attribute number. This is used by
 * {@link org.irods.jargon.core.query.ExtendedQueryAttribute
 * ExtendedQueryAttribute}.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * @since 2.3
 * 
 */
public final class ExtendedQueryAttribute {
	private final String table;
	private final String column;
	private final String attributeValue;

	/**
	 * Create method returns a new instance with the values provided.
	 * 
	 * @param table
	 *            <code>String</code> containing table name of extended ICAT
	 * @param column
	 *            <code>String</code> containing the column name in the provided
	 *            table
	 * @param attributeValue
	 *            <code>String</code> containing a valid integer value,
	 *            representing the mapped attribute in IRODS
	 * @return <code>ExtendedQueryAttribute</code> instance
	 * @throws JargonException
	 */
	public static ExtendedQueryAttribute instance(String table, String column,
			String attributeValue) throws JargonException {
		return new ExtendedQueryAttribute(table, column, attributeValue);
	}

	private ExtendedQueryAttribute(String table, String column,
			String attributeValue) throws JargonException {
		if (table == null || table.length() == 0) {
			throw new JargonException(
					"table must be provided, and cannot be null");
		}

		if (column == null || column.length() == 0) {
			throw new JargonException(
					"column must be provided, and cannot be null");
		}

		if (attributeValue == null || attributeValue.length() == 0) {
			throw new JargonException(
					"attribute value must be provided, and cannot be null");
		}

		try {
			Integer.parseInt(attributeValue);
		} catch (NumberFormatException nfe) {
			throw new JargonException(
					"attribute value must be numeric, invalid value was:"
							+ attributeValue);
		}

		this.table = table;
		this.column = column;
		this.attributeValue = attributeValue;
	}

	public String getTable() {
		return table;
	}

	public String getColumn() {
		return column;
	}

	public String getAttributeValue() {
		return attributeValue;
	}
	
	public boolean equals(ExtendedQueryAttribute otherExtendedQueryAttribute) {
		if (otherExtendedQueryAttribute == null) {
			return false;
		}
		
		if (!isMatched(otherExtendedQueryAttribute.getTable(), otherExtendedQueryAttribute.getColumn())) {
			return false;
		}
		
		if (!otherExtendedQueryAttribute.getAttributeValue().equals(this.attributeValue)) {
			return false;
		}
		
		return true;
		
	}
	
	public boolean isMatched(String table, String column) {
		if (!column.equals(this.column)) {
			return false;
		}
		
		if (!table.equals(this.table)) {
			return false;
		}
		return true;
	}

}
