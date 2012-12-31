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
//  SRBContainer.java  -  edu.sdsc.grid.io.srb.SRBContainer
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-.GeneralFile
//              |
//              +-.RemoteFile
//                    |
//                    +-.SRBFile
//                          |
//                          +-.SRBContainer
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.srb;

import java.io.IOException;
import java.net.URI;
import java.util.Vector;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.GeneralMetaData;
import edu.sdsc.grid.io.GeneralRandomAccessFile;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.RemoteFile;
import edu.sdsc.grid.io.ResourceMetaData;
import edu.sdsc.grid.io.StandardMetaData;
import edu.sdsc.grid.io.UserMetaData;
import edu.sdsc.grid.io.local.LocalFile;

//import java.io.FilenameFilter;
//import java.io.FileFilter;
//import java.util.Iterator;

/**
 * An abstract representation of a container on the SRB. A container is a way to
 * put together a lot of small files into one larger file to improve
 * performance. This works very well with resources that include tapes (such as
 * HPSS). The whole container is retrieved from tape, cached on SRB disk, and
 * then multiple files can be quickly read and written on the container copy on
 * disk. The SRB handles the book-keeping for the container.
 * <P>
 * SRBContainer subclasses SRBFile and have certain similarities with SRB
 * directories/collections, the logical groupings of files on the SRB.
 * Containers represent a physical grouping of files on a particular SRB
 * resource. Containers can be placed anywhere on the SRB the user has write
 * privileges.
 * <P>
 * 
 * @author Lucas Gilbert, San Diego Supercomputer Center
 * @since JARGON1.0
 * @see edu.sdsc.grid.io.srb.SRBFile
 */
public class SRBContainer extends SRBFile {
	// ----------------------------------------------------------------------
	// Constants
	// ----------------------------------------------------------------------
	/**
	 * The default maximum container size, in bytes. i.e. 209715200.
	 */
	final static int DEFAULT_CONTAINER_SIZE = 209715200;

	/**
	 * Synchronize all containers in a container family.
	 */
	// public static final String ALL_SYNC =
	// "synchronize all containers in family";

	/**
	 * Delete the cache copy once the copies have been synchronized.
	 */
	public static final String PURGE_SYNC = "synchronize then purge cache";

	/**
	 * Delete the cache copy once the copies have been synchronized.
	 */
	public static final String PRIMARY_SYNC = "synchronize primary copy";

	// ----------------------------------------------------------------------
	// Fields
	// ----------------------------------------------------------------------
	/**
	 * The maximum size, in bytes, this container can hold.
	 */
	long containerMaxSize = DEFAULT_CONTAINER_SIZE;

	// ----------------------------------------------------------------------
	// Constructors and Destructors
	// ----------------------------------------------------------------------
	/**
	 * Creates a new <code>SRBContainer</code> instance by converting the given
	 * pathname string into an abstract pathname.
	 * 
	 * @param fileSystem
	 *            The connection to the SRB
	 * @param filePath
	 *            The pathname string
	 */
	public SRBContainer(final SRBFileSystem fileSystem, final String filePath) {
		super(fileSystem, filePath);
	}

	/**
	 * Creates a new <code>SRBContainer</code> instance by converting the given
	 * pathname string into an abstract pathname.
	 * 
	 * @param fileSystem
	 *            The connection to the SRB
	 * @param filePath
	 *            A pathname string
	 * @param containerMaxSize
	 *            The size of the container, in bytes
	 */
	public SRBContainer(final SRBFileSystem fileSystem, final String filePath,
			final long containerMaxSize) {
		super(fileSystem, filePath);

		setContainerMaxSize(containerMaxSize);
	}

	/**
	 * Creates a new <code>SRBContainer</code> instance by converting the given
	 * pathname string into an abstract pathname.
	 * <P>
	 * If <code>parent</code> is <code>null</code> then the new
	 * <code>SRBContainer</code> instance is created as if by invoking the
	 * single-argument <code>SRBContainer</code> constructor on the given
	 * <code>child</code> pathname string.
	 * <P>
	 * Otherwise the parent abstract pathname is taken to denote a directory,
	 * and the child pathname string is taken to denote the container name. If
	 * the <code>child</code> pathname string is absolute then it is converted
	 * into a relative pathname in a SRB pathname. If parent is the empty string
	 * then the new <code>SRBContainer</code> instance is created by converting
	 * <code>child</code> into an abstract pathname and resolving the result
	 * against the SRB default container directory. Otherwise each pathname
	 * string is converted into an abstract pathname and the <code>child</code>
	 * abstract pathname is resolved against the <code>parent</code>.
	 * <P>
	 * 
	 * @param parent
	 *            The parent abstract pathname
	 * @param child
	 *            The child pathname string
	 */
	public SRBContainer(final SRBFile parent, final String child) {
		super(parent, child);
	}

	/**
	 * Creates a new <code>SRBContainer</code> instance by converting the given
	 * pathname string into an abstract pathname.
	 * <P>
	 * If <code>parent</code> is <code>null</code> then the new
	 * <code>SRBContainer</code> instance is created as if by invoking the
	 * single-argument <code>SRBContainer</code> constructor on the given
	 * <code>child</code> pathname string.
	 * <P>
	 * Otherwise the parent abstract pathname is taken to denote a directory,
	 * and the child pathname string is taken to denote the container name. If
	 * the <code>child</code> pathname string is absolute then it is converted
	 * into a relative pathname in a SRB pathname. If parent is the empty string
	 * then the new <code>SRBContainer</code> instance is created by converting
	 * <code>child</code> into an abstract pathname and resolving the result
	 * against the SRB default container directory. Otherwise each pathname
	 * string is converted into an abstract pathname and the <code>child</code>
	 * abstract pathname is resolved against the <code>parent</code>.
	 * <P>
	 * 
	 * @param parent
	 *            The parent abstract pathname
	 * @param child
	 *            The child pathname string
	 * @param containerMaxSize
	 *            The size of the container, in bytes
	 */
	public SRBContainer(final SRBFile parent, final String child,
			final long containerMaxSize) {
		super(parent, child);

		setContainerMaxSize(containerMaxSize);
	}

