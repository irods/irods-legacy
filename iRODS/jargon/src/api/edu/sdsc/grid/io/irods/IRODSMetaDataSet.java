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
//  IRODSMetaDataSet.java  -  edu.sdsc.grid.io.irods.IRODSMetaDataSet
//
//  CLASS HIERARCHY
//  java.lang.Object
//     |
//     +-.IRODSMetaDataSet
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.irods;

import java.util.HashMap;

import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.query.ExtensibleMetaDataMapping;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.DirectoryMetaData;
import edu.sdsc.grid.io.FileMetaData;
import edu.sdsc.grid.io.MetaDataField;
import edu.sdsc.grid.io.MetaDataGroup;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.ResourceMetaData;
import edu.sdsc.grid.io.UserMetaData;
import edu.sdsc.grid.io.ZoneMetaData;

/**
 * Constructs the various metadata attributes defined by the iRODS server such
 * that 'common names' can be translated to numeric values used by IRODS or SRB
 * protocols, and also can provide translation from integer value to common
 * name.
 */
public final class IRODSMetaDataSet extends MetaDataSet implements
		DirectoryMetaData, FileMetaData, ResourceMetaData, UserMetaData,
		ZoneMetaData {

	private static Logger log = LoggerFactory.getLogger(IRODSMetaDataSet.class);

	// Metadata attribute names
	// irods file access, beyond FileMetaData
	public static final String FILE_REPLICA_STATUS = "File Replica Status";
	public static final String FILE_RESOURCE_GROUP = "File Resource Group Name";
	public static final String FILE_OWNER_ZONE = "File Owner Zone";
	public static final String FILE_STATUS = "File Data Status";
	public static final String FILE_EXPIRY = "File Expiration time";
	public static final String FILE_MAP_ID = "File Map Identifier";

	public static final String FILE_ACCESS_TYPE = "File Access Type";

	/**
	 * File access constraint/permissions.
	 */
	public static final String FILE_ACCESS_NAME = ACCESS_CONSTRAINT;
	static final String FILE_TOKEN_NAMESPACE = "File Access Token Namespace";

	// for the various possible types of iRODS directories,
	// like mounted or tarfile
	/**
	 * Directory Type
	 */
	public static final String DIRECTORY_TYPE = "Directory Type";
	public static final String DIRECTORY_TOKEN_NAMESPACE = "Directory Access Token Namespace";
	public static final String DIRECTORY_USER_NAME = "Directory User Name";
	public static final String DIRECTORY_USER_ZONE = "Directory User Zone";
	public static final String DIRECTORY_ACCESS_TYPE = "Directory Access Type";

	// special for file permissions
	static final String FILE_PERMISSION_USER_NAME = "COL_DATA_USER_NAME";
	static final String FILE_PERMISSION_USER_ZONE = "COL_DATA_USER_ZONE";

	// R_RULE_EXEC
	/**
	 * Rule Execution Identifier
	 */
	public static final String RULE_ID = "Rule Execution Identifier";

	/**
	 * Rule Execution Name
	 */
	public static final String RULE_NAME = "Rule Execution Name";

	/**
	 * Rule Execution REI (Rule Execution Information) File Path
	 */
	public static final String RULE_REI_FILE_PATH = "Rule Execution REI (Rule Execution Information) File Path";

	/**
	 * Rule Execution User Name
	 */
	public static final String RULE_USER_NAME = "Rule Execution User Name";

	/**
	 * Rule Execution Address (hostname, if specified)
	 */
	public static final String RULE_ADDRESS = "Rule Execution Address (hostname, if specified)";

	/**
	 * Rule Execution Time (when to execute)
	 */
	public static final String RULE_TIME = "Rule Execution Time (when to execute)";

	/**
	 * Rule Execution Frequency (when to repeat (seconds))
	 */
	public static final String RULE_FREQUENCY = "Rule Execution Frequency (when to repeat (seconds))";

	/**
	 * Rule Execution Priority
	 */
	public static final String RULE_PRIORITY = "Rule Execution Priority";

	/**
	 * Rule Execution Estimated Exectuion Time (if specified)
	 */
	public static final String RULE_ESTIMATED_EXE_TIME = "Rule Execution Estimated Exectuion Time (if specified)";

	/**
	 * Rule Execution Notification Address (if specified)
	 */
	public static final String RULE_NOTIFICATION_ADDR = "Rule Execution Notification Address (if specified)";

	/**
	 * Rule Execution Last Execution Time (if repeating)
	 */
	public static final String RULE_LAST_EXE_TIME = "Rule Execution Last Execution Time (if repeating)";

	/**
	 * Rule Execution Status
	 */
	public static final String RULE_STATUS = "Rule Execution Status";

	/**
	 * iRODS AVU, user definable metadata. When used as a select, will return
	 * all the attribute names of all file AVU which match the query conditions.
	 */
	public static final String META_DATA_ATTR_NAME = "Definable metadata attribute name for files";
	/**
	 * iRODS AVU, user definable metadata. When used as a select, will return
	 * all the attribute values of all file AVU which match the query
	 * conditions.
	 */
	public static final String META_DATA_ATTR_VALUE = "Definable metadata value name for files";
	/**
	 * iRODS AVU, user definable metadata. When used as a select, will return
	 * all the attribute units of all file AVU which match the query conditions.
	 */
	public static final String META_DATA_ATTR_UNITS = "Definable metadata units name for files";

	/**
	 * iRODS AVU, user definable metadata. When used as a select, will return
	 * all the attribute names of all collection AVU which match the query
	 * conditions.
	 */
	public static final String META_COLL_ATTR_NAME = "Definable metadata attribute name for collections";
	/**
	 * iRODS AVU, user definable metadata. When used as a select, will return
	 * all the attribute values of all collection AVU which match the query
	 * conditions.
	 */
	public static final String META_COLL_ATTR_VALUE = "Definable metadata value name for collections";
	/**
	 * iRODS AVU, user definable metadata. When used as a select, will return
	 * all the attribute units of all collection AVU which match the query
	 * conditions.
	 */
	public static final String META_COLL_ATTR_UNITS = "Definable metadata units name for collections";

	/**
	 * iRODS AVU, user definable metadata. When used as a select, will return
	 * all the attribute names of all resource AVU which match the query
	 * conditions.
	 */
	public static final String META_RESOURCE_ATTR_NAME = "Definable metadata attribute name for resources";
	/**
	 * iRODS AVU, user definable metadata. When used as a select, will return
	 * all the attribute values of all resource AVU which match the query
	 * conditions.
	 */
	public static final String META_RESOURCE_ATTR_VALUE = "Definable metadata value name for resources";
	/**
	 * iRODS AVU, user definable metadata. When used as a select, will return
	 * all the attribute units of all resource AVU which match the query
	 * conditions.
	 */
	public static final String META_RESOURCE_ATTR_UNITS = "Definable metadata units name for resources";

	/**
	 * iRODS AVU, user definable metadata. When used as a select, will return
	 * all the attribute names of all user AVU which match the query conditions.
	 */
	public static final String META_USER_ATTR_NAME = "Definable metadata attribute name for users";
	/**
	 * iRODS AVU, user definable metadata. When used as a select, will return
	 * all the attribute values of all user AVU which match the query
	 * conditions.
	 */
	public static final String META_USER_ATTR_VALUE = "Definable metadata value name for users";
	/**
	 * iRODS AVU, user definable metadata. When used as a select, will return
	 * all the attribute units of all user AVU which match the query conditions.
	 */
	public static final String META_USER_ATTR_UNITS = "Definable metadata units name for users";

	static final String FILE_ID = "File Identifier";
	static final String DIRECTORY_ID = "Directory Identifier";
	static final String RESOURCE_ID = "Resource Identifier";
	static final String USER_ID = "User Identifier";
	static final String ZONE_ID = "Zone Identifier";

	// R_TOKN_MAIN
	static final String TOKEN_NAMESPACE = "Token Namespace";
	static final String TOKEN_ID = "Token Identifier";
	static final String TOKEN_NAME = "Token Name";
	static final String TOKEN_VALUE = "Token Value (meaning depends on type of token)";
	static final String TOKEN_VALUE2 = "Token Value2 (meaning depends on type of token)";
	static final String TOKEN_VALUE3 = "Token Value3 (meaning depends on type of token)";
	static final String TOKEN_COMMENT = "Token Comment";

	static final String META_NAMESPACE_COLL = "Token Comment";
	static final String META_NAMESPACE_DATA = "Token Comment";
	static final String META_NAMESPACE_RESC = "Token Comment";
	static final String META_NAMESPACE_USER = "Token Comment";

	/*
	 * These are the values used in general-query and general-update to specify
	 * the columns. The site-extension tables should start at 10,000; and the
	 * core tables use valued less than 10,000:
	 */

	private static int TOTAL_METADATA_FIELDS = 500;
	private static HashMap<String, Integer> jargonToIRODS = new HashMap<String, Integer>(
			TOTAL_METADATA_FIELDS);
	private static HashMap<Integer, String> iRODSToJargon = new HashMap<Integer, String>(
			TOTAL_METADATA_FIELDS);

	static IRODSProtocol protocol;

	final static String DEFINABLE_METADATA = "jargonUserDefinableAttribute";
	final static String EXTENSIBLE_METADATA = "jargonExtensibleAttribute";

	private static final int EXTENSIBLE_START_NUM = 10001;

	/*
	 * These are the Table Column names used with the GenQuery. Also see the
	 * rcatGeneralQuerySetup routine which associates these values with tables
	 * and columns.
	 */
	/* R_ZONE_MAIN: */
	/** Zone Identifier (a number) */
	static final Integer COL_ZONE_ID = new Integer(101);

	/** Zone Name */
	static final Integer COL_ZONE_NAME = new Integer(102);

	/* R_USER_MAIN: */
	/** User Identifier (a number) */
	static final Integer COL_USER_ID = new Integer(201);

	/** User Name */
	static final Integer COL_USER_NAME = new Integer(202);

	/** User Type */
	static final Integer COL_USER_TYPE = new Integer(203);

	/** User Zone */
	static final Integer COL_USER_ZONE = new Integer(204);

	/**
	 * User GSI Distinguished Name, the COL_USER_DN value is the current value
	 * in the irods 2.2 api, the COL_USER_DN_INVALID is for backwards
	 * compatability, and is deprecated as of irods 2.2
	 */
	static final Integer COL_USER_DN_INVALID = new Integer(205);
	static final Integer COL_USER_DN = new Integer(1601);

	/** User Information (set by the admin, like comment) */
	static final Integer COL_USER_INFO = new Integer(206);

	/** User Comment (also set by the admin) */
	static final Integer COL_USER_COMMENT = new Integer(207);

	/** Time that this record was created (Unix Time) */
	static final Integer COL_USER_CREATE_TIME = new Integer(208);

	/** Time that this record was last modified (Unix Time) */
	static final Integer COL_USER_MODIFY_TIME = new Integer(209);

	/* R_RESC_MAIN: */
	/** Resource Identifier (a number) */
	static final Integer COL_R_RESC_ID = new Integer(301);

	/** Resource Name */
	static final Integer COL_R_RESC_NAME = new Integer(302);

	/** Zone Name */
	static final Integer COL_R_ZONE_NAME = new Integer(303);

	/** Resource Type */
	static final Integer COL_R_TYPE_NAME = new Integer(304);

	/** Resource Class */
	static final Integer COL_R_CLASS_NAME = new Integer(305);

	/** Resource Location (DNS hostname) */
	static final Integer COL_R_LOC = new Integer(306);

	/** Resource Vault Path (where physical files are stored) */
	static final Integer COL_R_VAULT_PATH = new Integer(307);

	/** Resource Free Space */
	static final Integer COL_R_FREE_SPACE = new Integer(308);

	/** Resource Free Space Time */
	static final Integer COL_R_FREE_SPACE_TIME = new Integer(314);

	/** Resource Information (set by admin) */
	static final Integer COL_R_RESC_INFO = new Integer(309);

	/** Resource Comment (set by admin) */
	static final Integer COL_R_RESC_COMMENT = new Integer(310);

	/** Time that this record was created (Unix Time) */
	static final Integer COL_R_CREATE_TIME = new Integer(311);

	/** Time that this record was last modified (Unix Time) */
	static final Integer COL_R_MODIFY_TIME = new Integer(312);

	/** Status of the resource */
	static final Integer COL_R_RESC_STATUS = new Integer(313);

	/* R_DATA_MAIN: */
	/** DataObject(file) Identifier (a number) */
	static final Integer COL_D_DATA_ID = new Integer(401);

	/** DataObject Collection ID (where this file is) */
	static final Integer COL_D_COLL_ID = new Integer(402);

	/** DataObject Name */
	static final Integer COL_DATA_NAME = new Integer(403);

	/** DataObject Replica Number (0 to N) */
	static final Integer COL_DATA_REPL_NUM = new Integer(404);

	/** DataObject Version */
	static final Integer COL_DATA_VERSION = new Integer(405);

	/** DataObject Type */
	static final Integer COL_DATA_TYPE_NAME = new Integer(406);

	/** DataObject Size */
	static final Integer COL_DATA_SIZE = new Integer(407);

	/** DataObject Resource Group Name */
	static final Integer COL_D_RESC_GROUP_NAME = new Integer(408);

	/** DataObject Resource Name */
	static final Integer COL_D_RESC_NAME = new Integer(409);

	/** DataObject Path (physical location) */
	static final Integer COL_D_DATA_PATH = new Integer(410);

	/** DataObject Owner Name */
	static final Integer COL_D_OWNER_NAME = new Integer(411);

	/** DataObject Owner Zone */
	static final Integer COL_D_OWNER_ZONE = new Integer(412);

	/**
	 * DataObject Replica Status (?I believe this indicates if this replica is
	 * current)
	 */
	static final Integer COL_D_REPL_STATUS = new Integer(413);

	/** DataObject Data Status (?) */
	static final Integer COL_D_DATA_STATUS = new Integer(414);

	/** DataObject Checksum */
	static final Integer COL_D_DATA_CHECKSUM = new Integer(415);

	/** DataObject Expiration time (currently unused) */
	static final Integer COL_D_EXPIRY = new Integer(416);

	/** DataObject Map Identifier (?) */
	static final Integer COL_D_MAP_ID = new Integer(417);

	/** DataObject Comment (unused, I believe) */
	static final Integer COL_D_COMMENTS = new Integer(418);

	/** Time that this record was created (Unix Time) */
	static final Integer COL_D_CREATE_TIME = new Integer(419);

	/** Time that this record was last modified (Unix Time) */
	static final Integer COL_D_MODIFY_TIME = new Integer(420);

	/* R_COLL_MAIN */
	/** Collection (directory) Identifier */
	static final Integer COL_COLL_ID = new Integer(500);

	/** Collection Name */
	static final Integer COL_COLL_NAME = new Integer(501);

	/** Collection Parent Name (up one level) */
	static final Integer COL_COLL_PARENT_NAME = new Integer(502);

	/** Collection Owner Name */
	static final Integer COL_COLL_OWNER_NAME = new Integer(503);

	/** Collection Owner Zone */
	static final Integer COL_COLL_OWNER_ZONE = new Integer(504);

	/** Collection Map ID (currently unused) */
	static final Integer COL_COLL_MAP_ID = new Integer(505);

	/** Collection Inheritance (?) */
	static final Integer COL_COLL_INHERITANCE = new Integer(506);

	/** Collection Comments */
	static final Integer COL_COLL_COMMENTS = new Integer(507);

	/** Time that this record was created (Unix Time) */
	static final Integer COL_COLL_CREATE_TIME = new Integer(508);

	/** Time that this record was last modified (Unix Time) */
	static final Integer COL_COLL_MODIFY_TIME = new Integer(509);

	static final Integer COL_COLL_TYPE = new Integer(510);
	static final Integer COL_COLL_INFO1 = new Integer(511);
	static final Integer COL_COLL_INFO2 = new Integer(512);

	/* R_META_MAIN */
	/** Metadata for a DataObject: Attribute Name */
	public static final Integer COL_META_DATA_ATTR_NAME = new Integer(600);

	/** Metadata for a DataObject: Attribute Value */
	public static final Integer COL_META_DATA_ATTR_VALUE = new Integer(601);

	/** Metadata for a DataObject: Attribute Units */
	static final Integer COL_META_DATA_ATTR_UNITS = new Integer(602);

	static final Integer COL_META_DATA_ATTR_ID = new Integer(603);

	/** Metadata for a Collection: Attribute Name */
	public static final Integer COL_META_COLL_ATTR_NAME = new Integer(610);

	/** Metadata for a Collection: Attribute Value */
	public static final Integer COL_META_COLL_ATTR_VALUE = new Integer(611);

	/** Metadata for a Collection: Attribute Units */
	static final Integer COL_META_COLL_ATTR_UNITS = new Integer(612);

	static final Integer COL_META_COLL_ATTR_ID = new Integer(613);

	/** (Used internally to ICAT) */
	static final Integer COL_META_NAMESPACE_COLL = new Integer(620);

	/** (Used internally to ICAT) */
	static final Integer COL_META_NAMESPACE_DATA = new Integer(621);

	/** (Used internally to ICAT) */
	static final Integer COL_META_NAMESPACE_RESC = new Integer(622);

	/** (Used internally to ICAT) */
	static final Integer COL_META_NAMESPACE_USER = new Integer(623);

	/** Metadata for a Resource: Attribute Name */
	public static final Integer COL_META_RESC_ATTR_NAME = new Integer(630);

	/** Metadata for a Resource: Attribute Value */
	public static final Integer COL_META_RESC_ATTR_VALUE = new Integer(631);

	/** Metadata for a Resource: Attribute Units */
	static final Integer COL_META_RESC_ATTR_UNITS = new Integer(632);

	static final Integer COL_META_RESC_ATTR_ID = new Integer(633);

	/** Metadata for a User: Attribute Name */
	public static final Integer COL_META_USER_ATTR_NAME = new Integer(640);

	/** Metadata for a User: Attribute Value */
	public static final Integer COL_META_USER_ATTR_VALUE = new Integer(641);

	/** Metadata for a User: Attribute Units */
	static final Integer COL_META_USER_ATTR_UNITS = new Integer(642);

	static final Integer COL_META_USER_ATTR_ID = new Integer(643);

	/* R_OBJT_ACCESS */
	/** Data Access Type: a number corresponding to access permissions */
	static final Integer COL_DATA_ACCESS_TYPE = new Integer(700);

	/** Data Access Name: a name corresponding to access permissions */
	static final Integer COL_DATA_ACCESS_NAME = new Integer(701);

	/** Data Access Token Namespace (used internally to specify name types) */
	static final Integer COL_DATA_TOKEN_NAMESPACE = new Integer(702);

	/** Data Access User Identifier */
	static final Integer COL_DATA_ACCESS_USER_ID = new Integer(703);

	/** Data Access Data Identifier */
	static final Integer COL_DATA_ACCESS_DATA_ID = new Integer(704);

	static final Integer COL_COLL_ACCESS_TYPE = new Integer(710);
	/** Collection Access Name: a name corresponding to access permissions */
	static final Integer COL_COLL_ACCESS_NAME = new Integer(711);
	static final Integer COL_COLL_TOKEN_NAMESPACE = new Integer(712);
	static final Integer COL_COLL_ACCESS_USER_ID = new Integer(713);
	static final Integer COL_COLL_ACCESS_COLL_ID = new Integer(714);

	/* R_RESC_GROUP */
	/** Resource Group Resource Identifier */
	static final Integer COL_RESC_GROUP_RESC_ID = new Integer(800);

	/** Resource Group Resource Name */
	static final Integer COL_RESC_GROUP_NAME = new Integer(801);

	/* R_USER_GROUP USER */
	/** User Group Identifier */
	static final Integer COL_USER_GROUP_ID = new Integer(900);

	/** User Group Name */
	static final Integer COL_USER_GROUP_NAME = new Integer(901);

	/* R_RULE_EXEC RuleExecInfo Structure */
	/** Rule Execution Identifier */
	static final Integer COL_RULE_EXEC_ID = new Integer(1000);

	/** Rule Execution Name */
	static final Integer COL_RULE_EXEC_NAME = new Integer(1001);

	/** Rule Execution REI (Rule Execution Information) File Path */
	static final Integer COL_RULE_EXEC_REI_FILE_PATH = new Integer(1002);

	/** Rule Execution User Name */
	static final Integer COL_RULE_EXEC_USER_NAME = new Integer(1003);

	/** Rule Execution Address (hostname, if specified) */
	static final Integer COL_RULE_EXEC_ADDRESS = new Integer(1004);

	/** Rule Execution Time (when to execute) */
	static final Integer COL_RULE_EXEC_TIME = new Integer(1005);

	/** Rule Execution Frequency (when to repeat (seconds)) */
	static final Integer COL_RULE_EXEC_FREQUENCY = new Integer(1006);

	/** Rule Execution Priority */
	static final Integer COL_RULE_EXEC_PRIORITY = new Integer(1007);

	/** Rule Execution Estimated Exectuion Time (if specified) */
	static final Integer COL_RULE_EXEC_ESTIMATED_EXE_TIME = new Integer(1008);

	/** Rule Execution Notification Address (if specified) */
	static final Integer COL_RULE_EXEC_NOTIFICATION_ADDR = new Integer(1009);

	/** Rule Execution Last Execution Time (if repeating) */
	static final Integer COL_RULE_EXEC_LAST_EXE_TIME = new Integer(1010);

	/** Rule Execution Status */
	static final Integer COL_RULE_EXEC_STATUS = new Integer(1011);

	/* R_TOKN_MAIN */
	/** Token Namespace */
	static final Integer COL_TOKEN_NAMESPACE = new Integer(1100);

	/** Token Identifier */
	static final Integer COL_TOKEN_ID = new Integer(1101);

	/** Token Name */
	static final Integer COL_TOKEN_NAME = new Integer(1102);

	/** Token Value (meaning depends on type of token) */
	static final Integer COL_TOKEN_VALUE = new Integer(1103);

	/** Token Value2 (meaning depends on type of token) */
	static final Integer COL_TOKEN_VALUE2 = new Integer(1104);

	/** Token Value3 (meaning depends on type of token) */
	static final Integer COL_TOKEN_VALUE3 = new Integer(1105);

	/** Token Comment */
	static final Integer COL_TOKEN_COMMENT = new Integer(1106);

	/* R_OBJT_AUDIT */
	static final Integer COL_AUDIT_OBJ_ID = new Integer(1200);
	static final Integer COL_AUDIT_USER_ID = new Integer(1201);
	static final Integer COL_AUDIT_ACTION_ID = new Integer(1202);
	static final Integer COL_AUDIT_COMMENT = new Integer(1203);
	static final Integer COL_AUDIT_CREATE_TIME = new Integer(1204);
	static final Integer COL_AUDIT_MODIFY_TIME = new Integer(1205);

	/* R_COLL_USER_MAIN (r_user_main for Collection information) */
	static final Integer COL_COLL_USER_NAME = new Integer(1300);
	static final Integer COL_COLL_USER_ZONE = new Integer(1301);

	/* R_DATA_USER_MAIN (r_user_main for Data information specifically) */
	static final Integer COL_DATA_USER_NAME = new Integer(1310);
	static final Integer COL_DATA_USER_ZONE = new Integer(1311);

	/*
	 * R_SERVER_LOAD * #define COL_SL_HOST_NAME 1400 #define COL_SL_RESC_NAME
	 * 1401 #define COL_SL_CPU_USED 1402 #define COL_SL_MEM_USED 1403 #define
	 * COL_SL_SWAP_USED 1404 #define COL_SL_RUNQ_LOAD 1405 #define
	 * COL_SL_DISK_SPACE 1406 #define COL_SL_NET_INPUT 1407 #define
	 * COL_SL_NET_OUTPUT 1408 #define COL_SL_NET_OUTPUT 1408 #define
	 * COL_SL_CREATE_TIME 1409
	 * 
	 * /* R_SERVER_LOAD_DIGEST * #define COL_SLD_RESC_NAME 1500 #define
	 * COL_SLD_LOAD_FACTOR 1501 #define COL_SLD_CREATE_TIME 1502
	 */

	static final String ALL_KW = "all"; // operation done on all replica
	static final String COPIES_KW = "copies"; // the number of copies
	static final String EXEC_LOCALLY_KW = "execLocally"; // execute locally
	static final String FORCE_FLAG_KW = "forceFlag"; // force update
	static final String CLI_IN_SVR_FIREWALL_KW = "cliInSvrFirewall";
	static final String REG_CHKSUM_KW = "regChksum"; // register checksum
	static final String VERIFY_CHKSUM_KW = "verifyChksum"; // verify checksum
	static final String VERIFY_BY_SIZE_KW = "verifyBySize"; // verify by size -
	// used by irsync
	static final String OBJ_PATH_KW = "objPath"; // logical path of the object
	static final String RESC_NAME_KW = "rescName"; // resource name
	static final String DEST_RESC_NAME_KW = "destRescName"; // destination
	// resource name
	static final String BACKUP_RESC_NAME_KW = "backupRescName"; // destination
	// resource name
	static final String DATA_TYPE_KW = "dataType"; // data type
	static final String DATA_SIZE_KW = "dataSize";
	static final String CHKSUM_KW = "chksum";
	static final String VERSION_KW = "version";
	static final String FILE_PATH_KW = "filePath"; // the physical file path
	static final String REPL_NUM_KW = "replNum"; // replica number
	static final String REPL_STATUS_KW = "replStatus"; // status of the replica
	static final String ALL_REPL_STATUS_KW = "allReplStatus"; // update all
	// replStatus
	static final String DATA_INCLUDED_KW = "dataIncluded"; // data included in
	// the input buffer
	static final String DATA_OWNER_KW = "dataOwner";
	static final String DATA_OWNER_ZONE_KW = "dataOwnerZone";
	static final String DATA_EXPIRY_KW = "dataExpiry";
	static final String DATA_COMMENTS_KW = "dataComments";
	static final String DATA_CREATE_KW = "dataCreate";
	static final String DATA_MODIFY_KW = "dataModify";
	static final String DATA_ACCESS_KW = "dataAccess";
	static final String DATA_ACCESS_INX_KW = "dataAccessInx";
	static final String NO_OPEN_FLAG_KW = "noOpenFlag";
	static final String STREAMING_KW = "streaming";
	static final String DATA_ID_KW = "dataId";
	static final String COLL_ID_KW = "collId";
	static final String RESC_GROUP_NAME_KW = "rescGroupName";
	static final String STATUS_STRING_KW = "statusString";
	static final String DATA_MAP_ID_KW = "dataMapId";
	static final String NO_PARA_OP_KW = "noParaOpr";
	static final String LOCAL_PATH_KW = "localPath";
	static final String RSYNC_MODE_KW = "rsyncMode";
	static final String RSYNC_DEST_PATH_KW = "rsyncDestPath";
	static final String RSYNC_CHKSUM_KW = "rsyncChksum";
	static final String CHKSUM_ALL_KW = "ChksumAll";
	static final String FORCE_CHKSUM_KW = "forceChksum";
	static final String COLLECTION_KW = "collection";
	static final String IRODS_ADMIN_KW = "irodsAdmin";
	static final String IRODS_ADMIN_RMTRASH_KW = "irodsAdminRmTrash";
	static final String IRODS_RMTRASH_KW = "irodsRmTrash";
	static final String RECURSIVE_OPR__KW = "recursiveOpr";

	// The following are the keyWord definition for the rescCond key/value pair
	// RESC_NAME_KW is defined above

	static final String RESC_ZONE_KW = "zoneName";
	static final String RESC_LOC_KW = "rescLoc";
	static final String RESC_TYPE_KW = "rescType";
	static final String RESC_CLASS_KW = "rescClass";
	static final String RESC_VAULT_PATH_KW = "rescVaultPath";
	static final String NUM_OPEN_PORTS_KW = "numOpenPorts";
	static final String PARA_OPR_KW = "paraOpr";
	static final String GATEWAY_ADDR_KW = "gateWayAddr";
	static final String RESC_MAX_OBJ_SIZE_KW = "rescMaxObjSize";
	static final String FREE_SPACE_KW = "freeSpace";
	static final String FREE_SPACE_TIME_KW = "freeSpaceTime";
	static final String FREE_SPACE_TIMESTAMP_KW = "freeSpaceTimeStamp";
	static final String RESC_TYPE_INX_KW = "rescTypeInx";
	static final String RESC_CLASS_INX_KW = "rescClassInx";
	static final String RESC_ID_KW = "rescId";
	static final String RESC_INFO_KW = "rescInfo";
	static final String RESC_COMMENTS_KW = "rescComments";
	static final String RESC_CREATE_KW = "rescCreate";
	static final String RESC_MODIFY_KW = "rescModify";

	// The following are the keyWord definition for the userCond key/value pair
	static final String USER_NAME_CLIENT_KW = "userNameClient";
	static final String RODS_ZONE_CLIENT_KW = "rodsZoneClient";
	static final String HOST_CLIENT_KW = "hostClient";
	static final String USER_TYPE_CLIENT_KW = "userTypeClient";
	static final String AUTH_STR_CLIENT_KW = "authStrClient";
	static final String USER_AUTH_SCHEME_CLIENT_KW = "userAuthSchemeClient";
	static final String USER_INFO_CLIENT_KW = "userInfoClient";
	static final String USER_COMMENT_CLIENT_KW = "userCommentClient";
	static final String USER_CREATE_CLIENT_KW = "userCreateClient";
	static final String USER_MODIFY_CLIENT_KW = "userModifyClient";
	static final String USER_NAME_PROXY_KW = "userNameProxy";
	static final String RODS_ZONE_PROXY_KW = "rodsZoneProxy";
	static final String HOST_PROXY_KW = "hostProxy";
	static final String USER_TYPE_PROXY_KW = "userTypeProxy";
	static final String AUTH_STR_PROXY_KW = "authStrProxy";
	static final String USER_AUTH_SCHEME_PROXY_KW = "userAuthSchemeProxy";
	static final String USER_INFO_PROXY_KW = "userInfoProxy";
	static final String USER_COMMENT_PROXY_KW = "userCommentProxy";
	static final String USER_CREATE_PROXY_KW = "userCreateProxy";
	static final String USER_MODIFY_PROXY_KW = "userModifyProxy";
	static final String ACCESS_PERMISSION_KW = "accessPermission";

	// The following are the keyWord definition for the collCond key/value pair

	static final String COLL_NAME_KW = "collName";
	static final String COLL_PARENT_NAME_KW = "collParentName";
	static final String COLL_OWNER_NAME_KW = "collOwnername";
	static final String COLL_OWNER_ZONE_KW = "collOwnerZone";
	static final String COLL_MAP_ID_KW = "collMapId";
	static final String COLL_INHERITANCE_KW = "collInheritance";
	static final String COLL_COMMENTS_KW = "collComments";
	static final String COLL_EXPIRY_KW = "collExpiry";
	static final String COLL_CREATE_KW = "collCreate";
	static final String COLL_MODIFY_KW = "collModify";
	static final String COLL_ACCESS_KW = "collAccess";
	static final String COLL_ACCESS_INX_KW = "collAccessInx";

	// The following are the keyWord definitions for the keyValPair_t input
	// to chlModRuleExec.

	static final String RULE_NAME_KW = "ruleName";
	static final String RULE_REI_FILE_PATH_KW = "reiFilePath";
	static final String RULE_USER_NAME_KW = "userName";
	static final String RULE_EXE_ADDRESS_KW = "exeAddress";
	static final String RULE_EXE_TIME_KW = "exeTime";
	static final String RULE_EXE_FREQUENCY_KW = "exeFrequency";
	static final String RULE_PRIORITY_KW = "priority";
	static final String RULE_ESTIMATE_EXE_TIME_KW = "estimateExeTime";
	static final String RULE_NOTIFICATION_ADDR_KW = "notificationAddr";
	static final String RULE_LAST_EXE_TIME_KW = "lastExeTime";
	static final String RULE_EXE_STATUS_KW = "exeStatus";

	private static ExtensibleMetaDataMapping extensibleMetaDataMapping = null;

	static {

		if (protocol == null) {
			protocol = new IRODSProtocol();
		}

		/*
		 * the extensible metadata mapping is a static field, due to how
		 * IRODSMetaDataSet and IRODSProtocol are constructed using static
		 * initializers. if the extensible metadata mapping is not already
		 * stored, a singleton ExtensibleMetaDataMapping will be consulted for
		 * either a pre-initialized value, or a value derived with default
		 * behavior, which is a properties file on the classpath.
		 */

		if (extensibleMetaDataMapping == null) {
			log.debug("looking up extensibleMetaDataMapping from singleton");
			try {
				extensibleMetaDataMapping = ExtensibleMetaDataMapping
						.instance();
			} catch (JargonException e) {

				log.error("exception deriving extensibleMetaDataMapping", e);
				throw new RuntimeException(e);
			}
			log.debug("IRODSMetaDataSet now has a cached ExtensibleMetaDataMapping");
		}

		// R_ZONE_MAIN:
		jargonToIRODS.put(ZONE_NAME, COL_ZONE_NAME);

		// R_USER_MAIN:
		jargonToIRODS.put(USER_ID, COL_USER_ID);
		jargonToIRODS.put(USER_NAME, COL_USER_NAME);
		jargonToIRODS.put(USER_TYPE, COL_USER_TYPE);
		jargonToIRODS.put(USER_ZONE, COL_USER_ZONE);
		jargonToIRODS.put(USER_DN, COL_USER_DN);
		jargonToIRODS.put(USER_DN_2_1, COL_USER_DN_INVALID);
		jargonToIRODS.put(USER_INFO, COL_USER_INFO);
		jargonToIRODS.put(USER_COMMENT, COL_USER_COMMENT);
		jargonToIRODS.put(USER_CREATE_DATE, COL_USER_CREATE_TIME);
		jargonToIRODS.put(USER_MODIFY_DATE, COL_USER_MODIFY_TIME);

		// R_RESC_MAIN:
		jargonToIRODS.put(RESOURCE_ID, COL_R_RESC_ID);
		jargonToIRODS.put(COLL_RESOURCE_NAME, COL_R_RESC_NAME);
		jargonToIRODS.put(RESOURCE_ZONE, COL_R_ZONE_NAME);
		jargonToIRODS.put(RESOURCE_TYPE, COL_R_TYPE_NAME);
		jargonToIRODS.put(RESOURCE_CLASS, COL_R_CLASS_NAME);
		jargonToIRODS.put(RESOURCE_LOCATION, COL_R_LOC);
		jargonToIRODS.put(RESOURCE_VAULT_PATH, COL_R_VAULT_PATH);
		jargonToIRODS.put(RESOURCE_FREE_SPACE, COL_R_FREE_SPACE);
		jargonToIRODS.put(RESOURCE_FREE_SPACE_TIME, COL_R_FREE_SPACE_TIME);
		jargonToIRODS.put(RESOURCE_INFO, COL_R_RESC_INFO);
		jargonToIRODS.put(RESOURCE_STATUS, COL_R_RESC_STATUS);
		jargonToIRODS.put(RESOURCE_COMMENTS, COL_R_RESC_COMMENT);
		jargonToIRODS.put(RESOURCE_CREATE_DATE, COL_R_CREATE_TIME);
		jargonToIRODS.put(RESOURCE_MODIFY_DATE, COL_R_MODIFY_TIME);

		// R_DATA_MAIN:
		jargonToIRODS.put(FILE_ID, COL_D_DATA_ID);
		jargonToIRODS.put(FILE_NAME, COL_DATA_NAME);
		jargonToIRODS.put(FILE_REPLICA_NUM, COL_DATA_REPL_NUM);
		jargonToIRODS.put(FILE_VERSION, COL_DATA_VERSION);
		jargonToIRODS.put(FILE_TYPE, COL_DATA_TYPE_NAME);
		jargonToIRODS.put(SIZE, COL_DATA_SIZE);
		jargonToIRODS.put(FILE_RESOURCE_GROUP, COL_D_RESC_GROUP_NAME);
		jargonToIRODS.put(RESOURCE_NAME, COL_D_RESC_NAME);
		jargonToIRODS.put(PATH_NAME, COL_D_DATA_PATH);
		jargonToIRODS.put(OWNER, COL_D_OWNER_NAME);
		jargonToIRODS.put(FILE_OWNER_ZONE, COL_D_OWNER_ZONE);
		jargonToIRODS.put(FILE_REPLICA_STATUS, COL_D_REPL_STATUS);
		jargonToIRODS.put(FILE_STATUS, COL_D_DATA_STATUS);
		jargonToIRODS.put(FILE_CHECKSUM, COL_D_DATA_CHECKSUM);
		jargonToIRODS.put(FILE_EXPIRY, COL_D_EXPIRY);
		jargonToIRODS.put(FILE_MAP_ID, COL_D_MAP_ID);
		jargonToIRODS.put(FILE_COMMENTS, COL_D_COMMENTS);
		jargonToIRODS.put(CREATION_DATE, COL_D_CREATE_TIME);
		jargonToIRODS.put(MODIFICATION_DATE, COL_D_MODIFY_TIME);

		// R_COLL_MAIN
		jargonToIRODS.put(DIRECTORY_NAME, COL_COLL_NAME);
		jargonToIRODS.put(PARENT_DIRECTORY_NAME, COL_COLL_PARENT_NAME);
		jargonToIRODS.put(DIRECTORY_OWNER, COL_COLL_OWNER_NAME);
		jargonToIRODS.put(DIRECTORY_OWNER_ZONE, COL_COLL_OWNER_ZONE);
		jargonToIRODS.put(DIRECTORY_INHERITANCE, COL_COLL_INHERITANCE);
		jargonToIRODS.put(DIRECTORY_COMMENTS, COL_COLL_COMMENTS);
		jargonToIRODS.put(DIRECTORY_CREATE_DATE, COL_COLL_CREATE_TIME);
		jargonToIRODS.put(DIRECTORY_CREATE_TIMESTAMP, COL_COLL_CREATE_TIME);
		jargonToIRODS.put(DIRECTORY_MODIFY_DATE, COL_COLL_MODIFY_TIME);
		jargonToIRODS.put(DIRECTORY_TYPE, COL_COLL_TYPE);

		// R_META_MAIN
		jargonToIRODS.put(META_DATA_ATTR_NAME, COL_META_DATA_ATTR_NAME);
		jargonToIRODS.put(META_DATA_ATTR_VALUE, COL_META_DATA_ATTR_VALUE);
		jargonToIRODS.put(META_DATA_ATTR_UNITS, COL_META_DATA_ATTR_UNITS);

		jargonToIRODS.put(META_COLL_ATTR_NAME, COL_META_COLL_ATTR_NAME);
		jargonToIRODS.put(META_COLL_ATTR_VALUE, COL_META_COLL_ATTR_VALUE);
		jargonToIRODS.put(META_COLL_ATTR_UNITS, COL_META_COLL_ATTR_UNITS);

		jargonToIRODS.put(META_NAMESPACE_COLL, COL_META_NAMESPACE_COLL);
		jargonToIRODS.put(META_NAMESPACE_DATA, COL_META_NAMESPACE_DATA);
		jargonToIRODS.put(META_NAMESPACE_RESC, COL_META_NAMESPACE_RESC);
		jargonToIRODS.put(META_NAMESPACE_USER, COL_META_NAMESPACE_USER);

		jargonToIRODS.put(META_RESOURCE_ATTR_NAME, COL_META_RESC_ATTR_NAME);
		jargonToIRODS.put(META_RESOURCE_ATTR_VALUE, COL_META_RESC_ATTR_VALUE);
		jargonToIRODS.put(META_RESOURCE_ATTR_UNITS, COL_META_RESC_ATTR_UNITS);

		jargonToIRODS.put(META_USER_ATTR_NAME, COL_META_USER_ATTR_NAME);
		jargonToIRODS.put(META_USER_ATTR_VALUE, COL_META_USER_ATTR_VALUE);
		jargonToIRODS.put(META_USER_ATTR_UNITS, COL_META_USER_ATTR_UNITS);

		// R_OBJT_ACCESS
		jargonToIRODS.put(FILE_ACCESS_TYPE, COL_DATA_ACCESS_TYPE);
		jargonToIRODS.put(FILE_ACCESS_NAME, COL_DATA_ACCESS_NAME);
		jargonToIRODS.put(FILE_ACCESS_USER_ID, COL_DATA_ACCESS_USER_ID);
		jargonToIRODS.put(FILE_ACCESS_DATA_ID, COL_DATA_ACCESS_DATA_ID);

		// id of the access type allowed
		jargonToIRODS.put(DIRECTORY_ACCESS_TYPE, COL_COLL_ACCESS_TYPE);
		jargonToIRODS.put(DIRECTORY_ACCESS_CONSTRAINT, COL_COLL_ACCESS_NAME);
		jargonToIRODS.put(DIRECTORY_TOKEN_NAMESPACE, COL_COLL_TOKEN_NAMESPACE);

		// R_RESC_GROUP
		jargonToIRODS.put(RESOURCE_GROUP_ID, COL_RESC_GROUP_RESC_ID);
		jargonToIRODS.put(RESOURCE_GROUP, COL_RESC_GROUP_NAME);

		// R_USER_GROUP / USER
		jargonToIRODS.put(USER_GROUP, COL_USER_GROUP_NAME);

		// R_RULE_EXEC
		jargonToIRODS.put(RULE_NAME, COL_RULE_EXEC_NAME);
		jargonToIRODS.put(RULE_REI_FILE_PATH, COL_RULE_EXEC_REI_FILE_PATH);
		jargonToIRODS.put(RULE_USER_NAME, COL_RULE_EXEC_USER_NAME);
		jargonToIRODS.put(RULE_ADDRESS, COL_RULE_EXEC_ADDRESS);
		jargonToIRODS.put(RULE_TIME, COL_RULE_EXEC_TIME);
		jargonToIRODS.put(RULE_FREQUENCY, COL_RULE_EXEC_FREQUENCY);
		jargonToIRODS.put(RULE_PRIORITY, COL_RULE_EXEC_PRIORITY);
		jargonToIRODS.put(RULE_ESTIMATED_EXE_TIME,
				COL_RULE_EXEC_ESTIMATED_EXE_TIME);
		jargonToIRODS.put(RULE_NOTIFICATION_ADDR,
				COL_RULE_EXEC_NOTIFICATION_ADDR);
		jargonToIRODS.put(RULE_LAST_EXE_TIME, COL_RULE_EXEC_LAST_EXE_TIME);
		jargonToIRODS.put(RULE_STATUS, COL_RULE_EXEC_STATUS);

		// R_TOKN_MAIN
		jargonToIRODS.put(TOKEN_NAMESPACE, COL_TOKEN_NAMESPACE);
		jargonToIRODS.put(TOKEN_ID, COL_TOKEN_ID);
		jargonToIRODS.put(TOKEN_NAME, COL_TOKEN_NAME);
		jargonToIRODS.put(TOKEN_VALUE, COL_TOKEN_VALUE);
		jargonToIRODS.put(TOKEN_VALUE2, COL_TOKEN_VALUE2);
		jargonToIRODS.put(TOKEN_VALUE3, COL_TOKEN_VALUE3);
		jargonToIRODS.put(TOKEN_COMMENT, COL_TOKEN_COMMENT);

		/*
		 * Table: R_OBJT_AUDIT COL_AUDIT_OBJ_ID - object id upon which action is
		 * being recorded COL_AUDIT_USER_ID - user id performing the action
		 * COL_AUDIT_ACTION_ID - action identifier COL_AUDIT_COMMENT - comment
		 * COL_AUDIT_CREATE_TIME - creation timestamp COL_AUDIT_MODIFY_TIME -
		 * last modification timestamp
		 */
		// R_COLL_USER_MAIN
		jargonToIRODS.put(DIRECTORY_USER_NAME, COL_COLL_USER_NAME);
		jargonToIRODS.put(DIRECTORY_USER_ZONE, COL_COLL_USER_ZONE);

		// R_DATA_USER_MAIN (r_user_main for Data information specifically)
		jargonToIRODS.put(FILE_PERMISSION_USER_NAME, COL_DATA_USER_NAME);
		jargonToIRODS.put(FILE_PERMISSION_USER_ZONE, COL_DATA_USER_ZONE);

		// R_ZONE_MAIN:
		iRODSToJargon.put(COL_ZONE_ID, ZONE_ID);
		iRODSToJargon.put(COL_ZONE_NAME, ZONE_NAME);

		// R_USER_MAIN:
		iRODSToJargon.put(COL_USER_ID, USER_ID);
		iRODSToJargon.put(COL_USER_NAME, USER_NAME);
		iRODSToJargon.put(COL_USER_TYPE, USER_TYPE);
		iRODSToJargon.put(COL_USER_ZONE, USER_ZONE);
		iRODSToJargon.put(COL_USER_DN, USER_DN);
		iRODSToJargon.put(COL_USER_DN_INVALID, USER_DN_2_1);
		iRODSToJargon.put(COL_USER_INFO, USER_INFO);
		iRODSToJargon.put(COL_USER_COMMENT, USER_COMMENT);
		iRODSToJargon.put(COL_USER_CREATE_TIME, USER_CREATE_DATE);
		iRODSToJargon.put(COL_USER_MODIFY_TIME, USER_MODIFY_DATE);

		// R_RESC_MAIN:
		iRODSToJargon.put(COL_R_RESC_ID, RESOURCE_ID);
		iRODSToJargon.put(COL_R_RESC_NAME, RESOURCE_NAME);
		iRODSToJargon.put(COL_R_ZONE_NAME, RESOURCE_ZONE);
		iRODSToJargon.put(COL_R_TYPE_NAME, RESOURCE_TYPE);
		iRODSToJargon.put(COL_R_CLASS_NAME, RESOURCE_CLASS);
		iRODSToJargon.put(COL_R_LOC, RESOURCE_LOCATION);
		iRODSToJargon.put(COL_R_VAULT_PATH, RESOURCE_VAULT_PATH);
		iRODSToJargon.put(COL_R_FREE_SPACE, RESOURCE_FREE_SPACE);
		iRODSToJargon.put(COL_R_FREE_SPACE_TIME, RESOURCE_FREE_SPACE_TIME);
		iRODSToJargon.put(COL_R_RESC_INFO, RESOURCE_INFO);
		iRODSToJargon.put(COL_R_RESC_STATUS, RESOURCE_STATUS);
		iRODSToJargon.put(COL_R_RESC_COMMENT, RESOURCE_COMMENTS);
		iRODSToJargon.put(COL_R_CREATE_TIME, RESOURCE_CREATE_DATE);
		iRODSToJargon.put(COL_R_MODIFY_TIME, RESOURCE_MODIFY_DATE);

		// R_DATA_MAIN:
		iRODSToJargon.put(COL_D_DATA_ID, FILE_ID);
		iRODSToJargon.put(COL_DATA_NAME, FILE_NAME);
		iRODSToJargon.put(COL_DATA_REPL_NUM, FILE_REPLICA_NUM);
		iRODSToJargon.put(COL_DATA_VERSION, FILE_VERSION);
		iRODSToJargon.put(COL_DATA_TYPE_NAME, FILE_TYPE);
		iRODSToJargon.put(COL_DATA_SIZE, SIZE);
		iRODSToJargon.put(COL_D_RESC_GROUP_NAME, FILE_RESOURCE_GROUP);
		iRODSToJargon.put(COL_D_RESC_NAME, RESOURCE_NAME);
		iRODSToJargon.put(COL_D_DATA_PATH, PATH_NAME);
		iRODSToJargon.put(COL_D_OWNER_NAME, OWNER);
		iRODSToJargon.put(COL_D_OWNER_ZONE, FILE_OWNER_ZONE);
		iRODSToJargon.put(COL_D_REPL_STATUS, FILE_REPLICA_STATUS);
		iRODSToJargon.put(COL_D_DATA_STATUS, FILE_STATUS);
		iRODSToJargon.put(COL_D_DATA_CHECKSUM, FILE_CHECKSUM);
		iRODSToJargon.put(COL_D_EXPIRY, FILE_EXPIRY);
		iRODSToJargon.put(COL_D_MAP_ID, FILE_MAP_ID);
		iRODSToJargon.put(COL_D_COMMENTS, FILE_COMMENTS);
		iRODSToJargon.put(COL_D_CREATE_TIME, CREATION_DATE);
		iRODSToJargon.put(COL_D_MODIFY_TIME, MODIFICATION_DATE);

		// R_COLL_MAIN
		iRODSToJargon.put(COL_COLL_NAME, DIRECTORY_NAME);
		iRODSToJargon.put(COL_COLL_PARENT_NAME, PARENT_DIRECTORY_NAME);
		iRODSToJargon.put(COL_COLL_OWNER_NAME, DIRECTORY_OWNER);
		iRODSToJargon.put(COL_COLL_OWNER_ZONE, DIRECTORY_OWNER_ZONE);
		iRODSToJargon.put(COL_COLL_INHERITANCE, DIRECTORY_INHERITANCE);
		iRODSToJargon.put(COL_COLL_COMMENTS, DIRECTORY_COMMENTS);
		iRODSToJargon.put(COL_COLL_CREATE_TIME, DIRECTORY_CREATE_TIMESTAMP);
		iRODSToJargon.put(COL_COLL_MODIFY_TIME, DIRECTORY_MODIFY_DATE);
		iRODSToJargon.put(COL_COLL_TYPE, DIRECTORY_TYPE);

		// R_META_MAIN
		iRODSToJargon.put(COL_META_DATA_ATTR_NAME, META_DATA_ATTR_NAME);
		iRODSToJargon.put(COL_META_DATA_ATTR_VALUE, META_DATA_ATTR_VALUE);
		iRODSToJargon.put(COL_META_DATA_ATTR_UNITS, META_DATA_ATTR_UNITS);

		iRODSToJargon.put(COL_META_COLL_ATTR_NAME, META_COLL_ATTR_NAME);
		iRODSToJargon.put(COL_META_COLL_ATTR_VALUE, META_COLL_ATTR_VALUE);
		iRODSToJargon.put(COL_META_COLL_ATTR_UNITS, META_COLL_ATTR_UNITS);

		iRODSToJargon.put(COL_META_NAMESPACE_COLL, META_NAMESPACE_COLL);
		iRODSToJargon.put(COL_META_NAMESPACE_DATA, META_NAMESPACE_DATA);
		iRODSToJargon.put(COL_META_NAMESPACE_RESC, META_NAMESPACE_RESC);
		iRODSToJargon.put(COL_META_NAMESPACE_USER, META_NAMESPACE_USER);

		iRODSToJargon.put(COL_META_RESC_ATTR_NAME, META_RESOURCE_ATTR_NAME);
		iRODSToJargon.put(COL_META_RESC_ATTR_VALUE, META_RESOURCE_ATTR_VALUE);
		iRODSToJargon.put(COL_META_RESC_ATTR_UNITS, META_RESOURCE_ATTR_UNITS);

		iRODSToJargon.put(COL_META_USER_ATTR_NAME, META_USER_ATTR_NAME);
		iRODSToJargon.put(COL_META_USER_ATTR_VALUE, META_USER_ATTR_VALUE);
		iRODSToJargon.put(COL_META_USER_ATTR_UNITS, META_USER_ATTR_UNITS);

		// R_OBJT_ACCESS
		iRODSToJargon.put(COL_DATA_ACCESS_TYPE, FILE_ACCESS_TYPE);
		iRODSToJargon.put(COL_DATA_ACCESS_NAME, FILE_ACCESS_NAME);
		iRODSToJargon.put(COL_DATA_ACCESS_USER_ID, FILE_ACCESS_USER_ID);
		iRODSToJargon.put(COL_DATA_ACCESS_DATA_ID, FILE_ACCESS_DATA_ID);

		// id of the access type allowed
		iRODSToJargon.put(COL_COLL_ACCESS_TYPE, DIRECTORY_ACCESS_TYPE);
		iRODSToJargon.put(COL_COLL_ACCESS_NAME, DIRECTORY_ACCESS_CONSTRAINT);
		iRODSToJargon.put(COL_COLL_TOKEN_NAMESPACE, DIRECTORY_TOKEN_NAMESPACE);

		// R_RESC_GROUP
		iRODSToJargon.put(COL_RESC_GROUP_RESC_ID, RESOURCE_GROUP_ID);
		iRODSToJargon.put(COL_RESC_GROUP_NAME, RESOURCE_GROUP);

		// R_USER_GROUP
		iRODSToJargon.put(COL_USER_GROUP_NAME, USER_GROUP);

		// R_RULE_EXEC
		iRODSToJargon.put(COL_RULE_EXEC_NAME, RULE_NAME);
		iRODSToJargon.put(COL_RULE_EXEC_REI_FILE_PATH, RULE_REI_FILE_PATH);
		iRODSToJargon.put(COL_RULE_EXEC_USER_NAME, RULE_USER_NAME);
		iRODSToJargon.put(COL_RULE_EXEC_ADDRESS, RULE_ADDRESS);
		iRODSToJargon.put(COL_RULE_EXEC_TIME, RULE_TIME);
		iRODSToJargon.put(COL_RULE_EXEC_FREQUENCY, RULE_FREQUENCY);
		iRODSToJargon.put(COL_RULE_EXEC_PRIORITY, RULE_PRIORITY);
		iRODSToJargon.put(COL_RULE_EXEC_ESTIMATED_EXE_TIME,
				RULE_ESTIMATED_EXE_TIME);
		iRODSToJargon.put(COL_RULE_EXEC_NOTIFICATION_ADDR,
				RULE_NOTIFICATION_ADDR);
		iRODSToJargon.put(COL_RULE_EXEC_LAST_EXE_TIME, RULE_LAST_EXE_TIME);
		iRODSToJargon.put(COL_RULE_EXEC_STATUS, RULE_STATUS);

		// R_TOKN_MAIN
		iRODSToJargon.put(COL_TOKEN_NAMESPACE, TOKEN_NAMESPACE);
		iRODSToJargon.put(COL_TOKEN_ID, TOKEN_ID);
		iRODSToJargon.put(COL_TOKEN_NAME, TOKEN_NAME);
		iRODSToJargon.put(COL_TOKEN_VALUE, TOKEN_VALUE);
		iRODSToJargon.put(COL_TOKEN_VALUE2, TOKEN_VALUE2);
		iRODSToJargon.put(COL_TOKEN_VALUE3, TOKEN_VALUE3);
		iRODSToJargon.put(COL_TOKEN_COMMENT, TOKEN_COMMENT);

		// R_COLL_USER_MAIN
		iRODSToJargon.put(COL_COLL_USER_NAME, DIRECTORY_USER_NAME);
		iRODSToJargon.put(COL_COLL_USER_ZONE, DIRECTORY_USER_ZONE);

		// R_DATA_USER_MAIN (r_user_main for Data information specifically)
		iRODSToJargon.put(COL_DATA_USER_NAME, FILE_PERMISSION_USER_NAME);
		iRODSToJargon.put(COL_DATA_USER_ZONE, FILE_PERMISSION_USER_ZONE);

		// register all MetaDataGroup subclasses
		// Data
		MetaDataGroup group = new MetaDataGroup(GROUP_DATA,
				"Core meta-information about datasets.");
		group.add(new MetaDataField(FILE_ID,
				"DataObject(file) Identifier (a number)", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(FILE_REPLICA_NUM,
				"DataObject Replica Number (0 to N)", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(FILE_VERSION, "DataObject Version",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_TYPE, "DataObject Type",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_RESOURCE_GROUP,
				"DataObject Resource Group Name", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RESOURCE_NAME, "DataObject Resource Name ",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(PATH_NAME,
				"DataObject Path (physical location)", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(FILE_OWNER_ZONE, "DataObject Owner Zone",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(
				FILE_REPLICA_STATUS,
				"DataObject Replica Status (?I believe this indicates if this replica is current)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_STATUS, "DataObject Data Status (?)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_CHECKSUM, "DataObject Checksum",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_EXPIRY,
				"DataObject Expiration time (currently unused)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_MAP_ID,
				"DataObject Map Identifier (?)", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_NAME, "filename",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_NAME, "directory path",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(SIZE, "size of this file",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(OWNER, "onwer of this file",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_COMMENTS, "comments about this file",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(CREATION_DATE,
				"number of seconds this file was created after the unix epoch",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(
				MODIFICATION_DATE,
				"number of seconds this file was modified after the unix epoch",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(
				FILE_ACCESS_TYPE,
				"Data Access Type: a number corresponding to access permissions",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_ACCESS_NAME,
				"Data Access Name: a name corresponding to access permissions",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_PERMISSION_USER_NAME,
				"Data Access User Name: access permissions by user name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_PERMISSION_USER_ZONE,
				"Data Access User Zone: access permissions by user zone",
				MetaDataField.STRING, protocol));
		add(group);

		// Collection
		group = new MetaDataGroup(GROUP_DIRECTORY,
				"Core meta-information about directories/collections.");
		group.add(new MetaDataField(DIRECTORY_ID,
				"Collection (directory) Identifier", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(DIRECTORY_NAME, "Collection Name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(PARENT_DIRECTORY_NAME,
				"Collection Parent Name (up one level)", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(DIRECTORY_OWNER, "Collection Owner Name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_OWNER_ZONE,
				"Collection Owner Zone", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_INHERITANCE,
				"Collection Inheritance (?)", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_COMMENTS, "Collection Comments",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_CREATE_DATE,
				"Time that this record was created (Unix Time)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_CREATE_TIMESTAMP,
				"Time that this record was created", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(DIRECTORY_MODIFY_DATE,
				"Time that this record was last modified (Unix Time)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(
				DIRECTORY_TYPE,
				"Type of directory on the iRODS server, e.g. a mount point or a tar file mount",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_ACCESS_TYPE,
				"Directory access constraint/permissions (integer).",
				MetaDataField.INT, protocol));
		group.add(new MetaDataField(DIRECTORY_ACCESS_CONSTRAINT,
				"Directory access constraint/permissions.",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_TOKEN_NAMESPACE,
				"Directory token namespace.", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_USER_NAME, "Directory User Name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_USER_ZONE, "Directory User Zone",
				MetaDataField.STRING, protocol));
		add(group);

		// Resource
		group = new MetaDataGroup(GROUP_RESOURCE,
				"Core information about iRODS resources.");
		group.add(new MetaDataField(RESOURCE_ID,
				"Resource Identifier (a number)", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(COLL_RESOURCE_NAME, "Resource Name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_ZONE, "Zone Name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_TYPE, "Resource Type",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_CLASS, "Resource Class",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_LOCATION,
				"Resource Location (DNS hostname)", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RESOURCE_VAULT_PATH,
				"Resource Vault Path (where physical files are stored)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_FREE_SPACE,
				"Resource Free Space ", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_FREE_SPACE_TIME,
				"Resource Free Space Time", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_INFO,
				"Resource Information (set by admin)", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RESOURCE_STATUS, "Resource Status",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_COMMENTS,
				"Resource Comment (set by admin)", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RESOURCE_CREATE_DATE,
				"Time that this record was created (Unix Time)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_MODIFY_DATE,
				"Time that this record was last modified (Unix Time)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_GROUP, "Resource Group Name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_GROUP_ID, "Resource Group ID",
				MetaDataField.STRING, protocol));
		add(group);

		// User
		group = new MetaDataGroup(GROUP_USER,
				"Core information about iRODS-registered users.");
		group.add(new MetaDataField(USER_ID, "User Identifier (a number)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_NAME, "User Name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_TYPE, "User Type",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_ZONE, "User Zone",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_DN,
				"User GSI Distinguished Name (irods2.2+)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(UserMetaData.USER_DN_2_1,
				"User GSI Distinguished Name (prior to irods2.2)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_INFO,
				"User Information (set by the admin, like comment)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_COMMENT,
				"User Comment (also set by the admin)", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(USER_CREATE_DATE,
				"Time that this record was created (Unix Time)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_MODIFY_DATE,
				"Time that this record was last modified (Unix Time)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_GROUP, "User Group Name",
				MetaDataField.STRING, protocol));
		add(group);

		// Zone
		group = new MetaDataGroup(GROUP_ZONE, "Core information about Zones.");
		group.add(new MetaDataField(ZONE_ID, "Zone Identifier (a number)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(ZONE_NAME, "Zone Name",
				MetaDataField.STRING, protocol));
		add(group);

		// User
		group = new MetaDataGroup(GROUP_DEFINABLE_METADATA,
				"Information about iRODS user definable AVU metadata");
		group.add(new MetaDataField(META_DATA_ATTR_NAME,
				"Metadata for a DataObject: Attribute Name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(META_DATA_ATTR_VALUE,
				"Metadata for a DataObject: Attribute Value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(META_DATA_ATTR_UNITS,
				"Metadata for a DataObject: Attribute Units",
				MetaDataField.STRING, protocol));

		group.add(new MetaDataField(META_COLL_ATTR_NAME,
				"Metadata for a Collection: Attribute Name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(META_COLL_ATTR_VALUE,
				"Metadata for a Collection: Attribute Value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(META_COLL_ATTR_UNITS,
				"Metadata for a Collection: Attribute Units",
				MetaDataField.STRING, protocol));

		add(group);

		// Rule
		group = new MetaDataGroup(GROUP_RULE,
				"MetaData Information about Rules.");
		group.add(new MetaDataField(RULE_ID, "Rule Execution Identifier",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RULE_NAME, "Rule Execution Name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RULE_REI_FILE_PATH,
				"Rule Execution REI (Rule Execution Information) File Path",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RULE_USER_NAME, "Rule Execution User Name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RULE_ADDRESS,
				"Rule Execution Address (hostname, if specified)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RULE_TIME,
				"Rule Execution Time (when to execute)", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RULE_FREQUENCY,
				"Rule Execution Frequency (when to repeat (seconds))",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RULE_PRIORITY, "Rule Execution Priority",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RULE_ESTIMATED_EXE_TIME,
				"Rule Execution Estimated Exectuion Time (if specified)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RULE_NOTIFICATION_ADDR,
				"Rule Execution Notification Address (if specified)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RULE_LAST_EXE_TIME,
				"Rule Execution Last Execution Time (if repeating)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RULE_STATUS, "Rule Execution Status",
				MetaDataField.STRING, protocol));
		add(group);

		// Token
		group = new MetaDataGroup(GROUP_TOKEN, "Core information about Tokens.");
		group.add(new MetaDataField(TOKEN_NAMESPACE, "Token Namespace",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TOKEN_ID, "Token Identifier",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TOKEN_NAME, "Token Name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TOKEN_VALUE,
				"Token Value (meaning depends on type of token)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TOKEN_VALUE2,
				"Token Value2 (meaning depends on type of token)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TOKEN_VALUE3,
				"Token Value3 (meaning depends on type of token)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TOKEN_COMMENT, "Token Comment",
				MetaDataField.STRING, protocol));
		add(group);

	}

	/**
	 * Default constructor called once from the initializer/static methods used
	 * to setup querying for iRODS.
	 */
	IRODSMetaDataSet(final IRODSProtocol protocol) {
		super();
		IRODSMetaDataSet.protocol = protocol;
	}

	/**
	 * Given the string <code>fieldName</code> return the appropriate
	 * MetaDataField for use in a metadata query.
	 */
	public static MetaDataField getField(final String fieldName) {
		if (log.isDebugEnabled()) {
			log.debug("getting irods metadata field for field name:"
					+ fieldName);
		}
		if (fieldName == null) {
			log.error("npe, metadata field name passed in is null");
			throw new NullPointerException("The fieldName cannot be null.");
		}

		MetaDataField field = (MetaDataField) metaDataFields.get(fieldName);

		if (field == null) {
			try {
				log.debug("field was null");
				field = (MetaDataField) metaDataFields.get(iRODSToJargon
						.get(Integer.decode(fieldName)));
			} catch (NumberFormatException e) {
				log.warn("number format exception for field:" + fieldName
						+ " logged and ignored");
			}
		}

		// if the field is still null, try to resolve it as extensible metadata
		if (field == null) {
			log.debug("field was still null, attempting to look up as extensbile metadata");

			String extensibleFieldName = extensibleMetaDataMapping
					.getColumnNameFromIndex(fieldName);
			if (extensibleFieldName != null) {
				if (log.isDebugEnabled()) {
					log.debug("resolved field as extensible field name:"
							+ extensibleFieldName);
				}
				field = new MetaDataField(extensibleFieldName,
						"extensible metadata", MetaDataField.STRING, protocol);
			}
		}

		return field;
	}

	/**
	 * Given a field name look up the field name in the extensible meta data
	 * that has been defined from soome source and stored in
	 * {@link org.irods.jargon.core.query.ExtensibleMetaDataMapping
	 * ExtensibleMetaDataMapping}.
	 * 
	 * @param fieldName
	 *            <code>String</code> that represents
	 * @return
	 */
	public static String getIDFromExtensibleMetaData(final String fieldName) {
		if (extensibleMetaDataMapping == null) {
			log.debug("no extensible meta data, return passed in field");
			return fieldName;
		} else {
			String temp = extensibleMetaDataMapping
					.getIndexFromColumnName(fieldName);
			log.debug("attempted extensible lookup and got:" + temp);
			return temp;
		}
	}

	/**
	 * Given a field name, as defined in IRODSMetaDataSet, give the numeric
	 * equivalent suitable for a gen query request to IRODS.
	 * 
	 * @param fieldName
	 *            <code>String</code> that represents
	 * @return
	 */
	public static String getID(final String fieldName) {
		log.debug("doing a getID for field:{}", fieldName);

		Object temp = jargonToIRODS.get(fieldName);

		if (temp == null) {
			log.debug(
					"not IRODSMetaDataSet field, returning passed-in field name instead:{}",
					fieldName);
			return fieldName;
		} else {
			String tempString = temp.toString();
			log.debug("successfully looked up, and returning:{}", tempString);
			return tempString;
		}
	}
}