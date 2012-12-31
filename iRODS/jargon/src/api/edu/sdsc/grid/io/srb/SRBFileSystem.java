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
//  SRBFileSystem.java  -  edu.sdsc.grid.io.srb.SRBFileSystem
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-edu.sdsc.grid.io.GeneralFileSystem
//            |
//            +-edu.sdsc.grid.io.RemoteFileSystem
//                  |
//                  +-edu.sdsc.grid.io.srb.SRBFileSystem
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.srb;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.ProtocolException;
import java.net.URL;

import org.ietf.jgss.GSSCredential;

import edu.sdsc.grid.io.DirectoryMetaData;
import edu.sdsc.grid.io.FileMetaData;
import edu.sdsc.grid.io.GeneralAccount;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.GeneralFileSystem;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.ProtocolCatalog;
import edu.sdsc.grid.io.RemoteFileSystem;
import edu.sdsc.grid.io.StandardMetaData;

/**
 * The SRBFileSystem class is the class for connection implementations to SRB
 * servers. It provides the framework to support a wide range of SRB semantics.
 * Specifically, the functions needed to interact with a SRB server.
 * <P>
 * Many of the methods remain package private, though a few "srb..." methods
 * have been made public. These methods are generally very low level and their
 * use is not recommended. Clear, high level equivalents are being developed
 * elsewhere in the JARGON API, and in the SRBAdmin tool. If you require the
 * functionality of any of these methods and it is not yet available as a high
 * level method, please notify the administrator of the JARGON package and we
 * will try to include your request in the next release.
 * 
 * @author Lucas Gilbert, San Diego Supercomputer Center
 * @since JARGON1.0
 * @see edu.sdsc.grid.io.srb.SRBCommands
 */
public class SRBFileSystem extends RemoteFileSystem {

	/**
	 * The SRB only has one root, "/".
	 */
	public static final String SRB_ROOT = "/";

	static {
		roots = new String[] { SRB_ROOT };
	}

	public static final int DELETE_TYPE_LOCATION = 1;
	public static final int DELETE_TYPE_USER = 2;
	public static final int DELETE_TYPE_RESOURCE = 3;

	/**
	 * Length of the user info byte array, which is passed during the connection
	 * handshake.
	 */
	static final int USER_INFO_BUFFER_LENGTH = 400;

	static final int USER_INFO_BUFFER_LENGTH_3_0 = 464;

	/**
	 * Length of mdas authorization password
	 */
	static final int MDAS_PASSWORD_LENGTH = 32;

	/**
	 * Default number of records returned by a query
	 */
	static int DEFAULT_RECORDS_WANTED = GeneralFileSystem.DEFAULT_RECORDS_WANTED;

	/**
	 * Used for proxy commands, no result.
	 */
	public static final int PORTAL_OFF = 0; // 0x00000000

	/**
	 * Used for proxy commands, create a portal and put it in env PORTAL_ENV.
	 */
	public static final int PORTAL_ON = 1; // 0x00000001

	/**
	 * Used for proxy commands, send result to the InputStream
	 */
	public static final int PORTAL_STD_IN_OUT = 2; // 0x00000002

	static int TOTAL_METADATA_ATTRIBUTES; // set in constructor

	static int MAX_TOKEN; // set in constructor

	static int MAX_FILE_SIZE; // set in constructor

	/**
	 * For ticket connections to the SRB
	 */
	static String TICKET_USER = "ticketuser";

	/**
	 * For ticket connections to the SRB
	 */
	static String TICKET_USER_DOMAIN = "sdsc";

	/**
	 * Standard SRB path separator character represented as a string for
	 * convenience. This string contains a single character, namely
	 * <code>{@link SRBFile#PATH_SEPARATOR_CHAR}</code>.
	 * 
	 * @deprecated Use SRBFile separator and pathSeparator
	 */
	@Deprecated
	public static final String PATH_SEPARATOR = GeneralFile.separator;

	/**
	 * This object handles the socket protocol and communications with the Srb
	 * server.
	 */
	private SRBCommands commands;

	/**
	 * Use this account object instead of the parent class's GeneralAccount
	 * object. Just so you don't have to recast it all the time.
	 */
	private SRBAccount srbAccount;

	/**
	 * Keep a separate version string here. Have to keep track of the version on
	 * a per object basis, but SRBAccount.version is a static variable.
	 */
	private String version;
	private float versionNumber;

	/**
	 * Used to specify a port range available through a firewall. Needed because
	 * some SRB commands open new ports on the client machine.
	 */
	int MIN_PORT = -1;

	/**
	 * Used to specify a port range available through a firewall. Needed because
	 * some SRB commands open new ports on the client machine.
	 */
	int MAX_PORT = -1;

	static {
		if (!ProtocolCatalog.has(new SRBProtocol())) {
			ProtocolCatalog.add(new SRBProtocol());
		}
	}

	/**
	 * Opens a socket connection to read from and write to. Loads the default
	 * SRB user account information from their home directory. The account
	 * information stored in this object cannot be changed once constructed.
	 * <P>
	 * This constructor is provided for convenience however, it is recommended
	 * that all necessary data be sent to the constructor and not left to the
	 * defaults.
	 * 
	 * @throws FileNotFoundException
	 *             if the user data file cannot be found.
	 * @throws IOException
	 *             if an IOException occurs.
	 */
	public SRBFileSystem() throws FileNotFoundException, IOException {
		this(new SRBAccount());
	}

	/**
	 * Opens a socket connection to read from and write to. Opens the account
	 * held in the SRBAccount object. The account information stored in this
	 * object cannot be changed once constructed.
	 * 
	 * @param srbAccount
	 *            The SRB account information object.
	 * @throws NullPointerException
	 *             if srbAccount is null.
	 * @throws IOException
	 *             if an IOException occurs.
	 */
	public SRBFileSystem(final SRBAccount srbAccount) throws IOException,
			NullPointerException {
		setAccount(srbAccount);
		commands = new SRBCommands();

		int status = 0;
		try {
			int option = srbAccount.getOptions();

			// test if the are using GSI without the userinfo,
			// if so get the DN, then use that to get the userinfo
			if (((option == SRBAccount.GSI_AUTH) || (option == SRBAccount.GSI_DELEGATE))
					&& (srbAccount.getUserName() == null)
					&& (srbAccount.getDomainName() == null)) {
				// Connect using a ticket user (which can do some limited
				// queries)
				SRBAccount tempAccount = new SRBAccount(srbAccount.getHost(),
						srbAccount.getPort(), TICKET_USER, "", "",
						TICKET_USER_DOMAIN, "");
				SRBFileSystem ticketFileSystem = new SRBFileSystem(tempAccount);

				String dn = GSIAuth.getDN(srbAccount);
				GSSCredential credential = null;
				try {
					credential = GSIAuth.getCredential(srbAccount);
				} catch (Throwable e) {
					throw new IllegalArgumentException(
							"Invalid or missing credentials");// , e);
				}

				// Use the DN to query for the userinfo
				tempAccount = ticketFileSystem.srbGetUserByDn(
						SRBFile.MDAS_CATALOG, dn);

				// If this DN isn't listed try to substitute the email component
				// and
				// try again
				if (tempAccount == null) {
					String email = "/E=";
					int loc = dn.indexOf(email);
					if (loc >= 0) {
						dn = dn.substring(0, loc) + "/emailAddress="
								+ dn.substring(loc + email.length());
						tempAccount = ticketFileSystem.srbGetUserByDn(
								SRBFile.MDAS_CATALOG, dn);
						if (tempAccount == null) {
							throw new SRBException("User not found for DN="
									+ dn);
						}
					} else {
						throw new SRBException("User not found for DN=" + dn);
					}
				}

				// set up the rest of the necessary account parts
				srbAccount.setUserName(tempAccount.getUserName());
				srbAccount.setProxyUserName(tempAccount.getProxyUserName());
				srbAccount.setDomainName(tempAccount.getDomainName());
				srbAccount.setProxyDomainName(tempAccount.getProxyDomainName());
				srbAccount.setMcatZone(tempAccount.getMcatZone());
				srbAccount.setHomeDirectory(tempAccount.getHomeDirectory());

				// might as well
				if (srbAccount.getGSSCredential() == null) {
					srbAccount.setGSSCredential(credential);
				}

				setAccount(srbAccount);
			}

			status = commands.connect(srbAccount,
					createUserInfoBuffer(srbAccount));
		} catch (ProtocolException e) {
			/*
			 * The master SRB sends the client to another port in the
			 * 10,000-50,000 range. Sometimes the client fails to connect, for a
			 * variety of reasons. Trying to connect again to the SRB a second
			 * time will usually be successful.
			 */
			status = commands.connect(srbAccount,
					createUserInfoBuffer(srbAccount));
		}

		// SRB client/server version mismatch
		if (status == -1118) {
			System.err.println("SRB client/server version mismatch. "
					+ "Trying alternate handshake. error: " + status);

			setVersion(SRBAccount.getVersion());
			commands = new SRBCommands();
			status = commands.connect(srbAccount,
					createUserInfoBuffer(srbAccount));
		} else if (status == -1107 || status == -1113) {
			// give it another try just in case.
			status = commands.connect(srbAccount,
					createUserInfoBuffer(srbAccount));
		}

		if (status < 0) {
			throw new SRBException("Connection Failed", status);
		}

		// make sure there is a default storage resource
		if (srbAccount.getDefaultStorageResource() == null) {
			String resource = SRBFile.getAvailableResource(this);
			if (resource != null) {
				srbAccount.setDefaultStorageResource(resource);
				setAccount(srbAccount);
			}
		}
		// make sure there is a home directory
		if (srbAccount.getHomeDirectory() == null) {
			srbAccount.setHomeDirectory(GeneralFile.separator
					+ SRBFile.LOCAL_HOME_DIRECTORY + GeneralFile.separator
					+ srbAccount.getUserName() + "."
					+ srbAccount.getDomainName());
		}
	}

