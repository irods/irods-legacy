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
	
	/**
	 * Get an access object that can execute iquest-like queries
	 * @return {@link org.irods.jargon.core.accessobject.IRODSGenQueryExecutor}
	 * @throws JargonException
	 */
	public IRODSGenQueryExecutor getIRODSGenQueryExcecutor() throws JargonException {
		return new IRODSGenQueryExecutorImpl(irodsCommands);
	}
	


}
