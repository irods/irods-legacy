/**
 * 
 */
package org.irods.jargon.core.accessobject;

import org.irods.jargon.core.exception.JargonException;

import edu.sdsc.grid.io.irods.IRODSCommands;

/**
 * Implementation of an IRODSAccessObjectFactory that will return subtypes of
 * <code>AbstractIRODSAccessObject</code>.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class IRODSAccessObjectFactoryImpl implements IRODSAccessObjectFactory {
	private final IRODSCommands irodsCommands;

	/**
	 * Returns an instance of an Access Object factory that will hold a live connection to iRODS.  Note
	 * that access objects rely on irodsCommands for any thread management.
	 * @param irodsCommands
	 * @throws JargonException
	 */
	public static IRODSAccessObjectFactory instance(IRODSCommands irodsCommands)
			throws JargonException {
		return new IRODSAccessObjectFactoryImpl(irodsCommands);
	}

	private IRODSAccessObjectFactoryImpl(final IRODSCommands irodsCommands)
			throws JargonException {
		if (irodsCommands == null) {
			throw new JargonException("irodsComands is null");
		}

		if (!irodsCommands.isConnected()) {
			throw new JargonException("irodsCommands is not connected");
		}

		this.irodsCommands = irodsCommands;
	}

	protected IRODSCommands getIrodsCommands() {
		return irodsCommands;
	}
	
	/* (non-Javadoc)
	 * @see org.irods.jargon.core.accessobject.IRODSAccessObjectFactory#getIRODSGenQueryExcecutor()
	 */
	public IRODSGenQueryExecutor getIRODSGenQueryExcecutor() throws JargonException {
		return new IRODSGenQueryExecutorImpl(irodsCommands);
	}

	/* (non-Javadoc)
	 * @see org.irods.jargon.core.accessobject.IRODSAccessObjectFactory#getRemoteExecutionOfCommandsAO()
	 */
	public RemoteExecutionOfCommandsAO getRemoteExecutionOfCommandsAO()
			throws JargonException {
		return new RemoteExecutionOfCommandsAOImpl(irodsCommands);
	}
	
	//@Override
	/* (non-Javadoc)
	 * @see org.irods.jargon.core.accessobject.IRODSAccessObjectFactory#getBulkFileOperationsAO()
	 */
	public BulkFileOperationsAO getBulkFileOperationsAO() throws JargonException {
		return new BulkFileOperationsAOImpl(irodsCommands);
	}
	
	
	public FileCatalogObjectAO getFileCatalogObjectAO() throws JargonException {
		return new FileCatalogObjectAOImpl(irodsCommands);
	}
	 
}