	/**
	 * Finalizes the object by explicitly letting go of each of its internally
	 * held values.
	 */
	@Override
	protected void finalize() throws Throwable {
		super.finalize();

		// remove this from the connection pool
		SRBFile.uriFileSystems.remove(this);

		close();
		if (commands != null) {
			commands = null;
		}
		if (account != null) {
			account = null;
		}
	}

	// General
	/**
	 * Loads the account information for this file system.
	 */
	@Override
	protected void setAccount(GeneralAccount account) throws IOException {
		if (account == null) {
			account = new SRBAccount();
		}

		srbAccount = (SRBAccount) account.clone();
		this.account = srbAccount;

		setVersion(SRBAccount.getVersion());
	}

	private void setVersion(final String version) {
		if (version == null) {
			return;
		}

		this.version = version;
		versionNumber = SRBAccount.getVersionNumber();
		if (versionNumber >= 3.3) {
			TOTAL_METADATA_ATTRIBUTES = 500;
			MAX_TOKEN = 500;
			MAX_FILE_SIZE = 2700;
		} else if (versionNumber >= 3.02) {
			TOTAL_METADATA_ATTRIBUTES = 500;
			MAX_TOKEN = 500;
			MAX_FILE_SIZE = 2700;
		} else if (versionNumber >= 3) {
			TOTAL_METADATA_ATTRIBUTES = 500;
			MAX_TOKEN = 500;
			MAX_FILE_SIZE = 500;
		} else if (versionNumber >= 2) {
			TOTAL_METADATA_ATTRIBUTES = 300;
			MAX_TOKEN = 200;
			MAX_FILE_SIZE = 400;
		} else if (versionNumber >= 1) {
			TOTAL_METADATA_ATTRIBUTES = 180;
			MAX_TOKEN = 200;
			MAX_FILE_SIZE = 400;
		} else {
			throw new IllegalArgumentException("Invalid version");
		}
	}

	/**
	 * Returns the account used by this SRBFileSystem.
	 */
	@Override
	public GeneralAccount getAccount() throws NullPointerException {
		if (srbAccount != null) {
			return (SRBAccount) srbAccount.clone();
		}

		throw new NullPointerException();
	}

	/**
	 * Returns the root directories of the SRB file system.
	 */
	@Override
	public String[] getRootDirectories() {
		return roots;
	}

	/**
	 * Set the default storage resource. Only used when there wasn't one
	 * provided and we had to query the SRB for it.
	 */
	void setDefaultStorageResource(final String resource) {
		srbAccount.setDefaultStorageResource(resource);
	}

	/**
	 * Only used by the SRBFile( uri ) constructor.
	 */
	void setProxyMcatZone(final String zone) {
		srbAccount.setProxyMcatZone(zone);
	}

	/**
	 * Only used by the SRBFile( uri ) constructor.
	 */
	void setMcatZone(final String zone) {
		srbAccount.setMcatZone(zone);
	}

	/**
	 * Used to specify a port range available through a firewall. Needed because
	 * some SRB commands open new ports on the client machine.
	 */
	public/* static */void setFirewallPorts(final int minPort, final int maxPort) {
		if ((minPort >= 0) && (maxPort >= 0)) {
			MIN_PORT = minPort;
			MAX_PORT = maxPort;
		}
	}

	/**
	 * Sets the default number of records that will be returned by a query. Must
	 * be a positive integer.
	 */
	public void setQueryRecordsWanted(final int num) {
		if (num > 0) {
			DEFAULT_RECORDS_WANTED = num;
		}
	}

	/**
	 * @return the default number of records that will be returned by a query.
	 */
	public int getQueryRecordsWanted() {
		return DEFAULT_RECORDS_WANTED;
	}

	/**
	 * @return the SRB password
	 */
	@Override
	public String getPassword() {
		return srbAccount.getPassword();
	}

	/**
	 * Currently, just the mdas text password as a byte array.
	 */
	byte[] getPasswordBytes() {
		byte password[] = new byte[MDAS_PASSWORD_LENGTH];

		System.arraycopy(srbAccount.getPassword().getBytes(), 0, password, 0,
				srbAccount.getPassword().length());

		return password;
	}

	/**
	 * @return the proxy user name
	 */
	public String getProxyUserName() {
		return srbAccount.getProxyUserName();
	}

	/**
	 * @return the proxy domain name
	 */
	public String getProxyDomainName() {
		return srbAccount.getProxyDomainName();
	}

	/**
	 * @return the options
	 */
	public int getOptions() {
		return srbAccount.getOptions();
	}

	/**
	 * @return the domain name used by the client. Only different from the
	 *         proxyDomainName for ticketed users.
	 */
	public String getDomainName() {
		return srbAccount.getDomainName();
	}

	/**
	 * @return the SRB version
	 */
	public String getVersion() {
		return version;
	}

	/**
	 * @return the default storage resource.
	 */
	public String getDefaultStorageResource() {
		return srbAccount.getDefaultStorageResource();
	}

	// SRB 3.0
	/**
	 * @return the proxy mcat zone.
	 */
	public String getProxyMcatZone() {
		return srbAccount.getProxyMcatZone();
	}

	/**
	 * @return the client mcat zone.
	 */
	public String getMcatZone() {
		return srbAccount.getMcatZone();
	}

	/**
	 * @return the exec file.
	 */
	public String getExecFile() {
		return srbAccount.getExecFile();
	}

	/**
	 * @return the version number
	 */
	public float getVersionNumber() {
		return versionNumber;
	}

	/**
	 * The number of bytes transfered by this filesystem object so far during
	 * the currently executing SRBFile.copyTo/copyFrom command.
	 * 
	 * @return the number of bytes that have been transfered so far.
	 */
	long fileCopyStatus() {
		return commands.getBytesMoved();
	}

	// GeneralFileSystem methods

	/**
	 * Queries the file server to find all files that match a set of conditions.
	 * For all those that match, the fields indicated in the select array are
	 * returned in the result object.
	 * 
	 * @param conditions
	 *            The conditional statements that describe the values to query
	 *            the server, like WHERE in SQL.
	 * @param selects
	 *            The attributes to be returned from those values that met the
	 *            conditions, like SELECT in SQL.
	 * @param recordsWanted
	 *            The number of values to return with the query, use the
	 *            getMoreRecords() method in MetaDataRecordList to continue the
	 *            search, if more records are available.
	 */
	@Override
	public MetaDataRecordList[] query(final MetaDataCondition[] conditions,
			final MetaDataSelect[] selects, final int recordsWanted)
			throws IOException {
		return query(conditions, selects, recordsWanted, false, false);
	}

	/**
	 * Queries the file server to find all files that match a set of conditions.
	 * For all those that match, the fields indicated in the select array are
	 * returned in the result object.
	 * 
	 * @param conditions
	 *            The conditional statements that describe the values to query
	 *            the server, like WHERE in SQL.
	 * @param selects
	 *            The attributes to be returned from those values that met the
	 *            conditions, like SELECT in SQL.
	 * @param recordsWanted
	 *            The number of values to return with the query, use the
	 *            getMoreRecords() method in MetaDataRecordList to continue the
	 *            search, if more records are available.
	 * @param orderBy
	 *            sorts the query's returned values. Ordered matching the order
	 *            of the selects array.
	 */
	public MetaDataRecordList[] query(final MetaDataCondition[] conditions,
			final MetaDataSelect[] selects, final int recordsWanted,
			final boolean orderBy) throws IOException {
		return query(conditions, selects, recordsWanted, orderBy, false);
	}

