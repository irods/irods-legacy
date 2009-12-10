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
//  IRODSCommands.java  -  edu.sdsc.grid.io.irods.IRODSCommands
//
//  CLASS HIERARCHY
//  java.lang.Object
//     |
//     +-.IRODSCommands
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.irods;

import edu.sdsc.grid.io.*;
import edu.sdsc.grid.io.local.*;

import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.ConnectException;
import java.net.ProtocolException;
import java.net.SocketException;
import java.net.URI;
import java.nio.channels.ClosedChannelException;
import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InterruptedIOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.TreeMap;
import java.util.Date;
import java.util.Vector;

import java.security.MessageDigest;
import java.security.GeneralSecurityException;

/**
 * Instances of this class support socket I/O to a iRODS server.
 *<P>
 * Handles socket level protocol for interacting with the iRODS.
 *
 * <P>
 *
 * @author Lucas Gilbert, San Diego Supercomputer Center
 * @since JARGON2.0
 */
class IRODSCommands {
	/**
	 * A positive debug value turns on debugging. Higher values turn on more,
	 * maybe.
	 */
	private static int DEBUG = 0;
	private boolean extraDebug = false;
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
	 * Approximate maximum number of bytes transfered by each thread during a
	 * parallel transfer.
	 */
	static final int TRANSFER_THREAD_SIZE = 6000000;

	static final int SYS_CLI_TO_SVR_COLL_STAT_REPLY = 99999997;
	static final int SYS_CLI_TO_SVR_COLL_STAT_SIZE = 10;

	/**
	 * Maximum threads to open for a parallel transfer. More than this usually
	 * won't help, might even be slower.
	 */
	static final int MAX_THREAD_NUMBER = 16;

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
	 * Size of the socket send buffer
	 */
	static int OUTPUT_BUFFER_LENGTH = GeneralFileSystem.getWriteBufferSize();

	/**
	 * 4 bytes at the front of the header, outside XML
	 */
	static final int HEADER_INT_LENGTH = 4;

	/**
	 * Maximum password length. Used in challenge response.
	 */
	static final int MAX_PASSWORD_LENGTH = 50;
	/**
	 * Standard challenge length. Used in challenge response.
	 */
	static final int CHALLENGE_LENGTH = 64;

	/**
	 * Max number of SQL attributes a query can return?
	 */
	static final int MAX_SQL_ATTR = 50;
	
	// Various iRODS message types, in include/rodsDef.h
	static final String RODS_CONNECT = "RODS_CONNECT";
	static final String RODS_VERSION = "RODS_VERSION";
	static final String RODS_API_REQ = "RODS_API_REQ";
	static final String RODS_DISCONNECT = "RODS_DISCONNECT";
	static final String RODS_REAUTH = "RODS_REAUTH";
	static final String RODS_API_REPLY = "RODS_API_REPLY";

	// Various iRODS message types, in include/api/dataObjInpOut.h
	// definition for oprType in dataObjInp_t, portalOpr_t and l1desc_t
	static final int DONE_OPR = 9999;
	static final int PUT_OPR = 1;
	static final int GET_OPR = 2;
	static final int SAME_HOST_COPY_OPR = 3;
	static final int COPY_TO_LOCAL_OPR = 4;
	static final int COPY_TO_REM_OPR = 5;
	static final int REPLICATE_OPR = 6;
	static final int REPLICATE_DEST = 7;
	static final int REPLICATE_SRC = 8;
	static final int COPY_DEST = 9;
	static final int COPY_SRC = 10;
	static final int RENAME_DATA_OBJ = 11;
	static final int RENAME_COLL = 12;
	static final int MOVE_OPR = 13;
	static final int RSYNC_OPR = 14;
	static final int PHYMV_OPR = 15;
	static final int PHYMV_SRC = 16;
	static final int PHYMV_DEST = 17;

	/* from apiNumber.h - header file for API number assignment */
	/* 500 - 599 - Internal File I/O API calls */
	static final int FILE_CREATE_AN = 500;
	static final int FILE_OPEN_AN = 501;
	static final int FILE_WRITE_AN = 502;
	static final int FILE_CLOSE_AN = 503;
	static final int FILE_LSEEK_AN = 504;
	static final int FILE_READ_AN = 505;
	static final int FILE_UNLINK_AN = 506;
	static final int FILE_MKDIR_AN = 507;
	static final int FILE_CHMOD_AN = 508;
	static final int FILE_RMDIR_AN = 509;
	static final int FILE_STAT_AN = 510;
	static final int FILE_FSTAT_AN = 511;
	static final int FILE_FSYNC_AN = 512;
	static final int FILE_STAGE_AN = 513;
	static final int FILE_GET_FS_FREE_SPACE_AN = 514;
	static final int FILE_OPENDIR_AN = 515;
	static final int FILE_CLOSEDIR_AN = 516;
	static final int FILE_READDIR_AN = 517;
	static final int FILE_PUT_AN = 518;
	static final int FILE_CHKSUM_AN = 520;
	static final int CHK_N_V_PATH_PERM_AN = 521;
	static final int FILE_RENAME_AN = 522;
	static final int FILE_TRUNCATE_AN = 523;
	static final int FILE_STAGE_TO_CACHE_AN = 524;
	static final int FILE_SYNC_TO_ARCH_AN = 525;

	/* 600 - 699 - Object File I/O API calls */
	static final int DATA_OBJ_CREATE_AN = 601;
	static final int DATA_OBJ_OPEN_AN = 602;
	static final int DATA_OBJ_READ_AN = 603;
	static final int DATA_OBJ_WRITE_AN = 604;
	static final int DATA_OBJ_CLOSE_AN = 605;
	static final int DATA_OBJ_PUT_AN = 606;
	static final int DATA_PUT_AN = 607;
	static final int DATA_OBJ_GET_AN = 608;
	static final int DATA_GET_AN = 609;
	static final int DATA_OBJ_REPL_AN = 610;
	static final int DATA_COPY_AN = 611;
	static final int DATA_OBJ_LSEEK_AN = 612;
	static final int DATA_OBJ_COPY_AN = 613;
	static final int SIMPLE_QUERY_AN = 614;
	static final int DATA_OBJ_UNLINK_AN = 615;
	static final int COLL_CREATE_AN = 616;
	static final int REG_DATA_OBJ_AN = 619;
	static final int UNREG_DATA_OBJ_AN = 620;
	static final int REG_REPLICA_AN = 621;
	static final int MOD_DATA_OBJ_META_AN = 622;
	static final int RULE_EXEC_SUBMIT_AN = 623;
	static final int RULE_EXEC_DEL_AN = 624;
	static final int EXEC_MY_RULE_AN = 625;
	static final int OPR_COMPLETE_AN = 626;
	static final int DATA_OBJ_RENAME_AN = 627;
	static final int DATA_OBJ_RSYNC_AN = 628;
	static final int DATA_OBJ_CHKSUM_AN = 629;
	static final int PHY_PATH_REG_AN = 630;
	static final int DATA_OBJ_PHYMV_AN = 631;
	static final int DATA_OBJ_TRIM_AN = 632;
	static final int OBJ_STAT_AN = 633;
	static final int EXEC_CMD_AN = 634;
	static final int SUB_STRUCT_FILE_CREATE_AN = 635;
	static final int SUB_STRUCT_FILE_OPEN_AN = 636;
	static final int SUB_STRUCT_FILE_READ_AN = 637;
	static final int SUB_STRUCT_FILE_WRITE_AN = 638;
	static final int SUB_STRUCT_FILE_CLOSE_AN = 639;
	static final int SUB_STRUCT_FILE_UNLINK_AN = 640;
	static final int SUB_STRUCT_FILE_STAT_AN = 641;
	static final int SUB_STRUCT_FILE_FSTAT_AN = 642;
	static final int SUB_STRUCT_FILE_LSEEK_AN = 643;
	static final int SUB_STRUCT_FILE_RENAME_AN = 644;
	static final int QUERY_SPEC_COLL_AN = 645;
	static final int SUB_STRUCT_FILE_MKDIR_AN = 647;
	static final int SUB_STRUCT_FILE_RMDIR_AN = 648;
	static final int SUB_STRUCT_FILE_OPENDIR_AN = 649;
	static final int SUB_STRUCT_FILE_READDIR_AN = 650;
	static final int SUB_STRUCT_FILE_CLOSEDIR_AN = 651;
	static final int DATA_OBJ_TRUNCATE_AN = 652;
	static final int SUB_STRUCT_FILE_TRUNCATE_AN = 653;
	static final int GET_XMSG_TICKET_AN = 654;
	static final int SEND_XMSG_AN = 655;
	static final int RCV_XMSG_AN = 656;
	static final int SUB_STRUCT_FILE_GET_AN = 657;
	static final int SUB_STRUCT_FILE_PUT_AN = 658;
	static final int SYNC_MOUNTED_COLL_AN = 659;
	static final int STRUCT_FILE_SYNC_AN = 660;
	static final int CLOSE_COLLECTION_AN = 661;
	static final int RM_COLL_AN = 663;
	static final int STRUCT_FILE_EXTRACT_AN = 664;
	static final int STRUCT_FILE_EXT_AND_REG_AN = 665;
	static final int STRUCT_FILE_BUNDLE_AN = 666;
	static final int CHK_OBJ_PERM_AND_STAT_AN = 667;
	static final int GET_REMOTE_ZONE_RESC_AN = 668;
	static final int DATA_OBJ_OPEN_AND_STAT_AN = 669;
	static final int L3_FILE_GET_SINGLE_BUF_AN = 670;
	static final int L3_FILE_PUT_SINGLE_BUF_AN = 671;
	static final int DATA_OBJ_CREATE_AND_STAT_AN = 672;

	static final int COLL_REPL_AN = 677;
	static final int OPEN_COLLECTION_AN = 678;
	static final int MOD_COLL_AN = 680;
	static final int RM_COLL_OLD_AN = 682;
	static final int REG_COLL_AN = 683;

	/* 700 - 799 - Metadata API calls */
	static final int GET_MISC_SVR_INFO_AN = 700;
	static final int GENERAL_ADMIN_AN = 701;
	static final int GEN_QUERY_AN = 702;
	static final int AUTH_REQUEST_AN = 703;
	static final int AUTH_RESPONSE_AN = 704;
	static final int AUTH_CHECK_AN = 705;
	static final int MOD_AVU_METADATA_AN = 706;
	static final int MOD_ACCESS_CONTROL_AN = 707;
	static final int RULE_EXEC_MOD_AN = 708;
	static final int GET_TEMP_PASSWORD_AN = 709;
	static final int GENERAL_UPDATE_AN = 710;
	static final int GSI_AUTH_REQUEST_AN = 711;
	static final int READ_COLLECTION_AN = 713;
	static final int USER_ADMIN_AN = 714;
	static final int GENERAL_ROW_INSERT_AN = 715;
	static final int GENERAL_ROW_PURGE_AN = 716;
	static final int KRB_AUTH_REQUEST_AN = 717;

	// iRODS communication types
	static final char OPEN_START_TAG = Tag.OPEN_START_TAG;
	static final char CLOSE_START_TAG = Tag.CLOSE_START_TAG;
	static final String OPEN_END_TAG = Tag.OPEN_END_TAG;
	static final char CLOSE_END_TAG = Tag.CLOSE_END_TAG;

	// typical header tags
	static final String type = "type";
	static final String msgLen = "msgLen";
	static final String errorLen = "errorLen";
	static final String bsLen = "bsLen";
	static final String intInfo = "intInfo";

