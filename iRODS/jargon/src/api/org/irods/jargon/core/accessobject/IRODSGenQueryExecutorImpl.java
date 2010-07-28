/**
 * 
 */
package org.irods.jargon.core.accessobject;

import java.util.ArrayList;
import java.util.List;

import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.packinstr.GenQueryInp;
import org.irods.jargon.core.packinstr.GenQueryOut;
import org.irods.jargon.core.query.IRODSQuery;
import org.irods.jargon.core.query.IRODSQueryResultRow;
import org.irods.jargon.core.query.IRODSQueryResultSet;
import org.irods.jargon.core.query.IRODSQueryTranslator;
import org.irods.jargon.core.query.JargonQueryException;
import org.irods.jargon.core.query.TranslatedIRODSQuery;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.irods.IRODSCommands;
import edu.sdsc.grid.io.irods.Tag;

/**
 * Classic API adaptation of access object to process 'iquest-like' queries.  This service is usable but should be considered 'early access' and subject
 * to API changes.
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
public class IRODSGenQueryExecutorImpl extends AbstractIRODSAccessObject implements IRODSGenQueryExecutor {

	private  Logger log = LoggerFactory.getLogger(this.getClass());

	/**
	 * @param irodsCommands
	 * @throws JargonException
	 */
	public IRODSGenQueryExecutorImpl(IRODSCommands irodsCommands)
			throws JargonException {
		super(irodsCommands);
	}
	
	/**
	 * Execute a requery meant to retrieve more results.  The previous result set contains 
	 * information to requery iRODS.
	 * @param irodsQueryResultSet {@link org.irods.jargon.core.query.IRODSQueryResultSet} that contains the results
	 * of the previous query.
	 * @return <code>IRODSQueryResultSet</code> containing the previous batch of query results.
	 * @throws JargonException
	 * @throws JargonQueryException
	 */
	public IRODSQueryResultSet getMoreResults(IRODSQueryResultSet irodsQueryResultSet) throws JargonException, JargonQueryException {
		log.info("getting more results for query");
		if (irodsQueryResultSet == null) {
			throw new JargonException("null irodsQueryResultSet");
		}
		
		if (!irodsQueryResultSet.isHasMoreRecords()) {
			throw new JargonQueryException("no more results");
		}
		
		return executeTranslatedIRODSQuery(irodsQueryResultSet.getTranslatedIRODSQuery(), 1);
	}
	
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

	public IRODSQueryResultSet executeIRODSQuery(IRODSQuery irodsQuery,
			int numberOfRecordsDesired) throws JargonException, JargonQueryException {
	
		log.info("executing irods query");
		IRODSQueryTranslator irodsQueryTranslator = new IRODSQueryTranslator(getIrodsCommands().getIrodsServerProperties());
		TranslatedIRODSQuery translatedIRODSQuery = irodsQueryTranslator.getTranslatedQuery(irodsQuery);
		return executeTranslatedIRODSQuery(translatedIRODSQuery,  0);
	}
	
	private IRODSQueryResultSet executeTranslatedIRODSQuery(final TranslatedIRODSQuery translatedIRODSQuery, final int continueIndex) throws JargonException {
		GenQueryInp genQueryInp = GenQueryInp.instance(translatedIRODSQuery, continueIndex);
		Tag response = getIrodsCommands().irodsFunction(genQueryInp);  
		
		if (response == null) {
			log.info("null response from IRODS call indicates no rows found, translated query was:" + translatedIRODSQuery);
			int continuation = 0;
			List<IRODSQueryResultRow> result = translateResponseIntoResultSet(response, translatedIRODSQuery);
			IRODSQueryResultSet resultSet = IRODSQueryResultSet.instance(translatedIRODSQuery, result, continuation);
			return resultSet;
		}
		
		if (log.isDebugEnabled()) {
			log.debug("query reult for translated query:" + translatedIRODSQuery);
			log.debug(response.parseTag());
		}
		
		int continuation = response.getTag(GenQueryOut.CONTINUE_INX).getIntValue();
		List<IRODSQueryResultRow> result = translateResponseIntoResultSet(response, translatedIRODSQuery);
		
		return IRODSQueryResultSet.instance(translatedIRODSQuery, result, continuation);
		
	}
	
	List<IRODSQueryResultRow>  translateResponseIntoResultSet(Tag queryResponse, TranslatedIRODSQuery translatedIRODSQuery) throws JargonException {
	
		List<IRODSQueryResultRow> resultSet = new ArrayList<IRODSQueryResultRow>();
		List<String> row = new ArrayList<String>();
		
		if (queryResponse == null) {
			// no response, create an empty result set, and never return null
			log.info("empty result set from query, returning as an empty result set ( no rows found)");
			return resultSet;
		}
		
		int rows = queryResponse.getTag(GenQueryOut.ROW_CNT).getIntValue();
		if (log.isDebugEnabled()) {
			log.debug("rows returned:" + rows);
		}
		
		int attributes = queryResponse.getTag(GenQueryOut.ATTRIB_CNT).getIntValue();
		
		for (int i = 0; i < rows; i++) {
			// new row
			row = new ArrayList<String>();
			for (int j = 0; j < attributes; j++) {

				row.add(queryResponse.getTags()[4 + j].getTags()[2 + i].getStringValue());
			}
			
			resultSet.add(IRODSQueryResultRow.instance(row, translatedIRODSQuery));
			
		}

		return resultSet;
		
	}

}
