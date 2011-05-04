//
//  Copyright (c) 2008  San Diego Supercomputer Center (SDSC),
//  University of California, San Diego (UCSD), San Diego, CA, USA.
//
//  Users and possessors of this source code are hereby granted a
//  nonexclusive, royalty-free copyright and design patent license
//  to use this code in individual software.  License is not granted
//  for commercial resale, in whole or in part, without prior written
//  permission from SDSC/UCSD.  This source is provided "AS IS"
//  without express or implied warranty of any kind.
//
//
//  FILE
//  IRODSFileInputStream.java  -  edu.sdsc.grid.io.irods.IRODSFileInputStream
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-java.io.InputStream
//          |
//          +-edu.sdsc.grid.io.GeneralFileInputStream
//              |
//              +-.RemoteFileInputStream
//                  |
//                  +-.irods.IRODSFileInputStream
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
import edu.sdsc.grid.io.GeneralFileSystem;
import edu.sdsc.grid.io.GeneralMetaData;
import edu.sdsc.grid.io.GeneralRandomAccessFile;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.RemoteFileInputStream;
import edu.sdsc.grid.io.StandardMetaData;

/**
 * A IRODSFileInputStream obtains input bytes from a file in a IRODS file
 * system. What files are available depends on the host environment.
 * <P>
 * IRODSFileInputStream is meant for reading streams of raw bytes such as image
 * data.
 * <P>
 * 
 * @author Lucas Gilbert
 * @since JARGON2.0
 */
public class IRODSFileInputStream extends RemoteFileInputStream {

	public static final Logger log = LoggerFactory
			.getLogger(IRODSFileInputStream.class);

	/**
	 * Position of read/write pointer in file.
	 */
	long filePointer = 0;

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
	 * Holds the server connection used by this stream.
	 */
	IRODSFile file;

	/**
	 * Creates a <code>IRODSFileInputStream</code> by opening a connection to an
	 * actual file, the file named by the path name <code>name</code> in the
	 * file system. A new <code>FileDescriptor</code> object is created to
	 * represent this file connection.
	 * <p>
	 * First, if there is a security manager, its <code>checkRead</code> method
	 * is called with the <code>name</code> argument as its argument.
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
	public IRODSFileInputStream(final IRODSFileSystem fileSystem,
			final String name) throws IOException {
		super(fileSystem, name);

		this.fileSystem = fileSystem;
		try {
			this.lookForReroutingOfConnection(name, "");
		} catch (JargonException e) {
			throw new IOException(e);
		}
	}

	/**
	 * Creates a <code>IRODSFileInputStream</code> by opening a connection to an
	 * actual file, the file named by the <code>File</code> object
	 * <code>file</code> in the file system. A new <code>FileDescriptor</code>
	 * object is created to represent this file connection.
	 * <p>
	 * First, if there is a security manager, its <code>checkRead</code> method
	 * is called with the path represented by the <code>file</code> argument as
	 * its argument.
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
	public IRODSFileInputStream(final IRODSFile file) throws IOException {
		super(file);
		fileSystem = (IRODSFileSystem) file.getFileSystem();
		try {
			this.lookForReroutingOfConnection(file.getAbsolutePath(), file.getResource());
		} catch (JargonException e) {
			throw new IOException(e);
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
		if (file != null) {
			file = null;
		}
	}

	/**
	 * Sets the IRODS server used of this IRODSRandomAccessFile object.
	 * 
	 * @param fleServer
	 *            The IRODS server to be used.
	 * @throws IllegalArgumentException
	 *             - if the argument is null.
	 * @throws ClassCastException
	 *             - if the argument is not a IRODSFileSystem object.
	 */
	protected void setFileSystem(final GeneralFileSystem fileSystem)
			throws IllegalArgumentException, ClassCastException {
		if (fileSystem == null) {
			throw new IllegalArgumentException(
					"Illegal fileSystem, cannot be null");
		}

		this.fileSystem = (IRODSFileSystem) fileSystem;
	}

	/**
	 * Returns the irods file system object.
	 * 
	 * @throws NullPointerException
	 *             if fileSystem is null.
	 * @return RemoteFileSystem
	 */
	public GeneralFileSystem getFileSystem() {
		return fileSystem;
	}

	/**
	 * Opens the given file for use by this stream.
	 * 
	 * @exception IOException
	 *                if an I/O error occurs.
	 */
	@Override
	protected void open(final GeneralFile file) throws IOException {
		this.file = (IRODSFile) file;
		fd = ((IRODSFileSystem) file.getFileSystem()).commands.fileOpen(
				(IRODSFile) file, true, false);
	}

