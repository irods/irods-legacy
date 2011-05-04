package org.irods.jargon.core.accessobject;

import org.irods.jargon.core.connection.ConnectionConstants;
import org.irods.jargon.core.connection.IRODSServerProperties;
import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.packinstr.DataObjInp;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

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
	//@Override
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
	
	//@Override
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
	
	
	//@Override
	/* (non-Javadoc)
	 * @see org.irods.jargon.core.accessobject.FileCatalogObjectAO#rerouteIrodsFileWhenIRODSIsSource(java.lang.String, java.lang.String)
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

		return createIRODSFileSystemForRerouting(
				reroutedIRODSFileSystem, detectedHost);

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

		return createIRODSFileSystemForRerouting(
				reroutedIRODSFileSystem, detectedHost);


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
			IRODSFileSystem reroutedIRODSFileSystem, String detectedHost)
			throws IllegalArgumentException, NullPointerException,
			JargonException {
		IRODSAccount thisAccount = getIrodsCommands().getIrodsAccount();
		if (detectedHost.equals(USE_THIS_ADDRESS)) {
			log.debug("host is equal, do not reroute resource");
		} else {
			log.info("connection will be rerouted to target host: {}", detectedHost);
			IRODSAccount reroutedAccount = new IRODSAccount(
					detectedHost, thisAccount.getPort(),
					thisAccount.getUserName(),
					thisAccount.getPassword(),
					thisAccount.getHomeDirectory(),
					thisAccount.getZone(),
					thisAccount.getDefaultStorageResource());
			try {
				reroutedIRODSFileSystem = new IRODSFileSystem(
						reroutedAccount);
			} catch (Exception e) {
				log.error("error created rerouting iRODS file system", e);
				throw new JargonException("exception creating rerouted irodsFileSystem", e);
			}
		}
		return reroutedIRODSFileSystem;
	}

}
