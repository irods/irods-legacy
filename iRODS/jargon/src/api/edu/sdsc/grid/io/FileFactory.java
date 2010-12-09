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
//  FileFactory.java  -  edu.sdsc.grid.io.FileFactory
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-.FileFactory
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io;

import java.io.IOException;
import java.io.Serializable;
import java.lang.reflect.InvocationTargetException;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.HashMap;

import org.ietf.jgss.GSSCredential;
import org.irods.jargon.core.exception.JargonRuntimeException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.ftp.FTPAccount;
import edu.sdsc.grid.io.ftp.FTPFile;
import edu.sdsc.grid.io.ftp.FTPFileSystem;
import edu.sdsc.grid.io.http.HTTPAccount;
import edu.sdsc.grid.io.http.HTTPFile;
import edu.sdsc.grid.io.http.HTTPFileInputStream;
import edu.sdsc.grid.io.http.HTTPFileOutputStream;
import edu.sdsc.grid.io.http.HTTPFileSystem;
import edu.sdsc.grid.io.http.HTTPRandomAccessFile;
import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSFile;
import edu.sdsc.grid.io.irods.IRODSFileInputStream;
import edu.sdsc.grid.io.irods.IRODSFileOutputStream;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.grid.io.irods.IRODSMetaDataRecordList;
import edu.sdsc.grid.io.irods.IRODSRandomAccessFile;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.grid.io.local.LocalFileInputStream;
import edu.sdsc.grid.io.local.LocalFileOutputStream;
import edu.sdsc.grid.io.local.LocalFileSystem;
import edu.sdsc.grid.io.local.LocalMetaDataRecordList;
import edu.sdsc.grid.io.local.LocalRandomAccessFile;
import edu.sdsc.grid.io.srb.SRBAccount;
import edu.sdsc.grid.io.srb.SRBFile;
import edu.sdsc.grid.io.srb.SRBFileInputStream;
import edu.sdsc.grid.io.srb.SRBFileOutputStream;
import edu.sdsc.grid.io.srb.SRBFileSystem;
import edu.sdsc.grid.io.srb.SRBMetaDataRecordList;
import edu.sdsc.grid.io.srb.SRBRandomAccessFile;

/**
 * Operations include creating appropriately typed GeneralFile and
 * GeneralRandomAccessFile objects. Creating a file object can use a "URI" (not
 * a "URL").
 * <P>
 * 
 * @author Lucas Gilbert, San Diego Supercomputer Center
 */
public final class FileFactory {

	private static final String URI_SYNTAX_EXCEPTION_LOGGED_AND_IGNORED = "URI Syntax exception logged and ignored";

	/**
	 * Registration for other file types so they are known and used by the
	 * FileFactory methods.
	 * 
	 * 
	 */
	private static final HashMap<Serializable, Object> classToType = new HashMap<Serializable, Object>();

	private static final int RAF = 0;
	private static final int INPUT = 1;
	private static final int OUTPUT = 2;
	private static final int RECORD_LIST = 3;

	private static final Logger LOG = LoggerFactory
			.getLogger(FileFactory.class);

	static {

		try {
			// register SRB
			FileFactory.registerFileSystem(new URI("srb://fake"),
					SRBAccount.class, SRBFileSystem.class, SRBFile.class,
					SRBRandomAccessFile.class, SRBFileInputStream.class,
					SRBFileOutputStream.class, SRBMetaDataRecordList.class);
		} catch (URISyntaxException e) {
			LOG.warn(URI_SYNTAX_EXCEPTION_LOGGED_AND_IGNORED, e);
		}

		try {
			// register iRODS
			FileFactory.registerFileSystem(new URI("irods://fake"),
					IRODSAccount.class, IRODSFileSystem.class, IRODSFile.class,
					IRODSRandomAccessFile.class, IRODSFileInputStream.class,
					IRODSFileOutputStream.class, IRODSMetaDataRecordList.class);
		} catch (URISyntaxException e) {
			LOG.warn(URI_SYNTAX_EXCEPTION_LOGGED_AND_IGNORED, e);
		}

		// Could also handle anything of URLConnection: mailto, gopher,
		// others...
		try {
			// register http
			FileFactory.registerFileSystem(new URI("http://fake"),
					HTTPAccount.class, HTTPFileSystem.class, HTTPFile.class,
					HTTPRandomAccessFile.class, HTTPFileInputStream.class,
					HTTPFileOutputStream.class, null);

			// register https
			FileFactory.registerFileSystem(new URI("https://fake"),
					HTTPAccount.class, HTTPFileSystem.class, HTTPFile.class,
					HTTPRandomAccessFile.class, HTTPFileInputStream.class,
					HTTPFileOutputStream.class, null);
		} catch (URISyntaxException e) {
			LOG.warn(URI_SYNTAX_EXCEPTION_LOGGED_AND_IGNORED, e);
		}

		try {
			// register ftp
			FileFactory.registerFileSystem(new URI("ftp://fake"),
					FTPAccount.class, FTPFileSystem.class, FTPFile.class, null,
					null, null, null);
		} catch (URISyntaxException e) {
			LOG.warn(URI_SYNTAX_EXCEPTION_LOGGED_AND_IGNORED, e);
		}

	}

