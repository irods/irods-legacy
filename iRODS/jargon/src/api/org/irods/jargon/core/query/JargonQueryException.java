/**
 * 
 */
package org.irods.jargon.core.query;

/**
 * Exception with the format or content of a query.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class JargonQueryException extends Exception {

	private String query = "";

	/**
	 * 
	 */
	public JargonQueryException() {
		super();
	}

	/**
	 * @param message
	 */
	public JargonQueryException(String message) {
		super(message);
	}

	/**
	 * @param message
	 * @param cause
	 */
	public JargonQueryException(String message, Throwable cause) {
		super(message, cause);
	}

	/**
	 * @param cause
	 */
	public JargonQueryException(Throwable cause) {
		super(cause);
	}

	public String getQuery() {
		return query;
	}

	public void setQuery(String query) {
		this.query = query;
	}

}
