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
//  IRODSFile.java  -  edu.sdsc.grid.io.irods.IRODSFile
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-.GeneralFile
//              |
//              +-.RemoteFile
//                 |
//                 +-.irods.IRODSFile
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.irods;

import java.io.*;
import java.net.*;
import java.util.*;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.local.*;
import edu.sdsc.grid.io.*;

/**
 * An abstract representation of file and directory pathnames on an iRODS
 * server.
 *<P>
 * Shares many similarities with the java.io.File class: User interfaces and
 * operating systems use system-dependent pathname strings to name files and
 * directories. This class presents an abstract, system-independent view of
 * hierarchical pathnames. An abstract pathname has two components:
 *<P>
 * Instances of the IRODSFile class are immutable; that is, once created, the
 * abstract pathname represented by a IRODSFile object will never change.
 *<P>
 * 
 * @author Lucas Gilbert, San Diego Supercomputer Center
 * @since JARGON2.0
 * @see java.io.File
 * @see edu.sdsc.grid.io.GeneralFile
 */
public class IRODSFile extends RemoteFile {

	/**
	 * Standard iRODS path separator character represented as a string for
	 * convenience. This string contains a single character, namely
	 * <code>{@link #PATH_SEPARATOR_CHAR}</code>.
	 */
	public static final String PATH_SEPARATOR = "/";

	/**
	 * The iRODS path separator character, '/'.
	 */
	public static final char PATH_SEPARATOR_CHAR = '/';

	/**
	 * Whether this abstract pathname is a file or directory is undetermined.
	 */
	static final int PATH_IS_UNKNOWN = 0;

	/**
	 * This abstract pathname is a file.
	 */
	static final int PATH_IS_FILE = 1;

	/**
	 * This abstract pathname is a directory.
	 */
	static final int PATH_IS_DIRECTORY = 2;

	/** integer from some queries signifying user can read a file */
	static final int READ_PERMISSIONS = 1050;
	/** integer from some queries signifying user can write to a file */
	static final int WRITE_PERMISSIONS = 1120;

	private static Logger log = LoggerFactory.getLogger(IRODSFile.class);

	/**
	 * Connection to the iRODS server.
	 */
	IRODSFileSystem iRODSFileSystem;

	/**
	 * The storage resource name. A physical iRODS resource is a system that is
	 * capable of storing data sets and is accessible to the iRODS. It is
	 * registered in iRODS with its physical characteristics such as its
	 * physical location, resource type, latency, and maximum file size. For
	 * example, HPSS can be a resource, as can a Unix file system.
	 */
	String resource;

	/**
	 * The data type of the file. The default value is "generic".
	 */
	String dataType = "generic";

	/**
	 * If delete on exit gets set.
	 */
	boolean deleteOnExit = false;

	/**
	 * For list() should the entire contents be listed, or just
	 * IRODSFileSystem.DEFAULT_RECORDS_WANTED in number.
	 */
	static boolean completeDirectoryList = true;

	/**
	 * Whether this abstract pathname is actually a directory or a file. Reduces
	 * the number of network calls.
	 */
	int pathNameType = PATH_IS_UNKNOWN;

	/**
	 * If true, the cached value for the isDirectory or isFile methods will
	 * always be used. The cache can be refresh by calling the
	 * <code>isFile(true)</code> or <code>isDirectory(true)</code>. When false,
	 * inside any particular IRODSFile method isFile(true) or isDirectory(true)
	 * is called only once. However if you know the status as a normal file will
	 * not change and want to perform a lot of actions on that file, then
	 * setting this value to false could save a number of network calls. Care
	 * should be taken though, as most methods don't have a refresh cache
	 * option, so not updating at the beginning of each method could cause
	 * errors.
	 */
	boolean useCache = false;

	/**
	 * Creates a new IRODSFile instance by converting the given pathname string
	 * into an abstract pathname.
	 *<P>
	 * 
	 * @param fileSystem
	 *            The connection to the iRODS server
	 * @param filePath
	 *            A pathname string
	 */
	public IRODSFile(IRODSFileSystem fileSystem, String filePath) {
		this(fileSystem, "", filePath);
	}

	/**
	 * Creates a new iRODSFile instance from a parent pathname string and a
	 * child pathname string.
	 *<P>
	 * If parent is null then the new IRODSFile instance is created as if by
	 * invoking the single-argument IRODSFile constructor on the given child
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
	 *            The connection to the iRODS server
	 * @param parent
	 *            The parent pathname string
	 * @param child
	 *            The child pathname string
	 */
	public IRODSFile(IRODSFileSystem fileSystem, String parent, String child) {
		super(fileSystem, parent, child);
		resource = iRODSFileSystem.getDefaultStorageResource();
		makePathCanonical(parent);
	}

	/**
	 * Creates a new IRODSFile instance from a parent abstract pathname and a
	 * child pathname string.
	 *<P>
	 * If parent is null then the new IRODSFile instance is created as if by
	 * invoking the single-argument IRODSFile constructor on the given child
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
	public IRODSFile(IRODSFile parent, String child) {
		this((IRODSFileSystem) parent.getFileSystem(),
				parent.getAbsolutePath(), child);
	}

	/**
	 * Creates a new IRODSFile instance by converting the given file: URI into
	 * an abstract pathname.
	 *<P>
	 * iRODS URI protocol:<br>
	 * irods:// [ userName [ : password ] @ ] host [ : port ][ / path ]
	 *<P>
	 * example:<br>
	 * irods://irods@irods.sdsc.edu:21/pub/testfile.txt
	 * 
	 * @param uri
	 *            An absolute, hierarchical URI using a supported scheme.
	 * @throws NullPointerException
	 *             if <code>uri</code> is <code>null</code>.
	 * @throws IllegalArgumentException
	 *             If the preconditions on the parameter do not hold.
	 */
	public IRODSFile(URI uri) throws IOException, URISyntaxException {
		super(uri);

		if (uri.getScheme().equals("irods")) {
			String userInfo = uri.getUserInfo();
			String userName = null, password = "", zone = "", homeDirectory = null;
			int index = -1;

			if ((userInfo == null) || (userInfo.equals(""))) {
				// anon. login
				userName = IRODSAccount.PUBLIC_USERNAME;
				homeDirectory = uri.getPath();
			} else {
				index = userInfo.indexOf(":");
				if (index >= 0) {
					password = userInfo.substring(index + 1); // password
					userInfo = userInfo.substring(0, index);
				}

				index = userInfo.indexOf(".");
				if (index >= 0) {
					userName = userInfo.substring(0, index);
					zone = userInfo.substring(index + 1);
					homeDirectory = PATH_SEPARATOR + zone + PATH_SEPARATOR
							+ userName;
				} else {
					userName = userInfo;
					homeDirectory = uri.getPath();
				}

				// connection pool? insecure?
				setFileSystem(new IRODSFileSystem(new IRODSAccount(uri
						.getHost(), uri.getPort(), userName, password,
						homeDirectory, zone, "")) // default resource see
				);

				// iRODSFileSystem.setDefaultStorageResource( resource );
			}

			setFileName(uri.getPath());
			// resource = getAvailableResource();
		} else {
			throw new URISyntaxException(uri.toString(), "Wrong URI scheme");
		}
	}
	
	/**
	 * This method is here for correctness, and will help avoid dropping connections when disconnect was never called.
	 * Note that this method closes the underlying <code>IRODSFileSystem</code>, and care must be taken so as not to close
	 * the <code>IRODSFileSystem</code> when it is intended for re-use.  Specifically, this method is added for occasions where
	 * the <code>IRODSFile</code> is created with a URI parameter.  In that case, the IRODSFileSystem is not reused, and without
	 * calling close, the termination of the <code>IRODSFile</code> causes a 
	 * "readMsgHeader:header read- read 0 bytes, expect 4, status = -4000" error in IRODS.
	 * @throws NullPointerException
	 * @throws IOException
	 */
	public void close() throws NullPointerException, IOException {
		log.info("close called on IRODSFile, will close file system");
		((IRODSFileSystem) this.getFileSystem()).close();
	}

	/**
	 * Finalizes the object by explicitly letting go of each of its internally
	 * held values.
	 */
	protected void finalize() throws Throwable {
		if (deleteOnExit)
			delete();
		
		super.finalize();

		if (resource != null)
			resource = null;

		if (dataType != null)
			dataType = null;
	}

	
	/**
	 * Step three of IRODSFile( uri )
	 * 
	 * Query the file system to determine this IRODSFile's storage resource,
	 * Currently iRODS does not have access control on resources. Just pick one
	 * at random then use that for the default resource of the fileSystem object
	 * as well.
	 */
	protected String getAvailableResource() throws IOException {
		MetaDataRecordList[] recordList = this
				.query(new String[] { IRODSMetaDataSet.RESOURCE_NAME });

		if (recordList != null && recordList.length > 0) {
			return recordList[0].getStringValue(0);
		} else {
			throw new IOException("No resources available");
		}
	}

	
	/**
	 * Sets the file system used of this GeneralFile object. The file system
	 * object must be a subclass of the GeneralFileSystem matching this file
	 * object. eg. XYZFile requires XYZFileSystem.
	 * 
	 * @param fileSystem
	 *            The file system to be used.
	 * @throws IllegalArgumentException
	 *             - if the argument is null.
	 * @throws ClassCastException
	 *             - if the argument is not an object of the approriate
	 *             subclass.
	 */
	protected void setFileSystem(GeneralFileSystem fileSystem)
			throws IllegalArgumentException, ClassCastException {
		if (fileSystem == null)
			throw new IllegalArgumentException(
					"Illegal fileSystem, cannot be null");

		this.fileSystem = fileSystem;
		iRODSFileSystem = (IRODSFileSystem) fileSystem;
	}

