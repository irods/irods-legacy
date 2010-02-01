/**
 * 
 */
package org.irods.jargon.core.exception;

/**
 * Exception (usually treated as a 'normal' result) of adding duplicate data to IRODS.
 * A program may wish to inform the caller of a duplicate data situation, but not treat it as an error.
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
public class DuplicateDataException extends Exception {

	/**
	 * 
	 */
	public DuplicateDataException() {
	}

	/**
	 * @param message
	 */
	public DuplicateDataException(String message) {
		super(message);
	}

	/**
	 * @param cause
	 */
	public DuplicateDataException(Throwable cause) {
		super(cause);
	}

	/**
	 * @param message
	 * @param cause
	 */
	public DuplicateDataException(String message, Throwable cause) {
		super(message, cause);
	}

}
