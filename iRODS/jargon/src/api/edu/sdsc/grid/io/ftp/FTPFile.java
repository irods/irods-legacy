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
//	RemoteFile.java	-  edu.sdsc.grid.io.RemoteFile
//
//  CLASS HIERARCHY
//	java.lang.Object
//	    |
//	    +-.GeneralFile
//	            |
//	            +-.RemoteFile
//                 |
//                 +-.ftp.FTPFile
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.ftp;

import java.io.*;
import java.net.*;
import java.util.*;

import edu.sdsc.grid.io.local.*;
import edu.sdsc.grid.io.*;

import org.globus.ftp.*;
import org.globus.ftp.exception.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * An abstract representation of file and directory pathnames on a ftp server.
 *<P>
 * Shares many similarities with the java.io.File class: User interfaces and
 * operating systems use system-dependent pathname strings to name files and
 * directories. This class presents an abstract, system-independent view of
 * hierarchical pathnames. An abstract pathname has two components:
 *<P>
 * Instances of the FTPFile class are immutable; that is, once created, the
 * abstract pathname represented by a FTPFile object will never change.
 *<P>
 * 
 * @author Lucas Gilbert, San Diego Supercomputer Center
 * @see java.io.File
 * @see edu.sdsc.grid.io.GeneralFile
 */
public class FTPFile extends RemoteFile {

	/**
	 * Standard FTP path separator character represented as a string for
	 * convenience. This string contains a single character, namely
	 * <code>{@link #PATH_SEPARATOR_CHAR}</code>.
	 */
	public static final String PATH_SEPARATOR = "/";

	/**
	 * The FTP path separator character, '/'.
	 */
	public static final char PATH_SEPARATOR_CHAR = '/';

	/**
	 * The ftpClient which handles the ftp protocol level
	 */
	protected FTPClient ftpClient;

	private static Logger log = LoggerFactory.getLogger(FTPFile.class);

	/**
	 * Creates a new FTPFile instance by converting the given pathname string
	 * into an abstract pathname.
	 *<P>
	 * 
	 * @param fileSystem
	 *            The connection to the ftp server
	 * @param filePath
	 *            A pathname string
	 */
	public FTPFile(FTPFileSystem fileSystem, String filePath)
			throws IOException {
		this(fileSystem, "", filePath);
	}

	/**
	 * Creates a new FTPFile instance from a parent pathname string and a child
	 * pathname string.
	 *<P>
	 * If parent is null then the new FTPFile instance is created as if by
	 * invoking the single-argument FTPFile constructor on the given child
	 * pathname string.
	 *<P>
	 * Otherwise the parent pathname string is taken to denote a directory, and
	 * the child pathname string is taken to denote either a directory or a
	 * file. If the child pathname string is absolute then it is converted into
	 * a relative pathname in a system-dependent way. If parent is the empty
	 * string then the new RemoteFile instance is created by converting child
	 * into an abstract pathname and resolving the result against a
	 * system-dependent default directory. Otherwise each pathname string is
	 * converted into an abstract pathname and the child abstract pathname is
	 * resolved against the parent.
	 *<P>
	 * 
	 * @param fileSystem
	 *            The connection to the ftp server
	 * @param parent
	 *            The parent pathname string
	 * @param child
	 *            The child pathname string
	 */
	public FTPFile(FTPFileSystem fileSystem, String parent, String child)
			throws IOException {
		super(fileSystem, parent, child);

		ftpClient = fileSystem.getFTPClient();
	}

	/**
	 * Creates a new FTPFile instance from a parent abstract pathname and a
	 * child pathname string.
	 *<P>
	 * If parent is null then the new FTPFile instance is created as if by
	 * invoking the single-argument FTPFile constructor on the given child
	 * pathname string.
	 *<P>
	 * Otherwise the parent abstract pathname is taken to denote a directory,
	 * and the child pathname string is taken to denote either a directory or a
	 * file. If the child pathname string is absolute then it is converted into
	 * a relative pathname in a system-dependent way. If parent is the empty
	 * abstract pathname then the new RemoteFile instance is created by
	 * converting child into an abstract pathname and resolving the result
	 * against a system-dependent default directory. Otherwise each pathname
	 * string is converted into an abstract pathname and the child abstract
	 * pathname is resolved against the parent.
	 *<P>
	 * 
	 * @param parent
	 *            The parent abstract pathname
	 * @param child
	 *            The child pathname string
	 */
	public FTPFile(FTPFile parent, String child) throws IOException {
		this((FTPFileSystem) parent.getFileSystem(), parent.getParent(), child);
	}

