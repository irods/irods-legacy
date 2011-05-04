package org.irods.jargon.core.accessobject;

import org.irods.jargon.core.exception.JargonException;

import edu.sdsc.grid.io.irods.IRODSFileSystem;

/**
 * Simplified interface that describes an access object that deals with iRODS data objects (files).  Note that this mirrors the access object components in
 * the newer Jargon-core API, but is greatly simplified, existing in this API version mostly to remove code from the <code>IRODSCommands</code> object.
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
public interface FileCatalogObjectAO {

	/**
	 * For a get operation, determine the resource server host that actually contains the data.  This is used to determine connection re-routing.
	 * @param sourceAbsolutePath <code>String</code> with the absolute path to the iRODS file that will be retrieved
	 * @param resourceName <code>String</code> with an optional resource name.  If provided, the connection is re-routed to the resource host
	 * @return <code>String</code> with either the host name, or the string 'thisHost', which indicates that re-routing is not needed
	 * @throws JargonException
	 */
	String getHostForGetOperation(
			final String sourceAbsolutePath, final String resourceName)
			throws JargonException;

	/**
	 * For a put operation, determine the resource server host that is associated with the given resource.  This is used to determine connection re-routing.
	 * @param sourceAbsolutePath <code>String</code> with the absolute path to the iRODS file that will the target of the put
	 * @param resourceName <code>String</code> with an optional resource name.  If provided, the connection is re-routed to the resource host
	 * @return <code>String</code> with either the host name, or the string 'thisHost', which indicates that re-routing is not needed
	 * @throws JargonException
	 */
	String getHostForPutOperation(final String targetAbsolutePath, final String resourceName)
			throws JargonException; 

	/**
	 * For a given absolute path (which can be a file or a collection), return the <code>IRODSFileSystem</code> that connects to the optimal
	 * resource to access that file when the iRODS file is a source.  Note that <code>null</code> will be returned if no rerouting option was discovered.
	 * @param irodsFileAbsolutePath <code>String</code> with the absolute path to the iRODS file or collection that is the target
	 * @param resourceName <code>String</code> with an optional resource name.  Giving the name will return a connection to the host for that resource
	 * server.  Note that this can be set to blank (not null) to base selection on a query of resources that the file does exist on.
	 * @return {@link IRODSFileSystem} that points to a resource server that has the given file, or <code>null</code> if no resource switch should be done.
	 * @throws JargonException
	 */
	IRODSFileSystem rerouteIrodsFileWhenIRODSIsSource(String irodsFileAbsolutePath,
			String resourceName) throws JargonException;

}