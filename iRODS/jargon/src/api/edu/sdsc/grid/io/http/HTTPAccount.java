//	Copyright (c) 2006, Regents of the University of California
//	All rights reserved.
//
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided that the following conditions are
//	met:
//
//	  * Redistributions of source code must retain the above copyright notice,
//	this list of conditions and the following disclaimer.
//	  * Redistributions in binary form must reproduce the above copyright
//	notice, this list of conditions and the following disclaimer in the
//	documentation and/or other materials provided with the distribution.
//	  * Neither the name of the University of California, San Diego (UCSD) nor
//	the names of its contributors may be used to endorse or promote products
//	derived from this software without specific prior written permission.
//
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//	IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//  FILE
//	HTTPAccount.java	-  edu.sdsc.grid.io.http.HTTPAccount
//
//  CLASS HIERARCHY
//	java.lang.Object
//	    |
//	    +-.GeneralAccount
//						|
//						+-.RemoteAccount
//                  |
//                  +.http.HTTPAccount
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.http;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

import edu.sdsc.grid.io.RemoteAccount;

/**
 * An object to hold the user information used when connecting to a HTTP file
 * system.
 * <P>
 * 
 * @author Lucas Gilbert, San Diego Supercomputer Center
 * @since Jargon2.0
 */
public class HTTPAccount extends RemoteAccount {

	URI uri;

	/**
	 * Constructs an object to hold the user information used when connecting to
	 * a http server.
	 * <P>
	 * 
	 * @param host
	 *            the http system domain name
	 * @param port
	 *            the port on the http system
	 * @param userName
	 *            the user name
	 * @param password
	 *            the password
	 * @param homeDirectory
	 *            home directory on the http system
	 */
	HTTPAccount(final String host, final int port, final String userName,
			final String password, final String homeDirectory) {
		super(host, port, userName, password, homeDirectory);

		try {
			// just test it is valid
			uri = new URI(toString());
		} catch (URISyntaxException e) {
			IllegalArgumentException x = new IllegalArgumentException(
					"Invalid URI");
			x.initCause(e);
			throw x;
		}
	}

	/**
	 * Constructs an object to hold the user information used when connecting to
	 * a http server.
	 */
	public HTTPAccount(final URI uri) {
		super(uri.getHost(), uri.getPort(), uri.getUserInfo(), uri
				.getAuthority(), uri.getPath());
		this.uri = uri;
	}

	/**
	 * Finalizes the object by explicitly letting go of each of its internally
	 * held values.
	 * <P>
	 */
	@Override
	protected void finalize() {
		super.finalize();

	}

	// ----------------------------------------------------------------------
	// Setters and Getters
	// ----------------------------------------------------------------------
	/**
	 * Sets the home directory of this HTTPAccount.
	 * 
	 * @throws NullPointerException
	 *             if homeDirectory is null.
	 */
	@Override
	public void setHomeDirectory(final String homeDirectory) {
		if (homeDirectory == null || homeDirectory.equals("")) {
			this.homeDirectory = HTTPFileSystem.HTTP_ROOT;
		} else {
			this.homeDirectory = homeDirectory;
		}
	}

	@Override
	public void setPort(int port) {
		if (port > 0) {
			this.port = port;
		} else {
			// default http port
			port = 80;
		}
	}

	URI getURI() {
		return uri;
	}

	URL getURL() {
		try {
			return uri.toURL();
		} catch (MalformedURLException e) {
			IllegalArgumentException x = new IllegalArgumentException(
					"Invalid URI");
			x.initCause(e);
			throw x;
		}
	}

	// ----------------------------------------------------------------------
	// Methods
	// ----------------------------------------------------------------------
	/**
	 * Tests this account object for equality with the given object. Returns
	 * <code>true</code> if and only if the argument is not <code>null</code>
	 * and both are account objects with equivalent connection parameters.
	 * 
	 * @param obj
	 *            The object to be compared with this abstract pathname
	 * 
	 * @return <code>true</code> if and only if the objects are the same;
	 *         <code>false</code> otherwise
	 */
	@Override
	public boolean equals(final Object obj) {
		try {
			if (obj == null) {
				return false;
			}

			HTTPAccount temp = (HTTPAccount) obj;

			if (!uri.equals(temp.uri)) {
				return false;
			}

			// else //everything is equal
			return true;
		} catch (ClassCastException e) {
			return false;
		}
	}

	/**
	 * Returns a string representation of this file system object.
	 */
	@Override
	public String toString() {
		return uri.toString();
	}
}