	/**
	 * Creates a new <code>SRBContainer</code> instance by converting the given
	 * pathname string into an abstract pathname.
	 * <P>
	 * If <code>parent</code> is <code>null</code> then the new
	 * <code>SRBContainer</code> instance is created as if by invoking the
	 * single-argument <code>SRBContainer</code> constructor on the given
	 * <code>child</code> pathname string.
	 * <P>
	 * Otherwise the parent abstract pathname is taken to denote a directory,
	 * and the child pathname string is taken to denote the container name. If
	 * the <code>child</code> pathname string is absolute then it is converted
	 * into a relative pathname in a SRB pathname. If parent is the empty string
	 * then the new <code>SRBContainer</code> instance is created by converting
	 * <code>child</code> into an abstract pathname and resolving the result
	 * against the SRB default container directory. Otherwise each pathname
	 * string is converted into an abstract pathname and the <code>child</code>
	 * abstract pathname is resolved against the <code>parent</code>.
	 * <P>
	 * 
	 * @param fileSystem
	 *            The connection to the SRB
	 * @param parent
	 *            The parent pathname string
	 * @param child
	 *            The child pathname string
	 */
	public SRBContainer(final SRBFileSystem fileSystem, final String parent,
			final String child) {
		super(fileSystem, parent, child);
	}

	/**
	 * Creates a new <code>SRBContainer</code> instance by converting the given
	 * pathname string into an abstract pathname.
	 * <P>
	 * If <code>parent</code> is <code>null</code> then the new
	 * <code>SRBContainer</code> instance is created as if by invoking the
	 * single-argument <code>SRBContainer</code> constructor on the given
	 * <code>child</code> pathname string.
	 * <P>
	 * Otherwise the parent abstract pathname is taken to denote a directory,
	 * and the child pathname string is taken to denote the container name. If
	 * the <code>child</code> pathname string is absolute then it is converted
	 * into a relative pathname in a SRB pathname. If parent is the empty string
	 * then the new <code>SRBContainer</code> instance is created by converting
	 * <code>child</code> into an abstract pathname and resolving the result
	 * against the SRB default container directory. Otherwise each pathname
	 * string is converted into an abstract pathname and the <code>child</code>
	 * abstract pathname is resolved against the <code>parent</code>.
	 * <P>
	 * 
	 * @param fileSystem
	 *            The connection to the SRB
	 * @param parent
	 *            The parent pathname string
	 * @param child
	 *            The child pathname string
	 * @param containerMaxSize
	 *            The size of the container, in bytes
	 */
	public SRBContainer(final SRBFileSystem fileSystem, final String parent,
			final String child, final long containerMaxSize) {
		super(fileSystem, parent, child);

		setContainerMaxSize(containerMaxSize);
	}

	/**
	 * Creates a new <code>SRBContainer</code> instance by converting the given
	 * uri string into an abstract pathname.
	 * <P>
	 * 
	 * @param fileSystem
	 *            The connection to the SRB
	 * @param uri
	 *            The uri pathname string
	 * @param containerMaxSize
	 *            The size of the container, in bytes
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	public SRBContainer(final URI uri) throws IOException {
		super(uri);

		setContainerMaxSize(containerMaxSize);
	}

	/**
	 * Finalizes the object by explicitly letting go of each of its internally
	 * held values.
	 * <P>
	 */
	/*
	 * protected void finalize( ) throws Throwable { super.finalize(); }
	 */

	// ----------------------------------------------------------------------
	// Setters and Getters
	// ----------------------------------------------------------------------
	/**
	 * Sets the container size of this SRBContainer object, in bytes. The max
	 * size of a container cannot be changed if the container already exists on
	 * the SRB.
	 * <P>
	 * Currently, containers cannot have a max size over 2GB, though they will
	 * accept files larger than 2GB. If the container runs out of space it will
	 * be renamed to a new container plus some random digits and a new container
	 * will be created. <br>
	 * eg. container foo will be renamed foo.1234 and a new foo container will
	 * be created.
	 * 
	 * @throws IllegalArgumentException
	 *             If the <code>containerMaxSize</code> is less than zero.
	 */
	public void setContainerMaxSize(final long containerMaxSize) {
		if (containerMaxSize < 0) {
			throw new IllegalArgumentException();
		}
		this.containerMaxSize = containerMaxSize;
	}

	/**
	 * @return the size of this SRBContainer, in bytes.
	 */
	public long getContainerMaxSize() {
		try {
			MetaDataRecordList[] rl = query(SRBMetaDataSet.CONTAINER_MAX_SIZE);
			if (rl != null) {
				return Long.parseLong(rl[0].getValue(
						SRBMetaDataSet.CONTAINER_MAX_SIZE).toString());
			}
		} catch (IOException e) {
			// container doesn't exist
		}

		return 0;
	}