	/**
	 * Queries the file server to find all files that match a set of conditions.
	 * For all those that match, the fields indicated in the select array are
	 * returned in the result object.
	 * <P>
	 * While condition and select array objects have all been checked for
	 * self-consistency during their construction, there are additional problems
	 * that must be detected at query time:
	 * <P>
	 * <ul>
	 * <li>Redundant selection fields
	 * <li>Redundant condition fields
	 * <li>Fields incompatible with a file server
	 * </ul>
	 * <P>
	 * For instance, it is possible to build a condition object appropriate for
	 * the SRB, then pass that object in a local file system query. That will
	 * find that the condition is incompatible and generate a mismatch
	 * exception.
	 * <P>
	 * Query is implemented by the file-server-specific classes, like that for
	 * the SRB, FTP, etc. Those classes must re-map condition and select field
	 * names and operator codes to those required by a particular file server
	 * and protocol version. Once re-mapped, they issue the query and get
	 * results. The results are then mapped back to the standard public field
	 * names of the MetaDataGroups.
	 * <P>
	 * The orderBy variable sorts the query's returned values. The order will
	 * match the order of the selects array. <br>
	 * E.g., where selects[0] = SIZE<br>
	 * and selects[1] = OWNER<br>
	 * The files returned
	 * 
	 * @param conditions
	 *            The conditional statements that describe the values to query
	 *            the server, like WHERE in SQL and all conditions in the array
	 *            will be AND-ed together to form the query.
	 * @param selects
	 *            The attributes to be returned from those values that met the
	 *            conditions, like SELECT in SQL.
	 * @param recordsWanted
	 *            The number of values to return with the query, use the
	 *            getMoreRecords() method in MetaDataRecordList to continue the
	 *            search, if more records are available.
	 * @param orderBy
	 *            sorts the query's returned values. Ordered matching the order
	 *            of the selects array.
	 * @param nonDistinct
	 *            If true, allows redundencies in returned data.
	 */
	public MetaDataRecordList[] query(final MetaDataCondition[] conditions,
			final MetaDataSelect[] selects, final int recordsWanted,
			final boolean orderBy, final boolean nonDistinct)
			throws IOException {
		/*
		 * Query is implemented by the file-server-specific classes, like that
		 * for the SRB, FTP, etc. Those classes must re-map condition and select
		 * field names and operator codes to those required by a particular file
		 * server and protocol version. Once re-mapped, they issue the query and
		 * get results. The results are then mapped back to the standard public
		 * field names of the MetaDataGroups. So, if a MetaDataGroup uses a name
		 * like "file path", but the SRB calls it "data name", then query maps
		 * first from "file path" to "data name" before issuing the query, and
		 * then from "data name" back to "file path" within the results. The
		 * programmer using this API should never see the internal field names.
		 */
		// if (protocol == SRB)
		boolean hasZone = false;
		if (conditions != null) {
			for (MetaDataCondition condition : conditions) {
				if ((condition != null)
						&& (condition.getFieldName().equals(
								StandardMetaData.DIRECTORY_NAME)
								|| condition
										.getFieldName()
										.equals(DirectoryMetaData.PARENT_DIRECTORY_NAME) || condition
								.getFieldName().equals(
										SRBMetaDataSet.CONTAINER_NAME))) {
					hasZone = true;
					break;
				}
			}
			if (!hasZone) {
				for (int i = 0; i < conditions.length; i++) {
					if ((conditions[i] != null)
							&& conditions[i].getFieldName().equals(
									SRBMetaDataSet.CURRENT_ZONE)) {
						conditions[i] = MetaDataSet.newCondition(
								StandardMetaData.DIRECTORY_NAME,
								MetaDataCondition.EQUAL, "MCAT_NAME="
										+ conditions[i].getStringValue());
						break;
					}
				}
			} else {
				for (int i = 0; i < conditions.length; i++) {
					if ((conditions[i] != null)
							&& conditions[i].getFieldName().equals(
									SRBMetaDataSet.CURRENT_ZONE)) {
						conditions[i] = null;
						break;
					}
				}
			}
		}

		return srbGenQuery(SRBFile.catalogType, null, conditions, selects,
				recordsWanted, orderBy, nonDistinct);
	}

	/**
	 * Tests this filesystem object for equality with the given object. Returns
	 * <code>true</code> if and only if the argument is not <code>null</code>
	 * and both are filesystem objects connected to the same filesystem using
	 * the same account information.
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
			if (obj == null) {
				return false;
			}

			SRBFileSystem temp = (SRBFileSystem) obj;

			if (getAccount().equals(temp.getAccount())) {
				if (isConnected() == temp.isConnected()) {
					return true;
				}
			}
		} catch (ClassCastException e) {
			return false;
		}
		return false;
	}

	/**
	 * Checks if the socket is connected.
	 */
	@Override
	public boolean isConnected() {
		return commands.isConnected();
	}

	/**
	 * Returns a string representation of this file system object. The string is
	 * formated according to the SRB URI model. Note: the user password will not
	 * be included in the URI.
	 */
	@Override
	public String toString() {
		return new String("srb://" + getUserName() + "." + getDomainName()
				+ "@" + getHost() + ":" + getPort());
	}

	void reconnect() throws IOException {
		close();
		commands.connect(srbAccount, createUserInfoBuffer(srbAccount));
	}

	/**
	 * Closes the connection to the SRB file system. The filesystem cannot be
	 * reconnected after this method is called. If this object, or another
	 * object which uses this filesystem, tries to send a command to the server
	 * a ClosedChannelException will be thrown.
	 */
	public void close() throws IOException {
		commands.close();
	}

	/**
	 * Returns if the connection to the SRB has been closed or not.
	 * 
	 * @return true if the connection has been closed
	 */
	public boolean isClosed() throws IOException {
		return commands.isClosed();
	}

	// ----------------------------------------------------------------------
	// SRB Methods
	// ----------------------------------------------------------------------
	/**
	 * Prepares the userInfoBuffer for transfer by loading the user info. The
	 * user info was either passed to the constructor or obtained from Mdas
	 * files.
	 * <P>
	 * 
	 * @return userInfoBuffer Byte array which gets transfered to the srb server
	 */
	/*
	 * private byte[] createUserInfoBuffer( ) { return
	 * createUserInfoBuffer(account); }
	 */

	static byte[] createUserInfoBuffer(final SRBAccount account) {
		/*
		 * Zone srb 3.0 handshake typedef struct StartupInfo { // PacketHdr hdr;
		 * char proxyUserName[NAMEDATALEN]; // proxy User Name char
		 * proxyDomainName[NAMEDATALEN]; char proxyMcatZone[NAMEDATALEN]; char
		 * proxyAuth[NAMEDATALEN]; char clientUserName[NAMEDATALEN]; // proxy
		 * User Name char clientDomainName[NAMEDATALEN]; char
		 * clientMcatZone[NAMEDATALEN]; char clientAuth[NAMEDATALEN]; char
		 * version[PATH_SIZE]; // The version number char options[ARGV_SIZE]; //
		 * possible additional args char execFile[ARGV_SIZE]; // possible
		 * backend to use } StartupInfo;
		 */
		byte userInfoBuffer[] = null;
		// total length of the user info buffer,
		// LSBF problem
		// including len and messageType, byte value 190,
		// the int value is the same as USER_INFO_BUFFER_LENGTH
		byte bufferLength[] = { 0, 0, 1, -112 };
		byte messageType = 7; // see SRB c client list of message types
		String temp = null;

		if (SRBAccount.getVersion().equals(SRBAccount.SRB_VERSION_2)
				|| SRBAccount.getVersion().equals(SRBAccount.SRB_VERSION_1_1_8)) {
			userInfoBuffer = new byte[USER_INFO_BUFFER_LENGTH];

			System.arraycopy(bufferLength, 0, userInfoBuffer, 0,
					bufferLength.length);

			// messageType
			userInfoBuffer[4] = messageType;

			// ProxyUserName
			System.arraycopy(account.getProxyUserName().getBytes(), 0,
					userInfoBuffer, 8, account.getProxyUserName().length());

			// ProxyDomainName
			System.arraycopy(account.getProxyDomainName().getBytes(), 0,
					userInfoBuffer, 40, account.getProxyDomainName().length());

			// ClientUserName
			System.arraycopy(account.getUserName().getBytes(), 0,
					userInfoBuffer, 72, account.getUserName().length());

			// ClientDomainName
			System.arraycopy(account.getDomainName().getBytes(), 0,
					userInfoBuffer, 104, account.getDomainName().length());

			// Options
			userInfoBuffer[136] = (byte) account.getOptions();

			// Version
			System.arraycopy(SRBAccount.getVersion().getBytes(), 0,
					userInfoBuffer, 264, SRBAccount.getVersion().length());
		} else {
			userInfoBuffer = new byte[USER_INFO_BUFFER_LENGTH_3_0];

			// the length got longer, now 1D0,
			bufferLength[3] = -48;

			System.arraycopy(bufferLength, 0, userInfoBuffer, 0,
					bufferLength.length);

			// messageType
			userInfoBuffer[4] = messageType;

			// ProxyUserName
			System.arraycopy(account.getProxyUserName().getBytes(), 0,
					userInfoBuffer, 8, account.getProxyUserName().length());

			// ProxyDomainName
			System.arraycopy(account.getProxyDomainName().getBytes(), 0,
					userInfoBuffer, 40, account.getProxyDomainName().length());

			// ProxyMcatZone
			temp = account.getProxyMcatZone();
			if (temp != null) {
				System.arraycopy(account.getProxyMcatZone().getBytes(), 0,
						userInfoBuffer, 72, account.getProxyMcatZone().length());
			}

			// ClientUserName
			System.arraycopy(account.getUserName().getBytes(), 0,
					userInfoBuffer, 104, account.getUserName().length());

			// ClientDomainName
			System.arraycopy(account.getDomainName().getBytes(), 0,
					userInfoBuffer, 136, account.getDomainName().length());

			// ClientMcatZone
			temp = account.getMcatZone();
			if (temp != null) {
				System.arraycopy(account.getMcatZone().getBytes(), 0,
						userInfoBuffer, 168, account.getMcatZone().length());
			}

			// these two got switched
			// Version
			System.arraycopy(SRBAccount.getVersion().getBytes(), 0,
					userInfoBuffer, 200, SRBAccount.getVersion().length());

			// Options
			userInfoBuffer[264] = (byte) account.getOptions();

			// execFile
			if (account.getExecFile() != null) {
				System.arraycopy(account.getExecFile().getBytes(), 0,
						userInfoBuffer, 328, account.getExecFile().length());
			}
		}

		return userInfoBuffer;
	}

