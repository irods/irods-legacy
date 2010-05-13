/**
 * 
 */
package org.irods.jargon.core.exception;

/**
 * Runtime version of JargonException for cases where JargonException cannot be
 * thrown
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class JargonRuntimeException extends RuntimeException {

	/**
	 * 
	 */
	public JargonRuntimeException() {
	}

	/**
	 * @param message
	 */
	public JargonRuntimeException(String message) {
		super(message);
	}

	/**
	 * @param cause
	 */
	public JargonRuntimeException(Throwable cause) {
		super(cause);
	}

	/**
	 * @param message
	 * @param cause
	 */
	public JargonRuntimeException(String message, Throwable cause) {
		super(message, cause);
	}

}
