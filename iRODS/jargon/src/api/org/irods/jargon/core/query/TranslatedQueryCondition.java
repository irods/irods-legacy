/**
 * 
 */
package org.irods.jargon.core.query;

import org.irods.jargon.core.query.SelectField.SelectFieldSource;

/**
 * Represents the field name, operator, and value for a condition in an IRODS
 * query after translating the field names to the internal IRODS values. This is
 * an internal representation for the query parser. This is an immutable object,
 * and is thread-safe.
 * 
 * @author mikeconway
 * 
 */
public class TranslatedQueryCondition {
	public static TranslatedQueryCondition instance(
			final RodsGenQueryEnum fieldName, final String operator,
			final String value) throws JargonQueryException {
		return new TranslatedQueryCondition(fieldName, operator, value);
	}
	private final String columnName;
	private final String columnNumericTranslation;
	private final SelectFieldSource fieldSource;
	private final String operator;

	private final String value;

	private TranslatedQueryCondition(final RodsGenQueryEnum fieldName,
			final String operator, final String value)
			throws JargonQueryException {
		if (fieldName == null) {
			throw new JargonQueryException("field name in condition is null");
		}

		if (operator == null) {
			throw new JargonQueryException("operator is null");
		}

		if (value == null) {
			throw new JargonQueryException("value in condition is null");
		}

		this.columnName = fieldName.getName();
		this.fieldSource = SelectField.SelectFieldSource.DEFINED_QUERY_FIELD;
		this.operator = operator;
		this.value = value;
		this.columnNumericTranslation = String.valueOf(fieldName
				.getNumericValue());

	}

	private TranslatedQueryCondition(final String fieldName,
			final SelectFieldSource fieldSource,
			final String columnNumericTranslation, final String operator,
			final String value) throws JargonQueryException {
		if (fieldName == null || fieldName.length() == 0) {
			throw new JargonQueryException(
					"field name in condition is blank or null");
		}

		if (fieldSource == null) {
			throw new JargonQueryException("field source in condition is null");
		}

		if (columnNumericTranslation == null
				|| columnNumericTranslation.length() == 0) {
			throw new JargonQueryException(
					"field source in condition is blank or null");
		}

		if (operator == null) {
			throw new JargonQueryException("operator is null");
		}

		if (value == null) {
			throw new JargonQueryException("value in condition is null");
		}

		this.columnName = fieldName;
		this.fieldSource = SelectField.SelectFieldSource.DEFINED_QUERY_FIELD;
		this.operator = operator;
		this.value = value;
		this.columnNumericTranslation = columnNumericTranslation;

	}

	public String getColumnName() {
		return columnName;
	}

	public String getColumnNumericTranslation() {
		return columnNumericTranslation;
	}

	public SelectFieldSource getFieldSource() {
		return fieldSource;
	}

	public String getOperator() {
		return operator;
	}

	public String getValue() {
		return value;
	}

	@Override
	public String toString() {
		StringBuilder b = new StringBuilder();
		String tabOver = "    ";
		char cr = '\n';
		b.append("translated query element:");
		b.append(cr);

		b.append(tabOver);
		b.append("columnName:");
		b.append(columnName);

		b.append(cr);
		b.append(tabOver);
		b.append("fieldSource:");
		b.append(fieldSource);

		b.append(cr);
		b.append(tabOver);
		b.append("fieldTranslation:");
		b.append(columnNumericTranslation);

		b.append(cr);
		b.append(tabOver);
		b.append("Operator:");
		b.append(operator);
		b.append(cr);
		b.append(tabOver);
		b.append("Value:");
		b.append(value);
		return b.toString();
	}

}
