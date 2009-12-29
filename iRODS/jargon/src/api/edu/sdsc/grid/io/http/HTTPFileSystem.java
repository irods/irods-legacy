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
//	HTTPFileSystem.java	-  edu.sdsc.grid.io.HTTPFileSystem
//
//  CLASS HIERARCHY
//	java.lang.Object
//	    |
//	    +-edu.sdsc.grid.io.GeneralFileSystem
//	 			   |
//	 			   +-.RemoteFileSystem
//                  |
//                  +.http.HTTPFileSystem
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.http;

import edu.sdsc.grid.io.local.*;
import edu.sdsc.grid.io.*;

import java.io.*;

import java.net.URI;
import java.net.URLConnection;



/**
 * The HTTPFileSystem class is the class for connection implementations
 * to HTTP servers. 
 *<P>
 * @author	Lucas Gilbert, San Diego Supercomputer Center
 * @since   Jargon2.0
 */
public class HTTPFileSystem extends RemoteFileSystem
{
//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------
	/**
	 * HTTP has only one root, "/".
	 */
//I think...  
	public static final String HTTP_ROOT = "/";

//----------------------------------------------------------------------
//  Fields
//----------------------------------------------------------------------
	/**
	 * Use this account object instead of the parent class's
	 * GeneralAccount object.
   * Just so you don't have to recast it all the time.
	 */
	private HTTPAccount httpAccount;

//HttpURLConnection?
  URLConnection conn;
  
  /**
   * Debug setting
   */
  static int DEBUG = GeneralFileSystem.DEBUG;
  
//----------------------------------------------------------------------
//  Constructors and Destructors
//----------------------------------------------------------------------
	/**
	 * Opens a socket connection to read from and write to. Opens the account
	 * held in the HTTPAccount object. The account information stored in this
	 * object cannot be changed once constructed.
	 *
	 * @param httpAccount The HTTP account information object.
	 * @throws NullPointerException if httpAccount is null.
	 * @throws IOException if an IOException occurs.
	 */
	public HTTPFileSystem( HTTPAccount httpAccount )
		throws IOException
	{
		setAccount( httpAccount );
    conn = httpAccount.getURL().openConnection();
  }
  
	/**
	 * Opens a socket connection to read from and write to. Opens the account
	 * held in the URI. The account information stored in this
	 * object cannot be changed once constructed.
	 *
	 * @param httpAccount The HTTP account information object.
	 * @throws NullPointerException if httpAccount is null.
	 * @throws IOException if an IOException occurs.
	 */
	public HTTPFileSystem( URI uri )
		throws IOException
	{  
		setAccount( uri );
    
    conn = uri.toURL().openConnection();
	}



//----------------------------------------------------------------------
// Setters and Getters
//----------------------------------------------------------------------
	/**
	 * Loads the account information for this file system.
	 */
	protected void setAccount( GeneralAccount account )
		throws IOException
	{
		if ( account == null )
			throw new NullPointerException("Account information cannot be null");

		httpAccount = (HTTPAccount) account.clone();
		this.account = httpAccount;
	}
  
  /**
	 * Loads the account information for this file system.
	 */
	protected void setAccount( URI uri )
		throws IOException
	{
		if ( uri == null )
			throw new NullPointerException("Account information cannot be null");

		
		httpAccount = new HTTPAccount( uri );
    this.account = httpAccount;
	}

/* 
 * These three methods and anything that use them are quite ridiculous.
 * Create a new connection to the HTTP server everytime because URLConnection
 * doesn't behave like a regular socket.
 *
 */
  URLConnection getNewConn( )
    throws IOException
  {
    return httpAccount.getURL().openConnection();
  }
  InputStream getNewInputStream( )
    throws IOException
  {
    return getNewConn().getInputStream();
  }
  OutputStream getNewOutputStream( )
    throws IOException
  {
    return getNewConn().getOutputStream();
  }

  
  

//----------------------------------------------------------------------
// Methods
//----------------------------------------------------------------------
//General
  /**
	 *
	 */
	public MetaDataRecordList[] query(
  	MetaDataCondition[] conditions, MetaDataSelect[] selects )
  	throws IOException
  {
    throw new UnsupportedOperationException();
  }

	/**
	 *
	 */
	public MetaDataRecordList[] query(	MetaDataCondition[] conditions,
		MetaDataSelect[] selects, int numberOfRecordsWanted )
  	throws IOException
  {
    throw new UnsupportedOperationException();
  }

	/**
	 * Returns the root directories of the HTTP file system.
	 */
	public String[] getRootDirectories( )
	{
		String[] root = { HTTP_ROOT };

		return root;
	}

	/**
	 * Tests this filesystem object for equality with the given object.
	 * Returns <code>true</code> if and only if the argument is not
	 * <code>null</code> and both are filesystem objects connected to the
	 * same filesystem using the same account information.
	 *
	 * @param   obj   The object to be compared with this abstract pathname
	 *
	 * @return  <code>true</code> if and only if the objects are the same;
	 *          <code>false</code> otherwise
	 */
	public boolean equals( Object obj )
	{
    if (obj instanceof HTTPFileSystem) {
      if (toString().equals(obj.toString())) {
        return true;
      }
    }
    return false;
	}

	/**
	 * Returns a string representation of this file system object.
	 */
	public String toString( )
	{
		return conn.getURL().toString();
	}

  /**
   * Checks if the socket is connected.
   */
  public boolean isConnected( )
  {
    if (conn != null) {
      //just in case?
      try {
        conn.connect();
      } catch (IOException e) {
        return false;
      }
      
      return true;
    }
    return false;
  }

	/**
	 * Closes the connection to the HTTP file system. The filesystem
	 * cannot be reconnected after this method is called. If this object,
	 * or another object which uses this filesystem, tries to send a
	 * command to the server a ClosedChannelException will be thrown.
	 */
	public void close( ) 
    throws IOException
	{
//TODO?		conn.close();
    conn = null;
	}
  
  
}


