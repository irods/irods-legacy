/**
 * 
 */
package org.irods.jargon.core.accessobject;

import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.packinstr.DataObjInp;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.irods.IRODSCommands;
import edu.sdsc.grid.io.irods.Tag;

/**
 * This is a simplfied version of the access object defined for the new
 * Jargon-core API for Data Object operations. In this 'older' Jargon API, it
 * serves to better organize the code that traditionally was kept in the
 * <code>IRODSCommands</code> object. It is not the goal to replicate the entire
 * range of capabilities in the Jargon-core API in the older Jargon codebase.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class FileCatalogObjectAOImpl extends AbstractIRODSAccessObject implements
		FileCatalogObjectAO {

	/**
	 * value returned from 'get host for get operation' indicating that the
	 * given host address should be used
	 */
	public static final String USE_THIS_ADDRESS = "thisAddress";
	public static final String STR_PI = "STR_PI";
	public static final String MY_STR = "myStr";

	public static final Logger log = LoggerFactory
			.getLogger(FileCatalogObjectAOImpl.class);

	/**
	 * @param irodsCommands
	 * @throws JargonException
	 */
	protected FileCatalogObjectAOImpl(final IRODSCommands irodsCommands)
			throws JargonException {
		super(irodsCommands);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * org.irods.jargon.core.accessobject.FileCatalogObjectAO#getHostForGetOperation
	 * (java.lang.String, java.lang.String)
	 */
	@Override
	public String getHostForGetOperation(final String sourceAbsolutePath,
			final String resourceName) throws JargonException {

		if (sourceAbsolutePath == null || sourceAbsolutePath.length() == 0) {
			throw new IllegalArgumentException(
					"Null or empty sourceAbsolutePath");
		}

		if (resourceName == null) {
			throw new IllegalArgumentException("null resourceName");
		}

		log.info("getHostForGetOperation with sourceAbsolutePath: {}",
				sourceAbsolutePath);
		log.info("resourceName:{}", resourceName);

		DataObjInp dataObjInp = DataObjInp.instanceForGetHostForGet(
				sourceAbsolutePath, resourceName);
		Tag result = this.getIrodsCommands().irodsFunction(dataObjInp);

		// irods file doesn't exist
		if (result == null) {
			throw new JargonException(
					"null response from lookup of resource for get operation");
		}

		// Need the total dataSize
		Tag temp = result.getTag(MY_STR);
		if (temp == null) {
			throw new JargonException(
					"no host name info in response to lookup of resource for get operation");
		}

		String hostResponse = temp.getStringValue();

		log.debug("result of get host lookup:{}", hostResponse);
		return hostResponse;
	}

}
