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
//  SRBRandomAccessFile.java  -  edu.sdsc.grid.io.srb.SRBRandomAccessFile
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-.GeneralRandomAccessFile
//          |
//          +-.RemoteRandomAccessFile
//              |
//              +-.SRBRandomAccessFile
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.srb;

import edu.sdsc.grid.io.*;

import java.io.IOException;
import java.io.FileNotFoundException;

/**
 * Instances of this class support both reading and writing to a SRB random
 * access file.
 *<P>
 * This class behaves exactly as described in
 * {@link edu.sdsc.grid.io.GeneralRandomAccessFile}
 *<P>
 * A random access file behaves like a large array of bytes stored in the file
 * system. There is a kind of cursor, or index into the implied array, called
 * the <em>file pointer</em>; input operations read bytes starting at the file
 * pointer and advance the file pointer past the bytes read. If the random
 * access file is created in read/write mode, then output operations are also
 * available; output operations write bytes starting at the file pointer and
 * advance the file pointer past the bytes written. Output operations that write
 * past the current end of the implied array cause the array to be extended. The
 * file pointer can be read by the <code>getFilePointer</code> method and set by
 * the <code>seek</code> method.
 * <p>
 * It is generally true of all the reading routines in this class that if
 * end-of-file is reached before the desired number of bytes has been read, an
 * <code>EOFException</code> (which is a kind of <code>IOException</code>) is
 * thrown. If any byte cannot be read for any reason other than end-of-file, an
 * <code>IOException</code> other than <code>EOFException</code> is thrown. In
 * particular, an <code>IOException</code> may be thrown if the stream has been
 * closed.
 * 
 * @author Lucas Gilbert
 * @since JARGON1.0
 */
public class SRBRandomAccessFile extends RemoteRandomAccessFile {
	// ----------------------------------------------------------------------
	// Constants
	// ----------------------------------------------------------------------
	// SRB open flag values as ints
	static final int O_RDONLY = 0; // 00 octal
	static final int O_WRONLY = 1; // 01 octal
	static final int O_RDWR = 2; // 02 octal
	static final int O_APPEND = 1024; // 02000 octal
	static final int O_SYNC = 4096; // 010000 octal
	static final int O_DSYNC = O_SYNC; // Doesn't exist?

	// ----------------------------------------------------------------------
	// Fields
	// ----------------------------------------------------------------------
	/**
	 * Position of read/write pointer in file.
	 */
	long filePointer = 0;

	/**
	 * Holds the connection through which data is sent.
	 */
	SRBFileSystem fileSystem;

	// ----------------------------------------------------------------------
	// Constructors and Destructors
	// ----------------------------------------------------------------------
	/**
	 * Creates a random access file stream to read from, and optionally to write
	 * to, a file with the specified name. A new file descriptor is obtained
	 * from the SRB which represents the connection to the file.
	 *<P>
	 * The <tt>mode</tt> argument specifies the access mode with which the file
	 * is to be opened. The permitted values and their meanings are as specified
	 * for the <a href="#mode"><tt>GeneralRandomAccessFile(File,String)</tt></a>
	 * constructor.
	 *<P>
	 * On construction a check is made to see if read access to the file is
	 * allowed. If the mode allows writing, write access to the file is also
	 * checked.
	 * 
	 * @param filePath
	 *            the SRB file path
	 * @param mode
	 *            the access <a href="#mode">mode</a>
	 * @throws IllegalArgumentException
	 *             if the mode argument is not equal to one of <tt>"r"</tt>,
	 *             <tt>"rw"</tt>, <tt>"rws"</tt>, or <tt>"rwd"</tt>
	 * @throws IOException
	 *             If an I/O error occurs
	 * @throws FileNotFoundException
	 *             If the file exists but is a directory rather than a regular
	 *             file, or cannot be opened or created for any other reason
	 * @throws SecurityException
	 *             If denied read access to the file or the mode is "rw" and
	 *             denied write access to the file.
	 */
	public SRBRandomAccessFile(SRBFileSystem srbFileSystem, String filePath,
			String mode) throws IllegalArgumentException,
			FileNotFoundException, SecurityException, IOException {
		this(new SRBFile(srbFileSystem, filePath), mode);
	}

