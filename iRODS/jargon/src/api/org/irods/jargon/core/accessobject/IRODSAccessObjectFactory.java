/**
 * 
 */
package org.irods.jargon.core.accessobject;

import org.irods.jargon.core.exception.JargonException;

/**
 * Defines the interface for a factory to create access objects for IRODS.
 * Access object are a concept in the newer jargon-core API, and the intent is
 * not to re-implement all of these capabilities in jargon-trunk. The access
 * objects here in jargon-trunk are meant to aid in transition to the new API,
 * and to help in eventual refactoring.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public interface IRODSAccessObjectFactory {

	/**
	 * Get an access object that can execute iquest-like queries
	 * 
	 * @return {@link org.irods.jargon.core.accessobject.IRODSGenQueryExecutor}
	 * @throws JargonException
	 */
	IRODSGenQueryExecutor getIRODSGenQueryExcecutor() throws JargonException;

	/**
	 * Get an access object that can executes scripts on a remote server
	 * 
	 * @return {@link org.irods.jargon.core.accessobject.RemoteExecutionOfCommansAO}
	 * @throws JargonException
	 */
	RemoteExecutionOfCommandsAO getRemoteExecutionOfCommandsAO()
			throws JargonException;

	/**
	 * Get an access object to do bulk file operations (transfer and
	 * bundle/unbundle of tar files)
	 * 
	 * @return
	 * @throws JargonException
	 */
	BulkFileOperationsAO getBulkFileOperationsAO() throws JargonException;

	/**
	 * Get an access object to interact with data objects and collections
	 * 
	 * @return
	 * @throws JargonException
	 */
	FileCatalogObjectAO getFileCatalogObjectAO() throws JargonException;

}
