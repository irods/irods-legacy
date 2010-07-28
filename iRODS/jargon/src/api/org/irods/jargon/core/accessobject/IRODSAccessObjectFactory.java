/**
 * 
 */
package org.irods.jargon.core.accessobject;

import org.irods.jargon.core.exception.JargonException;

/**
 * Defines the interface for a factory to create access objects for IRODS.  Note that this functionality should be considered early access and 
 * subject to API change.
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
public interface IRODSAccessObjectFactory {
	
	/**
	 * Get an access object that can execute iquest-like queries
	 * @return {@link org.irods.jargon.core.accessobject.IRODSGenQueryExecutor}
	 * @throws JargonException
	 */
	IRODSGenQueryExecutor getIRODSGenQueryExcecutor() throws JargonException;

}