	/**
	 * Set the file name.
	 * 
	 * @param fleName
	 *            The file name or fileName plus some or all of the directory
	 *            path.
	 */
	protected void setFileName(String filePath) {
		// used when parsing the filepath
		int index;

		// in case they used the local pathSeperator
		// in the fileName instead of the iRODS PATH_SEPARATOR.
		String localSeparator = System.getProperty("file.separator");

		if (filePath == null) {
			throw new NullPointerException("The file name cannot be null");
		}

		// replace local separators with iRODS separators.
		if (!localSeparator.equals(PATH_SEPARATOR)) {
			index = filePath.lastIndexOf(localSeparator);
			while ((index >= 0)
					&& ((filePath.substring(index + 1).length()) > 0)) {
				filePath = filePath.substring(0, index) + PATH_SEPARATOR
						+ filePath.substring(index + 1);
				index = filePath.lastIndexOf(localSeparator);
			}
		}
		fileName = filePath;

		if (fileName.length() > 1) { // add to allow path = root "/"
			index = fileName.lastIndexOf(PATH_SEPARATOR);
			while ((index == fileName.length() - 1) && (index >= 0)) {
				// remove '/' at end of filename, if exists
				fileName = fileName.substring(0, index);
				index = fileName.lastIndexOf(PATH_SEPARATOR);
			}

			// seperate directory and file
			if ((index >= 0) && ((fileName.substring(index + 1).length()) > 0)) {
				// have to run setDirectory(...) again
				// because they put filepath info in the filename
				setDirectory(fileName.substring(0, index + 1));
				fileName = fileName.substring(index + 1);
			}
		}
	}

	/**
	 * Set the directory.
	 * 
	 * @param dir
	 *            The directory path, need not be absolute.
	 */
	// though everything will be converted to a canonical path to avoid errors
	protected void setDirectory(String dir) {
		if (directory == null) {
			directory = new Vector();
		}
		if (dir == null) {
			return;
		}

		// in case they used the local pathSeperator
		// in the fileName instead of the iRODS PATH_SEPARATOR.
		String localSeparator = System.getProperty("file.separator");
		int index = dir.lastIndexOf(localSeparator);
		if ((index >= 0) && ((dir.substring(index + 1).length()) > 0)) {
			dir = dir.substring(0, index) + PATH_SEPARATOR
					+ dir.substring(index + 1);
			index = dir.lastIndexOf(localSeparator);
		}

		while ((directory.size() > 0) && // only if this is the dir cut from
				// fileName
				dir.startsWith(PATH_SEPARATOR))// && //strip these
		// (dir.length() > 1)) //but not if they only wanted
		{
			dir = dir.substring(1);
			// problems if dir passed from filename starts with PATH_SEPARATOR
		}

		// create directory vector
		index = dir.indexOf(PATH_SEPARATOR);

		if (index >= 0) {
			do {
				directory.add(dir.substring(0, index));
				do {
					dir = dir.substring(index + 1);
					index = dir.indexOf(PATH_SEPARATOR);
				} while (index == 0);
			} while (index >= 0);
		}
		// add the last path item
		if ((!dir.equals("")) && (dir != null)) {
			directory.add(dir);
		}
	}

	/**
	 * Helper for setting the directory to an absolute path
	 * 
	 * @param dir
	 *            Used to determine if the path is absolute.
	 */
	
	void makePathCanonical(String dir) {
		int i = 0; // where to insert into the Vector
		boolean absolutePath = false;
		String canonicalTest = null;

		if (dir == null) {
			dir = "";
		}

		// In case this abstract path is supposed to be root
		if ((fileName.equals(IRODSFileSystem.IRODS_ROOT) || fileName.equals(""))
				&& dir.equals("")) {
			return;
		}

		// In case this abstract path is supposed to be the home directory
		if (fileName.equals("") && dir.equals("")) {
			String home = iRODSFileSystem.getHomeDirectory();
			int index = home.lastIndexOf(separator);
			setDirectory(home.substring(0, index));
			setFileName(home.substring(index + 1));
			return;
		}

		// if dir not absolute
		if (dir.startsWith(iRODSFileSystem.IRODS_ROOT))
			absolutePath = true;

		// if directory not already absolute
		if (directory.size() > 0) {
			if (directory.get(0).toString().length() == 0) {
				// The /'s were all striped when the vector was created
				// so if the first element of the vector is null
				// but the vector isn't null, then the first element
				// is really a /.
				absolutePath = true;
			}
		}
		if (!absolutePath) {
			String home = iRODSFileSystem.getHomeDirectory();
			int index = home.indexOf(separator);
			// allow the first index to = 0,
			// because otherwise separator won't get added in front.
			if (index >= 0) {
				do {
					directory.add(i, home.substring(0, index));
					home = home.substring(index + 1);
					index = home.indexOf(separator);
					i++;
				} while (index > 0);
			}
			if ((!home.equals("")) && (home != null)) {
				directory.add(i, home);
			}
		}

		// first, made absolute, then canonical
		for (i = 0; i < directory.size(); i++) {
			canonicalTest = directory.get(i).toString();
			if (canonicalTest.equals(".")) {
				directory.remove(i);
				i--;
			} else if ((canonicalTest.equals("..")) && (i >= 2)) {
				directory.remove(i);
				directory.remove(i - 1);
				i--;
				if (i > 0)
					i--;
			} else if (canonicalTest.equals("..")) {
				// at root, just remove the ..
				directory.remove(i);
				i--;
			} else if (canonicalTest.startsWith(separator)) {
				// if somebody put filepath as /foo//bar or /foo////bar
				do {
					canonicalTest = canonicalTest.substring(1);
				} while (canonicalTest.startsWith(separator));
				directory.remove(i);
				directory.add(i, canonicalTest);
			}
		}
		// also must check fileName
		if (fileName.equals(".")) {
			fileName = directory.get(directory.size() - 1).toString();
			directory.remove(directory.size() - 1);
		} else if (fileName.equals("..")) {
			if (directory.size() > 1) {
				fileName = directory.get(directory.size() - 2).toString();
				directory.remove(directory.size() - 1);
				directory.remove(directory.size() - 1);
			} else {
				// at root
				fileName = separator;
				directory.remove(directory.size() - 1);
			}
		}
	}

	/**
	 * This abstract method gets the path separator as defined by the subclass.
	 */
	public String getPathSeparator() {
		return PATH_SEPARATOR;
	}

	/**
	 * This abstract method gets the path separator char as defined by the
	 * subclass.
	 */
	public char getPathSeparatorChar() {
		return PATH_SEPARATOR_CHAR;
	}

	
	/**
	 * Copies this file to another file. This object is the source file. The
	 * destination file is given as the argument. If the destination file, does
	 * not exist a new one will be created. Otherwise the source file will be
	 * appended to the destination file. Directories will be copied recursively.
	 * 
	 * @param file
	 *            The file to receive the data.
	 * @throws NullPointerException
	 *             If file is null.
	 * @throws IOException
	 *             If an IOException occurs.
	 */

	public void copyTo(GeneralFile file, boolean forceOverwrite)
			throws IOException {
		copyTo(file, forceOverwrite, "");
	}