	/**
	 * @return resource the physical resource where this SRBFile is stored.
	 * 
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	@Override
	public String getResource() throws IOException {
		MetaDataRecordList[] rl = query(ResourceMetaData.RESOURCE_NAME);
		if (rl != null) {
			return rl[0].getValue(ResourceMetaData.RESOURCE_NAME).toString();
		}

		return null;
	}

	/**
	 * @return dataType The dataType string of this SRBFile.
	 * 
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	@Override
	public String getDataType() throws IOException {
		MetaDataRecordList[] rl = query(SRBMetaDataSet.FILE_TYPE_NAME);
		if (rl != null) {
			return rl[0].getValue(SRBMetaDataSet.FILE_TYPE_NAME).toString();
		}

		return null;
	}

	// ----------------------------------------------------------------------
	// SRBContainer Methods
	// ----------------------------------------------------------------------
	/**
	 * Adds the named file to this container. Note, files already on the SRB
	 * cannot be added to a container. One reason being a name conflict will
	 * occur.
	 */
	public void include(final GeneralFile file) throws IOException {
		if (!exists()) {
			createNewFile();
		}

		int fd = srbFileSystem.srbObjCreate(catalogType, file.getName()
				+ "&CONTAINER=" + getAbsolutePath(), dataType, resource,
				file.getParent(), "", file.length());

		byte inputBuffer[] = new byte[BUFFER_MAX_SIZE];
		long length = file.length();
		GeneralRandomAccessFile in = FileFactory.newRandomAccessFile(file, "r");
		if (length > BUFFER_MAX_SIZE) {
			do {
				in.read(inputBuffer);
				srbFileSystem.srbObjWrite(fd, inputBuffer, inputBuffer.length);
				length -= BUFFER_MAX_SIZE;
			} while (length > BUFFER_MAX_SIZE);
		}
		inputBuffer = new byte[(int) length];
		in.read(inputBuffer);
		srbFileSystem.srbObjWrite(fd, inputBuffer, inputBuffer.length);

		in.close();
		srbFileSystem.srbObjClose(fd);
	}

	// ----------------------------------------------------------------------
	// GeneralFile Methods
	// ----------------------------------------------------------------------
	@Override
	public MetaDataRecordList[] query(final MetaDataSelect[] selects)
			throws IOException {
		return query(selects, SRBFileSystem.DEFAULT_RECORDS_WANTED);
	}

	@Override
	public MetaDataRecordList[] query(final MetaDataSelect[] selects,
			final int recordsWanted) throws IOException {
		MetaDataCondition iConditions[] = { MetaDataSet.newCondition(
				SRBMetaDataSet.CONTAINER_NAME, MetaDataCondition.EQUAL,
				getAbsolutePath()) };

		return srbFileSystem.query(iConditions, selects, recordsWanted);
	}

	@Override
	public void copyTo(final GeneralFile file, final boolean forceOverwrite,
			final boolean bulkCopy) throws IOException {
		String list[] = list();
		GeneralFile temp;

		if (!file.exists()) {
			file.mkdir();
		} else if (file.isFile()) {
			throw new IllegalArgumentException(
					"The destination cannot be a file.");
		}

		if (file instanceof SRBContainer) {
			for (String element : list) {
				((SRBContainer) file).include(new SRBFile(srbFileSystem,
						element));
			}
		} else {
			for (String element : list) {
				temp = FileFactory.newFile(this, element);
				temp.copyTo(FileFactory.newFile(file, temp.getName()));
			}
		}
	}

	@Override
	public void copyFrom(final GeneralFile file, final boolean forceOverwrite,
			final boolean bulkCopy) throws IOException {
		String list[] = file.list();

		if (!exists()) {
			createNewFile();
		}

		if (file.isFile()) {
			include(file);
		} else {
			for (String element : list) {
				include(FileFactory.newFile(file, element));
			}
		}
	}

	// ----------------------------------------------------------------------
	// RemoteFile and SRBFile Methods
	// ----------------------------------------------------------------------
	/**
	 * @deprecated Superseded by copyFrom( GeneralFile ). This single new method
	 *             replaces all the other older copyFrom...() methods. It
	 *             functions in the same manner as those old methods, but can
	 *             except any GeneralFile. It also fixes some bugs in those
	 *             methods.
	 */
	@Deprecated
	public void copyFromLocal(final LocalFile localFile) throws IOException {
		if (localFile.isDirectory()) {
			copyFrom(localFile, false, false);
		}

		throw new UnsupportedOperationException(
				"Cannot use copyFromLocal for containers");
	}

	/**
	 * @deprecated Superseded by copyFrom( GeneralFile ). This single new method
	 *             replaces all the other older copyFrom...() methods. It
	 *             functions in the same manner as those old methods, but can
	 *             except any GeneralFile. It also fixes some bugs in those
	 *             methods.
	 */
	@Deprecated
	public void copyFromLocal(final String localFilePath) throws IOException {
		LocalFile localFile = new LocalFile(localFilePath);
		if (localFile.isDirectory()) {
			copyFrom(localFile, false, false);
		}

		throw new UnsupportedOperationException(
				"Cannot use copyFromLocal for containers");
	}

	/**
	 * Copies the SRB container to the local file system, as if it were a
	 * directory. The local file must be a directory. If the local file does not
	 * exist, a new one will be created.
	 * <P>
	 * 
	 * @deprecated Superseded by copyFrom( GeneralFile ). This single new method
	 *             replaces all the other older copyFrom...() methods. It
	 *             functions in the same manner as those old methods, but can
	 *             except any GeneralFile. It also fixes some bugs in those
	 *             methods.
	 * @param localFile
	 *            The file to receive the data
	 * @throws NullPointerException
	 *             If localFile is null.
	 */
	@Deprecated
	public void copyToLocal(final LocalFile localFile) throws IOException {
		copyTo(localFile, false, false);
	}

	/**
	 * Copies the SRB container to the local file system, as if it were a
	 * directory. If the local file does not exist, a new one will be created.
	 * <P>
	 * 
	 * @deprecated Superseded by copyFrom( GeneralFile ). This single new method
	 *             replaces all the other older copyFrom...() methods. It
	 *             functions in the same manner as those old methods, but can
	 *             except any GeneralFile. It also fixes some bugs in those
	 *             methods.
	 * @param localFile
	 *            The file to be copied
	 * @throws NullPointerException
	 *             If localFile is null.
	 */
	@Deprecated
	public void copyToLocal(final String localFilePath) throws IOException {
		copyTo(new LocalFile(localFilePath), false, false);
	}