	/**
	 * Creates a new FTPFile instance by converting the given file: URI into an
	 * abstract pathname.
	 *<P>
	 * FTP URI protocol:<br>
	 * ftp:// [ userName [ : password ] @ ] host [ : port ][ / path ]
	 *<P>
	 * example:<br>
	 * ftp://ftp@ftp.sdsc.edu:21/pub/testfile.txt
	 * 
	 * @param uri
	 *            An absolute, hierarchical URI using a supported scheme.
	 * @throws NullPointerException
	 *             if <code>uri</code> is <code>null</code>.
	 * @throws IllegalArgumentException
	 *             If the preconditions on the parameter do not hold.
	 */
	public FTPFile(URI uri) throws IOException, URISyntaxException {
		super(uri);

		if (uri.getScheme().equals("ftp")) {
			String userinfo = uri.getUserInfo();
			if (userinfo != null) {
				int index = userinfo.indexOf(":");
				if (index >= 0) {
					setFileSystem(new FTPFileSystem(new FTPAccount(uri
							.getHost(), uri.getPort(), userinfo.substring(0,
							index - 1), userinfo.substring(index + 1), uri
							.getPath())));
				} else {
					setFileSystem(new FTPFileSystem(new FTPAccount(uri
							.getHost(), uri.getPort(), userinfo, "", uri
							.getPath())));
				}
			} else {
				fileSystem = new FTPFileSystem(new FTPAccount(uri.getHost(),
						uri.getPort(), null, "", uri.getPath()));
			}

			setFileName(uri.getPath());
			ftpClient = ((FTPFileSystem) fileSystem).getFTPClient();
		} else {
			throw new URISyntaxException(uri.toString(), "Wrong URI scheme");
		}
	}

	// ----------------------------------------------------------------------
	// Setters and Getters
	// ----------------------------------------------------------------------
	/**
   * 
   */
	FTPClient getFTPClient() {
		return ftpClient;
	}

	// ----------------------------------------------------------------------
	// GeneralFile Methods
	// ----------------------------------------------------------------------
	// http://www.cogkit.org/release/4_1_2/api/jglobus/org/globus/ftp/FTPClient.html
	/**
	 * Not implemented
	 */
	@Override
	public void replicate(String resource) {
		throw new UnsupportedOperationException();
	}

	/**
	 * Not implemented
	 */
	@Override
	public String getResource() {
		throw new UnsupportedOperationException();
	}

	/**
	 * Copies this file to another file. This object is the source file. The
	 * destination file is given as the argument. If the destination file, does
	 * not exist a new one will be created. Otherwise the source file will be
	 * appended to the destination file. Directories will be copied recursively.
	 * 
	 * @param destinationFile
	 *            The file to receive the data.
	 * @throws NullPointerException
	 *             If file is null.
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	@Override
	public void copyTo(GeneralFile destinationFile, boolean forceOverwrite)
			throws IOException {
		if (destinationFile == null) {
			throw new NullPointerException();
		}

		if (isDirectory()) {
			// recursive copy
			GeneralFile[] fileList = listFiles();

			destinationFile.mkdir();
			if (fileList != null) {
				for (int i = 0; i < fileList.length; i++) {
					fileList[i].copyTo(FileFactory.newFile(
							destinationFile.getFileSystem(), destinationFile.getAbsolutePath(),
							fileList[i].getName()), forceOverwrite);
				}
			}
		} else {
			if (destinationFile.isDirectory()) {
				// change the destination from a directory to a file
				destinationFile = FileFactory.newFile(destinationFile, getName());
			}
			try {
				if (destinationFile instanceof LocalFile) {
					ftpClient.get(getPath(), ((LocalFile) destinationFile).getFile());
				} else if (destinationFile instanceof FTPFile) {
					ftpClient.transfer(getPath(), ((FTPFile) destinationFile)
							.getFTPClient(), destinationFile.getPath(), !forceOverwrite,
							null);
				} else {
					super.copyTo(destinationFile);
				}
			} catch (FTPException e) {
				IOException io = new IOException();
				io.initCause(e);
				throw io;
			}
		}
	}

	/**
	 * Copies this file to another file. This object is the source file. The
	 * destination file is given as the argument. If the destination file, does
	 * not exist a new one will be created. Otherwise the source file will be
	 * appended to the destination file. Directories will be copied recursively.
	 * 
	 * @param sourceFile
	 *            The file to receive the data.
	 * @throws NullPointerException
	 *             If file is null.
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	@Override
	public void copyFrom(GeneralFile sourceFile, boolean forceOverwrite)
			throws IOException {
		if (sourceFile == null) {
			throw new NullPointerException();
		}

		if (sourceFile.isDirectory()) {
			// recursive copy
			GeneralFile[] fileList = sourceFile.listFiles();

			mkdir();
			if (fileList != null) {
				for (int i = 0; i < fileList.length; i++) {
					FileFactory.newFile(this, fileList[i].getName()).copyFrom(
							fileList[i], forceOverwrite);
				}
			}
		} else {
			if (isDirectory()) {
				// change the destination from a directory to a file
				GeneralFile subFile = FileFactory.newFile(this, sourceFile.getName());
				subFile.copyFrom(sourceFile);
				return;
			}
			try {
				if (sourceFile instanceof LocalFile) {
					ftpClient.put(((LocalFile) sourceFile).getFile(), getPath(),
							!forceOverwrite);
				} else if (sourceFile instanceof FTPFile) {
					ftpClient.transfer(sourceFile.getPath(), ftpClient, getPath(),
							!forceOverwrite, null);
				} else {
					super.copyTo(sourceFile);
				}
			} catch (FTPException e) {
				IOException io = new IOException();
				io.initCause(e);
				throw io;
			}
		}
	}

	/**
	 * Tests whether the application can read the file denoted by this abstract
	 * pathname.
	 * 
	 * @return <code>true</code> if and only if the file specified by this
	 *         abstract pathname exists <em>and</em> can be read; otherwise
	 *         <code>false</code>.
	 */
	@Override
	public boolean canRead() {
		return true;
	}

