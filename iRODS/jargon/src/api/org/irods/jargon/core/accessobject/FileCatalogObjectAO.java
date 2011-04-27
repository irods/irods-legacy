package org.irods.jargon.core.accessobject;

import org.irods.jargon.core.exception.JargonException;

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

}