	/**
	 * Copy a file from IRODS (get) specifying a particular resource. This is
	 * equivalent to an iget with a -R
	 * 
	 * Copies this file to another file. This object is the source file. The
	 * destination file is given as the argument. If the destination file, does
	 * not exist a new one will be created. Otherwise the source file will be
	 * appended to the destination file. Directories will be copied recursively.
	 * 
	 * @param file
	 *            {@link GeneralFile GeneralFile} is the local file that will be
	 *            copied to
	 * @param forceOverwrite
	 * @param resource
	 *            <code>String</code> containing the name of the resource from
	 *            which the source file will be copied.
	 * @throws IOException
	 */
	public void copyTo(GeneralFile file, boolean forceOverwrite, String resource)
			throws IOException {
		if (log.isInfoEnabled()) {
			log.info("copying file:" + this.getAbsolutePath() + " to file:" + file.getAbsolutePath());
		}
		if (file == null) {
			log.error("dest file is null");
			throw new IllegalArgumentException("file cannot be null");
		}

		if (resource == null) {
			log.error("resource is null");
			throw new IllegalArgumentException(
					"resource cannot be null, set to blank if not used");
		}

		if (isDirectory()) {
			log.info("    this is a directory, will recursively copy");
			// recursive copy
			GeneralFile[] fileList = listFilesNext(true);

			file.mkdir();
			while (fileList != null && fileList.length > 0) {
				for (int i = 0; i < fileList.length; i++) {
					fileList[i].copyTo(FileFactory.newFile(
							file.getFileSystem(), file.getAbsolutePath(),
							fileList[i].getName()), forceOverwrite);
				}
				fileList = listFilesNext(false);
			}
		} else {
			if (!forceOverwrite && file.isDirectory()) {
				log.info("no force overwrite an dest file is directory");
				// change the destination from a directory to a file
				file = FileFactory.newFile(file, getName());
			}
			try {
				if (file instanceof LocalFile) {
					if (file.exists()) {
						if (forceOverwrite) {
							log.info("deleting a local file because forceOverwrite was specified");
							file.delete();
						} else {
							log.error("file:" + file.getAbsolutePath() + " already exists, and overwriting was not allowed");
							throw new IOException(
									"File exists and overwriting not allowed");
						}
					}

					iRODSFileSystem.commands.get(this, file, resource);

				} else if (file instanceof IRODSFile) {
					log.info("dest is an IRODS file");
					iRODSFileSystem.commands.copy(this, (IRODSFile) file,
							forceOverwrite);
				} else {
					super.copyTo(file, forceOverwrite);
				}
			} catch (IRODSException e) {
				log.error("IRODSException in coptyTo operation for file:" + this.getAbsolutePath() + " type is:" + e.getType());
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
	 * @param file
	 *            The file to receive the data.
	 * @throws NullPointerException
	 *             If file is null.
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	public void copyFrom(final GeneralFile file, final boolean forceOverwrite)
			throws IOException {
		if (log.isInfoEnabled()) {
			log.info("copy of:" + file.getAbsolutePath() + " to:" + this.getAbsolutePath());
		}
		if (file == null) {
			throw new NullPointerException();
		}

		if (file.isDirectory()) {
			// recursive copy
			GeneralFile[] fileList = file.listFiles();

			mkdir();
			if (fileList != null) {
				for (int i = 0; i < fileList.length; i++) {
					FileFactory.newFile(this, fileList[i].getName()).copyFrom(
							fileList[i], forceOverwrite);
				}
			}
		} else {
			if (!forceOverwrite && isDirectory()) {
				// change the destination from a directory to a file
				GeneralFile subFile = FileFactory.newFile(this, file.getName());
				subFile.copyFrom(file);
				return;
			}
			try {
				if (file instanceof LocalFile) {
					iRODSFileSystem.commands.put(file, this, forceOverwrite);
				} else if (file instanceof IRODSFile) {
					iRODSFileSystem.commands.copy((IRODSFile) file, this,
							forceOverwrite);
				} else {
					super.copyTo(file, forceOverwrite);
				}
			} catch (IRODSException e) {
				log.error("exception in copyFrom, rethrown as IOException for File contract", e);
				IOException io = new IOException();
				io.initCause(e);
				throw io;
			}
		}
	}

	/**
	 * iRODS does md5 by default.
	 * 
	 * @return the md5 string for this file
	 * @throws java.io.IOException
	 */
	protected String checksumMD5() throws IOException {
		return iRODSFileSystem.commands.checksum(this);
	}
	
	/**
	 * Queries the file server to find all files that match a set of conditions.
	 * For all those that match, the fields indicated in the select array are
	 * returned in the result object.
	 * 
	 * This will issue the query with 'distinct' being the default query type
	 * 
	 * @param conditions
	 *            {@link edu.sdsc.grid.io.MetaDataCondition MetaDataCondition}
	 *            containing the query conditions
	 * @param selects
	 *            {@link edu.sdsc.grid.io.MetaDataSelect MetaDataSelect}
	 *            containing the fields to query
	 * @param numberOfRecordsWanted
	 *            <code>int</code> containing the number of records to return
	 *            (per request). Note that <code>MetaDataRecordList</code> has
	 *            the facility to re-query for more results
	 * @param namespace
	 *            (@link edu.sdsc.grid.io.Namespace Namespace} that describes
	 *            the particular object type (e.g. Resource, Collection, User)
	 * @return {@link edu.sdsc.grid.io.MetaDataRecordList MetaDataRecordList}
	 *         containing the results, and the ability to requery.
	 * @throws IOException
	 */
	public MetaDataRecordList[] query(MetaDataCondition[] conditions,
			MetaDataSelect[] selects) throws IOException {
		 return query(conditions, selects, true);
	}

	/**
	 * Queries the file server to find all files that match a set of conditions.
	 * For all those that match, the fields indicated in the select array are
	 * returned in the result object.
	 *<P>
	 * While condition and select array objects have all been checked for
	 * self-consistency during their construction, there are additional problems
	 * that must be detected at query time:
	 *<P>
	 * <ul>
	 * <li>Redundant selection fields
	 * <li>Redundant query fields
	 * <li>Fields incompatible with a file server
	 * </ul>
	 *<P>
	 * For instance, it is possible to build a condition object appropriate for
	 * the iRODS, then pass that object in a local file system query. That will
	 * find that the condition is incompatible and generate a mismatch
	 * exception.
	 *<P>
	 * Query is implemented by the file-server-specific classes, like that for
	 * the iRODS, FTP, etc. Those classes must re-map condition and select field
	 * names and operator codes to those required by a particular file server
	 * and protocol version. Once re-mapped, they issue the query and get
	 * results. The results are then mapped back to the standard public field
	 * names of the MetaDataGroups. So, if a MetaDataGroup uses a name like
	 * "file path", but the iRODS calls it "data name", then query maps first
	 * from "file path" to "data name" before issuing the query, and then from
	 * "data name" back to "file path" within the results. The programmer using
	 * this API should never see the internal field names.
	 * @param conditions
	 *            {@link edu.sdsc.grid.io.MetaDataCondition MetaDataCondition}
	 *            containing the query conditions
	 * @param selects
	 *            {@link edu.sdsc.grid.io.MetaDataSelect MetaDataSelect}
	 *            containing the fields to query
	 * @param numberOfRecordsWanted
	 *            <code>int</code> containing the number of records to return
	 *            (per request). Note that <code>MetaDataRecordList</code> has
	 *            the facility to re-query for more results
	 * @param namespace
	 *            (@link edu.sdsc.grid.io.Namespace Namespace} that describes
	 *            the particular object type (e.g. Resource, Collection, User)
	 *            being queried
	 * @param distinctQuery
	 *            <code>boolean</code> that will cause the query to eith0er
	 *            select 'distinct' or select all. A <code>true</code> value
	 *            will select distinct.
	 * @return {@link edu.sdsc.grid.io.MetaDataRecordList MetaDataRecordList}
	 *         containing the results, and the ability to requery.
	 * @throws IOException
	 */
	public MetaDataRecordList[] query(MetaDataCondition[] conditions,
			MetaDataSelect[] selects, boolean distinctQuery) throws IOException {
		
		MetaDataCondition iConditions[] = null;
		String fieldName = null;
		int operator = MetaDataCondition.EQUAL;
		String value = null;
		int conditionsLength = 0;
		if (conditions == null) {
			conditions = new MetaDataCondition[0];
		} else {
			conditionsLength = conditions.length;
		}

		if (isDirectory()) {
			iConditions = new MetaDataCondition[conditionsLength + 1];

			System.arraycopy(conditions, 0, iConditions, 0, conditionsLength);

			fieldName = GeneralMetaData.DIRECTORY_NAME;
			operator = MetaDataCondition.EQUAL;
			value = getAbsolutePath();
			iConditions[conditionsLength] = MetaDataSet.newCondition(fieldName,
					operator, value);

			return iRODSFileSystem
					.query(iConditions, selects,
							IRODSFileSystem.DEFAULT_RECORDS_WANTED,
							Namespace.DIRECTORY, distinctQuery);
		} else {
			iConditions = new MetaDataCondition[conditionsLength + 3];

			System.arraycopy(conditions, 0, iConditions, 0, conditionsLength);

			fieldName = GeneralMetaData.DIRECTORY_NAME;
			operator = MetaDataCondition.EQUAL;
			value = getParent();
			iConditions[conditionsLength] = MetaDataSet.newCondition(fieldName,
					operator, value);

			fieldName = GeneralMetaData.FILE_NAME;
			value = fileName;
			iConditions[conditionsLength + 1] = MetaDataSet.newCondition(
					fieldName, operator, value);

			// else last condition = null, will be ignored

			return fileSystem.query(iConditions, selects);
		}
	}

	/**
	 * Queries metadata specific to this file object, selecting one metadata
	 * value, <code>fieldName</code>, and returns the first result of that
	 * query. Returns null if the query had no results.
	 * 
	 * @param fieldName
	 *            The string name used to form the select object.
	 * @return The metadata values for this file refered to by
	 *         <code>fieldName</code>
	 */
	public String firstQueryResult(String fieldName) throws IOException {
		try {
			MetaDataSelect[] temp = { MetaDataSet.newSelection(fieldName) };
			MetaDataRecordList[] rl = query(temp);
			if (rl != null) {
				return rl[0].getStringValue(0);
			}
		} catch (IllegalArgumentException e) {
			log.error("illegal arg exception", e);
		}
		return null;
	}

	/**
	 * Used to modify the metadata associated with this file object. Does not
	 * overwrite. If an already existing value conflicts, inserts new metadata
	 * value. If duplicate metadata is reentered, no action is taken.
	 * 
	 * @param values
	 *            <code>String[]</code> containing an AVU in the form (attrib
	 *            name, attrib value) or (attrib name, attrib value, attrib
	 *            units)
	 */
	public void modifyMetaData(String[] metaData) throws IOException {
		if (metaData.length < 2 || metaData.length > 3) {
			throw new IllegalArgumentException(
					"metadata length must be 2 (name and value) or 3 (name, value, units) ");
		}

		if (metaData[0].equals("") || metaData[1].equals("")) {
			throw new IllegalArgumentException(
					"The metadata attribute and value " + "cannot be empty.");
		}

		try {
			iRODSFileSystem.commands.modifyMetaData(this, metaData);
		} catch (IRODSException e) {
			int error = e.getType()
					- IRODSException.CAT_SUCCESS_BUT_WITH_NO_INFO;
			if (error >= 0 && error < 1000) {
				// ignore, the metadata already exists.
				// (The <1000 part is something else ignorable.)

				log.warn("metadata already exists, logged and ignored");
				return;
			}
			error = e.getType()
					- IRODSException.CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME;
			if (error >= 0 && error < 1000) {
				// ignore, the metadata already exists.
				// (The <1000 part is something else ignorable.)

				log.warn("metadata already exists, logged and ignored");
				return;
			}

			throw e;
		}
	}

	/**
	 * Deletes this <code>metadata</code> associated with this file object. The
	 * String array <code>metadata</code> must include the complete AVU to be
	 * deleted.
	 * 
	 * Metadata strings may contain wildcards: % allows you to match any string
	 * of any length (including zero length). _ allows you to match on a single
	 * character.
	 */
	public void deleteMetaData(String[] metaData) throws IOException {
		if (metaData == null && metaData.length < 2) {
			throw new IllegalArgumentException(
					"The metadata must contain at least "
							+ "an attribute and value.");
		} else if (metaData[0].equals("") && metaData[1].equals("")) {
			throw new IllegalArgumentException(
					"The metadata attribute and value " + "cannot be empty.");
		}
		iRODSFileSystem.commands.deleteMetaData(this, metaData);
	}

	/**
	 * Deletes all <code>metadata</code> associated with this file object.
	 */
	void deleteMetaData() throws IOException {
		throw new RuntimeException("not implemented");
	}

	
	/**
	 * Sets the physical resource this IRODSFile object will be stored on. If
	 * null, a default resource will be chosen by the irods server.
	 * 
	 * This setter refers to the object parameter only, to move a file to a new
	 * physical resource see renameTo(GeneralFile)
	 * 
	 * Note that the fileSystem query for ResourceMetaData.RESOURCE_NAME will fail if
	 * there are no files in a particular resource, due to the way that the query is put together.
	 * 
	 * @param resource
	 *            The name of resource to be used.
	 * @throws IllegalArgumentException
	 *             If resourceName is not a valid resource.
	 * @throws IOException
	 *             If an IOException occurs during the system change.
	 */
	public void setResource(String resourceName) throws IOException {
		
		if (resourceName == null) {
			throw new IllegalArgumentException("resourceName is null");
		}
		
		if (resourceName != null) {
			// Make sure valid resource
			MetaDataRecordList[] rl = fileSystem.query(MetaDataSet
					.newSelection(ResourceMetaData.RESOURCE_NAME));
			
			if (rl == null) {  
				log.warn("no resources returned from query, accept the given resource:" + resourceName);
				resource = resourceName;
				return;
			}
			
			for (int i = rl.length - 1; i >= 0; i--) {
				if (resourceName.equals(rl[i].getStringValue(0))) {
					resource = resourceName;
					return;
				}
			}
			throw new IllegalArgumentException("The resource, " + resourceName
					+ ", does not exist");
		}
	}

	/**
	 * Return the first physical resource found where the file is stored, if
	 * available. Otherwise, will return the default resource set by the
	 * IRODSAccount, if available. Otherwise, will return null.
	 * 
	 * @return <code>String</code> containing the resource for the file
	 * 
	 * @throws IOException
	 *             If an IOException occurs during the system query.
	 */
	public String getResource() throws IOException {
		// I may have set the resource already
		if (resource == null) {
			// for files, get the actual resource associated with the file,
			// otherwise,
			// get any default set by the IRODS account
			if (this.isFile()) {
				resource = firstQueryResult(ResourceMetaData.RESOURCE_NAME);
			} else {
				resource = ((IRODSFileSystem) fileSystem)
						.getDefaultStorageResource();
			}
		} else {
			// note that there is some inconsistency between nulls and "" values
			// for resource, try and
			// standardize on null. This probably needs more work.
		}

		return resource;

	}

	/**
	 * For a file, get all of the resources where the file is stored. If called
	 * on a collection, will return an empty list.
	 * 
	 * @return <code>List<String></code> with the resource names for a file.
	 * @throws IOException
	 */
	public List<String> getAllResourcesForFile() throws IOException {
		List<String> resources = new ArrayList<String>();

		if (isFile()) {
			MetaDataRecordList[] lists = query(new String[] { IRODSMetaDataSet.RESOURCE_NAME });

			for (MetaDataRecordList l : lists) {
				resources.add(l.getStringValue(0));
			}
		}

		return resources;

	}

	/**
	 * @return dataType The dataType string of this file. Will not query the
	 *         server if this abstract pathname is a directory. Returns null if
	 *         the file does not exist.
	 * 
	 * @throws IOException
	 *             If an IOException occurs during the system query.
	 */
	public String getDataType() throws IOException {
		if (dataType != null) {
			return dataType;
		} else if (isDirectory()) {
			return null;
		}

		dataType = firstQueryResult(IRODSMetaDataSet.FILE_TYPE);
		if (dataType != null)
			return dataType;

		return "generic";
	}

	public void replicate(String newResource) throws IOException {
		iRODSFileSystem.commands.replicate(this, newResource);
	}

	/**
	 * Change the permissions for this IRODSFile. May throw IRODSException if
	 * attempting to change permissions inapproriate to object type, e.g.
	 * setting a file to "inherit".
	 * <P>
	 * 
	 * @param permission
	 *            "w" - write;"r" - read;"own" or "all" - owner;"n" - null;
	 * @param userName
	 *            The permissions are changed for this user. The user's zone may
	 *            also be included in the username, seperated by #, i.e.
	 *            username#zonename
	 * @param recursive
	 *            Changes this and all subdirectories
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	public void changePermissions(String permission, String userName,
			boolean recursive) throws IOException {
		int index = userName.indexOf('#');
		String zone = "";
		if (index > 0) { // username can't be of zero length
			zone = userName.substring(index + 1);
			userName = userName.substring(0, index);
		}

		if (permission == null) {
			permission = "";
		}

		permission = permission.toLowerCase();

		if (permission.equals("n") || permission.equals("")) // or "" or null
		{
			permission = "null";
		} else if (permission.equals("r") || permission.equals("read")) {
			permission = "read";
		} else if (permission.equals("w") || permission.equals("write")) {
			permission = "write";
		} else if (permission.equals("all") || permission.equals("ownership")
				|| permission.equals("own") || permission.equals("o")) {
			permission = "own";
		} 

		iRODSFileSystem.commands.chmod(this, permission, userName, zone,
				recursive);
	}

	/**
	 * Tests whether the application can read the file denoted by this abstract
	 * pathname.
	 * 
	 * @return <code>true</code> if and only if the file specified by this
	 *         abstract pathname exists <em>and</em> can be read; otherwise
	 *         <code>false</code>.
	 */
	public boolean canRead() {
		return canRead(true);
	}

	boolean canRead(boolean update) {
		if (update || (pathNameType == PATH_IS_UNKNOWN)
				|| pathNameType == PATH_IS_FILE) {
			try {
				if (filePermissions() >= READ_PERMISSIONS) {
					pathNameType = PATH_IS_FILE;
					return true;
				}
			} catch (IOException e) {
				log.warn("io exception is logged and ignored", e);
			}
		}

		// if this far, then not a file
		if (pathNameType == PATH_IS_UNKNOWN
				|| pathNameType == PATH_IS_DIRECTORY) {
			try {
				if (directoryPermissions() >= READ_PERMISSIONS) {
					pathNameType = PATH_IS_DIRECTORY;
					return true;
				}
			} catch (IOException e) {
				log.warn("io exception is logged and ignored", e);
			}
		}
		return false;
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
	public boolean canWrite() {
		return canWrite(true);
	}

	boolean canWrite(boolean update) {
		if (update || (pathNameType == PATH_IS_UNKNOWN)
				|| pathNameType == PATH_IS_FILE) {
			try {
				if (filePermissions() >= WRITE_PERMISSIONS) {
					pathNameType = PATH_IS_FILE;
					return true;
				}
			} catch (IOException e) {
				log.warn("io exception is logged and ignored", e);
			}
		}

		// if this far, then not a file
		if (pathNameType == PATH_IS_UNKNOWN
				|| pathNameType == PATH_IS_DIRECTORY) {
			try {
				if (directoryPermissions() >= WRITE_PERMISSIONS) {
					pathNameType = PATH_IS_DIRECTORY;
					return true;
				}
			} catch (IOException e) {
				log.warn("io exception is logged and ignored", e);
			}
		}
		return false;
	}

	/**
	 * Get the permissions of the current user for this file: write, read, all,
	 * or null.
	 * <P>
	 * 
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	public String getPermissions() throws IOException {
		int permit = filePermissions();
		if (permit < 0)
			permit = directoryPermissions();

		if (permit >= READ_PERMISSIONS) {
			if (permit >= WRITE_PERMISSIONS) {
				return "all";
			}
			return "read";
		}
		return null;
	}

	private int filePermissions() throws IOException {
		int permissions = 0;
		String zone = iRODSFileSystem.getZone();
		MetaDataCondition conditions[] = {
				MetaDataSet.newCondition(GeneralMetaData.DIRECTORY_NAME,
						MetaDataCondition.EQUAL, getParent()),
				MetaDataSet.newCondition(GeneralMetaData.FILE_NAME,
						MetaDataCondition.EQUAL, fileName),
				MetaDataSet.newCondition(
						IRODSMetaDataSet.FILE_PERMISSION_USER_NAME,
						MetaDataCondition.EQUAL, iRODSFileSystem.getUserName()),
				// if zone available
				(zone != null && !zone.equals("")) ? MetaDataSet.newCondition(
						IRODSMetaDataSet.FILE_PERMISSION_USER_ZONE,
						MetaDataCondition.EQUAL, zone) : null };

		MetaDataSelect selects[] = { MetaDataSet.newSelection(
				IRODSMetaDataSet.FILE_ACCESS_TYPE, 1024) };

		MetaDataRecordList[] rl = fileSystem.query(conditions, selects);
		if (rl != null) {
			// return highest value
			for (MetaDataRecordList record : rl) {
				if (record.getIntValue(0) > permissions)
					permissions = record.getIntValue(0);
			}
		} else
			return -1;

		return permissions;
	}

	private int directoryPermissions() throws IOException {
		int permissions = 0;
		String zone = iRODSFileSystem.getZone();
		MetaDataCondition conditions[] = {
				MetaDataSet.newCondition(GeneralMetaData.DIRECTORY_NAME,
						MetaDataCondition.EQUAL, getAbsolutePath()),
				MetaDataSet.newCondition(IRODSMetaDataSet.DIRECTORY_USER_NAME,
						MetaDataCondition.EQUAL, iRODSFileSystem.getUserName()),
				// if zone available
				(zone != null && !zone.equals("")) ? MetaDataSet.newCondition(
						IRODSMetaDataSet.DIRECTORY_USER_ZONE,
						MetaDataCondition.EQUAL, zone) : null };

		MetaDataSelect selects[] = { MetaDataSet.newSelection(
				IRODSMetaDataSet.DIRECTORY_ACCESS_TYPE, 1024) };

		MetaDataRecordList[] rl = fileSystem.query(conditions, selects);
		if (rl != null) {
			// return highest value
			for (MetaDataRecordList record : rl) {
				if (record.getIntValue(0) > permissions)
					permissions = record.getIntValue(0);
			}
		} else
			return -1;

		return permissions;
	}

	/**
	 * Atomically creates a new, empty file named by this abstract pathname if
	 * and only if a file with this name does not yet exist. The check for the
	 * existence of the file and the creation of the file if it does not exist
	 * are a single operation that is atomic with respect to all other
	 * filesystem activities that might affect the file.
	 * <P>
	 * Note: this method should <i>not</i> be used for file-locking, as the
	 * resulting protocol cannot be made to work reliably.
	 * 
	 * @return <code>true</code> if the named file does not exist and was
	 *         successfully created; <code>false</code> if the named file
	 *         already exists
	 * 
	 * @throws IOException
	 *             If an I/O error occurred
	 */
	public boolean createNewFile() throws IOException {
		try {
			if (!isFile()) {
				getParentFile().mkdirs();

				int fd = iRODSFileSystem.commands
						.fileCreate(this, false, false);

				// Be sure to close files after a create() or open().
				iRODSFileSystem.commands.fileClose(fd);
				return true;
			}
		} catch (IRODSException e) {
			log.warn("io exception is logged and ignored", e);
			// catch already exists (as a directory probably) and just return
			// false
			if (e.getType() != -511017) {
				log
						.error("irods exception when creating new file, is not an already exists exception");
				throw e;
			}
		}

		return false;
	}

	/**
	 * Used by RandomAccessFile and streams
	 * 
	 * @param rw
	 *            true if read-write, false if read-only
	 * @return the irods file descriptor, so doesn't need to be opened again.
	 *         returns -1 if false, on error, or if file exists.
	 */
	int createNewFile(boolean fd, boolean rw) throws IOException {
		if (!fd) {
			createNewFile();
			return -1;
		}
		try {
			if (!isFile()) {
				getParentFile().mkdirs();

				if (rw) {
					return iRODSFileSystem.commands
							.fileCreate(this, true, true);
				} else {
					return iRODSFileSystem.commands.fileCreate(this, true,
							false);
				}
			}
		} catch (IRODSException e) {
			log.warn("io exception is logged and ignored", e);
		}

		return -1;
	}

	/**
	 * <p>
	 * Creates a new empty file in the specified directory, using the given
	 * prefix and suffix strings to generate its name. If this method returns
	 * successfully then it is guaranteed that:
	 * 
	 * <ol>
	 * <li>The file denoted by the returned abstract pathname did not exist
	 * before this method was invoked, and
	 * <li>Neither this method nor any of its variants will return the same
	 * abstract pathname again in the current invocation of the virtual machine.
	 * </ol>
	 * 
	 * This method provides only part of a temporary-file facility. To arrange
	 * for a file created by this method to be deleted automatically, use the
	 * <code>{@link #deleteOnExit}</code> method.
	 * 
	 * <p>
	 * The <code>prefix</code> argument must be at least three characters long.
	 * It is recommended that the prefix be a short, meaningful string such as
	 * <code>"hjb"</code> or <code>"mail"</code>. The <code>suffix</code>
	 * argument may be <code>null</code>, in which case the suffix
	 * <code>".tmp"</code> will be used.
	 * 
	 * <p>
	 * To create the new file, the prefix and the suffix may first be adjusted
	 * to fit the limitations of the underlying platform. If the prefix is too
	 * long then it will be truncated, but its first three characters will
	 * always be preserved. If the suffix is too long then it too will be
	 * truncated, but if it begins with a period character (<code>'.'</code>)
	 * then the period and the first three characters following it will always
	 * be preserved. Once these adjustments have been made the name of the new
	 * file will be generated by concatenating the prefix, five or more
	 * internally-generated characters, and the suffix.
	 * 
	 * <p>
	 * If the <code>directory</code> argument is <code>null</code> then the
	 * default temporary-file directory will be used. Since iRODS does not have
	 * a standard temporary directory, files will be placed in a temp/ directory
	 * in the user's iRODS home directory. There are certain difficulties
	 * creating a static connection to the iRODS. For this static method to
	 * connect to the iRODS, .Mdas files must be available in the local home
	 * directory/.irods. That is the information that will be used when storing
	 * the temporary file. This comprimise is necessary to maintain the designs
	 * unity with the java.io.File class.
	 * 
	 * @param prefix
	 *            The prefix string to be used in generating the file's name;
	 *            must be at least three characters long
	 * 
	 * @param suffix
	 *            The suffix string to be used in generating the file's name;
	 *            may be <code>null</code>, in which case the suffix
	 *            <code>".tmp"</code> will be used
	 * 
	 * @param directory
	 *            The directory in which the file is to be created, or
	 *            <code>null</code> if the default temporary-file directory is
	 *            to be used
	 * 
	 * @return An abstract pathname denoting a newly-created empty file
	 * 
	 * @throws IllegalArgumentException
	 *             If the <code>prefix</code> argument contains fewer than three
	 *             characters
	 * 
	 * @throws IOException
	 *             If a file could not be created
	 */
	public static GeneralFile createTempFile(String prefix, String suffix,
			GeneralFile directory) throws IOException, IllegalArgumentException {
		String randomChars = "";
		for (int i = 0; i < 8; i++)
			randomChars += ((char) (65 + Math.random() * 25));

		if (prefix == null)
			throw new NullPointerException();
		if (prefix.length() < 3)
			throw new IllegalArgumentException("Prefix string too short");

		if (suffix == null)
			suffix = ".tmp";

		if (directory == null) {
			IRODSFileSystem fs = new IRODSFileSystem();
			directory = FileFactory.newFile(fs, fs.getHomeDirectory(), "temp");
			directory.mkdir();
		}

		GeneralFile temp = FileFactory.newFile(directory, prefix + randomChars
				+ suffix);

		if (temp.createNewFile())
			return temp;
		else {
			log.error("io exception, temp file already exists");
			throw new IOException("The temp file already exists.");
		}
	}

	/**
	 * Deletes the file or directory denoted by this abstract pathname. If this
	 * pathname denotes a directory, then the directory must be empty in order
	 * to be deleted.
	 * 
	 * @return <code>true</code> if and only if the file or directory is
	 *         successfully deleted; <code>false</code> otherwise
	 */
	public boolean delete() {
		return delete(false);
	}

	/**
	 * Deletes the file or directory denoted by this abstract pathname. If this
	 * pathname denotes a directory, then the directory must be empty in order
	 * to be deleted.
	 * 
	 * @return <code>true</code> if and only if the file or directory is
	 *         successfully deleted; <code>false</code> otherwise
	 */
	public boolean delete(boolean force) {
		try {
			if (isDirectory()) {
				if (log.isDebugEnabled()) {
					log.debug("deleting a directory:" + this.getAbsolutePath() + " with force option of " + force);
				}
				iRODSFileSystem.commands.deleteDirectory(this, force);
			} else if (isFile(false)) {
				if (log.isDebugEnabled()) {
					log.debug("deleting a file:" + this.getAbsolutePath() + " with force option of " + force);
				}
				iRODSFileSystem.commands.deleteFile(this, force);
			}
			return true;
		} catch (IOException e) {
			log.warn("io exception is logged and ignored", e);
			return false;
		}
	}

	/**
	 * Deletes only the replica of this file named in the <code>resource</code>.
	 * Nothing occurs if this pathname denotes a directory, returns false.
	 * 
	 * @param resource
	 *            Name of the resource where the file is deleted from, returns
	 *            false if resource is null or does not exist.
	 * @return <code>true</code> if and only if the file successfully deleted;
	 *         <code>false</code> otherwise
	 */
	public boolean deleteReplica(String resource) {
		try {
			if (resource == null)
				return false;

			if (isFile()) {
				iRODSFileSystem.commands.deleteReplica(this, resource);
			}
		} catch (IOException e) {
			log.warn("io exception is logged and ignored", e);
		}

		return false;
	}

	/**
	 * Requests that the file or directory denoted by this abstract pathname be
	 * deleted when the virtual machine terminates. Deletion will be attempted
	 * only for normal termination of the virtual machine, as defined by the
	 * Java Language Specification.
	 * 
	 * <p>
	 * Once deletion has been requested, it is not possible to cancel the
	 * request. This method should therefore be used with care.
	 * 
	 * <P>
	 * Note: this method should <i>not</i> be used for file-locking, as the
	 * resulting protocol cannot be made to work reliably.
	 */
	public void deleteOnExit() {
		deleteOnExit = true;
	}

	/**
	 * @return This abstract pathname as a pathname string.
	 */
	public String getPath() {
		return getAbsolutePath();
	}

	/**
	 * Tests this abstract pathname for equality with the given object. Returns
	 * <code>true</code> if and only if the argument is not <code>null</code>
	 * and is an abstract pathname that denotes the same file or directory as
	 * this abstract pathname on the same filesystem. Does not compare other
	 * user or host information of the filesystems.
	 * 
	 * @param obj
	 *            The object to be compared with this abstract pathname
	 * 
	 * @return <code>true</code> if and only if the objects are the same;
	 *         <code>false</code> otherwise
	 */
	public boolean equals(Object obj) {
		try {
			if (obj == null)
				return false;

			if (obj instanceof IRODSFile) {
				IRODSFile temp = (IRODSFile) obj;
				return getAbsolutePath().equals(temp.getAbsolutePath());
			}
		} catch (ClassCastException e) {
			log.warn("io exception is logged and ignored", e);
		}
		return false;
	}

	/**
	 * Tests whether the file denoted by this abstract pathname exists.
	 * 
	 * @return <code>true</code> if and only if the file denoted by this
	 *         abstract pathname exists; <code>false</code> otherwise
	 */
	public boolean exists() {
		try {
			MetaDataRecordList[] rl = null;
			int operator = MetaDataCondition.EQUAL;

			// if it is a file
			MetaDataCondition conditions[] = null;

			conditions = new MetaDataCondition[2];
			conditions[0] = MetaDataSet.newCondition(
					GeneralMetaData.DIRECTORY_NAME, operator, getParent());
			conditions[1] = MetaDataSet.newCondition(GeneralMetaData.FILE_NAME,
					operator, getName());
			// }

			MetaDataSelect selects[] = { MetaDataSet
					.newSelection(GeneralMetaData.FILE_NAME) };

			rl = fileSystem.query(conditions, selects, 3);

			if (rl != null)
				return true;

			// if it is a directory
			conditions = new MetaDataCondition[1];
			conditions[0] = MetaDataSet
					.newCondition(GeneralMetaData.DIRECTORY_NAME, operator,
							getAbsolutePath());
			selects[0] = MetaDataSet
					.newSelection(GeneralMetaData.DIRECTORY_NAME);
			rl = fileSystem.query(conditions, selects, 3);

			if (rl != null)
				return true;

		} catch (IOException e) {
			log.warn("io exception is logged and ignored", e);
		}

		return false;
	}

	/**
	 * Returns the canonical pathname string of this abstract pathname.
	 * 
	 * @return The canonical pathname string denoting the same file or directory
	 *         as this abstract pathname
	 * 
	 * @throws IOException
	 *             If an I/O error occurs, which is possible because the
	 *             construction of the canonical pathname may require filesystem
	 *             queries
	 */
	public String getCanonicalPath() throws IOException {
		if ((directory != null) && (!directory.isEmpty())) {
			int size = directory.size();
			String path = (String) directory.firstElement();
			int i = 1;

			while (i < size) {
				path += separator + directory.get(i);
				i++;
			}

			return path + separator + fileName;
		}

		return fileName;
	}

	/**
	 * Computes a hash code for this abstract pathname. The hash code of an
	 * abstract pathname is equal to the exclusive <em>or</em> of its pathname
	 * string and the decimal value <code>1234321</code>.
	 * 
	 * @return A hash code for this abstract pathname
	 */
	public int hashCode() {
		return getAbsolutePath().toLowerCase().hashCode() ^ 1234321;
	}

	/**
	 * Tests whether this abstract pathname is absolute. A pathname is absolute
	 * if its prefix is <code>"/"</code>.
	 * 
	 * @return <code>true</code> if this abstract pathname is absolute,
	 *         <code>false</code> otherwise
	 */
	public boolean isAbsolute() {
		// all path names are made absolute at construction.
		return true;
	}

	/**
	 * Tests whether the file denoted by this abstract pathname is a directory.
	 * Also known on iRODS as a collection.
	 *<P>
	 * An iRODS collection is a logical name given to a set of data sets. All
	 * data sets stored in iRODS are stored in some collection. A collection can
	 * have sub-collections, and hence provides a hierarchical structure. A
	 * collection in iRODS can be equated to a directory in a Unix file system.
	 * But unlike a file system, a collection is not limited to a single device
	 * (or partition). A collection is logical but the datsets grouped under a
	 * collection can be stored in heterogeneous storage devices. There is one
	 * obvious restriction, the name given to a data set in a collection or
	 * sub-collection should be unique in that collection.
	 * 
	 * @return <code>true</code> if and only if the file denoted by this
	 *         abstract pathname exists <em>and</em> is a directory;
	 *         <code>false</code> otherwise
	 */
	public boolean isDirectory() {
		if (useCache) {
			return isDirectory(false);
		} else {
			return isDirectory(true);
		}
	}

	/**
	 * Tests whether the file denoted by this abstract pathname is a directory.
	 * Also known on iRODS as a collection.
	 *<P>
	 * 
	 * @param update
	 *            If true, send a new query to iRODS to determine if this
	 *            abstract pathname refers to a directory. If false, this method
	 *            will return a previously stored value. Also queries iRODS if
	 *            the value is not already stored with this object.
	 * @return <code>true</code> if and only if the file denoted by this
	 *         abstract pathname exists <em>and</em> is a directory;
	 *         <code>false</code> otherwise
	 */
	boolean isDirectory(boolean update) {
		if (update || (pathNameType == PATH_IS_UNKNOWN)) {
			// run the code below
		} else if (pathNameType == PATH_IS_FILE) {
			return false;
		} else if (pathNameType == PATH_IS_DIRECTORY) {
			return true;
		}

		MetaDataRecordList[] rl = null;
		MetaDataCondition[] conditions = { MetaDataSet.newCondition(
				GeneralMetaData.DIRECTORY_NAME, MetaDataCondition.EQUAL,
				getAbsolutePath()) };
		MetaDataSelect[] selects = { MetaDataSet
				.newSelection(GeneralMetaData.DIRECTORY_NAME) };

		try {
			rl = fileSystem.query(conditions, selects, 3);

			if (rl != null && rl.length > 0) {
				pathNameType = PATH_IS_DIRECTORY;
				return true;
			}

		} catch (IOException e) {
			log.warn("io exception is logged and ignored", e);
		}

		return false;
	}

	/**
	 * Tests whether the file denoted by this abstract pathname is a normal
	 * file. A file is <em>normal</em> if it is not a directory or a container.
	 *<P>
	 * In the terminology of iRODS, files are known as data sets. A data set is
	 * a "stream-of-bytes" entity that can be uniquely identified. For example,
	 * a file in HPSS or Unix is a data set, or a LOB stored in an iRODS Vault
	 * database is a data set. Importantly, note that a data set is not a set of
	 * data objects/files. Each data set in iRODS is given a unique internal
	 * identifier by iRODS. A dataset is associated with a collection.
	 * 
	 * @return <code>true</code> if and only if the file denoted by this
	 *         abstract pathname exists <em>and</em> is a normal file;
	 *         <code>false</code> otherwise
	 */
	public boolean isFile() {
		if (useCache) {
			return isFile(false);
		} else {
			return isFile(true);
		}
	}

	/**
	 * Tests whether the file denoted by this abstract pathname is a file. Also
	 * known on iRODS as a dataset.
	 *<P>
	 * 
	 * @param update
	 *            If true, send a new query to iRODS to determine if this
	 *            abstract pathname refers to a file. If false, this method will
	 *            return a previously stored value. Also queries iRODS if the
	 *            value is not already stored with this object.
	 * @return <code>true</code> if and only if the file denoted by this
	 *         abstract pathname exists <em>and</em> is a directory;
	 *         <code>false</code> otherwise
	 */
	public boolean isFile(boolean update) {
		if ((pathNameType == PATH_IS_UNKNOWN) || update) {
			// run the code below
		} else if (pathNameType == PATH_IS_FILE) {
			return true;
		} else if (pathNameType == PATH_IS_DIRECTORY) {
			return false;
		}

		MetaDataRecordList[] rl = null;
		MetaDataCondition[] conditions = {
				MetaDataSet.newCondition(GeneralMetaData.DIRECTORY_NAME,
						MetaDataCondition.EQUAL, getParent()),
				MetaDataSet.newCondition(GeneralMetaData.FILE_NAME,
						MetaDataCondition.EQUAL, getName()) };
		MetaDataSelect[] selects = { MetaDataSet
				.newSelection(GeneralMetaData.FILE_NAME) };

		try {
			rl = fileSystem.query(conditions, selects, 3);

			if (rl != null) {
				pathNameType = PATH_IS_FILE;
				return true;
			}

		} catch (IOException e) {
			log.warn("io exception is logged and ignored", e);
		}

		return false;
	}

	/**
	 * Tests whether the file named by this abstract pathname is a hidden file.
	 * 
	 * @return <code>true</code> if and only if the file denoted by this
	 *         abstract pathname is hidden.
	 */
	public boolean isHidden() {
		return false;
	}

	/**
	 * Returns the time that the file denoted by this abstract pathname was last
	 * modified.
	 * 
	 * @return A <code>long</code> value representing the time the file was last
	 *         modified, measured in milliseconds since the epoch (00:00:00 GMT,
	 *         January 1, 1970), or <code>0L</code> if the file does not exist
	 *         or if an I/O error occurs
	 */
	public long lastModified() {
		long lastModified = 0;
		String result = null;
		try {
			if (isDirectory()) {
				result = firstQueryResult(DirectoryMetaData.DIRECTORY_MODIFY_DATE);
				if (result != null) {
					lastModified = Long.parseLong(result);
				}
			} else {
				result = firstQueryResult(GeneralMetaData.MODIFICATION_DATE);
				if (result != null) {
					lastModified = Long.parseLong(result);
				}
			}
		} catch (IOException e) {
			log.warn("io exception is logged and ignored", e);
			return 0;
		}
		// irods returns seconds
		return lastModified * 1000;
	}

	/**
	 * Returns an array of strings naming the files and directories in the
	 * directory denoted by this abstract pathname.
	 *<P>
	 * There is no guarantee that the name strings in the resulting array will
	 * appear in any specific order; they are not, in particular, guaranteed to
	 * appear in alphabetical order.
	 *<P>
	 * If this IRODSFile object denotes a file, the directory containing that
	 * file will be listed instead.
	 *<P>
	 * This method will return all the files in the directory. Listing
	 * directories with a large number of files may take a very long time. The
	 * more generic IRODSFile.query() method could be used to iterate through
	 * the file list piecewise.
	 * 
	 * @return An array of strings naming the files and directories in the
	 *         directory denoted by this abstract pathname.
	 */
	public String[] list() {
		return list(null);
	}

	/**
	 * Returns the array of strings naming the files and directories in the
	 * directory denoted by this abstract pathname and which match a query
	 * formed using these <code>conditions</code>.
	 * 
	 * @see list()
	 */
	public String[] list(MetaDataCondition[] conditions) {
		MetaDataSelect selects[] = {
				MetaDataSet.newSelection(GeneralMetaData.FILE_NAME),
				MetaDataSet.newSelection(GeneralMetaData.DIRECTORY_NAME), };
		MetaDataRecordList[] rl1, rl2, temp;
		Vector list = null;
		String path;

		MetaDataCondition con[] = null;
		if (conditions == null) {
			con = new MetaDataCondition[1];
		} else {
			con = new MetaDataCondition[conditions.length + 1];
			System.arraycopy(conditions, 0, con, 1, conditions.length);
		}

		try {
			// Have to do two queries, one for files and one for directories.
			if (isDirectory()) {
				path = getAbsolutePath();
			} else {
				path = getParent();
			}

			// get all the files
			con[0] = MetaDataSet.newCondition(GeneralMetaData.DIRECTORY_NAME,
					MetaDataCondition.EQUAL, path);
			rl1 = fileSystem.query(con, selects);
			if (completeDirectoryList) {
				rl1 = MetaDataRecordList.getAllResults(rl1);
			}

			// get all the sub-directories
			selects[0] = MetaDataSet
					.newSelection(IRODSMetaDataSet.DIRECTORY_TYPE);
			con[0] = MetaDataSet.newCondition(
					IRODSMetaDataSet.PARENT_DIRECTORY_NAME,
					MetaDataCondition.EQUAL, path);
			rl2 = iRODSFileSystem.query(con, selects);
			if (completeDirectoryList) {
				rl2 = MetaDataRecordList.getAllResults(rl2);
			}

			// change to relative path
			if (rl2 != null) {
				String absolutePath = null;
				String relativePath = null;
				for (int i = 0; i < rl2.length; i++) {
					// only one record per rl
					absolutePath = rl2[i].getStringValue(1);
					relativePath = absolutePath.substring(absolutePath
							.lastIndexOf("/") + 1);
					rl2[i].setValue(0, relativePath);
				}
			}
		} catch (IOException e) {
			log.warn("io exception is logged and ignored", e);
			return null;
		}

		if ((rl1 != null) && (rl2 != null)) {
			// length of previous query + (new query - table and attribute
			// names)
			temp = new MetaDataRecordList[rl1.length + rl2.length];
			// copy files
			System.arraycopy(rl1, 0, temp, 0, rl1.length);
			System.arraycopy(rl2, 0, temp, rl1.length, rl2.length);
		} else if (rl1 != null) {
			temp = rl1;
		} else if (rl2 != null) {
			temp = rl2;
		} else {
			return new String[0];
		}

		list = new Vector();
		for (int i = 0; i < temp.length; i++) {
			if (temp[i].getStringValue(0) != null) {
				// only one record per rl
				list.add(temp[i].getStringValue(0));
			}
		}

		return (String[]) list.toArray(new String[0]);
	}

	MetaDataRecordList[] rl1, rl2;

	/**
	 * Returns a list of files in this directory, in groups of numbering
	 * <code>DEFAULT_RECORDS_WANTED</code> at a time. If called again it returns
	 * the next group in the set, unless reset is true. If there are no more
	 * results it returns null. If reset is true the list starts from the
	 * beginning.
	 * 
	 * @param reset
	 *            start a new list
	 * @return
	 */
	private GeneralFile[] listFilesNext(boolean reset) {
		if (isDirectory(reset)) {
			Vector<GeneralFile> list = null;
			MetaDataRecordList[] temp;
			if (reset) {
				// New query----------------------------------------------
				MetaDataSelect selects[] = {
						MetaDataSet.newSelection(GeneralMetaData.FILE_NAME),
						MetaDataSet
								.newSelection(GeneralMetaData.DIRECTORY_NAME), };
				String path;

				MetaDataCondition con[] = new MetaDataCondition[1];
				try {
					path = getAbsolutePath();
					// get all the files
					con[0] = MetaDataSet.newCondition(
							GeneralMetaData.DIRECTORY_NAME,
							MetaDataCondition.EQUAL, path);
					rl1 = fileSystem.query(con, selects);

					// get all the sub-directories
					selects[0] = MetaDataSet
							.newSelection(IRODSMetaDataSet.DIRECTORY_TYPE);
					con[0] = MetaDataSet.newCondition(
							IRODSMetaDataSet.PARENT_DIRECTORY_NAME,
							MetaDataCondition.EQUAL, path);
					rl2 = iRODSFileSystem.query(con, selects);

					// change to relative path
					if (rl2 != null) {
						String absolutePath = null;
						String relativePath = null;
						for (int i = 0; i < rl2.length; i++) {
							// only one record per rl
							absolutePath = rl2[i].getStringValue(1);
							relativePath = absolutePath.substring(absolutePath
									.lastIndexOf("/") + 1);
							rl2[i].setValue(0, relativePath);
						}
					}
				} catch (IOException e) {
					log.warn("io exception is logged and ignored", e);
					return null;
				}
			} else {
				// Using old query----------------------------------------------
				try {
					if (rl1 != null && rl1[rl1.length - 1] != null)
						rl1 = rl1[rl1.length - 1].getMoreResults();
					if (rl2 != null && rl2[rl2.length - 1] != null)
						rl2 = rl2[rl2.length - 1].getMoreResults();
				} catch (IOException e) {
					log.warn("io exception is logged and ignored", e);
					return null;
				}
			}
			// Combine the two queries
			if ((rl1 != null) && (rl2 != null)) {
				// length of previous query + (new query - table and attribute
				// names)
				temp = new MetaDataRecordList[rl1.length + rl2.length];
				// copy files
				System.arraycopy(rl1, 0, temp, 0, rl1.length);
				System.arraycopy(rl2, 0, temp, rl1.length, rl2.length);
			} else if (rl1 != null) {
				temp = rl1;
			} else if (rl2 != null) {
				temp = rl2;
			} else {
				return new GeneralFile[0];
			}

			list = new Vector();
			for (int i = 0; i < temp.length; i++) {
				if (temp[i].getStringValue(0) != null) {
					// only one record per rl
					list.add(FileFactory.newFile(this, temp[i]
							.getStringValue(0)));
				}
			}
			return list.toArray(new GeneralFile[0]);
		} else {
			// for now shouldn't use this method if this object is a file.
			return listFiles();
		}
	}

	/**
	 * Creates the directory named by this abstract pathname.
	 */
	public boolean mkdir() {
		try {
			if (!isDirectory()) {
				iRODSFileSystem.commands.mkdir(this);
				return true;
			}
		} catch (IOException e) {
			log.warn("io exception is logged and ignored", e);
		}
		return false;
	}

	/**
	 * Renames the file denoted by this abstract pathname. Will attempt to
	 * overwrite existing files with the same name at the destination. If
	 * <code>dest</code> resource does not equal <code>dest</code>'s resource
	 * the physical resource of the files will also be moved. If both filepaths
	 * are equal, but the resources different, only a physical resource move
	 * will occur.
	 *<P>
	 * Whether or not this method can move a file from one filesystem to another
	 * is platform-dependent. The return value should always be checked to make
	 * sure that the rename operation was successful.
	 * 
	 * After an unsuccessful attempt, some errors may cause a whole/partial copy
	 * of this file/directory to be left at <code>dest</code>.
	 * 
	 * After a successful move, this file object is no longer valid, only the
	 * <code>dest</code> file object should be used.
	 * 
	 * @param dest
	 *            The new abstract pathname for the named file
	 * @throws NullPointerException
	 *             - If dest is null
	 *             
	 */
	public boolean renameTo(GeneralFile dest) throws IllegalArgumentException,
			NullPointerException {
		try {
			if (dest instanceof IRODSFile) {
				// if the path is different
				if (!getAbsolutePath().equals(dest.getAbsolutePath())) {
					if (isDirectory()) {
						iRODSFileSystem.commands.renameDirectory(this,
								(IRODSFile) dest);
					} else if (isFile(true)) {
						iRODSFileSystem.commands.renameFile(this,
								(IRODSFile) dest);
					}
				}

				// if the resource is different
				if (!getResource().equals(((IRODSFile) dest).getResource()))
					iRODSFileSystem.commands.physicalMove(this,
							(IRODSFile) dest);
			} else {
				return super.renameTo(dest);
			}
		} catch (IOException e) {
			log.warn("io exception is logged and ignored", e);
			return false;
		}

		return true;
	}

	/**
	 * Constructs a <tt>irods:</tt> URI that represents this abstract pathname.
	 * 
	 * <p>
	 * The exact form of the URI is according to iRODS. If it can be determined
	 * that the file denoted by this abstract pathname is a directory, then the
	 * resulting URI will end with a slash.
	 * 
	 * <p>
	 * For a given abstract pathname <i>f</i>, it is guaranteed that
	 * 
	 * <blockquote><tt>
	 * new {@link #IRODSFile(java.net.URI) IRODSFile}
	 * (</tt><i>&nbsp;f</i><tt>.toURI()).equals(</tt><i>&nbsp;f</i><tt>)
   * </tt>
	 * </blockquote>
	 * 
	 * so long as the original abstract pathname, the URI, and the new abstract
	 * pathname are all created in (possibly different invocations of) the same
	 * Java virtual machine. However, this relationship typically does not hold
	 * when a <tt>irods:</tt> URI that is created in a virtual machine on one
	 * operating system is converted into an abstract pathname in a virtual
	 * machine on a different operating system.
	 * 
	 * @return An absolute, hierarchical URI with a scheme equal to
	 *         <tt>"irods"</tt>, a path representing this abstract pathname, and
	 *         undefined authority, query, and fragment components
	 * 
	 * @see #IRODSFile(java.net.URI)
	 * @see java.net.URI
	 * @see java.net.URI#toURL()
	 */
	public URI toURI() {
		URI uri = null;

		try {
			if (isDirectory()) {
				uri = new URI("irods", iRODSFileSystem.getUserName(),
						iRODSFileSystem.getHost(), iRODSFileSystem.getPort(),
						getAbsolutePath() + "/", "", "");
			} else {
				uri = new URI("irods", iRODSFileSystem.getUserName(),
						iRODSFileSystem.getHost(), iRODSFileSystem.getPort(),
						getAbsolutePath(), "", "");
			}
		} catch (URISyntaxException e) {
			log.warn("io exception is logged and ignored", e);
		}

		return uri;
	}

	/**
	 * Constructs a <tt>irods:</tt> URI that represents this abstract pathname.
	 * 
	 * <p>
	 * The exact form of the URI is according to the IRODS. If it can be
	 * determined that the file denoted by this abstract pathname is a
	 * directory, then the resulting URI will end with a slash.
	 * 
	 * <p>
	 * For a given abstract pathname <i>f</i>, it is guaranteed that
	 * 
	 * <blockquote><tt>
	 * new {@link #IRODSFile(java.net.URI) IRODSFile}
	 * (</tt><i>&nbsp;f</i><tt>.toURI()).equals(</tt><i>&nbsp;f</i><tt>)
   * </tt>
	 * </blockquote>
	 * 
	 * so long as the original abstract pathname, the URI, and the new abstract
	 * pathname are all created in (possibly different invocations of) the same
	 * Java virtual machine. However, this relationship typically does not hold
	 * when a <tt>irods:</tt> URI that is created in a virtual machine on one
	 * operating system is converted into an abstract pathname in a virtual
	 * machine on a different operating system.
	 * 
	 * @return An absolute, hierarchical URI with a scheme equal to
	 *         <tt>"irods"</tt>, a path representing this abstract pathname, and
	 *         undefined authority, query, and fragment components
	 * 
	 * @see #IRODSFile(java.net.URI)
	 * @see java.net.URI
	 * @see java.net.URI#toURL()
	 */
	public URI toURI(boolean includePassword) {
		if (!includePassword)
			return toURI();

		URI uri = null;

		try {
			if (isDirectory()) {
				uri = new URI("irods", iRODSFileSystem.getUserName() + ":"
						+ iRODSFileSystem.getPassword(), iRODSFileSystem
						.getHost(), iRODSFileSystem.getPort(),
						getAbsolutePath() + "/", "", "");
			} else {
				uri = new URI("irods", iRODSFileSystem.getUserName() + ":"
						+ iRODSFileSystem.getPassword(), iRODSFileSystem
						.getHost(), iRODSFileSystem.getPort(),
						getAbsolutePath(), "", "");
			}
		} catch (URISyntaxException e) {
			log.warn("URISyntax exception is logged and ignored", e);
		}

		return uri;
	}

	/**
	 * Returns a string representation of this file object. The string is
	 * formated according to the iRODS URI model. Note: the user password will
	 * not be included in the URI.
	 */
	public String toString() {
		return new String("irods://" + iRODSFileSystem.getUserName() + "@"
				+ iRODSFileSystem.getHost() + ":" + iRODSFileSystem.getPort()
				+ getAbsolutePath());
	}
}