	/**
	 * Copies this file to a new location on the server. If the
	 * <code>remoteFile</code> does not exist, one will be created. Otherwise
	 * this SRB file is appended to the end of the remote file.
	 * <P>
	 * Copying a container to a new logical space would be an unusual action.
	 * Replicate is better.
	 * 
	 * @deprecated Superseded by copyFrom( GeneralFile ). This single new method
	 *             replaces all the other older copyFrom...() methods. It
	 *             functions in the same manner as those old methods, but can
	 *             except any GeneralFile. It also fixes some bugs in those
	 *             methods.
	 * @param remoteFile
	 *            The local file to receive the data.
	 * @throws ClassCastException
	 *             - if the argument is not a SRBFile object.
	 * @throws NullPointerException
	 *             If remoteFile is null.
	 */
	@Deprecated
	public void copyToRemote(final RemoteFile remoteFile) throws IOException,
			ClassCastException {
		copyTo(remoteFile);
	}

	/**
	 * Get the permissions of the current user for this SRBContainer: write,
	 * read, all, annotate or null.
	 * <P>
	 * 
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	@Override
	public String getPermissions() throws IOException {
		MetaDataRecordList[] rl = null;
		String userName = srbFileSystem.getUserName();
		String userDomain = srbFileSystem.getDomainName();

		MetaDataCondition conditions[] = {
				MetaDataSet.newCondition(UserMetaData.USER_NAME,
						MetaDataCondition.EQUAL, userName),
				MetaDataSet.newCondition(SRBMetaDataSet.USER_DOMAIN,
						MetaDataCondition.EQUAL, userDomain), };
		MetaDataSelect selects[] = {
				MetaDataSet.newSelection(UserMetaData.USER_NAME),
				MetaDataSet.newSelection(SRBMetaDataSet.USER_DOMAIN),
				MetaDataSet.newSelection(GeneralMetaData.ACCESS_CONSTRAINT), };
		rl = query(conditions, selects);

		if (rl != null) {
			for (MetaDataRecordList element : rl) {
				if (element.getValue(UserMetaData.USER_NAME).equals(userName)
						&& element.getValue(SRBMetaDataSet.USER_DOMAIN).equals(
								userDomain)) {
					return element.getValue(GeneralMetaData.ACCESS_CONSTRAINT)
							.toString();
				}
			}
		}

		return null;
	}

	/**
	 * Gets all the non null permissions of all SRB users for this SRBContainer:
	 * write, read, all, annotate or null.
	 * <P>
	 * 
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	@Override
	public MetaDataRecordList[] getPermissions(final boolean allUsers)
			throws IOException {
		return query(GeneralMetaData.ACCESS_CONSTRAINT);
	}

	/**
	 * Replicates this container to a new resource. Each replicant will
	 * increment its replication number by 1 from the last replication.
	 * <P>
	 * 
	 * @param newResource
	 *            The storage resource name of the new copy.
	 */
	@Override
	public void replicate(final String newResource) throws IOException {
		srbFileSystem.srbReplContainer(catalogType, getPath(), newResource);
	}

	/**
	 * Sync a container. With backup() you can sync the permanant replica with
	 * the temporary replica. The default is to synchronize all archival
	 * resources. Syncronization types can also be either
	 * <code>PURGE_SYNC</code> or <code>PRIMARY_SYNC</code>
	 * 
	 * @param syncType
	 *            synchronization type, see PURGE_SYNC and PRIMARY_SYNC. default
	 *            value is to synchronize all archival resources.
	 */
	@Override
	public void backup(final String syncType) throws IOException {
		int syncFlag = 0;
		String path = getAbsolutePath();

		// purge the cache copies after sync is done.
		// PURGE_FLAG = 1
		if (syncType == PURGE_SYNC) {
			syncFlag = 1;
		}
		// Synchronize to the primary archival resource
		// only. The default is to synchronize all archival resources.
		// PRIMARY_FLAG = 2
		else if (syncType == PRIMARY_SYNC) {
			syncFlag = 2;
		}
		// The default is to synchronize all archival resources.
		else {
			path += ".*";
		}

		srbFileSystem.srbSyncContainer(catalogType, path, syncFlag);
	}

	/**
	 * Loads the local files on to the SRB in this container in the
	 * <code>srbDirectory</code>.
	 */
	void bulkLoad(final LocalFile[] files) throws IOException {
		/*
		 * which container or resource, get size and MaxSize of container open
		 * container or temp file for upload create two 8MB byte buffers create
		 * new threads main thread starts loading local files into buffer -one
		 * buffer gets filled well the other buffer is used in transfer also
		 * loads myResultStruct (upto 300 files)
		 * -somehow->dataName,collectionName,offset -once full create
		 * registration thread make sure not to overflow container
		 * 
		 * only one transfer thread waits on buffer to have data -main gives
		 * signal check enough space, otherwise create new container uses
		 * regular srbobjwrite udates status and wait again
		 * 
		 * upto 4 registration threads
		 */

		int MAX_REGISTRATION_THREADS = 4;

		// Two buffers are used switching back and forth
		// from loading to transfering files
		byte[] buffer1 = new byte[BULK_LOAD_BUFFER_SIZE];
		byte[] buffer2 = new byte[BULK_LOAD_BUFFER_SIZE];

		// the file to receive the data
		GeneralRandomAccessFile raf = new SRBRandomAccessFile(this, "rw");

		// one transfer thread
		Thread loadThread = null;
		LoadThread load = null;

		// if this object isn't a dir, make it one.
		mkdir();

		RegistrationThread[] registration = new RegistrationThread[MAX_REGISTRATION_THREADS];
		for (int i = 0; i < MAX_REGISTRATION_THREADS; i++) {
			registration[i] = new RegistrationThread(srbFileSystem,
					catalogType, getAbsolutePath());
		}
		Thread[] registrationThreads = new Thread[MAX_REGISTRATION_THREADS];
		for (int i = 0; i < MAX_REGISTRATION_THREADS; i++) {
			registrationThreads[i] = new Thread(registration[i]);
		}

		load = new LoadThread(raf, buffer1, buffer2);
		loadThread = new Thread(load);
		loadThread.start();

		// Now start copying
		for (LocalFile file : files) {
			loadBuffer(file, load, this, registrationThreads, registration,
					null);
		}

		// make sure the load buffers have finished loading all the files
		if ((useBuffer == 0) || (useBuffer == 1)) {
			load.buffer1Length = bufferPos;
		} else if (useBuffer == 2) {
			load.buffer2Length = bufferPos;
		}

		load.writeBuffer1 = true;
		while (load.writeBuffer1) {
			// wait?
		}
		load.writeBuffer2 = true;

		// end thread stuff
		load.keepLoading = false;
		raf.close();
		try {
			if (loadThread.isAlive()) {
				loadThread.join();
			}
			for (int i = 0; i < MAX_REGISTRATION_THREADS; i++) {
				registration[i].forceRegister();
			}
			for (int i = 0; i < MAX_REGISTRATION_THREADS; i++) {
				if (registrationThreads[i] != null) {
				}
			}
		} catch (InterruptedException e) {

		}
	}