	/**
	 * Stores these class types for later use. Does not use the values from this
	 * object. The connection of a GeneralFileSystem will not be used when
	 * creating another.
	 */
	public static void registerFileSystem(final GeneralAccount account,
			final GeneralFileSystem fileSystem, final GeneralFile file) {
		registerFileSystem(null, account.getClass(), fileSystem.getClass(),
				file.getClass(), null, null, null, null);
	}

	/**
	 * Stores these class types for later use. Does not use the values from this
	 * object. The connection of a GeneralFileSystem will not be used when
	 * creating another.
	 */
	public static void registerFileSystem(final URI uri,
			final GeneralAccount account, final GeneralFileSystem fileSystem,
			final GeneralFile file,
			final GeneralRandomAccessFile randomAccessFile) {
		registerFileSystem(uri, account.getClass(), fileSystem.getClass(),
				file.getClass(), randomAccessFile.getClass(), null, null, null);
	}

	/**
	 * Stores these class types for later use. Does not use the values from this
	 * object. The connection of a GeneralFileSystem will not be used when
	 * creating another.
	 */
	public static void registerFileSystem(final URI uri,
			final GeneralAccount account, final GeneralFileSystem fileSystem,
			final GeneralFile file,
			final GeneralRandomAccessFile randomAccessFile,
			final GeneralFileInputStream in, final GeneralFileOutputStream out) {
		registerFileSystem(uri, account.getClass(), fileSystem.getClass(),
				file.getClass(), randomAccessFile.getClass(), in.getClass(),
				out.getClass(), null);
	}

	/**
	 * Stores these class types for later use. Does not use the values from this
	 * object. The connection of a GeneralFileSystem will not be used when
	 * creating another.
	 */
	public static void registerFileSystem(final URI uri,
			final GeneralAccount account, final GeneralFileSystem fileSystem,
			final GeneralFile file,
			final GeneralRandomAccessFile randomAccessFile,
			final GeneralFileInputStream in, final GeneralFileOutputStream out,
			final MetaDataRecordList rl) {
		registerFileSystem(uri, account.getClass(), fileSystem.getClass(),
				file.getClass(), randomAccessFile.getClass(), in.getClass(),
				out.getClass(), rl.getClass());
	}

	/**
	 * Stores these class types for later use. Does not use the values from this
	 * object. The connection of this GeneralFileSystem will not be used when
	 * creating a new connection or for sending commands.
	 */
	static void registerFileSystem(final URI uri, final Class account,
			final Class fileSystem, final Class file,
			final Class randomAccessFile, final Class inputStream,
			final Class outputStream, final Class recordList) {
		if (fileSystem == null) {
			throw new IllegalArgumentException("FileSystem cannot be null");
		}
		if (account != null) {
			classToType.put(account, fileSystem);
		}
		if (uri != null) {
			classToType.put(uri.getScheme(), file);
		}
		if (file != null) {
			classToType.put(fileSystem, file);
		}

		Object[] various = new Object[4];
		if (randomAccessFile != null) {
			various[0] = randomAccessFile;
		}
		if (inputStream != null) {
			various[1] = inputStream;
		}
		if (outputStream != null) {
			various[2] = outputStream;
		}
		if (recordList != null) {
			various[3] = recordList;
		}
		classToType.put(file, various);
	}

