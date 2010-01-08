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
//	HTTPFileOutputStream.java	-  edu.sdsc.grid.io.http.HTTPFileOutputStream
//
//  CLASS HIERARCHY
//	java.lang.Object
//	    |
//	    +-java.io.OuputStream
//					|
//					+-edu.sdsc.grid.io.GeneralFileOutputStream
//							|
//							+-edu.sdsc.grid.io.HTTPFileOutputStream
//
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.http;

import edu.sdsc.grid.io.*;


import java.io.*;


/**
 * A HTTPFileOutputStream writes bytes to a file in a file system.
 * What files are available depends on the host environment.
 *<P>
 * HTTPFileOutputStream is meant for writing streams of raw bytes such
 * as image data.
 *<P>
 * Basically just wraps java.io.FileOuputStream.
 * <P>
 * @author  Lucas Gilbert
 * @since   Jargon2.0
 */
public class HTTPFileOutputStream extends RemoteFileOutputStream
{
//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------




//----------------------------------------------------------------------
//  Fields
//----------------------------------------------------------------------
	/**
	 *
	 */
	private OutputStream out;



//----------------------------------------------------------------------
//  Constructors and Destructors
//----------------------------------------------------------------------
	/**
	 * Creates a <code>HTTPFileOutputStream</code> by
	 * opening a connection to an actual file,
	 * the file named by the path name <code>name</code>
	 * in the file system.
	 * <p>
	 * First, the security is checked to verify the file can be written.
	 * <p>
	 * If the named file does not exist, is a directory rather than a regular
	 * file, or for some other reason cannot be opened for reading then a
	 * <code>IOException</code> is thrown.
	 *
	 * @param      name   the system-dependent file name.
	 * @exception  IOException  if the file does not exist,
	 *                   is a directory rather than a regular file,
	 *                   or for some other reason cannot be opened for
	 *                   reading.
	 */
	public HTTPFileOutputStream( HTTPFileSystem fileSystem, String name )
		throws IOException
	{
		super( fileSystem, name);
	}


	/**
	 * Creates a <code>HTTPFileOutputStream</code> by
	 * opening a connection to an actual file,
	 * the file named by the <code>HTTPFile</code>
	 * object <code>file</code> in the file system.
	 * A new <code>FileDescriptor</code> object
	 * is created to represent this file connection.
	 * <p>
	 * First, the security is checked to verify the file can be written.
	 * <p>
	 * If the named file does not exist, is a directory rather than a regular
	 * file, or for some other reason cannot be opened for reading then a
	 * <code>IOException</code> is thrown.
	 *
	 * @param      file   the file to be opened for reading.
	 * @exception  IOException  if the file does not exist,
	 *                   is a directory rather than a regular file,
	 *                   or for some other reason cannot be opened for
	 *                   reading.
	 * @see        java.io.File#getPath()
	 */
	public HTTPFileOutputStream( HTTPFile file )
		throws IOException
	{
		super( file );
	}



	/**
	 * Ensures that the <code>close</code> method of this file input stream is
	 * called when there are no more references to it.
	 *
	 * @exception  IOException  if an I/O error occurs.
	 * @see        edu.sdsc.grid.io.HTTPFileOutputStream#close()
	 */
	protected void finalize()
		throws IOException
	{
		//flush doesn't do anything, because there is no buffer
		
		close();
	}


	/**
	 * Opens the given file for use by this stream.
	 *
	 * @param	file the file to be opened.
	 * @exception  IOException  if an I/O error occurs.
	 */
	protected void open( GeneralFile file )
		throws IOException
	{
		out = ((HTTPFile) file).httpFileSystem.conn.getOutputStream();
	}


	/**
	 * Writes the specified byte to this file output stream. Implements
	 * the <code>write</code> method of <code>OutputStream</code>.
	 *
	 * @param	b   the byte to be written.
	 * @exception  IOException  if an I/O error occurs.
	 */
	public void write( int b )
		throws IOException
	{
		out.write(b);
	}

	/**
	 * Writes <code>b.length</code> bytes from the specified byte array
	 * to this file output stream.
	 *
	 * @param	b   the data.
	 * @exception  IOException  if an I/O error occurs.
	 */
	public void write( byte b[] )
		throws IOException
	{
		out.write(b);
	}

	/**
	 * Writes <code>len</code> bytes from the specified byte array
	 * starting at offset <code>off</code> to this file output stream.
	 *
	 * @param	b     the data.
	 * @param	off   the start offset in the data.
	 * @param	len   the number of bytes to write.
	 * @exception  IOException  if an I/O error occurs.
	 */
	public void write( byte b[], int off, int len )
		throws IOException
	{
		out.write(b, off, len);
	}


	/**
	 * Closes this file output stream and releases any system resources
	 * associated with this stream. This file output stream may no longer
	 * be used for writing bytes.
	 *
	 * <p> If this stream has an associated channel then the channel is closed
	 * as well.
	 *
	 * @exception  IOException  if an I/O error occurs.
	 */
	public void close()
		throws IOException
	{
		out.close();
	}
}


