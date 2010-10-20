//	Copyright (c) 2007, Regents of the University of California
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
//	FTPAccount.java	-  edu.sdsc.grid.io.FTPAccount
//
//  CLASS HIERARCHY
//	java.lang.Object
//	    |
//	    +-.GeneralAccount
//						|
//						+-.RemoteAccount
//                  |
//                  +.ftp.FTPAccount
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.ftp;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

import edu.sdsc.grid.io.*;
import org.ietf.jgss.*;

/**
 * An object to hold the user information used when connecting to a FTP file
 * system.
 *<P>
 * 
 * @author Lucas Gilbert, San Diego Supercomputer Center
 */
public class FTPAccount extends RemoteAccount {
	// ----------------------------------------------------------------------
	// Constants
	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	// Fields
	// ----------------------------------------------------------------------

	/**
	 * Set to true if this account should attempt to use the GridFTP protocol.
	 */
	boolean useGridFTP = false;

	GSSCredential credential;

	// ----------------------------------------------------------------------
	// Constructors and Destructors
	// ----------------------------------------------------------------------
	/**
	 * Constructs an object to hold the user information used when connecting to
	 * a ftp server.
	 * <P>
	 * 
	 * @param host
	 *            the ftp system domain name
	 * @param port
	 *            the port on the ftp system
	 * @param userName
	 *            the user name
	 * @param password
	 *            the password
	 * @param homeDirectory
	 *            home directory on the ftp system
	 */
	public FTPAccount(String host, int port, String userName, String password,
			String homeDirectory) {
		super(host, port, userName, password, homeDirectory);
	}

	/**
	 * Constructs an object to hold the user information used when connecting to
	 * a ftp server.
	 * <P>
	 * 
	 * @param host
	 *            the ftp system domain name
	 * @param port
	 *            the port on the ftp system
	 * @param userName
	 *            the user name
	 * @param password
	 *            the password
	 * @param homeDirectory
	 *            home directory on the ftp system
	 */
	public FTPAccount(String host, int port, String userName,
			GSSCredential credential, String homeDirectory) {
		super(host, port, userName, "", homeDirectory);
		this.credential = credential;
		useGridFTP = true;
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
	 * Sets the home directory of this FTPAccount.
	 * 
	 * @throws NullPointerException
	 *             if homeDirectory is null.
	 */
	@Override
	public void setHomeDirectory(String homeDirectory) {
		if (homeDirectory == null)
			throw new NullPointerException(
					"The home directory string cannot be null");

		this.homeDirectory = homeDirectory;
	}

	@Override
	public void setPort(int port) {
		if (port > 0)
			this.port = port;
		else {
			// default ftp port
			this.port = 21;
		}
	}

	URI getURI() {
		try {
			return new URI(toString());
		} catch (URISyntaxException e) {
			IllegalArgumentException x = new IllegalArgumentException(
					"Invalid URI");
			x.initCause(e);
			throw x;
		}
	}

	URL getURL() {
		try {
			return getURI().toURL();
		} catch (MalformedURLException e) {
			IllegalArgumentException x = new IllegalArgumentException(
					"Invalid URI");
			x.initCause(e);
			throw x;
		}
	}

	public boolean isUseGridFTP() {
		return useGridFTP;
	}

	public void setUseGridFTP(boolean useGridFTP) {
		this.useGridFTP = useGridFTP;
	}

	public void setGSSCredential(GSSCredential credential) {
		this.credential = credential;
	}

	public GSSCredential getGSSCredential() {
		return credential;
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
	public boolean equals(Object obj) {
		try {
			if (obj == null || !(obj instanceof FTPAccount))
				return false;

			FTPAccount temp = (FTPAccount) obj;

			if (!getHost().equals(temp.getHost()))
				return false;
			if (getPort() != temp.getPort())
				return false;
			if (!getUserName().equals(temp.getUserName()))
				return false;
			if (!getPassword().equals(temp.getPassword()))
				return false;

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
		return new String("ftp://" + getUserName() + "@" + getHost() + ":"
				+ getPort() + getHomeDirectory());
	}
}
