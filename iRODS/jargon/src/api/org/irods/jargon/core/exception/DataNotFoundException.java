/**
 * 
 */
package org.irods.jargon.core.exception;

/**
 * The data requested does not exist in IRODS.  Generally, this is non-fatal, and should be handled as a message to the user
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
public class DataNotFoundException extends Exception {

	/**
	 * 
	 */
	public DataNotFoundException() {
	}

	/**
	 * @param message
	 */
	public DataNotFoundException(String message) {
		super(message);
	}

	/**
	 * @param cause
	 */
	public DataNotFoundException(Throwable cause) {
		super(cause);
	}

	/**
	 * @param message
	 * @param cause
	 */
	public DataNotFoundException(String message, Throwable cause) {
		super(message, cause);
	}

}