	int bufferPos = 0;
	int useBuffer = 0;
	int numRegistrationThreads = 0;

	// file currently being loaded into the buffer
	private void loadBuffer(final GeneralFile file, final LoadThread load,
			final SRBFile tempFile, final Thread[] registrationThreads,
			final RegistrationThread[] registration, String topLevelFile)
			throws IOException {
		if (file == null) {
			return;
		}

		if (file.isDirectory()) {
			if (topLevelFile == null) {
				topLevelFile = file.getName();
			} else {
				topLevelFile += separator + file.getName();
			}

			GeneralFile[] files = file.listFiles();
			if (files == null) {
				return;
			}

			for (GeneralFile file2 : files) {
				loadBuffer(file2, load, tempFile, registrationThreads,
						registration, topLevelFile);
			}
		} else {
			GeneralRandomAccessFile readFile = null;
			try {
				readFile = FileFactory.newRandomAccessFile(file, "r");
			} catch (SecurityException e) {
				return;
			}
			long toRead = file.length();
			int temp = 0;

			while (toRead > 0) {
				// load the buffer that is not being transferred
				// this if, or something else is cutting up the transfer
				// so the buffer2 isn't always getting sent.
				// it should switch back and forth.
				while ((load.writeBuffer1 && load.writeBuffer2)
						|| (load.writeBuffer1 && (useBuffer == 1))
						|| (load.writeBuffer2 && (useBuffer == 2))) {
					// just wait
				}

				// warning, had problems went I limit the BULK_LOAD_BUFFER_SIZE
				// to something small
				// it would occasionally not upload buffer2
				if ((useBuffer == 0) || (useBuffer == 1)) {
					if ((toRead + bufferPos) < BULK_LOAD_BUFFER_SIZE) {
						// read [the last of] this file into this buffer
						temp = readFile.read(load.buffer1, bufferPos,
								(int) toRead);
						bufferPos += temp;
						toRead -= temp;
					} else if (bufferPos < BULK_LOAD_BUFFER_SIZE) {
						// fill this buffer
						temp = readFile.read(load.buffer1, bufferPos,
								BULK_LOAD_BUFFER_SIZE - bufferPos);
						bufferPos += temp;
						toRead -= temp;

						// signal to the load thread, and reset local variables
						load.buffer1Length = bufferPos;
						load.writeBuffer1 = true;
						useBuffer = 2;
						bufferPos = 0;

						// if possible, load the remainder of the file into the
						// other buffer
						if (toRead < BULK_LOAD_BUFFER_SIZE) {
							temp = readFile.read(load.buffer2, bufferPos,
									(int) toRead);
							bufferPos += temp;
							toRead -= temp;
						}
					} else {
						// buffer is full, signal and reset
						load.buffer1Length = bufferPos;
						load.writeBuffer1 = true;
						useBuffer = 2;
						bufferPos = 0;

						// if possible, load the remainder of the file into the
						// other buffer
						if (toRead < BULK_LOAD_BUFFER_SIZE) {
							temp = readFile.read(load.buffer2, bufferPos,
									(int) toRead);
							bufferPos += temp;
							toRead -= temp;
						}
					}
				}

				if (useBuffer == 2) {
					if ((toRead + bufferPos) < BULK_LOAD_BUFFER_SIZE) {
						// read [the last of] this file into this buffer
						temp = readFile.read(load.buffer2, bufferPos,
								(int) toRead);
						bufferPos += temp;
						toRead -= temp;
					} else if (bufferPos < BULK_LOAD_BUFFER_SIZE) {
						// fill this buffer
						temp = readFile.read(load.buffer2, bufferPos,
								BULK_LOAD_BUFFER_SIZE - bufferPos);
						bufferPos += temp;
						toRead -= temp;

						// signal to the load thread, and reset local variables
						load.buffer2Length = bufferPos;
						load.writeBuffer2 = true;
						useBuffer = 1;
						bufferPos = 0;

						// if possible, load the remainder of the file into the
						// other buffer
						if (toRead < BULK_LOAD_BUFFER_SIZE) {
							temp = readFile.read(load.buffer1, bufferPos,
									(int) toRead);
							bufferPos += temp;
							toRead -= temp;
						}
					} else {
						load.buffer2Length = bufferPos;
						load.writeBuffer2 = true;
						useBuffer = 1;
						bufferPos = 0;

						// if possible, load the remainder of the file into the
						// other buffer
						if (toRead < BULK_LOAD_BUFFER_SIZE) {
							temp = readFile.read(load.buffer1, bufferPos,
									(int) toRead);
							bufferPos += temp;
							toRead -= temp;
						}
					}
				}
			}
			readFile.close();

			filesReadyToRegister++;

			if (topLevelFile == null) {
				registration[numRegistrationThreads % 4].addFile(file, "");
			} else {
				registration[numRegistrationThreads % 4].addFile(file,
						topLevelFile);
			}

			if ((((filesReadyToRegister % 10) == 0))
					&& (numRegistrationThreads < 4)) {
				if (!registrationThreads[numRegistrationThreads].isAlive()) {
					registrationThreads[numRegistrationThreads].start();
				}
				numRegistrationThreads++;
			}
		}
	}

