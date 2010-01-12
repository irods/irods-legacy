/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.irods.jargon.core.exception;

import java.io.IOException;


/**
 * 
 * @author toaster
 */
public class IRODSIOException extends IOException {


	
	public IRODSIOException(Exception e) {
		super(e);
	}


	/**
	 * Constructs an instance of <code>IRodsRequestException</code> with the
	 * specified detail message.
	 * 
	 * @param msg
	 *            the detail message.
	 */
	public IRODSIOException(String msg) {
		super(msg);
	}

	public IRODSIOException(String msg, Throwable t) {
		super(msg + t.getMessage());
	}
}
