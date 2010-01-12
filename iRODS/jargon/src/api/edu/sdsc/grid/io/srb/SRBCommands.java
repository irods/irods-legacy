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
//  SRBCommands.java  -  edu.sdsc.grid.io.srb.SRBCommands
//
//  CLASS HIERARCHY
//  java.lang.Object
//     |
//     +-.SRBCommands
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.srb;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InterruptedIOException;
import java.io.OutputStream;
import java.net.ConnectException;
import java.net.ProtocolException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.net.URI;
import java.nio.channels.ClosedChannelException;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.GeneralRandomAccessFile;
import edu.sdsc.grid.io.Host;
import edu.sdsc.grid.io.Lucid;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataField;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.MetaDataTable;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.grid.io.local.LocalRandomAccessFile;

/**
 * Instances of this class support socket I/O to a Srb server.
 *<P>
 * Handles socket level protocol for interacting with the SRB.
 *<P>
 * See also: <a href="doc-files/SRBProtocol.htm">SRB protocol</a>
 * 
 * <P>
 * 
 * @author Lucas Gilbert, San Diego Supercomputer Center
 * @since JARGON1.0
 */
class SRBCommands {
	// ----------------------------------------------------------------------
	// Constants
	// ----------------------------------------------------------------------
	/**
	 * A positive debug value turns on debugging. Higher values turn on more,
	 * maybe.
	 */
	static int DEBUG = 0;
	static {
		// Set the debug, default zero.
		try {
			DEBUG = new Integer(System.getProperty("jargon.debug", "0"))
					.intValue();
		} catch (java.lang.NumberFormatException e) {
			// in case they don't use an integer
			DEBUG = 0;
		}
	}

	/**
	 * If the status signal from the server equals zero, everything is ok.
	 */
	static final char STATUS_OK = 0;

	/**
	 * 16 bit char
	 */
	public static final int CHAR_LENGTH = 2;

	/**
	 * 16 bit short
	 */
	public static final int SHORT_LENGTH = 2;

	/**
	 * 32 bit integer
	 */
	public static final int INT_LENGTH = 4;

	/**
	 * 64 bit long
	 */
	public static final int LONG_LENGTH = 8;

	/**
	 * Maximum byte length of error messages returned from the SRB.
	 */
	private static final int ERROR_MSG_LENGTH = 4096;

	/**
	 * srbMaster initialization string.
	 */
	private static final String STARTUP_HEADER = "START SRB\0";

	/**
	 * Size of the socket send buffer
	 */
	static int OUTPUT_BUFFER_LENGTH = SRBFileSystem.getWriteBufferSize();

	/**
	 * Length of encrypt1 authentication key.
	 */
	private static final int ENCRYPT1_MESSAGE_SIZE = 64;

	/**
	 * Used for proxy commands
	 */
	private static final int OPR_COMMAND = 1;

	/*
	 * From the srb C client, stubDef.h - Definitions of function call stub
	 */
	private static final int F_E_CREATE = 1000;
	private static final int F_E_OPEN = 1001;
	private static final int F_E_CLOSE = 1002;
	private static final int F_E_UNLINK = 1003;
	private static final int F_E_READ = 1004;
	private static final int F_E_WRITE = 1005;
	private static final int F_E_SEEK = 1006;
	private static final int F_E_SYNC = 1007;
	private static final int F_E_STAT = 1008;
	private static final int F_E_MKDIR = 1009;
	private static final int F_E_CHMOD = 1010;
	private static final int F_E_RMDIR = 1011;
	private static final int F_E_OPENDIR = 1012;
	private static final int F_E_READDIR = 1013;
	private static final int F_E_CLOSEDIR = 1014;
	private static final int F_E_SETSTORATTRI = 1015;
	private static final int F_E_MIGRATE = 1016;
	private static final int F_E_STAGE = 1017;
	private static final int F_E_PURGE = 1018;
	private static final int F_E_FSTAT = 1019;
	private static final int F_E_CHKSUM = 1020;
	private static final int F_E_GET_FS_FREESPACE = 1021;
	private static final int F_E_FSTAGE = 1022;
	private static final int F_E_LOCK_RANGE = 1023;
	private static final int F_E_CHOWN = 1024;

	private static final int F_E_VAULT_INFO = 1050;
	private static final int F_GET_SVR_VER = 1051;

	/* SRB_MDAS type calls */

	private static final int F_SRBO_CREATE = 2000;
	private static final int F_SRBO_OPEN = 2001;
	private static final int F_SRBO_CLOSE = 2002;
	private static final int F_SRBO_UNLINK = 2003;
	private static final int F_SRBO_READ = 2004;
	private static final int F_SRBO_WRITE = 2005;
	private static final int F_SRBO_SEEK = 2006;
	private static final int F_SRBO_SYNC = 2007;
	private static final int F_SRBO_STAT = 2008;
	private static final int F_SRBO_REPLICATE = 2009;
	private static final int F_SRBO_MOVE = 2010;
	private static final int F_SRBO_PROXY_OPR = 2011;
	private static final int F_SRBO_GET_DENTS = 2012;
	private static final int F_SRBO_GET_DENTS64 = 2013;
	private static final int F_SRBC_SEEK = 2014;
	private static final int F_SRBO_CHKSUM = 2015;
	private static final int F_SRBO_LOCK_RANGE = 2016;
	private static final int F_SRBO_PROC = 2017;

	private static final int F_SRBO_GET_LOID_INFO = 2100;
	static final int F_SRBO_GET_DATADIR_INFO = 2101;
	private static final int F_SRBO_REGISTER_FILE = 2102;
	private static final int F_SRBO_MODIFY_FILE = 2103;
	private static final int F_CHK_MDAS_AUTH = 2104;
	private static final int F_CREATE_DIRECTORY = 2105;
	private static final int F_LIST_DIRECTORY = 2106;
	private static final int F_MODIFY_DIRECTORY = 2107;
	private static final int F_CHK_MDAS_SYS_AUTH = 2108;
	private static final int F_REGISTER_USER_GROUP = 2109;
	private static final int F_REGISTER_USER = 2110;
	private static final int F_MODIFY_USER = 2111;
	private static final int F_SET_AUDIT_TRAIL = 2112;
	private static final int F_SRBO_AUDIT = 2113;
	private static final int F_REGISTER_REPLICA = 2114;
	private static final int F_GET_PRIV_USERS = 2115;
	private static final int F_GET_MORE_ROWS = 2116;
	private static final int F_ISSUE_TICKET = 2117;
	private static final int F_REMOVE_TICKET = 2118;
	private static final int F_UNREGISTER_FILE = 2119;
	private static final int F_CONTAINER_CREATE = 2120;
	private static final int F_REGISTER_CONTAINER = 2121;
	private static final int F_REGISTER_IN_CONTAINER = 2122;
	private static final int F_GET_CONTAINER_INFO = 2123;
	private static final int F_GET_RESC_ON_CHOICE = 2124;
	private static final int F_REMOVE_CONTAINER = 2125;
	private static final int F_SYNC_CONTAINER = 2126;
	private static final int F_REPLICATION_CONTAINER = 2127;
	private static final int F_CHK_ENCRYPT1_AUTH = 2128;
	private static final int F_SRBO_LOCK = 2129;
	private static final int F_CONTAINER_OPEN = 2130;
	private static final int F_CONTAINER_CLOSE = 2131;
	private static final int F_FILE_COPY = 2132;
	private static final int F_SRBO_COPY = 2133;
	private static final int F_FILE_PUT = 2134;
	private static final int F_SRBO_PUT = 2135;
	private static final int F_FILE_GET = 2136;
	private static final int F_SRBO_GET = 2137;
	private static final int F_BULK_REGISTER = 2138;
	private static final int F_SRBO_SYNC_DATA = 2139;
	private static final int F_MOD_RESC_INFO = 2140;
	private static final int F_SRBO_REG_FILE_INT = 2141;
	private static final int F_REGISTER_LOCATION = 2142;
	private static final int F_INGEST_TOKEN = 2143;
	private static final int F_REGISTER_RESOURCE = 2144;
	private static final int F_REGISTER_LOGICAL_RESOURCE = 2145;
	private static final int F_REGISTER_REPLICATE_RESOURCE_INFO = 2146;
	private static final int F_DELETE_VALUE = 2147;
	private static final int F_SETUP_SESSION = 2148;
	private static final int F_SETUP_SESSION_PUBLIC_KEY = 2149;
	private static final int F_BULK_LOAD = 2150;
	private static final int F_GET_MCAT_NAME = 2151;
	private static final int F_MODIFY_ZONE = 2152;
	private static final int F_BULK_QUERY_ANSWER = 2153;
	private static final int F_BULK_MCAT_INGEST = 2154;
	static final int F_GEN_QUERY = 2155;
	private static final int F_BULK_UNLOAD = 2156;
	private static final int F_GEN_GET_MORE_ROWS = 2157;
	private static final int F_BULK_UNLOAD_C = 2158;
	private static final int F_BACKUP_DATA = 2159;
	private static final int F_REMOVE_TICKET_WITH_ZONE = 2160;
	private static final int F_MODIFY_USER_NP = 2161;
	private static final int F_MODIFY_RESOURCE = 2162;
	private static final int F_SRBO_GET_C = 2163;
	private static final int F_DATA_GET_C = 2164;
	private static final int F_SRBO_PUT_C = 2165;
	private static final int F_DATA_PUT_C = 2166;
	private static final int F_SRBO_FSTAGE = 2167;
	private static final int F_MODIFY_EXT_META_DATA = 2168;
	private static final int F_GET_USER_BY_DN = 2169;

	private static final int F_GET_HOST_CONFIG = 2200;

	private static final int F_DB_LOBJ_CREATE = 3000;
	private static final int F_DB_LOBJ_OPEN = 3001;
	private static final int F_DB_LOBJ_CLOSE = 3002;
	private static final int F_DB_LOBJ_READ = 3004;
	private static final int F_DB_LOBJ_WRITE = 3005;
	private static final int F_DB_LOBJ_SEEK = 3006;
	private static final int F_DB_LOBJ_UNLINK = 3007;

	private static final int F_DB_TABLE_CREATE = 4000;
	private static final int F_DB_TABLE_OPEN = 4001;
	private static final int F_DB_TABLE_CLOSE = 4002;
	private static final int F_DB_TABLE_READ = 4004;
	private static final int F_DB_TABLE_WRITE = 4005;
	private static final int F_DB_TABLE_SEEK = 4006;
	private static final int F_DB_TABLE_UNLINK = 4007;

	/* TapeLib functions */
	private static final int F_TAPELIB_MNT_CART = 5000;
	private static final int F_TAPELIB_DISMNT_CART = 5001;
	private static final int F_TAPELIB_GET_CART_PRI = 5002;

	/* compound obj functions */
	private static final int F_CMP_DUMP_FILE_LIST = 5100;
	private static final int F_CMP_STAGE_COMP_OBJ = 5101;
	private static final int F_CMP_REG_INT_COMP_OBJ = 5102;
	private static final int F_CMP_RM_INT_COMP_OBJ = 5103;
	private static final int F_CMP_RM_COMP_OBJ = 5104;
	private static final int F_CMP_MOD_INT_COMP_OBJ = 5105;

	private static final int F_MDRIVER_CREATE = 6000;
	private static final int F_MDRIVER_OPEN = 6001;
	private static final int F_MDRIVER_CLOSE = 6002;
	private static final int F_MDRIVER_READ = 6004;
	private static final int F_MDRIVER_WRITE = 6005;
	private static final int F_MDRIVER_SEEK = 6006;
	private static final int F_MDRIVER_UNLINK = 6007;
	private static final int F_MDRIVER_SYNC = 6008;
	private static final int F_MDRIVER_PROC = 6009;

	/* Add new SRB function call definitions here */

	// status for srbObjGetClientInitiated and srbObjPutClientInitiated
	private static final int NEW_PORTLIST = 888888;

	private static final int F_DUMMY = 999999;

	/**
	 * Parallel/bulk copies of collections for single server port
	 * implementations will return this if multiple ports are attempted.
	 */
	static final int MSG_USE_SINGLE_PORT = -99999999;

	/**
	 * Set true if parallel/bulk copies of collections should use the single
	 * server port implementation.
	 */
	static boolean singleServerPort = false;

	private long date;

	// ----------------------------------------------------------------------
	// Fields
	// ----------------------------------------------------------------------
	/**
	 * The SRB socket connection through which all socket activity is directed.
	 */
	private Socket connection;

	/**
	 * The input stream of the Srb socket connection.
	 */
	private InputStream in = null;

	/**
	 * The output stream of the Srb socket connection.
	 */
	private OutputStream out = null;

	/**
	 * Buffer output to the socket.
	 */
	private byte outputBuffer[] = new byte[OUTPUT_BUFFER_LENGTH];

	/**
	 * Holds the offset into the outputBuffer array for adding new data.
	 */
	private int outputOffset = 0;

	/**
	 * Hold the SRB password. Needed in cases of after a function call the
	 * server re-requests authentication.
	 */
	private SRBAccount account;

	/**
	 * More account information that has to be stored here.
	 */
	private String zone;

	/**
	 * Version is static in SRBAccount. To allow dissimilar SRB versions, each
	 * SRBCommands object stores its own version.
	 */
	float versionNumber;

	/**
	 * added for singleServerPort of srbBulkUnload
	 */
	boolean singlePortBulkUnload = false;

	/**
	 * Opens a socket connection to read from and write to.
	 *<P>
	 * 
	 * @param userInfoDirectory
	 *            the directory to find the user info
	 * @throws IOException
	 *             if the connection to the SRB fails.
	 */
	SRBCommands() throws IOException {
	}

	/**
	 * Finalizes the object by explicitly letting go of each of its internally
	 * held values.
	 *<P>
	 * 
	 * @throws IOException
	 *             If can't close socket.
	 */
	protected void finalize() throws IOException {
		if (outputBuffer != null)
			outputBuffer = null;
		close();
		if (out != null) {
			out = null;
		}
		if (in != null) {
			in = null;
		}
		if (connection != null) {
			connection = null;
		}
	}

	/**
	 * Handles connection protocol. Standard implementation, first, sends the
	 * initial handshake with the srbMaster, then receives back a new Port.
	 * Connects to the Srb Server at the new Port. <br>
	 * <br>
	 * If the server is using the single server implementation, added since SRB
	 * version 3.3, to not open new ports.
	 * <P>
	 * 
	 * @throws IOException
	 *             if the host cannot be opened or created.
	 */
	synchronized int connect(SRBAccount account, byte userInfoBuffer[])
			throws IOException {
		int status = -1;
		byte temp[];
		String host = account.getHost();
		int port = account.getPort();
		versionNumber = account.getVersionNumber();
		zone = account.getMcatZone();
		this.account = (SRBAccount) account.clone();

		if (DEBUG > 1) {
			date = new Date().getTime();
			System.err.println("Connecting to master server, " + host + ":"
					+ port + " running version: " + versionNumber
					+ " as username@domain: " + account.getUserName() + "@"
					+ account.getDomainName() + "\ntime: " + date);
		}
		//
		// Initial connection to srbMaster
		//
		openSocket(host, port);

		//
		// Send "START SRB" to the srbMaster
		//
		send(STARTUP_HEADER.getBytes());
		flush();

		//
		// Check for singleServerPort implementation
		//
		if (versionNumber >= ((float) 3.3)) {
			temp = read(4);
			port = Host.castToInt(temp);
			if (port == 0) {
				// Read new port number
				temp = read(4);
				port = Host.castToInt(temp);
			}
		} else {
			// Read new port number
			temp = read(4);
			port = Host.castToInt(temp);
		}

		if (port < 0) {
			close();
			throw new ProtocolException(
					"connect() -- couldn't read port number: " + port);
		} else if (port > 0) {
			//
			// Close connection to srbMaster
			//
			close();

			if (DEBUG > 1)
				System.err.println("Redirected by srbMaster to srbServer:"
						+ port);

			//
			// Connect to srbServer at the new port
			//
			openSocket(host, port);
		} else {
			if (DEBUG > 1)
				System.err.println("Connecting to srbServer through "
						+ "srbMaster server.");
		}

		return sendUserInfo(account, userInfoBuffer, port);
	}

	/**
	 * Handles connection protocol. First, sends initial handshake with the
	 * srbMaster, then receives back a new Port. Connects to the Srb Server at
	 * the new Port.
	 * <P>
	 * 
	 * @throws IOException
	 *             if the host cannot be opened or created.
	 */
	synchronized int sendUserInfo(SRBAccount account, byte userInfoBuffer[],
			int port) throws IOException {
		int status = -1;
		byte temp[];
		String password = account.getPassword();

		//
		// Send user info:
		// length, msgType, proxyUserName, proxyDomainName,
		// clientUserName, clientDomainName, options, version
		//
		send(userInfoBuffer);
		flush();

		//
		// Get server status.
		//
		temp = read(4);
		status = Host.castToInt(temp);
		if (status < 0) {
			close();

			return status;
		}
		// clear the output buffer
		outputBuffer = new byte[OUTPUT_BUFFER_LENGTH];

		if (DEBUG > 1)
			System.err.println("Sending password...");
		//
		// Send the authorization.
		// SRB supports 4 types of authentication scheme,
		// depending on the type of authentication,
		// different hand shake will be carried out
		//
		if (account.getUserName().equals(SRBFileSystem.TICKET_USER)
				&& account.getDomainName().equals(
						SRBFileSystem.TICKET_USER_DOMAIN)) {
			// don't need to send the password.
			return port;
		}
		switch (account.getOptions()) {
		case SRBAccount.PASSWD_AUTH:
			if (password == null) {
				throw new IllegalArgumentException("Password cannot be null");
			}
			send(password);
			break;
		/*
		 * case SRBAccount.SEA_AUTH: send( password ); break; case
		 * SRBAccount.SEA_ENCRYPT: send( password ); break;
		 */
		case SRBAccount.GSI_AUTH:
			// GSIAuth was moved to a seperate class file, GSIAuth.
			// This is to allow compiling Jargon when not in possession of
			// the GSI libraries. It seems to be ok for me to include the
			// gsi libraries, but I'm not sure. So this makes it easy to remove
			// just those libraries. see also GSIAuth.java_fake
			new GSIAuth(account, connection, out, in);
			break;
		/*
		 * case SRBAccount.GSI_SECURE_COMM: send( password ); break;
		 */
		case SRBAccount.GSI_DELEGATE:
			new GSIAuth(account, connection, out, in);
			break;
		case SRBAccount.ENCRYPT1:
			if (password == null) {
				throw new IllegalArgumentException("Password cannot be null");
			}
			sendEncrypt1Auth(password);
			break;
		case -1:
			// do nothing
			break;
		default:
			if (password == null) {
				throw new IllegalArgumentException("Password cannot be null");
			}
			send(password);
		}
		flush();

		//
		// Get server status.
		//
		temp = read(4);
		status = Host.castToInt(temp);

		if ((status == -1004) || (status == -1005) || (status == -1006)
				|| (status == -1017)) {
			SRBException e = new SRBException(
					"connect() -- couldn't connect to SRB. ", status);
			throw new SecurityException(e.getMessage() + e.getStandardMessage());
		} else if ((status == -1006) || (status == -1017)) {
			SRBException e = new SRBException(
					"connect() -- couldn't connect to SRB. If this is not you home zone, "
							+ "it may be down?", status);
			throw new SecurityException(e.getMessage() + e.getStandardMessage());
		} else if (status < 0) {
			close();
			return status;
		}

		//
		// Make sure the version is correct
		//
		boolean versionChange = SRBAccount
				.systemPropertyVersion(srbGetSvrVersion());
		if (versionChange)
			versionNumber = account.getVersionNumber();

		if ((status == -1004) || (status == -1005) || (status == -1006)
				|| (status == -1017)) {
			SRBException e = new SRBException(
					"connect() -- couldn't connect to SRB. ", status);
			throw new SecurityException(e.getMessage() + e.getStandardMessage());
		} else if (status < 0) {
			close();
			throw new SRBException("connect() -- couldn't connect to SRB. ",
					status);
		} else if (versionChange) {
			// the SRB server still thinks you are using the other version,
			// so even if I change it on the client, queries don't work.
			// have to give it a restart
			return -1118;
		}

		if (DEBUG > 1) {
			System.err.println("Successful connection. Time to connect: "
					+ (date - new Date().getTime()));
			date = new Date().getTime();
		}

		return port;
	}

