/**
 * @file  icatGeneralMS.c
 *
 */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"

/** \mainpage iRODS Documentation

This documentation is generated from the iRODS code.

\section mainpage Main Project Page
 - http://www.irods.org

\section msibyalphabet Microservices by Alphabet
 - <a href="globals.html">Full Alphabetical List</a>

\section msicore Core Microservices

 \subsection msiruleengine Rule Engine Microservices
  - #msiAdmChangeCoreIRB    - Changes the core.irb file from the client
  - #msiAdmAppendToTopOfCoreIRB - Prepends another irb file to the core.irb file
  - #msiAdmAddAppRuleStruct - Adds application level IRB rules and DVM and FNM mappings to the rule engine
  - #msiAdmClearAppRuleStruct - Clears application level IRB rules and DVM and FNM mappings that were loaded into the rule engine
  - #msiAdmShowIRB      - Displays the currently loaded rules
  - #msiAdmShowDVM      - Displays the currently loaded variable name mappings
  - #msiAdmShowFNM      - Displays the currently loaded microservices and action (function) name mappings
  - #msiAdmReadRulesFromFileIntoStruct - Reads configuration file into a rule structure
  - #msiAdmInsertRulesFromStructIntoDB - Writes a rule structure into the current rule base
  - #msiGetRulesFromDBIntoStruct - Populates a rule structure with rules from the given base name
  - #msiAdmWriteRulesFromStructIntoFile - Writes to file the rules within a given rule structure
  - #msiAdmReadDVMapsFromFileIntoStruct - Reads dvm configuration file into a variable mapping structure
  - #msiAdmInsertDVMapsFromStructIntoDB - Writes a variable mapping  structure into the current dvm base
  - #msiGetDVMapsFromDBIntoStruct - Populates a dvm structure with variable mappings from the given base name
  - #msiAdmWriteDVMapsFromStructIntoFile - Writes to file the variable mappings within a given dvm structure

  - #msiAdmReadFNMapsFromFileIntoStruct - Reads fnm configuration file into a function mapping structure
  - #msiAdmInsertFNMapsFromStructIntoDB - Writes a function mapping  structure into the current fnm base
  - #msiGetFNMapsFromDBIntoStruct - Populates a fnm structure function mappings  from the given base name
  - #msiAdmWriteFNMapsFromStructIntoFile - Writes to file the function mappings within a given fnm structure
  
 \subsection msiworkflow Workflow Microservices
  - #nop, #null       - No action
  - #cut              - Not to retry any other applicable rules for this action
  - #succeed          - Succeed immediately
  - #fail             - Fail immediately - recovery and retries are possible
  - #msiGoodFailure   - Useful when you want to fail but no recovery initiated
  - #msiSleep         - Sleep
  - #whileExec        - While loop
  - #forExec          - For loop with initial, step and end condition
  - #forEachExec      - For loop iterating over a row of tables or a list
  - #breakExec        - Breaks out of whileExec, forExec and forEachExec loops
  - #writeString      - Writes a string to stdout buffer
  - #writeLine        - Writes a line (with end-of-line) to stdout buffer
  - #assign           - Assigns a value to a parameter
  - #ifExec           - If-then-else conditional branch
  - #delayExec        - Delays an execution of microservices or rules
  - #remoteExec       - Invokes remote execution of microservices or rules
  - #msiApplyAllRules - Applies all applicable rules when executing a given rule

 \subsection msihelper Helper Microservices
  Can be called by client through irule.
  - #msiGetStdoutInExecCmdOut - Gets stdout buffer from ExecCmdOut into buffer
  - #msiGetStderrInExecCmdOut - Gets stderr buffer from ExecCmdOut into buffer
  - #msiWriteRodsLog - Writes a message into the server rodsLog
  - #msiAddKeyValToMspStr - Adds a key and value to existing msKeyValStr
  - #msiSplitPath - Splits a pathname into parent and child values
  - #msiGetSessionVarValue - Gets the value of a session variable in the rei
  - #msiExit - Add a user message to the error stack

 \subsection msidatabase Database Object (DBO) and DB Resource (DBR) Microservices
  Can be called by client through irule.
  - #msiDboExec - Execute a database object on a DBR
  - #msiDbrCommit - Executes a commit on a DBR
  - #msiDbrRollback - Executes a rollback on a DBR

 \subsection msilowlevel Data Object Low-level Microservices
  Can be called by client through irule.
  - #msiDataObjCreate - Create a data object
  - #msiDataObjOpen - Open a data object
  - #msiDataObjClose - Close an opened data object
  - #msiDataObjLseek - Repositions the read/write offset of an open data object
  - #msiDataObjRead - Read an opened data object 
  - #msiDataObjWrite - Write to an opened data object

 \subsection msidataobject Data Object Microservices
  Can be called by client through irule.
  - #msiDataObjUnlink - Delete a data object
  - #msiDataObjRepl - Replicate a data object
  - #msiDataObjReplWithOptions - Replicate a data object, with options
  - #msiDataObjCopy - Copy a data object
  - #msiDataObjGet - Get a data object
  - #msiDataObjGetWithOptions - Get a data object, with options
  - #msiDataObjPut - Put a data object
  - #msiDataObjPutWithOptions - Put a data object, with options
  - #msiDataObjChksum - Checksum a data object
  - #msiDataObjChksumWithOptions - Checksum a data object, with options
  - #msiDataObjPhymv - Move a data object from one resource to another
  - #msiDataObjRename - Rename a data object 
  - #msiDataObjTrim - Trim the replicas of a data object
  - #msiPhyPathReg - Register a physical file into iRODS
  - #msiObjStat - Stat an iRODS object
  - #msiDataObjRsync - Syncs a data object from a source to a destination
  - #msiCollRsync - Recursively syncs a source collection to a target collection 
  - #msiGetObjType - Finds if a given value is a data, coll, resc, ...
  - #msiCheckPermission - Check if a data object permission is the same as the one given
  - #msiCheckOwner - Check if the user is the owner of the data object
  - #msiSetReplComment - Sets the data_comments attribute of a data object

 \subsection msicollection Collection Microservices
  - #msiCollCreate - Create a collection
  - #msiCollRepl - Replicate all files in a collection
  - #msiRmColl - Delete a collection
  - #msiAutoReplicateService - Checks and repairs up to n replicas
  - #msiDataObjAutoMove - Used to automatically move the newly created file into a destination collection
  - #msiTarFileExtract - Extracts a tar object file into a target collection
  - #msiTarFileCreate - Creates a tar object file from a target collection
  - #msiPhyBundleColl - Bundles a collection into a number of tar files, similar to the iphybun command

 \subsection msiproxy Proxy Command Microservices
  - #msiExecCmd - Remotely execute a command

 \subsection msiicat iCAT Microservices
 iCAT System Services
  - #msiVacuum - Postgres vacuum - done periodically
  - #msiQuota - Calculates storage usage and sets quota values
  - #msiCommit  - Commits the database transaction
  - #msiRollback - Rollback the current database transaction
  - #msiSetACL - Changes the ACL for a given pathname, either a collection or a data object
  - #msiCreateUser - Creates a new user
  - #msiDeleteUser - Deletes a user
  - #msiAddUserToGroup - Adds a user to a group
  - #msiCreateCollByAdmin  - Creates a collection by administrator
  - #msiDeleteCollByAdmin  - Deletes a collection by administrator
  - #msiRenameLocalZone  - Renames the local zone by updating various tables
  - #msiRenameCollection  - Renames a collection; used via a rule with the above #msiRenameLocalZone
  - #msiExecStrCondQuery - Creates an iCAT query, given a condition string, and executes it and returns the values
  - #msiExecStrCondQueryWithOptions - Like #msiExecStrCondQuery, with extra options
  - #msiExecGenQuery - Executes a given general query structure and returns results
  - #msiMakeQuery - Creates a psuedo-SQL query, given a select list and a condition list
  - #msiGetMoreRows - Continues an unfinished query and #msiExecStrCondQuery and returns results
  - #msiMakeGenQuery  - A combination of #msiMakeQuery and #msiExecGenQuery and returns the results of the execution
  - #msiGetContInxFromGenQueryOut - Gets continue index value generated by #msiExecGenQuery
  - #msiAddSelectFieldToGenQuery - Sets a select field in a genQueryInp_t
  - #msiPrintGenQueryInp - Prints the given GenQueryInp_MS_T to the given target buffer
  - #msiAddConditionToGenQuery - Adds a condition to a genQueryInp_t
  - #msiPrintGenQueryOutToBuffer - Writes the contents of a GenQueryOut_MS_T into a BUF_LEN_MS_T
  - #msiDeleteUnusedAVUs - Deletes unused AVUs from the iCAT

 \subsection msirda Rule-oriented Database Access Microservices
  - #msiRdaToStdout   - Calls new RDA functions to interface to an arbitrary database returning results in stdout
  - #msiRdaToDataObj  - As above but stores results in an iRODS DataObject
  - #msiRdaNoResults  - As above, performs a SQL operation but without resulting output
  - #msiRdaCommit     - Commit changes to the database
  - #msiRdaRollback   - Rollback (don't commit) changes to the database

 \subsection msixmsg XMessaging System Microservices
  - #msiXmsgServerConnect - Connects to the XMessage Server as designate by iRODS Environment file/variable
  - #msiXmsgCreateStream - Creates a new Message Stream
  - #msiCreateXmsgInp  - Creates an Xmsg packet, given all information values
  - #msiSendXmsg - Sends an Xmsg packet
  - #msiRcvXmsg -  Receives an Xmsg packet
  - #msiXmsgServerDisConnect  - Disconnects from the XMessage Server
  - #readXMsg - Reads a message packet from an XMsgStream
  - #writeXMsg - Writes a given string into an XMsgStream

 \subsection msistring String Manipulation Microservices
  - #msiStrlen   - Returns the length of a given string
  - #msiStrchop  - Removes the last character of a given string
  - #msiSubstr   - Returns a substring of the given string
 
 \subsection msiemail Email Microservices
  - #msiSendMail   - Sends email
  - #msiSendStdoutAsEmail - Sends rei's stdout as email

 \subsection msikv Key-Value (Attr-Value) Microservices
  - #writeKeyValPairs - Writes key-value pairs to stdout or stderr and with given separator
  - #msiPrintKeyValPair - Prints key-value pairs to rei's stdout separated by =
  - #msiGetValByKey  - Extracts the corresponding value, given a key and a keyValPair struct
  - #msiString2KeyValPair - Converts a \%-separated key=value pair strings into keyValPair structure
  - #msiStrArray2String - Converts an array of strings to a string separated by \%-signs
  - #msiAssociateKeyValuePairsToObj  - Ingests object metadata into iCAT from a AVU structure
  - #msiRemoveKeyValuePairsFromObj  - Removes object metadata from iCAT using a AVU structure
  - #msiAddKeyVal - Adds a new key and value to a keyValPair_t 

 \subsection msiotheruser Other User Microservices
  - #msiExtractNaraMetadata - Extracts NARA style metadata from AVU triplets
  - #msiApplyDCMetadataTemplate - Adds Dublin Core Metadata fields to an object or collection
  - #msiRegisterData - Registers a new data object
  - #writeBytesBuf - Writes the buffer in an inOutStruct to stdout or stderr
  - #msiStrToBytesBuf - Converts a string to a bytesBuf_t
  - #msiFreeBuffer - Frees a buffer in an inOutStruct, or stdout or stderr
  - #writePosInt  - Writes an integer to stdout or stderr
  - #msiGetDiffTime - Returns the difference between two system timestamps given in unix format (stored in string)
  - #msiGetSystemTime - Returns the local system time of server
  - #msiHumanToSystemTime - Converts a human readable date to a system timestamp
  - #msiGetIcatTime - Returns the system time for the iCAT server
  - #msiGetTaggedValueFromString  - Gets the value from a file in tagged-format (psuedo-XML), given a tag-name
  - #msiExtractTemplateMDFromBuf     - Extracts AVU info using template
  - #msiReadMDTemplateIntoTagStruct  - Loads template file contents into tag structure

 \subsection msisystem System Microservices
  Can only be called by the server process
  - #msiSetDefaultResc - Sets the default resource
  - #msiSetNoDirectRescInp - Sets a list of resources that cannot be used by a normal user directly
  - #msiSetRescSortScheme - Sets the scheme for selecting the best resource to use
  - #msiSetMultiReplPerResc - Sets the number of copies per resource to unlimited
  - #msiSetDataObjPreferredResc - Specifies the preferred copy to use, if the data has multiple copies
  - #msiSetDataObjAvoidResc - Specifies the copy to avoid 
  - #msiSetGraftPathScheme - Sets the scheme for composing the physical path in the vault to GRAFT_PATH
  - #msiSetRandomScheme - Sets the the scheme for composing the physical path in the vault to RANDOM
  - #msiSetResource  - sets the resource from default
  - #msiSortDataObj - Sort the replica randomly when choosing which copy to use
  - #msiSetNumThreads - specify the parameters for determining the number of threads to use for data transfer
  - #msiSysChksumDataObj - Checksums a data object
  - #msiSysReplDataObj - Replicates a data object
  - #msiSysMetaModify - Modifies system metadata
  - #msiStageDataObj - Stages the data object to the specified resource before operation
  - #msiNoChkFilePathPerm - Does not check file path permission when registering
  - #msiNoTrashCan - Sets the policy to no trash can
  - #msiSetPublicUserOpr - Sets a list of operations that can be performed by the user "public"
  - #msiCheckHostAccessControl - Sets the access control policy
  - #msiServerMonPerf - Monitors the servers' activity and performance
  - #msiFlushMonStat - Flushes the servers' monitoring statistics
  - #msiDigestMonStat - Calculates and stores a digest performance value for each connected resource
  - #msiDeleteDisallowed - Sets the policy for determining certain data cannot be deleted
  - #msiSetDataTypeFromExt - Gets the data type based on file name extension
  - #msiSetReServerNumProc - Sets the number of processes for the rule engine server
  - #msiSetRescQuotaPolicy - Sets the resource quota to on or off
  - #msiListEnabledMS - Returns the list of compiled microservices on the local iRODS server
  - #msiSetBulkPutPostProcPolicy - Sets whether acPostProcForPut should be run after a bulk put

 \section msimodules Module Microservices

 \subsection msiera ERA - Electronic Records Archives Program
  - #msiRecursiveCollCopy  - Recursively copies a collection and its contents including metadata
  - #msiGetDataObjACL - Gets ACL (Access Control List) for a data object in | separated format
  - #msiGetCollectionACL- Gets ACL (Access Control List) for a collection in | separated format
  - #msiGetDataObjAVUs - Retrieves metadata AVU triplets for a data object and returns them as an XML file
  - #msiGetDataObjPSmeta - Retrieves metadata AVU triplets for a data object in | separated format
  - #msiGetCollectionPSmeta- Retrieves metadata AVU triplets for a collection in | separated format
  - #msiGetDataObjAIP - Gets the Archival Information Package of a data object in XML format
  - #msiLoadMetadataFromDataObj - Parses an iRODS object for new metadata AVUs from | separated format
  - #msiExportRecursiveCollMeta - Exports metadata AVU triplets for a collection and its contents in | separated format
  - #msiCopyAVUMetadata - Copies metadata triplets from an iRODS object to another
  - #msiStripAVUs - Strips a data object of its metadata AVU triplets
  - #msiGetUserInfo - Gets Information about user
  - #msiGetUserACL  - Gets User ACL for all objects and collections
  - #msiCreateUserAccountsFromDataObj - Creates new user from information in an iRODS data object
  - #msiLoadUserModsFromDataObj - Modifies user information  from information in an iRODS data object
  - #msiDeleteUsersFromDataObj - Deletes user from information in an iRODS data object
  - #msiLoadACLFromDataObj - Loads ACL from information in an iRODS data object
  - #msiGetAuditTrailInfoByUserID - Retrieves Audit Trail information for a user ID
  - #msiGetAuditTrailInfoByObjectID - Retrieves Audit Trail information for an object ID
  - #msiGetAuditTrailInfoByActionID - Retrieves Audit Trail information for a given action ID
  - #msiGetAuditTrailInfoByKeywords - Retrieves Audit Trail information by keywords in the comment field
  - #msiGetAuditTrailInfoByTimeStamp - Retrieves Audit Trail information by timestamp period
  - #msiSetDataType  - Sets data type for an object
  - #msiGuessDataType - Guesses the data type of an object based on its file extension
  - #msiMergeDataCopies - Custom microservice for NARA consolidation rule
  - #msiFlagDataObjwithAVU - Flags a data object with an AVU
  - #msiGetCollectionContentsReport - Returns the number of objects in a collection by data type
  - #msiGetCollectionSize - Returns the object count and total disk usage of a collection
  - #msiCollectionSpider - Applies a microservice sequence to all data objects in a collection, recursively
  - #msiIsColl - Checks if an iRODS path is a collection. For use in workflows
  - #msiIsData - Checks if an iRODS path is a data object (an iRODS file). For use in workflows
  - #msiStructFileBundle - Bundles a collection for export
  - #msiFlagInfectedObjs - Parses output from clamscan and flags infected objects

 \subsection msiurl URL
  - #msiFtpGet - Gets a remote file using FTP and writes it to an iRODS object
  - #msiTwitterPost - Posts a message to twitter.com

 \subsection msixml XML
  - #msiLoadMetadataFromXml - Loads AVU metadata from an XML file of AVU triplets
  - #msiXmlDocSchemaValidate - Validates an XML file against an XSD schema, both iRODS objects
  - #msiXsltApply - Returns the xml object after applying the xslt transformation, given an xml object and an xslt object
 
 \subsection msiimage Image
  - #msiImageConvert - Reads a source image file and write it out as a new image file in a chosen format
  - #msiImageGetProperties - Gets the properties of an image file
  - #msiImageScale - Reads a source image file, scale it up or down in size, and write it out as a new image file in a chosen format

 \subsection msiintegritychecks Integrity Checks
  - #msiVerifyOwner - Checks if files in a given collection have a consistent owner
  - #msiVerifyACL - Checks the ACL on a collection
  - #msiVerifyExpiry - Checks whether files in a collection have expired or not expired
  - #msiVerifyAVU - Performs operations on the AVU metadata on files in a given collection
  - #msiVerifyDataType - Checks if files in a given collection are of a given data type(s)
  - #msiVerifyFileSizeRange - Checks to see if file sizes are NOT within a certain range

 \subsection msihdf HDF
  - #msiH5File_open - Opens an HDF file
  - #msiH5File_close - Closes an HDF file
  - #msiH5Dataset_read - Reads data from an HDF file
  - #msiH5Dataset_read_attribute - Reads data attribute from an HDF file
  - #msiH5Group_read_attribute - Reads attributes of a group in an HDF file

 \subsection msiproperties Properties
  - #msiPropertiesNew - Creates a new empty property list
  - #msiPropertiesClear - Clears a property list
  - #msiPropertiesClone - Clones a property list, returning a new property list
  - #msiPropertiesAdd - Adds a property and value to a property list.  If the property is already in the list, its value is changed.  Otherwise the property is added.
  - #msiPropertiesRemove - Removes a property from the list
  - #msiPropertiesGet - Gets the value of a property in a property list.  The property list is left unmodified.
  - #msiPropertiesSet - Sets the value of a property in a property list.  If the property is already in the list, its value is changed.  Otherwise the property is added.
  - #msiPropertiesExists - Returns true (integer 1) if the keyword has a property value in the property list, and false (integer 0) otherwise.  The property list is unmodified.
  - #msiPropertiesToString - Converts a property list into a string buffer.  The property list is left unmodified.
  - #msiPropertiesFromString - Parses a string into a new property list.  The existing property list, if any, is deleted.

 \subsection msiwebservices Web Services
  - #msiGetQuote - Returns stock quotation (delayed by web service) using web service provided by http://www.webserviceX.NET
  - #msiIp2location - Returns host name and other details given an ipaddress using web service provided by http://ws.fraudlabs.com/
  - #msiConvertCurrency - Returns conversion rate for currencies from one country to another using web service provided by http://www.webserviceX.NET/
  - #msiObjByName - Returns position and type of an astronomical object given a name from the NASA/IPAC Extragalactic Database (NED) using web service at http://voservices.net/NED/ws_v2_0/NED.asmx
  - #msiSdssImgCutout_GetJpeg - Returns an image buffer given a position and cutout from the SDSS Image Cut Out service using web service provided by http://skyserver.sdss.org

 \subsection msiguinot Guinot
  - #msiGetFormattedSystemTime - Returns the local system time

 \subsection msiz3950 Z3950
  - #msiz3950Submit - Retrieves a record from a Z39.50 server

**/