	/**
	 * Tests whether the application can modify to the file denoted by this
	 * abstract pathname.
	 * 
	 * @return <code>true</code> if and only if the file system actually
	 *         contains a file denoted by this abstract pathname <em>and</em>
	 *         the application is allowed to write to the file; otherwise
	 *         <code>false</code>.
	 */
	@Override
	public boolean canWrite() {
		return false;
	}

	/**
	 * Deletes the file or directory denoted by this abstract pathname. If this
	 * pathname denotes a directory, then the directory must be empty in order
	 * to be deleted.
	 */
	@Override
	public boolean delete() {
		try {
			if (isDirectory()) {
				ftpClient.deleteDir(getPath());
			} else {
				ftpClient.deleteFile(getPath());
			}
		} catch (IOException e) {
			return false;
		} catch (FTPException e) {
			return false;
		}
		return true;
	}

	/**
	 * @return This abstract pathname as a pathname string.
	 */
	@Override
	public String getPath() {
		return getAbsolutePath();
	}

	/**
	 * Tests whether the file denoted by this abstract pathname exists.
	 * 
	 * @return <code>true</code> if and only if the file denoted by this
	 *         abstract pathname exists; <code>false</code> otherwise
	 */
	@Override
	public boolean exists() {
		try {
			ftpClient.exists(getPath());
		} catch (IOException e) {
			return false;
		} catch (FTPException e) {
			return false;
		}
		return true;
	}

	/**
	 * Tests whether the file denoted by this abstract pathname is a directory.
	 * 
	 * @return <code>true</code> if and only if the file denoted by this
	 *         abstract pathname exists <em>and</em> is a directory;
	 *         <code>false</code> otherwise
	 */
	@Override
	public boolean isDirectory() {
		return false;
	}

	/**
	 * Tests whether the file denoted by this abstract pathname is a normal
	 * file.
	 * 
	 * @return <code>true</code> if and only if the file denoted by this
	 *         abstract pathname exists <em>and</em> is a normal file;
	 *         <code>false</code> otherwise
	 */
	@Override
	public boolean isFile() {
		return true;
	}

	/**
	 * Returns the time that the file denoted by this abstract pathname was last
	 * modified.
	 * 
	 * @return A <code>long</code> value representing the time the file was last
	 *         modified, measured in system-dependent way.
	 */
	@Override
	public long lastModified() {
		try {
			Date date = ftpClient.lastModified(getPath());
			return date.getTime();
		} catch (IOException e) {
			return 0;
		} catch (FTPException e) {
			return 0;
		}
	}

	/**
	 * Returns the length of the file denoted by this abstract pathname.
	 * 
	 * @return The length, in bytes, of the file denoted by this abstract
	 *         pathname, or <code>0L</code> if the file does not exist
	 */
	@Override
	public long length() {
		try {
			return ftpClient.size(getPath());
		} catch (IOException e) {
			return 0;
		} catch (FTPException e) {
			return 0;
		}
	}

	/**
	 * Returns an array of strings naming the files and directories in the
	 * directory denoted by this abstract pathname.
	 *<P>
	 * There is no guarantee that the name strings in the resulting array will
	 * appear in any specific order; they are not, in particular, guaranteed to
	 * appear in alphabetical order.
	 *<P>
	 * If this GeneralFile object denotes a file, the results are unspecified.
	 * 
	 * @return An array of strings naming the files and directories in the
	 *         directory denoted by this abstract pathname.
	 */
	@Override
	public String[] list() {
		try {
			Vector list = ftpClient.list(getPath());
			Object[] listO = list.toArray();
			String[] listS = new String[listO.length];
			for (int i = 0; i < listO.length; i++) {
				listS[i] = listO[i].toString();
			}
			return listS;
		} catch (IOException e) {
			return null;
		} catch (FTPException e) {
			return null;
		}
	}

