/**
 *
 */
package org.irods.jargon.core.exception;

/**
 * A security exception from Jargon
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class JargonSecurityException extends Exception {

	/**
	 * 
	 */
	private static final long serialVersionUID = 7582201634619448087L;

	/**
	 *
	 */
	public JargonSecurityException() {
		// TODO Auto-generated constructor stub
	}

	/**
	 * @param message
	 */
	public JargonSecurityException(final String message) {
		super(message);
		// TODO Auto-generated constructor stub
	}

	/**
	 * @param message
	 * @param cause
	 */
	public JargonSecurityException(final String message, final Throwable cause) {
		super(message, cause);
		// TODO Auto-generated constructor stub
	}

	/**
	 * @param cause
	 */
	public JargonSecurityException(final Throwable cause) {
		super(cause);
		// TODO Auto-generated constructor stub
	}

}