	class LoadThread implements Runnable {
		GeneralRandomAccessFile out;
		byte[] buffer1;
		byte[] buffer2;
		boolean writeBuffer1 = false;
		boolean writeBuffer2 = false;
		int buffer1Length = 0;
		int buffer2Length = 0;
		boolean keepLoading = true;

		LoadThread(final GeneralRandomAccessFile out, final byte[] buffer1,
				final byte[] buffer2) {
			this.out = out;
			this.buffer1 = buffer1;
			this.buffer2 = buffer2;
		}

		public void run() {
			while (keepLoading) {
				sendBuffer();
			}
		}

		void sendBuffer() {
			try {
				if (writeBuffer1) {
					out.write(buffer1, 0, buffer1Length);
					writeBuffer1 = false;
				}

				if (writeBuffer2) {
					out.write(buffer2, 0, buffer2Length);
					writeBuffer2 = false;
				}
			} catch (IOException e) {
			}
		}
	}

	// keep track of which files have been registered
	int registeredFiles = 0;
	// and which still need to be.
	int filesReadyToRegister = 0;

	int registrationOffset = 0;

	class RegistrationThread implements Runnable {
		SRBFileSystem fileSystem;
		int catalogType;

		// directory to load into on the SRB
		String bloadFilePath;

		// loop until false
		boolean keepLoading = false;

		boolean newFiles = false;

		Vector files = new Vector();
		Vector paths = new Vector();

		RegistrationThread(final SRBFileSystem fileSystem, final int catType,
				final String bloadFilePath) throws IOException {
			this.fileSystem = fileSystem;
			this.catalogType = catType;
			this.bloadFilePath = bloadFilePath;
		}

		void addFile(final GeneralFile file, final String relativePath) {
			files.add(file);
			paths.add(relativePath);
			newFiles = true;
		}

		public void run() {
			keepLoading = true;
			while (keepLoading) {
				register();
			}
		}

		// get the files that are ready to be registered
		SRBMetaDataRecordList[] getFileRegistry() throws IOException {
			SRBMetaDataRecordList rl = null;
			SRBMetaDataRecordList[] recordLists = new SRBMetaDataRecordList[files
					.size()];

			GeneralFile tempFile = null;
			String fileName = null;
			String dirName = null;
			long size = 0;

			// create the registry for the files loaded so far.
			for (int i = 0; i < recordLists.length; i++) {
				tempFile = (GeneralFile) files.get(i);
				fileName = tempFile.getName();
				registrationOffset += size;// I hope the reg threads always
				// run in order...
				size = tempFile.length();
				dirName = (String) paths.get(i);
				if (dirName != "") {
					dirName = getAbsolutePath() + separator + dirName;
				} else {
					dirName = getAbsolutePath();
				}

				rl = new SRBMetaDataRecordList(
						MetaDataSet.getField(StandardMetaData.FILE_NAME),
						fileName);
				rl.addRecord(
						MetaDataSet.getField(StandardMetaData.DIRECTORY_NAME),
						dirName);
				rl.addRecord(MetaDataSet.getField(GeneralMetaData.SIZE), size);
				rl.addRecord(MetaDataSet.getField(SRBMetaDataSet.OFFSET),
						registrationOffset);

				recordLists[i - registeredFiles] = rl;
			}

			for (SRBMetaDataRecordList recordList : recordLists) {
				try {// HACK
					files.remove(0);
					paths.remove(0);
				} catch (ArrayIndexOutOfBoundsException e) {
					break;
				}
			}

			return recordLists;
		}

		void register() {
			try {
				if (!newFiles) {
					return;
				}
				if (files.size() < 5) {
					return;
				}
				fileSystem.srbBulkLoad(catalogType, bloadFilePath,
						getFileRegistry());
			} catch (IOException e) {
			}
		}

		void forceRegister() {
			if (!newFiles) {
				return;
			}

			try {
				fileSystem.srbBulkLoad(catalogType, bloadFilePath,
						getFileRegistry());
			} catch (IOException e) {
			}
		}
	}

	/**
	 * Tests whether the application can read from the container denoted by this
	 * abstract pathname.
	 * 
	 * @return <code>true</code> if and only if the container specified by this
	 *         abstract pathname exists <em>and</em> can be read; otherwise
	 *         <code>false</code>.
	 */
	@Override
	public boolean canRead() {
		try {
			if (exists()) {
				MetaDataRecordList[] rl = query(GeneralMetaData.ACCESS_CONSTRAINT);
				if (rl != null) {
					if (rl[0].getStringValue(0).equals("all")
							|| rl[0].getStringValue(0).equals("read")) {
						return true;
					}
				}
			}
		} catch (IOException e) {
		}

		return false;
	}

	/**
	 * Tests whether the application can modify to the container denoted by this
	 * abstract pathname.
	 * 
	 * @return <code>true</code> if and only if the container system actually
	 *         contains a file denoted by this abstract pathname <em>and</em>
	 *         the application is allowed to write to the container; otherwise
	 *         <code>false</code>.
	 */
	@Override
	public boolean canWrite() {
		try {
			if (exists()) {
				MetaDataRecordList[] rl = query(GeneralMetaData.ACCESS_CONSTRAINT);
				if (rl != null) {
					if (rl[0].getStringValue(0).equals("all")
							|| rl[0].getStringValue(0).equals("write")) {
						return true;
					}
				}
			}
		} catch (IOException e) {
		}

		return false;
	}