/**
 * \fn msiGetIcatTime (msParam_t *timeOutParam, msParam_t *typeInParam, ruleExecInfo_t *rei)
 *
 * \brief   This function returns the system time for the iCAT server
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  DICE
 * \date  	2008
 *
 * \remark Jewel Ward - msi documentation, 2009-06-10
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-25
 *
 * \note	This function returns the system time for the iCAT server
 *
 * \usage
 * 
 *  As seen in server/config/reConfigs/core.irb
 * 
 * acPurgeFiles(*Condition)|(*Condition == null) %% (*Condition == '')|msiGetIcatTime(*Time,unix)##acGetIcatResults(remove,DATA_EXPIRY < '*Time',*List)##forEachExec(*List,msiDataObjUnlink(*List,*Status)##msiGetValByKey(*List,DATA_NAME,*D)##msiGetValByKey(*List,COLL_NAME,*E)##writeLine(stdout,Purged File *E\*D at *Time ),nop)|nop##nop
 * (note that the \* should be a forward slash * but to avoid a compiler
 *  warning about a / * (no blank) within a comment this was changed.)
 *
 * \param[out] timeOutParam - a msParam of type STR_MS_T
 * \param[in] typeInParam - a msParam of type STR_MS_T
 *    \li "icat" or "unix" will return seconds since epoch
 *    \li otherwise, human friendly
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect none
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int
msiGetIcatTime(msParam_t* timeOutParam,  msParam_t* typeInParam, ruleExecInfo_t *rei)
{
  char *type;
  char tStr0[TIME_LEN],tStr[TIME_LEN];
  int i;

  type = typeInParam->inOutStruct;

  if (!strcmp(type,"icat") || !strcmp(type,"unix")) {
    getNowStr(tStr);
  }
  else { /* !strcmp(type,"human") */
    getNowStr(tStr0);
    getLocalTimeFromRodsTime(tStr0,tStr);
  }
  i = fillStrInMsParam (timeOutParam,tStr);
  return(i);
}

