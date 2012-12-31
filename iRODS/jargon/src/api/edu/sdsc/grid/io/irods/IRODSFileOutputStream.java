//  Copyright (c) 2008, Regents of the University of California
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
//                  +-edu.sdsc.grid.io.IRODSFileOutputStream
//
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.irods;

import java.io.IOException;

import org.irods.jargon.core.accessobject.FileCatalogObjectAO;
import org.irods.jargon.core.accessobject.IRODSAccessObjectFactory;
import org.irods.jargon.core.accessobject.IRODSAccessObjectFactoryImpl;
import org.irods.jargon.core.exception.JargonException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.RemoteFileOutputStream;

/**
 * A IRODSFileOutputStream writes bytes to a file in a file system. What files
 * are available depends on the host environment.
 * <P>
 * IRODSFileOutputStream is meant for writing streams of raw bytes such as image
 * data.
 * 
 * @author Lucas Gilbert
 * @since JARGON2.0
 */
public final class IRODSFileOutputStream extends RemoteFileOutputStream {

	public static final Logger log = LoggerFactory
			.getLogger(IRODSFileOutputStream.class);
	/**
	 * Holds the server connection used by this stream.
	 */
	protected IRODSFileSystem fileSystem;

	/**
	 * Holds an irodsFileSystem object when the input stream has been rerouted
	 * to a new irods server
	 */
	private IRODSFileSystem reroutedFileSystem = null;

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
	public IRODSFileOutputStream(final IRODSFileSystem fileSystem,
			final String name) throws IOException {
		this(fileSystem, name, "");
	}

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
	 * @param resourceName
	 *            the target resource to
	 * @exception IOException
	 *                if the file does not exist, is a directory rather than a
	 *                regular file, or for some other reason cannot be opened
	 *                for reading.
	 */
	public IRODSFileOutputStream(final IRODSFileSystem fileSystem,
			final String name, final String resourceName) throws IOException {
		super(fileSystem, name);

		this.fileSystem = fileSystem;

		try {
			this.lookForReroutingOfConnection(name, resourceName);
		} catch (JargonException e) {
			throw new IOException(e.getMessage());
		}
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
	public IRODSFileOutputStream(final IRODSFile file) throws IOException {
		super(file);
		fileSystem = (IRODSFileSystem) file.getFileSystem();
		try {
			this.lookForReroutingOfConnection(file.getAbsolutePath(),
					file.getResource());
		} catch (JargonException e) {
			throw new IOException(e.getMessage());
		}
	}

	/**
	 * Finalizes the object by explicitly letting go of each of its internally
	 * held values.
	 */
	@Override
	protected void finalize() throws IOException {
		// calls close()
		super.finalize();

		if (fileSystem != null) {
			fileSystem = null;
		}
	}

	/**
	 * Opens the given file for use by this stream.
	 * 
	 * @exception IOException
	 *                if an I/O error occurs.
	 */
	@Override
	protected void open(final GeneralFile file) throws IOException {
		if (!file.exists()) {
			fd = ((IRODSFileSystem) file.getFileSystem()).commands.fileCreate(
					(IRODSFile) file, false, true);
		} else {
			fd = ((IRODSFileSystem) file.getFileSystem()).commands.fileOpen(
					(IRODSFile) file, false, true);
		}
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
		fileSystem.commands.fileWrite(fd, buffer, offset, length);
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
			fileSystem.commands.fileClose(fd);
			fileSystem = null;
		}

		// if there was a rerouted connection, shut down that IRODSFileSystem
		// entirely
		if (reroutedFileSystem != null) {
			log.info("shutting down rerouted file system");
			reroutedFileSystem.close();
		}
	}

	/**
	 * Method takes the iRODS absolute path for the stream file, and evaluates
	 * whether the connection should be rerouted to another server. This will
	 * switch out the connection used for the stream and open a new one.
	 * 
	 * @param irodsAbsolutePath
	 * @param resourceName
	 * @throws JargonException
	 */
	private void lookForReroutingOfConnection(final String irodsAbsolutePath,
			final String resourceName) throws JargonException {
		IRODSAccessObjectFactory irodsAccessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(fileSystem.commands);
		FileCatalogObjectAO fileCatalogObjectAO = irodsAccessObjectFactory
				.getFileCatalogObjectAO();
		IRODSFileSystem tempReroutedFileSystem = fileCatalogObjectAO
				.rerouteIrodsFileWhenIRODSIsSource(irodsAbsolutePath,
						resourceName);
		if (tempReroutedFileSystem == null) {
			log.info("no override of stream connection");
			return;
		}

		log.debug("connection will be rerouted, switch to the new connection and close the file that was opened, close old file...");
		try {
			/*
			 * note that close will look at reroutedFileSystem and close it,
			 * this is done so that when close is called by a client it will
			 * disconnect from the rerouted connection. The client will be
			 * unaware that the additional connection exists, and otherwise, an
			 * agent connection will be retained and then closed without
			 * disconnecting. There is a bit of a shuffle of the file system
			 * object in this class, it keeps references to the original, as
			 * well as the rerouted. For this reason, the close of the file
			 * descriptor needs to happen before the rerouted connection is
			 * assigned, or the close connection will just close the newly
			 * opened rerouted connection.
			 * 
			 * The timing is a bit clunky in this version of Jargon, but could
			 * not be avoided.
			 */
			close();
			fileSystem = tempReroutedFileSystem;
			reroutedFileSystem = tempReroutedFileSystem;
			log.debug("open file at resource server...");
			IRODSFile irodsFile = new IRODSFile(reroutedFileSystem,
					irodsAbsolutePath);
			irodsFile.setResource(resourceName);
			open(irodsFile);
		} catch (Exception e) {
			log.error("error rerouting stream connection", e);
			throw new JargonException(e);
		}
		log.info("rerouting setup complete for stream");
	}

}
