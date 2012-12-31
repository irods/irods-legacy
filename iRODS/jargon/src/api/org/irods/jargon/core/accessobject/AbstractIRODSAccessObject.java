/**
 * 
 */
package org.irods.jargon.core.accessobject;

import org.irods.jargon.core.exception.JargonException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.irods.IRODSCommands;

/**
 * Classic API version of an access object. This is an object that holds an
 * <code>IRODSCommands</code> object that wraps a connection to IRODS. These
 * Access Objects are created via an <code>IRODSAccessObjectFactory</code> and
 * rely on the synchronization of the <code>IRODSCommands</code> object to
 * coordinate access to the socket over which IRODS communicates.
 * 
 * NOTE: this is a design change under consideration, and should not yet be
 * considered a stable part of the Jargon API. This AccessObject strategy is
 * meant to transition to a newer Jargon API style under parallel development.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public abstract class AbstractIRODSAccessObject {
	private final IRODSCommands irodsCommands;
	private Logger log = LoggerFactory.getLogger(this.getClass());

	protected AbstractIRODSAccessObject(final IRODSCommands irodsCommands)
			throws JargonException {
		if (irodsCommands == null) {
			throw new JargonException("irodsCommands is null");
		}

		if (!irodsCommands.isConnected()) {
			throw new JargonException("irodsCommands is not connected");
		}

		this.irodsCommands = irodsCommands;
	}

	protected IRODSCommands getIrodsCommands() {
		return irodsCommands;
	}

}
