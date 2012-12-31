//  Copyright (c) 2005, Regents of the University of California
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//    * Redistributions of source code must retain the above copyright notice,
//  this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
//    * Neither the name of the University of California, San Diego (UCSD) nor
//  the names of its contributors may be used to endorse or promote products
//  derived from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//  FILE
//  GeneralRandomAccessFile.java  -  edu.sdsc.grid.io.GeneralRandomAccessFile
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-java.io.OuputStream
//          |
//          +-edu.sdsc.grid.io.GeneralFileOutputStream
//              |
//              +-edu.sdsc.grid.io.RemoteFileOutputStream
//                  |
//                  +-edu.sdsc.grid.io.SRBFileOutputStream
//
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.srb;

import java.io.IOException;

import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.RemoteFileOutputStream;

/**
 * A SRBFileOutputStream writes bytes to a file in a file system. What files are
 * available depends on the host environment.
 * <P>
 * SRBFileOutputStream is meant for writing streams of raw bytes such as image
 * data.
 * 
 * @author Lucas Gilbert
 * @since JARGON1.4
 */
public class SRBFileOutputStream extends RemoteFileOutputStream {
	// ----------------------------------------------------------------------
	// Constants
	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	// Fields
	// ----------------------------------------------------------------------
	/**
	 * Holds the server connection used by this stream.
	 */
	protected SRBFileSystem fileSystem;

	// ----------------------------------------------------------------------
	// Constructors and Destructors
	// ----------------------------------------------------------------------
	/**
	 * Creates a <code>FileOuputStream</code> by opening a connection to an
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
	public SRBFileOutputStream(final SRBFileSystem fileSystem, final String name)
			throws IOException {
		super(fileSystem, name);

		this.fileSystem = fileSystem;
	}

	/**
	 * Creates a <code>FileInputStream</code> by opening a connection to an
	 * actual file, the file named by the <code>File</code> object
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
	public SRBFileOutputStream(final SRBFile file) throws IOException {
		super(file);
		fileSystem = (SRBFileSystem) file.getFileSystem();
	}

	// ----------------------------------------------------------------------
	// Methods
	// ----------------------------------------------------------------------
	/**
	 * Opens the given file for use by this stream.
	 * 
	 * @exception IOException
	 *                if an I/O error occurs.
	 */
	@Override
	protected void open(final GeneralFile file) throws IOException {
		if (!file.exists()) {
			// files don't appear on SRB until they they have been closed
			// for the first time. This was causing some confusion.
			file.createNewFile();
		}

		fd = ((SRBFileSystem) file.getFileSystem()).srbObjOpen(file.getName(),
				SRBRandomAccessFile.O_RDWR, file.getParent());
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
	@Override
	public void write(final byte buffer[], final int offset, final int length)
			throws IOException {
		byte b[] = new byte[length];
		System.arraycopy(buffer, offset, b, 0, length);
		fileSystem.srbObjWrite(fd, b, length);
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
	@Override
	public void close() throws IOException {
		if (fileSystem != null) {
			fileSystem.srbObjClose(fd);
			fileSystem = null;
		}
	}

	/**
	 * Returns the file descriptor associated with this stream.
	 * 
	 * @return the <code>FileDescriptor</code> object that represents the
	 *         connection to the file in the file system being used by this
	 *         <code>FileOutputStream</code> object.
	 * 
	 * @exception IOException
	 *                if an I/O error occurs.
	 * @see java.io.FileDescriptor
	 */
	/*
	 * public final FileDescriptor getFD() throws IOException { //Why is it
	 * final?!! }
	 */
	/**
	 * Returns the unique {@link java.nio.channels.FileChannel FileChannel}
	 * object associated with this file output stream. </p>
	 * 
	 * <p>
	 * The initial {@link java.nio.channels.FileChannel#position()
	 * </code>position<code>} of the returned channel will be equal to the
	 * number of bytes written to the file so far unless this stream is in
	 * append mode, in which case it will be equal to the size of the file.
	 * Writing bytes to this stream will increment the channel's position
	 * accordingly. Changing the channel's position, either explicitly or by
	 * writing, will change this stream's file position.
	 * 
	 * @return the file channel associated with this file output stream
	 */
	/*
	 * public FileChannel getChannel() { synchronized (this) { if (channel ==
	 * null) channel = FileChannelImpl.open(fd, false, true, this, append);
	 * return channel; } }
	 */
}