	// leaf tags
	static final String irodsProt = "irodsProt";
	static final String reconnFlag = "reconnFlag";
	static final String connectCnt = "connectCnt";
	static final String proxyUser = "proxyUser";
	static final String proxyRcatZone = "proxyRcatZone";
	static final String clientUser = "clientUser";
	static final String clientRcatZone = "clientRcatZone";
	static final String relVersion = "relVersion";
	static final String apiVersion = "apiVersion";
	static final String option = "option";
	static final String status = "status";
	static final String challenge = "challenge";
	static final String response = "response";
	static final String username = "username";
	static final String objPath = "objPath";
	static final String createMode = "createMode";
	static final String openFlags = "openFlags";
	static final String offset = "offset";
	static final String dataSize = "dataSize";
	static final String numThreads = "numThreads";
	static final String oprType = "oprType";
	static final String ssLen = "ssLen";
	static final String objSize = "objSize";
	static final String objType = "objType";
	static final String numCopies = "numCopies";
	static final String dataId = "dataId";
	static final String chksum = "chksum";
	static final String ownerName = "ownerName";
	static final String ownerZone = "ownerZone";
	static final String createTime = "createTime";
	static final String modifyTime = "modifyTime";
	static final String inx = "inx";
	static final String maxRows = "maxRows";
	static final String continueInx = "continueInx";
	static final String partialStartIndex = "partialStartIndex";
	static final String ivalue = "ivalue";
	static final String svalue = "svalue";
	static final String iiLen = "iiLen";
	static final String isLen = "isLen";
	static final String keyWord = "keyWord";
	static final String rowCnt = "rowCnt";
	static final String attriCnt = "attriCnt";
	static final String attriInx = "attriInx";
	static final String reslen = "reslen";
	static final String queryValue = "value";
	static final String collName = "collName";
	static final String recursiveFlag = "recursiveFlag";
	static final String accessLevel = "accessLevel";
	static final String userName = "userName";
	static final String zone = "zone";
	static final String path = "path";
	static final String l1descInx = "l1descInx";
	static final String len = "len";
	static final String fileInx = "fileInx";
	static final String whence = "whence";
	static final String dataObjInx = "dataObjInx";
	static final String bytesWritten = "bytesWritten";
	static final String msg = "msg";
	static final String myRule = "myRule";
	static final String outParamDesc = "outParamDesc";
	static final String hostAddr = "hostAddr";
	static final String rodsZone = "rodsZone";
	static final String port = "port";
	static final String ServerDN = "ServerDN";
	static final String flags = "flags";
	static final String collection = "collection";
	static final String cmd = "cmd";
	static final String cmdArgv = "cmdArgv";
	static final String execAddr = "execAddr";
	static final String hintPath = "hintPath";
	static final String addPathToArgv = "addPathToArgv";
	static final String options = "options";
	static final String portNum = "portNum";
	static final String cookie = "cookie";
	static final String buflen = "buflen";
	static final String buf = "buf";

	// Complex tags
	static final String CollOprStat_PI = "CollOprStat_PI";
	static final String MsgHeader_PI = "MsgHeader_PI";
	static final String StartupPack_PI = "StartupPack_PI";
	static final String Version_PI = "Version_PI";

	static final String authRequestOut_PI = "authRequestOut_PI";
	static final String authResponseInp_PI = "authResponseInp_PI";

	static final String gsiAuthRequestOut_PI = "gsiAuthRequestOut_PI";

	static final String DataObjInp_PI = "DataObjInp_PI";
	static final String GenQueryInp_PI = "GenQueryInp_PI";
	static final String ModAVUMetadataInp_PI = "ModAVUMetadataInp_PI";
	static final String InxIvalPair_PI = "InxIvalPair_PI";
	static final String InxValPair_PI = "InxValPair_PI";
	static final String KeyValPair_PI = "KeyValPair_PI";
	static final String RodsObjStat_PI = "RodsObjStat_PI";
	static final String SqlResult_PI = "SqlResult_PI";
	static final String DataObjCopyInp_PI = "DataObjCopyInp_PI";
	static final String ExecCmd_PI = "ExecCmd_PI";
	static final String PortList_PI = "PortList_PI";
	static final String StructFileExtAndRegInp_PI = "StructFileExtAndRegInp_PI";

	static final String CollInp_PI = "CollInp_PI";
	// new function after iRODS201
	static final String CollInpNew_PI = "CollInpNew_PI";

	static final String modAccessControlInp_PI = "modAccessControlInp_PI";
	static final String dataObjReadInp_PI = "dataObjReadInp_PI";
	static final String dataObjWriteInp_PI = "dataObjWriteInp_PI";
	static final String fileLseekInp_PI = "fileLseekInp_PI";
	static final String dataObjCloseInp_PI = "dataObjCloseInp_PI";
	static final String BinBytesBuf_PI = "BinBytesBuf_PI";

	static final String RErrMsg_PI = "RErrMsg_PI";

	// rules related tags
	static final String ExecMyRuleInp_PI = "ExecMyRuleInp_PI";
	static final String RHostAddr_PI = "RHostAddr_PI";
	static final String MsParamArray_PI = "MsParamArray_PI";
	static final String MsParam_PI = "MsParam_PI";
	static final String paramLen = "paramLen";
	static final String label = "label";
	static final String dummyInt = "dummyInt";

	// admin tags
	static final String generalAdminInp_PI = "generalAdminInp_PI";
	static final String simpleQueryInp_PI = "simpleQueryInp_PI";
	static final String simpleQueryOut_PI = "simpleQueryOut_PI";
	static final String sql = "sql";
	static final String control = "control";
	static final String form = "form";
	static final String maxBufSize = "maxBufSize";
	static final String outBuf = "outBuf";
	static final String arg0 = "arg0";
	static final String arg1 = "arg1";
	static final String arg2 = "arg2";
	static final String arg3 = "arg3";
	static final String arg4 = "arg4";
	static final String arg5 = "arg5";
	static final String arg6 = "arg6";
	static final String arg7 = "arg7";
	static final String arg8 = "arg8";
	static final String arg9 = "arg9";

	static String encoding = "utf-8";
	static {
		try {
			new String(new byte[0], encoding);
		} catch (UnsupportedEncodingException e) {
			encoding = java.nio.charset.Charset.defaultCharset().name();
			if (DEBUG > 0)
				System.err
						.print("utf-8 unavailable " + e.getLocalizedMessage());
		}
	}

	/**
	 * The iRODS socket connection through which all socket activity is
	 * directed.
	 */
	private Socket connection;

	/**
	 * The input stream of the iRODS socket connection.
	 */
	private InputStream in = null;

	/**
	 * The output stream of the iRODS socket connection.
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
	 * Hold the iRODS account info
	 */
	IRODSAccount account;

	/**
	 * Used in Debug mode
	 */
	private long date;
	
	private String reportedIRODSVersion = "";

	

	/**
	 * Creates a new instance of IRODSCommands
	 */
	IRODSCommands() {

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
	 * Handles connection protocol.
	 *
	 * @throws IOException
	 *             if the host cannot be opened or created.
	 */
	void connect(IRODSAccount irodsAccount) throws IOException {
		Tag message;
		// irodsAccount was already cloned by the IRODSFileSystem
		account = irodsAccount;

		if (DEBUG > 1) {
			date = new Date().getTime();
			System.err.println("Connecting to server, " + account.getHost()
					+ ":" + account.getPort() + " running version: "
					+ account.version + " as username: "
					+ account.getUserName() + "\ntime: " + date);
		}

		// Initial connection to irods server
		openSocket(account.getHost(), account.getPort());

		// Send the user info
		message = sendStartupPacket(account);
		status(message);

		// Request for authorization challenge

		if (account.getAuthenticationScheme().equals(IRODSAccount.GSI_PASSWORD)) {
			sendGSIPassword();
		} else {
			sendStandardPassword();
		}
	}

	void sendStandardPassword() throws IOException {
		send(createHeader(RODS_API_REQ, 0, 0, 0, AUTH_REQUEST_AN));
		flush();
		Tag message = readMessage(false);

		// Create and send the response

		String response = challengeResponse(message.getTag(challenge)
				.getStringValue(), account.getPassword());
		message = new Tag(authResponseInp_PI, new Tag[] {
				new Tag(this.response, response),
				new Tag(this.username, account.getUserName()), });

		try {
			// should be a header with no body if successful
			message = irodsFunction(RODS_API_REQ, message, AUTH_RESPONSE_AN);
		} catch (IRODSException e) {
			if (e.getType() == IRODSException.CAT_INVALID_AUTHENTICATION) {
				SecurityException se = new SecurityException(
						"Invalid authentication");
				se.initCause(e);
				throw se;
			} else {
				throw e;
			}
		}
	}

	void sendGSIPassword() throws IOException {
		send(createHeader(RODS_API_REQ, 0, 0, 0, GSI_AUTH_REQUEST_AN));
		flush();

		// Create and send the response

		account.serverDN = readMessage(false).getTag(ServerDN).getStringValue();
		new GSIAuth(account, connection, out, in);
	}
	
	/**
	 * <code>String</code> containing the IRODS version as reported by the connected IRODS version
	 * @return <code>String</code> with value returned in response to IRODS startup packet
	 */
	public synchronized String getReportedIRODSVersion() {
		return reportedIRODSVersion;
	}

	/**
	 * @param reportedIRODSVersion
	 */
	protected synchronized void setReportedIRODSVersion(String reportedIRODSVersion) {
		this.reportedIRODSVersion = reportedIRODSVersion;
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
			send(createHeader(RODS_DISCONNECT, 0, 0, 0, 0));
			flush();
			out.close();
			in.close();
			connection.close();
		}
	}