	/**
	 * Returns true iff <code>fileSystem</code> been registered.
	 */
	static boolean isFileSystemRegistered(final GeneralFileSystem fileSystem) {
		return classToType.containsKey(fileSystem.getClass());
	}

	private static Object fromMap(final Object key, final Object arg)
			throws IOException {
		return fromMap(key, new Object[] { arg }, -1);
	}

	private static Object fromMap(final Object key, final Object[] arg)
			throws IOException {
		return fromMap(key, arg, -1);
	}

	// first argument is the key for the Hashmap
	private static Object fromMap(final Object key, final Object[] args,
			final int subKey) throws IOException {
		if ((key != null) && (args != null)) {
			Object main = classToType.get(key);
			if (main == null) {
				return null;
			}
			if (subKey == RECORD_LIST) {
				// MetaDataRecordLists
				// returns the file type which will return the array...
				main = classToType.get(main);
				if (main == null) {
					return null;
				}
				main = ((Object[]) main)[subKey];
			} else if (main instanceof Object[]) {
				main = ((Object[]) main)[subKey];
			}
			if (main == null) {
				// return null;
				throw new UnsupportedOperationException(
						"Class not supported for " + key);
			}
			Class[] argsClass = new Class[args.length];
			for (int i = 0; i < args.length; i++) {
				argsClass[i] = args[i].getClass();
			}
			;
			try {
				return ((Class) main).getConstructor(argsClass).newInstance(
						args);
			} catch (NoSuchMethodException e) {
				Throwable x = e.getCause();
				if (x instanceof IOException) {
					throw (IOException) x;
				}
				// else
				JargonRuntimeException re = new JargonRuntimeException();
				re.initCause(e);
				throw re;
			} catch (InstantiationException e) {
				Throwable x = e.getCause();
				if (x instanceof IOException) {
					throw (IOException) x;
				}
				// else
				JargonRuntimeException re = new JargonRuntimeException();
				re.initCause(e);
				throw re;
			} catch (IllegalAccessException e) {
				Throwable x = e.getCause();
				if (x instanceof IOException) {
					throw (IOException) x;
				}
				// else
				JargonRuntimeException re = new JargonRuntimeException();
				re.initCause(e);
				throw re;
			} catch (IllegalArgumentException e) {
				Throwable x = e.getCause();
				if (x instanceof IOException) {
					throw (IOException) x;
				}
				// else
				JargonRuntimeException re = new JargonRuntimeException();
				re.initCause(e);
				throw re;
			} catch (InvocationTargetException e) {
				Throwable x = e.getCause();
				if (x instanceof IOException) {
					throw (IOException) x;
				}
				// else
				JargonRuntimeException re = new JargonRuntimeException();
				re.initCause(e);
				throw re;
			}
		}
		return null;
	}

	/**
	 * Creates a filesystem appropriate to the account object.
	 * 
	 * @param account
	 *            the account object used to initialize the filesystem.
	 * @return a GeneralFileSystem object instanced from the appropriate
	 *         subclass. null will return a LocalFileSystem object.
	 * @throws IOException
	 *             If an IO error occurs during the connection to the
	 *             filesystem.
	 */
	public static GeneralFileSystem newFileSystem(final GeneralAccount account)
			throws IOException {
		GeneralFileSystem fs = (GeneralFileSystem) fromMap(account.getClass(),
				account);
		if (fs == null) {
			// Default to local
			return new LocalFileSystem();
		}
		return fs;
	}

	/**
	 * Creates a filesystem based on the URI.
	 * 
	 * @param uri
	 *            the uri used to initialize the filesystem.
	 * @return a GeneralFileSystem object instanced from the appropriate
	 *         subclass.
	 * @throws IOException
	 *             If an IO error occurs during the connection to the
	 *             filesystem.
	 */
	public static GeneralFileSystem newFileSystem(final URI uri)
			throws IOException {
		GeneralFileSystem fs = ((GeneralFile) fromMap(uri.getScheme(), uri))
				.getFileSystem();
		if (fs == null) {
			// Default to local
			return new LocalFileSystem();
		}
		return fs;
	}