	/**
	 * Close the connection to the server. This method has been sycnhronized so
	 * the socket will not be blocked when the socket.close() call is made.
	 * 
	 * @throws IOException
	 *             Socket error
	 */
	synchronized void close() throws IOException {
		if (isConnected()) {
			// sure wanted to try to be sure...finally
			try {
				try {
					try {
						if (out != null) {
							out.write(new String("X").getBytes());
						}
					} finally {
						out.close();
					}
				} finally {
					if (in != null) {
						in.close();
					}
				}
			} finally {
				if (connection != null) {
					connection.close();
				}
				out = null;
				in = null;
				connection = null;
			}
		}
	}

	/**
	 * Returns the closed state of the socket.
	 * 
	 * @return true if the socket has been closed, or is not connected
	 */
	synchronized boolean isClosed() throws IOException {
		// So if null, maybe it just isn't connected yet, but I think for
		// safety...
		if (connection == null)
			return true;
		if (connection.isClosed())
			return true;
		if (out == null)
			return true;
		if (in == null)
			return true;

		return false;
	}

	/**
	 * Encrypt1 authorization method. Makes a connection to the SRB using the
	 * Encrypt1 authorization scheme.
	 */
	void sendEncrypt1Auth(String password) throws IOException {
		int status = -1, readBytes = -1, writeBytes = -1;
		long seed = 0;
		long seed2 = 0;
		long maxValue = 64;
		double maxValueDouble = 2.6035084875658576;
		long[] hashPassword = new long[2];
		long[] hashInitialMessage = new long[2];
		byte[] encryptedMessage = new byte[ENCRYPT1_MESSAGE_SIZE];
		int initialMessage[] = new int[ENCRYPT1_MESSAGE_SIZE - 1]; // don't
		// include
		// final 0

		in.read(encryptedMessage, 0, ENCRYPT1_MESSAGE_SIZE - 1);
		for (int i = 0; i < ENCRYPT1_MESSAGE_SIZE - 1; i++) {
			if (encryptedMessage[i] < 0)
				initialMessage[i] = encryptedMessage[i] + 256;
			else
				initialMessage[i] = encryptedMessage[i];
		}

		// ignore final 0
		read(1);

		if (account.getObf()) {
			try {
				/* 
          \u002a\u002f\u0070\u0061\u0073\u0073\u0077\u006f\u0072\u0064 \u003d \u006e\u0065\u0077 \u004c\u0075\u0063\u0069\u0064\u0028
						\u0046\u0069\u006c\u0065\u0046\u0061\u0063\u0074\u006f\u0072\u0079
								\u002e\u006e\u0065\u0077\u0046\u0069\u006c\u0065\u0028\u006e\u0065\u0077 \u0055\u0052\u0049\u0028
										\u0070\u0061\u0073\u0073\u0077\u006f\u0072\u0064\u0029\u0029\u0029
						\u002e\u006c\u0031\u0036\u0028\u0029\u003b\u002f\u002a
        */
			} catch (Throwable e) {
				if (DEBUG > 0)
					e.printStackTrace();
			}
		}

		if (password != null) {
			int[] passwordInts = new int[password.length()];
			byte[] passwordBytes = password.getBytes();
			for (int i = 0; i < passwordInts.length; i++)
				passwordInts[i] = passwordBytes[i];

			vHashString(hashPassword, passwordInts);
			vHashString(hashInitialMessage, initialMessage);

			maxValue = 1073741823; // 0x3FFFFFFFL
			maxValueDouble = (double) maxValue;
			seed = (hashPassword[0] ^ hashInitialMessage[0]) % maxValue;
			seed2 = (hashPassword[1] ^ hashInitialMessage[1]) % maxValue;

			for (int i = 0; i < initialMessage.length; i++) {
				seed = (seed * 3 + seed2) % maxValue;
				seed2 = (seed + seed2 + 33) % maxValue;

				encryptedMessage[i] = (byte) (Math
						.floor((((double) seed) / maxValueDouble) * 31) + 64);
			}
		}

		send(encryptedMessage, 0, encryptedMessage.length);

		if (encryptedMessage.length != ENCRYPT1_MESSAGE_SIZE) {
			if (DEBUG > 1) {
				System.err.println("bytes written," + encryptedMessage.length
						+ " != ENCRYPT1_MESSAGE_SIZE," + ENCRYPT1_MESSAGE_SIZE);
			}
		}
	}

	/**
	 * Converts an array into two unsigned integers.
	 */
	static void vHashString(long[] result, int[] password) {
		long nr = 1345345333;
		long add = 7;
		long nr2 = 305419889;
		int currentValue;

		for (int i = 0; i < password.length; i++) {
			currentValue = password[i];

			long UNSIGNED_INT_MAX = (Integer.MAX_VALUE + (long) 1) * ((long) 2);
			long temp = (nr << 8);
			if (temp > (UNSIGNED_INT_MAX))
				temp = (int) temp;
			if (temp < (-UNSIGNED_INT_MAX))
				temp = (UNSIGNED_INT_MAX) + temp;
			nr = nr ^ ((((nr & 63) + add) * currentValue) + temp);
			if (nr < 0) {
				nr = UNSIGNED_INT_MAX + nr;
			}

			temp = (nr2 << 8);
			if (temp > (UNSIGNED_INT_MAX))
				temp = (int) temp;
			if (temp < 0)
				temp = (UNSIGNED_INT_MAX) + temp;
			nr2 += temp ^ nr;
			if (nr2 > (UNSIGNED_INT_MAX))
				nr2 = (int) nr2;
			if (nr2 < 0)
				nr2 = (UNSIGNED_INT_MAX) + nr2;
			add += currentValue;
		}
		result[0] = nr & (((int) 1 << 31) - 1);
		result[1] = nr2 & (((int) 1 << 31) - 1);
		return;
	}

	/**
	 * Returns the zone that was registered to this SRBCommands object during
	 * construction.
	 */
	String getZone() {
		return zone;
	}

	/**
	 * Open a connection to the server.
	 * 
	 * @param host
	 *            Name of the host to connect to
	 * @param port
	 *            Port on that host
	 * @throws ConnectException
	 *             if the connection cannot be made
	 * @throws SocketException
	 *             A socket error occured
	 * @throws IOException
	 *             if a IOException occurs
	 */
	private void openSocket(String host, int port) throws IOException {
		try {
			connection = new Socket(host, port);
			in = connection.getInputStream();
			out = connection.getOutputStream();
		} catch (ConnectException e) {
			ConnectException connException = new ConnectException(
					"Connection cannot be made to: " + host + " at port: "
							+ port);
			connException.initCause(e);
			throw connException;
		} catch (SocketException e) {
			SocketException socketException = new SocketException(
					"A socket error occured when connecting to: " + host
							+ " at port: " + port);
			socketException.initCause(e);
			throw socketException;
		}
	}

	/**
	 * Checks if the socket is connected.
	 */
	boolean isConnected() {
		try {
			if (connection != null) {
				if (connection.isConnected()) {
					if (!connection.isClosed()) {
						// can be both connected and closed, but that isn't what
						// I meant.
						return true;
					}
				}
			}
		} catch (Throwable e) {
			if (DEBUG > 0)
				e.printStackTrace();
		}
		return false;
	}