/**
 * \fn msiVacuum (ruleExecInfo_t *rei)
 *
 * \brief   Postgres vacuum, done periodically to optimize indices and performance
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  Wayne Schroeder
 * \date    December 2006
 *
 * \remark Jewel Ward - msi documentation, 2009-06-19
 * \remark Terrell Russell - reviewed documentation, 2009-06-23
 *
 * \note The effect of this is that iCAT database gets vacuumed.
 *       This micro-service works with PostgreSQL only.
 *
 * \usage This is run via an 'iadmin' command (iadmin pv) via a rule
 *    in the core.irb. It is not designed for general use in other
 *    situations (i.e. don't call this from other rules).
 *    The core.irb rule is:
 *    acVacuum(*arg1)||delayExec(*arg1,msiVacuum,nop)|nop
 *
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect none
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int
msiVacuum(ruleExecInfo_t *rei)
{
   int i;
   rodsLog(LOG_NOTICE, "msiVacuum called\n");

   i = doForkExec("/usr/bin/perl", "./vacuumdb.pl");

   if (i) {
      rodsLog(LOG_ERROR, "msiVacuum doForkExec failure\n");
   }

   rodsLog(LOG_NOTICE, "msiVacuum done\n");

   return(0);
}


/**
 * \fn msiQuota (ruleExecInfo_t *rei)
 *
 * \brief  Calculates storage usage and sets quota values (over/under/how-much).
 *
 * \module core
 *
 * \since pre-2.3
 *
 * \author  Wayne Schroeder
 * \date    January 2010
 *
 * \note Causes the ICAT quota tables to be updated.
 *
 * \usage This is run via an admin rule (see the Quotas page on the
 * irods web site).
 *
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence rei->rsComm->clientUser.authFlag (must be admin)
 * \DolVarModified none
 * \iCatAttrDependence Utilizes ICAT data-object information
 * \iCatAttrModified Updates the quota tables
 * \sideeffect none
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int
msiQuota(ruleExecInfo_t *rei)
{
   int status;

#ifdef RODS_CAT
   rodsLog(LOG_NOTICE, "msiQuota/chlCalcUsageAndQuota called\n");
   status = chlCalcUsageAndQuota(rei->rsComm);
#else
   status =  SYS_NO_RCAT_SERVER_ERR;
#endif
   return(status);
}

/**
 *\fn msiSetResource (msParam_t *xrescName, ruleExecInfo_t *rei)
 *
 * \brief   This microservice sets the resource as part of a workflow execution.
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  DICE
 * \date	2008
 *
 * \remark Jewel Ward - msi documentation, 2009-06-19
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-23
 *
 * \note	This microservice sets the resource as part of a workflow execution.
 *
 * \usage
 * 
 *  As seen in server/config/reConfigs/nvo.irb
 * 
 * acRegisterData|$objPath like /home/collections.nvo/2mass/fits-images\*|cut##acCheckDataType(fits image)##msiSetResource(nvo-image-resource)##msiRegisterData|nop
 * (note that the \* should be a forward slash * but to avoid a compiler
 * warning about a / * (no blank) within a comment this was changed.)

 *
 * \param[in] xrescName - is a msParam of type STR_MS_T
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect none
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int  msiSetResource(msParam_t* xrescName, ruleExecInfo_t *rei)
{
  char *rescName;

  rescName = (char *) xrescName->inOutStruct;
  if (reTestFlag > 0 ) {
    if (reTestFlag == LOG_TEST_1) 
      rodsLog (LOG_NOTICE,"   Calling msiSetResource\n");
  }

  strcpy(rei->doi->rescName,rescName);
  return(0);
}


/**
 * \fn msiCheckOwner (ruleExecInfo_t *rei)
 *
 * \brief   This microservice checks whether the user is the owner
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  DICE
 * \date	2008
 *
 * \remark Jewel Ward - msi documentation, 2009-06-19
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-25
 *
 * \note	This microservice checks whether the user is the owner
 *
 * \usage
 * 
 *  As seen in server/config/reConfigs/core3.irb
 * 
 *  acDeleteData|msiCheckOwner|msiDeleteData|recover_msiDeleteData
 *
 *  
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect none
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int msiCheckOwner(ruleExecInfo_t *rei)
{
    if (reTestFlag > 0 ) {
    if (reTestFlag == LOG_TEST_1) 
      rodsLog (LOG_NOTICE,"   Calling msiCheckOwner\n");
  }

  if (!strcmp(rei->doi->dataOwnerName,rei->uoic->userName) &&
      !strcmp(rei->doi->dataOwnerZone,rei->uoic->rodsZone))
    return(0);
  else
    return(ACTION_FAILED_ERR);

}

/**
 * \fn msiCheckPermission (msParam_t *xperm, ruleExecInfo_t *rei)
 *
 * \brief   This microservice checks the authorization permission (whether or not permission is granted)
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  DICE
 * \date  	2008
 *
 * \remark Jewel Ward - msi documentation, 2009-06-19
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-25
 *
 * \note	This microservice checks the authorization permission (whether or not permission is granted).
 *
 * \usage
 * 
 * As seen in server/config/reConfigs/core3.irb
 * 
 *  acDeleteData|msiCheckPermission(delete)|msiDeleteData|recover_msiDeleteData
 *
 *  
 * \param[in] xperm - a msParam of type STR_MS_T
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect none
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int msiCheckPermission(msParam_t* xperm, ruleExecInfo_t *rei)
{
  char *perm;

  perm = (char *) xperm->inOutStruct;
    if (reTestFlag > 0 ) {
    if (reTestFlag == LOG_TEST_1) 
      rodsLog (LOG_NOTICE,"   Calling msiCheckPermission\n");
  }
  if (strstr(rei->doi->dataAccess,perm) != NULL)
    return(0);
  else
    return(ACTION_FAILED_ERR);

}


/**
 * \fn msiCommit (ruleExecInfo_t *rei)
 *
 * \brief This microservice commits pending database transactions,
 * registering the new state information into the iCAT.
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author Wayne Schroeder
 * \date June 2009 or so
 *
 * \remark Jewel Ward - msi documentation, 2009-06-19
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-25
 *
 * \note This is used to commit changes (in any) into the ICAT
 * database as part of a rule and microservice chain.  See core.irb
 * for examples.  In other cases, ICAT updates and inserts are
 * automatically committed into the ICAT Database as part of the
 * normal operations (in the 'C' code).
 *
 * \usage 
 *
 * As seen in server/config/reConfigs/core.irb
 * 
 * acCreateUserF1|$otherUserName == anonymous|msiCreateUser##msiCommit|msiRollback##nop
 * 
 *  
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified  none
 * \iCatAttrDependence commits pending updates (if any)
 * \iCatAttrModified pending updates (if any) are committed into the ICAT db
 * \sideeffect none
 *
 * \return integer
 * \retval (status)
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int
msiCommit(ruleExecInfo_t *rei) {
   int status;

  /**** This is Just a Test Stub  ****/
  if (reTestFlag > 0 ) {
    if (reTestFlag == LOG_TEST_1) {
      rodsLog (LOG_NOTICE,"   Calling msiCommit\n");
    }
    if (reLoopBackFlag > 0)
      return(0);
  }
  /**** This is Just a Test Stub  ****/