	/**
	 * Note: Use of this method is inadvisable due to the long delays that can
	 * occur with network communcations. Reading even a few bytes in this manner
	 * could cause noticeable slowdowns.
	 * 
	 * Reads the next byte of data from the input stream. The value byte is
	 * returned as an <code>int</code> in the range <code>0</code> to
	 * <code>255</code>. If no byte is available because the end of the stream
	 * has been reached, the value <code>-1</code> is returned. This method
	 * blocks until input data is available, the end of the stream is detected,
	 * or an exception is thrown.
	 * 
	 * @return the next byte of data, or <code>-1</code> if the end of the
	 *         stream is reached.
	 * @exception IOException
	 *                if an I/O error occurs.
	 */
	@Override
	public int read() throws IOException {
		try {
			byte buffer[] = new byte[1];
			int temp = fileSystem.commands.fileRead(fd, buffer, 0, 1);
			if (buffer != null) {
				if (temp < 0) {
					return -1;
				}
				// if temp = 0 is an error?
				filePointer += temp; // 0 or 1
				return (buffer[0] & 0xFF);
			}
		} catch (IRODSException e) {
			// -1 just means EOF
			if (e.getType() != -1) {
				throw e;
			}
		}
		return -1;
	}

	/**
	 * Reads up to <code>len</code> bytes of data from the input stream into an
	 * array of bytes. An attempt is made to read as many as <code>len</code>
	 * bytes, but a smaller number may be read, possibly zero. The number of
	 * bytes actually read is returned as an integer.
	 * 
	 * <p>
	 * This method blocks until input data is available, end of file is
	 * detected, or an exception is thrown.
	 * 
	 * <p>
	 * If <code>b</code> is <code>null</code>, a
	 * <code>NullPointerException</code> is thrown.
	 * 
	 * <p>
	 * If <code>off</code> is negative, or <code>len</code> is negative, or
	 * <code>off+len</code> is greater than the length of the array
	 * <code>b</code>, then an <code>IndexOutOfBoundsException</code> is thrown.
	 * 
	 * <p>
	 * If <code>len</code> is zero, then no bytes are read and <code>0</code> is
	 * returned; otherwise, there is an attempt to read at least one byte. If no
	 * byte is available because the stream is at end of file, the value
	 * <code>-1</code> is returned; otherwise, at least one byte is read and
	 * stored into <code>b</code>.
	 * 
	 * <p>
	 * The first byte read is stored into element <code>b[off]</code>, the next
	 * one into <code>b[off+1]</code>, and so on. The number of bytes read is,
	 * at most, equal to <code>len</code>. Let <i>k</i> be the number of bytes
	 * actually read; these bytes will be stored in elements <code>b[off]</code>
	 * through <code>b[off+</code><i>k</i><code>-1]</code>, leaving elements
	 * <code>b[off+</code><i>k</i><code>]</code> through
	 * <code>b[off+len-1]</code> unaffected.
	 * 
	 * <p>
	 * In every case, elements <code>b[0]</code> through <code>b[off]</code> and
	 * elements <code>b[off+len]</code> through <code>b[b.length-1]</code> are
	 * unaffected.
	 * 
	 * <p>
	 * If the first byte cannot be read for any reason other than end of file,
	 * then an <code>IOException</code> is thrown. In particular, an
	 * <code>IOException</code> is thrown if the input stream has been closed.
	 * 
	 * <p>
	 * The <code>read(b,</code> <code>off,</code> <code>len)</code> method for
	 * class <code>InputStream</code> simply calls the method
	 * <code>read()</code> repeatedly. If the first such call results in an
	 * <code>IOException</code>, that exception is returned from the call to the
	 * <code>read(b,</code> <code>off,</code> <code>len)</code> method. If any
	 * subsequent call to <code>read()</code> results in a
	 * <code>IOException</code>, the exception is caught and treated as if it
	 * were end of file; the bytes read up to that point are stored into
	 * <code>b</code> and the number of bytes read before the exception occurred
	 * is returned. Subclasses are encouraged to provide a more efficient
	 * implementation of this method.
	 * 
	 * @param b
	 *            the buffer into which the data is read.
	 * @param off
	 *            the start offset in array <code>b</code> at which the data is
	 *            written.
	 * @param len
	 *            the maximum number of bytes to read.
	 * @return the total number of bytes read into the buffer, or
	 *         <code>-1</code> if there is no more data because the end of the
	 *         stream has been reached.
	 * @exception IOException
	 *                if an I/O error occurs.
	 * @exception NullPointerException
	 *                if <code>b</code> is <code>null</code>.
	 * @see java.io.InputStream#read()
	 */
	@Override
	public int read(final byte b[], final int off, final int len)
			throws IOException {
		int temp = fileSystem.commands.fileRead(fd, b, off, len);
		if (temp > 0) {
			filePointer += temp;
		}
		return temp;
	}