	/**
	 * Proxy Operation that executes a command. The results of the command will
	 * be returned by the InputStream. The protocol of the return value on the
	 * InputStream depends on the command that was run. The InputStream is
	 * opened on a different port than the main SRB connection. It can be read
	 * independently of other SRB calls.
	 * 
	 * @param command
	 *            The command to run.
	 * @param commandArgs
	 *            The command argument string.
	 * 
	 * @return any byte stream output.
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	public InputStream executeProxyCommand(final String command,
			final String commandArgs) throws IOException {
		// This method can't be done at this level,
		// So it had to be passed to SRBCommand
		// Plus dealing with the return value as a byte[] wasn't viable

		return srbExecCommand(command, commandArgs, null, -1);
	}

	/**
	 * Proxy Operation that executes a command.
	 * 
	 * @param command
	 *            The command to run.
	 * @param commandArgs
	 *            The command argument string.
	 * @param hostAddress
	 *            The host address where this proxy operation should be
	 *            performed. null = the server for the current connect.
	 * @param portalFlag
	 *            The portal flag. Valid flags are - PORTAL_OFF, PORTAL_ON,
	 *            PORTAL_STD_IN_OUT.
	 * 
	 * @return any byte stream output.
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	public InputStream executeProxyCommand(final String command,
			final String commandArgs, final String hostAddress,
			final int portalFlag) throws IOException {
		return srbExecCommand(command, commandArgs, hostAddress, portalFlag);
	}

	/**
	 * Proxy Operation that executes a command. Only valid for version 3.x.x and
	 * above. Method will not work for version 2.x.x, or before.
	 * 
	 * @param command
	 *            The command to run.
	 * @param commandArgs
	 *            The command argument string.
	 * @param hostAddress
	 *            The host address where this proxy operation should be
	 *            performed. null = the server for the current connect.
	 * @param fileName
	 *            The SRB path to a file to perform proxy operation on.
	 * @param portalFlag
	 *            The portal flag. Valid flags are - PORTAL_OFF, PORTAL_ON,
	 *            PORTAL_STD_IN_OUT.
	 * 
	 * @return any byte stream output.
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	public InputStream executeProxyCommand(final String command,
			final String commandArgs, final String hostAddress,
			final String fileName, final int portalFlag) throws IOException {
		return srbExecCommand(command, commandArgs, hostAddress, fileName,
				portalFlag);
	}

	/**
	 * If this resource is a 'http file system' type resource, then the
	 * <code>url</code> will be registered with that resource as the abstract
	 * pathname <code>file</code>. The URL can then be read using the SRBFile,
	 * SRBRandomAccessFile and others as if it were a standard SRB file.
	 */
	public void registerURL(final SRBFile file, final URL url)
			throws IOException {
		srbRegisterDataset(0, file.getName(), "URL", file.getResource(),
				file.getParent(), url.toString(), 0);
	}