#ifdef RODS_CAT
   status = chlCommit(rei->rsComm);
#else
   status =  SYS_NO_RCAT_SERVER_ERR;
#endif
   return(status);
}

/**
 * \fn msiRollback (ruleExecInfo_t *rei)
 *
 * \brief   This function deletes user and collection information from the iCAT by rolling back the database transaction
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author Wayne Schroeder
 * \date June 2009 or so
 *
 * \remark Jewel Ward - msi documentation, 2009-06-19
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-23
 *
 * \note This is used to not-commit changes into the ICAT database as
 * part of a rule and microservice chain.  See core.irb for examples.
 * In other cases, ICAT updates and inserts are automatically
 * rolled-back as part of the normal operations (in the 'C' code).
 *
 * \usage
 * 
 *  As seen in server/config/reConfigs/core.irb
 * 
 *  acCreateUserF1|$otherUserName == anonymous|msiCreateUser##msiCommit|msiRollback##nop
 *  
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence pending updates (if any) are canceled
 * \iCatAttrModified  pending updates (if any) are canceled
 * \sideeffect none
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int
msiRollback(ruleExecInfo_t *rei)
{
   int status;
  /**** This is Just a Test Stub  ****/
  if (reTestFlag > 0 ) {
    if (reTestFlag == LOG_TEST_1) {
      rodsLog (LOG_NOTICE,"   Calling msiRollback\n");
    }
    if (reLoopBackFlag > 0)
      return(0);
  }
  /**** This is Just a Test Stub  ****/