	/**
	 * Creates a filesystem based on the URI.
	 * 
	 * @param uri
	 *            the uri used to initialize the filesystem.
	 * @return a GeneralFileSystem object instanced from the appropriate
	 *         subclass.
	 * @throws IOException
	 *             If an IO error occurs during the connection to the
	 *             filesystem.
	 */
	public static GeneralFileSystem newFileSystem(final URI uri,
			final GSSCredential gssCredential) throws IOException {
		GeneralFileSystem fs = ((GeneralFile) fromMap(uri.getScheme(),
				new Object[] { uri, gssCredential })).getFileSystem();
		if (fs == null) {
			// Default to local
			return new LocalFileSystem();
		}
		return fs;
	}

	/**
	 * Creates an abstract pathname using this uri. Currently supported URI
	 * schemes are "file://", "http://", "ftp://", "srb://", and "irods://".
	 * 
	 * 
	 * @param uri
	 *            A URI object of the supported schemes.
	 * @return a GeneralFile object instanced from the appropriate subclass.
	 * @throws SecurityException
	 *             The most likely cause is the URI did not include a password.
	 *             For security reasons, uri's are generally given without the
	 *             password. Acquire a new password and
	 * @see GeneralFile newFile( URI uri, String password )
	 * 
	 * @throws NullPointerException
	 *             If uri argument is null.
	 * @throws IOException
	 *             If an IO error occurs testing the connection to the
	 *             filesystem.
	 */
	public static GeneralFile newFile(final URI uri) throws IOException {
		if (uri.getScheme().equals("file")) {
			return new LocalFile(uri);
		}

		GeneralFile file = (GeneralFile) fromMap(uri.getScheme(), uri);

		if (file == null) {
			// Default to local
			return new LocalFile(uri);
		}
		return file;
	}

	/**
	 * Creates an abstract pathname using this uri. Currently supported URI
	 * schemes are "file://", "http://", "ftp://", "srb://", and "irods://".
	 * <P>
	 * Including a text password in a URI string is not advisable for security
	 * reasons. This method allows the password to obtained by more secure
	 * methods. The connection to the file will then be made through the default
	 * authorization method of the file's filesystem.
	 * 
	 * @param uri
	 *            A URI object of the supported schemes.
	 * @param password
	 *            The user's password.
	 * @return a GeneralFile object instanced from the appropriate subclass.
	 * @throws IllegalArgumentException
	 *             If the password contains illegal charaters.
	 * @throws NullPointerException
	 *             If uri argument is null.
	 * @throws IOException
	 *             If an IO error occurs testing the connection to the
	 *             filesystem.
	 */
	public static GeneralFile newFile(final URI uri, final String password)
			throws IOException {
		int index = -1;
		String userInfo = uri.getUserInfo();
		Class file = (Class) classToType.get(uri.getScheme());
		URI newURI = null;
		if (file != null && password != null) {
			if (uri.getScheme().equals("file")) {
				return new LocalFile(uri);
			}
			if ((password.indexOf(":") >= 0) || (password.indexOf("@") >= 0)) {
				throw new IllegalArgumentException(
						"Password contains illegal charaters");
			}

			// weird srb thing
			if (file.equals(SRBFile.class)
					&& ((userInfo == null) || (userInfo == ""))) {
				// anon. login
				userInfo = "public.npaci:CANDO";
			} else {
				index = userInfo.indexOf(":");
				if (index >= 0) {
					// remove password already in uri and
					userInfo = userInfo.substring(0, index);
				}
				// add param password
				userInfo += ":" + password;
			}

			try {
				newURI = new URI(uri.getScheme(), userInfo, uri.getHost(),
						uri.getPort(), uri.getPath(), uri.getQuery(),
						uri.getFragment());
			} catch (URISyntaxException e) {
				throw new IllegalArgumentException(
						"Password contains illegal characters", e);
			}
		}
		return newFile(newURI);
	}