	/**
	 * Compares two containers lexicographically.
	 * 
	 * @param pathname
	 *            The container to be compared to this abstract pathname
	 * 
	 * @return Zero if the argument is equal to this container, a value less
	 *         than zero if this container is lexicographically less than the
	 *         argument, or a value greater than zero if this container is
	 *         lexicographically greater than the argument
	 */
	@Override
	public int compareTo(final GeneralFile pathname) {
		return getAbsolutePath().compareTo(
				((SRBContainer) pathname).getAbsolutePath());
	}

	/**
	 * Compares this container to another object. If the other object is an
	 * container, then this function behaves like <code>{@link
	 * #compareTo(GeneralFile)}</code> . Otherwise, it throws a
	 * <code>ClassCastException</code>, since containers can only be compared to
	 * containers.
	 * 
	 * @param o
	 *            The <code>Object</code> to be compared to this abstract
	 *            pathname
	 * 
	 * @return If the argument is an container, returns zero if the argument is
	 *         equal to this container, a value less than zero if this container
	 *         is lexicographically less than the argument, or a value greater
	 *         than zero if this container is lexicographically greater than the
	 *         argument
	 *         <P>
	 * @throws ClassCastException
	 *             - if the argument is not an container
	 */
	@Override
	public int compareTo(final Object o) throws ClassCastException {
		return compareTo((SRBContainer) o);
	}

	/**
	 * Atomically creates a new, empty container named by this abstract pathname
	 * if and only if a container with this name does not yet exist.
	 */
	@Override
	public boolean createNewFile() {
		try {
			if (!exists()) {
				srbFileSystem
						.srbContainerCreate(catalogType, getAbsolutePath(),
								dataType, resource, containerMaxSize);
				return true;
			}
		} catch (IOException e) {
			// don't do anything.
		}

		return false;
	}