#ifdef RODS_CAT
   status = chlRollback(rei->rsComm);
#else
   status =  SYS_NO_RCAT_SERVER_ERR;
#endif
   return(status);
}

/**
 * \fn msiSetACL (msParam_t *recursiveFlag, msParam_t *accessLevel, msParam_t *userName, msParam_t *pathName, ruleExecInfo_t *rei)
 *
 * \brief   This micro-service changes the ACL for a given pathname,
 *            either a collection or a data object.
 *
 * \module core
 *
 * \since 2.3
 *
 * \author  Jean-Yves Nief
 * \date    2010-02-11
 *
 * \remark Terrell Russell - msi documentation, 2010-04-05
 *
 * \note This microservice modifies the access rights on a given iRODS object or
	 collection. For the collections, the modification can be recursive and the
	 inheritence bit can be changed as well.
 *
 * \usage msiSetACL(recursiveFlag, accessLevel, userName, pathName)
 * 
 * \param[in] recursiveFlag - a STR_MS_T, either "default" or "recursive".  "recursive"
 *    is only relevant if set with accessLevel set to "inherit".
 * \param[in] accessLevel - a STR_MS_T containing one of the following:
 *    \li read
 *    \li write
 *    \li own
 *    \li inherit
 *    \li null
 * \param[in] userName - a STR_MS_T, the user name or group name who will have ACL changed.
 * \param[in] pathName - a STR_MS_T, the collection or data object that will have its ACL changed.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence $userName and/or $objPath and/or $collName
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect none
 *
 * \return integer
 * \retval 0 on success
 * \pre N/A
 * \post N/A
 * \sa N/A
 * \bug  no known bugs
**/
int msiSetACL (msParam_t *recursiveFlag, msParam_t *accessLevel, msParam_t *userName, 
      msParam_t *pathName, ruleExecInfo_t *rei) {
	char *acl, *path, *recursiveFlg, *user;
	int recFlg, rc;
	modAccessControlInp_t modAccessControlInp;
	rsComm_t *rsComm;
  
	RE_TEST_MACRO ("    Calling msiSetACL")
    /* the above line is needed for loop back testing using irule -i option */
   
	if ( recursiveFlag == NULL || accessLevel == NULL || userName == NULL ||
		 pathName == NULL ) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
							"msiSetACL: one of the input parameter is NULL");
		return (rei->status);
	}
  
	recFlg = 0; /* non recursive mode */
	if ( strcmp (recursiveFlag->type, STR_MS_T) == 0 ) {
		recursiveFlg = (char *) recursiveFlag->inOutStruct;
		if ( strcmp(recursiveFlg,"recursive") == 0 ) {
			/* recursive mode */
			recFlg = 1;
		}
	} 
	else {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
							"msiSetACL: Unsupported input recursiveFlag type %i",
							recursiveFlag->type);
		return (rei->status);
	}
	
	if ( strcmp (accessLevel->type, STR_MS_T) == 0 ) {
		acl = (char *) accessLevel->inOutStruct;
	} 
	else {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
							"msiSetACL: Unsupported input accessLevel type %s",
							accessLevel->type);
		return (rei->status);
	}
  
	if ( strcmp (userName->type, STR_MS_T) == 0 ) {
		user = (char *) userName->inOutStruct;
	} 
	else {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
							"msiSetACL: Unsupported input userName type %s",
							userName->type);
		return (rei->status);
	}
	
	if ( strcmp (pathName->type, STR_MS_T) == 0 ) {
		path = (char *) pathName->inOutStruct;
	} 
	else {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
							"msiSetACL: Unsupported input pathName type %s",
							pathName->type);
		return (rei->status);
	}
	
	rsComm = rei->rsComm;
	modAccessControlInp.recursiveFlag = recFlg;
	modAccessControlInp.accessLevel = acl;
	modAccessControlInp.userName = user;
	modAccessControlInp.zone = rei->uoic->rodsZone;
	modAccessControlInp.path = path;
	rc = rsModAccessControl(rsComm, &modAccessControlInp);
	if ( rc < 0 ) {
		rodsLog (LOG_NOTICE, "msiSetACL: ACL modifications has failed for user %s on object %s, error = %i\n", user, path, rc);
	}
  
	return (rei->status);
}