	/**
	 * Creates an abstract pathname using this uri. Currently supported URI
	 * schemes are "file://", "http://", "ftp://", "srb://", and "irods://".
	 * 
	 * 
	 * @param uri
	 *            A URI object of the supported schemes. A URI including a
	 *            password component will be overridden by the credential
	 *            authorization.
	 * @return a GeneralFile object instanced from the appropriate subclass.
	 * @throws SecurityException
	 *             The most likely cause is the URI did not include a password.
	 *             For security reasons, uri's are generally given without the
	 *             password. *
	 * @throws NullPointerException
	 *             If uri argument is null.
	 * @throws IOException
	 *             If an IO error occurs testing the connection to the
	 *             filesystem.
	 */
	public static GeneralFile newFile(final URI uri,
			final GSSCredential credential) throws IOException {
		int index = -1;
		String userInfo = uri.getUserInfo();
		String userName, domain, password = null;

		if (uri.getScheme().equals("srb")) {
			if ((userInfo == null) || (userInfo.equals(""))) {
				// anon. login
				userName = "public";
				domain = "npaci";
			} else {
				index = userInfo.indexOf(":");
				if (index >= 0) {
					userName = userInfo.substring(0, index);
					password = userInfo.substring(index + 1);
				} else {
					userName = userInfo;
				}

				index = userName.indexOf(".");
				if (index >= 0) {
					userName = userInfo.substring(0, index);
					domain = userInfo.substring(index + 1);
				} else {
					throw new IllegalArgumentException("No SRB domain given");
				}
			}

			SRBAccount account = new SRBAccount(uri.getHost(), uri.getPort(),
					userName, password, "/home/" + userName + "." + domain,
					domain, null);
			account.setGSSCredential(credential);
			return newFile(account, uri.getPath());
		} else if (uri.getScheme().equals("irods")) {
			if ((userInfo == null) || (userInfo.equals(""))) {
				// anon. login
				userName = "public";
			} else {
				index = userInfo.indexOf(":");
				if (index >= 0) {
					userName = userInfo.substring(0, index);
					password = userInfo.substring(index + 1);
				} else {
					userName = userInfo;
				}
			}

			IRODSAccount account = new IRODSAccount(uri.getHost(),
					uri.getPort(), userName, password, uri.getPath(), null,
					null);
			account.setGSSCredential(credential);
			return newFile(account, uri.getPath());
		}
		// else if other types

		// Default to local
		return new LocalFile(uri);
	}

	/**
	 * Creates an abstract pathname using this uri. Currently supported URI
	 * schemes are "file://", "http://", "ftp://", "srb://", and "irods://".
	 * <P>
	 * Including a text password in a URI string is not advisable for security
	 * reasons. This method allows for authentication with GSI, if supported by
	 * that filesystem.
	 * 
	 * @param uri
	 *            A URI object of the supported schemes.
	 * @param proxyFilePath
	 *            The location of the proxy file on the local filesystem.
	 * @param certificateAurthority
	 *            The locations of the GSI Certificate Authority (CA). The list
	 *            can contain multiple files that are comma separated. By
	 *            default, the CA definition comes from the user's
	 *            cog.properties file.
	 * @return a GeneralFile object instanced from the appropriate subclass.
	 * @throws IllegalArgumentException
	 *             If the password contains illegal charaters.
	 * @throws NullPointerException
	 *             If uri argument is null.
	 * @throws IOException
	 *             If an IO error occurs testing the connection to the
	 *             filesystem.
	 */
	protected static GeneralFile newFile(final URI uri,
			final String proxyFilePath, final String certificateAurthority)
			throws IOException {
		GeneralFile file = (GeneralFile) fromMap(uri.getScheme(), new Object[] {
				uri, proxyFilePath, certificateAurthority });
		if (file == null) {
			// Default to local
			return new LocalFile(uri);
		}
		return file;
	}

	/**
	 * Creates an abstract pathname using a GeneralAccount object to first
	 * create a GeneralFileSystem connection. Useful if you have some account
	 * info, but you don't know which the file system the file is on. Calls the
	 * constructor( FileSystem, String ) of the appropriate subclass.
	 * 
	 * @param account
	 *            A generic account object.
	 * @param path
	 *            The path to the file.
	 * @return a GeneralFile object instanced from the appropriate subclass.
	 * @throws NullPointerException
	 *             If the path argument is null.
	 */
	public static GeneralFile newFile(final GeneralAccount account,
			final String path) throws IOException {
		GeneralFile file = (GeneralFile) fromMap(account.getClass(),
				new Object[] { account, path });
		if (file == null) {
			// Default to local
			return new LocalFile(path);
		}
		return file;
	}

