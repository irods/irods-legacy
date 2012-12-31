/**
 * 
 */
package org.irods.jargon.core.query;

import org.irods.jargon.core.exception.JargonException;

/**
 * @author Mike Conway - DICE (www.irods.org) A field in a select, including the
 *         type of select (e.g. a field, versus a sum() or count() of a field.
 *         This is an immutable, thread-safe type
 */
public class SelectField {
	public enum SelectFieldSource {
		AVU, DEFINED_QUERY_FIELD, EXTENSIBLE_METADATA, UNKNOWN
	}

	public enum SelectFieldTypes {
		AVG, COUNT, FIELD, FILE_ACCESS, MAX, MIN, SUM
	}

	public static SelectField instance(final RodsGenQueryEnum selectField,
			final SelectFieldTypes selectFieldType,
			final SelectFieldSource selectFieldSource) throws JargonException {
		if (selectField == null) {
			throw new JargonException("select field was null");
		}
		return new SelectField(selectField.getName(),
				String.valueOf(selectField.getNumericValue()), selectFieldType,
				selectFieldSource);
	}

	public static SelectField instance(final String selectFieldName,
			final String selectFieldNumericTranslation,
			final SelectFieldTypes selectFieldType,
			final SelectFieldSource selectFieldSource) throws JargonException {
		return new SelectField(selectFieldName, selectFieldNumericTranslation,
				selectFieldType, selectFieldSource);
	}

	private final String selectFieldColumnName;
	private final String selectFieldNumericTranslation;

	private final SelectFieldSource selectFieldSource;

	private final SelectFieldTypes selectFieldType;

	private SelectField(final String selectFieldColumnName,
			final String selectFieldNumericTranslation,
			final SelectFieldTypes selectFieldType,
			final SelectFieldSource selectFieldSource) throws JargonException {

		if (selectFieldColumnName == null
				|| selectFieldColumnName.length() == 0) {
			throw new JargonException("select field was or missing");
		}

		if (selectFieldType == null) {
			throw new JargonException("field type was null");
		}

		if (selectFieldSource == null) {
			throw new JargonException("field source was null");
		}

		if (selectFieldNumericTranslation == null
				|| selectFieldNumericTranslation.length() == 0) {
			throw new JargonException("field translation is null or blank");
		}

		this.selectFieldColumnName = selectFieldColumnName;
		this.selectFieldType = selectFieldType;
		this.selectFieldSource = selectFieldSource;
		this.selectFieldNumericTranslation = selectFieldNumericTranslation;

	}

	public String getSelectFieldColumnName() {
		return selectFieldColumnName;
	}

	public String getSelectFieldNumericTranslation() {
		return selectFieldNumericTranslation;
	}

	public SelectFieldSource getSelectFieldSource() {
		return selectFieldSource;
	}

	public SelectFieldTypes getSelectFieldType() {
		return selectFieldType;
	}

}