	/**
	 * Skips over and discards <code>n</code> bytes of data from the input
	 * stream. The <code>skip</code> method may, for a variety of reasons, end
	 * up skipping over some smaller number of bytes, possibly <code>0</code>.
	 * The actual number of bytes skipped is returned.
	 * 
	 * @param n
	 *            the number of bytes to be skipped.
	 * @return the actual number of bytes skipped.
	 * @exception IOException
	 *                if an I/O error occurs.
	 */
	@Override
	public long skip(final long n) throws IOException {
		long length = available();
		if (length <= 0) {
			return 0;
		}

		if ((filePointer + n) < length) {
			fileSystem.commands.fileSeek(fd, n,
					GeneralRandomAccessFile.SEEK_CURRENT);
			filePointer += n;
			return n;
		} else {
			fileSystem.commands.fileSeek(fd, length,
					GeneralRandomAccessFile.SEEK_CURRENT);
			filePointer += length;
			return length;
		}
	}

	public long seek(final long n) throws IOException {
		long seekVal = fileSystem.commands.fileSeek(fd, n,
				GeneralRandomAccessFile.SEEK_CURRENT);
		filePointer = seekVal;
		return seekVal;

	}

	/**
	 * Returns the number of bytes that can be read from this file input stream
	 * without blocking.
	 * 
	 * @return the number of bytes that can be read from this file input stream
	 *         without blocking.
	 * @exception IOException
	 *                if an I/O error occurs.
	 */
	@Override
	public int available() throws IOException {
		MetaDataRecordList[] rl = null;
		MetaDataCondition[] conditions = {
				MetaDataSet.newCondition(StandardMetaData.DIRECTORY_NAME,
						MetaDataCondition.EQUAL, file.getParent()),
				MetaDataSet.newCondition(StandardMetaData.FILE_NAME,
						MetaDataCondition.EQUAL, file.getName()) };
		MetaDataSelect[] selects = { MetaDataSet
				.newSelection(GeneralMetaData.SIZE) };
		int available = 0;

		try {
			rl = fileSystem.query(conditions, selects, 3);

			if (rl != null) {
				// would use .getIntValue but it could be long...
				available = (int) (Long.parseLong(rl[0].getValue(
						GeneralMetaData.SIZE).toString()) - filePointer);
			}
		} catch (IOException e) {

		}

		if (available <= 0) {
			return 0;
		} else {
			return available;
		}
	}

	/**
	 * Closes this file input stream and releases any system resources
	 * associated with the stream.
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
		if (file != null) {
			file = null;
		}

		// if there was a rerouted connection, shut down that IRODSFileSystem
		// entirely
		if (reroutedFileSystem != null) {
			log.info("shutting down rerouted file system");
			reroutedFileSystem.close();
		}
	}

	@Override
	public String toString() {
		return "Input: " + file;
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
		IRODSFileSystem tempReroutedFileSystem = fileCatalogObjectAO.rerouteIrodsFileWhenIRODSIsSource(
				irodsAbsolutePath, resourceName);
		if (tempReroutedFileSystem == null) {
			log.info("no override of stream connection");
			return;
		}

		log.debug("connection will be rerouted, switch to the new connection and close the file that was opened, close old file...");
		try {
			/* note that close will look at reroutedFileSystem and close it, this is done so that when close is called by a client
			 * it will disconnect from the rerouted connection.  The client will be unaware that the additional connection exists, and otherwise,
			 * an agent connection will be retained and then closed without disconnecting.  There is a bit of a shuffle of the file system object
			 * in this class, it keeps references to the original, as well as the rerouted.  For this reason, the close of the file descriptor needs to happen
			 * before the rerouted connection is assigned, or the close connection will just close the newly opened rerouted connection.
			 * 
			 * The timing is a bit clunky in this version of Jargon, but could not be avoided.
			 */
			close();
			setFileSystem(tempReroutedFileSystem);
			reroutedFileSystem = tempReroutedFileSystem;
			log.debug("open file at resource server...");
			file = new IRODSFile(reroutedFileSystem, irodsAbsolutePath);
			file.setResource(resourceName);
			open(file);
		} catch (Exception e) {
			log.error("error rerouting stream connection", e);
			throw new JargonException(e);
		}
		log.info("rerouting setup complete for stream");
	}

}
