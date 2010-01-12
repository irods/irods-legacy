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
//	FTPFileOutputStream.java	-  edu.sdsc.grid.io.ftp.FTPFileOutputStream
//
//  CLASS HIERARCHY
//	java.lang.Object
//	    |
//	    +-java.io.OuputStream
//					|
//					+-edu.sdsc.grid.io.GeneralFileOutputStream
//							|
//							+-edu.sdsc.grid.io.RemoteFileOutputStream
//              		|
//              		+-edu.sdsc.grid.io.FTPFileOutputStream
//
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.ftp;

import org.globus.ftp.exception.*;
import edu.sdsc.grid.io.*;
import edu.sdsc.grid.io.local.LocalFile;

import java.io.*;

/**
 * A FTPFileOutputStream writes bytes to a file in a FTP file system. <br>
 * <br>
 * When possible use FTPFile.copyTo. Especially avoid editing an existing file,
 * it possibly could be very inefficient. <br>
 * <br>
 * This class is rather inefficient. FTP doesn't actually allow streaming or
 * random access of files. So it advisable to use FTPFile.copyTo instead.
 * However there are sometimes that you don't care so much that it works well,
 * rather just that it works at all... <br>
 * <br>
 * Bytes are stored locally until close() is called, only then are they sent to
 * the ftp server.
 */
public class FTPFileOutputStream extends RemoteFileOutputStream {
	// ----------------------------------------------------------------------
	// Constants
	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	// Fields
	// ----------------------------------------------------------------------
	/**
	 *
	 */
	private FTPFile file;
	private LocalFile temp;
	private OutputStream out;

	// ----------------------------------------------------------------------
	// Constructors and Destructors
	// ----------------------------------------------------------------------
	/**
	 * Creates a <code>FTPFileOutputStream</code> by opening a connection to an
	 * actual file, the file named by the path name <code>name</code> in the
	 * file system.
	 * <p>
	 * First, the security is checked to verify the file can be written.
	 * <p>
	 * If the named file does not exist, is a directory rather than a regular
	 * file, or for some other reason cannot be opened for reading then a
	 * <code>IOException</code> is thrown.
	 * 
	 * @param name
	 *            the system-dependent file name.
	 * @exception IOException
	 *                if the file does not exist, is a directory rather than a
	 *                regular file, or for some other reason cannot be opened
	 *                for reading.
	 */
	public FTPFileOutputStream(FTPFileSystem fileSystem, String name)
			throws IOException {
		super(fileSystem, name);
	}

	/**
	 * Creates a <code>FTPFileOutputStream</code> by opening a connection to an
	 * actual file, the file named by the <code>FTPFile</code> object
	 * <code>file</code> in the file system. A new <code>FileDescriptor</code>
	 * object is created to represent this file connection.
	 * <p>
	 * First, the security is checked to verify the file can be written.
	 * <p>
	 * If the named file does not exist, is a directory rather than a regular
	 * file, or for some other reason cannot be opened for reading then a
	 * <code>IOException</code> is thrown.
	 * 
	 * @param file
	 *            the file to be opened for reading.
	 * @exception IOException
	 *                if the file does not exist, is a directory rather than a
	 *                regular file, or for some other reason cannot be opened
	 *                for reading.
	 * @see java.io.File#getPath()
	 */
	public FTPFileOutputStream(FTPFile file) throws IOException {
		super(file);
	}

	/**
	 * Ensures that the <code>close</code> method of this file input stream is
	 * called when there are no more references to it.
	 * 
	 * @exception IOException
	 *                if an I/O error occurs.
	 * @see edu.sdsc.grid.io.FTPFileOutputStream#close()
	 */
	protected void finalize() throws IOException {
		close();
	}

	// ----------------------------------------------------------------------
	// Methods
	// ----------------------------------------------------------------------
	/**
	 * Opens the given file for use by this stream.
	 * 
	 * @param file
	 *            the file to be opened.
	 * @exception IOException
	 *                if an I/O error occurs.
	 */
	protected void open(GeneralFile file) throws IOException {
		this.file = (FTPFile) file;
		temp = (LocalFile) LocalFile.createTempFile(""
				+ (int) (Math.random() * 999), ""
				+ new java.util.Date().getTime());
		out = FileFactory.newFileOutputStream(temp);
	}

	/**
	 * Writes the specified byte to this file output stream. Implements the
	 * <code>write</code> method of <code>OutputStream</code>.
	 * 
	 * @param b
	 *            the byte to be written.
	 * @exception IOException
	 *                if an I/O error occurs.
	 */
	public void write(int b) throws IOException {
		out.write(b);
	}

	/**
	 * Writes <code>b.length</code> bytes from the specified byte array to this
	 * file output stream.
	 * 
	 * @param b
	 *            the data.
	 * @exception IOException
	 *                if an I/O error occurs.
	 */
	public void write(byte b[]) throws IOException {
		out.write(b);
	}

	/**
	 * Writes <code>len</code> bytes from the specified byte array starting at
	 * offset <code>off</code> to this file output stream.
	 * 
	 * @param b
	 *            the data.
	 * @param off
	 *            the start offset in the data.
	 * @param len
	 *            the number of bytes to write.
	 * @exception IOException
	 *                if an I/O error occurs.
	 */
	public void write(byte b[], int off, int len) throws IOException {
		out.write(b, off, len);
	}

	/**
	 * Closes this file output stream and releases any system resources
	 * associated with this stream. This file output stream may no longer be
	 * used for writing bytes.
	 * 
	 * <p>
	 * If this stream has an associated channel then the channel is closed as
	 * well.
	 * 
	 * @exception IOException
	 *                if an I/O error occurs.
	 */
	public void close() throws IOException {
		if (temp.length() < file.length()) {
			// only partially overwrote the existing file
			// download it
			LocalFile temp2 = (LocalFile) LocalFile.createTempFile(""
					+ (int) (Math.random() * 999), ""
					+ new java.util.Date().getTime());
			file.copyTo(temp2);

			// should create a new DataSource and append the existing files
			// during the upload, that'd be a little faster at least...
			out = FileFactory.newFileOutputStream(temp2);
			InputStream in = FileFactory.newFileInputStream(temp);

			// write the new bytes into the front of the old file.
			byte[] buffer = new byte[65535];
			int read;
			while ((read = in.read(buffer)) > 0) {
				out.write(buffer, 0, read);
			}
			// make sure it uploads the complete file
			temp = temp2;
		}
		try {
			((FTPFileSystem) file.getFileSystem()).getFTPClient().put(
					temp.getFile(), file.getPath(), false);
		} catch (FTPException e) {
			IOException x = new IOException();
			x.initCause(e);
			throw x;
		}
	}
}
