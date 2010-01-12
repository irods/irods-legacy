/**
 *
 */
package org.irods.jargon.core.exception;

/**
 * A general exception that has occurrred in IRODS.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 * 
 */
public class JargonException extends Exception {

	/**
	 * 
	 */
	private static final long serialVersionUID = -4060585048895549767L;

	/**
	 *
	 */
	public JargonException() {
	}

	/**
	 * @param message
	 */
	public JargonException(final String message) {
		super(message);
	}

	/**
	 * @param message
	 * @param cause
	 */
	public JargonException(final String message, final Throwable cause) {
		super(message, cause);
	}

	/**
	 * @param cause
	 */
	public JargonException(final Throwable cause) {
		super(cause);
	}

}
