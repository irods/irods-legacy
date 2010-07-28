/**
 * 
 */
package org.irods.jargon.core.query;

import org.irods.jargon.core.exception.JargonException;
//TODO: clean up and comment, evaluate necessary initializers and constructors
/**
 * Immutable representation of a query of IRODS data
 * 
 * @author Mike Conway - DICE (www.irods.org)
 */
public final class IRODSQuery {

	public enum RowCountOptions {
		NO_ROW_COUNT, ROW_COUNT_FOR_THIS_RESULT, ROW_COUNT_INCLUDING_SKIPPED_ROWS
	}
	/**
	 * Creates an immutable description of a general query to IRODS with
	 * defaults.
	 * 
	 * @param queryString
	 *            <code>String</code> version of an IRODS Query
	 * @param numberOfResultsDesired
	 *            <code>int</code> with the number of desired results
	 * @param partialStartIndex
	 *            <code>int</code> with offest into results
	 * @return
	 * @throws JargonException
	 */
	public static IRODSQuery instance(final String queryString,
			final int numberOfResultsDesired) throws JargonException {
		return new IRODSQuery(queryString, numberOfResultsDesired, RowCountOptions.NO_ROW_COUNT, 0);
	}
	/**
	 * Creates an immutable description of a general query to IRODS.
	 * 
	 * @param queryString
	 *            <code>String</code> version of an IRODS Query
	 * @param numberOfResultsDesired
	 *            <code>int</code> with the number of desired results
	 * @param partialStartIndex
	 *            <code>int</code> with offest into results
	 * @param rowCountOption
	 * @return <code>RowCountOptions</code> enumeration indicating the type of
	 *         row count to be returned.
	 * @throws JargonException
	 */
	public static IRODSQuery instance(final String queryString,
			final int numberOfResultsDesired,
			final RowCountOptions rowCountOption) throws JargonException {
		return new IRODSQuery(queryString, numberOfResultsDesired,
				rowCountOption, 0);
	}

	private final int numberOfResultsDesired;;

	private final String queryString;

	private final int skip;

	private IRODSQuery(final String queryString,
			final int numberOfResultsDesired) throws JargonException {
		this(queryString, numberOfResultsDesired, RowCountOptions.NO_ROW_COUNT, 0);
	}

	private IRODSQuery(final String queryString,
			final int numberOfResultsDesired,
			final RowCountOptions rowCountOption, final int skip) throws JargonException {
		if (queryString == null || queryString.length() == 0) {
			throw new JargonException("query string must be supplied");
		}

		if (numberOfResultsDesired <= 0) {
			throw new JargonException(
					"number of results desired must be greater than zero");
		}

		if (rowCountOption == null) {
			throw new JargonException("row count option cannot be null");
		}
		
		if (skip < 0) {
			throw new JargonException("skip value cannot be negative");
		}

		this.queryString = queryString;
		this.numberOfResultsDesired = numberOfResultsDesired;
		this.skip = skip;
	}

	public int getNumberOfResultsDesired() {
		return numberOfResultsDesired;
	}

	public String getQueryString() {
		return queryString;
	}

}