	/**
	 * Creates an abstract pathname using a GeneralAccount object to first
	 * create a GeneralFileSystem connection. Useful if you have some account
	 * info, but you don't know which the file system the file is on. Calls the
	 * constructor( FileSystem, String ) of the appropriate subclass.
	 * 
	 * @param account
	 *            A generic account object.
	 * @param parent
	 *            The directory path to the file.
	 * @param child
	 *            The file name string.
	 * @return a GeneralFile object instanced from the appropriate subclass.
	 * @throws NullPointerException
	 *             If the path argument is null.
	 */
	public static GeneralFile newFile(final GeneralAccount account,
			final String parent, final String child) throws IOException {
		GeneralFile file = (GeneralFile) fromMap(account.getClass(),
				new Object[] { account, parent, child });
		if (file == null) {
			// Default to local
			return new LocalFile(parent, child);
		}
		return file;
	}

	/**
	 * Creates an abstract pathname using a GeneralFileSystem object. Useful for
	 * when you don't know which the file system the file is on. Calls the
	 * constructor( FileSystem, String ) of the appropriate subclass.
	 * 
	 * @param fs
	 *            A generic file system reference object.
	 * @param path
	 *            The path to the file.
	 * @return a GeneralFile object instanced from the appropriate subclass.
	 * @throws NullPointerException
	 *             If the path argument is null.
	 */
	public static GeneralFile newFile(final GeneralFileSystem fs,
			final String path) {
		GeneralFile file = null;
		try {
			file = (GeneralFile) fromMap(fs.getClass(),
					new Object[] { fs, path });
			if (file == null) {
				// Default to local
				return new LocalFile(path);
			}
		} catch (IOException e) {
			// such error an error really isn't possible.
			LOG.warn("IOException, logged and ignored", e);

		}
		return file;
	}

	/**
	 * Creates an abstract pathname using a GeneralFileSystem object. Useful for
	 * when you don't know which the file system the file is on. Calls the
	 * constructor( FileSystem, String, String ) of the appropriate subclass.
	 * 
	 * @param fs
	 *            A generic file system reference object.
	 * @param parent
	 *            The directory path to the file.
	 * @param child
	 *            The file name string.
	 * @return a GeneralFile object instanced from the appropriate subclass.
	 * @throws NullPointerException
	 *             If the child argument is null.
	 */
	public static GeneralFile newFile(final GeneralFileSystem fs,
			final String parent, final String child) {
		GeneralFile file = null;
		try {
			file = (GeneralFile) fromMap(fs.getClass(), new Object[] { fs,
					parent, child });
			if (file == null) {
				// Default to local
				return new LocalFile(parent, child);
			}
		} catch (IOException e) {
			// such error an error really isn't possible.
			LOG.warn("io exception, logged and ignored", e);
		}
		return file;
	}

	/**
	 * Creates an abstract pathname using a GeneralFileSystem object. Useful for
	 * when you don't know which the file system the file is on. Calls the
	 * constructor( File, String ) of the appropriate subclass.
	 * 
	 * @param parent
	 *            An abstract pathname to the file.
	 * @param child
	 *            The file name string.
	 * @return a GeneralFile object instanced from the appropriate subclass.
	 * @throws NullPointerException
	 *             If the child argument is null.
	 */
	public static GeneralFile newFile(final GeneralFile parent,
			final String child) {
		GeneralFile file = null;
		try {
			file = (GeneralFile) fromMap(parent.getFileSystem().getClass(),
					new Object[] { parent, child });
			if (file == null) {
				// Default to local
				if (!(parent instanceof LocalFile)) {
					throw new IllegalArgumentException(
							"parent is not a LocalFile");
				}

				LocalFile localFileParent = (LocalFile) parent;

				return new LocalFile(localFileParent, child);
			}
		} catch (IOException e) {
			// such error an error really isn't possible.
			LOG.warn("io exception, logged and ignored", e);
		}
		return file;
	}

	/**
	 * Creates a new directory of symbolic links. The conditions given are used
	 * for a query, the results define which links get created. Thus the new
	 * directory will [appear to] contain all files on the filesystem wil match
	 * the given query. Clearly this will only work on fileSystem which support
	 * querying and symbolic links.
	 */

	protected static GeneralFile newFile(final GeneralFileSystem fileSystem,
			final String filePath, final MetaDataCondition[] conditions)
			throws NullPointerException, IOException {
		GeneralFile dir = newFile(fileSystem, filePath);
		dir.mkdir();

		MetaDataSelect[] selects = MetaDataSet.newSelection(new String[] {
				StandardMetaData.FILE_NAME, StandardMetaData.DIRECTORY_NAME });
		MetaDataRecordList[] rl = fileSystem.query(conditions, selects);

		if (rl == null) {
			return dir;
		}
		for (int i = 0; i < rl.length; i++) {
			// new file.link based on the result(dir, filename)
			((SRBFile) newFile(fileSystem, rl[i].getStringValue(1),
					rl[i].getStringValue(0))).link(((SRBFile) newFile(dir,
					rl[i].getStringValue(0) + "_" + i)));
		}

		return dir;
	}