	/**
	 * Creates the directory named by this abstract pathname.
	 */
	@Override
	public boolean mkdir() {
		try {
			ftpClient.makeDir(getPath());
			return true;
		} catch (IOException e) {
			return false;
		} catch (FTPException e) {
			return false;
		}
	}

	/**
	 * Renames the file denoted by this abstract pathname.
	 *<P>
	 * Whether or not this method can move a file from one filesystem to another
	 * is platform-dependent. The return value should always be checked to make
	 * sure that the rename operation was successful.
	 * 
	 * @param dest
	 *            The new abstract pathname for the named file
	 * 
	 * @throws IllegalArgumentException
	 *             If parameter <code>dest</code> is not a
	 *             <code>GeneralFile</code>.
	 * @throws NullPointerException
	 *             - If dest is null
	 */
	@Override
	public boolean renameTo(GeneralFile dest) throws IllegalArgumentException,
			NullPointerException {
		try {
			if (dest instanceof FTPFile) {
				if (ftpClient.equals(((FTPFile) dest).ftpClient)) {
					ftpClient.rename(getPath(), dest.getPath());
				} else {

					if (!dest.exists()) {
						copyTo(dest);
						delete();
					} else
						return false;
				}
			} else {
				if (!dest.exists()) {
					copyTo(dest);
					delete();
				} else
					return false;
			}
		} catch (IOException e) {
			return false;
		} catch (FTPException e) {
			return false;
		}

		return true;
	}

	/**
	 * Returns the pathname string of this abstract pathname.
	 */
	@Override
	public String toString() {
		String username = ((FTPFileSystem) fileSystem).getUserName(), portString;
		int port = ((FTPFileSystem) fileSystem).getPort();

		if (username != null)
			username += "@";
		else
			username = "";
		if (port > 0 && port != 21)
			portString = ":" + port;
		else
			portString = ""; // standard look without the port 21

		return "http://" + username + ((FTPFileSystem) fileSystem).getHost()
				+ portString + getAbsolutePath();
	}

	/**
	 * Constructs a <tt>file:</tt> URI that represents this abstract pathname.
	 * 
	 * <p>
	 * The exact form of the URI is system-dependent. If it can be determined
	 * that the file denoted by this abstract pathname is a directory, then the
	 * resulting URI will end with a slash.
	 * 
	 * <p>
	 * For a given abstract pathname <i>f</i>, it is guaranteed that
	 * 
	 * <blockquote><tt>
	 * new {@link #GeneralFile(java.net.URI) GeneralFile}
	 * (</tt><i>&nbsp;f</i><tt>.toURI()).equals(</tt><i>&nbsp;f</i><tt>)
	 * </tt>
	 * </blockquote>
	 * 
	 * so long as the original abstract pathname, the URI, and the new abstract
	 * pathname are all created in (possibly different invocations of) the same
	 * Java virtual machine. However, this relationship typically does not hold
	 * when a <tt>file:</tt> URI that is created in a virtual machine on one
	 * operating system is converted into an abstract pathname in a virtual
	 * machine on a different operating system.
	 * 
	 * @return An absolute, hierarchical URI with a scheme equal to
	 *         <tt>"file"</tt>, a path representing this abstract pathname, and
	 *         undefined authority, query, and fragment components
	 * 
	 * @see #GeneralFile(java.net.URI)
	 * @see java.net.URI
	 * @see java.net.URI#toURL()
	 */
	@Override
	public URI toURI() {
		try {
			return new URI(toString());
		} catch (URISyntaxException e) {
			log.warn("URI syntax exception, logged and ignored", e);
		}
		return null;
	}

	/**
	 * Converts this abstract pathname into a <code>file:</code> URL. The exact
	 * form of the URL is system-dependent. If it can be determined that the
	 * file denoted by this abstract pathname is a directory, then the resulting
	 * URL will end with a slash.
	 * 
	 * <p>
	 * <b>Usage note:</b> This method does not automatically escape characters
	 * that are illegal in URLs. It is recommended that new code convert an
	 * abstract pathname into a URL by first converting it into a URI, via the
	 * {@link #toURI() toURI} method, and then converting the URI into a URL via
	 * the {@link java.net.URI#toURL() URI.toURL} method.
	 * 
	 * @return A URL object representing the equivalent file URL
	 * 
	 * @throws MalformedURLException
	 *             If the path cannot be parsed as a URL
	 * 
	 * @see #toURI()
	 * @see java.net.URI
	 * @see java.net.URI#toURL()
	 * @see java.net.URL
	 */
	@Override
	public URL toURL() throws MalformedURLException {
		return new URL(toString());
	}
}