	/**
	 * Returns the closed state of the socket.
	 *
	 * @return true if the socket has been closed, or is not connected
	 */
	synchronized boolean isClosed() throws IOException {

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
	 * Handles sending the userinfo connection protocol. First, sends initial
	 * handshake with IRODS.
	 * <P>
	 *
	 * @throws IOException
	 *             if the host cannot be opened or created.
	 */
	private Tag sendStartupPacket(IRODSAccount account) throws IOException {
		/*
		 * String out2 = "<StartupPack_PI>" + "<irodsProt>"+"1"+"</irodsProt>" +
		 * "<reconnFlag>"+"0"+"</reconnFlag>" +
		 * "<connectCnt>"+"0"+"</connectCnt>" + "<proxyUser>"
		 * +account.getUserName()+ "</proxyUser>" + "<proxyRcatZone>"
		 * +account.getZone()+ "</proxyRcatZone>" + "<clientUser>"
		 * +account.getUserName()+ "</clientUser>" + "<clientRcatZone>"
		 * +account.getZone()+ "</clientRcatZone>" + "<relVersion>"
		 * +account.getVersion()+ "</relVersion>" + "<apiVersion>"
		 * +account.getAPIVersion()+ "</apiVersion>" + "<option>"
		 * +account.getOption()+ "</option>" + "</StartupPack_PI>";
		 */
		Tag startupPacket = new Tag(StartupPack_PI, new Tag[] {
				new Tag(irodsProt, "1"), new Tag(reconnFlag, "0"),
				new Tag(connectCnt, "0"),
				new Tag(proxyUser, account.getUserName()),
				new Tag(proxyRcatZone, account.getZone()),
				new Tag(clientUser, account.getUserName()),
				new Tag(clientRcatZone, account.getZone()),
				new Tag(relVersion, account.getVersion()),
				new Tag(apiVersion, account.getAPIVersion()),
				new Tag(option, account.getOption()), });
		String out = startupPacket.parseTag();
		send(createHeader(RODS_CONNECT, out.length(), 0, 0, 0));
		send(out);
		flush();
		Tag responseMessage = readMessage();
		
		// look for and retain the version of IRODS I am talking to
		String reportedRelVersion = responseMessage.getTag(relVersion).value;
		this.setReportedIRODSVersion(reportedRelVersion);
		
		return responseMessage;
	}

	/**
	 * Add the password to the end of the challenge string, pad to the correct
	 * length, and take the md5 of that.
	 */
	private String challengeResponse(String challenge, String password)
			throws SecurityException, IOException {
		// Convert base64 string to a byte array
		byte[] chal = null;
		byte[] temp = Base64.fromString(challenge);
		// new sun.misc.BASE64Decoder().decodeBuffer(challenge);

		if (account.getObf()) {
			try {
				// TODO don't touch for now
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

		if (password.length() < MAX_PASSWORD_LENGTH) {
			// pad the end with zeros to MAX_PASSWORD_LENGTH
			chal = new byte[CHALLENGE_LENGTH + MAX_PASSWORD_LENGTH];
		} else {
			throw new IllegalArgumentException("Password is too long");
		}

		// add the password to the end
		System.arraycopy(temp, 0, chal, 0, temp.length);
		temp = password.getBytes(encoding);
		System.arraycopy(temp, 0, chal, CHALLENGE_LENGTH, temp.length);

		// get the md5 of the challenge+password
		try {
			MessageDigest digest = MessageDigest.getInstance("MD5");
			chal = digest.digest(chal);
		} catch (GeneralSecurityException e) {
			SecurityException se = new SecurityException();
			se.initCause(e);
			throw se;
		}

		// after md5 turn any 0 into 1
		for (int i = 0; i < chal.length; i++) {
			if (chal[i] == 0)
				chal[i] = 1;
		}

		// return to Base64
		return Base64.toString(chal);
		// new sun.misc.BASE64Encoder().encode( chal );
	}

	// ----------------------------------------------------------------------
	// Socket Methods
	// ----------------------------------------------------------------------
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
		if (DEBUG > 5) {
			if (DEBUG > 8) {
				System.err.print("write: " + new String(value));
			} else if (!extraDebug) {
				System.err.print("write: " + new String(value));
			}
			if (DEBUG > 8) {
				for (int i = 0; i < value.length; i++) {
					System.err.print(value[i] + " ");
				}
			}
		}

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
		send(value.getBytes(encoding));
	}

	/**
	 * Writes an int to the output stream as four bytes, network order (high
	 * byte first).
	 *
	 * @param value
	 *            value to be sent
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private void sendInNetworkOrder(int value) throws IOException {
		byte bytes[] = new byte[INT_LENGTH];

		Host.copyInt(value, bytes);
		send(bytes);
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
	 * Writes an long to the output stream as eight bytes, low byte first.
	 *
	 * @param value
	 *            value to be sent
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private void send(InputStream source, long length) throws IOException {
		byte[] temp = new byte[Math.min(IRODSFileSystem.BUFFER_SIZE,
				(int) length)];
		while (length > 0) {
			if (temp.length > length) {
				temp = new byte[(int) length];
			}
			length -= source.read(temp, 0, temp.length);
			send(temp);
		}
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
			// hopefully this isn't too slow to check.
			throw new ClosedChannelException();
		}
		out.write(outputBuffer, 0, outputOffset);

		outputOffset = 0;
	}

	//
	// Input
	//
	/**
	 * Reads a byte from the server.
	 *
	 * @throws IOException
	 *             If an IOException occurs
	 */
	private byte read() throws IOException {
		return (byte) in.read();
	}

	private int read(byte[] value) throws ClosedChannelException,
			InterruptedIOException, IOException {
		return read(value, 0, value.length);
	}

	/**
	 * read length bytes from the server socket connection and write them to
	 * destination
	 */
	private void read(OutputStream destination, long length) throws IOException {
		byte[] temp = new byte[Math.min(IRODSFileSystem.BUFFER_SIZE,
				(int) length)];
		int n = 0;
		while (length > 0) {
			n = read(temp, 0, Math.min(IRODSFileSystem.BUFFER_SIZE,
					(int) length));
			if (n > 0) {
				length -= n;
				destination.write(temp, 0, n);
			} else {
				// TODO? always just EOF?
				length = n;
			}
		}
	}

	/**
	 * read length bytes from the server socket connection and write them to
	 * destination
	 */
	private void read(GeneralRandomAccessFile destination, long length)
			throws IOException {
		byte[] temp = new byte[Math.min(IRODSFileSystem.BUFFER_SIZE,
				(int) length)];
		int n = 0;
		while (length > 0) {
			n = read(temp, 0, Math.min(IRODSFileSystem.BUFFER_SIZE,
					(int) length));
			if (n > 0) {
				length -= n;
				destination.write(temp, 0, n);
			} else {
				// TODO? always just EOF?
				length = n;
			}
		}
	}

	/**
	 * Reads a byte array from the server. Blocks until <code>length</code>
	 * number of bytes are read.
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
	private int read(byte[] value, int offset, int length)
			throws ClosedChannelException, InterruptedIOException, IOException {
		int result = 0;
		if (length + offset > value.length) {
			throw new IndexOutOfBoundsException(
					"length + offset larger than byte array");
		}
		int bytesRead = 0;
		while (bytesRead < length) {
			int read = in.read(value, offset + bytesRead, length - bytesRead);
			if (read == -1)
				break;
			bytesRead += read;
		}
		result = bytesRead;
		if (DEBUG > 5) {
			if (DEBUG > 7) {
				System.err.print("Read: "
						+ new String(value, offset, offset + bytesRead));
			} else if (!extraDebug) {
				System.err.print("Read: "
						+ new String(value, offset, offset + bytesRead));
			}
			if (DEBUG > 6) {
				if (DEBUG > 7) {
					for (int i = offset; i < offset + bytesRead; i++) {
						System.err.print(value[i] + " ");
					}
				} else if (!extraDebug) {
					for (int i = offset; i < offset + bytesRead; i++) {
						System.err.print(value[i] + " ");
					}
				}
			}
		}
		return result;
	}

	/**
	 * Just a simple message to check if there was an error.
	 */
	void status(Tag message) throws IOException {
		Tag s = message.getTag("status");
		if ((s != null) && (s.getIntValue() < 0))
			throw new IRODSException("" + s.getIntValue());
	}

	// ----------------------------------------------------------------------
	// Struct Methods
	// ----------------------------------------------------------------------
	/**
	 * Create the iRODS header packet
	 */
	private byte[] createHeader(String type, int messageLength,
			int errorLength, long byteStringLength, int intInfo)
			throws IOException {
		if (DEBUG > 0) {
			if (DEBUG > 1) {
				// to print how long this function call took
				date = new Date().getTime();
			}
			System.err.println("\n functionID: " + intInfo);
		}

		String header = "<MsgHeader_PI>" + "<type>" + type + "</type>"
				+ "<msgLen>" + messageLength + "</msgLen>" + "<errorLen>"
				+ errorLength + "</errorLen>" + "<bsLen>" + byteStringLength
				+ "</bsLen>" + "<intInfo>" + intInfo + "</intInfo>"
				+ "</MsgHeader_PI>";
		byte[] temp = header.getBytes(encoding);
		byte[] full = new byte[4 + temp.length];
		// load first 4 byte with header length
		Host.copyInt(temp.length, full);
		// copy rest of header into full
		System.arraycopy(temp, 0, full, 4, temp.length);
		return full;
	}

	private Tag readMessage() throws IOException {
		return readMessage(true);
	}

	private Tag readMessage(boolean decode) throws IOException {
		Tag header = readHeader();
		Tag message = null;

		if (DEBUG > 1) {
			// print how long this function call took
			System.err.println("\n" + (new Date().getTime() - date)
					+ " millisecs");
		}

		// TODO probably shouldn't be so hardcoded...
		String type = header.tags[0].getStringValue();
		if (true) {// type.equals()) {
			// TODO which types?
			int messageLength = header.tags[1].getIntValue();
			int errorLength = header.tags[2].getIntValue();
			int bytesLength = header.tags[3].getIntValue();
			int info = header.tags[4].getIntValue();

			// Reports iRODS errors, throw exception if appropriate
			if (info < 0) {
				// if nothing else, read the returned bytes and throw them away
				if (messageLength > 0)
					read(new byte[messageLength], 0, messageLength);

				if (info == IRODSException.CAT_NO_ROWS_FOUND
						|| info == IRODSException.CAT_SUCCESS_BUT_WITH_NO_INFO) {
					if (errorLength != 0) {
						byte[] errorMessage = new byte[errorLength];
						read(errorMessage, 0, errorLength);
						if (DEBUG > 1) {
							Tag errorTag = readNextTag(errorMessage);
							System.err.println("IRODS error occured "
									+ errorTag.getTag(RErrMsg_PI).getTag(msg)
									+ " : " + info);
						}
					}

					// query with no results
					return null;
				} else if (info == IRODSException.OVERWITE_WITHOUT_FORCE_FLAG) {
					// TODO keep?
					throw new IRODSException(
							"Attempt to overwrite file without force flag. ",
							info);
				} else {
					if (errorLength != 0) {
						byte[] errorMessage = new byte[errorLength];
						read(errorMessage, 0, errorLength);
						Tag errorTag = readNextTag(errorMessage);

						throw new IRODSException("IRODS error occured "
								+ errorTag.getTag(RErrMsg_PI).getTag(msg), info);
					}
					throw new IRODSException("IRODS error occured " + info,
							info);
				}
			}

			if (errorLength != 0) {
				byte[] errorMessage = new byte[errorLength];
				read(errorMessage, 0, errorLength);
				Tag errorTag = readNextTag(errorMessage);

				throw new IRODSException("IRODS error occured "
						+ errorTag.getTag(RErrMsg_PI).getTag(msg), info);
			}

			if (messageLength > 0) {
				message = readMessageBody(messageLength, decode);
			}
			if (bytesLength != 0 || info > 0) {
				if (message == null) {
					message = new Tag(MsgHeader_PI);
				}

				// lets the bytes get read later,
				// instead of passing a 32MB buffer around.
				message.addTag(header);
			}
		}

		return message;
	}

	/**
	 * Going to read the header somewhat differently
	 */
	private Tag readHeader() throws IOException {
		byte[] header;
		int length = readHeaderLength();
		if (length < 0)
			throw new ProtocolException();
		else if (length > 10000000) {
			/*
			 * Protocol failure: One cause, if running a rule that uses
			 * msiDataObjPut or Get, then when the server requests the Put, the
			 * client instead send a query or other function, the next function
			 * sent to the server (such as the Put itself) will fail and the
			 * server will return a RError_PI message. However this message,
			 * unlike regular server communications, does not have the 4 byte
			 * message length in front of the message. Causing unexpected
			 * results when attempting to parse the message, ie. <REr are
			 * interpreted as the message length.
			 *
			 * <RError_PI> <count>1 </count> <RErrMsg_PI> <status>-808000
			 * </status> <msg>ERROR: msiDataObjPut: rsDataObjPut failed for
			 * <MsgHeader_PI> <type>RODS_API_REPLY </type> <msgLen>0 </msgLen>
			 * <errorLen>0 </errorLen> <bsLen>0 </bsLen> <intInfo>-808000
			 * </intInfo> </MsgHeader_PI> , status = -808000 </msg>
			 * </RErrMsg_PI> </RError_PI>
			 */

			if (DEBUG > 0)
				System.out.println("\nprotocol error " + length);

			// to recover from some protocol errors, (slowly and if lucky)
			// read until a new message header is found.
			boolean cont = true; // TODO thread to eventually end this when
			// blocked
			int sigh;
			byte[] temp = new byte[13];
			String newHeader = "MsgHeader_PI>";
			// hopefully won't be too many bytes...
			do {
				sigh = in.read();
				if (sigh == (int) '<') {
					sigh = in.read(temp);
					if (true) {// (sigh == 13) {
						if (new String(temp, encoding).equals(newHeader)) {
							temp = new byte[1000];
							// find the end of the header and proceed from there
							for (int i = 0; i < temp.length; i++) {
								temp[i] = (byte) in.read();
								if (temp[i] == '>' && temp[i - 1] == 'I'
										&& temp[i - 2] == 'P'
										&& temp[i - 3] == '_'
										&& temp[i - 4] == 'r'
										&& temp[i - 5] == 'e'
										&& temp[i - 6] == 'd'
										&& temp[i - 7] == 'a'
										&& temp[i - 8] == 'e'
										&& temp[i - 9] == 'H'
										&& temp[i - 10] == 'g'
										&& temp[i - 11] == 's'
										&& temp[i - 12] == 'M'
										&& temp[i - 13] == '/'
										&& temp[i - 14] == '<') {
									// almost forgot the '\n'
									in.read();

									// <MsgHeader_PI> + the above header
									header = new byte[i + 1 + 14];
									System.arraycopy(("<" + newHeader)
											.getBytes(), 0, header, 0, 14);
									System
											.arraycopy(temp, 0, header, 14,
													i + 1);
									return readNextTag(header);
								}
							}
						}
					}
				} else if (sigh == -1) {
					throw new ProtocolException(
							"Server connection lost, due to error");
				}
			} while (cont);

		}

		header = new byte[length];
		read(header, 0, length);

		return readNextTag(header);
	}

	private int readHeaderLength() throws IOException {
		byte[] headerInt = new byte[HEADER_INT_LENGTH];
		read(headerInt, 0, HEADER_INT_LENGTH);
		/*
		 * int hi = Host.castToInt(headerInt); if (hi > 0){
		 * System.out.print(hi+" "); } while (hi == 808333322) {
		 * System.out.print( new String(headerInt)); read(headerInt, 0,
		 * HEADER_INT_LENGTH); hi = Host.castToInt(headerInt); return 142; }
		 */
		return Host.castToInt(headerInt);
	}

	/*
	 * private Tag readMessageBody( int length ) throws IOException { return
	 * readMessageBody(length, true); }
	 */
	private Tag readMessageBody(int length, boolean decode) throws IOException {
		byte[] body = new byte[length];
		read(body, 0, length);
		return readNextTag(body, decode);
	}

	// TODO maybe can make this just one function instead of two...
	/**
	 * Read the data buffer to discover the first tag. Fill the values of that
	 * tag according to the above defined static final values.
	 *
	 * @throws UnsupportedEncodingException
	 *             shouldn't throw, already tested for
	 */
	static Tag readNextTag(byte[] data) throws UnsupportedEncodingException {
		return readNextTag(data, true);
	}

	static Tag readNextTag(byte[] data, boolean decode)
			throws UnsupportedEncodingException {
		if (data == null)
			return null;

		String d = new String(data, encoding);

		if (DEBUG > 4) {
			System.err.println(d);
		}
		// remove the random '\n'
		// had to find the end, sometimes '\n' is there, sometimes not.
		// TODO probably isn't best way
		d = d.replaceAll(CLOSE_END_TAG + "\n", "" + CLOSE_END_TAG);

		int start = d.indexOf(OPEN_START_TAG), end = d.indexOf(CLOSE_START_TAG,
				start);
		int offset = 0;
		if (start < 0)
			return null;

		String tagName = d.substring(start + 1, end);
		end = d.lastIndexOf(OPEN_END_TAG + tagName + CLOSE_END_TAG);

		Tag tag = new Tag(tagName);
		offset = start + tagName.length() + 2;

		while (d.indexOf(OPEN_START_TAG, offset) >= 0 && offset >= 0
				&& offset < end) {
			// send the rest of the bytes read
			offset = readSubTag(tag, d, offset, decode);
		}
		// if (value.length > 1) TODO?

		return tag;
	}

	/**
	 * Read the data buffer to discover a sub tag. Fill the values of that tag
	 * according to the above defined static final values.
	 *
	 * @throws UnsupportedEncodingException
	 *             shouldn't throw, already tested for
	 */
	private static int readSubTag(Tag tag, String data, int offset,
			boolean decode) throws UnsupportedEncodingException {
		// easier to just write a second slightly modified method
		// instead of try to mix the two together,
		// even though they are very similar.
		int start = data.indexOf(OPEN_START_TAG, offset);
		if (start < 0) {
			// TODO
			return 1;
		}
		int closeStart = data.indexOf(CLOSE_START_TAG, start);
		String tagName = data.substring(start + 1, closeStart);
		int end = data.indexOf(OPEN_END_TAG + tagName + CLOSE_END_TAG,
				closeStart);
		int subTagStart = data.indexOf(OPEN_START_TAG, closeStart);

		Tag subTag = new Tag(tagName);
		tag.addTag(subTag);
		offset = start + tagName.length() + 2;
		if (subTagStart == end) {
			subTag.setValue(data.substring(offset, end), decode);
			return end + tagName.length() + 3; // endTagLocation + </endTag>
		} else {
			while (data.indexOf(OPEN_START_TAG, offset) >= 0 && offset >= 0
					&& offset < end) {
				// read the subTag, get new offset
				offset = readSubTag(subTag, data, offset, decode);
			}
			return offset + tagName.length() + 3; // endTagLocation + </endTag>
		}
		// if (value.length > 1) TODO?
	}

	/**
	 * Creates the KeyValPair_PI tag.
	 */
	Tag createKeyValueTag(String keyword, String value) {
		return createKeyValueTag(new String[][] { { keyword, value } });
	}

	/**
	 * Creates the KeyValPair_PI tag.
	 */
	Tag createKeyValueTag(String[][] keyValue) {
		/*
		 * Must be like the following: <KeyValPair_PI> <ssLen>3</ssLen>
		 * <keyWord>dataType</keyWord> <keyWord>destRescName</keyWord>
		 * <keyWord>dataIncluded</keyWord> <svalue>generic</svalue>
		 * <svalue>resourceB</svalue> <svalue></svalue> </KeyValPair_PI>
		 */

		Tag pair = new Tag(KeyValPair_PI, new Tag(ssLen, 0));
		int i = 0, ssLength = 0;

		// return the empty Tag
		if (keyValue == null)
			return pair;

		for (; i < keyValue.length; i++) {
			if (keyValue[i] != null && keyValue[i][0] != null) {
				pair.addTag(keyWord, keyValue[i][0]);
				ssLength++;
			}
		}

		// just use index zero because they have to be in order...
		pair.tags[0].setValue(ssLength);
		if (i == 0)
			return pair;

		for (i = 0; i < keyValue.length; i++) {
			if (keyValue[i] != null && keyValue[i][0] != null) {
				pair.addTag(svalue, keyValue[i][1]);
			}
		}

		return pair;
	}

	// ----------------------------------------------------------------------
	// Basic irodsFunction format
	// ----------------------------------------------------------------------
	/**
	 * Create a typical iRODS api call Tag
	 */
	Tag irodsFunction(String type, Tag message, int intInfo) throws IOException {
		return irodsFunction(type, message, 0, null, 0, null, intInfo);
	}

	/**
	 * Create an iRODS message Tag, including header. Send the bytes of the byte
	 * array, no error stream.
	 */
	synchronized Tag irodsFunction(String type, Tag message,
			byte[] errorStream, int errorOffset, int errorLength, byte[] bytes,
			int byteOffset, int byteStringLength, int intInfo)
			throws IOException {
		String out = message.parseTag();

		if (DEBUG > 3) {
			System.err.println(out);
		}
		send(createHeader(RODS_API_REQ, out.getBytes(encoding).length,
				errorLength, byteStringLength, intInfo));
		send(out);
		if (byteStringLength > 0)
			send(bytes, byteOffset, byteStringLength);
		flush();
		return readMessage();
	}

	/**
	 * Create an iRODS message Tag, including header.
	 */
	synchronized Tag irodsFunction(String type, Tag message, int errorLength,
			InputStream errorStream, long byteStringLength,
			InputStream byteStream, int intInfo) throws IOException {
		String out = message.parseTag();

		if (DEBUG > 3) {
			System.err.println(out);
		}
		send(createHeader(RODS_API_REQ, out.getBytes(encoding).length,
				errorLength, byteStringLength, intInfo));
		send(out);
		if (errorLength > 0)
			send(errorStream, errorLength);
		if (byteStringLength > 0)
			send(byteStream, byteStringLength);
		flush();
		return readMessage();
	}

	// ----------------------------------------------------------------------
	// irods functions
	// ----------------------------------------------------------------------
	/*
	 * Functions which call irodsFunction(...) then send or recieve more bytes,
	 * such as get, put, fileRead, or really any time the message header
	 * bytesLength > 0 Must be synchronized for to be thread safe.
	 */

	/**
	 *
	 * @return various server information
	 * @throws java.io.IOException
	 */
	String miscServerInfo() throws IOException {
		send(createHeader(RODS_API_REQ, 0, 0, 0, GET_MISC_SVR_INFO_AN));
		flush();
		Tag message = readMessage();
		return message.parseTag();
		/*
		 * <MiscSvrInfo_PI> <serverType>1</serverType>
		 * <serverBootTime>1225230863</serverBootTime>
		 * <relVersion>rods1.1</relVersion> <apiVersion>d</apiVersion>
		 * <rodsZone>tempZone</rodsZone> </MiscSvrInfo_PI> I RCAT_ENABLED
		 * relVersion=rods1.1 apiVersion=d rodsZone=tempZone up 3 days, 1:26
		 */
	}

	void chmod(IRODSFile file, String permission, String user, String zoneName,
			boolean recursive) throws IOException {
		Tag message = new Tag(modAccessControlInp_PI,
				new Tag[] { new Tag(recursiveFlag, recursive ? 1 : 0),
						new Tag(accessLevel, permission),
						new Tag(userName, user), new Tag(zone, zoneName),
						new Tag(path, file.getAbsolutePath()), });

		irodsFunction(RODS_API_REQ, message, MOD_ACCESS_CONTROL_AN);
	}

	void copy(IRODSFile source, IRODSFile destination, boolean overwriteFlag)
			throws IOException {
		String[][] keyword = new String[2][2];
		String resource = destination.getResource();
		if (overwriteFlag) {
			keyword[0] = new String[] { IRODSMetaDataSet.FORCE_FLAG_KW, "" };
		}
		if (resource != null && !resource.equals("")) {
			keyword[1] = new String[] { IRODSMetaDataSet.DEST_RESC_NAME_KW,
					resource };
		}

		Tag message = new Tag(DataObjCopyInp_PI, new Tag[] {
		// define the source
				new Tag(DataObjInp_PI, new Tag[] {
						new Tag(objPath, source.getAbsolutePath()),
						new Tag(createMode, 0), new Tag(openFlags, 0),
						new Tag(offset, 0), new Tag(dataSize, source.length()),
						new Tag(numThreads, 0), new Tag(oprType, COPY_SRC),
						createKeyValueTag(null), }),
				// define the destination
				new Tag(DataObjInp_PI, new Tag[] {
						new Tag(objPath, destination.getAbsolutePath()),
						new Tag(createMode, 0), new Tag(openFlags, 0),
						new Tag(offset, 0), new Tag(dataSize, 0),
						new Tag(numThreads, 0), new Tag(oprType, COPY_DEST),
						createKeyValueTag(keyword), }), });

		irodsFunction(RODS_API_REQ, message, DATA_OBJ_COPY_AN);
	}

	/**
	 * Delete the given collection from IRODS.
	 * @param file {@link IRODSFile IRODSFile} that is a collection to be deleted
	 * @param force <code>boolean</code> indicates Immediate removal of data-objects
	 * without putting them in trash
	 * @throws IOException
	 */
	void deleteDirectory(IRODSFile file, boolean force) throws IOException {
		String[][] keyword = null;
		if (force) {
			keyword = new String[][] {
					new String[] { IRODSMetaDataSet.FORCE_FLAG_KW, "" },
					new String[] { IRODSMetaDataSet.RECURSIVE_OPR__KW, "" }, };
		} else {
			keyword = new String[][] { new String[] {
					IRODSMetaDataSet.RECURSIVE_OPR__KW, "" }, };
		}
		Tag message = new Tag(CollInp_PI, new Tag[] {
				new Tag(collName, file.getAbsolutePath()),
				createKeyValueTag(keyword), });

		Tag reply = irodsFunction(RODS_API_REQ, message, RM_COLL_AN);

		// may be a status reply if more than n files exist in the collection,
		// these need to be responded to

		processClientStatusMessages(reply);

	}

	/**
	 * Respond to client status messages for an operation until exhausted.
	 * @param reply <code>Tag</code> containing status messages from IRODS
	 * @throws IOException
	 */
	private void processClientStatusMessages(Tag reply) throws IOException {
		if (reply.getLength() > 0) {
			if (reply.tagName.equals(CollOprStat_PI)) {
				// formulate an answer status reply

				boolean done = false;
				Tag ackResult = null;

				/*
				 * There is a known issue here.  The client status messages returned only have a file count.  If
				 * I get a file count less than the known threshold, then I know I'm done.  However, there is an edge case
				 * Where I get a file count equal to the theshold, but there are no more files to process.  Currently this
				 * results in an NPE, but needs to be addressed within IRODS. As a work-around, the npe is trapped and treated
				 * as the end of the operation.
				 */

				while (!done) {
					sendInNetworkOrder(SYS_CLI_TO_SVR_COLL_STAT_REPLY);
					flush();
					try {
						ackResult = readMessage();
					} catch (NullPointerException npe) {
						if (IRODSCommands.DEBUG > 0) {
							System.err
									.println("Boundary error on SYS_CLI_TO_SVR_COLL_STAT_REPLY, ignored");
						}
						// treat npe the same as a valid end of status messages condition
						done = true;
						continue;
					}
					if (reply.tagName.equals(CollOprStat_PI)) {
						Tag fileCountTag = reply.getTag("filesCnt");
						if (Integer.parseInt((String) fileCountTag.getValue()) < SYS_CLI_TO_SVR_COLL_STAT_SIZE) {
							done = true;
							continue;
						}
						continue;
					} else {
						done = true;
					}
				}
			}
		}
	}

	void deleteFile(IRODSFile file, boolean force) throws IOException {
		String[][] keyword = null;
		if (force) {
			keyword = new String[][] { new String[] {
					IRODSMetaDataSet.FORCE_FLAG_KW, "" } };
		}
		Tag message = new Tag(DataObjInp_PI, new Tag[] {
				new Tag(objPath, file.getAbsolutePath()),
				new Tag(createMode, 0), new Tag(openFlags, 0),
				new Tag(offset, 0), new Tag(dataSize, 0),
				new Tag(numThreads, 0), new Tag(oprType, 0),
				createKeyValueTag(keyword), });

		irodsFunction(RODS_API_REQ, message, DATA_OBJ_UNLINK_AN);
	}

	// POSIX commands
	int fileCreate(IRODSFile file, boolean read, boolean write)
			throws IOException {
		int rw = 0;
		if (read && write)
			rw = 2;
		else if (write)
			rw = 1;

		String resource = file.getResource();
		String[][] keyword = {
				{ IRODSMetaDataSet.DATA_TYPE_KW, file.getDataType() }, null };
		if (resource != null && !resource.equals("")) {
			keyword[1] = new String[] { IRODSMetaDataSet.DEST_RESC_NAME_KW,
					resource };
		}
		Tag message = new Tag(DataObjInp_PI, new Tag[] {
				new Tag(objPath, file.getAbsolutePath()),
				new Tag(createMode, 488), // octal for 750 owner has rwx, group?
				// has r+x
				new Tag(openFlags, rw), new Tag(offset, 0),
				new Tag(dataSize, -1), new Tag(numThreads, 0),
				new Tag(oprType, 0), createKeyValueTag(keyword), });

		message = irodsFunction(RODS_API_REQ, message, DATA_OBJ_CREATE_AN);
		if (message != null)
			return message.getTag(MsgHeader_PI).getTag(intInfo).getIntValue();

		return -1;
	}

	void fileClose(int fd) throws IOException {
		Tag message = new Tag(dataObjCloseInp_PI, new Tag[] {
				new Tag(l1descInx, fd), new Tag(bytesWritten, 0), });

		irodsFunction(RODS_API_REQ, message, DATA_OBJ_CLOSE_AN);
	}

	int fileOpen(IRODSFile file, boolean read, boolean write)
			throws IOException {
		int rw = 0;
		if (read && write)
			rw = 2;
		else if (write)
			rw = 1;

		Tag message = new Tag(DataObjInp_PI, new Tag[] {
				new Tag(objPath, file.getAbsolutePath()),
				new Tag(createMode, 0), // can ignore on open
				new Tag(openFlags, rw), new Tag(offset, 0),
				new Tag(dataSize, 0), new Tag(numThreads, 0),
				new Tag(oprType, 0), createKeyValueTag(null), });

		message = irodsFunction(RODS_API_REQ, message, DATA_OBJ_OPEN_AN);
		if (message != null)
			return message.getTag(MsgHeader_PI).getTag(intInfo).getIntValue();

		return -1;
	}

	/**
	 * Read a file to the given stream.
	 */
	synchronized int fileRead(int fd, OutputStream destination, long length)
			throws IOException {
		
		// shim code for  Bug 40 -  IRODSCommands.fileRead() with length of 0 causes null message from irods 
		if (length == 0) {
			length = 1;
		}
		
		if (fd == 0 || destination == null) {
			throw new IllegalArgumentException("invalid parameters for fileRead");
		}
		
		// FIXME: test other file read methods with length 0
		
		// XXX: length param is unused
		Tag message = new Tag(dataObjReadInp_PI, new Tag[] {
				new Tag(l1descInx, fd), new Tag(len, length), });

		message = irodsFunction(RODS_API_REQ, message, DATA_OBJ_READ_AN);
		// Need the total dataSize
		// FIXME: length of zero causes message to be null
		length = message.getTag(MsgHeader_PI).getTag(bsLen).getIntValue();

		// read the message byte stream into the local file
		extraDebug = true;
		read(destination, length);
		extraDebug = false;

		return message.getTag(MsgHeader_PI).getTag(intInfo).getIntValue();
	}

	/**
	 * Read a file into the given byte array.
	 */
	synchronized int fileRead(int fd, byte buffer[], int offset, int length)
			throws IOException {
		
		Tag message = new Tag(dataObjReadInp_PI, new Tag[] {
				new Tag(l1descInx, fd), new Tag(len, length), });

		message = irodsFunction(RODS_API_REQ, message, DATA_OBJ_READ_AN);
		// Need the total dataSize
		if (message == null)
			return -1; // TODO um, return -1?

		length = message.getTag(MsgHeader_PI).getTag(bsLen).getIntValue();

		// read the message byte stream into the local file
		extraDebug = true;
		int read = read(buffer, offset, length);
		extraDebug = false;

		if (read == message.getTag(MsgHeader_PI).getTag(intInfo).getIntValue()) {
			return read;
		} else {
			// TODO exception ok?
			throw new ProtocolException("Bytes read mismatch");
		}
	}

	/**
	 * Set the file position for the IRODS file to the specified position
	 * @param fd <code>int</code> with the file descriptor created by the {@link #fileOpen(IRODSFile, boolean, boolean) fileOpen} method
	 * @param seek <code>long</code> that is the offset value
	 * @param whence <code>int</code> that specifies the postion to compute the offset from //FIXME: enumeration?  what values?
	 * @return <code>long</code with the new offset.
	 * @throws IOException
	 */
	long fileSeek(int fd, long seek, int whence) throws IOException {
		
		if (whence == GeneralRandomAccessFile.SEEK_START || whence == GeneralRandomAccessFile.SEEK_CURRENT || whence == GeneralRandomAccessFile.SEEK_END) {
			//ok
		} else {
			throw new IllegalArgumentException("whence value in seek must be SEEK_START, SEEK_CURRENT, or SEEK_END");
		}
		
		if (fd <= 0) {
			throw new IllegalArgumentException("no valid file handle provided");
		}
		
		
		Tag message = new Tag(fileLseekInp_PI, new Tag[] {
				new Tag(fileInx, fd), new Tag(offset, seek),
				new Tag(this.whence, whence), });

		message = irodsFunction(RODS_API_REQ, message, DATA_OBJ_LSEEK_AN);
		return message.getTag(offset).getLongValue();
	}

	/**
	 * Write a file into the given InputStream.
	 */
	int fileWrite(int fd, InputStream source, long length) throws IOException {
		Tag message = new Tag(dataObjWriteInp_PI, new Tag[] {
				new Tag(dataObjInx, fd), new Tag(len, length), });

		message = irodsFunction(RODS_API_REQ, message, 0, null, length, source,
				DATA_OBJ_WRITE_AN);
		return message.getTag(MsgHeader_PI).getTag(intInfo).getIntValue();
	}

	/**
	 * Write a file into the given byte array.
	 */
	int fileWrite(int fd, byte buffer[], int offset, int length)
			throws IOException {
		Tag message = new Tag(dataObjWriteInp_PI, new Tag[] {
				new Tag(dataObjInx, fd), new Tag(len, length), });

		message = irodsFunction(RODS_API_REQ, message, null, 0, 0, buffer,
				offset, length, DATA_OBJ_WRITE_AN);
		return message.getTag(MsgHeader_PI).getTag(intInfo).getIntValue();
	}

	synchronized void get(IRODSFile source, GeneralFile destination)
			throws IOException {
		Tag message = new Tag(DataObjInp_PI, new Tag[] {
				new Tag(objPath, source.getAbsolutePath()),
				new Tag(createMode, 0), new Tag(openFlags, 0),
				new Tag(offset, 0), new Tag(dataSize, 0),
				new Tag(numThreads, 0), new Tag(oprType, GET_OPR),
				createKeyValueTag(null), });

		message = irodsFunction(RODS_API_REQ, message, DATA_OBJ_GET_AN);

		// irods file doesn't exist
		if (message == null)
			return;

		// Need the total dataSize
		Tag temp = message.getTag(MsgHeader_PI);
		if (temp == null) {
			// length is zero
			destination.createNewFile();
			return;
		}
		temp = temp.getTag(bsLen);
		if (temp == null) {
			temp = message.getTag(MsgHeader_PI).getTag(bsLen);
			return;
		}
		long length = temp.getIntValue();

		// if length == zero, check for multiple thread copy
		if (length == 0) {
			int threads = message.getTag(numThreads).getIntValue();
			if (threads > 0) {
				String host = message.getTag(PortList_PI).getTag(hostAddr)
						.getStringValue();
				int port = message.getTag(PortList_PI).getTag(portNum)
						.getIntValue();
				int pass = message.getTag(PortList_PI).getTag(cookie)
						.getIntValue();

				// TODO check windowsize, l1descInx or sock?

				Thread[] transferThreads = new Thread[threads];
				TransferThread[] transfer = new TransferThread[threads];
				for (int i = 0; i < threads; i++) {
					transfer[i] = new TransferThread(host, port, pass,
							FileFactory.newRandomAccessFile(destination, "rw"));
					transferThreads[i] = new Thread(transfer[i]);
				}
				for (int i = 0; i < threads; i++) {
					transferThreads[i].start();
				}

				try {
					for (int i = 0; i < threads; i++) {
						if (transferThreads[i].isAlive())
							transferThreads[i].join();
					}
				} catch (InterruptedException e) {
					if (DEBUG > 2) {
						System.err.println("Probably not an important error");
						e.printStackTrace();
					}
				}
				for (int i = 0; i < threads; i++) {
					transfer[i].close();
				}
			}
		} else {
			// read the message byte stream into the local file
			read(FileFactory.newRandomAccessFile(destination, "rw"), length);
		}

		// TODO can also match the checksum in the message
	}

	/**
   *
   */
	void mkdir(IRODSFile directory) throws IOException {
		if (directory == null) {
			throw new NullPointerException("Directory path cannot be null");
		}
		Tag message = new Tag(CollInp_PI, new Tag[] {
				new Tag(collName, directory.getAbsolutePath()),
				createKeyValueTag(null), });

		/*
		 * New version after 201 Tag message = new Tag(CollInpNew_PI, new Tag[]{
		 * new Tag(collName, directory.getAbsolutePath()), new Tag(flags, 0),
		 * createKeyValueTag( null ), } );
		 */
		irodsFunction(RODS_API_REQ, message, COLL_CREATE_AN);
	}

	// TODO needed by send(datastream)? synchronized
	void put(GeneralFile source, IRODSFile destination, boolean overwriteFlag)
			throws IOException {
		String resource = destination.getResource();
		long length = source.length();

		if (length > TRANSFER_THREAD_SIZE) {
			String[][] keyword = {
					{ IRODSMetaDataSet.DATA_TYPE_KW, destination.getDataType() },
					{ null }, { null } };
			if (overwriteFlag) {
				keyword[1] = new String[] { IRODSMetaDataSet.FORCE_FLAG_KW, "" };
			}
			if (resource != null && !resource.equals("")) {
				keyword[2] = new String[] { IRODSMetaDataSet.DEST_RESC_NAME_KW,
						resource };
			}
			Tag message = new Tag(DataObjInp_PI, new Tag[] {
					new Tag(objPath, destination.getAbsolutePath()),
					new Tag(createMode, 448), // octal for 700 owner has rw
					new Tag(openFlags, 1), new Tag(offset, 0),
					new Tag(dataSize, length), new Tag(numThreads, 0),
					new Tag(oprType, PUT_OPR), createKeyValueTag(keyword), });

			message = irodsFunction(RODS_API_REQ, message, DATA_OBJ_PUT_AN);

			if (message == null) {
				// TODO error?!
				return;
			}

			int threads = message.getTag(numThreads).getIntValue();
			if (threads > 0) {
				InputStream[] inputs = new InputStream[threads];
				for (int i = 0; i < threads; i++) {
					inputs[i] = FileFactory.newFileInputStream(source);
				}

				synchronized (this) {
					String host = message.getTag(PortList_PI).getTag(hostAddr)
							.getStringValue();
					int port = message.getTag(PortList_PI).getTag(portNum)
							.getIntValue();
					int pass = message.getTag(PortList_PI).getTag(cookie)
							.getIntValue();
					long transferLength = length / threads;

					Thread[] transferThreads = new Thread[threads];
					TransferThread[] transfer = new TransferThread[threads];
					for (int i = 0; i < threads - 1; i++) {
						transfer[i] = new TransferThread(host, port, pass, // connection
								// info
								inputs[i], // sourceFile
								transferLength * i, // offset
								transferLength // length
						);
						transferThreads[i] = new Thread(transfer[i]);
					}
					// last thread is a little different
					transfer[threads - 1] = new TransferThread(host, port,
							pass, // connection info
							inputs[threads - 1], // sourceFile
							transferLength * (threads - 1), // offset
							(int) (length - transferLength * (threads - 1)) // length
					);
					transferThreads[threads - 1] = new Thread(
							transfer[threads - 1]);

					for (int i = 0; i < threads; i++) {
						transferThreads[i].start();
					}

					try {
						for (int i = 0; i < threads; i++) {
							if (transferThreads[i].isAlive())
								transferThreads[i].join();
						}
					} catch (InterruptedException e) {
						if (DEBUG > 2) {
							System.err
									.println("Probably not an important error");
							e.printStackTrace();
						}
					}
					for (int i = 0; i < threads; i++) {
						if (transferThreads[i].isAlive())
							transfer[i].close();
					}

					// return complete( file descriptor )
					operationComplete(message.getTag(l1descInx).getIntValue());
				}
			}
		} else {
			String[][] keyword = {
					{ IRODSMetaDataSet.DATA_TYPE_KW, destination.getDataType() },
					{ IRODSMetaDataSet.DATA_INCLUDED_KW, "" }, { null },
					{ null } };
			if (overwriteFlag) {
				keyword[2] = new String[] { IRODSMetaDataSet.FORCE_FLAG_KW, "" };
			}
			if (resource != null && !resource.equals("")) {
				keyword[3] = new String[] { IRODSMetaDataSet.DEST_RESC_NAME_KW,
						resource };
			}
			Tag message = new Tag(DataObjInp_PI, new Tag[] {
					new Tag(objPath, destination.getAbsolutePath()),
					new Tag(createMode, 448), // octal for 700 owner has rw
					new Tag(openFlags, 1), new Tag(offset, 0),
					new Tag(dataSize, length), new Tag(numThreads, 0),
					new Tag(oprType, PUT_OPR), createKeyValueTag(keyword), });
			// send the message, no result expected.
			// exception thrown on error.
			irodsFunction(RODS_API_REQ, message, 0, null, length, FileFactory
					.newFileInputStream(source), DATA_OBJ_PUT_AN);
		}
	}

	/**
	 * Add or update an AVU value for a data object or collection
	 * @param file {@line edu.sdsc.grid.io.irods.IRODSFile IRODSFile} describing the object or collection
	 * @param values <code>String[]</code> containing an AVU in the form (attrib name, attrib value) or (attrib name, attrib value, attrib units)
	 * @throws IOException
	 */
	void modifyMetaData(IRODSFile file, String[] values) throws IOException {
		
		if (file == null) {
			throw new IllegalArgumentException("irods file must not be null");
		}
		
		if (values.length < 2 || values.length > 3) {
			throw new IllegalArgumentException("metadata length must be 2 (name and value) or 3 (name, value, units) ");
		}
		
		Tag message = new Tag(ModAVUMetadataInp_PI, new Tag[] { new Tag("arg0",
				"add"), });
		if (file.isDirectory()) {
			message.addTag("arg1", "-c");
		} else {
			message.addTag("arg1", "-d");
		}

		message.addTag("arg2", file.getAbsolutePath());

		for (int i = 0, j = 0; i < 7; i++) {
			j = i + 3;
			if (i < values.length) {
				message.addTag("arg" + j, values[i]);
			} else {
				message.addTag("arg" + j, "");
			}
		}

		irodsFunction(RODS_API_REQ, message, MOD_AVU_METADATA_AN);
	}

	void deleteMetaData(IRODSFile file, String[] values) throws IOException {
		Tag message = new Tag(ModAVUMetadataInp_PI, new Tag[] { new Tag("arg0",
				"rmw"), });
		if (file.isDirectory()) {
			message.addTag("arg1", "-c");
		} else {
			message.addTag("arg1", "-d");
		}

		message.addTag("arg2", file.getAbsolutePath());

		for (int i = 0, j = 0; i < 7; i++) {
			j = i + 3;
			if (i < values.length) {
				message.addTag("arg" + j, values[i]);
			} else {
				message.addTag("arg" + j, "");
			}
		}

		irodsFunction(RODS_API_REQ, message, MOD_AVU_METADATA_AN);
	}

	void renameFile(IRODSFile source, IRODSFile destination) throws IOException {
		Tag message = new Tag(DataObjCopyInp_PI, new Tag[] {
		// define the source
				new Tag(DataObjInp_PI, new Tag[] {
						new Tag(objPath, source.getAbsolutePath()),
						new Tag(createMode, 0), new Tag(openFlags, 0),
						new Tag(offset, 0), new Tag(dataSize, 0),
						new Tag(numThreads, 0),
						new Tag(oprType, RENAME_DATA_OBJ),
						createKeyValueTag(null), }),
				// define the destination
				new Tag(DataObjInp_PI, new Tag[] {
						new Tag(objPath, destination.getAbsolutePath()),
						new Tag(createMode, 0), new Tag(openFlags, 0),
						new Tag(offset, 0), new Tag(dataSize, 0),
						new Tag(numThreads, 0),
						new Tag(oprType, RENAME_DATA_OBJ),
						createKeyValueTag(null), }), });

		irodsFunction(RODS_API_REQ, message, DATA_OBJ_RENAME_AN);
	}

	void renameDirectory(IRODSFile source, IRODSFile destination)
			throws IOException {
		Tag message = new Tag(DataObjCopyInp_PI, new Tag[] {
		// define the source
				new Tag(DataObjInp_PI, new Tag[] {
						new Tag(objPath, source.getAbsolutePath()),
						new Tag(createMode, 0), new Tag(openFlags, 0),
						new Tag(offset, 0), new Tag(dataSize, 0),
						new Tag(numThreads, 0), new Tag(oprType, RENAME_COLL),
						createKeyValueTag(null), }),
				// define the destination
				new Tag(DataObjInp_PI, new Tag[] {
						new Tag(objPath, destination.getAbsolutePath()),
						new Tag(createMode, 0), new Tag(openFlags, 0),
						new Tag(offset, 0), new Tag(dataSize, 0),
						new Tag(numThreads, 0), new Tag(oprType, RENAME_COLL),
						createKeyValueTag(null), }), });

		irodsFunction(RODS_API_REQ, message, DATA_OBJ_RENAME_AN);
	}

	void physicalMove(IRODSFile source, IRODSFile destination)
			throws IOException {
		Tag message = new Tag(DataObjInp_PI, new Tag[] {
				new Tag(collName, source.getAbsolutePath()),
				new Tag(createMode, 0),
				new Tag(openFlags, 0),
				new Tag(offset, 0),
				new Tag(dataSize, 0),
				new Tag(numThreads, 0),
				new Tag(oprType, PHYMV_OPR),
				createKeyValueTag(IRODSMetaDataSet.DEST_RESC_NAME_KW,
						destination.getResource()) });

		irodsFunction(RODS_API_REQ, message, DATA_OBJ_PHYMV_AN);
	}

	void replicate(IRODSFile file, String newResource) throws IOException {
		Tag message = new Tag(DataObjInp_PI, new Tag[] {
				new Tag(objPath, file.getAbsolutePath()),
				new Tag(createMode, 0),
				new Tag(openFlags, 0),
				new Tag(offset, 0),
				new Tag(dataSize, 0),
				new Tag(numThreads, 0),
				new Tag(oprType, REPLICATE_OPR),
				createKeyValueTag(IRODSMetaDataSet.DEST_RESC_NAME_KW,
						newResource), });

		irodsFunction(RODS_API_REQ, message, DATA_OBJ_REPL_AN);
	}

	void deleteReplica(IRODSFile file, String resource) throws IOException {
		Tag message = new Tag(DataObjInp_PI, new Tag[] {
				new Tag(objPath, file.getAbsolutePath()),
				new Tag(createMode, 0), new Tag(openFlags, 0),
				new Tag(offset, 0), new Tag(dataSize, 0),
				new Tag(numThreads, 0), new Tag(oprType, 0),
				createKeyValueTag(IRODSMetaDataSet.RESC_NAME_KW, resource), });

		irodsFunction(RODS_API_REQ, message, DATA_OBJ_TRIM_AN);
	}

	String[] stat(IRODSFile file) throws IOException {
		String[] data;
		Tag message = new Tag(DataObjInp_PI, new Tag[] {
				new Tag(objPath, file.getAbsolutePath()),
				new Tag(createMode, 0), new Tag(openFlags, 0),
				new Tag(offset, 0), new Tag(dataSize, 0),
				new Tag(numThreads, 0), new Tag(oprType, 0),
				createKeyValueTag(null), });

		irodsFunction(RODS_API_REQ, message, OBJ_STAT_AN);

		// TODO convert result
		/*
		 * <RodsObjStat_PI> <objSize>0</objSize> <objType>2</objType>
		 * <numCopies>0</numCopies> <dataId>10548</dataId> <chksum></chksum>
		 * <ownerName>rods</ownerName> <ownerZone>tempZone</ownerZone>
		 * <createTime>1207730866</createTime>
		 * <modifyTime>1207730866</modifyTime> <SpecColl_PI> <class>2</class>
		 * <type>0</type> <collection>/tempZone/home/rods/lee</collection>
		 * <objPath></objPath> <resource>demoResc</resource>
		 * <phyPath>/tmp/lee</phyPath> <cacheDir></cacheDir>
		 * <cacheDirty>0</cacheDirty> <replNum>0</replNum> </SpecColl_PI>
		 * </RodsObjStat_PI>
		 */
		data = null;

		return data;
	}

	/**
	 * Take an existing IRODS collection and create a tar file in irods from the
	 * objects in the collection
	 *
	 * @param tarFile
	 *            {@link edu.sdsc.grid.io.irods.IRODSFile IRODSFile} that will
	 *            be the destination <code>.tar</code> file
	 * @param directory
	 *            {@link edu.sdsc.grid.io.irods.IRODSFile IRODSFile} that is the
	 *            collection to be tar'd
	 * @param resource
	 *            <code>String</code> with the resource for the
	 *            <code>.tar</code> file.
	 * @throws IOException
	 */
	void createBundle(IRODSFile tarFile, IRODSFile directory, String resource)
			throws IOException {

		if (tarFile == null || directory == null || resource == null
				|| resource.length() == 0) {
			throw new IllegalArgumentException(
					"Null values not allowed for parameters");
		} else if (!directory.isDirectory()) {
			throw new IllegalArgumentException(
					"Directory must refer to an IRODS Collection");
		}

		String[][] keyword = new String[1][2];
		keyword[0] = new String[] { IRODSMetaDataSet.DEST_RESC_NAME_KW,
				resource };

		Tag message = new Tag(StructFileExtAndRegInp_PI, new Tag[] {
				new Tag(objPath, tarFile.getAbsolutePath()),
				new Tag(collection, directory.getAbsolutePath()),
				new Tag(oprType, 0), new Tag(flags, 0),
				createKeyValueTag(keyword) });

		irodsFunction(RODS_API_REQ, message, STRUCT_FILE_BUNDLE_AN);
	}

	void extractBundle(IRODSFile tarFile, IRODSFile directory)
			throws IOException {
		Tag message = new Tag(StructFileExtAndRegInp_PI,
				new Tag[] { new Tag(objPath, tarFile.getAbsolutePath()),
						new Tag(collection, directory.getAbsolutePath()),
						new Tag(oprType, 0), new Tag(flags, 0),
						createKeyValueTag(null) });

		irodsFunction(RODS_API_REQ, message, STRUCT_FILE_BUNDLE_AN);
	}

	synchronized InputStream executeCommand(String command, String args,
			String hostAddress, String somePathInfoMaybe_whoknows)
			throws IOException {
		Tag message = new Tag(ExecCmd_PI, new Tag[] {
				new Tag(cmd, command),
				new Tag(cmdArgv, args), // TODO string array?
				new Tag(execAddr, hostAddress), new Tag(hintPath, ""),
				new Tag(addPathToArgv, 0), createKeyValueTag(null) });
		String buffer = "";

		try {
			message = irodsFunction(RODS_API_REQ, message, EXEC_CMD_AN);
		} catch (NullPointerException e) {
			NullPointerException x = new NullPointerException(
					"Can occur if the command output is too long");
			x.initCause(e);
			throw x;
		}
		if (message != null) {
			// message
			int length = message.getTag(BinBytesBuf_PI, 0).getTag(buflen)
					.getIntValue();
			if (length > 0) {
				buffer += message.getTag(BinBytesBuf_PI, 0).getTag(buf)
						.getStringValue();
			}

			// error
			length = message.getTag(BinBytesBuf_PI, 1).getTag(buflen)
					.getIntValue();
			if (length > 0) {
				buffer += message.getTag(BinBytesBuf_PI, 1).getTag(buf)
						.getStringValue();
			}
		}

		return new java.io.ByteArrayInputStream(Base64.fromString(buffer));
	}

	String checksum(IRODSFile file) throws IOException {
		Tag message = new Tag(DataObjInp_PI, new Tag[] {
				new Tag(objPath, file.getAbsolutePath()),
				new Tag(createMode, 0), new Tag(openFlags, 0),
				new Tag(offset, 0), new Tag(dataSize, 0),
				new Tag(numThreads, 0), new Tag(oprType, 0),
				createKeyValueTag(null) });

		message = irodsFunction(RODS_API_REQ, message, DATA_OBJ_CHKSUM_AN);
		if (message != null)
			return message.getTag(Rule.myStr).getStringValue();

		return null;
	}


	/**
	 * @deprecated this method apparently only returns null, and will be removed in a later release.  This method will
	 * throw a RuntimeException if invoked. {@see executeRule(String, Parameter[], Parameter[]) executeRule(String, Parameter[], Parameter{}}
	 */
	Parameter[] executeRule(Rule rule) throws IOException {
		throw new RuntimeException("Unimplemented functionality");
	
	}

	
	/**
	 * Execute an IRODS rule and return the result as a <code>Tag</code>.  Note that the result in <code>Tag</code> format can be processed by 
	 * {@link edu.sdsc.grid.io.irods.Rule#readResult(IRODSFileSystem, Tag) edu.sdsc.grio.io.irods.Rule.readResult(IRODSFileSystem, Tag)}
	 * 
	 * Note that this method currently can return null.  This behavior will be corrected in upcoming versions of Jargon.  The Rule.readResult() method
	 * was updated to return an empty Parameter[] and to tolerate a null input to ensure that NullPointerExceptions do not occur.  These are interim 
	 * fixes...this entire arrangement will be reconsidered.
	 * 
	 * @param rule <code>String</code> with the text of the rule to be executed
	 * @param input {@link edu.sdsc.grid.io.irods.Parameter Parameter[]} for inputs to the rule
	 * @param output {@link edu.sdsc.grid.io.irods.Parameter Parameter[]} containing rule output
	 * @return {@link edu.sdsc.grid.io.irods.Tag Tag} containing the response from IRODS for the rule invocation.  
	 * @throws IOException
	 */
	Tag executeRule(String rule, Parameter[] input, Parameter[] output)
			throws IOException {
		// create the rule tag
		Tag message = new Tag(ExecMyRuleInp_PI, new Tag[] {
				new Tag(myRule, rule),
				new Tag(RHostAddr_PI, new Tag[] { new Tag(hostAddr, ""),
						new Tag(rodsZone, ""), new Tag(port, 0),
						new Tag(dummyInt, 0), }), createKeyValueTag(null),
				});

		// add output parameter tags
		// They get cat together seperated by '%'
		if (output != null) {
			String temp = "";
			for (Parameter out : output)
				temp += out.getUniqueName() + "%";

			message.addTag(new Tag(outParamDesc, temp.substring(0, temp
					.length() - 1)));
		}

		// add input parameter tags
		if (input != null) {
			Tag paramArray = new Tag(MsParamArray_PI, new Tag[] {
					new Tag(paramLen, input.length), new Tag(oprType, 0)
					});
			for (Parameter in : input) {
				paramArray.addTag(in.createMsParamArray());
			}
			message.addTag(paramArray);
		}

		// send rule tag
		message = irodsFunction(RODS_API_REQ, message, EXEC_MY_RULE_AN);

		if (message == null || message.getTag(paramLen).getIntValue() <= 0) {
			return null;
		}
		return message;
	}

	/**
	 * TODO not really sure. send when certain rules are finished?
	 */
	void operationComplete(int status) throws IOException {
		Tag message = new Tag(Rule.INT_PI, new Tag[] { new Tag(Rule.myInt,
				status), });
		irodsFunction(RODS_API_REQ, message, OPR_COMPLETE_AN);
	}

	// ---------------------------------------------------------
	// Admin methods
	// ---------------------------------------------------------

	/**
	 * General iRODS Admin commands. See also iadmin
	 */
	void admin(String[] args) throws IOException {
		if (args == null || args.length <= 0)
			return;
		else if (args.length != 10) {
			// ...I have the feeling the server won't like to receive any less.
			String[] temp = new String[10];
			System.arraycopy(args, 0, temp, 0, args.length);
			args = temp;
		}

		Tag message = new Tag(generalAdminInp_PI, new Tag[] {
				new Tag(arg0, args[0] != null ? args[0] : ""),
				new Tag(arg1, args[1] != null ? args[1] : ""),
				new Tag(arg2, args[2] != null ? args[2] : ""),
				new Tag(arg3, args[3] != null ? args[3] : ""),
				new Tag(arg4, args[4] != null ? args[4] : ""),
				new Tag(arg5, args[5] != null ? args[5] : ""),
				new Tag(arg6, args[6] != null ? args[6] : ""),
				new Tag(arg7, args[7] != null ? args[7] : ""),
				new Tag(arg8, args[8] != null ? args[8] : ""),
				new Tag(arg9, args[9] != null ? args[10] : ""), });

		irodsFunction(RODS_API_REQ, message, GENERAL_ADMIN_AN);
	}

	// ---------------------------------------------------------
	// Query methods
	// ---------------------------------------------------------
	/**
	 * Made before the general query was available. Allowed queries:
	 * "select token_name from r_tokn_main where token_namespace = 'token_namespace'"
	 * , "select token_name from r_tokn_main where token_namespace = ?",
	 * "select * from r_tokn_main where token_namespace = ? and token_name like ?"
	 * , "select resc_name from r_resc_main",
	 * "select * from r_resc_main where resc_name=?",
	 * "select zone_name from r_zone_main",
	 * "select * from r_zone_main where zone_name=?",
	 * "select user_name from r_user_main where user_type_name='rodsgroup'","select user_name from r_user_main, r_user_group where r_user_group.user_id=r_user_main.user_id and r_user_group.group_user_id=(select user_id from r_user_main where user_name=?)"
	 * , "select * from r_data_main where data_id=?","select data_name, data_id, data_repl_num from r_data_main where coll_id =(select coll_id from r_coll_main where coll_name=?)"
	 * , "select coll_name from r_coll_main where parent_coll_name=?",
	 * "select * from r_user_main where user_name=?",
	 * "select user_name from r_user_main where user_type_name != 'rodsgroup'","select r_resc_group.resc_group_name, r_resc_group.resc_id, resc_name, r_group.create_ts, r_resc_group.modify_ts from r_resc_main, r_resc_group where r_resc_main.resc_id = r_resc_group.resc_id and resc_group_name=?"
	 * , "select distinct resc_group_name from r_resc_group",
	 * "select coll_id from r_coll_main where coll_name = ?"
	 *
	 */
	String[] simpleQuery(String statement, String arg) throws IOException {
		Tag message = null;
		// TODO could be more args?
		if (arg == null) {
			message = new Tag(simpleQueryInp_PI, new Tag[] {
					new Tag(sql, statement), new Tag(control, 0), // don't know
					new Tag(form, 1), // don't know
					new Tag(maxBufSize, 1024), });
		} else {
			message = new Tag(simpleQueryInp_PI, new Tag[] {
					new Tag(sql, statement), new Tag(arg1, arg),
					new Tag(control, 0), // don't know
					new Tag(form, 1), // don't know
					new Tag(maxBufSize, 1024), });
		}

		message = irodsFunction(RODS_API_REQ, message, SIMPLE_QUERY_AN);
		if (message == null) {// TODO um? ||
			// message.getTag(paramLen).getIntValue() <= 0)
			// {
			return null;
		}
		// TODO odd...
		String output = message.getTag(outBuf).getStringValue();
		return output.split("\n");
	}

	// ---------------------------------------------------------
	// New query methods
	// ---------------------------------------------------------
	// TODO sigh, find a better way and remove synchronized
	MetaDataCondition[] conditions;

	/**
	 * Send a query to iRODS
	 */
	synchronized MetaDataRecordList[] query(MetaDataCondition[] tempConditions,
			MetaDataSelect[] selects, int numberOfRecordsWanted,
			Namespace namespace) throws IOException {
		Tag message = new Tag(GenQueryInp_PI, new Tag[] {
				new Tag(maxRows, numberOfRecordsWanted),
				new Tag(continueInx, 0), // new query
				new Tag(partialStartIndex, 0), // not sure
				createKeyValueTag(null), });
		Tag[] subTags = null;
		int j = 1;
		String[] selectedAVU = new String[selects.length];
		// TODO fix silly global conditions
		conditions = tempConditions;

		// package the selects
		if (selects == null) {
			// TODO error?
			throw new NullPointerException(
					"Query must have at least one select value");
		} else {
			// fix the selects if there are AVU parts
			selects = checkForAVU(conditions, selects, namespace, selectedAVU);
		}
		selects = (MetaDataSelect[]) IRODSFileSystem
				.cleanNullsAndDuplicates(selects);

		subTags = new Tag[selects.length * 2 + 1];
		subTags[0] = new Tag(iiLen, selects.length);
		for (int i = 0; i < selects.length; i++) {
			subTags[j] = new Tag(inx, IRODSMetaDataSet.getID(selects[i]
					.getFieldName()));
			j++;
		}
		for (int i = 0; i < selects.length; i++) {
			// New for loop because they have to be in a certain order...
			subTags[j] = new Tag(ivalue, selects[i].getOperation());
			j++;
		}
		message.addTag(new Tag(InxIvalPair_PI, subTags));

		// package the conditions
		if (conditions != null) {
			// fix the conditions if there are AVU parts, also remove nulls
			conditions = (MetaDataCondition[]) IRODSFileSystem
					.cleanNullsAndDuplicates(checkForAVU(conditions, namespace));

			subTags = new Tag[conditions.length * 2 + 1];
			subTags[0] = new Tag(isLen, conditions.length);
			j = 1;
			for (int i = 0; i < conditions.length; i++) {
				subTags[j] = new Tag(inx, IRODSMetaDataSet.getID(conditions[i]
						.getFieldName()));
				j++;
			}
			for (int i = 0; i < conditions.length; i++) {
				// New for loop because they have to be in a certain order...
				subTags[j] = new Tag(svalue, " "
						+ conditions[i].getOperatorString() + " '"
						+ conditions[i].getStringValue() + "'");
				j++;
			}
			message.addTag(new Tag(InxValPair_PI, subTags));
		} else {
			// need this tag, just create a blank one
			message.addTag(new Tag(InxValPair_PI, new Tag(isLen, 0)));
		}

		// send command to
		// server***************************************************
		message = irodsFunction(RODS_API_REQ, message, GEN_QUERY_AN);

		if (message == null) {
			// query had no results
			return null;
		}

		int rows = message.getTag(rowCnt).getIntValue();
		int attributes = message.getTag(attriCnt).getIntValue();
		int continuation = message.getTag(continueInx).getIntValue();

		String[] results = new String[attributes];
		MetaDataField[] fields = new MetaDataField[attributes];
		MetaDataRecordList[] rl = new MetaDataRecordList[rows];
		for (int i = 0; i < attributes; i++) {
			// TODO just hard-coded the first "SqlResult_PI" result location...
			// definately TODO
			fields[i] = IRODSMetaDataSet.getField(message.tags[4 + i].getTag(
					attriInx).getStringValue());
		}
		for (int i = 0; i < rows; i++) {
			for (j = 0; j < attributes; j++) {
				// TODO just hard-coded the first "SqlResult_PI" result
				// location...
				results[j] = message.tags[4 + j].tags[2 + i].getStringValue();
			}
			if (continuation > 0) {
				rl[i] = new IRODSMetaDataRecordList(this, fields, results,
						continuation);
			} else {
				// No more results, don't bother with sending the IRODSCommand
				// object
				rl[i] = new IRODSMetaDataRecordList(null, fields, results,
						continuation);
			}
		}

		return rl;
	}

	MetaDataRecordList[] getMoreResults(int continuationIndex,
			int numberOfRecordsWanted) throws IOException {
		Tag message = new Tag(GenQueryInp_PI, new Tag[] {
				new Tag(maxRows, numberOfRecordsWanted),
				new Tag(continueInx, continuationIndex),
				new Tag(partialStartIndex, 0), // not sure
				new Tag(options, 32), // not sure 32?
				createKeyValueTag(null), });

		/*
		 * <GenQueryInp_PI> <maxRows>500</maxRows> <continueInx>1</continueInx>
		 * <partialStartIndex>0</partialStartIndex> <options>32</options>
		 * <KeyValPair_PI> <ssLen>0</ssLen> </KeyValPair_PI> <InxIvalPair_PI>
		 * <iiLen>11</iiLen> <inx>501</inx> <inx>403</inx> <inx>401</inx>
		 * <inx>421</inx> <inx>407</inx> <inx>420</inx> <inx>419</inx>
		 * <inx>409</inx> <inx>411</inx> <inx>404</inx> <inx>413</inx>
		 * <ivalue>1</ivalue> <ivalue>1</ivalue> <ivalue>1</ivalue>
		 * <ivalue>1</ivalue> <ivalue>1</ivalue> <ivalue>1</ivalue>
		 * <ivalue>1</ivalue> <ivalue>1</ivalue> <ivalue>1</ivalue>
		 * <ivalue>1</ivalue> <ivalue>1</ivalue></InxIvalPair_PI>
		 * <InxValPair_PI> <isLen>1</isLen> <inx>501</inx> <svalue> =
		 * '/tempZone/home/rods/myJargonLargeFilesCopyTest0'</svalue>
		 * </InxValPair_PI> </GenQueryInp_PI>
		 */
		int j = 1;
		MetaDataSelect[] selects = new MetaDataSelect[] { MetaDataSet
				.newSelection("file name") };
		Tag[] subTags = new Tag[selects.length * 2 + 1];
		subTags[0] = new Tag(iiLen, selects.length);
		for (int i = 0; i < selects.length; i++) {
			subTags[j] = new Tag(inx, IRODSMetaDataSet.getID(selects[i]
					.getFieldName()));
			j++;
		}
		for (int i = 0; i < selects.length; i++) {
			// New for loop because they have to be in a certain order...
			subTags[j] = new Tag(ivalue, selects[i].getOperation());
			j++;
		}
		message.addTag(new Tag(InxIvalPair_PI, subTags));

		message.addTag(new Tag(InxValPair_PI, new Tag(isLen, 0)));

		message = irodsFunction(RODS_API_REQ, message, GEN_QUERY_AN);

		if (message == null) {
			// query had no results
			return null;
		}

		int rows = message.getTag(rowCnt).getIntValue();
		int attributes = message.getTag(attriCnt).getIntValue();
		int continuation = message.getTag(continueInx).getIntValue();

		String[] results = new String[attributes];
		MetaDataField[] fields = new MetaDataField[attributes];
		MetaDataRecordList[] rl = new MetaDataRecordList[rows];
		for (int i = 0; i < attributes; i++) {
			// TODO just hard-coded the first "SqlResult_PI" result location...
			// definately TODO
			fields[i] = IRODSMetaDataSet.getField(message.tags[4 + i].getTag(
					attriInx).getStringValue());
		}
		for (int i = 0; i < rows; i++) {
			for (j = 0; j < attributes; j++) {
				// TODO just hard-coded the first "SqlResult_PI" result
				// location...
				results[j] = message.tags[4 + j].tags[2 + i].getStringValue();
			}
			if (continuation > 0) {
				rl[i] = new IRODSMetaDataRecordList(this, fields, results,
						continuation);
			} else {
				// No more results, don't bother with sending the IRODSCommand
				// object
				rl[i] = new IRODSMetaDataRecordList(null, fields, results,
						continuation);
			}
		}
		return rl;
	}

	// user definable query functions----------------------------------------
	/**
	 * Checks to see if any of these conditions are really a user definable
	 * metadata AVU. And if so convert it to something irods will understand.
	 *
	 * @param conditions
	 * @return
	 */
	MetaDataCondition[] checkForAVU(MetaDataCondition[] conditions,
			Namespace namespace) {
		Vector<MetaDataCondition> newConditions = null;
		for (int i = conditions.length - 1; i >= 0; i--) {
			if (checkForAVU(conditions[i].getField())) {
				if (newConditions == null) {
					newConditions = new Vector();
				}
				newConditions
						.add(MetaDataSet.newCondition(
								getAttributeValue(namespace), conditions[i]
										.getOperator(), conditions[i]
										.getStringValue()));

				conditions[i] = MetaDataSet.newCondition(
						getAttributeName(namespace), MetaDataCondition.EQUAL,
						conditions[i].getFieldName());
			}
		}

		if (newConditions != null) {
			for (MetaDataCondition c : conditions) {
				newConditions.add(c);
			}
			conditions = newConditions.toArray(conditions);
		}
		return conditions;
	}

	/**
	 * Checks the selects for user definable metadata AVU. And if so convert it
	 * to something irods will understand.
	 */
	MetaDataSelect[] checkForAVU(
			MetaDataCondition[] ughsighmaybedeletetempConditions,
			MetaDataSelect[] selects, Namespace namespace, String[] selectedAVU) {
		Vector<MetaDataCondition> newConditions = null;
		Vector<MetaDataSelect> newSelects = null;
		for (int i = selects.length - 1; i >= 0; i--) {
			if (checkForAVU(selects[i].getField())) {
				if (newConditions == null) {
					newConditions = new Vector();
				}
				newConditions.add(MetaDataSet.newCondition(
						getAttributeName(namespace), MetaDataCondition.EQUAL,
						selects[i].getFieldName()));

				// store for creating the query return
				selectedAVU[i] = selects[i].getFieldName();

				if (newSelects == null) {
					newSelects = new Vector();
				}
				// get the AVU value
				selects[i] = IRODSMetaDataSet
						.newSelection(getAttributeValue(namespace));
				// get the AVU attribute
				newSelects.add(MetaDataSet
						.newSelection(getAttributeName(namespace)));
			}
		}

		if (newConditions != null) {
			// join the conditions
			if (conditions != null && conditions.length > 0) {
				for (MetaDataCondition c : conditions) {
					newConditions.add(c);
				}
				conditions = newConditions.toArray(conditions);
			} else if (conditions != null) {
				// conditions length = 0
				conditions = newConditions.toArray(conditions);
			} else {
				conditions = newConditions.toArray(new MetaDataCondition[0]);
				/*
				 * throw new RuntimeException(
				 * "Must instantiate an IRODSFileSystem at least once " +
				 * "(for the static variables) " +
				 * "before creating query selects or conditions for iRODS");
				 */
			}

			// join the selects
			for (MetaDataSelect s : selects) {
				newSelects.add(s);
			}
			selects = newSelects.toArray(selects);
		}
		return selects;
	}

	/**
	 * Just checks the extensible value of this field to see if it was declared
	 * DEFINABLE_METADATA during the conditions creation.
	 *
	 * @param field
	 * @return
	 */
	boolean checkForAVU(MetaDataField field) {
		String ext = field.getExtensibleName(null);
		if ((ext != null) && ext.equals(IRODSMetaDataSet.DEFINABLE_METADATA))
			return true;

		return false;
	}

	String getAttributeName(Namespace namespace) {
		switch (namespace) {
		default:
		case FILE:
			return IRODSMetaDataSet.META_DATA_ATTR_NAME;
		case DIRECTORY:
			return IRODSMetaDataSet.META_COLL_ATTR_NAME;
		case RESOURCE:
			return IRODSMetaDataSet.META_RESOURCE_ATTR_NAME;
		case USER:
			return IRODSMetaDataSet.META_USER_ATTR_NAME;
		}
	}

	String getAttributeValue(Namespace namespace) {
		switch (namespace) {
		default:
		case FILE:
			return IRODSMetaDataSet.META_DATA_ATTR_VALUE;
		case DIRECTORY:
			return IRODSMetaDataSet.META_COLL_ATTR_VALUE;
		case RESOURCE:
			return IRODSMetaDataSet.META_RESOURCE_ATTR_VALUE;
		case USER:
			return IRODSMetaDataSet.META_USER_ATTR_VALUE;
		}
	}

	// ---------------------------------------------------------
	// Parallel transfer
	// ---------------------------------------------------------
	// TODO for testing
	int incThread = 0;

	class TransferThread implements Runnable {
		// Need to use GeneralRandomAccessFile
		// for a way to skip bytes according to the offset in the header.
		GeneralRandomAccessFile local;

		Socket s;
		InputStream in;
		OutputStream out;

		long transferLength;
		long offset;
		int which;

		/**
		 * Used by client parallel transfer get
		 */
		TransferThread(String host, int port, int cookie,
				GeneralRandomAccessFile destination) throws IOException {
			local = destination;
			s = new Socket(host, port);
			byte[] outputBuffer = new byte[4];
			Host.copyInt(cookie, outputBuffer);
			in = s.getInputStream();
			s.getOutputStream().write(outputBuffer);
			which = incThread;
			incThread++;
		}

		/**
		 * Used by client parallel transfer put
		 *
		 * @param host
		 * @param port
		 * @param cookie
		 * @param in
		 *            file to upload
		 * @param offset
		 *            offset into the file
		 * @param length
		 *            number of bytes this thread should transfer
		 * @throws java.io.IOException
		 */
		TransferThread(String host, int port, int cookie, InputStream in,
				long offset, long length) throws IOException {
			this.in = in;
			transferLength = length;
			if (offset > 0)
				in.skip(offset);
			this.offset = offset;
			s = new Socket(host, port);
			out = s.getOutputStream();
			// write the cookie
			byte b[] = new byte[4];
			Host.copyInt(cookie, b);
			out.write(b);
			which = incThread;
			incThread++;
		}

		protected void finalize() throws Throwable {
			if (local != null) {
				local.close();
				local = null;
			}
			if (in != null) {
				in.close();
				in = null;
			}
			if (out != null) {
				out.close();
				out = null;
			}

			super.finalize();
		}

		int readInt() throws IOException {
			byte[] b = new byte[4];
			int read = in.read(b);
			if (read != 4) {
				// TODO
			}
			return Host.castToInt(b);
		}

		long readLong() throws IOException {
			byte[] b = new byte[8];
			int read = in.read(b);
			if (read != 8) {
				// TODO
			}
			return Host.castToLong(b);
		}

		public void run() {
			try {
				if (local != null) {
					get();
				} else {
					put();
				}
			} catch (Throwable e) {// IOException e) {
				if (IRODSCommands.DEBUG > 0)
					e.printStackTrace();
				throw new RuntimeException("IOException in thread.", e);
			}
		}

		void close() {
			// garbage collector can be too slow
			if (out != null) {
				try {
					/*
					 * TODO not sure if necessary
					 * System.out.println("writing to out at close, thread "
					 * +which);
					 *
					 * String closeMessage =
					 * "<MsgHeader_PI><type>RODS_API_REQ</type>" +
					 * "<msgLen>35</msgLen><errorLen>0</errorLen><bsLen>0</bsLen>"
					 * + "<intInfo>626</intInfo></MsgHeader_PI>" +
					 * "<INT_PI><myInt>3</myInt> </INT_PI>";
					 * out.write(closeMessage.getBytes(encoding));
					 */
					out.close();
				} catch (IOException e) {
					throw new RuntimeException("IOException in thread.", e);
				}
				out = null;
			}
			if (in != null) {
				try {
					in.close();
				} catch (IOException e) {
					throw new RuntimeException("IOException in thread.", e);
				}
				in = null;
			}
			if (s != null) {
				try {
					s.close();
				} catch (IOException e) {
					throw new RuntimeException("IOException in thread.", e);
				}
				s = null;
			}
		}

		void put() throws IOException {
			// Holds all the data for transfer
			byte[] buffer = null;
			int read = 0;

			// begin transfer
			if (transferLength <= 0)
				return;
			else {
				// length has a max of 8mb?
				buffer = new byte[OUTPUT_BUFFER_LENGTH];
			}

			while (transferLength > 0) {
				// need Math.min or reads into what the other threads are
				// transferring
				read = in.read(buffer, 0, (int) Math.min(OUTPUT_BUFFER_LENGTH,
						transferLength));
				if (read > 0) {
					transferLength -= read;
					out.write(buffer, 0, read);
				} else if (read < 0) {
					throw new IOException("oops");
				}
			}
		}

		void get() throws IOException {
			// read the header
			int operation = readInt();
			// TODO not sure what flags there can be
			int flags = readInt();
			// Where to seek into the data
			long offset = readLong();
			// How much to read/write
			long length = readLong();

			// Holds all the data for transfer
			byte[] buffer = null;
			int read = 0;

			if (operation != GET_OPR) {
				// TODO got confused somehow?
				if (IRODSCommands.DEBUG > 0)
					System.err.println("Parallel transfer expected GET, "
							+ "server requested " + operation);
				return;
			}

			if (offset < 0)
				return;
			else if (offset > 0) {
				local.seek(offset);
			}

			if (length <= 0)
				return;
			else {
				// length has a max of 8mb?
				buffer = new byte[OUTPUT_BUFFER_LENGTH];
			}

			while (length > 0) {
				read = in.read(buffer, 0, Math.min(OUTPUT_BUFFER_LENGTH,
						(int) length));
				if (read > 0) {
					length -= read;
					if (length == 0) {
						local.write(buffer, 0, read);

						// read the next header
						operation = readInt();
						flags = readInt();
						offset = readLong();
						length = readLong();

						if (operation == DONE_OPR) {
							return;
						}

						// probably unnecessary
						local.seek(offset, GeneralRandomAccessFile.SEEK_START);

						// subtract the status message, an int = 9999, and a
						// bunch of 0's
						// TODO local.write(buffer, 0, read+(int)length);
					} else if (length < 0) {
						// oops? Not sure it's even be logically possible
						throw new ProtocolException();
					} else {
						local.write(buffer, 0, read);
					}
				}
			}
		}
	}
}