	/**
	 * Opens a random accecss connection to the file on an arbitrary file
	 * system. Useful for when you don't know which the file system the file is
	 * on.
	 * 
	 * @return a GeneralRandomAccessFile object instanced from the appropriate
	 *         subclass.
	 * @throws NullPointerException
	 *             If the file argument is null.
	 * @throws IOException
	 *             If an IO error occurs opening the file.
	 */
	public static GeneralRandomAccessFile newRandomAccessFile(
			final GeneralFileSystem fileSystem, final String filePath,
			final String mode) throws IOException {
		return newRandomAccessFile(newFile(fileSystem, filePath), mode);
	}

	/**
	 * Opens a random access connection to the file on an arbitrary file system.
	 * Useful for when you don't know which the file system the file is on.
	 * 
	 * @return a GeneralRandomAccessFile object instanced from the appropriate
	 *         subclass.
	 * @throws NullPointerException
	 *             If the file argument is null.
	 * @throws IOException
	 *             If an IO error occurs opening the file.
	 */
	public static GeneralRandomAccessFile newRandomAccessFile(
			final GeneralFile file, final String mode) throws IOException {

		GeneralRandomAccessFile raf = (GeneralRandomAccessFile) fromMap(
				file.getClass(), new Object[] { file, mode }, RAF);
		if (raf == null) {
			if (!(file instanceof LocalFile)) {
				throw new IllegalArgumentException("file is not a LocalFile");
			}
			LocalFile localFile = (LocalFile) file;

			// Default to local
			return new LocalRandomAccessFile(localFile, mode);
		}
		return raf;
	}

	/**
	 * Opens a random accecss connection to the file on an arbitrary file
	 * system. Useful for when you don't know which the file system the file is
	 * on.
	 * 
	 * @return a GeneralRandomAccessFile object instanced from the appropriate
	 *         subclass.
	 * @throws NullPointerException
	 *             If the file argument is null.
	 * @throws IOException
	 *             If an IO error occurs opening the file.
	 */
	public static GeneralRandomAccessFile newRandomAccessFile(final URI uri,
			final String mode) throws IOException {
		return newRandomAccessFile(newFile(uri), mode);
	}

	/**
	 * Opens an input stream for the file on an arbitrary file system. Useful
	 * for when you don't know which the file system the file is on.
	 * 
	 * @return a GeneralFileInputStream object instanced from the appropriate
	 *         subclass.
	 * @throws NullPointerException
	 *             If the file argument is null.
	 * @throws IOException
	 *             If an IO error occurs opening the file.
	 */
	public static GeneralFileInputStream newFileInputStream(
			final GeneralFile file) throws IOException {
		GeneralFileInputStream in = (GeneralFileInputStream) fromMap(
				file.getClass(), new Object[] { file }, INPUT);
		if (in == null) {
			// Default to local

			if (!(file instanceof LocalFile)) {
				throw new IllegalArgumentException("file is not a LocalFile");
			}

			LocalFile localFile = (LocalFile) file;

			return new LocalFileInputStream(localFile);

		}
		return in;
	}

	/**
	 * Opens an input stream for the file on an arbitrary file system. Useful
	 * for when you don't know which the file system the file is on.
	 * 
	 * @return a GeneralFileInputStream object instanced from the appropriate
	 *         subclass.
	 * @throws NullPointerException
	 *             If the file argument is null.
	 * @throws IOException
	 *             If an IO error occurs opening the file.
	 */
	public static GeneralFileInputStream newFileInputStream(final URI uri)
			throws IOException {
		return newFileInputStream(newFile(uri));
	}