	/**
	 * Creates a random access file stream to read from, and optionally to write
	 * to, a file with the specified name. A new file descriptor is obtained
	 * from the SRB which represents the connection to the file.
	 * 
	 * <a name="mode">
	 * <p>
	 * The <tt>mode</tt> argument specifies the access mode in which the file is
	 * to be opened. The permitted values and their meanings are:
	 * 
	 * <blockquote>
	 * <table>
	 * <tr>
	 * <td valign="top"><tt>"r"</tt></td>
	 * <td>Open for reading only. Invoking any of the <tt>write</tt> methods of
	 * the resulting object will cause an {@link java.io.IOException} to be
	 * thrown.</td>
	 * </tr>
	 * <tr>
	 * <td valign="top"><tt>"rw"</tt></td>
	 * <td>Open for reading and writing. If the file does not already exist then
	 * an attempt will be made to create it.</td>
	 * </tr>
	 * <tr>
	 * <td valign="top"><tt>"rws"</tt></td>
	 * <td>Open for reading and writing, as with <tt>"rw"</tt>, and also require
	 * that every update to the file's content or metadata be written
	 * synchronously to the underlying storage device.</td>
	 * </tr>
	 * <tr>
	 * <td valign="top"><tt>"rwd"&nbsp;&nbsp;</tt></td>
	 * <td>Open for reading and writing, as with <tt>"rw"</tt>, and also require
	 * that every update to the file's content be written synchronously to the
	 * underlying storage device.</td>
	 * </tr>
	 * </table>
	 * </blockquote>
	 * 
	 * The <tt>"rws"</tt> and <tt>"rwd"</tt> modes work much like the
	 * {@link java.nio.channels.FileChannel#force(boolean) force(boolean)}
	 * method of the {@link java.nio.channels.FileChannel} class, passing
	 * arguments of <tt>true</tt> and <tt>false</tt>, respectively, except that
	 * they always apply to every I/O operation and are therefore often more
	 * efficient. If the file resides on a local storage device then when an
	 * invocation of a method of this class returns it is guaranteed that all
	 * changes made to the file by that invocation will have been written to
	 * that device. This is useful for ensuring that critical information is not
	 * lost in the event of a system crash. If the file does not reside on a
	 * local device then no such guarantee is made.
	 * 
	 * <p>
	 * The <tt>"rwd"</tt> mode can be used to reduce the number of I/O
	 * operations performed. Using <tt>"rwd"</tt> only requires updates to the
	 * file's content to be written to storage; using <tt>"rws"</tt> requires
	 * updates to both the file's content and its metadata to be written, which
	 * generally requires at least one more low-level I/O operation.
	 *<P>
	 * On construction a check is made to see if read access to the file is
	 * allowed. If the mode allows writing, write access to the file is also
	 * checked.
	 * 
	 * @param file
	 *            the file object
	 * @param mode
	 *            the access mode, as described <a href="#mode">above</a>
	 * @throws IOException
	 *             If an I/O error occurs
	 * @throws IllegalArgumentException
	 *             if the mode argument is not equal to one of <tt>"r"</tt>,
	 *             <tt>"rw"</tt>, <tt>"rws"</tt>, or <tt>"rwd"</tt>
	 * @throws FileNotFoundException
	 *             If the file exists but is a directory rather than a regular
	 *             file, or cannot be opened or created for any other reason
	 * @throws SecurityException
	 *             If denied read access to the file or the mode is "rw" and
	 *             denied write access to the file.
	 */
	public SRBRandomAccessFile(SRBFile file, String mode)
			throws IllegalArgumentException, FileNotFoundException,
			SecurityException, IOException {
		super(file, mode);

		swapNeeded = false;

		setFileSystem(file.getFileSystem());
	}

	/**
	 * Finalizes the object by explicitly letting go of each of its internally
	 * held values.
	 */
	protected void finalize() throws Throwable {
		super.finalize();
		close();
	}

	// ----------------------------------------------------------------------
	// Setters and Getters
	// ----------------------------------------------------------------------
	/**
	 * Sets the boolean rw value according to the mode and checks that such
	 * permissions are available.
	 *<P>
	 * "r" would allow for read-only access. "rw" would allow for read-write
	 * access. Case-insensitive.
	 * 
	 * @throws IllegalArgumentException
	 *             the mode is invalid.
	 * @throws SecurityException
	 *             if the permissions are wrong.
	 */
	/*
	 * protected void rwCheck( GeneralFile file, String mode ) throws
	 * IllegalArgumentException, SecurityException { mode = mode.toLowerCase();
	 * 
	 * if (mode.equals("r")) { rw = 0; } else if (mode.equals("rw")) { rw = 1; }
	 * else if (mode.equals("rws")) { rw = 2; } else if (mode.equals("rwd")) {
	 * rw = 3; } else { throw new IllegalArgumentException("Illegal mode \"" +
	 * mode + "\" must be one of \"r\", \"rw\", \"rws\", or \"rwd\""); }
	 * 
	 * 
	 * if (!((SRBFile) file).canRead("isFile")) throw new SecurityException(
	 * "Wrong permissions to access this file.");
	 * 
	 * if (rw > 0) { if (!((SRBFile) file).canWrite("isFile")) throw new
	 * SecurityException( "Wrong permissions to access this file."); } }
	 */

