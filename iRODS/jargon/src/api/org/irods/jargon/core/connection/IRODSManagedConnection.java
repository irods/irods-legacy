/**
 * 
 */
package org.irods.jargon.core.connection;

import org.irods.jargon.core.exception.JargonException;

import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSConnection;


/**
 * Represents the interface for maintenance of a connection to the IRODS system, and the iteraction between an
 * {@link IRODSProtocolManager IRODSConnectionManager} and an {@link IRODSConnection IRODSConnection}
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
public interface IRODSManagedConnection {
	
	//void disconnect() throws JargonException;
	//void disconnectWithIOException() throws JargonException;
	IRODSAccount getIRODSAccount();
	//IRODSServerProperties getIRODSServerProperties() throws JargonException;
	void shutdown() throws JargonException; 
	void obliterateConnectionAndDiscardErrors();
	String getConnectionUri() throws JargonException;
	boolean isConnected();
	

}