	// ----------------------------------------------------------------------
	// Command methods
	// ----------------------------------------------------------------------
	/*
	 * Register New User: srbRegisterUser(catalogType, userName, selDomain,
	 * userPasswd, selUserType, userAddr, userPhone, userEMail);
	 * 
	 * Register New Group: srbRegisterUserGrp(catalogType, groupName,
	 * groupPasswd, "group", groupAddr, groupPhone, groupEMail );
	 * 
	 * Add Group to User: srbModifyUser(catalogType, selUserName+"@"+selDomain,
	 * selGroup1, SRBMetaDataSet.U_INSERT_GROUP);
	 * 
	 * Delete Group from User: srbModifyUser(catalogType,
	 * selUserName+"@"+selDomain, selDelGroup1, SRBMetaDataSet.U_DELETE_GROUP);
	 * 
	 * Add Owner to Group: srbModifyUser(catalogType, selUserName+"@"+selDomain,
	 * selGroup2, SRBMetaDataSet.U_ADD_GROUP_OWNER);
	 * 
	 * Delete a Group: srbModifyUser(catalogType, selUserName+"@"+selDomain,
	 * selDelGroup2, SRBMetaDataSet.U_REMOVE_GROUP_OWNER);
	 * 
	 * 
	 * Change Info field(s): Change Address: srbModifyUser(catalogType,
	 * selUserName+"@"+selDomain, userAddress, SRBMetaDataSet.U_UPDATE_ADDRESS);
	 * 
	 * Change Email: srbModifyUser(catalogType, selUserName+"@"+selDomain,
	 * userEmail, SRBMetaDataSet.U_UPDATE_EMAIL);
	 * 
	 * Change Phone: srbModifyUser(catalogType, selUserName+"@"+selDomain,
	 * userPhone, SRBMetaDataSet.U_UPDATE_PHONE);
	 * 
	 * Change password: srbModifyUser(catalogType, selUserName+"@"+selDomain,
	 * userPasswd, SRBMetaDataSet.SU_CHANGE_PASSWORD);
	 * 
	 * Add Authentication Scheme: srbModifyUser(catalogType,
	 * selUserName+"@"+selDomain, new String(selAuthScheme + ":" + inputDn),
	 * SRBMetaDataSet.U_INSERT_AUTH_MAP);
	 * 
	 * Delete Authentication Scheme: srbModifyUser(catalogType,
	 * selUserName+"@"+selDomain, selDn, SRBMetaDataSet.U_DELETE_AUTH_MAP);
	 * 
	 * Change User Type: srbModifyUser(catalogType, selUserName+"@"+selDomain,
	 * selUType, SRBMetaDataSet.U_CHANGE_TYPE);
	 * 
	 * Change Zone: srbModifyZone(catalogType, selZone, selUserName, selDomain,
	 * "", "", "", SRBMetaDataSet.Z_MODIFY_ZONE_FOR_USER );
	 */

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjCreate
	 */
	int srbObjCreate(final int catType, final String fileName,
			String dataTypeName, String resourceName, String collectionName,
			String serverLocalPath, final long dataSize) throws IOException {
		if (dataTypeName == null) {
			dataTypeName = "generic";// DefMdasDataTypeName
		}

		if (resourceName == null) {
			resourceName = getDefaultStorageResource();// DefMdasResourceName
		}

		if (collectionName == null) {
			collectionName = getHomeDirectory();// DefMdasCollectionName
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + GeneralFile.separator
					+ collectionName;
		}

		if (serverLocalPath == null) {
			serverLocalPath = "";
		}

		return commands.srbObjCreate(catType, fileName, dataTypeName,
				resourceName, collectionName, serverLocalPath, dataSize);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjOpen
	 */
	int srbObjOpen(final String objID, final int openFlag, String collectionName)
			throws IOException {
		if (collectionName == null) {
			collectionName = getHomeDirectory();
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + GeneralFile.separator
					+ collectionName;
		}

		return commands.srbObjOpen(objID, openFlag, collectionName);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjClose
	 */
	void srbObjClose(final int srbFD) throws IOException {

		commands.srbObjClose(srbFD);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjUnlink
	 */
	void srbObjUnlink(final String objID, String collectionName)
			throws IOException {
		if (collectionName == null) {
			collectionName = getHomeDirectory();
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + GeneralFile.separator
					+ collectionName;
		}

		commands.srbObjUnlink(objID, collectionName);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjRead
	 */
	byte[] srbObjRead(final int srbFD, final int length) throws IOException {

		return commands.srbObjRead(srbFD, length);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjWrite
	 */
	int srbObjWrite(final int srbFD, final byte outputBuffer[], final int length)
			throws IOException {
		// should 0 = write whole file?
		if (length <= 0) {
			return 0;
		}

		return commands.srbObjWrite(srbFD, outputBuffer, length);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjSeek
	 */
	void srbObjSeek(final int srbFD, final long offset, final int whence)
			throws IOException {

		commands.srbObjSeek(srbFD, offset, whence);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjSync
	 */
	void srbObjSync(final int srbFD) throws IOException {

		commands.srbObjSync(srbFD);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjStat
	 */
	long[] srbObjStat(final int catType, String pathName, int myType)
			throws IOException {
		if (pathName == null) {
			pathName = getHomeDirectory();
		} else if (!pathName.startsWith(SRB_ROOT)) {
			pathName = getHomeDirectory() + GeneralFile.separator + pathName;
		}
		/*
		 * Definition for isDir in myType in srbObjStat call #define IS_UNKNOWN
		 * -1 // don't know if it is file or dir #define IS_FILE 0 // this is a
		 * file #define IS_DIR_1 1 // is a collection. new desc #define IS_DIR_2
		 * 2 // is a collection. listing data #define IS_DIR_3 3 // is a
		 * collection. done listing data #define IS_DIR_4 4 // is a collection.
		 * listing collection
		 */
		if ((myType > 4) || (myType < -1)) {
			myType = -1;
		}

		return commands.srbObjStat(catType, pathName, myType);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjStat64
	 */
	long[] srbObjStat64(final int catType, String collectionName,
			final String fileName) throws IOException {
		if (collectionName == null) {
			collectionName = getHomeDirectory();
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + GeneralFile.separator
					+ collectionName;
		}

		// Make sure collectionName ends with '/'
		if (collectionName.substring(collectionName.length() - 1) != GeneralFile.separator) {
			// don't use: System.getProperty( "file.separator" )
			// because the srb is expecting '/'
			if (collectionName.substring(collectionName.length() - 1) == System
					.getProperty("file.separator")) {
				collectionName = collectionName.substring(0,
						collectionName.length() - 2)
						+ GeneralFile.separator;
			} else {
				collectionName = collectionName + GeneralFile.separator;
			}
		}

		return commands.srbObjStat64(catType, collectionName + fileName);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjReplicate
	 */
	void srbObjReplicate(final int catType, final String objID,
			String collectionName, String newResourceName,
			final String newPathName) throws IOException {
		if (collectionName == null) {
			collectionName = getHomeDirectory();
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + GeneralFile.separator
					+ collectionName;
		}

		if (newResourceName == null) {
			newResourceName = getDefaultStorageResource();// DefMdasResourceName
		}

		commands.srbObjReplicate(catType, objID, collectionName,
				newResourceName, newPathName);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjMove
	 */
	void srbObjMove(final int catType, final String objID,
			String collectionName, String srcResource, String newResourceName,
			final String newPathName, String container) throws IOException {
		if (collectionName == null) {
			collectionName = getHomeDirectory();
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + collectionName;
		}

		if (srcResource == null) {
			srcResource = "";
		}
		if (newResourceName == null) {
			newResourceName = getDefaultStorageResource();// DefMdasResourceName
		}
		if (container == null) {
			container = "";
		}

		commands.srbObjMove(catType, objID, collectionName, srcResource,
				newResourceName, newPathName, container);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjProxyOpr
	 */
	byte[] srbObjProxyOpr(final int operation, final int inputInt1,
			final int inputInt2, final int inputInt3, final int inputInt4,
			final String inputStr1, final String inputStr2,
			final String inputStr3, final String inputStr4,
			final byte[] inputBStrm1, final byte[] inputBStrm2,
			final byte[] inputBStrm3) throws IOException {

		return commands.srbObjProxyOpr(operation, inputInt1, inputInt2,
				inputInt3, inputInt4, inputStr1, inputStr2, inputStr3,
				inputStr4, inputBStrm1, inputBStrm2, inputBStrm3);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbCollSeek
	 */
	void srbCollSeek(final int srbFD, final int offset, final int whence,
			final int is64Flag) throws IOException {

		commands.srbCollSeek(srbFD, offset, whence, is64Flag);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbGetDatasetInfo
	 */
	SRBMetaDataRecordList[] srbGetDatasetInfo(final int catType,
			final String objID, String collectionName, int recordsWanted)
			throws IOException {
		if (collectionName == null) {
			collectionName = getHomeDirectory();
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + collectionName;
		}

		if (recordsWanted < 1) {
			// use default
			recordsWanted = DEFAULT_RECORDS_WANTED;
		}

		return commands.srbGetDatasetInfo(catType, objID, collectionName,
				recordsWanted);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbGenQuery
	 */
	SRBMetaDataRecordList[] srbGenQuery(final int catType,
			final String myMcatZone, final MetaDataCondition[] conditions,
			final MetaDataSelect[] selects, int recordsWanted,
			final boolean orderBy, final boolean nonDistinct)
			throws IOException {
		if (recordsWanted < 1) {
			// use default
			recordsWanted = DEFAULT_RECORDS_WANTED;
		}

		SRBMetaDataRecordList[] rl = commands.srbGenQuery(catType, myMcatZone,
				conditions, selects, recordsWanted, orderBy, nonDistinct);

		return rl;
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRegisterDataset
	 */
	void srbRegisterDataset(final int catType, final String objID,
			String dataTypeName, String resourceName, String collectionName,
			final String pathName, final long dataSize) throws IOException {
		if (dataTypeName == null) {
			dataTypeName = "generic";// DefMdasDataTypeName
		}

		if (resourceName == null) {
			resourceName = getDefaultStorageResource();// DefMdasResourceName
		}

		if (collectionName == null) {
			collectionName = getHomeDirectory();
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + collectionName;
		}

		commands.srbRegisterDataset(catType, objID, dataTypeName, resourceName,
				collectionName, pathName, dataSize);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbModifyDataset
	 */
	int srbModifyDataset(final int catType, final String objID,
			String collectionName, String resourceName, String pathName,
			final String dataValue1, final String dataValue2,
			final int actionType) throws IOException {
		if (collectionName == null) {
			collectionName = getHomeDirectory();// DefMdasCollectionName
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + GeneralFile.separator
					+ collectionName;
		}

		if ((pathName == null) && (resourceName == null)) {
			// Actually this should mean replica number is being used.
			/*
			 * MetaDataRecordList[] rl = null; MetaDataCondition[] conditions =
			 * { MetaDataSet.newCondition( SRBMetaDataSet.DIRECTORY_NAME,
			 * MetaDataCondition.EQUAL, collectionName ),
			 * MetaDataSet.newCondition( SRBMetaDataSet.FILE_NAME,
			 * MetaDataCondition.EQUAL, objID ) }; MetaDataSelect[] selects = {
			 * MetaDataSet.newSelection( SRBMetaDataSet.PATH_NAME ) };
			 * 
			 * try { rl = query( conditions, selects, 3 );
			 * 
			 * if( rl == null ) pathName = ""; else pathName =
			 * rl[0].getStringValue(0); } catch ( IOException e ) { pathName =
			 * ""; }
			 */
		} else if (resourceName == null) {
			resourceName = getDefaultStorageResource();// DefMdasResourceName
		} else if ((pathName == null) || pathName.equals("")) {
			MetaDataCondition[] conditions = {
					MetaDataSet.newCondition(StandardMetaData.FILE_NAME,
							MetaDataCondition.EQUAL, objID),
					MetaDataSet.newCondition(StandardMetaData.DIRECTORY_NAME,
							MetaDataCondition.EQUAL, collectionName) };
			MetaDataSelect[] selects = { MetaDataSet
					.newSelection(FileMetaData.PATH_NAME) };
			MetaDataRecordList rl[] = query(conditions, selects, 3);
			if (rl != null) {
				pathName = rl[0].getValue(FileMetaData.PATH_NAME).toString();
			}
		}

		return commands.srbModifyDataset(catType, objID, collectionName,
				resourceName, pathName, dataValue1, dataValue2, actionType);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbChkMdasAuth
	 */
	void srbChkMdasAuth(final String userName, final String srbAuth,
			String mdasDomain) throws IOException {
		if ((userName == null) || (srbAuth == null)) {
			throw new NullPointerException(
					"Null value entered for Mdas authorization");
		}

		if (mdasDomain == null) {
			mdasDomain = getDomainName();// default user mdas home domain
		}

		commands.srbChkMdasAuth(userName, srbAuth, mdasDomain);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbCreateCollect
	 */
	void srbCreateCollect(final int catType, String parentCollection,
			final String newCollection) throws IOException {
		if (parentCollection == null) {
			parentCollection = getHomeDirectory();// DefMdasCollectionName
		} else if (!parentCollection.startsWith(SRB_ROOT)) {
			parentCollection = getHomeDirectory() + GeneralFile.separator
					+ parentCollection;
		}

		commands.srbCreateCollect(catType, parentCollection, newCollection);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbListCollect
	 */
	SRBMetaDataRecordList[] srbListCollect(final int catType,
			String collectionName, final String flag, int recordsWanted)
			throws IOException {
		if (collectionName == null) {
			collectionName = getHomeDirectory();// DefMdasCollectionName
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + GeneralFile.separator
					+ collectionName;
		}

		if (recordsWanted < 1) {
			// use default
			recordsWanted = DEFAULT_RECORDS_WANTED;
		}

		return commands.srbListCollect(catType, collectionName, flag,
				recordsWanted);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbModifyCollect
	 */
	void srbModifyCollect(final int catType, String collectionName,
			final String dataValue1, final String dataValue2,
			final String dataValue3, final int actionType) throws IOException {
		if (collectionName == null) {
			collectionName = getHomeDirectory();// DefMdasCollectionName
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + GeneralFile.separator
					+ collectionName;
		}

		commands.srbModifyCollect(catType, collectionName, dataValue1,
				dataValue2, dataValue3, actionType);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbChkMdasSysAuth
	 */
	void srbChkMdasSysAuth(final String userName, final String srbAuth,
			final String mdasDomain) throws IOException {

		commands.srbChkMdasSysAuth(userName, srbAuth, mdasDomain);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRegisterUserGrp
	 */
	/**
	 * Register a user group
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param userGrpName
	 *            The name of the user group to register.
	 * @param userGrpPasswd
	 *            The user group passwd.
	 * @param userGrpType
	 *            The user group type. Currently, at SDSC valid userType are:
	 *            "staff", "sdsc staff", "sdsc staff scientist",
	 *            "sdsc senior staff scientist", "pto staff", "ucsd staff"
	 *            "student", "sdsc student", "uva student", "project",
	 *            "umd student", "public", "sysadmin", " deleted"
	 * @param userGrpAddress
	 *            The mailing address of the user group.
	 * @param userGrpPhone
	 *            The phone number of the user group.
	 * @param userGrpEmail
	 *            The Email address of the user group.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	public void srbRegisterUserGrp(final int catType, final String userGrpName,
			final String userGrpPasswd, final String userGrpType,
			final String userGrpAddress, final String userGrpPhone,
			final String userGrpEmail) throws IOException {

		commands.srbRegisterUserGrp(catType, userGrpName, userGrpPasswd,
				userGrpType, userGrpAddress, userGrpPhone, userGrpEmail);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRegisterUser
	 */
	/**
	 * Register a user.
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param userName
	 *            The name of the user to register.
	 * @param userDomain
	 *            The domain of the user to register.
	 * @param userPasswd
	 *            The user passwd.
	 * @param userType
	 *            The user type. Currently, at SDSC valid userType are: "staff",
	 *            "sdsc staff", "sdsc staff scientist",
	 *            "sdsc senior staff scientist", "pto staff", "ucsd staff"
	 *            "student", "sdsc student", "uva student", "project",
	 *            "umd student", "public", "sysadmin", " deleted"
	 * @param userAddress
	 *            The mailing address of the user.
	 * @param userPhone
	 *            The phone number of the user.
	 * @param userEmail
	 *            The Email address of the user.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	public void srbRegisterUser(final int catType, final String userName,
			final String userDomain, final String userPasswd,
			final String userType, final String userAddress,
			final String userPhone, final String userEmail) throws IOException {

		commands.srbRegisterUser(catType, userName, userDomain, userPasswd,
				userType, userAddress, userPhone, userEmail);
	}

	/*
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbModifyUser
	 */
	/**
	 * Modify a user info.
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param dataValue1
	 *            , String dataValue2 and int actionType - They are used to
	 *            specify the user attributes to modify. A normal user may use
	 *            it to modify his/her own password and a limited set of
	 *            attributes. A user with MDAS sys admin privilege can also use
	 *            these input values to modify other user's attributes.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	public void srbModifyUser(int catType, final String dataValue1,
			final String dataValue2, final int actionType) throws IOException {
		if (catType < 0) {
			catType = 0;
		}

		commands.srbModifyUser(catType, dataValue1, dataValue2, actionType);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbSetAuditTrail
	 */
	int srbSetAuditTrail(final int set_value) throws IOException {

		return commands.srbSetAuditTrail(set_value);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjAudit
	 */
	void srbObjAudit(final int catType, final String userName,
			final String objID, String collectionName, final String dataPath,
			String resourceName, final String accessMode, final String comment,
			final int success, final String domainName) throws IOException {

		if (resourceName == null) {
			resourceName = getDefaultStorageResource();// DefMdasResourceName
		}

		if (collectionName == null) {
			collectionName = getHomeDirectory();// DefMdasCollectionName
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + GeneralFile.separator
					+ collectionName;
		}

		commands.srbObjAudit(catType, userName, objID, collectionName,
				dataPath, resourceName, accessMode, comment, success,
				domainName);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRegisterReplica
	 */
	void srbRegisterReplica(final int catType, final String objID,
			String collectionName, String origResourceName,
			final String origPathName, final String newResourceName,
			final String newPathName, final String userName,
			final String domainName) throws IOException {
		if (origResourceName == null) {
			origResourceName = getDefaultStorageResource();// DefMdasResourceName
		}

		if (collectionName == null) {
			collectionName = getHomeDirectory();// DefMdasCollectionName
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + GeneralFile.separator
					+ collectionName;
		}

		commands.srbRegisterReplica(catType, objID, collectionName,
				origResourceName, origPathName, newResourceName, newPathName,
				userName, domainName);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbGetPrivUsers
	 */
	SRBMetaDataRecordList[] srbGetPrivUsers(final int catalog, int recordsWanted)
			throws IOException {

		if (recordsWanted < 1) {
			// use default
			recordsWanted = DEFAULT_RECORDS_WANTED;
		}

		return commands.srbGetPrivUsers(catalog, recordsWanted);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbGetMoreRows
	 */
	SRBMetaDataRecordList[] srbGetMoreRows(final int catalog,
			final int contDesc, int recordsWanted) throws IOException {

		if (recordsWanted < 1) {
			// use default
			recordsWanted = DEFAULT_RECORDS_WANTED;
		}

		return commands.srbGetMoreRows(catalog, contDesc, recordsWanted);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbIssueTicket
	 */
	void srbIssueTicket(final String objID, String collectionName,
			final String collectionFlag, final String beginTime,
			final String endTime, final int accessCnt, final String ticketUser)
			throws IOException {
		if (collectionName == null) {
			collectionName = getHomeDirectory();// DefMdasCollectionName
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + GeneralFile.separator
					+ collectionName;
		}

		commands.srbIssueTicket(objID, collectionName, collectionFlag,
				beginTime, endTime, accessCnt, ticketUser);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRemoveTicket
	 */
	void srbRemoveTicket(final String ticket) throws IOException {

		commands.srbRemoveTicket(ticket);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbUnregisterDataset
	 */
	void srbUnregisterDataset(final String objID, String collectionName)
			throws IOException {
		if (collectionName == null) {
			collectionName = getHomeDirectory();// DefMdasCollectionName
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + GeneralFile.separator
					+ collectionName;
		}

		commands.srbUnregisterDataset(objID, collectionName);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbContainerCreate
	 */
	void srbContainerCreate(final int catType, final String containerName,
			final String containerType, String resourceName,
			final long containerSize) throws IOException {
		if (resourceName == null) {
			resourceName = getDefaultStorageResource();// DefMdasResourceName
		}

		commands.srbContainerCreate(catType, containerName, containerType,
				resourceName, containerSize);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRegisterContainer
	 */
	void srbRegisterContainer(final int catType, final String containerName,
			String resourceName, final long containerSize) throws IOException {
		if (resourceName == null) {
			resourceName = getDefaultStorageResource();// DefMdasResourceName
		}

		commands.srbRegisterContainer(catType, containerName, resourceName,
				containerSize);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRegisterInContDataset
	 */
	void srbRegisterInContDataset(final int catType, final String objID,
			String collectionName, final String containerName,
			String dataTypeName, final long dataSize, final long baseOffset)
			throws IOException {
		if (dataTypeName == null) {
			dataTypeName = "generic";// DefMdasDataTypeName
		}

		if (collectionName == null) {
			collectionName = getHomeDirectory();// DefMdasCollectionName
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + GeneralFile.separator
					+ collectionName;
		}

		commands.srbRegisterInContDataset(catType, objID, collectionName,
				containerName, dataTypeName, dataSize, baseOffset);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbGetContainerInfo
	 */
	SRBMetaDataRecordList[] srbGetContainerInfo(final int catType,
			final String containerName, int recordsWanted) throws IOException {
		if (recordsWanted < 1) {
			// use default
			recordsWanted = DEFAULT_RECORDS_WANTED;
		}

		return commands.srbGetContainerInfo(catType, containerName,
				recordsWanted);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbGetResOnChoice
	 */
	String srbGetResOnChoice(final int catType, final String logResName,
			final String phyResName, final String inputFlag) throws IOException {

		return commands.srbGetResOnChoice(catType, logResName, phyResName,
				inputFlag);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRmContainer
	 */
	void srbRmContainer(final int catType, final String containerName,
			final boolean force) throws IOException {
		commands.srbRmContainer(catType, containerName, force);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbSyncContainer
	 */
	void srbSyncContainer(final int catType, final String containerName,
			final int syncFlag) throws IOException {
		commands.srbSyncContainer(catType, containerName, syncFlag);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbReplContainer
	 */
	void srbReplContainer(final int catType, final String containerName,
			String newResourceName) throws IOException {
		if (newResourceName == null) {
			newResourceName = getDefaultStorageResource();// DefMdasResourceName
		}

		commands.srbReplContainer(catType, containerName, newResourceName);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjOpenWithTicket
	 */
	int srbObjOpenWithTicket(final String objID, final int oflag,
			final String collectionName, final String ticket)
			throws IOException {

		// return commands.srbObjOpenWithTicket( objID, oflag, collectionName,
		// ticket );
		return 0;
	}

	// ------------------------------------------------------------------------
	// SRB 2.0 functions
	// ------------------------------------------------------------------------
	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRegInternalCompObj
	 */
	void srbRegInternalCompObj(final String objName, final String objCollName,
			final int objReplNum, final int objSegNum,
			final String intObjRescName, final String dataPathName,
			final long dataSize, final long offset, final int inpIntReplNum,
			final int intSegNum, final int objTypeInx, final String phyResLoc)
			throws IOException {
		commands.srbRegInternalCompObj(objName, objCollName, objReplNum,
				objSegNum, intObjRescName, dataPathName, dataSize, offset,
				inpIntReplNum, intSegNum, objTypeInx, phyResLoc);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRmIntCompObj
	 */
	void srbRmIntCompObj(final String objName, final String objCollName,
			final int objReplNum, final int objSegNum, final int inpIntReplNum,
			final int intSegNum) throws IOException {
		commands.srbRmIntCompObj(objName, objCollName, objReplNum, objSegNum,
				inpIntReplNum, intSegNum);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRmCompObj
	 */
	void srbRmCompObj(final String objName, final String objCollName,
			final int objReplNum, final int objSegNum) throws IOException {
		commands.srbRmCompObj(objName, objCollName, objReplNum, objSegNum);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbModInternalCompObj
	 */
	void srbModInternalCompObj(final String objID, final String collectionName,
			final int objReplNum, final int objSegNum, final int inpIntReplNum,
			final int intSegNum, final String data_value_1,
			final String data_value_2, final String data_value_3,
			final String data_value_4, final int retraction_type)
			throws IOException {
		commands.srbModInternalCompObj(objID, collectionName, objReplNum,
				objSegNum, inpIntReplNum, intSegNum, data_value_1,
				data_value_2, data_value_3, data_value_4, retraction_type);
	}

	/*
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbModifyRescInfo
	 */
	/**
	 * Modify/create/delete a SRB resource.
	 * 
	 * Input - int catType - catalog type. e,g., MDAS_CATALOG. String
	 * resourceName - The storage resource name. e.g. "mda18-unix-sdsc" int
	 * actionType - The type of retraction. See srbC_mdas_externs.h for the
	 * actionType definition. For tapes, valid values are T_INSERT_TAPE_INFO,
	 * T_UPDATE_TAPE_INFO, T_UPDATE_TAPE_INFO_2, T_DELETE_TAPE_INFO. String
	 * dataValue1 - Input value 1. String dataValue2 - Input value 2. String
	 * dataValue3 - Input value 3. String dataValue4 - Input value 4.
	 */
	public void srbModifyRescInfo(final int catType, final String resourceName,
			final int actionType, final String dataValue1,
			final String dataValue2, final String dataValue3,
			final String dataValue4) throws IOException {
		commands.srbModifyRescInfo(catType, resourceName, actionType,
				dataValue1, dataValue2, dataValue3, dataValue4);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRegisterLocation
	 */
	/**
	 * Register location information.
	 * 
	 * Input - String locName - The location name. String fullAddr - Full
	 * Address. String parentLoc - Parent location String serverUser - Server
	 * User String serverUserDomain - Server User Domain.
	 */
	public void srbRegisterLocation(final String locName,
			final String fullAddr, final String parentLoc,
			final String serverUser, final String serverUserDomain)
			throws IOException {
		commands.srbRegisterLocation(locName, fullAddr, parentLoc, serverUser,
				serverUserDomain);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbIngestToken
	 */
	/**
	 * Ingest Token.
	 * 
	 * Input - String typeName - The type name. String newValue - The new value.
	 * String parentValue - Parent value.
	 */
	public void srbIngestToken(final String typeName, final String newValue,
			final String parentValue) throws IOException {
		commands.srbIngestToken(typeName, newValue, parentValue);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRegisterResource
	 */
	/**
	 * Register Resource.
	 * 
	 * Input - String rescName - The resource name. String rescType - Resource
	 * type. String location - Location. String phyPath - Physical Path. String
	 * class - className. int size - size.
	 */
	public void srbRegisterResource(final String rescName,
			final String rescType, final String location, final String phyPath,
			final String className, final int size) throws IOException {
		commands.srbRegisterResource(rescName, rescType, location, phyPath,
				className, size);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRegisterLogicalResource
	 */
	/**
	 * Register Logical Resource.
	 * 
	 * Input - String rescName - The resource name. String rescType - Resource
	 * type. String phyResc - Physical resource. String phyPath - Physical path.
	 */
	public void srbRegisterLogicalResource(final String rescName,
			final String rescType, final String phyResc, final String phyPath)
			throws IOException {
		commands.srbRegisterLogicalResource(rescName, rescType, phyResc,
				phyPath);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbRegisterReplicateResourceInfo
	 */
	/**
	 * srbRegisterReplicateResourceInfo
	 * 
	 * Input - String physicalRescName - The physical resource name. String
	 * rescType - Resource type. String oldLogicalRescName - old logical
	 * resource name. String indefaultPath - Indefault Path.
	 */
	public void srbRegisterReplicateResourceInfo(final String physicalRescName,
			final String rescType, final String oldLogicalRescName,
			final String inDefaultPath) throws IOException {
		commands.srbRegisterReplicateResourceInfo(physicalRescName, rescType,
				oldLogicalRescName, inDefaultPath);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbDeleteValue
	 */
	/**
	 * Delete a Value; a single MCAT entry.
	 * 
	 * @param valueType
	 *            - the value (token) type.
	 * @param deleteValue
	 *            - The value (name) that is being deleted.
	 */
	public void srbDeleteValue(final int valueType, final String deleteValue)
			throws IOException {
		commands.srbDeleteValue(valueType, deleteValue);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbExecCommand
	 */
	InputStream srbExecCommand(final String command, final String commandArgs,
			final String hostAddress, int portalFlag) throws IOException {
		if (command == null) {
			throw new NullPointerException("No command given");
		}

		if ((portalFlag < 0) || (portalFlag > 2)) {
			portalFlag = PORTAL_STD_IN_OUT;
		}

		return commands.srbExecCommand(command, commandArgs, hostAddress,
				portalFlag, MIN_PORT, MAX_PORT);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name. Only valid for version 3.x.x and above. Method
	 * will not work for version 2.x.x, or before.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbExecCommand
	 */
	InputStream srbExecCommand(final String command, final String commandArgs,
			String hostAddress, final String fileName, int portalFlag)
			throws IOException {
		if (command == null) {
			throw new NullPointerException("No command given");
		}

		// Can not use hostAddress and fileName at same time
		if ((fileName != null) && (hostAddress != null)) {
			hostAddress = null;
		}

		if ((portalFlag < 0) || (portalFlag > 2)) {
			portalFlag = PORTAL_STD_IN_OUT;
		}

		return commands.srbExecCommand(command, commandArgs, hostAddress,
				fileName, portalFlag, MIN_PORT, MAX_PORT);
	}

	/**
	 * Sync the permanant replica with the temporary replica.
	 * 
	 * @param catType
	 *            catalog type - 0 - MCAT
	 * @param objID
	 *            The SRB object ID to Sync.
	 * @param collectionName
	 *            The name of the collection this objID belongs.
	 * @param resource
	 *            The resource for the object to sync to. A null or empty string
	 *            means synchronize existing copies.
	 */
	void srbSyncData(final SRBFile file) throws IOException {
		if (file == null) {
			throw new NullPointerException("No file given");
		}

		commands.srbSyncData(SRBFile.catalogType, file.getName(),
				file.getParent(), file.getResource());
	}

	/**
	 * Open a container for ingestion.
	 * 
	 * @param catType
	 *            - catalog type - 0 - MCAT
	 * @param containerName
	 *            - The name of the container.
	 * @param openFlag
	 *            - O_RDWR, O_WRONLY or O_RDONLY.
	 * @return The file descriptor for the container.
	 */
	int srbContainerOpen(final int catType, final String containerName,
			final int openFlag) throws IOException {
		if (containerName == null) {
			throw new NullPointerException("No container given");
		}

		return commands.srbContainerOpen(catType, containerName, openFlag);
	}

	/**
	 * Close an opened a container. *
	 * 
	 * @param catType
	 *            - catalog type - 0 - MCAT
	 * @param confFd
	 *            - The fd returned from srbContainerOpen ().
	 */
	void srbContainerClose(final int confFd) throws IOException {
		commands.srbContainerClose(confFd);
	}

	/**
	 * Copy a dataset.
	 * 
	 * @param srcObjID
	 *            - The source object.
	 * @param srcCollection
	 *            - The source collection.
	 * @param destObjID
	 *            - The destination object.
	 * @param destCollection
	 *            - The destination collection.
	 * @param destResource
	 *            - The resource to put the destination object.
	 * @return the number of bytes copied. Returns a negative value upon
	 *         failure.
	 */
	long srbObjCopy(final String srcObjID, final String srcCollection,
			final String destObjID, final String destCollection,
			final String destResource) throws IOException {
		if (srcObjID == null) {
			throw new NullPointerException("No container given");
		}

		return commands.srbObjCopy(srcObjID, srcCollection, destObjID,
				destCollection, destResource);
	}

	/**
	 * The client initiated version of srbObjPut. Copy a dataset from local
	 * space to SRB space.
	 * 
	 * @param destObjID
	 *            The destination objID.
	 * @param destCollection
	 *            The destination collwction.
	 * @param destResLoc
	 *            The destination resource.
	 * @param dataType
	 *            The data type.
	 * @param destPath
	 *            The destination path name.
	 * @param locFilePath
	 *            The local fullPath name.
	 * @param size
	 *            The size of the file. negative means don't know.
	 * @param forceFlag
	 *            over write flag
	 * @param numThreads
	 *            number of threads
	 * @return The number of bytes copied. Returns a negative value upon
	 *         failure.
	 */
	long srbObjPutClientInitiated(final String destObjID,
			final String destCollection, String destResLoc, String dataType,
			String destPath, final String localFilePath, final long srcSize,
			final int forceFlag, final int numThreads) throws IOException {
		if (destResLoc == null) {
			destResLoc = getDefaultStorageResource();// DefMdasResourceName
		}
		if (dataType == null) {
			dataType = "generic";// DefMdasDataTypeName
		}
		if (destPath == null) {
			destPath = "";
		}

		return commands.srbObjPutClientInitiated(destObjID, destCollection,
				destResLoc, dataType, destPath, localFilePath, srcSize,
				forceFlag, numThreads);
	}

	/**
	 * The client initiated version of srbObjGet. Copy a dataset from SRB space
	 * to local space.
	 * 
	 * @param srcObjID
	 *            The source objID.
	 * @param srcCollection
	 *            The source collwction.
	 * @param localFilePath
	 *            The local fullPath name.
	 * @param flag
	 *            not used currently
	 * @param numThreads
	 *            number of threads
	 * @return The number of bytes copied. Returns a negative value upon
	 *         failure.
	 */
	synchronized long srbObjGetClientInitiated(final String srcObjID,
			final String srcCollection, final GeneralFile file, final int flag,
			int numThreads, final boolean forceOverwrite) throws IOException {
		if ((srcObjID == null) || (srcCollection == null) || (file == null)) {
			throw new NullPointerException();
		}
		if (numThreads < 1) {
			numThreads = 1;
		}

		return commands.srbObjGetClientInitiated(srcObjID, srcCollection, file,
				flag, numThreads, forceOverwrite);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbBulkRegister
	 */
	void srbBulkRegister(final int catType, final String bulkLoadFilePath,
			final SRBMetaDataRecordList[] rl) throws IOException {
		if (bulkLoadFilePath == null) {
			throw new NullPointerException();
		}
		if (rl == null) {
			throw new NullPointerException();
		}

		commands.srbBulkRegister(catType, bulkLoadFilePath, rl);
	}

	// SRB 3.0 functions

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbGetMcatZone
	 */
	void srbGetMcatZone(final String userName, final String domainName,
			final String mcatName) throws IOException {
		commands.srbGetMcatZone(userName, domainName, mcatName);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbSetupSessionPublicKey
	 */
	String srbSetupSessionPublicKey() throws IOException {
		return commands.srbSetupSessionPublicKey();
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbSetupSession
	 */
	void srbSetupSession(final String sessionKey) throws IOException {
		commands.srbSetupSession(sessionKey);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbBulkLoad
	 */
	void srbBulkLoad(final int catType, final String bulkLoadFilePath,
			final SRBMetaDataRecordList[] rl) throws IOException {
		if (bulkLoadFilePath == null) {
			throw new NullPointerException();
		}
		if (rl == null) {
			throw new NullPointerException();
		}
		if (rl.length == 0) {
			return;
		}

		commands.srbBulkLoad(catType, bulkLoadFilePath, rl);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbBulkLoad
	 */
	void srbBulkUnload(final int catType, final int flag,
			final String srbUnloadDirPath, final String localDirPath)
			throws IOException {
		if (srbUnloadDirPath == null) {
			throw new NullPointerException();
		}
		if (localDirPath == null) {
			throw new NullPointerException();
		}

		commands.srbBulkUnload(catType, flag, srbUnloadDirPath, localDirPath);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbModifyZone
	 */
	/**
	 * Modify and Insert SRB zone and zone information in the Metadata Catalog.
	 * Information about the operation performed is also logged in the audit
	 * trail (if turned on). This is a privileged function and should be called
	 * only by a srbAdmin user.
	 * 
	 * @param catType
	 *            - catalog type. e,g., MDAS_CATALOG.
	 * @param zone_name
	 *            - name of the zone
	 * @param dataValue1
	 *            - Input value 1.
	 * @param dataValue2
	 *            - Input value 2.
	 * @param dataValue3
	 *            - Input value 3.
	 * @param dataValue4
	 *            - Input value 4.
	 * @param dataValue5
	 *            - Input value 5.
	 * @param actionType
	 *            - The type of action. performed values supported are:<br>
	 *            INSERT_NEW_LOCAL_ZONE<br>
	 *            dv1 = locn_desc<br>
	 *            dv2 = port_number<br>
	 *            dv3 = username@domain of remote MCAT admin<br>
	 *            dv4 = zone_contact<br>
	 *            dv5 = zone_comment<br>
	 *            INSERT_NEW_ALIEN_ZONE<br>
	 *            dv1-5 = same as for INSERT_NEW_LOCAL_ZONE<br>
	 *            MODIFY_ZONE_INFO<br>
	 *            dv1-5 = same as for INSERT_NEW_LOCAL_ZONE<br>
	 *            empty string implies no change.<br>
	 *            MODIFY_ZONE_FOR_USER<br>
	 *            dv1 = user_name<br>
	 *            dv2 = domain_name<br>
	 *            CHANGE_ZONE_NAME<br>
	 *            dv1 = new name<br>
	 *            MODIFY_ZONE_LOCAL_FLAG<br>
	 *            dv1 = new value (integer)<br>
	 *            MODIFY_ZONE_STATUS<br>
	 *            dv1 = new value (integer)<br>
	 */
	public void srbModifyZone(final int catType, final String zoneName,
			final String dataValue1, final String dataValue2,
			final String dataValue3, final String dataValue4,
			final String dataValue5, final int actionType) throws IOException {
		commands.srbModifyZone(catType, zoneName, dataValue1, dataValue2,
				dataValue3, dataValue4, dataValue5, actionType);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbBulkQueryAnswer
	 */
	MetaDataRecordList[] srbBulkQueryAnswer(final int catType,
			final String queryInfo, final MetaDataRecordList myresult,
			final int rowsWanted) throws IOException {
		return commands.srbBulkQueryAnswer(catType, queryInfo, myresult,
				rowsWanted);
	}

	/**
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbBulkMcatIngest
	 */
	void srbBulkMcatIngest(final int catType, final String ingestInfo,
			final SRBMetaDataRecordList[] rl) throws IOException {
		commands.srbBulkMcatIngest(catType, ingestInfo, rl);
	}

	// SRB3.0.2
	/*
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbBackupData /** Backup a data
	 * object - Make a replica to the backup resource. Skip it if a good copy
	 * already exist.
	 * 
	 * @param catType - catalog type - 0 - MCAT
	 * 
	 * @param objID The SRB object ID to Sync.
	 * 
	 * @param collectionName The name of the collection this objID belongs.
	 * 
	 * @param backupResource - The backup resource
	 * 
	 * @param flag - not used.
	 */
	synchronized void srbBackupData(final int catType, final String objID,
			String collectionName, String backupResource, final int flag)
			throws IOException {
		if (collectionName == null) {
			collectionName = getHomeDirectory();
		} else if (!collectionName.startsWith(SRB_ROOT)) {
			collectionName = getHomeDirectory() + GeneralFile.separator
					+ collectionName;
		}

		if (backupResource == null) {
			backupResource = getDefaultStorageResource();// DefMdasResourceName
		}

		commands.srbBackupData(catType, objID, collectionName, backupResource,
				flag);
	}

	// SRB3.1
	/*
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjChksum
	 */
	/**
	 * Checksum a SRB data file. By default, if the chksum already already
	 * exists, do nothing and return the chksum value. If the chksum does not
	 * exist, compute and register it.
	 * 
	 * @param objID
	 *            The data name.
	 * @param collectionName
	 *            The collection name.
	 * @param chksumFlag
	 *            valid flags are :<br>
	 *            l_FLAG - list the registered chksum value.<br>
	 *            c_FLAG - compute chksum, but don't register<br>
	 *            f_FLAG - force compute and register of chksum even if one
	 *            already exist.
	 * @param inpChksum
	 *            Not used.
	 * 
	 * @return the checksum value
	 */
	byte[] srbObjChksum(final String objID, final String collectionName,
			final int chksumFlag, final String inpChksum) throws IOException {
		return commands.srbObjChksum(objID, collectionName, chksumFlag,
				inpChksum);
	}

	/*
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbModifyUserNonPriv
	 */
	/**
	 * Modify and Insert SRB user information in the Metadata Catalog.
	 * Information about the operation performed is also logged in the audit
	 * trail (if turned on). This is a non-privileged function.
	 * 
	 * @param catType
	 *            catalog type. e,g., MDAS_CATALOG.
	 * @param userNameDomain
	 *            name@domain of the user
	 * @param dataValue1
	 *            Input value 1.
	 * @param dataValue2
	 *            Input value 2.
	 * @param dataValue3
	 *            Input value 3.
	 * @param dataValue4
	 *            Input value 4.
	 * @param dataValue5
	 *            Input value 5.
	 * @param actionType
	 *            The type of action. performed values supported are:
	 */
	void srbModifyUserNonPriv(final int catType, final String userNameDomain,
			final String dataValue1, final String dataValue2,
			final String dataValue3, final String dataValue4,
			final String dataValue5, final int actionType) throws IOException {
		commands.srbModifyUserNonPriv(catType, userNameDomain, dataValue1,
				dataValue2, dataValue3, dataValue4, dataValue5, actionType);
	}

	/*
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbModifyResource
	 */
	/**
	 * Modify and Insert SRB resource information in the Metadata Catalog.
	 * Information about the operation performed is also logged in the audit
	 * trail (if turned on). This is a privileged function and should be called
	 * only by a srbAdmin user.
	 * 
	 * @param catType
	 *            catalog type. e,g., MDAS_CATALOG.
	 * @param resource_name
	 *            name of the resource
	 * @param dataValue1
	 *            Input value 1.
	 * @param dataValue2
	 *            Input value 2.
	 * @param dataValue3
	 *            Input value 3.
	 * @param dataValue4
	 *            Input value 4.
	 * @param actionType
	 *            The type of action. performed values supported are:
	 */
	void srbModifyResource(final int catType, String resourceName,
			final String dataValue1, final String dataValue2,
			final String dataValue3, final String dataValue4,
			final int actionType) throws IOException {
		if (resourceName == null) {
			resourceName = getDefaultStorageResource();// DefMdasResourceName
		}

		commands.srbModifyResource(catType, resourceName, dataValue1,
				dataValue2, dataValue3, dataValue4, actionType);
	}

	/*
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbGetUserByDn
	 */
	/**
	 * Get the zone and user@domain given the user's dn
	 * 
	 * @param catType
	 *            catalog type. e,g., MDAS_CATALOG.
	 * @param userDn
	 *            The Dn of the user
	 * @return output for zone:user@domain
	 */
	public SRBAccount srbGetUserByDn(final int catType, final String userDn)
			throws IOException {
		if (userDn == null) {
			return null;
		}

		String userInfo = commands.srbGetUserByDn(catType, 0, userDn);
		if (userInfo == null) {
			return null;
		}

		String zone = "", userName = "", domainName = "";

		int index = 0, index2 = userInfo.indexOf(":");
		if (index < 0) {
			zone = "";
		}
		zone = userInfo.substring(index, index2);

		index = index2;
		index2 = userInfo.indexOf("@");
		if (index < 0) {
			return null;
		}
		userName = userInfo.substring(index + 1, index2);

		domainName = userInfo.substring(index2 + 1, userInfo.length() - 1);

		SRBAccount dnAccount = new SRBAccount(srbAccount.getHost(),
				srbAccount.getPort(), userName, null,
				GeneralFile.PATH_SEPARATOR + zone + GeneralFile.PATH_SEPARATOR
						+ SRBFile.LOCAL_HOME_DIRECTORY
						+ GeneralFile.PATH_SEPARATOR + userName + "."
						+ domainName, domainName,
				srbAccount.getDefaultStorageResource());

		return dnAccount;
	}

	/*
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjProc
	 */
	/**
	 * Execute a remote procedure on srbObject
	 * 
	 * @param fd
	 *            The object descriptor (from the srbObjOpen call) to read.
	 * @param procedureName
	 *            - name of Remote Procedure.
	 * @param input
	 *            The input string.
	 * @param outputLength
	 *            The size of pre-allocated outBuf.
	 * @return The result of the remote procedure
	 */
	String srbObjProc(final int fd, final String procedureName,
			final String input, final int outputLength) throws IOException {
		byte[] result = commands.srbObjProc(fd, procedureName, input,
				outputLength);

		if (result != null) {
			return new String(result);
		}
		return null;
	}

	/*
	 * Ensures all variables have reason values, then calls the SRBCommand
	 * function of the same name.
	 * 
	 * @see edu.sdsc.grid.io.srb.SRBCommands#srbObjProc
	 */
	/**
	 * Modify an extensible metadata table in MCAT.
	 * 
	 * @param catType
	 *            - catalog type. e,g., MDAS_CATALOG.
	 * 
	 * @param dataValue1
	 *            ... char *dataValue5 They are used to specify the table and
	 *            rows attributes to modify/add/delete.
	 * @param retractionType
	 *            They are used to specify the table and rows attributes to
	 *            modify/add/delete.
	 */
	void srbModifyExtMetaData(final int catType, final String dataName,
			final String collName, final String dataValue1,
			final String dataValue2, final String dataValue3,
			final String dataValue4, final String dataValue5,
			final int retractionType) throws IOException {
		commands.srbModifyExtMetaData(catType, dataName, collName, dataValue1,
				dataValue2, dataValue3, dataValue4, dataValue5, retractionType);
	}
}
