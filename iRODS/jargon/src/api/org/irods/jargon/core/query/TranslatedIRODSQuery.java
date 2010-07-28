/**
 * 
 */
package org.irods.jargon.core.query;

import java.util.List;

import org.irods.jargon.core.exception.JargonException;

/**
 * Represents a query translated into a format that can be processed into the
 * IRODS protocol. Essentially this is a bridge between a query stated as plain
 * text and the format of queries understood in a <code>Tag</code> format.
 * 
 * This object is immutable, and is safe to share between threads.  This class is not marked final
 * to assist in testability.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class TranslatedIRODSQuery {
	public static TranslatedIRODSQuery instance(
			final List<SelectField> translatedSelectFields,
			final List<TranslatedQueryCondition> translatedQueryConditions,
			final IRODSQuery irodsQuery) throws JargonException {
		return new TranslatedIRODSQuery(translatedSelectFields,
				translatedQueryConditions, irodsQuery, true);

	}
	public static TranslatedIRODSQuery instance(
			final List<SelectField> translatedSelectFields,
			final List<TranslatedQueryCondition> translatedQueryConditions,
			final IRODSQuery irodsQuery, final boolean distinct)
			throws JargonException {
		return new TranslatedIRODSQuery(translatedSelectFields,
				translatedQueryConditions, irodsQuery, distinct);

	}
	private final boolean distinct;
	private final IRODSQuery irodsQuery;

	// TODO: fix lists to be immutable wrapped

	private final List<SelectField> selectFields;

	private final List<TranslatedQueryCondition> translatedQueryConditions;

	private TranslatedIRODSQuery(final List<SelectField> selectFields,
			final List<TranslatedQueryCondition> translatedQueryConditions,
			final IRODSQuery irodsQuery, final boolean distinct)
			throws JargonException {

		if (translatedQueryConditions == null) {
			throw new JargonException("conditions are null");
		}

		if (selectFields == null) {
			throw new JargonException("no select column names");
		}

		if (selectFields.isEmpty()) {
			throw new JargonException("no select column names");
		}

		if (irodsQuery == null) {
			throw new JargonException("irodsQuery is null");
		}

		this.translatedQueryConditions = translatedQueryConditions;
		this.selectFields = selectFields;
		this.irodsQuery = irodsQuery;
		this.distinct = distinct;

	}

	public IRODSQuery getIrodsQuery() {
		return irodsQuery;
	}

	/**
	 * Get the {@link org.irods.jargon.core.query.RodsGenQueryEnum
	 * RodsGenQueryEnum} data that describes the particular select column.
	 * 
	 * @return <code>RodsGenQueryEnum</code> with the column names translated in
	 *         the internal representation.
	 */
	public List<SelectField> getSelectFields() {
		return selectFields;
	}

	/**
	 * Get the condition portion of a query translated into the internal
	 * representation.
	 * 
	 * @return {@link org.irods.jargon.core.query.TranslatedQueryCondition
	 *         TranslatedQueryCondition} containing the internal represntation
	 *         of the condition portion of the query.
	 */
	public List<TranslatedQueryCondition> getTranslatedQueryConditions() {
		return translatedQueryConditions;
	}

	protected boolean isDistinct() {
		return distinct;
	}

}
