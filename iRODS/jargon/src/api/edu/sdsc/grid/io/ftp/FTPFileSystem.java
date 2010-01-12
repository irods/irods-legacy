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
//	FTPFileSystem.java	-  edu.sdsc.grid.io.FTPFileSystem
//
//  CLASS HIERARCHY
//	java.lang.Object
//	    |
//	    +-edu.sdsc.grid.io.GeneralFileSystem
//	 			   |
//	 			   +-.RemoteFileSystem
//                  |
//                  +.ftp.FTPFileSystem
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.ftp;

import edu.sdsc.grid.io.local.*;
import edu.sdsc.grid.io.*;

import java.io.*;
import java.net.URI;

import org.globus.ftp.*;
import org.globus.ftp.exception.*;
import org.ietf.jgss.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


/**
 * The FTPFileSystem class is the class for connection implementations
 * to FTP and GridFTP servers.
 *<P>
 * @author	Lucas Gilbert, San Diego Supercomputer Center
 */
public class FTPFileSystem extends RemoteFileSystem
{

	/**
	 * FTP has only one root, "/".
	 */
	public static final String FTP_ROOT = "/";


	/**
	 * Use this account object instead of the parent class's
	 * GeneralAccount object.
   * Just so you don't have to recast it all the time.
	 */
	private FTPAccount ftpAccount;

  private FTPClient ftpClient;
  
    
  boolean closed = false;

  private static Logger log = LoggerFactory.getLogger(FTPFileSystem.class);

	/**
	 * Opens a socket connection to read from and write to. Opens the account
	 * held in the FTPAccount object. The account information stored in this
	 * object cannot be changed once constructed.
	 *
	 * @param ftpAccount The FTP account information object.
	 * @throws NullPointerException if ftpAccount is null.
	 * @throws IOException if an IOException occurs.
	 */
	public FTPFileSystem( FTPAccount ftpAccount )
		throws IOException
	{
		setAccount( ftpAccount );
    
    try {
      if (ftpAccount.useGridFTP) {
        ftpClient = new GridFTPClient(
                ftpAccount.getHost(), ftpAccount.getPort() );
        if (ftpAccount.getGSSCredential() != null) {
          ((GridFTPClient)ftpClient).authenticate(ftpAccount.getGSSCredential());
        }
        else if (getUserName() == null) {
          ftpAccount.setUserName("anonymous");
          ftpAccount.setPassword("");
        }
        ftpClient.authorize( ftpAccount.getUserName(), ftpAccount.getPassword());
      }
      else {
        ftpClient = new FTPClient( ftpAccount.getHost(), ftpAccount.getPort() );
        if (getUserName() == null) {
          ftpAccount.setUserName("anonymous");
          ftpAccount.setPassword("");
        }
        ftpClient.authorize( ftpAccount.getUserName(), ftpAccount.getPassword());
      }
    } catch (FTPException e) {
      IOException io = new IOException();
      io.initCause(e);
      throw io;
    }
	}
	/**
	 * Opens a socket connection to read from and write to. Opens the account
	 * held in the FTPAccount object. The account information stored in this
	 * object cannot be changed once constructed.
	 *
	 * @param ftpAccount The FTP account information object.
	 * @throws NullPointerException if ftpAccount is null.
	 * @throws IOException if an IOException occurs.
	 */
	public FTPFileSystem( URI uri )
		throws IOException
	{
		setAccount( uri );
    
    //or just the IOException because I already use so many other places?
    try {
      ftpClient = new FTPClient( ftpAccount.getHost(), ftpAccount.getPort() );   
      if (getUserName() == null) {
        ftpAccount.setUserName("anonymous");
        ftpAccount.setPassword("");
      }
      ftpClient.authorize( ftpAccount.getUserName(), ftpAccount.getPassword());
    } catch (FTPException e) {
      IOException io = new IOException();
      io.initCause(e);
      throw io;
    }
	}


	/**
	 * Loads the account information for this file system.
	 */
	protected void setAccount( GeneralAccount account )
		throws IOException
	{
		if ( account == null )
			throw new NullPointerException("Account information cannot be null");

		ftpAccount = (FTPAccount) account.clone();
		this.account = ftpAccount;
	}
  
	protected void setAccount( URI uri )
		throws IOException
	{
		if ( uri == null )
			throw new NullPointerException("Account information cannot be null");

		ftpAccount = new FTPAccount(
      uri.getHost(), uri.getPort(), uri.getUserInfo(), "", uri.getPath() );
		this.account = ftpAccount;
	}

  
  FTPClient getFTPClient( )
  {
    return ftpClient;
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
	 * Returns the root directories of the FTP file system.
	 */
	public String[] getRootDirectories( )
	{
		String[] root = { FTP_ROOT };

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
    if (obj instanceof FTPFileSystem) {
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
		return new String( 
     "ftp://"+getUserName()+"@"+getHost()+":"+getPort()+getHomeDirectory() );
	}

  /**
   * Checks if the socket is connected.
   */
  public boolean isConnected( )
  {
    if (ftpClient != null && !closed) {      
      return true;
    }
    return false;
  }
  
	/**
	 * Closes the connection to the FTP file system. The filesystem
	 * cannot be reconnected after this method is called. If this object,
	 * or another object which uses this filesystem, tries to send a
	 * command to the server a ClosedChannelException will be thrown.
	 */
	public void close( ) 
    throws IOException, ServerException
	{
		ftpClient.close();
    closed = true;
	}
}

