/**
 *
 */
package edu.sdsc.jargon.testutils.icommandinvoke.icommands;

import java.util.ArrayList;
import java.util.List;

import edu.sdsc.jargon.testutils.icommandinvoke.IcommandException;

/**
 * Build an iput command using the given properties for invocation by an IcommandInvoker
 * @author Mike Conway, DICE (www.irods.org)
 * @since 10/16/2009
 *
 */
public class IputCommand implements Icommand {
	
	private String localFileName = "";
	private String irodsFileName = "";
	private boolean forceOverride = false;

	/* (non-Javadoc)
	 * @see org.irods.jargon.icommandinvoke.icommands.Icommand#buildCommand()
	 */
	@Override
	public List<String> buildCommand() throws IcommandException {
		if (localFileName.length() <= 0) {
			throw new IcommandException("no local file name provided for the iput command");
		}
		
		List<String> putCommand = new ArrayList<String>();
		putCommand.add("iput");
		
		if (forceOverride) {
			putCommand.add("-f");
		}
		
		putCommand.add(localFileName);
		
		
		putCommand.add(irodsFileName);
		return putCommand;
	}

	/**
	 * @return the localFileName
	 */
	public String getLocalFileName() {
		return localFileName;
	}

	/**
	 * @param localFileName the localFileName to set
	 */
	public void setLocalFileName(String localFileName) {
		this.localFileName = localFileName;
	}

	/**
	 * @return the irodsCollection
	 */
	public String getIrodsFileName() {
		return irodsFileName;
	}

	/**
	 * @param irodsCollection the irodsCollection to set
	 */
	public void setIrodsFileName(String irodsFileName) {
		this.irodsFileName = irodsFileName;
	}

	/**
	 * Flag indicating whether to force override of a file if file exists already
	 * @return the forceOverride
	 */
	public boolean isForceOverride() {
		return forceOverride;
	}

	/**
	 * * Flag indicating whether to force override of a file if file exists already
	 * @param forceOverride the forceOverride to set
	 */
	public void setForceOverride(boolean forceOverride) {
		this.forceOverride = forceOverride;
	}

}