	/**
	 * Opens this file. The file is opened in read-write mode if writeable is
	 * true, else the file is opened as read-only. If the <code>name</code>
	 * refers to a directory, an IOException is thrown.
	 * 
	 * @param file
	 *            the file to open
	 * @throws IOException
	 *             If an I/O error occurs
	 */
	protected void open(GeneralFile file) throws FileNotFoundException,
			SecurityException, IOException {
		file.createNewFile();
		rwCheck(file, mode);

		// super insures file.isFile()
		if (rw == 0) {
			fd = ((SRBFileSystem) file.getFileSystem()).srbObjOpen(file
					.getName(), O_RDONLY, file.getParent());
		} else if (rw == 1) {
			fd = ((SRBFileSystem) file.getFileSystem()).srbObjOpen(file
					.getName(), O_RDWR, file.getParent());
		} else if (rw == 2) {
			fd = ((SRBFileSystem) file.getFileSystem()).srbObjOpen(file
					.getName(), O_SYNC, file.getParent());
		} else if (rw == 3) {
			fd = ((SRBFileSystem) file.getFileSystem()).srbObjOpen(file
					.getName(), O_DSYNC, file.getParent());
		}
	}

	/**
	 * Sets the SRB server used of this SRBRandomAccessFile object.
	 * 
	 * @param fleServer
	 *            The SRB server to be used.
	 * @throws IllegalArgumentException
	 *             - if the argument is null.
	 * @throws ClassCastException
	 *             - if the argument is not a SRBFileSystem object.
	 */
	protected void setFileSystem(GeneralFileSystem fileSystem)
			throws IllegalArgumentException, ClassCastException {
		if (fileSystem == null)
			throw new IllegalArgumentException(
					"Illegal fileSystem, cannot be null");

		this.fileSystem = (SRBFileSystem) fileSystem;
	}

	/**
	 * Returns the srb file system object.
	 * 
	 * @throws NullPointerException
	 *             if fileSystem is null.
	 * @return RemoteFileSystem
	 */
	public GeneralFileSystem getFileSystem() {
		if (fileSystem != null)
			return fileSystem;

		throw new NullPointerException();
	}

	// ----------------------------------------------------------------------
	// Read/write
	// ----------------------------------------------------------------------
	/**
	 * Reads a byte of data from this file. The byte is returned as an integer
	 * in the range 0 to 255 (<code>0x00-0x0ff</code>). This method blocks if no
	 * input is yet available.
	 * <p>
	 * Although <code>SRBRandomAccessFile</code> is not a subclass of
	 * <code>InputStream</code>, this method behaves in exactly the same way as
	 * java.io.InputStream.read().
	 * 
	 * @return the next byte of data, or <code>-1</code> if the end of the file
	 *         has been reached.
	 * @throws IOException
	 *             if an I/O error occurs. Not thrown if end-of-file has been
	 *             reached.
	 */
	public int read() throws IOException {
		byte buffer[] = fileSystem.srbObjRead(fd, 1);
		if (buffer != null) {
			filePointer += buffer.length;

			return buffer[0];
		}
		return -1;
	}

	/**
	 * Reads a sub array as a sequence of bytes.
	 * 
	 * @param buffer
	 *            the buffer into which the data is read.
	 * @param offset
	 *            the start offset in the data
	 * @param len
	 *            the maximum number of bytes read.
	 * @throws IOException
	 *             If an I/O error has occurred.
	 */
	protected int readBytes(byte buffer[], int offset, int len)
			throws IOException {
		byte b[] = null;

		b = fileSystem.srbObjRead(fd, len);
		if (b != null) {
			System.arraycopy(b, 0, buffer, offset, b.length);
			filePointer += b.length;

			return b.length;
		}

		return 0;
	}