	/**
	 * Opens an output stream for the file on an arbitrary file system. Useful
	 * for when you don't know which the file system the file is on.
	 * 
	 * @return a GeneralFileOutputStream object instanced from the appropriate
	 *         subclass.
	 * @throws NullPointerException
	 *             If the file argument is null.
	 * @throws IOException
	 *             If an IO error occurs opening the file.
	 */
	public static GeneralFileOutputStream newFileOutputStream(
			final GeneralFile file) throws IOException {
		GeneralFileOutputStream out = (GeneralFileOutputStream) fromMap(
				file.getClass(), new Object[] { file }, OUTPUT);
		if (out == null) {
			// Default to local
			if (!(file instanceof LocalFile)) {
				throw new IllegalArgumentException("file is not a LocalFile");
			}
			
			LocalFile localFile = (LocalFile) file;
			
			return new LocalFileOutputStream(localFile);
		}
		return out;
	}

	/**
	 * Opens an output stream for the file on an arbitrary file system. Useful
	 * for when you don't know which the file system the file is on.
	 * 
	 * @return a GeneralFileOutputStream object instanced from the appropriate
	 *         subclass.
	 * @throws NullPointerException
	 *             If the file argument is null.
	 * @throws IOException
	 *             If an IO error occurs opening the file.
	 */
	public static GeneralFileOutputStream newFileOutputStream(final URI uri)
			throws IOException {
		return newFileOutputStream(newFile(uri));
	}

	/**
	 * Opens an output stream for the file on an arbitrary file system. Useful
	 * for when you don't know which the file system the file is on.
	 * 
	 * @return a GeneralFileOutputStream object instanced from the appropriate
	 *         subclass.
	 * @throws NullPointerException
	 *             If the file argument is null.
	 * @throws IOException
	 *             If an IO error occurs opening the file.
	 */
	public static MetaDataRecordList newMetaDataRecordList(
			final GeneralFileSystem fileSystem, final MetaDataField field,
			final int recordValue) {
		return newMetaDataRecordList(fileSystem, field, recordValue + "", true);
	}

	/**
	 * Opens an output stream for the file on an arbitrary file system. Useful
	 * for when you don't know which the file system the file is on.
	 * 
	 * @return a GeneralFileOutputStream object instanced from the appropriate
	 *         subclass.
	 * @throws NullPointerException
	 *             If the file argument is null.
	 * @throws IOException
	 *             If an IO error occurs opening the file.
	 */
	public static MetaDataRecordList newMetaDataRecordList(
			final GeneralFileSystem fileSystem, final MetaDataField field,
			final float recordValue) {
		return newMetaDataRecordList(fileSystem, field, recordValue + "", true);
	}

	/**
	 * Opens an output stream for the file on an arbitrary file system. Useful
	 * for when you don't know which the file system the file is on.
	 * 
	 * @return a GeneralFileOutputStream object instanced from the appropriate
	 *         subclass.
	 * @throws NullPointerException
	 *             If the file argument is null.
	 * @throws IOException
	 *             If an IO error occurs opening the file.
	 */
	public static MetaDataRecordList newMetaDataRecordList(
			final GeneralFileSystem fileSystem, final MetaDataField field,
			final String recordValue) {
		return newMetaDataRecordList(fileSystem, field, recordValue, true);
	}

	/**
	 * Opens an output stream for the file on an arbitrary file system. Useful
	 * for when you don't know which the file system the file is on.
	 * 
	 * @return a GeneralFileOutputStream object instanced from the appropriate
	 *         subclass.
	 * @throws NullPointerException
	 *             If the file argument is null.
	 * @throws IOException
	 *             If an IO error occurs opening the file.
	 */
	public static MetaDataRecordList newMetaDataRecordList(
			final GeneralFileSystem fileSystem, final MetaDataField field,
			final MetaDataTable recordValue) {
		return newMetaDataRecordList(fileSystem, field, recordValue, true);
	}

	// Does the work for the like above. Makes the code maintainence easier.
	private static MetaDataRecordList newMetaDataRecordList(
			final GeneralFileSystem fileSystem, final MetaDataField field,
			final Object recordValue, final boolean fake) {
		MetaDataRecordList rl = null;
		try {
			rl = (MetaDataRecordList) fromMap(fileSystem.getClass(),
					new Object[] { field, recordValue }, RECORD_LIST);
			if (rl == null) {
				// Default to local
				return new LocalMetaDataRecordList();
			}
		} catch (IOException e) {
			// such error an error really isn't possible.
			LOG.warn("io exception, logged and ignored", e);
		}
		return rl;
	}
}