	/**
	 * Creates an empty file in the default temporary-file directory, using the
	 * given prefix and suffix to generate its name. Invoking this method is
	 * equivalent to invoking <code>{@link #createTempFile(java.lang.String,
	 * java.lang.String, edu.sdsc.grid.io.GeneralFile)
	 * createTempFile(prefix,&nbsp;suffix,&nbsp;null)}</code> .
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
	 * @return An abstract pathname denoting a newly-created empty file
	 * 
	 * @throws IllegalArgumentException
	 *             If the <code>prefix</code> argument contains fewer than three
	 *             characters
	 * 
	 * @throws IOException
	 *             If a file could not be created
	 */
	public static GeneralFile createTempFile(final String prefix,
			final String suffix) throws IOException, IllegalArgumentException {
		return createTempFile(prefix, suffix, null);
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
	 * If the <code>directory</code> argument is <code>null</code> then the SRB
	 * default temporary-file directory will be used. The default temporary-file
	 * directory is specified by the system property <code>java.io.tmpdir</code>
	 * . On UNIX systems the default value of this property is typically
	 * <code>"/tmp"</code> or <code>"/var/tmp"</code>; on Microsoft Windows
	 * systems it is typically <code>"c:\\temp"</code>. A different value may be
	 * given to this system property when the Java virtual machine is invoked,
	 * but programmatic changes to this property are not guaranteed to have any
	 * effect upon the the temporary directory used by this method.
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
	public static GeneralFile createTempFile(final String prefix,
			String suffix, GeneralFile directory) throws IOException,
			IllegalArgumentException {
		if (prefix == null) {
			throw new NullPointerException();
		}
		if (prefix.length() < 3) {
			throw new IllegalArgumentException("Prefix string too short");
		}

		if (suffix == null) {
			suffix = ".tmp";
		}

		if (directory == null) {
			SRBFileSystem fs = new SRBFileSystem();
			directory = new SRBContainer(fs, "");
		}

		SRBContainer temp = new SRBContainer((SRBFile) directory, prefix
				+ suffix);
		if (temp.exists()) {
			throw new IOException();
		}
		temp.createNewFile();
		return temp;
	}

	/**
	 * Deletes the container denoted by this SRBContainer. The container must be
	 * empty in order to be deleted.
	 */
	@Override
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
	@Override
	public boolean delete(final boolean force) {
		try {
			if (exists()) {
				srbFileSystem.srbRmContainer(catalogType, getAbsolutePath(),
						true);
				return true;
			}
		} catch (IOException e) {
			return false;
		}
		return false;
	}

	/**
	 * Tests this abstract pathname for equality with the given object. Returns
	 * <code>true</code> if and only if the argument is not <code>null</code>
	 * and is an abstract pathname that denotes the same container as this
	 * abstract pathname.
	 * 
	 * @param obj
	 *            The object to be compared with this abstract pathname
	 * 
	 * @return <code>true</code> if and only if the objects are the same;
	 *         <code>false</code> otherwise
	 */
	@Override
	public boolean equals(final Object obj) {
		try {
			SRBContainer temp = (SRBContainer) obj;

			return getAbsolutePath().equals(temp.getAbsolutePath());
		} catch (ClassCastException e) {
			// ignore
		}

		return false;
	}

	/**
	 * Tests whether the container denoted by this abstract pathname exists.
	 * 
	 * @return <code>true</code> if and only if the container denoted by this
	 *         abstract pathname exists; <code>false</code> otherwise
	 */
	@Override
	public boolean exists() {
		try {
			MetaDataRecordList[] rl = null;
			int operator = MetaDataCondition.EQUAL;

			// if it is a file
			MetaDataCondition conditions[] = { MetaDataSet.newCondition(
					SRBMetaDataSet.CONTAINER_NAME, operator, getAbsolutePath()) };
			MetaDataSelect selects[] = { MetaDataSet
					.newSelection(SRBMetaDataSet.CONTAINER_NAME) };

			rl = srbFileSystem.query(conditions, selects, 3);

			if (rl != null) {
				return true;
			}
		} catch (IOException e) {
			// don't do anything
		}

		return false;
	}

	/**
	 * Returns the absolute form of this abstract pathname. Equivalent to
	 * <code>new&nbsp;SRBContainer(this.{@link #getFileSystem}(),
	 * this.{@link #getAbsolutePath}())</code>.
	 * 
	 * @return The absolute abstract pathname denoting the same container as
	 *         this abstract pathname
	 */
	@Override
	public GeneralFile getAbsoluteFile() {
		return new SRBContainer(srbFileSystem, getAbsolutePath());
	}

	/**
	 * Returns the canonical form of this abstract pathname. Equivalent to
	 * <code>new&nbsp;SRBContainer(this.{@link #getCanonicalPath}())</code>.
	 * 
	 * @return The canonical pathname string denoting the same container as this
	 *         abstract pathname
	 * 
	 * @throws IOException
	 *             If an I/O error occurs, which is possible because the
	 *             construction of the canonical pathname may require filesystem
	 *             queries
	 */
	@Override
	public GeneralFile getCanonicalFile() {
		return new SRBContainer(srbFileSystem, getAbsolutePath());
	}

	/**
	 * Returns the abstract pathname of this abstract pathname's parent, or
	 * <code>null</code> if this pathname does not name a parent directory.
	 * 
	 * <p>
	 * The <em>parent</em> of an abstract pathname consists of the pathname's
	 * prefix, if any, and each name in the pathname's name sequence except for
	 * the last. If the name sequence is empty then the pathname does not name a
	 * parent directory.
	 * 
	 * @return The abstract pathname of the parent directory named by this
	 *         abstract pathname, or <code>null</code> if this pathname does not
	 *         name a parent
	 */
	@Override
	public GeneralFile getParentFile() {
		return new SRBContainer(srbFileSystem, getParent());
	}

	/**
	 * Tests whether the object denoted by this abstract pathname is a
	 * container.
	 * 
	 * @return <code>true</code> if and only if the file denoted by this
	 *         abstract pathname exists <em>and</em> is a container;
	 *         <code>false</code> otherwise
	 */
	@Override
	public boolean isContainer() {
		if (exists()) {
			return true;
		}

		return false;
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
	 * file. A file is <em>normal</em> if it is not a directory or a container.
	 * Any non-directory or other subclass of SRBFile, such as a SRBContainer,
	 * file created by a Java application is guaranteed to be a normal file.
	 * 
	 * @return <code>true</code> if and only if the file denoted by this
	 *         abstract pathname exists <em>and</em> is a normal file;
	 *         <code>false</code> otherwise
	 */
	@Override
	public boolean isFile() {
		return false;
	}

	/**
	 * Tests whether the file named by this abstract pathname is a hidden file.
	 * 
	 * @return <code>true</code> if and only if the file denoted by this
	 *         abstract pathname is hidden.
	 */
	@Override
	public boolean isHidden() {
		return false;// SRB files can't be hidden?
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
	@Override
	public long lastModified() {
		try {
			MetaDataRecordList[] rl = query(GeneralMetaData.MODIFICATION_DATE);// wrong
			// attribute,
			// need
			// epoch
			// long
			if (rl != null) {
				return Long.parseLong(rl[0].getValue(
						GeneralMetaData.MODIFICATION_DATE).toString());
			}
		} catch (IOException e) {
			return 0;
		}

		return 0;
	}

	/**
	 * Returns the length of the container denoted by this abstract pathname.
	 * The return value is unspecified if this pathname denotes a directory.
	 * This is the actual amount of data in the container, use
	 * getContainerMaxSize() to get the maximum amount of data the container can
	 * hold.
	 * 
	 * @return The length, in bytes, of the container denoted by this abstract
	 *         pathname, or <code>0L</code> if the container does not exist
	 */
	@Override
	public long length() {
		try {
			MetaDataRecordList[] rl = query(SRBMetaDataSet.CONTAINER_SIZE);
			if (rl != null) {
				return Long.parseLong(rl[0].getValue(
						SRBMetaDataSet.CONTAINER_SIZE).toString());
			}
		} catch (IOException e) {
			return 0;
		}

		return 0;
	}

	/**
	 * Returns an array of strings naming the files and directories in the
	 * container denoted by this abstract pathname.
	 * <P>
	 * There is no guarantee that the name strings in the resulting array will
	 * appear in any specific order; they are not, in particular, guaranteed to
	 * appear in alphabetical order.
	 * 
	 * @return An array of strings naming the files and directories in the
	 *         container denoted by this abstract pathname. The array will be
	 *         empty if the directory is empty. Returns null if this abstract
	 *         pathname does not denote a directory, or if an I/O error occurs.
	 */
	@Override
	public String[] list() {
		String list[] = {};
		String fileName, containerName = getName();

		try {
			MetaDataRecordList[] rl = query(StandardMetaData.FILE_NAME);

			if (rl != null) {
				list = new String[rl.length - 1];

				for (int i = 0, j = 0; i < rl.length; i++) {
					fileName = rl[i].getStringValue(0);
					if (!fileName.equals(containerName)) {
						list[j] = fileName;
						j++;
					}
				}
			}
		} catch (IOException e) {
			return null;
		}

		return list;
	}

	/**
	 * Creates the container named by this abstract pathname. Same as calling
	 * the createNewFile method.
	 */
	@Override
	public boolean mkdir() {
		return createNewFile();
	}

	/**
	 * Renames the container denoted by this abstract pathname.
	 * 
	 * @param dest
	 *            The new abstract pathname for the named container
	 * 
	 * @throws IllegalArgumentException
	 *             If parameter <code>dest</code> is not a <code>SRBFile</code>.
	 */
	@Override
	public boolean renameTo(final GeneralFile dest) {
		try {
			if (exists()) {
				srbFileSystem.srbModifyDataset(catalogType, getName(),
						getParent(), resource, null, dest.getName(), null,
						SRBMetaDataSet.D_CHANGE_DNAME);
				return true;
			}
		} catch (IOException e) {
			// ignore
		}
		return false;
	}
}
