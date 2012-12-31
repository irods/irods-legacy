/**
 * 
 */
package org.irods.jargon.core.accessobject;

import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.query.IRODSQuery;
import org.irods.jargon.core.query.IRODSQueryResultSet;
import org.irods.jargon.core.query.JargonQueryException;

/**
 * Access object to process 'iquest-like' GenQuery. Note that this code should
 * be considered early access and subject to change.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public interface IRODSGenQueryExecutor {

	/**
	 * Execute an iquest-like query and return results in a convenient POJO
	 * object.
	 * 
	 * @param irodsQuery
	 *            {@link org.irods.jargon.core.query.IRODSQuery} that will wrap
	 *            the given iquest-like query
	 * @param continueIndex
	 *            <code>int</code> that indicates whether this is a requery when
	 *            more resuts than the limit have been generated
	 * @return {@link org.irods.jargon.core.query.IRODSQueryResultSet} that
	 *         contains the results of the query
	 * @throws JargonException
	 * @throws JargonQueryException
	 */
	IRODSQueryResultSet executeIRODSQuery(IRODSQuery irodsQuery,
			int continueIndex) throws JargonException, JargonQueryException;

	/**
	 * Execute a requery meant to retrieve more results. The previous result set
	 * contains information to requery iRODS.
	 * 
	 * @param irodsQueryResultSet
	 *            {@link org.irods.jargon.core.query.IRODSQueryResultSet} that
	 *            contains the results of the previous query.
	 * @return <code>IRODSQueryResultSet</code> containing the previous batch of
	 *         query results.
	 * @throws JargonException
	 * @throws JargonQueryException
	 */
	IRODSQueryResultSet getMoreResults(IRODSQueryResultSet irodsQueryResultSet)
			throws JargonException, JargonQueryException;

}