/**
 * \fn msiDeleteUnusedAVUs (ruleExecInfo_t *rei)
 *
 * \brief   This function deletes unused AVUs from the iCAT.  See 'iadmin rum'.
 *
 * \module  core
 *
 * \since   post-2.3
 *
 * \author  Wayne Schroeder
 * \date    April 13, 2010
 *
 * \remark  Terrell Russell - reviewed msi documentation, 2010-04-20
 *
 * \note  This causes the unused AVUs to be removed from the ICAT.  
 *
 * \usage See 'iadmin help rum'.  Do not call this directly.
 * 
 *  As seen in server/config/reConfigs/core.irb
 *  
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence rei->rsComm->clientUser.authFlag (must be admin)
 * \DolVarModified none
 * \iCatAttrDependence check AVU table
 * \iCatAttrModified update AVU table
 * \sideeffect none
 *
 * \return integer
 * \retval (status)
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int
msiDeleteUnusedAVUs(ruleExecInfo_t *rei)
{
   int status;

  /**** This is Just a Test Stub  ****/
  if (reTestFlag > 0 ) {
    if (reTestFlag == LOG_TEST_1) {
      rodsLog (LOG_NOTICE,"   Calling msiDeleteUnusedAVUs\n");
    }
    if (reLoopBackFlag > 0)
      return(0);
  }
  /**** This is Just a Test Stub  ****/

#ifdef RODS_CAT
   rodsLog(LOG_NOTICE, "msiDeleteUnusedAVUs/chlDelUnusedAVUs called\n");
   status = chlDelUnusedAVUs(rei->rsComm);
#else
   status =  SYS_NO_RCAT_SERVER_ERR;
#endif
   return(status);
}