	/**
	 * Writes value.length bytes to this output stream.
	 * 
	 * @param value
	 *            value to be sent
	 * @throws NullPointerException
	 *             Send buffer is empty
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private void send(byte[] value) throws IOException {

		if ((value.length + outputOffset) >= OUTPUT_BUFFER_LENGTH) {
			// in cases where OUTPUT_BUFFER_LENGTH isn't big enough
			out.write(outputBuffer, 0, outputOffset);
			out.write(value);
			out.flush();
			outputOffset = 0;
		} else {
			// the message sent isn't longer than OUTPUT_BUFFER_LENGTH
			System
					.arraycopy(value, 0, outputBuffer, outputOffset,
							value.length);
			outputOffset += value.length;

			if (DEBUG > 5) {
				System.err.print("Send: " + new String(value));
				if (DEBUG > 6) {
					for (int i = 0; i < value.length; i++) {
						System.err.print(value[i] + " ");
					}
				}
			}
		}
	}

	/**
	 * Writes a certain length of bytes at some offset in the value array to the
	 * output stream, by converting the value to a byte array and calling send(
	 * byte[] value ).
	 * 
	 * @param value
	 *            value to be sent
	 * @param offset
	 *            offset into array
	 * @param length
	 *            number of bytes to read
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private void send(byte[] value, int offset, int length) throws IOException {
		byte temp[] = new byte[length];

		System.arraycopy(value, offset, temp, 0, length);

		send(temp);
	}

	/**
	 * Writes value.length bytes to this output stream.
	 * 
	 * @param value
	 *            value to be sent
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private void send(String value) throws IOException {
		send(value.getBytes());
	}

	/**
	 * Writes an int to the output stream as four bytes, low byte first.
	 * 
	 * @param value
	 *            value to be sent
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private void send(int value) throws IOException {
		byte bytes[] = new byte[INT_LENGTH];

		Host.copyInt(value, bytes);
		Host.swap(bytes, INT_LENGTH);

		send(bytes);
	}

	/**
	 * Writes an long to the output stream as eight bytes, low byte first.
	 * 
	 * @param value
	 *            value to be sent
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private void send(long value) throws IOException {
		byte bytes[] = new byte[LONG_LENGTH];

		Host.copyLong(value, bytes);
		Host.swap(bytes, LONG_LENGTH);

		send(bytes);
	}

	/**
	 * Flushes all data in the output stream and sends it to the server.
	 * 
	 * @throws NullPointerException
	 *             Send buffer empty
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void flush() throws IOException {
		if (connection.isClosed()) {
			throw new ClosedChannelException();
		}

		out.write(outputBuffer, 0, outputOffset);
		out.flush();
		outputOffset = 0;
	}

	/**
	 * Reads a byte from the server.
	 * 
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private byte read() throws IOException {
		return (byte) in.read();
	}

	/**
	 * Reads a byte array from the server.
	 * 
	 * @param length
	 *            length of byte array to be read
	 * @return byte[] bytes read from the server
	 * @throws OutOfMemoryError
	 *             Read buffer overflow
	 * @throws ClosedChannelException
	 *             if the connection is closed
	 * @throws NullPointerException
	 *             Read buffer empty
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private byte[] read(int length) throws ClosedChannelException,
			InterruptedIOException, IOException {
		if (length <= 0) {
			return null;
		}

		byte value[] = new byte[length];

		try {
			// Can only read 1448 bytes in each loop
			int maxReadSize = 1448;
			int temp = 0;

			if (length > maxReadSize) {
				while ((length > (temp + maxReadSize - 1)) && (temp >= 0)) {
					temp += in.read(value, temp, maxReadSize);
				}
				while (((length - temp) > 0) && (temp >= 0)) {
					temp += in.read(value, temp, (length - temp));
				}
			} else {
				while (((length - temp) > 0) && (temp >= 0)) {
					temp += in.read(value, temp, (length - temp));
				}
			}
			if (temp < 0) {
				throw new SocketException("SRB socket connection is closed.");
			}
		} catch (IOException e) {
			IOException ioException = new IOException(
					"read() -- couldn't read complete packet");
			ioException.initCause(e);
			throw ioException;
		}

		if (DEBUG > 5) {
			System.err.print("Read: " + new String(value));
			if (DEBUG > 6) {
				for (int i = 0; i < value.length; i++) {
					System.err.print(value[i] + " ");
				}
			}
		}

		return value;
	}

	/**
	 * Reads a char from the server.
	 * 
	 * @return char char read from the server
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private char readChar() throws IOException {
		byte[] b = read(CHAR_LENGTH);
		Host.swap(b, CHAR_LENGTH);
		char value = (char) Host.castToShort(b);

		return value;
	}

	/**
	 * Reads a short from the server.
	 * 
	 * @return short short read from the server
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private short readShort() throws IOException {
		byte[] b = read(SHORT_LENGTH);
		Host.swap(b, SHORT_LENGTH);
		short value = Host.castToShort(b);

		return value;
	}

	/**
	 * Reads an int from the server.
	 * 
	 * @return int int read from the server
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private int readInt() throws IOException {
		byte[] b = read(INT_LENGTH);
		Host.swap(b, INT_LENGTH);
		int value = Host.castToInt(b);

		return value;
	}

	/**
	 * Reads an unsigned int from the server.
	 * 
	 * @return long unsigned int read from the server
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private long readUnsignedInt() throws IOException {
		// The SRB appears to send things MSBF sometimes and other times LSBF.
		long value = Host.castToUnsignedInt(read(INT_LENGTH));

		return value;
	}

	/**
	 * Reads a long from the server.
	 * 
	 * @return long long read from the server
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private long readLong() throws IOException {
		byte[] b = read(LONG_LENGTH);
		Host.swap(b, LONG_LENGTH);
		long value = Host.castToLong(b);

		return value;
	}

	/**
	 * Reads a Null terminated string from the server.
	 * 
	 * @param length
	 *            length of the string to be read
	 * @return String the String read from the server
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private String readString() throws IOException {
		String value = "";
		byte b = read();
		while (b != 0) {
			value += (char) b;
			b = read();
		}

		return value;
	}

	/**
	 * Reads a String from the server.
	 * 
	 * @param length
	 *            length of the string to be read
	 * @return String the String read from the server
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private String readString(int length) throws IOException {
		String value = new String(read(length));

		return value;
	}

	/**
	 * Checks the status of the server.
	 * 
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private void status() throws IOException {
		byte status[] = read(1);

		if (status[0] < STATUS_OK) {
			// status = 0 or sometimes char '0'
			throw new SRBException("Unknown error received from server.");
		}
	}

	/**
	 * Every SRB command sent to the server starts with "F \0", then the
	 * function id, then the number of arguments in that function.
	 * 
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void startSRBCommand(int functionId, int nargs) throws IOException {
		String doFunction = "F \0"; // Always starts a command

		send(doFunction.getBytes());

		send(functionId);
		send(nargs);
	}

	/**
	 * Sends the length of an argument, then the argument itself. After a SRB
	 * command is started all the command arguments must be sent in this format.
	 * 
	 * @param arg
	 *            The argument sent to the server.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void sendArg(int arg) throws IOException {
		send(INT_LENGTH);
		send(arg);
	}

	/**
	 * Sends the length of an argument, then the argument itself. After a SRB
	 * command is started all the command arguments must be sent in this format.
	 * 
	 * @param arg
	 *            The argument sent to the server.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void sendArg(long arg) throws IOException {
		send(LONG_LENGTH);
		send(arg);
	}

	/**
	 * Sends the length of an argument, then the argument itself. After a SRB
	 * command is started all the command arguments must be sent in this format.
	 * 
	 * @param arg
	 *            The argument sent to the server.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void sendArg(String arg) throws IOException {
		if (arg == null) {
			// send a null 4 bytes, for the string length.
			send(new byte[4]);
		} else {
			send(arg.length());
			send(arg.getBytes());
		}
	}

	/**
	 * Sends the length of an argument, then the argument itself. After a SRB
	 * command is started all the command arguments must be sent in this format.
	 * 
	 * @param arg
	 *            The argument sent to the server.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void sendArg(byte arg[]) throws IOException {
		if (arg == null) {
			// send a null 4 bytes, for the byte[] length.
			send(new byte[4]);
		} else {
			send(arg.length);
			send(arg);
		}
	}

	/**
	 * Sends the length of an argument, then the argument itself. After a SRB
	 * command is started all the command arguments must be sent in this format.
	 *<P>
	 * This method allows only part of a byte array to be sent, from 0 to
	 * length.
	 *<P>
	 * 
	 * @param arg
	 *            The argument sent to the server.
	 * @param length
	 *            Number of bytes from arg to be sent.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void sendArg(byte arg[], int length) throws IOException {
		if (arg == null) {
			// send a null 4 bytes, for the byte[] length.
			send(new byte[4]);
		} else {
			send(length);
			send(arg, 0, length);
		}
	}

	/**
	 * Sends the length of an argument, then the argument itself. After a SRB
	 * command is started all the command arguments must be sent in this format.
	 *<P>
	 * This method allows only part of a byte array to be sent, from offset to
	 * length.
	 *<P>
	 * 
	 * @param arg
	 *            The argument sent to the server.
	 * @param offset
	 *            Offset into arg, signifying start of bytes to be sent.
	 * @param length
	 *            Number of bytes from arg to be sent.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void sendArg(byte arg[], int offset, int length) throws IOException {
		if (arg == null) {
			// send a null 4 bytes, for the byte[] length.
			send(new byte[4]);
		} else {
			send(length);
			send(arg, offset, length);
		}
	}

	/**
	 * Sends the length of an argument, then the argument itself. After a SRB
	 * command is started all the command arguments must be sent in this format.
	 * 
	 * @param arg
	 *            The argument sent to the server.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void sendArg(int arg[]) throws IOException {
		send(arg.length * INT_LENGTH);
		for (int i = 0; i < arg.length; i++) {
			send(arg[i]);
		}
	}

	/**
	 * Sends the length of an argument, then the argument itself. After a SRB
	 * command is started all the command arguments must be sent in this format.
	 *<P>
	 * The array must have uniform dimensions. (The string lengths must be
	 * equal.)
	 *<P>
	 * 
	 * @param arg
	 *            The argument sent to the server.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void sendArg(char arg[][]) throws IOException {
		int argLength = arg.length * arg[0].length;
		byte byteArg[] = new byte[argLength];

		int k = 0;
		for (int i = 0; i < arg.length; i++) {
			for (int j = 0; j < arg[i].length; j++) {
				byteArg[k] = (byte) arg[i][j];
				k++;
			}
		}

		send(argLength);
		send(byteArg);
	}

	/**
	 * Sends the length of an argument, then the argument itself. After a SRB
	 * command is started all the command arguments must be sent in this format.
	 *<P>
	 * This method recreates the structure that is returned by a query. See
	 * also, returnSRBMetaDataRecordList() and mdasC_sql_result_struct *myresult
	 * from the C client.
	 * 
	 * @param rl
	 *            The SRBMetaDataRecordList[] argument sent to the server.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void sendArg(SRBMetaDataRecordList[] rl) throws IOException {
		byte[] nullByte = { 0 }; // seriously, I have to do it this way?
		String nll = new String(nullByte);

		int argLength = 0;

		// number of metadata attributes
		int result_count = rl[0].getFieldCount();
		byte result_count_bytes[] = new byte[INT_LENGTH];
		Host.copyInt(result_count, result_count_bytes);

		// number of datasets described
		int row_count = rl.length;
		byte row_count_bytes[] = new byte[INT_LENGTH];
		Host.copyInt(row_count, row_count_bytes);

		// the byte string contents of rl
		String values = "";
		int continuation_index = 0;

		for (int i = 0; i < rl[0].getFieldCount(); i++) {
			values += SRBMetaDataSet.getSRBDatabaseName(rl[0].getFieldName(i));
			for (int j = 0; j < rl.length; j++) {
				values += rl[j].getStringValue(i) + nll;
			}
		}

		// The c code myresult struct this method replicated has a fixed length
		// have to fill in the rest of it with 0001's
		// tab_name=0100,att_name=0100,values=0100 * (100-rows used)
		byte[] extraRows = new byte[((100 - rl[0].getFieldCount()) * 2) * 3];
		extraRows[0] = 0x01;
		for (int i = 1; i < extraRows.length - 1; i += 2) {
			extraRows[i] = 0x00;
			extraRows[i + 1] = 0x01;
		}

		// send argLength
		// int 0, result_count, row_count, values, extra rows,
		// continuation_index
		send(INT_LENGTH + INT_LENGTH + INT_LENGTH + values.length()
				+ extraRows.length + INT_LENGTH);

		send(0);

		send(result_count_bytes);
		send(row_count_bytes);
		send(values);
		send(extraRows);
		send(continuation_index);

		if (DEBUG > 2) {
			System.err.print("sendArg(rl): " + new String(result_count_bytes));
			System.err.println(new String(row_count_bytes));
			System.err.println(values);
			System.err.println(new String(extraRows));
			System.err.println(continuation_index);
		}
	}

	/**
	 * Parses the value returned from the server after a SRB command has been
	 * sent. Insures the function worked properly. If the first two bytes
	 * returned = "VG" then the command worked.
	 * <P>
	 * 
	 * @return true on success
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void commandStatus() throws IOException {
		byte resultBuffer[] = read(1);
		String exception = "\nThe SRB server returned an error: ";

		if (DEBUG > 1) {
			System.err.println("\n" + (new Date().getTime() - date)
					+ " millisecs");
		}

		if ((char) resultBuffer[0] == 'A') {

			// server is requesting re-authentication
			// sent the integer password option
			byte[] option = { 0, 0, 0, (byte) (account.getOptions() - 48), };
			out.write(option);

			switch (account.getOptions()) {
			case SRBAccount.PASSWD_AUTH:
				send(account.getPassword());
				break;
			case SRBAccount.GSI_AUTH:
				// GSIAuth was moved to a seperate class file, GSIAuth.
				// This is to allow compiling Jargon when not in possession of
				// the GSI libraries. It seems to be ok for me to include the
				// gsi libraries, but I'm not sure. So this makes it easy to
				// remove
				// just those libraries. see also GSIAuth.java_fake
				new GSIAuth(account, connection, out, in);
				break;
			/*
			 * case SRBAccount.GSI_SECURE_COMM: send( password ); break;
			 */
			case SRBAccount.GSI_DELEGATE:
				new GSIAuth(account, connection, out, in);
				break;
			case SRBAccount.ENCRYPT1:
				sendEncrypt1Auth(account.getPassword());
				break;
			default:
				send(account.getPassword());
			}

			send(new byte[38]);
			read(4);

			commandStatus();
			return;
		} else if ((char) resultBuffer[0] != 'V') {
			int exceptionType = -1;

			int bytesRead = 0;

			int maxErrorMsgs = 10;
			resultBuffer = new byte[ERROR_MSG_LENGTH * maxErrorMsgs];

			boolean moreErrorMsg = true;
			while (moreErrorMsg) {
				bytesRead = in.read(resultBuffer, 0, ERROR_MSG_LENGTH
						* maxErrorMsgs);

				if (bytesRead <= 0)
					break;

				for (int i = 0; i < bytesRead; i++) {
					if ((char) resultBuffer[i] == 'V') {
						if (resultBuffer[i + 1] == 'G') {
							if (resultBuffer[i + 2] == 8) {
								byte[] unsignedIntBuffer = {
										resultBuffer[i + 6],
										resultBuffer[i + 7],
										resultBuffer[i + 8],
										resultBuffer[i + 9] };
								Host.swap(unsignedIntBuffer, INT_LENGTH);
								exceptionType = Host
										.castToInt(unsignedIntBuffer);
								moreErrorMsg = false;
								i = bytesRead;
							} else {
								byte[] unsignedIntBuffer = {
										resultBuffer[i + 6],
										resultBuffer[i + 7],
										resultBuffer[i + 8],
										resultBuffer[i + 9] };
								exceptionType = (int) Host
										.castToUnsignedInt(unsignedIntBuffer);

								if (exceptionType > 0) {
									Host.swap(unsignedIntBuffer, INT_LENGTH);
									exceptionType = Host
											.castToInt(unsignedIntBuffer);
								}
								moreErrorMsg = false;
								i = bytesRead;
							}
						} else if (resultBuffer[i - 1] == 0) {
							// does the same thing as above if,
							// but i-1 would cause
							// ArrayIndexOutOfBoundsException
							byte[] unsignedIntBuffer = { resultBuffer[i + 6],
									resultBuffer[i + 7], resultBuffer[i + 8],
									resultBuffer[i + 9] };
							exceptionType = (int) Host
									.castToUnsignedInt(unsignedIntBuffer);

							if (exceptionType > 0) {
								Host.swap(unsignedIntBuffer, INT_LENGTH);
								exceptionType = Host
										.castToInt(unsignedIntBuffer);
							}
							moreErrorMsg = false;
							break;
						} else {
							exception += (char) resultBuffer[i];
						}
					} else {
						exception += (char) resultBuffer[i];
					}
				}
			}
			// windows wasn't catching the status until another read
			if (resultBuffer[bytesRead - 1] != 48)
				status();

			throw new SRBException(exception, exceptionType);
		} else {
			resultBuffer = read(1);
		}

		switch ((char) resultBuffer[0]) {
		case 'G': // function returned properly
			return;

		case 'E':
			throw new SRBException("SRB Server: returned an unknown error");

		case 'N':
			resultBuffer = new byte[ERROR_MSG_LENGTH];
			in.read(resultBuffer);

			if (resultBuffer.length == 1) {
				throw new SRBException(
						"Return detected, but error message cannot be read");
			} else {
				exception = new String(resultBuffer);

				throw new SRBException(exception);
			}

		case '0': // no return value
			return;

		default:
			exception = "FATAL: SRB protocol error: ";
			exception += new String(resultBuffer);

			throw new SRBException(exception);
		}
	}

	/**
	 * Interpretes the return value after a SRB command was sent.
	 * 
	 * @return returnValue the integer result
	 */
	private int returnInt() throws IOException {
		byte resultBuffer[] = read(1);
		int returnValue = -1;

		if (resultBuffer[0] <= INT_LENGTH) {
			resultBuffer = read(3);

			returnValue = readInt();
			status();
		} else {
			// Drain it
			resultBuffer = read(resultBuffer[0] + 1);
			String exception = "The SRB server returned an error: ";
			exception += new String(resultBuffer);

			throw new SRBException(exception);
		}

		return returnValue;
	}

	/**
	 * Interpretes the return value after a SRB command was sent.
	 * 
	 * @return returnValue the long result
	 */
	private long returnLong() throws IOException {
		byte resultBuffer[] = read(1);
		long returnValue = -1;

		if (resultBuffer[0] <= LONG_LENGTH) {
			resultBuffer = read(3);

			returnValue = readLong();
			status();
		} else {
			// Drain it
			resultBuffer = read(resultBuffer[0] + 1);
			String exception = "The SRB server returned an error: ";
			exception += new String(resultBuffer);

			throw new SRBException(exception);
		}

		return returnValue;
	}

	/**
	 * Interpretes the return value after a SRB command was sent.
	 * 
	 * @return resultBuffer the byte array result
	 */
	private byte[] returnBytes() throws IOException {
		byte resultBuffer[] = read(4);
		int resultLength = 0;

		for (int i = resultBuffer.length - 1; i >= 0; i--) {
			if (resultBuffer[i] < 0) {
				resultLength = (resultLength << 8) + 256 + resultBuffer[i];
			} else {
				resultLength = (resultLength << 8) + resultBuffer[i];
			}
		}
		if (resultLength > 4) {
			read(4);
			resultLength -= 4;

			resultBuffer = read(resultLength);
			read(1); // get final '0';
			return resultBuffer;
		} else {
			String exception = "\nThe SRB server returned an error: ";
			resultBuffer = read(INT_LENGTH);
			int exceptionInt = (int) Host.castToUnsignedInt(resultBuffer);
			exception += exceptionInt;
			read(1); // get final '0';

			if (exceptionInt < 0) {
				throw new SRBException(exception);
			} else if (exceptionInt == 0) {
				return null;
			}
			return resultBuffer;
		}
	}

	/**
	 * Interpretes the return value after a SRB command was sent.
	 * 
	 * @return SRBMetaDataRecordList the query result. The SRBMetaDataRecordList
	 *         class replaces mdasC_sql_result_struct *myresult from the C
	 *         client.
	 */
	SRBMetaDataRecordList[] returnSRBMetaDataRecordList(
			boolean usePortalHeader, InputStream in) throws IOException {
		// if using a different InputStream
		InputStream tempIn = null;
		if (in != null) {
			tempIn = this.in;
			this.in = in;
			read(4);
		}

		int i = 0, j = 0, k = 0, temp = 0;
		int bufferLength;
		if (singlePortBulkUnload) {
			read(4);
		}

		// total buffer length to read
		if (usePortalHeader)
			bufferLength = Host.castToInt(read(INT_LENGTH));
		else
			bufferLength = readInt();

		int status = (int) readUnsignedInt();

		if ((bufferLength == 0) && (in != null)) {
			if (status == 0) {

				read(4);

				this.in = tempIn;
				return null;
			} else if (status < 0) {
				throw new SRBException(
						"Protocol error in returnSRBMetaDataRecordList", status);
			} else {
				System.err
						.print("Protocol error in returnSRBMetaDataRecordList");
			}
		}

		if (usePortalHeader)
			read(4); // junk

		// quick check if the server returned an error
		if (status < 0) {
			if (DEBUG > 2)
				System.err.println(status);
			if (status == -3005) {
				read(1); // the final '0';
				return null;
			} else {
				String exception = "\nSRB server status: " + status;
				read(1); // the final '0';
				throw new SRBException(exception);
			}
		}

		// number of columns in the result
		int fieldCount = (int) readUnsignedInt();
		// number of rows returned
		int recordCount = (int) readUnsignedInt();
		// continuation index is packed at the end this time.

		// total buffer length - length variables
		// (bufferLength, fieldCount, etc.)
		int lengthVars = 16;
		if (usePortalHeader)
			lengthVars += 7;

		byte resultBuffer[] = read(bufferLength - lengthVars);

		if (usePortalHeader) {
			read(3); // more junk
		}

		// more query data left? negative number = invalid
		byte[] b = read(INT_LENGTH);
		int continuationIndex = (short) Host.castToInt(b);

		if (usePortalHeader) {
			read(7); // junk
		}

		if (DEBUG > 2) {
			System.err.println("bufferLength " + bufferLength);
			System.err.println("status " + status);
			System.err.println("fieldCount " + fieldCount);
			System.err.println("recordCount " + recordCount);

			System.err.println("continuationIndex " + continuationIndex);
			System.err.println("resultBuffer " + resultBuffer.length);

			if (DEBUG > 4) {
				for (i = 0; i < resultBuffer.length; i++) {
					if (resultBuffer[i] > 32)
						System.err.print((char) resultBuffer[i]);
					else if (resultBuffer[i] > 1)
						System.err.print(resultBuffer[i]);
					else if (i > 1) {
						if ((resultBuffer[i - 1] != 1)
								&& (resultBuffer[i - 2] != 0)
								&& (resultBuffer[i] != 1)) {
							System.err.print(resultBuffer[i]);
						}
					}
				}
				System.err.print("\n");
				for (i = 0; i < resultBuffer.length; i++) {
					System.err.print(" " + resultBuffer[i]);
				}
			}
		}

		// if using a different InputStream
		if (in != null) {
			this.in = tempIn;
		} else {
			status();
		}

		// SRB table name
		String tabName[] = new String[fieldCount];
		// SRB attribute name
		String attributeName[] = new String[fieldCount];

		// For parsing the bytes returned from the server
		String returnValue[][] = new String[fieldCount][recordCount];

		for (i = 0; i < fieldCount; i++) {
			// first get null terminated string, tabName
			tabName[i] = "";
			while ((resultBuffer[j] != 0) && (j < resultBuffer.length - 1)) {
				tabName[i] += (char) resultBuffer[j];
				j++;
			}
			// while((resultBuffer[j] == 0)&&(j < resultBuffer.length - 1)){
			if ((resultBuffer[j] == 0) && (j < resultBuffer.length - 1)) {
				j++;
			}

			// then null terminated string, attributeName
			attributeName[i] = "";
			while ((resultBuffer[j] != 0) && (j < resultBuffer.length - 1)) {
				attributeName[i] += (char) resultBuffer[j];
				j++;
			}
			if ((resultBuffer[j] == 0) && (j < resultBuffer.length - 1)) {
				j++;
			}

			// lastly, null-term string, values
			for (k = 0; k < recordCount; k++) {
				returnValue[i][k] = "";

				while ((resultBuffer[j] != 0) && (j < resultBuffer.length - 1)) {
					if (resultBuffer[j] > 0) {
						returnValue[i][k] += (char) resultBuffer[j];
					} else {
						// fixes the signed byte problem.
						// or rather lack of unsigned bytes in java.
						returnValue[i][k] += (char) (256 + resultBuffer[j]);
					}
					j++;
				}

				if ((resultBuffer[j] == 0) && (j < resultBuffer.length - 1)) {
					j++;
				}
			}
			if (resultBuffer[j] == 0) {
				// actually, should always = 0;
				j++;
			}
		}

		return parseSRBMetaDataRecordList(fieldCount, recordCount,
				continuationIndex, tabName, attributeName, returnValue);
	}

	/**
	 * Parses the return value after a SRB query was sent.
	 * 
	 * @return SRBMetaDataRecordList the query result. The SRBMetaDataRecordList
	 *         class replaces mdasC_sql_result_struct *myresult from the C
	 *         client.
	 */
	SRBMetaDataRecordList[] parseSRBMetaDataRecordList(int fieldCount,
			int recordCount, int continuationIndex, String[] tabName,
			String[] attributeName, String[][] returnValue) throws IOException {
		SRBMetaDataRecordList[] rl;
		MetaDataField[] fields, fields2 = null;
		Object[] singleReturnValue, singleReturnValue2 = null;
		int i = 0, j = 0, k = 0, l = 0, temp = 0;

		// Get a list of the fields the query returned
		fields = new MetaDataField[fieldCount];
		for (i = 0; i < fieldCount; i++) {
			fields[i] = SRBMetaDataSet.getGeneralMetaData(tabName[i],
					attributeName[i]);
		}
		if (DEBUG > 3) {
			for (i = 0; i < fieldCount; i++) {
				System.err.println("fields " + fields[i]);
			}
		}

		rl = new SRBMetaDataRecordList[recordCount];

		singleReturnValue = new Object[fieldCount];

		int metaDataRows = 0;
		int totalRows = 0;
		String fieldName = null;
		for (i = 0; i < recordCount; i++) {
			int fakeRows = 0;
			for (j = 0; j < fieldCount; j++) {
				fieldName = fields[j].getName();

				String nextFieldName = null;
				String compare1 = null, compare2 = null;
				int oneIndex = -1;
				int twoIndex = -1;
				if ((fieldName == SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES0)
						|| (fieldName == SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES0)
						|| (fieldName == SRBMetaDataSet.DEFINABLE_METADATA_FOR_RESOURCES0)
						|| (fieldName == SRBMetaDataSet.DEFINABLE_METADATA_FOR_USERS0)) {
					if (j + 1 < fieldCount) {
						nextFieldName = fields[j + 1].getName();
					}

					if (nextFieldName == SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES1) {
						// get the filename and compare2 for comparison of
						// records
						// for uniqueness. oneIndex and twoIndex keep track of
						// where
						for (k = 0; k < fieldCount; k++) {
							if (fields[k].getName() == SRBMetaDataSet.FILE_NAME) {
								compare1 = returnValue[k][i];
								oneIndex = k;
								if (twoIndex >= 0)
									break;
							}
							if (fields[k].getName() == SRBMetaDataSet.DIRECTORY_NAME) {
								compare2 = returnValue[k][i];
								twoIndex = k;
								if (oneIndex >= 0)
									break;
							}
						}
					} else if (nextFieldName == SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES1) {
						// get the filename and compare2 for comparison of
						// records
						// for uniqueness. oneIndex and twoIndex keep track of
						// where
						for (k = 0; k < fieldCount; k++) {
							if (fields[k].getName() == SRBMetaDataSet.DIRECTORY_NAME) {
								compare1 = returnValue[k][i];
								oneIndex = k;
								break;
							}
						}
					} else if (nextFieldName == SRBMetaDataSet.DEFINABLE_METADATA_FOR_RESOURCES1) {
						// get the filename and compare2 for comparison of
						// records
						// for uniqueness. oneIndex and twoIndex keep track of
						// where
						for (k = 0; k < fieldCount; k++) {
							if (fields[k].getName() == SRBMetaDataSet.RESOURCE_NAME) {
								compare1 = returnValue[k][i];
								oneIndex = k;
								break;
							}
						}
					} else if (nextFieldName == SRBMetaDataSet.DEFINABLE_METADATA_FOR_USERS1) {
						// get the filename and compare2 for comparison of
						// records
						// for uniqueness. oneIndex and twoIndex keep track of
						// where
						for (k = 0; k < fieldCount; k++) {
							if (fields[k].getName() == SRBMetaDataSet.USER_NAME) {
								compare1 = returnValue[k][i];
								oneIndex = k;
								if (twoIndex >= 0)
									break;
							}
							if (fields[k].getName() == SRBMetaDataSet.USER_DOMAIN) {
								compare2 = returnValue[k][i];
								twoIndex = k;
								if (oneIndex >= 0)
									break;
							}
						}
					}

					if (compare1 != null) {
						for (l = 0; l < recordCount; l++) {
							if (returnValue[oneIndex][l].equals(compare1)) {
								if ((compare2 == null)
										|| (returnValue[twoIndex][l]
												.equals(compare2))) {
									metaDataRows++;
								}
							}
						}

						// the next n rows should actually be one record list.
						// compare the compare1 and 2,
						// every unique combination gets its own
						// MetaDataRecordList.
						// the next ten records of the row are part of the
						// definable table

						String[][] tableValues = new String[metaDataRows][10];
						int[] operators = new int[metaDataRows];
						boolean empty = true;

						for (k = 0; k < metaDataRows; k++) {
							for (l = 0; l < tableValues[k].length; l++) {
								tableValues[k][l] = returnValue[j + l][k + i];
								if (empty && !tableValues[k][l].equals(""))
									empty = false;
							}
							operators[k] = MetaDataCondition.EQUAL;
							if (empty)
								tableValues[k] = null;
						}

						// So if the entire row was empty strings, throw it away
						List _tableValues = new ArrayList(tableValues.length);
						List _operators = new ArrayList(tableValues.length);
						for (int _i = 0; _i < tableValues.length; _i++) {
							if (tableValues[_i] != null) {
								_tableValues.add(tableValues[_i]);
								_operators.add(new Integer(operators[_i]));
							}
						}
						if (_tableValues.size() == 0) {
							tableValues = null;
							operators = null;
						} else {
							tableValues = (String[][]) _tableValues
									.toArray(new String[0][0]);
							operators = new int[tableValues.length];
							for (int _i = 0; _i < _operators.size(); _i++)
								operators[_i] = ((Integer) _operators.get(_i))
										.intValue();
						}

						if (tableValues == null) {
							singleReturnValue[j] = null;
							fakeRows++;
						} else {
							singleReturnValue[j] = new MetaDataTable(operators,
									tableValues);
						}
						fakeRows += 9;

						if (tableValues != null) {
							i += metaDataRows - 1;
						}
						j += 9;// A user definable table has ten columns
						metaDataRows = 0;
					} else {
						singleReturnValue[j] = returnValue[j][i];
					}
				} else {
					singleReturnValue[j] = returnValue[j][i];
				}
			}

			if (fakeRows > 0) {
				fields2 = new MetaDataField[fieldCount - fakeRows];
				// Remove the fields of other columns from a user definable,
				// create just one a table
				singleReturnValue2 = new Object[fieldCount - fakeRows];

				for (k = 0, l = 0; k < singleReturnValue.length; k++) {
					if (singleReturnValue[k] != null) {
						singleReturnValue2[l] = singleReturnValue[k];

						// replace first row field with table field
						if (fields[k].getName() == SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES0) {
							fields2[l] = MetaDataSet
									.getField(SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES);
						} else if (fields[k].getName() == SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES0) {
							fields2[l] = MetaDataSet
									.getField(SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES);
						} else if (fields[k].getName() == SRBMetaDataSet.DEFINABLE_METADATA_FOR_USERS0) {
							fields2[l] = MetaDataSet
									.getField(SRBMetaDataSet.DEFINABLE_METADATA_FOR_USERS);
						} else if (fields[k].getName() == SRBMetaDataSet.DEFINABLE_METADATA_FOR_RESOURCES0) {
							fields2[l] = MetaDataSet
									.getField(SRBMetaDataSet.DEFINABLE_METADATA_FOR_RESOURCES);
						} else
							fields2[l] = fields[k];
						l++;
					}
				}
				rl[i] = new SRBMetaDataRecordList(fields2, singleReturnValue2,
						continuationIndex, this);
				fields2 = null;
			} else {
				rl[i] = new SRBMetaDataRecordList(fields, singleReturnValue,
						continuationIndex, this);
			}
			totalRows++;
		}

		if (DEBUG > 3) {
			for (i = 0; i < rl.length; i++) {
				if (rl[i] != null) {
					for (j = 0; j < rl[i].getFieldCount(); j++) {
						System.err.println("rl[" + i + "]." + j + " "
								+ rl[i].getValue(j));
					}
				}
			}
		}

		SRBMetaDataRecordList[] rl2 = new SRBMetaDataRecordList[totalRows];
		for (i = 0, j = 0; i < rl.length; i++) {
			if (rl[i] != null) {
				rl2[j] = rl[i];
				j++;
			}
		}

		return rl2;
	}

	/**
	 * Create a SRB object.
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param objID
	 *            The SRB object ID to create. The objID is a user defined name
	 *            to be registered with MDAS. This ID will be used for
	 *            subsequent reference of the data object. One or more
	 *            conditions can be appended to the objID. Each condition must
	 *            be preceded by the character '&'. Currently, two conditions
	 *            are supported: 1) COPIES=MMM where MMM may be: a) an integer n
	 *            which means n replica should be created. The "resourceName"
	 *            input is the logical resource in which this object is to be
	 *            stored. This logical resource must consist of at least n
	 *            physical resources. e.g. foo&COPIES=2 specifies the creation
	 *            of two replica of data object "foo". b) the keyword RR which
	 *            means a single copy should be created in one of the physical
	 *            resources belonging to the input logical resource
	 *            ("resourceName") chosen in a Round-Robin fashion. e.g.
	 *            foo&COPIES=RR. c) the keyword RANDOM produces similar effect
	 *            as the RR keyword. The only difference is the selection
	 *            algorithm is random rather than Round-Robin. e.g.
	 *            foo&COPIES=RANDOM. 2) CONTAINER=containerName. This keyword
	 *            specifies the object is to be placed in the given container.
	 *            The container must have already been created using the
	 *            srbContainerCreate() call.
	 * 
	 * @param dataTypeName
	 *            Data type. e.g. "generic"
	 * @param resourceName
	 *            The storage resource name. This may be a the name of a single
	 *            resource or a resource group (or logical resource) consisting
	 *            of two or more physical resources. e.g. "mda18-unix-sdsc"
	 * 
	 * @param collectionName
	 *            The collection name.
	 * @param localPathName
	 *            The file/DB path of the data.
	 * @param dataSize
	 *            File size. Used by HPSS to determine COS. -1 => don't know and
	 *            the default COS will be used.
	 * 
	 * 
	 * @return the object descriptor.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized int srbObjCreate(int catType, String objID,
			String dataTypeName, String resourceName, String collectionName,
			String localPathName, long dataSize) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjCreate " + objID + " "
					+ collectionName + " " + resourceName);
		}
		startSRBCommand(F_SRBO_CREATE, 7);

		sendArg(catType);
		sendArg(objID);
		sendArg(dataTypeName);
		sendArg(resourceName);
		sendArg(collectionName);
		sendArg(localPathName);
		sendArg(dataSize);
		flush();

		commandStatus();
		int fd = returnInt();
		if (fd < 0) {
			throw new SRBException(fd);
		} else
			return fd;
	}

	/**
	 * Open a SRB object.
	 * 
	 * @param objID
	 *            The SRB object ID to open. The objID is obtained through
	 *            registration with MDAS. One or more conditions can be appended
	 *            to the objID. Each condition must be preceded by the character
	 *            '&'. Currently, only one condition is supported. i.e., COPY=n
	 *            (where n = replica number beginning with 0). e.g. foo&COPY=1
	 *            specifies opening replica number 1 of data object "foo".
	 * 
	 * @param openFlag
	 *            Unix type open flag. O_CREAT is not supported.
	 * @param collectionName
	 *            The name of the collection this objID belongs.
	 * @return the object descriptor.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized int srbObjOpen(String objID, int openFlag,
			String collectionName) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjOpen");
		}
		startSRBCommand(F_SRBO_OPEN, 3);

		sendArg(objID);
		sendArg(openFlag);
		sendArg(collectionName);
		flush();

		commandStatus();
		int fd = returnInt();
		if (fd < 0) {
			throw new SRBException(fd);
		} else
			return fd;
	}

	/**
	 * Close an opened object.
	 * 
	 * @param srbFD
	 *            The object descriptor (from the srbObjOpen call).
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbObjClose(int srbFD) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjClose");
		}
		startSRBCommand(F_SRBO_CLOSE, 1);

		sendArg(srbFD);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Unlink an SRB object.
	 * 
	 * @param objID
	 *            The SRB object ID to unlink. The objID is obtained through
	 *            registration with MDAS.
	 * @param collectionName
	 *            The name of the collection this objID belongs.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbObjUnlink(String objID, String collectionName)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjUnlink");
		}
		startSRBCommand(F_SRBO_UNLINK, 2);

		sendArg(objID);
		sendArg(collectionName);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Read some length of bytes of the SRB object into a buffer.
	 * 
	 * @param srbFD
	 *            The object descriptor (from the srbObjOpen call) to read.
	 * @param length
	 *            The number of bytes to read.
	 * @return a byte array filled with the data read from the server.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized byte[] srbObjRead(int srbFD, int length) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjRead");
		}
		startSRBCommand(F_SRBO_READ, 2);

		sendArg(srbFD);
		sendArg(length);
		flush();

		commandStatus();
		return returnBytes();
	}

	/**
	 * Write length bytes of output into the Object srbFD.
	 * 
	 * @param srbFD
	 *            The Object descriptor to write (from srbObjOpen).
	 * @param outputBuffer
	 *            [] The output buffer.
	 * @param length
	 *            The length to write.
	 * @return the number of bytes written.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized int srbObjWrite(int srbFD, byte output[], int length)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjWrite");
		}
		startSRBCommand(F_SRBO_WRITE, 2);

		sendArg(srbFD);
		sendArg(output, length);
		flush();

		commandStatus();
		int result = returnInt();
		if (result < 0) {
			throw new SRBException("Write failed", result);
		} else
			return result;
	}

	/**
	 * Change the current read or write location on an srb obj file currently.
	 * 
	 * @param desc
	 *            The object descriptor (from the srbObjOpen call) to seek.
	 * @param offset
	 *            int whence - Same definition as in Unix.
	 * @param whence
	 *            Same definition as in Unix. SEEK_SET pointer is set to the
	 *            value of the Offset parameter. SEEK_CUR pointer is set to its
	 *            current location plus the value of the Offset parameter.
	 *            SEEK_END pointer is set to the size of the file plus the value
	 *            of the Offset parameter.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbObjSeek(int desc, long offset, int whence)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjSeek");
		}
		startSRBCommand(F_SRBO_SEEK, 3);

		sendArg(desc);
		sendArg(offset);
		sendArg(whence);
		flush();

		commandStatus();
		int status = (int) returnLong();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Sync an opened object (call fsync for UNIX FS).
	 * 
	 * @param desc
	 *            The object descriptor (from the srbObjOpen call).
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbObjSync(int desc) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjSync");
		}
		startSRBCommand(F_SRBO_SYNC, 1);

		sendArg(desc);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Stat a srb Path.
	 * 
	 * @param catType
	 *            The catalog type.
	 * @param filePath
	 *            the SRB path.
	 * @param myType
	 *            file or dir state : IS_UNKNOWN, IS_FILE, IS_DIR_1, IS_DIR_2,
	 *            IS_DIR_3, IS_DIR_4.
	 * @return statBuffer array of stat values upon success.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized long[] srbObjStat(int catType, String filePath, int myType)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjStat");
		}
		long statBuffer[] = new long[30];
		long temp = 0;
		int i = 0, j = 0;
		int endLoop = 0; // stores array index of when to end a for loop
		int resultLength = 0;

		startSRBCommand(F_SRBO_STAT, 3);
		sendArg(catType);
		sendArg(filePath);
		sendArg(myType);
		flush();

		commandStatus();
		byte resultBuffer[] = returnBytes();

		// interprete the return as a array of Stat values.
		if (resultBuffer != null) {
			// statBuffer[0], file length
			for (j = 0; j < 8; j++) {
				if (resultBuffer[j] < 0) {
					temp = (temp << 8) + 256 + resultBuffer[j];
				} else {
					temp = (temp << 8) + resultBuffer[j];
				}
			}
			statBuffer[0] = temp;
			i++;

			// statBuffer[1-21]
			for (; i < statBuffer.length - 7; i++) {
				temp = 0;
				endLoop = j + 4;
				for (; j < endLoop; j++) {
					if (resultBuffer[j] < 0) {
						temp = (temp << 8) + 256 + resultBuffer[j];
					} else {
						temp = (temp << 8) + resultBuffer[j];
					}
				}
				statBuffer[i] = temp;
			}

			// statBuffer[22], Process' access to file
			// statBuffer[23], null
			for (; i < statBuffer.length - 5; i++) {
				temp = 0;
				endLoop = j + 2;
				for (; j < endLoop; j++) {
					if (resultBuffer[j] < 0) {
						temp = (temp << 8) + 256 + resultBuffer[j];
					} else {
						temp = (temp << 8) + resultBuffer[j];
					}
				}
				statBuffer[i] = temp;
			}

			// statBuffer[24-29], Reserved
			for (; i < statBuffer.length; i++) {
				temp = 0;
				endLoop = j + 4;
				for (; j < endLoop; j++) {
					if (resultBuffer[j] < 0) {
						temp = (temp << 8) + 256 + resultBuffer[j];
					} else {
						temp = (temp << 8) + resultBuffer[j];
					}
				}
				statBuffer[i] = temp;
			}
		}
		return statBuffer;
	}

	/**
	 * Stat a srb Path The result is placed in statbuf which is of type (strust
	 * stat) of the local fs.
	 * 
	 * @param catType
	 *            The catalog type.
	 * @param path
	 *            the SRB path.
	 * @return stat result in statbuf.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	long[] srbObjStat64(int catType, String path) throws IOException {
		return srbObjStat(catType, path, 0);// IS_UNKNOWN);

	}

	/**
	 * Make a copy of an SRB object
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param objID
	 *            The SRB object ID to unlink. The objID is obtained through
	 *            registration with MDAS.
	 * @param collectionName
	 *            The name of the collection this objID belongs.
	 * @param origPathName
	 *            The file/DB path of the original copy.
	 * @param newResourceName
	 *            The storage resource name of the new copy. e.g.
	 *            "mda18-unix-sdsc"
	 * @param newPathName
	 *            The file/DB path of the new copy.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbObjReplicate(int catType, String objID,
			String collectionName, String newResourceName, String newPathName)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjReplicate");
		}
		startSRBCommand(F_SRBO_REPLICATE, 5);

		sendArg(catType);
		sendArg(objID);
		sendArg(collectionName);
		sendArg(newResourceName);
		sendArg(newPathName);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Move a copy of an SRB object to a new location.
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param objID
	 *            The SRB object ID to unlink. The objID is obtained through
	 *            registration with MDAS.
	 * @param collectionName
	 *            The name of the collection this objID belongs.
	 * @param srcResource
	 *            The source resource to move - not used
	 * @param newResourceName
	 *            The storage resource name of the new copy. e.g.
	 *            "mda18-unix-sdsc"
	 * @param newPathName
	 *            The file/DB path of the new copy.
	 * @param container
	 *            The container to move into
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbObjMove(int catType, String objID,
			String collectionName, String srcResource, String newResourceName,
			String newPathName, String container) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjMove");
		}

		if (versionNumber >= ((float) 3.31)) {
			startSRBCommand(F_SRBO_MOVE, 7);

			sendArg(catType);
			sendArg(objID);
			sendArg(collectionName);
			sendArg(srcResource);
			sendArg(newResourceName);
			sendArg(newPathName);
			sendArg(container);
		} else {
			startSRBCommand(F_SRBO_MOVE, 5);

			sendArg(catType);
			sendArg(objID);
			sendArg(collectionName);
			sendArg(newResourceName);
			sendArg(newPathName);
		}
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Proxy Operation request.
	 * 
	 * @param operation
	 *            The type of proxy operation. Valid operations: OPR_COPY - Copy
	 *            from the object descriptor given in sourceInp to the object
	 *            descriptor given in destInp. If successful, the number of
	 *            bytes copied is returned. Negative values mean failure.
	 * 
	 *            OPR_COPY, OPR_COMMAND, SFO_CREATE_INDEX, SFO_DELETE_INDEX,
	 *            SFO_SEARCH_INDEX, SFO_GET_MORE_SEARCH_RESULT,
	 *            SFO_APPLY_FILTER, SFO_GET_MORE_FILTER_RESULT
	 * 
	 *            {OPR_COPY, (proxyFuncPtr) proxyCopy, "proxyCopy"},
	 *            {OPR_COMMAND, (proxyFuncPtr) proxyCommand, "proxyCommand"},
	 *            {SFO_CREATE_INDEX,(proxyFuncPtr) svrSfoCreateIndex,
	 *            "svrSfoCreateIndex"}, {SFO_DELETE_INDEX,(proxyFuncPtr)
	 *            svrSfoDeleteIndex, "svrSfoDeleteIndex"},
	 *            {SFO_SEARCH_INDEX,(proxyFuncPtr) svrSfoSearchIndex,
	 *            "svrSfoSearchIndex"},
	 *            {SFO_GET_MORE_SEARCH_RESULT,(proxyFuncPtr)
	 *            svrSfoGetMoreSearchResult, "svrSfoGetMoreSearchResult"},
	 *            {SFO_APPLY_FILTER,(proxyFuncPtr) svrSfoApplyFilter,
	 *            "svrSfoApplyFilter"},
	 *            {SFO_GET_MORE_FILTER_RESULT,(proxyFuncPtr)
	 *            svrSfoGetMoreFilterResult, "svrSfoGetMoreFilterResult"},
	 * 
	 * @param inputInt1
	 *            intput integer 1.
	 * @param inputInt2
	 *            intput integer 2.
	 * @param inputInt3
	 *            intput integer 3.
	 * @param inputInt4
	 *            intput integer 4.
	 * @param inputStr1
	 *            Input String 1.
	 * @param inputStr2
	 *            Input String 2.
	 * @param inputStr3
	 *            Input String 3.
	 * @param inputStr4
	 *            Input String 4.
	 * @param inputBStrm1
	 *            Input Byte stream 1.
	 * @param inputBStrm2
	 *            Input Byte stream 2.
	 * @param inputBStrm3
	 *            Input Byte stream 3.
	 * @return outBuf any byte stream output.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized byte[] srbObjProxyOpr(int operation, int inputInt1,
			int inputInt2, int inputInt3, int inputInt4, String inputStr1,
			String inputStr2, String inputStr3, String inputStr4,
			byte[] inputBStrm1, byte[] inputBStrm2, byte[] inputBStrm3)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjProxyOpr");
		}
		startSRBCommand(F_SRBO_PROXY_OPR, 12);

		sendArg(operation);
		sendArg(inputInt1);
		sendArg(inputInt2);
		sendArg(inputInt3);
		sendArg(inputInt4);
		sendArg(inputStr1);
		sendArg(inputStr2);
		sendArg(inputStr2);
		sendArg(inputStr4);
		sendArg(inputBStrm1);
		sendArg(inputBStrm2);
		sendArg(inputBStrm3);
		flush();

		commandStatus();

		// Sometimes returns bytes, other times returns int?
		byte resultBuffer[] = read(4);
		int resultLength = 0;

		for (int i = resultBuffer.length - 1; i >= 0; i--) {
			if (resultBuffer[i] < 0) {
				resultLength = (resultLength << 8) + 256 + resultBuffer[i];
			} else {
				resultLength = (resultLength << 8) + resultBuffer[i];
			}
		}

		if (resultLength > 4) {
			// ignore sizeOf(resultLength) from result to get real data length.
			read(4);
			resultLength -= 4;

			resultBuffer = read(resultLength);
			read(1); // get final '0';
			return resultBuffer;
		} else {
			resultBuffer = read(4);
			status();
			return resultBuffer;
		}
	}

	/**
	 * Seek into a collection. A collection must have been opened using
	 * srbObjOpen.
	 *<P>
	 * 
	 * @param desc
	 *            The object descriptor (from the srbObjOpen call) to seek.
	 * @param offset
	 *            The position of the next operation
	 * @param whence
	 *            Same definition as in Unix. SEEK_SET pointer is set to the
	 *            value of the Offset parameter. SEEK_CUR pointer is set to its
	 *            current location plus the value of the Offset parameter.
	 *            SEEK_END pointer is set to the size of the file plus the value
	 *            of the Offset parameter.
	 * @param is64Flag
	 *            valid vaiue: IS_64_BIT or IS_32_BIT.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbCollSeek(int desc, int offset, int whence, int is64Flag)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbCollSeek");
		}
		startSRBCommand(F_SRBC_SEEK, 4);

		sendArg(desc);
		sendArg(offset);
		sendArg(whence);
		sendArg(is64Flag);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * srbGetDatasetInfo Get Info on a SRB data object.
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param objID
	 *            The SRB object ID to unlink. The objID is obtained through
	 *            registration with MDAS.
	 * @param collectionName
	 *            The name of the collection this objID belongs.
	 * @param rowsWanted
	 *            number of rows of result wanted.
	 * @return SRBMetaDataRecordList. Use srbGetMoreRows() to retrieve more
	 *         rows.
	 */
	synchronized SRBMetaDataRecordList[] srbGetDatasetInfo(int catType,
			String objID, String collectionName, int rowsWanted)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbGetDatasetInfo");
		}
		startSRBCommand(F_SRBO_GET_LOID_INFO, 4);

		sendArg(catType);
		sendArg(objID);
		sendArg(collectionName);
		sendArg(rowsWanted);
		flush();

		commandStatus();
		return returnSRBMetaDataRecordList(false, null);
	}

	// There is a the possibility that a lot of unnecessary queries get sent.
	// This is to help with keeping track and reducing the number.
	int srbGetDataDirInfoCount = 0;

	/**
	 * This is a more compact form of srbGetDataDirInfo. Instead of using fixed
	 * array of selval and qval, it uses the genQuery struct.
	 * <P>
	 * Only valid for version 3.x.x and above. Method will not work for version
	 * 2.x.x, or before.
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param myMcatZone
	 *            The MCAT zone for this query.
	 * @param metaDataConditions
	 *            The query conditions.
	 * @param metaDataSelects
	 *            The query selects.
	 * @param rowsWanted
	 *            The number of rows of result wanted.
	 * 
	 * @return SRBMetaDataRecordList. Use srbGetMoreRows() to retrieve more
	 *         rows.
	 * 
	 *         Note : We cannot use the generic routine clCall() because the
	 *         input byte stream is not a char string.
	 */
	synchronized SRBMetaDataRecordList[] srbGenQuery(int catType,
			String myMcatZone, MetaDataCondition[] metaDataConditions,
			MetaDataSelect[] metaDataSelects, int rowsWanted, boolean orderBy,
			boolean nonDistinct) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbGenQuery " + srbGetDataDirInfoCount);
			srbGetDataDirInfoCount++;
		}

		SRBMetaDataCommands mdc = new SRBMetaDataCommands(this);
		return mdc.srbGenQuery(catType, myMcatZone, metaDataConditions,
				metaDataSelects, rowsWanted, orderBy, nonDistinct);
	}

	/**
	 * Register a SRB dataset
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param objID
	 *            The SRB object ID to create. The objID is obtained through
	 *            registration with MDAS.
	 * @param dataTypeName
	 *            Data type. e.g. "generic"
	 * @param resourceName
	 *            The storage resource name. e.g. "mda18-unix-sdsc"
	 * @param collectionName
	 *            The collection name.
	 * @param pathName
	 *            The file/DB path of the data.
	 * @param dataSize
	 *            The size of the dataset if known. 0 = unknown.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbRegisterDataset(int catType, String objID,
			String dataTypeName, String resourceName, String collectionName,
			String pathName, long dataSize) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRegisterDataset");
		}
		startSRBCommand(F_SRBO_REGISTER_FILE, 7);

		sendArg(catType);
		sendArg(objID);
		sendArg(dataTypeName);
		sendArg(resourceName);
		sendArg(collectionName);
		sendArg(pathName);
		sendArg(dataSize);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Modify a SRB dataset.
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param objID
	 *            The SRB object ID to modify. The objID must already have been
	 *            registered with the MDAS catalog.
	 * @param collectionName
	 *            The name of the collection this objID belongs.
	 * @param resourceName
	 *            The storage resource name. e.g. "mda18-unix-sdsc"
	 * @param pathName
	 *            The file/DB path of the data.
	 * @param dataValue1
	 *            Input value 1.
	 * @param dataValue2
	 *            Input value 2.
	 * @param actionType
	 *            The type of retraction. See srbC_mdas_externs.h for the
	 *            actionType definition.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized int srbModifyDataset(int catType, String objID,
			String collectionName, String resourceName, String pathName,
			String dataValue1, String dataValue2, int actionType)
			throws IOException {
		if (DEBUG > 0) {
			System.err.println("\n srbModifyDataset");
			if (DEBUG > 2) {
				System.err.println(" catType: " + catType + " objID: " + objID
						+ " collectionName: " + collectionName
						+ " resourceName: " + resourceName + " pathName: "
						+ pathName + " dataValue1: " + dataValue1
						+ " dataValue2: " + dataValue2 + " actionType: "
						+ actionType);
			}
		}

		startSRBCommand(F_SRBO_MODIFY_FILE, 8);

		sendArg(catType);
		sendArg(objID);
		sendArg(collectionName);
		sendArg(resourceName);
		sendArg(pathName);
		sendArg(dataValue1);
		sendArg(dataValue2);
		sendArg(actionType);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		} else
			return status;
	}

	/**
	 * Authenticate a userName/passwd.
	 * 
	 * @param userName
	 *            and
	 * @param srbAuth
	 *            The userName/passwd pair to authenticate.
	 * @param mdasDomain
	 *            The MDAS Domain.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbChkMdasAuth(String userName, String srbAuth,
			String mdasDomain) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbChkMdasAuth");
		}
		startSRBCommand(F_CHK_MDAS_AUTH, 3);

		sendArg(userName);
		sendArg(srbAuth);
		sendArg(mdasDomain);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Create a SRB collection.
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param parentCollect
	 *            The parent collection in which to create the new collection.
	 * @param newCollect
	 *            The name of the collection to create.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbCreateCollect(int catType, String parentCollect,
			String newCollect) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbCreateCollect");
		}
		startSRBCommand(F_CREATE_DIRECTORY, 3);

		sendArg(catType);
		sendArg(parentCollect);
		sendArg(newCollect);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * List a SRB collection
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param collectionName
	 *            The collection to list.
	 * @param flag
	 *            "C" - non-recursive. "R" - recursive mdasC_sql_result_struct
	 *            *listResult - The address points to the result. A pointer to a
	 *            user supplied mdasC_sql_result_struct.
	 * @return The list of items in the collection.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized SRBMetaDataRecordList[] srbListCollect(int catType,
			String collectionName, String flag, int rowsWanted)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbListCollect");
		}
		startSRBCommand(F_LIST_DIRECTORY, 4);

		sendArg(catType);
		sendArg(collectionName);
		sendArg(flag);
		sendArg(rowsWanted);
		flush();

		commandStatus();
		return returnSRBMetaDataRecordList(false, null);
	}

	/**
	 * Modify a SRB collection.
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param collectionName
	 *            The name of the collection to modify.
	 * @param dataValue1
	 *            Input value 1.
	 * @param dataValue2
	 *            Input value 2.
	 * @param dataValue3
	 *            Input value 3.
	 * @param actionType
	 *            The type of retraction. See srbC_mdas_externs.h for the
	 *            actionType definition.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbModifyCollect(int catType, String collectionName,
			String dataValue1, String dataValue2, String dataValue3,
			int actionType) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbModifyCollect " + catType + " "
					+ collectionName + " " + dataValue1 + " " + dataValue2
					+ " " + dataValue3 + " " + actionType);
		}
		startSRBCommand(F_MODIFY_DIRECTORY, 6);

		sendArg(catType);
		sendArg(collectionName);
		sendArg(dataValue1);
		sendArg(dataValue2);
		sendArg(dataValue3);
		sendArg(actionType);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Authenticate a userName/passwd for sys admin access.
	 * 
	 * @param userName
	 *            userName to authenticate.
	 * @param srbAuth
	 *            password to authenticate.
	 * @param mdasDomain
	 *            The MDAS Domain.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbChkMdasSysAuth(String userName, String srbAuth,
			String mdasDomain) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbChkMdasSysAuth");
		}
		startSRBCommand(F_CHK_MDAS_SYS_AUTH, 3);

		sendArg(userName);
		sendArg(srbAuth);
		sendArg(mdasDomain);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

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
	synchronized void srbRegisterUserGrp(int catType, String userGrpName,
			String userGrpPasswd, String userGrpType, String userGrpAddress,
			String userGrpPhone, String userGrpEmail) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRegisterUserGrp");
		}
		startSRBCommand(F_REGISTER_USER_GROUP, 7);

		sendArg(catType);
		sendArg(userGrpName);
		sendArg(userGrpPasswd);
		sendArg(userGrpType);
		sendArg(userGrpAddress);
		sendArg(userGrpPhone);
		sendArg(userGrpEmail);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

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
	synchronized void srbRegisterUser(int catType, String userName,
			String userDomain, String userPasswd, String userType,
			String userAddress, String userPhone, String userEmail)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRegisterUser");
		}
		startSRBCommand(F_REGISTER_USER, 8);

		sendArg(catType);
		sendArg(userName);
		sendArg(userDomain);
		sendArg(userPasswd);
		sendArg(userType);
		sendArg(userAddress);
		sendArg(userPhone);
		sendArg(userEmail);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

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
	synchronized void srbModifyUser(int catType, String dataValue1,
			String dataValue2, int actionType) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbModifyUser");
		}
		startSRBCommand(F_MODIFY_USER, 4);

		sendArg(catType);
		sendArg(dataValue1);
		sendArg(dataValue2);
		sendArg(actionType);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Setting and Unsetting Audit Trail.
	 * 
	 * @param set_value
	 *            The Audit Trail value to set. AUDIT_TRAIL_OFF turn on audit
	 *            trail. AUDIT_TRAIL_ON turn on audit trail.
	 *            GET_AUDIT_TRAIL_SETTING return the cuurent audit trail setting
	 *            without modifying the setting.
	 * 
	 * @return the currently audit trail setting (after processing the latest
	 *         change request).
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized int srbSetAuditTrail(int set_value) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbSetAuditTrail");
		}
		startSRBCommand(F_SET_AUDIT_TRAIL, 1);

		sendArg(set_value);
		flush();
		commandStatus();
		return returnInt();
	}

	/**
	 * Make an audit entry for an object
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param userName
	 *            The userName.
	 * @param objID
	 *            The objID of the object to audit.
	 * @param collectionName
	 *            The name of the collection this objID belongs.
	 * @param dataPath
	 *            The path name of the object.
	 * @param resourceName
	 *            the resource name of the object.
	 * @param accessMode
	 *            The access mode ("read", "write", "all");
	 * @param comment
	 *            comments to be included. int success Indication whether the
	 *            operation was successful. 0 = failure, 1 success.
	 * @param domainName
	 *            The domain name of the user.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbObjAudit(int catType, String userName, String objID,
			String collectionName, String dataPath, String resourceName,
			String accessMode, String comment, int success, String domainName)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjAudit");
		}
		startSRBCommand(F_SRBO_AUDIT, 10);

		sendArg(catType);
		sendArg(userName);
		sendArg(objID);
		sendArg(collectionName);
		sendArg(dataPath);
		sendArg(resourceName);
		sendArg(accessMode);
		sendArg(comment);
		sendArg(success);
		sendArg(domainName);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Register a SRB Replica
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param objID
	 *            The SRB object ID to create. The objID is obtained through
	 *            registration with MDAS.
	 * @param collectionName
	 *            The collection name.
	 * @param origPathName
	 *            The orighnal file/DB path of the data. (This entry can be
	 *            NULL).
	 * @param newResourceName
	 *            The new storage resource name. e.g. "mda18-unix-sdsc"
	 * @param newPathName
	 *            The new file/DB path of the data.
	 * @param userName
	 *            The new file/DB path of the data. struct varlena *domainName
	 *            The domain name to which the object belongs. Valid domains
	 *            "sdsc".
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbRegisterReplica(int catType, String objID,
			String collectionName, String origResourceName,
			String origPathName, String newResourceName, String newPathName,
			String userName, String domainName) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRegisterReplica");
		}
		startSRBCommand(F_REGISTER_REPLICA, 9);

		sendArg(catType);
		sendArg(objID);
		sendArg(collectionName);
		sendArg(origResourceName);
		sendArg(origPathName);
		sendArg(newResourceName);
		sendArg(newPathName);
		sendArg(userName);
		sendArg(domainName);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Read the privileged users list and put it in a user supplied
	 * mdasC_sql_result_struct.
	 * 
	 * @param catalog
	 *            The catalog type. e.g., MDAS_CATALOG
	 * @param rowsWanted
	 *            The number of rows to be returned.
	 * @return SRBMetaDataRecordList. Use srbGetMoreRows() to retrieve more
	 *         rows.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized SRBMetaDataRecordList[] srbGetPrivUsers(int catalog,
			int rowsWanted) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbGetPrivUsers");
		}
		startSRBCommand(F_GET_PRIV_USERS, 2);

		sendArg(catalog);
		sendArg(rowsWanted);
		flush();

		commandStatus();
		return returnSRBMetaDataRecordList(false, null);
	}

	/**
	 * Get more rows of result from a srbGetDatasetInfo, srbGetDataDirInfo,
	 * srbListCollect or srbGetPrivUsers calls and put the results in a user
	 * supplied mdasC_sql_result_struct.
	 * 
	 * @param catalog
	 *            The catalog type. e.g., MDAS_CATALOG mdasC_sql_result_struct
	 *            *srbUserList A pointer to a user supplied
	 *            mdasC_sql_result_struct.
	 * @param contDesc
	 *            The continuation descriptor. This is a non negative integer
	 *            returned from a srbGetDatasetInfo, srbGetDataDirInfo,
	 *            srbListCollect or srbGetPrivUsers call as
	 *            SRBMetaDataRecordList->continuationIndex.
	 * @param rowsWanted
	 *            The number of rows to be returned.
	 * @return the values from the last query.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized SRBMetaDataRecordList[] srbGetMoreRows(int catalog,
			int contDesc, int rowsWanted) throws IOException {
		// method no longer used as of jargon1.4.22
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbGetMoreRows " + catalog + " " + contDesc
					+ " " + rowsWanted);
		}

		if (contDesc < 0)
			return null;

		return srbGenGetMoreRows(catalog, contDesc, rowsWanted);
	}

	/**
	 * This is a more compact form of srbGetMoreRows. The result is packed with
	 * packMsg. Get more rows of result from a srbGetDatasetInfo,
	 * srbGetDataDirInfo, srbListCollect or srbGetPrivUsers calls and put the
	 * results in a user supplied mdasC_sql_result_struct.
	 * 
	 * @param catalog
	 *            The catalog type. e.g., MDAS_CATALOG
	 * @param contDesc
	 *            The continuation descriptor. This is a non negative integer
	 *            returned from a srbGetDatasetInfo, srbGetDataDirInfo,
	 *            srbListCollect or srbGetPrivUsers call as
	 *            SRBMetaDataRecordList.continuationIndex.
	 * @param rowsWanted
	 *            The number of rows to be returned.
	 * @return Further results from the query.
	 */
	SRBMetaDataRecordList[] srbGenGetMoreRows(int catalog, int contDesc,
			int rowsWanted) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbGenGetMoreRows " + catalog + " "
					+ contDesc + " " + rowsWanted);
		}
		if (contDesc < 0)
			return null;

		try {
			startSRBCommand(F_GEN_GET_MORE_ROWS, 3);

			sendArg(catalog);
			sendArg(contDesc);
			sendArg(rowsWanted);
			flush();

			commandStatus();
			return returnSRBMetaDataRecordList(false, null);
		} catch (SRBException e) {
			if (e.getType() == -3005) {
				// MCAT error, the object queried does not exist
				return null;
			} else
				throw e;
		}
	}

	/**
	 * Issue a ticket.
	 * 
	 * @param objID
	 *            The object ID
	 * @param collectionName
	 *            The collection name
	 * @param collectionFlag
	 *            The collect flag if vCollectionName is non NULL. "R" the
	 *            ticket is for all dataset and sub-collection recursively. "D"
	 *            the ticket is for the datasets directly beneath the
	 *            colloection.
	 * @param beginTime
	 *            The beginning time when the ticket becomes effective. A NULL
	 *            means no time limit.
	 * @param endTime
	 *            The ending time of the ticket.
	 * @param accessCnt
	 *            The number of time the ticket can be used to access the
	 *            dataset.
	 * @param ticketUser
	 *            The user/userGroup that will use the ticket. Multiply users
	 *            can be specified with the following format:
	 *            user1@domain1&user2@domain2 .... If it is NULL, => all users.
	 * @param ticket
	 *            The address to put the output ticket.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbIssueTicket(String objID, String collectionName,
			String collectionFlag, String beginTime, String endTime,
			int accessCnt, String ticketUser) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbIssueTicket");
		}
		startSRBCommand(F_ISSUE_TICKET, 7);

		sendArg(objID);
		sendArg(collectionName);
		sendArg(collectionFlag);
		sendArg(beginTime);
		sendArg(endTime);
		sendArg(accessCnt);
		sendArg(ticketUser);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}

	}

	/**
	 * Cancel a ticket.
	 * 
	 * @param ticket
	 *            The ticket to remove.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbRemoveTicket(String ticket) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRemoveTicket");
		}
		startSRBCommand(F_REMOVE_TICKET, 1);

		sendArg(ticket);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Unregister an SRB object
	 * 
	 * @param objID
	 *            The SRB object ID to unlink. The objID is obtained through
	 *            registration with MDAS.
	 * @param collectionName
	 *            The name of the collection this objID belongs.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbUnregisterDataset(String objID, String collectionName)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbUnregisterDataset");
		}
		startSRBCommand(F_UNREGISTER_FILE, 2);

		sendArg(objID);
		sendArg(collectionName);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Create a container
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param containName
	 *            The name of the container to be created.
	 * @param containerType
	 *            Data type of the container. e.g. "generic"
	 * @param resourceName
	 *            The storage resource name. This should be is a logical
	 *            resource (resource group) consisting of two physical
	 *            resources, a TEMPORARY_RES_CL and a PERMANENT_RES_CL class.
	 * @param containerSize
	 *            The size of the container to be created.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbContainerCreate(int catType, String containerName,
			String containerType, String resourceName, long containerSize)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbContainerCreate");
		}
		startSRBCommand(F_CONTAINER_CREATE, 5);

		sendArg(catType);
		sendArg(containerName);
		sendArg(containerType);
		sendArg(resourceName);
		sendArg(containerSize);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Register a container to MCAT.
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param containerName
	 *            The name of the container to be registered..
	 * @param resourceName
	 *            The storage resource name. e.g. "mda18-unix-sdsc"
	 * @param containerSize
	 *            The size of the container.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbRegisterContainer(int catType, String containerName,
			String resourceName, long containerSize) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRegisterContainer");
		}
		startSRBCommand(F_SRBO_CREATE, 4);

		sendArg(catType);
		sendArg(containerName);
		sendArg(resourceName);
		sendArg(containerSize);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Register a inContainer dataset
	 * 
	 * @param catType
	 *            catalog type. e.g., MDAS_CATALOG.
	 * @param objID
	 *            The SRB object ID to create. The objID is obtained through
	 *            registration with MDAS.
	 * @param collectionName
	 *            The collection name.
	 * @param containerName
	 *            The collection name.
	 * @param dataTypeName
	 *            Data type. e.g. "generic"
	 * @param dataSize
	 *            The size of the dataset if known. 0 = unknown.
	 * @param baseOffset
	 *            The offset of the dataset if known. 0 = unknown.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbRegisterInContDataset(int catType, String objID,
			String collectionName, String containerName, String dataTypeName,
			long dataSize, long baseOffset) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRegisterInContDataset");
		}
		startSRBCommand(F_REGISTER_IN_CONTAINER, 7);

		sendArg(catType);
		sendArg(objID);
		sendArg(collectionName);
		sendArg(containerName);
		sendArg(dataTypeName);
		sendArg(dataSize);
		sendArg(baseOffset);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Get Info on a SRB container.
	 * 
	 * @param catType
	 *            The catalog type. e.g., MDAS_CATALOG
	 * @param containerName
	 *            The name of the container.
	 * @param rowsWanted
	 *            number of rows of result wanted.
	 * @return SRBMetaDataRecordList. Use srbGetMoreRows() to retrieve more
	 *         rows.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized SRBMetaDataRecordList[] srbGetContainerInfo(int catType,
			String containerName, int rowsWanted) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbGetContainerInfo");
		}
		startSRBCommand(F_GET_CONTAINER_INFO, 3);

		sendArg(catType);
		sendArg(containerName);
		sendArg(rowsWanted);
		flush();

		commandStatus();
		return returnSRBMetaDataRecordList(false, null);
	}

	/**
	 * Given the logical resource name and the inputFlag, return a physical
	 * resource name.
	 * 
	 * @param catType
	 *            catalog type 0 MCAT
	 * @param logResName
	 *            The logical resource name.
	 * @param phyResName
	 *            The output physical resource.
	 * @param inputFlag
	 *            The Input flag, valid inputs are: "RR" Round Robin. "RANDOM"
	 *            randomly selecting a physical resource from the input logical
	 *            resource.
	 * @return The Physical resource string.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized String srbGetResOnChoice(int catType, String logResName,
			String phyResName, String inputFlag) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjProxyOpr");
		}
		startSRBCommand(F_GET_RESC_ON_CHOICE, 3);

		sendArg(catType);
		sendArg(logResName);
		sendArg(inputFlag);
		flush();

		commandStatus();
		return new String(returnBytes());
	}

	/**
	 * Remove a container
	 * 
	 * @param catType
	 *            catalog type 0 MCAT
	 * @param containerName
	 *            The name of the container.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbRmContainer(int catType, String containerName,
			boolean force) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRmContainer");
		}
		if (versionNumber <= 3) {
			startSRBCommand(F_REMOVE_CONTAINER, 2);

			sendArg(catType);
			sendArg(containerName);
		} else if (force) {
			startSRBCommand(F_REMOVE_CONTAINER, 3);

			sendArg(catType);
			sendArg(containerName);
			sendArg(SRBMetaDataSet.D_DELETE_ONE);
		} else {
			startSRBCommand(F_REMOVE_CONTAINER, 3);

			sendArg(catType);
			sendArg(containerName);
			sendArg(SRBMetaDataSet.D_SU_DELETE_TRASH_ONE);
		}

		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Sync a container. Sync the permanant replica with the temporary replica.
	 * 
	 * @param catType
	 *            catalog type 0 MCAT
	 * @param containerName
	 *            The name of the container.
	 * @param syncFlag
	 *            valid values are: PURGE_FLAG purge the cache copies after sync
	 *            is done. PRIMARY_FLAG Synchronize to the primary archival
	 *            resource only. The default is to synchronize all archival
	 *            resources.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbSyncContainer(int catType, String containerName,
			int syncFlag) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbSyncContainer");
		}
		startSRBCommand(F_SYNC_CONTAINER, 3);

		sendArg(catType);
		sendArg(containerName);
		sendArg(syncFlag);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Replicate a container.
	 * 
	 * @param catType
	 *            catalog type 0 MCAT
	 * @param containerName
	 *            The name of the container.
	 * @param newResourceName
	 *            The resource for the replica.
	 * @throws IOException
	 *             If an IOException occurs
	 */
	synchronized void srbReplContainer(int catType, String containerName,
			String newResourceName) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbReplContainer");
		}
		startSRBCommand(F_REPLICATION_CONTAINER, 3);

		sendArg(catType);
		sendArg(containerName);
		sendArg(newResourceName);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Register an internal compound obj.
	 * 
	 * @param objID
	 *            The name of the compound obj
	 * @param collectionName
	 *            The collection
	 * @param objReplNum
	 *            The replication number of the compound obj
	 * @param objSegNum
	 *            The segment number of the compound obj
	 * @param intObjRescName
	 *            The resource where the internal compound obj is located.
	 * @param dataPathName
	 *            The physical path of the int comp obj.
	 * @param dataSize
	 *            The size of the int comp obj.
	 * @param offset
	 *            The offset of the int comp obj.
	 * @param inpIntReplNum
	 *            The replication number of the int compound obj.
	 * @param intSegNum
	 *            The segment number of the int compound obj.
	 * @param objTypeInx
	 *            In case of failure, if objTypeInx >=0, the int comp obj will
	 *            be removed. It is the object type index used to do the
	 *            unlinking.
	 * @param phyResLoc
	 *            Valid only when objTypeInx >=0. This is the resouce location
	 *            used to do the unlinking.
	 */
	synchronized void srbRegInternalCompObj(String objName, String objCollName,
			int objReplNum, int objSegNum, String intObjRescName,
			String dataPathName, long dataSize, long offset, int inpIntReplNum,
			int intSegNum, int objTypeInx, String phyResLoc) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRegInternalCompObj");
		}
		startSRBCommand(F_CMP_REG_INT_COMP_OBJ, 12);

		sendArg(objName);
		sendArg(objCollName);
		sendArg(objReplNum);
		sendArg(objSegNum);
		sendArg(intObjRescName);
		sendArg(dataPathName);
		sendArg(dataSize);
		sendArg(offset);
		sendArg(inpIntReplNum);
		sendArg(intSegNum);
		sendArg(objTypeInx);
		sendArg(phyResLoc);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Unregister an internal compound obj.
	 * 
	 * @param objID
	 *            The name of the compound obj
	 * @param collectionName
	 *            The collection
	 * @param objReplNum
	 *            The replication number of the compound obj
	 * @param objSegNum
	 *            The segment number of the compound obj
	 * @param inpIntReplNum
	 *            The replication number of the int compound obj.
	 * @param intSegNum
	 *            The segment number of the int compound obj.
	 */
	synchronized void srbRmIntCompObj(String objName, String objCollName,
			int objReplNum, int objSegNum, int inpIntReplNum, int intSegNum)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRmIntCompObj");
		}
		startSRBCommand(F_CMP_STAGE_COMP_OBJ, 6);

		sendArg(objName);
		sendArg(objCollName);
		sendArg(objReplNum);
		sendArg(objSegNum);
		sendArg(inpIntReplNum);
		sendArg(intSegNum);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Unregister a compound obj.
	 * 
	 * Input - String objID - The name of the compound obj String collectionName
	 * - The collection int objReplNum - The replication number of the compound
	 * obj int objSegNum - The segment number of the compound obj.
	 */
	synchronized void srbRmCompObj(String objName, String objCollName,
			int objReplNum, int objSegNum) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRmCompObj");
		}
		startSRBCommand(F_CMP_RM_COMP_OBJ, 4);

		sendArg(objName);
		sendArg(objCollName);
		sendArg(objReplNum);
		sendArg(objSegNum);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Modify a SRB internal comp obj.
	 * 
	 * Input - int catType - catalog type. e,g., MDAS_CATALOG. String objID -
	 * The SRB object ID to modify. The objID must already have been registered
	 * with the MDAS catalog. int objReplNum - The replication number of the
	 * compound obj int objSegNum - The segment number of the compound obj int
	 * inpIntReplNum - The replication number of the int compound obj. int
	 * intSegNum - The segment number of the int compound obj. String dataValue1
	 * - Input value 1. String dataValue2 - Input value 2. String dataValue3 -
	 * Input value 3. String dataValue4 - Input value 4. int actionType - The
	 * type of retraction. See srbC_mdas_externs.h for the actionType
	 * definition.
	 */
	synchronized void srbModInternalCompObj(String objID,
			String collectionName, int objReplNum, int objSegNum,
			int inpIntReplNum, int intSegNum, String data_value_1,
			String data_value_2, String data_value_3, String data_value_4,
			int retraction_type) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbModInternalCompObj");
		}
		startSRBCommand(F_CMP_MOD_INT_COMP_OBJ, 11);

		sendArg(objID);
		sendArg(collectionName);
		sendArg(objReplNum);
		sendArg(objSegNum);
		sendArg(inpIntReplNum);
		sendArg(intSegNum);
		sendArg(data_value_1);
		sendArg(data_value_2);
		sendArg(data_value_3);
		sendArg(data_value_4);
		sendArg(retraction_type);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

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
	synchronized void srbModifyRescInfo(int catType, String resourceName,
			int actionType, String dataValue1, String dataValue2,
			String dataValue3, String dataValue4) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbModifyRescInfo");
		}
		startSRBCommand(F_MOD_RESC_INFO, 7);

		sendArg(catType);
		sendArg(resourceName);
		sendArg(actionType);
		sendArg(dataValue1);
		sendArg(dataValue2);
		sendArg(dataValue3);
		sendArg(dataValue4);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Register location information.
	 * 
	 * Input - String locName - The location name. String fullAddr - Full
	 * Address. String parentLoc - Parent location String serverUser - Server
	 * User String serverUserDomain - Server User Domain.
	 */
	synchronized void srbRegisterLocation(String locName, String fullAddr,
			String parentLoc, String serverUser, String serverUserDomain)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRegisterLocation");
		}
		startSRBCommand(F_REGISTER_LOCATION, 5);

		sendArg(locName);
		sendArg(fullAddr);
		sendArg(parentLoc);
		sendArg(serverUser);
		sendArg(serverUserDomain);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Ingest Token.
	 * 
	 * Input - String typeName - The type name. String newValue - The new value.
	 * String parentValue - Parent value.
	 */
	synchronized void srbIngestToken(String typeName, String newValue,
			String parentValue) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbIngestToken");
		}
		startSRBCommand(F_INGEST_TOKEN, 3);

		sendArg(typeName);
		sendArg(newValue);
		sendArg(parentValue);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Register Resource.
	 * 
	 * Input - String rescName - The resource name. String rescType - Resource
	 * type. String location - Location. String phyPath - Physical Path. String
	 * class - className. int size - size.
	 */
	synchronized void srbRegisterResource(String rescName, String rescType,
			String location, String phyPath, String className, int size)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRegisterResource");
		}
		startSRBCommand(F_REGISTER_RESOURCE, 6);

		sendArg(rescName);
		sendArg(rescType);
		sendArg(location);
		sendArg(phyPath);
		sendArg(className);
		sendArg(size);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Register Logical Resource.
	 * 
	 * Input - String rescName - The resource name. String rescType - Resource
	 * type. String phyResc - Physical resource. String phyPath - Physical path.
	 */
	synchronized void srbRegisterLogicalResource(String rescName,
			String rescType, String phyResc, String phyPath) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRegisterLogicalResource");
		}
		startSRBCommand(F_REGISTER_LOGICAL_RESOURCE, 4);

		sendArg(rescName);
		sendArg(rescType);
		sendArg(phyResc);
		sendArg(phyPath);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * srbRegisterReplicateResourceInfo
	 * 
	 * Input - String physicalRescName - The physical resource name. String
	 * rescType - Resource type. String oldLogicalRescName - old logical
	 * resource name. String indefaultPath - Indefault Path.
	 */
	synchronized void srbRegisterReplicateResourceInfo(String physicalRescName,
			String rescType, String oldLogicalRescName, String indefaultPath)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbRegisterReplicateResourceInfo");
		}
		startSRBCommand(F_REGISTER_REPLICATE_RESOURCE_INFO, 4);

		sendArg(physicalRescName);
		sendArg(rescType);
		sendArg(oldLogicalRescName);
		sendArg(indefaultPath);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Delete a Value; a single MCAT entry.
	 * 
	 * Input - int valueType - the value (token) type. String deleteValue - The
	 * value (name) that is being deleted.
	 */
	synchronized void srbDeleteValue(int valueType, String deleteValue)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbDeleteValue");
		}
		startSRBCommand(F_DELETE_VALUE, 2);

		sendArg(valueType);
		sendArg(deleteValue);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Register files in bulk.
	 * 
	 * @param catType
	 *            catalog type - 0 - MCAT
	 * @param bulkLoadFilePath
	 *            The path where the files will be loaded, a container or
	 *            temporary file.
	 * 
	 * @param recordList
	 *            the metadata describing the objects to be registered. for
	 *            transfer it will be converted into: mdasC_sql_result_struct
	 *            *myresult - The mdasC_sql_result_struct that contains the
	 *            objects to be registered. myresult->sqlresult[0].values should
	 *            contain dataNameList myresult->sqlresult[1].values should
	 *            contain collectionNameList myresult->sqlresult[2].values
	 *            should contain dataSizeList (in ascii) (I will perform atol)
	 *            myresult->sqlresult[3].values should contain offSetList (in
	 *            ascii) myresult->row_count should contain the number of
	 *            datsets to be registered.
	 */
	synchronized void srbBulkRegister(int catType, String containerName,
			SRBMetaDataRecordList[] rl) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbBulkRegister");
		}
		startSRBCommand(F_BULK_REGISTER, 3);

		sendArg(catType);
		sendArg(containerName);
		sendArg(rl);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
		status();
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
	 *            performed. NULL => the server for the current connect.
	 * @param portalFlag
	 *            The portal flag (see definition in srb.h). Valid flags are -
	 *            PORTAL_OFF, PORTAL_ON, PORTAL_STD_IN_OUT.
	 * 
	 * @return any byte stream output.
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	synchronized InputStream srbExecCommand(String command, String commandArgs,
			String hostAddress, int portalFlag, int fireWallMinPort,
			int fireWallMaxPort) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbExecCommand");
		}
		// the output of the proxy command is received on a different port
		// open a new ServerSocket to listen on
		ServerSocket serverSocket = new ServerSocket();

		if (fireWallMinPort >= 0) {
			do {
				if (fireWallMinPort > fireWallMaxPort) {
					throw new IOException(
							"The bind operation failed, all ports already bound.");
				}
				try {
					serverSocket = new ServerSocket(fireWallMinPort);
				} catch (IOException e) {
					if (SRBCommands.DEBUG > 0)
						e.printStackTrace();
				}
				fireWallMinPort++;
			} while (!serverSocket.isBound());
		} else {
			// use any free port
			serverSocket = new ServerSocket(0);
		}
		int localPort = serverSocket.getLocalPort();

		String localAddress = connection.getLocalAddress().getHostAddress();
		Socket receiveSocket = null;
		DataInputStream receiveIn = null;

		startSRBCommand(F_SRBO_PROXY_OPR, 12);

		sendArg(OPR_COMMAND);
		sendArg(portalFlag);
		sendArg(localPort);
		sendArg(0);
		sendArg(0);
		sendArg(command);
		sendArg(commandArgs);
		sendArg(hostAddress);
		sendArg(localAddress);
		sendArg("");
		sendArg("");
		sendArg("");
		flush();

		receiveSocket = serverSocket.accept();
		receiveIn = new DataInputStream(receiveSocket.getInputStream());

		commandStatus();
		returnInt(); // just checks for errors.
		return receiveIn;
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
	 *            performed. NULL => the server for the current connect.
	 * @param fileName
	 *            The SRB path to a file to perform proxy operation on.
	 * @param portalFlag
	 *            The portal flag (see definition in srb.h). Valid flags are -
	 *            PORTAL_OFF, PORTAL_ON, PORTAL_STD_IN_OUT.
	 * 
	 * @return any byte stream output.
	 * @throws IOException
	 *             If an IOException occurs.
	 */
	synchronized InputStream srbExecCommand(String command, String commandArgs,
			String hostAddress, String fileName, int portalFlag,
			int fireWallMinPort, int fireWallMaxPort) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbExecCommand");
		}
		// the output of the proxy command is received on a different port
		// open a new ServerSocket to listen on
		ServerSocket serverSocket = new ServerSocket();

		if (fireWallMinPort >= 0) {
			do {
				if (fireWallMinPort > fireWallMaxPort) {
					throw new IOException(
							"The bind operation failed, all ports already bound.");
				}
				try {
					serverSocket = new ServerSocket(fireWallMinPort);
				} catch (IOException e) {
					if (SRBCommands.DEBUG > 0)
						e.printStackTrace();
				}
				fireWallMinPort++;
			} while (!serverSocket.isBound());
		} else {
			// use any free port
			serverSocket = new ServerSocket(0);
		}
		int localPort = serverSocket.getLocalPort();
		int resolvePath = 1;

		String localAddress = connection.getLocalAddress().getHostAddress();
		Socket receiveSocket = null;
		DataInputStream receiveIn = null;

		startSRBCommand(F_SRBO_PROXY_OPR, 12);

		sendArg(OPR_COMMAND);
		sendArg(portalFlag);
		sendArg(localPort);
		sendArg(resolvePath);
		sendArg(0);
		sendArg(command);
		sendArg(commandArgs);
		sendArg(hostAddress);
		sendArg(localAddress);
		sendArg(fileName);
		sendArg("");
		sendArg("");
		flush();

		receiveSocket = serverSocket.accept();
		receiveIn = new DataInputStream(receiveSocket.getInputStream());

		commandStatus();
		returnInt(); // just checks for errors.
		return receiveIn;
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
	synchronized void srbSyncData(int catType, String objID,
			String collectionName, String resource) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbSyncData");
		}
		startSRBCommand(F_SRBO_SYNC_DATA, 4);

		sendArg(catType);
		sendArg(objID);
		sendArg(collectionName);
		sendArg(resource);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
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
	synchronized int srbContainerOpen(int catType, String containerName,
			int openFlag) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbContainerOpen");
		}
		startSRBCommand(F_CONTAINER_OPEN, 3);

		sendArg(catType);
		sendArg(containerName);
		sendArg(openFlag);
		flush();

		commandStatus();
		return returnInt();
	}

	/**
	 * Close an opened a container.
	 * 
	 * @param catType
	 *            - catalog type - 0 - MCAT
	 * @param confFd
	 *            - The fd returned from srbContainerOpen ().
	 */
	synchronized void srbContainerClose(int confFd) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbContainerClose");
		}
		startSRBCommand(F_CONTAINER_CLOSE, 1);

		sendArg(confFd);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
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
	synchronized long srbObjCopy(String srcObjID, String srcCollection,
			String destObjID, String destCollection, String destResource)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjCopy");
		}
		startSRBCommand(F_SRBO_COPY, 5);

		sendArg(srcObjID);
		sendArg(srcCollection);
		sendArg(destObjID);
		sendArg(destCollection);
		sendArg(destResource);
		flush();

		commandStatus();
		long status = returnLong();
		if (status < 0) {
			throw new SRBException("", (int) status);
		}
		return status;
	}

	/**
	 * Copy a dataset from local space to SRB space.
	 * 
	 * @param destObjID
	 *            - The destination objID.
	 * @param destCollection
	 *            - The destination collection.
	 * @param destResLoc
	 *            - The destination resource.
	 * @param dataType
	 *            - The data type.
	 * @param destPath
	 *            - The destination path name.
	 * @param localFilePath
	 *            - The local filepath.
	 * @param size
	 *            - The size of the file. negative means don't know.
	 * @return the number of bytes copied.
	 */

	/**
	 * The client initiated version of srbObjPut. Copy a dataset from local
	 * space to SRB space.
	 * 
	 * @param destObjID
	 *            The destination objID.
	 * @param destCollection
	 *            The destination collection.
	 * @param destResLoc
	 *            The destination resource.
	 * @param dataType
	 *            The data type.
	 * @param destPath
	 *            The destination path name.
	 * @param localFilePath
	 *            The local filepath.
	 * @param size
	 *            The size of the file. negative means don't know.
	 * @param forceFlag
	 *            over write flag
	 * @param numThreads
	 *            number of threads
	 * @return The number of bytes copied. Returns a negative value upon
	 *         failure.
	 */
	synchronized long srbObjPutClientInitiated(String destObjID,
			String destCollection, String destResLoc, String dataType,
			String destPath, String localFilePath, long srcSize, int forceFlag,
			int numThreads) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjPutClientInitiated" + " destObjID "
					+ destObjID + " destCollection " + destCollection
					+ " destResLoc " + destResLoc + " dataType " + dataType
					+ " destPath " + destPath + " localFilePath "
					+ localFilePath + " srcSize " + srcSize + " forceFlag "
					+ forceFlag + " numThreads " + numThreads);
		}

		totalBytesMoved = 0;
		bytesMoved = 0;
		byte[] temp = null;

		GeneralFile source = new LocalFile(localFilePath);
		// if the source doesn't exist create the destination.
		if (!source.exists()) {
			srbObjCreate(SRBFile.MDAS_CATALOG, destObjID, dataType, destResLoc,
					destCollection, "", 0);
		}
		GeneralRandomAccessFile raf = new LocalRandomAccessFile(localFilePath,
				"r");

		// if files with zero bytes are copied the method might hang
		if (srcSize <= 0) {
			srbObjCreate(0, destObjID, dataType, destResLoc, destCollection,
					"", 0);
			return 0;
		}

		startSRBCommand(F_SRBO_PUT_C, 8);

		sendArg(destObjID);
		sendArg(destCollection);
		sendArg(destResLoc);
		sendArg(dataType);
		sendArg(destPath);
		sendArg(srcSize);
		sendArg(forceFlag);
		sendArg(numThreads);
		flush();

		commandStatus();
		// returnedByteCount or error message
		int returnedByteCount = readInt();
		if (returnedByteCount < 0) {
			throw new SRBException(returnedByteCount);
		}

		// new portlist number or error (can never be a long)
		int newPortList = (int) readUnsignedInt();

		if (newPortList == NEW_PORTLIST) {
			TransferThread transfer = null;
			Thread transferThreads = null;

			int bytesRead = 0;
			int numberOfAddresses = (int) readUnsignedInt();
			int[] portNum = new int[numberOfAddresses];
			int[] cookie = new int[numberOfAddresses];
			String[] hostAddr = new String[numberOfAddresses];

			byte[] cookiePort = null;
			for (int i = 0; i < numberOfAddresses; i++) {
				cookiePort = read(4);
				cookie[i] = ((cookiePort[0] & 0xff) << 8)
						+ (cookiePort[1] & 0xff);
				portNum[i] = ((cookiePort[2] & 0xff) << 8)
						+ (cookiePort[3] & 0xff);
			}

			for (int i = 0; i < numberOfAddresses; i++) {
				hostAddr[i] = readString();
			}
			status();

			// must be sent all at once.
			byte[] cookieReturn = { 0, 0, cookiePort[0], cookiePort[1] };
			Socket[] transferSocket = new Socket[numThreads];
			for (int i = 0; i < numThreads; i++) {
				try {
					// Thread[] transferThreads = new Thread[MAX_THREADS+1];
					transferSocket[i] = new Socket(hostAddr[0], portNum[0]);
					transferSocket[i].getOutputStream().write(cookieReturn);
				} catch (ConnectException e) {
					// trouble with doing a number of copies in a row.
					e.printStackTrace();
					for (Socket s : transferSocket) {
						if (s != null && !s.isClosed()) {
							s.close();
						}
					}
					close();
					// Kind of redundant to send account twice...oh well
					connect(account, SRBFileSystem
							.createUserInfoBuffer(account));
					throw e;
				}
			}

			// in first return read:
			// 4 bytes for the operation, should equal 9...don't know what 9
			// means...
			temp = new byte[4];
			bytesRead = transferSocket[0].getInputStream().read(temp);
			while (bytesRead < 4) {
				if (bytesRead < 0)
					throw new ProtocolException();
				temp[bytesRead - 1] = (byte) transferSocket[0].getInputStream()
						.read();
			}

			// 4 bytes for number of threads
			bytesRead = transferSocket[0].getInputStream().read(temp);
			while (bytesRead < 4) {
				if (bytesRead < 0)
					throw new ProtocolException();
				temp[bytesRead - 1] = (byte) transferSocket[0].getInputStream()
						.read();
			}
			if (numThreads <= Host.castToInt(temp)) {
				numThreads = Host.castToInt(temp);
			} else {
				// The server really shouldn't ever make a request for more.
				throw new ProtocolException(
						"Server requires more ports from client for transfer");
			}

			// last 8 long total length
			temp = new byte[8];
			bytesRead = transferSocket[0].getInputStream().read(temp);
			while (bytesRead < 8) {
				if (bytesRead < 0)
					throw new ProtocolException();
				temp[bytesRead - 1] = (byte) transferSocket[0].getInputStream()
						.read();
			}
			if (srcSize != Host.castToLong(temp)) {
				// probably error?
				if (DEBUG > 0)
					System.err.println("Local file size " + srcSize
							+ " does not equal file size expected by server "
							+ Host.castToLong(temp));
			}

			for (int i = 0; i < numThreads; i++) {
				transfer = new TransferThread(new Object(), transferSocket[i],
						raf, srcSize, new Object(), i);
				transferThreads = new Thread(transfer);
				transferThreads.start();
			}

			try {
				if (transferThreads.isAlive())
					transferThreads.join();
			} catch (InterruptedException e) {
				if (DEBUG > 2) {
					System.err.println("Probably not an important error");
					e.printStackTrace();
				}
			}
			if (raf != null) {
				// garbage collector is too slow
				raf.close();
				raf = null;
			}

			commandStatus();
			int status = returnInt();
			if (status >= 0) {
				return status;
			} else {
				throw new SRBException(status);
			}
		} else if (newPortList < 0) {
			// if error
			throw new SRBException(newPortList);
		} else {// if (returnedByteCount != MSG_USE_SINGLE_PORT) {
			// return status in case it equals MSG_USE_SINGLE_PORT
			// have to use a different copy method, single port
			return returnedByteCount;
		}
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
	 * @return The number of bytes copied.
	 */
	synchronized long srbObjGetClientInitiated(String srcObjID,
			String srcCollection, GeneralFile destination, int flag,
			int numThreads, boolean forceOverwrite) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjGetClientInitiated " + srcObjID + " "
					+ srcCollection + " " + destination + " " + flag + " "
					+ numThreads);
		}

		totalBytesMoved = 0;
		bytesMoved = 0;

		startSRBCommand(F_SRBO_GET_C, 4);

		sendArg(srcObjID);
		sendArg(srcCollection);
		sendArg(flag);
		sendArg(numThreads);
		flush();

		commandStatus();

		// length of file to be copied
		long srcSize = 1;

		// returnedByteCount or error message
		int returnedByteCount = readInt();
		if (returnedByteCount < 0) {
			throw new SRBException(returnedByteCount);
		}

		// new portlist number or error message (can never be a long)
		int newPortList = (int) readUnsignedInt();

		while (newPortList == NEW_PORTLIST) {
			int numberOfAddresses = (int) readUnsignedInt();
			int[] portNum = new int[numberOfAddresses];
			int[] cookie = new int[numberOfAddresses];
			String[] hostAddr = new String[numberOfAddresses];
			byte[] cookiePort = null;

			Socket[] transferSocket = new Socket[numThreads];
			GeneralRandomAccessFile raf = FileFactory.newRandomAccessFile(
					destination, "rw");

			// some filesystems can't truncate...
			if ((forceOverwrite) && (destination.length() > srcSize)) {
				try {
					raf.setLength(srcSize);
				} catch (UnsupportedOperationException e) {
					// this should only happen to fileSystems that can't
					// truncate
					if (SRBCommands.DEBUG > 0)
						e.printStackTrace();
					destination.delete();
					raf = FileFactory.newRandomAccessFile(destination, "rw");
				} catch (IOException e) {
					// I guess they might throw IOException instead
					if (SRBCommands.DEBUG > 0)
						e.printStackTrace();
					destination.delete();
					raf = FileFactory.newRandomAccessFile(destination, "rw");
				}
			} else if (!forceOverwrite) {
				// if file already existed, append
				raf.seek(raf.length());
			}

			Thread[] transferThreads = new Thread[numThreads];
			TransferThread[] transfer = new TransferThread[numThreads];

			for (int i = 0; i < numberOfAddresses; i++) {
				cookiePort = read(4);
				cookie[i] = ((cookiePort[0] & 0xff) << 8)
						+ (cookiePort[1] & 0xff);
				portNum[i] = ((cookiePort[2] & 0xff) << 8)
						+ (cookiePort[3] & 0xff);
			}

			for (int i = 0; i < numberOfAddresses; i++) {
				hostAddr[i] = readString();
			}
			status();

			// cookie bytes
			byte[] cookieBytes = { 0, 0, cookiePort[0], cookiePort[1] };
			for (int i = 0; i < numThreads; i++) {
				transferSocket[i] = new Socket(hostAddr[0], portNum[0]);
				OutputStream outy = transferSocket[i].getOutputStream();

				// send the cookie back to verify authenticity
				outy.write(cookieBytes);

				if (i == 0) {
					DataInputStream inny = new DataInputStream(
							transferSocket[i].getInputStream());
					inny.readInt();// um?
					inny.readInt();// um? number of threads?
					srcSize = inny.readLong();
				}

				transfer[i] = new TransferThread(raf, transferSocket[i], raf,
						srcSize, new Object(), i);
				transferThreads[i] = new Thread(transfer[i]);
				transferThreads[i].start();
			}

			try {
				for (int i = 0; i < numThreads; i++) {
					if (transferThreads[i].isAlive()) {
						transferThreads[i].join();
					}
					if (transferSocket[i] != null) {
						// garbage collector is too slow
						transferSocket[i].close();
						transferSocket[i] = null;
					}
				}
			} catch (InterruptedException e) {

			}
			if (raf != null) {
				// garbage collector is too slow
				raf.close();
				raf = null;
			}

			commandStatus();
			newPortList = (int) readUnsignedInt();
		}

		if (newPortList < 0)
			throw new SRBException((int) newPortList);

		// read 4 bytes, can be an error message.
		int error = readInt();
		if (error < 0)
			throw new SRBException(error);

		status();

		if (DEBUG > 2) {
			System.err.println("srbObjGetClientInitiated bytesMoved: "
					+ bytesMoved);
		}

		return srcSize;
	}

	/**
	 * Provides status on file transfer, in the form of how many bytes have been
	 * transfer so far.
	 */
	long getBytesMoved() {
		return bytesMoved;
	}

	//
	// Thread support for parallel put and get, bulk (un)load
	//
	long totalBytesMoved = 0;
	long bytesMoved = 0;
	long srcLength;
	static final int MAX_THREADS = 32;
	static final int READ = 1;
	static final int WRITE = 2;
	static final int PUT = 3;
	static final int GET = 4;
	static final int DONE = 0;

	class TransferThread implements Runnable {
		private Socket transferSocket;
		private GeneralFile file;
		private GeneralRandomAccessFile raf;
		private long srcSize;
		private Object listener;

		private SRBMetaDataRecordList[] rl;
		private String directoryPath;
		private String sourcePath;

		private Object syncObject;
		long length;
		int whichThread;

		/**
		 * Used by client parallel transfer put and get
		 */
		TransferThread(Object syncObject, Socket transferSocket,
				GeneralFile file, long srcSize, Object listener, int threadID) {
			whichThread = threadID;
			this.transferSocket = transferSocket;
			this.file = file;
			this.srcSize = srcSize;
			this.listener = listener;

			this.syncObject = syncObject;
		}

		TransferThread(Object syncObject, Socket transferSocket,
				GeneralRandomAccessFile raf, long srcSize, Object listener,
				int threadID) {
			whichThread = threadID;
			this.transferSocket = transferSocket;
			this.raf = raf;
			this.srcSize = srcSize;
			this.listener = listener;

			this.syncObject = syncObject;
		}

		/**
		 * Used by bulk unload
		 */
		TransferThread(Socket transferSocket, SRBMetaDataRecordList[] rl,
				String targetDirectoryPath, String sourcePath)
				throws IOException {
			this.transferSocket = transferSocket;
			this.rl = rl;
			this.directoryPath = targetDirectoryPath;
			this.sourcePath = sourcePath;
			srcSize = -1;
			listener = null;

			raf = correctedRAFPath(0);
			length = Long.parseLong(rl[0].getValue(
					rl[0].getFieldIndex(SRBMetaDataSet.SIZE)).toString());
		}

		protected void finalize() throws Throwable {
			if (transferSocket != null) {
				transferSocket.close();
				transferSocket = null;
			}
			if (raf != null) {
				raf.close();
				raf = null;
			}

			super.finalize();
		}

		/**
     *
     */
		private GeneralRandomAccessFile correctedRAFPath(int i)
				throws IOException {
			// get SRB directory path from rl
			String dir = rl[i].getValue(SRBMetaDataSet.DIRECTORY_NAME)
					.toString();

			LocalFile subDir;
			// subtract SRB specific directoryPath
			if (!dir.equals(sourcePath)) {
				dir = dir.substring(sourcePath.length() + 1);

				// create proper necessary subdirectories
				subDir = new LocalFile(directoryPath, dir);
			} else {
				subDir = new LocalFile(directoryPath);
			}
			subDir.mkdirs();

			return FileFactory.newRandomAccessFile(new LocalFile(subDir, rl[i]
					.getValue(SRBMetaDataSet.FILE_NAME).toString()), "rw");
		}

		public void run() {
			try {
				DataInputStream in = new DataInputStream(transferSocket
						.getInputStream());
				OutputStream out = transferSocket.getOutputStream();

				int operation; // Whether PUT(=3) or GET(=4), etc.
				long offset; // Where to seek into the data
				long transferLength; // How much to read/write
				byte[] buffer = null; // Holds all the data for transfer
				int i = 0;

				int temp = 0;
				int maxReadSize = 1448;

				GeneralRandomAccessFile innerRAF = null;

				// read the header
				operation = in.readInt();
				if ((operation < DONE) || (operation > GET))
					throw new ProtocolException("Unknown transfer operation");
				else if (operation == PUT) {
					innerRAF = FileFactory.newRandomAccessFile(raf.getFile(),
							"r");
				} else if (operation == GET) {
					innerRAF = FileFactory.newRandomAccessFile(raf.getFile(),
							"rw");
				}

				transferLength = in.readInt();

				buffer = new byte[LONG_LENGTH];
				in.read(buffer);
				offset = Host.castToUnsignedLong(buffer);

				if (offset < 0)
					return;

				if (transferLength <= 0)
					return;
				else {
					// transferLength has a max of 8mb?
					buffer = new byte[(int) transferLength];
				}

				while (transferLength > 0) {
					switch (operation) {
					// case READ:
					case GET:
						if (listener == null) {
							// bulk unload method.
							int continueFlag = 1;
							long toRead;
							int BUF_SIZE = 8388608; // 8 Mb

							buffer = new byte[BUF_SIZE];
							while (continueFlag > 0) {
								int bytesInBuf = 0;

								for (i = 0; i < rl.length; i++) {
									while (length > 0) {
										if (transferLength <= 0) {
											byte[] mybuffer;
											// read the header
											operation = in.readInt();
											transferLength = in.readInt();
											mybuffer = new byte[LONG_LENGTH];
											in.read(mybuffer);
										}

										if (length > BUF_SIZE) {
											if (BUF_SIZE > transferLength) {
												toRead = transferLength;
											} else {
												toRead = BUF_SIZE;
											}
										} else {
											if (length > transferLength) {
												toRead = transferLength;
											} else {
												toRead = length;
											}
										}

										bytesInBuf = 0;
										while (toRead > 0) {
											int readSz;
											int len;

											if (toRead > maxReadSize) {
												readSz = maxReadSize;
											} else {
												readSz = (int) toRead;
											}

											len = in.read(buffer, bytesInBuf,
													readSz);
											if (len <= 0) {
												System.err
														.print("BulkUnload sock read error."
																+ readSz
																+ "bytes more to read"
																+ "\n");
												break;
											}
											toRead -= len;
											bytesInBuf += len;
										}

										innerRAF.write(buffer, (int) offset,
												bytesInBuf);
										length -= bytesInBuf;
										transferLength -= bytesInBuf;
										bytesMoved += bytesInBuf;
										bytesInBuf = 0;
									} // while length > 0

									innerRAF.close();

									// start new file. Shouldn't be here. Should
									// be at top
									if (i + 1 < rl.length) {
										length = Long
												.parseLong(rl[i + 1]
														.getValue(
																rl[i + 1]
																		.getFieldIndex(SRBMetaDataSet.SIZE))
														.toString());
										innerRAF = correctedRAFPath(i + 1);
									}
								} // for loop

								// continue ?
								if (!rl[rl.length - 1].isQueryComplete()) {
									rl = returnSRBMetaDataRecordList(true, in);

									if (rl != null) {
										// more data/files to read
										length = Long
												.parseLong(rl[0]
														.getValue(
																rl[0]
																		.getFieldIndex(SRBMetaDataSet.SIZE))
														.toString());

										if (rl == null) {
											// no more data to read
											continueFlag = 0;
											transferLength = 0;
											break;
										}
										innerRAF = correctedRAFPath(0); // index
										// 0
										// correct
										// ?

										in.read();// ignore status byte

										continueFlag = 1;
									} else {
										continueFlag = 0;
									}
									transferLength = 0;
								} else {
									continueFlag = 0;
									transferLength = 0;
									break;
								}
							} // while continueFlag
						} // listen
						else {
							temp = 0;

							if (transferLength > maxReadSize) {
								while (transferLength > (temp + maxReadSize - 1)
										&& (temp >= 0)) {
									temp += in.read(buffer, temp, maxReadSize);
								}
								while ((transferLength - temp) > 0
										&& (temp >= 0)) {
									temp += in.read(buffer, temp,
											((int) transferLength - temp));
								}
							} else {
								while ((transferLength - temp) > 0
										&& (temp >= 0)) {
									temp += in.read(buffer, temp,
											((int) transferLength - temp));
								}
							}
							bytesMoved += temp;
							temp = 0;

							if (innerRAF.getFilePointer() != offset) {
								innerRAF.seek(offset);
							}

							if (transferLength > 0) {
								innerRAF.write(buffer);
							}

							// read the header
							operation = in.readInt();
							if ((operation < DONE) || (operation > GET)
									&& (operation != 167772160)) {
								throw new ProtocolException(
										"Unknown transfer operation");
							}
							transferLength = in.readInt();

							buffer = new byte[LONG_LENGTH];
							in.read(buffer);
							offset = Host.castToUnsignedLong(buffer);

							if (offset < 0)
								break;

							if (transferLength <= 0)
								break;
							else if (transferLength >= Integer.MAX_VALUE)
								buffer = new byte[Integer.MAX_VALUE - 1];
							else
								buffer = new byte[(int) transferLength];
						}
						break;
					// case WRITE:
					case PUT:
						// read buffer loops until finished
						innerRAF.seek(offset);
						bytesMoved += innerRAF.read(buffer);
						out.write(buffer);

						// read the header
						operation = in.readInt();
						if ((operation < DONE) || (operation > GET))
							throw new ProtocolException(
									"Unknown transfer operation");

						transferLength = in.readInt();

						buffer = new byte[LONG_LENGTH];
						in.read(buffer);
						offset = Host.castToUnsignedLong(buffer);
						if (offset < 0)
							break;

						if (transferLength <= 0)
							break;
						else if (transferLength == buffer.length) {
							// don't do anything
						} else if (transferLength >= Integer.MAX_VALUE)
							buffer = new byte[Integer.MAX_VALUE - 1];
						else
							buffer = new byte[(int) transferLength];
						break;
					case DONE:
					default:

						// signal to the main thread if everyone is done
						transferLength = 0;
						break;
					}
				}

				// make sure not to copy more than total
				totalBytesMoved += bytesMoved;

				if ((totalBytesMoved >= srcSize) && (listener != null)
						&& (listener instanceof ServerSocket)) {
					((ServerSocket) listener).close();
				}

				if (innerRAF != null) {
					innerRAF.close();
					innerRAF = null;
				}
			} catch (SocketException e) {
				if (out != null) {
					try {
						out.close();
					} catch (IOException x) {
						x.initCause(e);
						throw new RuntimeException("IOException in thread.", x);
					}
					out = null;
				}
				if (in != null) {
					try {
						in.close();
					} catch (IOException x) {
						x.initCause(e);
						throw new RuntimeException("IOException in thread.", x);
					}
					in = null;
				}
				if (transferSocket != null) {
					try {
						// garbage collector is too slow
						transferSocket.close();
					} catch (IOException x) {
						x.initCause(e);
						throw new RuntimeException("IOException in thread.", x);
					}
					transferSocket = null;
				}
				if (SRBCommands.DEBUG > 0)
					e.printStackTrace();
			} catch (IOException e) {
				if (SRBCommands.DEBUG > 0)
					e.printStackTrace();
				throw new RuntimeException("IOException in thread.", e);
			}
		}
	}

	/**
	 * Get the mcatName of this user.
	 * 
	 * @param userName
	 *            - The input userName.
	 * @param domainName
	 *            - The input domainName.
	 * @param mcatName
	 *            - The output mcatname. It should be preallocated with length
	 *            SRBFileSystem.MAX_TOKEN.
	 * 
	 *            the CALLER must have allocated enough space to hold the result
	 *            returned
	 */
	synchronized void srbGetMcatZone(String userName, String domainName,
			String mcatName) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbGetMcatZone");
		}
		startSRBCommand(F_GET_MCAT_NAME, 2);

		sendArg(userName);
		sendArg(domainName);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Get the MCAT-enabled server's public key in preparation for transferring
	 * encryptioned information. Also see the sscSetupSessionPublicKey library
	 * routine.
	 * 
	 * @param publicKey
	 *            a string representation of the current public key, If
	 *            server-side secure communication is not supported, publicKey
	 *            will instead contain an error message string.
	 */
	synchronized String srbSetupSessionPublicKey() throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbSetupSessionPublicKey");
		}
		startSRBCommand(F_SETUP_SESSION_PUBLIC_KEY, 0);

		flush();
		commandStatus();
		long length = readLong();
		String key = new String(read((int) length - INT_LENGTH));
		status();
		return key;
	}

	/**
	 * Set up a session (for encryption) with the MCAT-enabled server. Also see
	 * the sscSetupSession library routine.
	 * 
	 * @param sessionKey
	 *            a string representation of the session key from
	 *            sscSetupSession (encrypted in the public key)
	 */
	synchronized void srbSetupSession(String sessionKey) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbSetupSession");
		}
		startSRBCommand(F_SETUP_SESSION, 1);

		sendArg(sessionKey);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Bulk load a set of data. All the actual data should already be contained
	 * in bloadFullPath, but in concatenated form just as in the case of a
	 * container.
	 * 
	 * @param catType
	 *            catalog type, 0 = MCAT
	 * @param bloadFullPath
	 *            The name of the container.
	 * @param rl
	 *            The list of the objects to be registered. It should contain
	 *            dataNameList, collectionNameList, dataSizeList (in ascii)(SRB
	 *            will perform atol), offSetList (in ascii), the number of
	 *            datsets to be registered.
	 */
	synchronized void srbBulkLoad(int catType, String bulkLoadFilePath,
			SRBMetaDataRecordList[] rl) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbBulkLoad");
		}
		startSRBCommand(F_BULK_LOAD, 3);

		sendArg(catType);
		sendArg(bulkLoadFilePath);
		sendArg(rl);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * The client initiated socket conection version of Bulk unload of
	 * non-container files in a collection recursively.
	 * 
	 * @param catType
	 *            catalog type, 0 = MCAT
	 * @param flag
	 *            flag value for future use.
	 * @param collection
	 *            The collection to unload
	 * @return
	 */
	synchronized void srbBulkUnload(int catType, int flag,
			String srbUnloadDirPath, String localDirPath) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbBulkUnload");
		}

		// reset, from previous uses
		totalBytesMoved = 0;
		bytesMoved = 0;

		int status = 0;

		Socket[] bulkConn = null;
		InputStream[] bulkIn = null;
		OutputStream[] bulkOut = null;
		SRBMetaDataRecordList[] rl = null;

		int numAddr = 1;

		int[] cookie = null;
		int[] portNum = null;
		byte[] temp = null;
		byte[] temp2 = null;

		String hostAddress = null;

		InputStream tempIn = in;
		TransferThread transfer[] = null;
		Thread transferThread[] = null;

		// 1st, inform the SRB of the collection to be downloaded
		startSRBCommand(F_BULK_UNLOAD_C, 3);

		sendArg(catType);
		sendArg(flag);
		sendArg(srbUnloadDirPath);
		flush();
		commandStatus();

		// packet length
		status = readInt();
		// Probably how many bytes to read,
		// but I just assume it says 4 because it should, and sometimes doesn't.
		temp = read(4);
		status = Host.castToInt(temp);
		if (status == MSG_USE_SINGLE_PORT) {
			// needed during the return with single server port protocol
			singlePortBulkUnload = true;
			numAddr = 1;
			status();
		} else if (status < 0) {
			throw new SRBException(status);
		} else {
			// returns the struct port_t
			// which is used to connect to the new location for the bulk unload
			numAddr = Host.castToInt(read(INT_LENGTH));

			cookie = new int[numAddr];
			portNum = new int[numAddr];
			temp = new byte[INT_LENGTH];
			temp2 = new byte[INT_LENGTH];

			for (int i = 0; i < numAddr; i++) {
				temp[2] = read();
				temp[3] = read();

				cookie[i] = Host.castToInt(temp);

				temp2[2] = read();
				temp2[3] = read();

				portNum[i] = Host.castToInt(temp2);
			}
			hostAddress = readString();
			status();

			// SRB only ever makes one connection for now
			// Open the new direct bulk unload connection(s)
			bulkConn = new Socket[numAddr];
			bulkIn = new InputStream[numAddr];
			bulkOut = new OutputStream[numAddr];
			for (int i = 0; i < numAddr; i++) {
				try {
					bulkConn[i] = new Socket(hostAddress, portNum[i]);
					bulkIn[i] = bulkConn[i].getInputStream();
					bulkOut[i] = bulkConn[i].getOutputStream();
				} catch (ConnectException e) {
					ConnectException connException = new ConnectException(
							"Bulk download connection cannot be made to: "
									+ hostAddress + " at port: " + portNum[i]);
					connException.initCause(e);
					throw connException;
				} catch (SocketException e) {
					SocketException socketException = new SocketException(
							"A bulk download socket error occured when connecting to: "
									+ hostAddress + " at port: " + portNum[i]);
					socketException.initCause(e);
					throw socketException;
				}
			}
		}

		transfer = new TransferThread[numAddr];
		transferThread = new Thread[numAddr];

		// The protocol changes slightly for single socket server connections
		if (status == MSG_USE_SINGLE_PORT) {
			rl = returnSRBMetaDataRecordList(true, null);

			if (rl != null) {
				// Just one thread
				transfer[0] = new TransferThread(connection, rl, localDirPath,
						srbUnloadDirPath);
				transferThread[0] = new Thread(transfer[0]);
				transferThread[0].start();
			}
		} else {
			tempIn = in;
			for (int i = 0; i < numAddr; i++) {
				bulkOut[i].write(temp);

				in = bulkIn[i];
				// 4bytes, junk I don't need
				read(4);
				rl = returnSRBMetaDataRecordList(true, null);
				// er
				in = tempIn;

				// Manages new threads the server initiates
				transfer[i] = new TransferThread(bulkConn[i], rl, localDirPath,
						srbUnloadDirPath);
				transferThread[i] = new Thread(transfer[i]);
				transferThread[i].start();
			}
		}

		try {
			for (int i = 0; i < transferThread.length; i++) {
				if (transferThread[i] != null) {
					if (transferThread[i].isAlive()) {
						transferThread[i].join();
					}
				}
			}
		} catch (InterruptedException e) {
			if (SRBCommands.DEBUG > 0)
				e.printStackTrace();
		}

		// reset
		if (singlePortBulkUnload)
			singlePortBulkUnload = false;
	}

	/**
	 * Modify and Insert SRB zone and zone information in the Metadata Catalog.
	 * Information about the operation performed is also logged in the audit
	 * trail (if turned on). This is a privileged function and should be called
	 * only by a srbAdmin user.
	 * 
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
	 * 
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
	synchronized void srbModifyZone(int catType, String zoneName,
			String dataValue1, String dataValue2, String dataValue3,
			String dataValue4, String dataValue5, int actionType)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbModifyZone");
		}
		startSRBCommand(F_MODIFY_ZONE, 8);

		sendArg(catType);
		sendArg(zoneName);
		sendArg(dataValue1);
		sendArg(dataValue2);
		sendArg(dataValue3);
		sendArg(dataValue4);
		sendArg(dataValue5);
		sendArg(actionType);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * srbBulkQueryAnswer - Get answers for canned queries from MCAT.
	 * 
	 * @param catType
	 *            - catalog type - 0 - MCAT
	 * @param queryInfo
	 *            - query information all info needed to perform the query is in
	 *            this string. The semantics for the string depends upon the
	 *            operation.
	 * @param rowsWanted
	 *            - number of rows of result wanted.
	 * 
	 * @return the query result, metadata record list.
	 */
	synchronized MetaDataRecordList[] srbBulkQueryAnswer(int catType,
			String queryInfo, MetaDataRecordList myresult, int rowsWanted)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbBulkQueryAnswer");
		}
		startSRBCommand(F_BULK_QUERY_ANSWER, 3);

		sendArg(catType);
		sendArg(queryInfo);
		sendArg(rowsWanted);
		flush();

		commandStatus();
		return returnSRBMetaDataRecordList(false, null);

	}

	/**
	 * srbBulkMcatIngest - Bulk ingestion of a set of metadata in to the
	 * SRB-MCAT system.
	 * 
	 * @param catType
	 *            - catalog type, 0 = MCAT
	 * @param ingestInfo
	 *            - information about what is done. All info needed to perform
	 *            the ingestion is in this string. The semantics for the string
	 *            depends upon the operation.
	 * @param The
	 *            MetaDataRecordList that contains the objects to be ingested.
	 */
	synchronized void srbBulkMcatIngest(int catType, String ingestInfo,
			SRBMetaDataRecordList[] rl) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbBulkMcatIngest");
		}
		startSRBCommand(F_BULK_MCAT_INGEST, 3);

		sendArg(catType);
		sendArg(ingestInfo);
		sendArg(rl);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	// SRB 3.0.2
	/**
	 * Backup a data object - Make a replica to the backup resource. Skip it if
	 * a good copy already exist.
	 * 
	 * @param catType
	 *            - catalog type - 0 - MCAT
	 * @param objID
	 *            The SRB object ID to Sync.
	 * @param collectionName
	 *            The name of the collection this objID belongs.
	 * @param backupResource
	 *            - The backup resource
	 * @param flag
	 *            - not used.
	 */
	synchronized void srbBackupData(int catType, String objID,
			String collectionName, String backupResource, int flag)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbBackupData");
		}
		startSRBCommand(F_BACKUP_DATA, 5);

		sendArg(catType);
		sendArg(objID);
		sendArg(collectionName);
		sendArg(backupResource);
		sendArg(flag);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	// SRB 3.1
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
	synchronized byte[] srbObjChksum(String objID, String collectionName,
			int chksumFlag, String inpChksum) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjChksum");
		}
		startSRBCommand(F_SRBO_CHKSUM, 4);

		sendArg(objID);
		sendArg(collectionName);
		sendArg(chksumFlag);
		sendArg(inpChksum);
		flush();

		commandStatus();

		int length = readInt();
		if (length < 0) {
			throw new SRBException(length);
		}

		byte[] checksum = null;
		byte[] temp = read(length);
		status();

		// don't know what the first 4 bytes are about
		for (int i = 4; i < length; i++) {
			if (temp[i] == 0) {
				checksum = new byte[i - 4];
				System.arraycopy(temp, 4, checksum, 0, i - 4);
				i = length;
			}
		}

		return checksum;
	}

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
	synchronized void srbModifyUserNonPriv(int catType, String userNameDomain,
			String dataValue1, String dataValue2, String dataValue3,
			String dataValue4, String dataValue5, int actionType)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbModifyUserNonPriv");
		}
		startSRBCommand(F_MODIFY_USER_NP, 8);

		sendArg(catType);
		sendArg(userNameDomain);
		sendArg(dataValue1);
		sendArg(dataValue2);
		sendArg(dataValue3);
		sendArg(dataValue4);
		sendArg(dataValue5);
		sendArg(actionType);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

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
	synchronized void srbModifyResource(int catType, String resourceName,
			String dataValue1, String dataValue2, String dataValue3,
			String dataValue4, int actionType) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbModifyResource");
		}
		startSRBCommand(F_MODIFY_RESOURCE, 7);

		sendArg(catType);
		sendArg(resourceName);
		sendArg(dataValue1);
		sendArg(dataValue2);
		sendArg(dataValue3);
		sendArg(dataValue4);
		sendArg(actionType);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}

	/**
	 * Get the Server version
	 * 
	 * @return the status.
	 */
	String srbGetSvrVersion() throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbGetSvrVersion");
		}

		int STRING_LEN1 = 512;
		byte[] buffer = new byte[1];
		String version = "";

		startSRBCommand(F_GET_SVR_VER, 0);
		flush();

		commandStatus();
		// Some junk
		read(8);
		buffer = read(1);
		while (buffer[0] != 0) {
			version += (char) buffer[0];
			buffer = read(1);
		}
		status();

		return version;
	}

	/**
	 * Get the zone:user@domain given the user's dn
	 * 
	 * @param catType
	 *            catalog type. e,g., MDAS_CATALOG.
	 * @param flag
	 *            not used
	 * @param userDn
	 *            The Dn of the user
	 * @return output for zone:user@domain
	 */
	String srbGetUserByDn(int catType, int flag, String userDn)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbGetUserByDn");
		}

		startSRBCommand(F_GET_USER_BY_DN, 3);

		sendArg(catType);
		sendArg(flag);
		sendArg(userDn);
		flush();

		commandStatus();
		try {
			byte[] dn = returnBytes();
			if (dn != null) {
				return new String(dn);
			}
		} catch (SRBException e) {
			if (e.getType() == -3005) {
				return null;
			}

			throw e;
		}

		return null;
	}

	/**
	 * Execute Procedure on srbObject
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
	byte[] srbObjProc(int fd, String procedureName, String input,
			int outputLength) throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbObjProc");
		}

		startSRBCommand(F_SRBO_PROC, 4);

		sendArg(fd);
		sendArg(outputLength);
		sendArg(procedureName);
		sendArg(input);
		flush();

		commandStatus();
		return returnBytes();
	}

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
	void srbModifyExtMetaData(int catType, String objID, String collectionName,
			String dataValue1, String dataValue2, String dataValue3,
			String dataValue4, String dataValue5, int retractionType)
			throws IOException {
		if (DEBUG > 0) {
			date = new Date().getTime();
			System.err.println("\n srbModifyExtMetaData");
		}
		startSRBCommand(F_MODIFY_EXT_META_DATA, 9);
		// SmodE -i dataName collName tableName insertAttrList
		// insertAttrValueList [insertAttrValueList ...]\n");
		// 0 1 2 3 4 5

		sendArg(catType);
		sendArg(objID);
		sendArg(collectionName);
		sendArg(dataValue1);
		sendArg(dataValue2);
		sendArg(dataValue3);
		sendArg(dataValue4);
		sendArg(dataValue5);
		sendArg(retractionType);
		flush();

		commandStatus();
		int status = returnInt();
		if (status < 0) {
			throw new SRBException(status);
		}
	}
}
