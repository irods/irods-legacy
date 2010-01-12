//	Copyright (c) 2008, Regents of the University of California
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
//	HTTPRandomAccessFile.java	-  edu.sdsc.grid.io.http.HTTPRandomAccessFile
//
//  CLASS HIERARCHY
//	java.lang.Object
//	    |
//	    +-.GeneralRandomAccessFile
//					|
//			    +-.HTTPRandomAccessFile
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.http;

import edu.sdsc.grid.io.*;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.File;
import java.io.RandomAccessFile;

import java.net.URLConnection;

/**
 * The class for random access to an http file. Included for completeness, as it
 * only works as read-only.
 * 
 * @see java.io.RandomAccessFile
 * @see edu.sdsc.grid.io.GeneralRandomAccessFile
 * @author Lucas Gilbert
 * @since Jargon2.0
 */
public class HTTPRandomAccessFile extends RemoteRandomAccessFile {
	private InputStream in;
	private OutputStream out;

	/**
	 * Position of read/write pointer in file.
	 */
	private long filePointer = 0;
	private long length;
	private HTTPFile httpFile;

	/**
	 * Creates a random access file stream to read from, and optionally to write
	 * to, the file specified by the {@link String} argument. A new file
	 * descriptor is obtained which represents the connection to the file.
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
	 */
	public HTTPRandomAccessFile(HTTPFileSystem fileSystem, String name,
			String mode) throws IOException {
		this(new HTTPFile(fileSystem, name), mode);
	}

	/**
	 * Creates a random access file stream to read from, and optionally to write
	 * to, the file specified by the {@link File} argument. A new file
	 * descriptor is obtained which represents the connection to the file.
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
	 */
	public HTTPRandomAccessFile(HTTPFile file, String mode) throws IOException {
		super(file, mode);
		httpFile = file;
	}

	protected void open(GeneralFile file) throws IOException {
		close();
		if (in == null) {
			// check first time
			rwCheck(file, mode);
		}

		URLConnection conn = ((HTTPFile) file).httpFileSystem.getNewConn();
		if (rw > 0) {
			conn.setDoOutput(true);
			out = conn.getOutputStream();
		}
		in = conn.getInputStream();
		// cache it for the URLConnection reasons
		length = file.length();
	}

	protected int readBytes(byte b[], int offset, int len) throws IOException {
		int read = in.read(b, offset, len);
		if (read > 0) {
			filePointer += read;
		}
		return read;
	}

	// Private method in wrapper, so call public.
	protected void writeBytes(byte b[], int offset, int len) throws IOException {
		out.write(b, offset, len);
	}

	/**
	 * Returns the current offset in this file.
	 * 
	 * @return the offset from the beginning of the file, in bytes, at which the
	 *         next read or write occurs.
	 * @throws IOException
	 *             if an I/O error occurs.
	 */
	public long getFilePointer() throws IOException {
		// keep an internal file pointer, for some fancy/inefficient business
		// since we aren't really quite random access...
		return filePointer;
	}

	/**
	 * Sets the file-pointer offset, measured from the beginning of this file,
	 * at which the next read or write occurs. The offset may be set beyond the
	 * end of the file. Setting the offset beyond the end of the file does not
	 * change the file length. The file length will change only by writing after
	 * the offset has been set beyond the end of the file.
	 * 
	 * @param pos
	 *            the offset position, measured in bytes from the beginning of
	 *            the file, at which to set the file pointer.
	 * @throws IOException
	 *             if <code>pos</code> is less than <code>0</code> or if an I/O
	 *             error occurs.
	 */
	public void seek(long position) throws IOException {
		seek(position, SEEK_CURRENT);
	}

	/**
	 * Sets the file-pointer offset at which the next read or write occurs. For
	 * writable files, the offset may be set beyond the end of the file. Setting
	 * the offset beyond the end of the file does not change the file length.
	 * The file length will change only by writing after the offset has been set
	 * beyond the end of the file. However, as of this release, all http files
	 * are read-only.
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
		switch (origin) {
		case 1:
			filePointer += in.skip(position + getFilePointer());
			break;
		case 2:
			// kind of pointless, always read only.
			filePointer += in.skip(position + length - getFilePointer());
			break;
		case 0:
		default:
			if (position < filePointer) {
				open(httpFile);
				in.skip(position);
			} else {
				in.skip(position - getFilePointer());
			}
			break;
		}
	}

	/**
	 * Returns the length of this file.
	 * 
	 * @return the length of this file, measured in bytes.
	 * @throws IOException
	 *             if an I/O error occurs.
	 */
	public long length() throws IOException {
		return length;
	}

	/**
	 * Unsupported
	 * 
	 * @param newLength
	 *            The desired length of the file
	 * @throws IOException
	 *             If an I/O error occurs
	 */
	public void setLength(long newLength) throws IOException {
		throw new UnsupportedOperationException();
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
		if (in != null) {
			in.close();
		}
		if (out != null) {
			out.close();
		}
	}
}
