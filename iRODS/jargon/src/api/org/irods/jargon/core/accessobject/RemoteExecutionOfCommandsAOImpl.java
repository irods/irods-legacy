/**
 * 
 */
package org.irods.jargon.core.accessobject;

import java.io.InputStream;
import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.remoteexecute.RemoteExecuteServiceImpl;
import org.irods.jargon.core.remoteexecute.RemoteExecutionService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.irods.IRODSCommands;

/**
 * Transitional implementation of an access object.  This object will process remote execution of commands (scripts) on an 
 * iRODS server.
 * 
 * Note that Access Objects are a new feature of the next version of Jargon, and are transitionally implemented in the current
 * release.  In this version of Jargon, the access objects are not thread safe.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
public class RemoteExecutionOfCommandsAOImpl extends AbstractIRODSAccessObject implements RemoteExecutionOfCommandsAO {

	private static final Logger log = LoggerFactory
	.getLogger(RemoteExecutionOfCommandsAOImpl.class);

	/**
	 * @param irodsCommands
	 * @throws JargonException
	 */
	protected RemoteExecutionOfCommandsAOImpl(IRODSCommands irodsCommands)
			throws JargonException {
		super(irodsCommands);
	}
	
	/* (non-Javadoc)
	 * @see org.irods.jargon.core.accessobject.RemoteExecutionOfCommandsAO#executeARemoteCommandAndGetStreamGivingCommandNameAndArgs(java.lang.String, java.lang.String)
	 */
	//@Override
	public InputStream executeARemoteCommandAndGetStreamGivingCommandNameAndArgs(final String commandToExecuteWithoutArguments, final String argumentsToPassWithCommand) throws JargonException {
		log.info("executing remote command");
		// input parms checked in instance method
		RemoteExecutionService remoteExecuteService = RemoteExecuteServiceImpl.instance(this.getIrodsCommands(), commandToExecuteWithoutArguments, argumentsToPassWithCommand, "", "");		
		return remoteExecuteService.execute();
	}
	
	
	/* (non-Javadoc)
	 * @see org.irods.jargon.core.accessobject.RemoteExecutionOfCommandsAO#executeARemoteCommandAndGetStreamGivingCommandNameAndArgsAndHost(java.lang.String, java.lang.String, java.lang.String)
	 */
	//@Override
	public InputStream executeARemoteCommandAndGetStreamGivingCommandNameAndArgsAndHost(final String commandToExecuteWithoutArguments, final String argumentsToPassWithCommand,final String executionHost) throws JargonException {
		log.info("executing remote command");
		// input parms checked in instance method
		RemoteExecutionService remoteExecuteService = RemoteExecuteServiceImpl.instance(this.getIrodsCommands(), commandToExecuteWithoutArguments, argumentsToPassWithCommand, executionHost, "");		
		return remoteExecuteService.execute();
	}
	
	/* (non-Javadoc)
	 * @see org.irods.jargon.core.accessobject.RemoteExecutionOfCommandsAO#executeARemoteCommandAndGetStreamUsingAnIRODSFileAbsPathToDetermineHost(java.lang.String, java.lang.String, java.lang.String)
	 */
	//@Override
	public InputStream executeARemoteCommandAndGetStreamUsingAnIRODSFileAbsPathToDetermineHost(final String commandToExecuteWithoutArguments, final String argumentsToPassWithCommand,final String absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn) throws JargonException {
		log.info("executing remote command");
		// input parms checked in instance method
		RemoteExecutionService remoteExecuteService = RemoteExecuteServiceImpl.instance(this.getIrodsCommands(), commandToExecuteWithoutArguments, argumentsToPassWithCommand, "" , absolutePathOfIrodsFileThatWillBeUsedToFindHostToExecuteOn);		
		return remoteExecuteService.execute();
	}

}
