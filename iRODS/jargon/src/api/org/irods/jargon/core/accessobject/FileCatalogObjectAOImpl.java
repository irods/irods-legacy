package org.irods.jargon.core.accessobject;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.irods.jargon.core.connection.ConnectionConstants;
import org.irods.jargon.core.connection.IRODSServerProperties;
import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.packinstr.DataObjInp;
import org.irods.jargon.core.query.IRODSQuery;
import org.irods.jargon.core.query.IRODSQueryResultRow;
import org.irods.jargon.core.query.IRODSQueryResultSet;
import org.irods.jargon.core.query.JargonQueryException;
import org.irods.jargon.core.query.RodsGenQueryEnum;
import org.irods.jargon.core.utils.IRODSDataConversionUtil;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.Namespace;
import edu.sdsc.grid.io.StandardMetaData;
import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSCommands;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
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
public class FileCatalogObjectAOImpl extends AbstractIRODSAccessObject
		implements FileCatalogObjectAO {

	/**
	 * value returned from 'get host for get operation' indicating that the
	 * given host address should be used
	 */

	public static final String STR_PI = "STR_PI";
	public static final String MY_STR = "myStr";

	public static final Logger log = LoggerFactory
			.getLogger(FileCatalogObjectAOImpl.class);
	private static final String EQUALS_AND_QUOTE = " = '";
	private static final String QUOTE = "'";

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
	// @Override
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

		/*
		 * If resource is specified, then the call for getHostForGet() will
		 * return the correct resource server, otherwise, I need to see if this
		 * is a data object. When a data object is being obtained, look to iRODS
		 * to find the resources that data object is located on and pick the
		 * first one
		 */

		if (resourceName.isEmpty()) {
			if (!isDirectory(sourceAbsolutePath)) {
				log.debug("this is a file, look for resource it is stored on to retrieve host");
				int lastSlash = sourceAbsolutePath.lastIndexOf('/');
				String parentCollection = sourceAbsolutePath.substring(0,
						lastSlash);
				String dataName = sourceAbsolutePath.substring(lastSlash + 1);
				List<String> hosts = listHostsForDataObject(parentCollection,
						dataName);

				if (hosts.size() > 0) {

					if (hosts.get(0).equals("localhost")) {
						return FileCatalogObjectAO.USE_THIS_ADDRESS;
					} else {
						return hosts.get(0);
					}
				} else {
					return FileCatalogObjectAO.USE_THIS_ADDRESS;
				}
			}
		}

		/*
		 * Will have returned if a data object already and no resource
		 * specified, otherwise, ask iRODS for a host
		 */

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

	// @Override
	public String getHostForPutOperation(final String targetAbsolutePath,
			final String resourceName) throws JargonException {

		if (targetAbsolutePath == null || targetAbsolutePath.length() == 0) {
			throw new IllegalArgumentException(
					"Null or empty targetAbsolutePath");
		}

		if (resourceName == null) {
			throw new IllegalArgumentException("null resourceName");
		}

		log.info("getHostForPutOperation with targetAbsolutePath: {}",
				targetAbsolutePath);
		log.info("resourceName:{}", resourceName);

		DataObjInp dataObjInp = DataObjInp.instanceForGetHostForPut(
				targetAbsolutePath, resourceName);
		Tag result = this.getIrodsCommands().irodsFunction(dataObjInp);

		// irods file doesn't exist
		if (result == null) {
			throw new JargonException(
					"null response from lookup of resource for put operation");
		}

		// Need the total dataSize
		Tag temp = result.getTag(MY_STR);
		if (temp == null) {
			throw new JargonException(
					"no host name info in response to lookup of resource for put operation");
		}

		String hostResponse = temp.getStringValue();

		log.debug("result of get host lookup:{}", hostResponse);
		return hostResponse;
	}

	// @Override
	/*
	 * (non-Javadoc)
	 * 
	 * @see org.irods.jargon.core.accessobject.FileCatalogObjectAO#
	 * rerouteIrodsFileWhenIRODSIsSource(java.lang.String, java.lang.String)
	 */
	public IRODSFileSystem rerouteIrodsFileWhenIRODSIsSource(
			final String irodsFileAbsolutePath, final String resourceName)
			throws JargonException {

		if (irodsFileAbsolutePath == null
				|| irodsFileAbsolutePath.length() == 0) {
			throw new IllegalArgumentException(
					"irodsFileAbsolutePath is null or empty");
		}

		if (resourceName == null) {
			throw new IllegalArgumentException(
					"resource name is null, set to blank if not used");
		}

		log.info("rerouteIrodsFile check for abs path: {}",
				irodsFileAbsolutePath);
		if (!ConnectionConstants.REROUTE_CONNECTIONS) {
			log.debug("connection not rerouted");
			return null;
		}

		IRODSFileSystem reroutedIRODSFileSystem = null;

		// I am wanting to do resource rerouting, does this server support it?
		IRODSServerProperties irodsServerProperties = getIrodsCommands()
				.getIrodsServerProperties();
		if (!irodsServerProperties
				.isTheIrodsServerAtLeastAtTheGivenReleaseVersion(ConnectionConstants.REROUTE_CONNECTIONS_MIN_RODS_VERSION)) {
			log.debug("no rerouting available in this iRODS version");
			return null;
		}

		// server does re-routing, continue with check

		IRODSAccessObjectFactory irodsAccessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(getIrodsCommands());
		FileCatalogObjectAO fileCatalogObjectAO = irodsAccessObjectFactory
				.getFileCatalogObjectAO();
		String detectedHost = fileCatalogObjectAO.getHostForGetOperation(
				irodsFileAbsolutePath, resourceName);

		return createIRODSFileSystemForRerouting(reroutedIRODSFileSystem,
				detectedHost);

	}

	public IRODSFileSystem rerouteIrodsFileWhenIRODSIsTarget(
			final String irodsFileAbsolutePath, final String resourceName)
			throws JargonException {

		if (irodsFileAbsolutePath == null
				|| irodsFileAbsolutePath.length() == 0) {
			throw new IllegalArgumentException(
					"irodsFileAbsolutePath is null or empty");
		}

		if (resourceName == null) {
			throw new IllegalArgumentException(
					"resource name is null, set to blank if not used");
		}

		log.info("rerouteIrodsFile check for abs path: {}",
				irodsFileAbsolutePath);
		if (!ConnectionConstants.REROUTE_CONNECTIONS) {
			log.debug("connection not rerouted");
			return null;
		}

		IRODSFileSystem reroutedIRODSFileSystem = null;

		// I am wanting to do resource rerouting, does this server support it?
		IRODSServerProperties irodsServerProperties = getIrodsCommands()
				.getIrodsServerProperties();
		if (!irodsServerProperties
				.isTheIrodsServerAtLeastAtTheGivenReleaseVersion(ConnectionConstants.REROUTE_CONNECTIONS_MIN_RODS_VERSION)) {
			log.debug("no rerouting available in this iRODS version");
			return null;
		}

		// server does re-routing, continue with check

		IRODSAccessObjectFactory irodsAccessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(getIrodsCommands());
		FileCatalogObjectAO fileCatalogObjectAO = irodsAccessObjectFactory
				.getFileCatalogObjectAO();
		String detectedHost = fileCatalogObjectAO.getHostForPutOperation(
				irodsFileAbsolutePath, resourceName);

		return createIRODSFileSystemForRerouting(reroutedIRODSFileSystem,
				detectedHost);

	}

	/**
	 * @param reroutedIRODSFileSystem
	 * @param detectedHost
	 * @return
	 * @throws IllegalArgumentException
	 * @throws NullPointerException
	 * @throws JargonException
	 */
	public IRODSFileSystem createIRODSFileSystemForRerouting(
			IRODSFileSystem reroutedIRODSFileSystem, final String detectedHost)
			throws IllegalArgumentException, NullPointerException,
			JargonException {
		IRODSAccount thisAccount = getIrodsCommands().getIrodsAccount();
		if (detectedHost.equals(USE_THIS_ADDRESS)) {
			log.debug("host is equal, do not reroute resource");
		} else {
			log.info("connection will be rerouted to target host: {}",
					detectedHost);
			IRODSAccount reroutedAccount = new IRODSAccount(detectedHost,
					thisAccount.getPort(), thisAccount.getUserName(),
					thisAccount.getPassword(), thisAccount.getHomeDirectory(),
					thisAccount.getZone(),
					thisAccount.getDefaultStorageResource());
			try {
				reroutedIRODSFileSystem = new IRODSFileSystem(reroutedAccount);
			} catch (Exception e) {
				log.error("error created rerouting iRODS file system", e);
				throw new JargonException(
						"exception creating rerouted irodsFileSystem", e);
			}
		}
		return reroutedIRODSFileSystem;
	}

	private boolean isDirectory(final String irodsFileAbsolutePath) {

		boolean isDirectory = false;

		MetaDataRecordList[] rl = null;
		MetaDataCondition[] conditions = { MetaDataSet.newCondition(
				StandardMetaData.DIRECTORY_NAME, MetaDataCondition.EQUAL,
				irodsFileAbsolutePath) };
		MetaDataSelect[] selects = { MetaDataSet
				.newSelection(StandardMetaData.DIRECTORY_NAME) };

		try {
			rl = this.getIrodsCommands().query(conditions, selects, 500,
					Namespace.DIRECTORY);

			if (rl != null && rl.length > 0) {

				isDirectory = true;
			}

		} catch (IOException e) {
			log.warn("io exception is logged and ignored", e);
		}

		return isDirectory;
	}

	/**
	 * FIXME: this needs to be refactored, such that the access objects have a
	 * ref to IRODSFileSystem, so that the Resource object can be used.
	 * 
	 * @param irodsCollectionAbsolutePath
	 * @param dataName
	 * @return
	 * @throws JargonException
	 */
	private List<String> listHostsForDataObject(
			final String irodsCollectionAbsolutePath, final String dataName)
			throws JargonException {

		if (irodsCollectionAbsolutePath == null
				|| irodsCollectionAbsolutePath.isEmpty()) {
			throw new IllegalArgumentException(
					"null or empty irodsCollectionAbsolutePath");
		}

		if (dataName == null || dataName.isEmpty()) {
			throw new IllegalArgumentException("null or empty dataName");
		}

		List<String> hosts = new ArrayList<String>();

		log.info("listHostsForDataObject for collection: {}",
				irodsCollectionAbsolutePath);
		log.info(" dataName:{}", dataName);

		final StringBuilder sb = new StringBuilder();
		sb.append("SELECT ");
		sb.append(RodsGenQueryEnum.COL_R_LOC.getName());
		sb.append(" WHERE ");
		sb.append(RodsGenQueryEnum.COL_COLL_NAME.getName());
		sb.append(EQUALS_AND_QUOTE);
		sb.append(IRODSDataConversionUtil
				.escapeSingleQuotes(irodsCollectionAbsolutePath));
		sb.append("' AND ");
		sb.append(RodsGenQueryEnum.COL_DATA_NAME.getName());
		sb.append(EQUALS_AND_QUOTE);
		sb.append(IRODSDataConversionUtil.escapeSingleQuotes(dataName));
		sb.append(QUOTE);

		final IRODSQuery irodsQuery = IRODSQuery.instance(sb.toString(), 5000);

		IRODSAccessObjectFactory irodsAccessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(this.getIrodsCommands());

		final IRODSGenQueryExecutor irodsGenQueryExecutor = irodsAccessObjectFactory
				.getIRODSGenQueryExcecutor();

		IRODSQueryResultSet resultSet;

		try {
			resultSet = irodsGenQueryExecutor.executeIRODSQuery(irodsQuery, 0);
		} catch (JargonQueryException e) {
			log.error("query exception for resource query: " + sb.toString(), e);
			throw new JargonException("error in resource query", e);
		}

		for (IRODSQueryResultRow resultRow : resultSet.getResults()) {
			hosts.add(resultRow.getColumn(0));
		}

		log.info("hosts for data object: {}", hosts);

		return hosts;

	}

}
