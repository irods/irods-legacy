package org.irods.jargon.core.accessobject;

import org.irods.jargon.core.exception.JargonException;

/**
 * Simplified interface that describes an access object that deals with iRODS data objects (files).  Note that this mirrors the access object components in
 * the newer Jargon-core API, but is greatly simplified, existing in this API version mostly to remove code from the <code>IRODSCommands</code> object.
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
public interface FileCatalogObjectAO {

	String getHostForGetOperation(
			final String sourceAbsolutePath, final String resourceName)
			throws JargonException;

}