	/**
	 * Writes a sub array as a sequence of bytes.
	 * 
	 * @param buffer
	 *            the data to be written
	 * @param offset
	 *            the start offset in the data
	 * @param len
	 *            the number of bytes that are written
	 * @throws IOException
	 *             If an I/O error has occurred.
	 */
	protected void writeBytes(byte buffer[], int offset, int len)
			throws IOException {
		byte b[] = new byte[len];
		System.arraycopy(buffer, offset, b, 0, len);
		filePointer += fileSystem.srbObjWrite(fd, b, len);
	}

	// ----------------------------------------------------------------------
	// Random access
	// ----------------------------------------------------------------------
	/**
	 * Returns the current offset in this file.
	 * 
	 * @return the offset from the beginning of the file, in bytes, at which the
	 *         next read or write occurs.
	 * @throws IOException
	 *             if an I/O error occurs.
	 */
	public long getFilePointer() throws IOException {
		return filePointer;
	}

	/**
	 * Sets the file-pointer offset at which the next read or write occurs. The
	 * offset may be set beyond the end of the file. Setting the offset beyond
	 * the end of the file does not change the file length. The file length will
	 * change only by writing after the offset has been set beyond the end of
	 * the file.
	 * 
	 * @param pos
	 *            the offset position, measured in bytes from the at which to
	 *            set the file pointer.
	 * @param origin
	 *            a Sets offset for the beginning of the seek.<br>
	 *            SEEK_START - sets the offset from the beginning of the file.
	 *            SEEK_CURRENT - sets the offset from the current position of
	 *            the filePointer.<br>
	 *            SEEK_END - sets the offset from the end of the file.<br>
	 * 
	 * @throws IOException
	 *             if <code>pos</code> is less than <code>0</code> or if an I/O
	 *             error occurs.
	 */
	public void seek(long position, int origin) throws IOException {
		if (position < 0) {
			throw new IllegalArgumentException();
		}

		fileSystem.srbObjSeek(fd, position, origin);
		filePointer = position;
	}

	/**
	 * Returns the length of this file.
	 * 
	 * @return the length of this file, measured in bytes.
	 * @throws IOException
	 *             if an I/O error occurs.
	 */
	public long length() throws IOException {
		MetaDataRecordList[] rl = null;
		MetaDataCondition[] conditions = {
				MetaDataSet.newCondition(SRBMetaDataSet.DIRECTORY_NAME,
						MetaDataCondition.EQUAL, file.getParent()),
				MetaDataSet.newCondition(SRBMetaDataSet.FILE_NAME,
						MetaDataCondition.EQUAL, file.getName()) };
		MetaDataSelect[] selects = { MetaDataSet
				.newSelection(SRBMetaDataSet.SIZE) };

		try {
			rl = fileSystem.query(conditions, selects, 3);

			if (rl != null)
				return Long.parseLong(rl[0].getValue(SRBMetaDataSet.SIZE)
						.toString());
		} catch (IOException e) {

		}

		return 0;
	}

	/**
	 * Sets the length of this file.
	 * 
	 * <p>
	 * Truncating a file on the SRB is not yet supported, so an
	 * UnsupportedOperationException will be thrown. (If the present length of
	 * the file as returned by the <code>length</code> method is greater than
	 * the <code>newLength</code> argument then the file will be truncated. In
	 * this case, if the file offset as returned by the
	 * <code>getFilePointer</code> method is greater then <code>newLength</code>
	 * then after this method returns the offset will be equal to
	 * <code>newLength</code>.)
	 * 
	 * <p>
	 * If the present length of the file as returned by the <code>length</code>
	 * method is smaller than the <code>newLength</code> argument then the file
	 * will be extended. In this case, the contents of the extended portion of
	 * the file are not defined.
	 * 
	 * @param newLength
	 *            The desired length of the file
	 * @throws IOException
	 *             If an I/O error occurs
	 * @throws UnsupportedOperationException
	 *             on truncate
	 */
	public void setLength(long newLength) throws IOException {

		long length = length();

		if (newLength > length) {
			seek(newLength - 1);
			write(0);
		} else if (newLength < length) {
			// The SRB does not currently support
			throw new UnsupportedOperationException();
		}
	}

	/**
	 * Closes this random access file stream and releases any system resources
	 * associated with the stream. A closed random access file cannot perform
	 * input or output operations and cannot be reopened.
	 * 
	 * @throws IOException
	 *             if an I/O error occurs.
	 */
	public void close() throws IOException {
		if (fileSystem != null) {
			fileSystem.srbObjClose(fd);
			fileSystem = null;
		}
		if (fileFormat != null)
			fileFormat = null;
	}
}
