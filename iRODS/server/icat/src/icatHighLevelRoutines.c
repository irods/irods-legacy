/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/**************************************************************************

  This file contains most of the ICAT (iRODS Catalog) high Level
  functions.  These, along with chlGeneralQuery, constitute the API
  between and Server (and microservices) and the ICAT code.  Each of
  the API routine names start with 'chl' for Catalog High Level.  
  Others are internal.

  Also see icatGeneralQuery.c for chlGeneralQuery, the other ICAT high
  level API call.

**************************************************************************/

#include "rods.h"
#include "rcMisc.h"

#include "icatMidLevelRoutines.h"
#include "icatMidLevelHelpers.h"
#include "icatHighLevelRoutines.h"
#include "icatLowLevel.h"

extern int get64RandomBytes(char *buf);

/* 
   Legal values for accessLevel in  chlModAccessControl (Access Parameter).
   Defined here since other code does not need them (except for help messages)
*/
#define AP_READ "read"
#define AP_WRITE "write"
#define AP_OWN "own"
#define AP_NULL "null"

#define MAX_PASSWORDS 40
/* TEMP_PASSWORD_TIME is the number of seconds the temp, one-time
   password can be used.  chlCheckAuth also checks for this column
   to be < 1000 to differentiate the row from regular passwords.
*/
#define TEMP_PASSWORD_TIME 10

#define PASSWORD_SCRAMBLE_PREFIX ".E_"
#define PASSWORD_KEY_ENV_VAR "irodsPKey"
#define PASSWORD_DEFAULT_KEY "a9_3fker"

int logSQL=0;

int _delColl(rsComm_t *rsComm, collInfo_t *collInfo);

icatSessionStruct icss={0};
char localZone[MAX_NAME_LEN]="";

/*
 Enable or disable some debug logging.
 By default this is off.
 */
int chlDebug(char *debugMode) {
   if (strstr(debugMode, "SQL")) {
      logSQL=1;
      chlDebugGenQuery(1);
      chlDebugGenUpdate(1);
      cmlDebug(1);
   }
   else {
      logSQL=0;
      chlDebugGenQuery(0);
      chlDebugGenUpdate(0);
      cmlDebug(0);
   }
   return(0);
}

/* 
 Possibly descramble a password (for user passwords stored in the ICAT).
 Called internally, from various chl functions.
 */
int
icatDescramble(char *pw) {
   char *cp1, *cp2, *cp3;
   int i,len;
   char pw2[MAX_PASSWORD_LEN+10];
   char unscrambled[MAX_PASSWORD_LEN+10];

   len = strlen(PASSWORD_SCRAMBLE_PREFIX);
   cp1=pw;
   cp2=PASSWORD_SCRAMBLE_PREFIX;  /* if starts with this, it is scrambled */
   for (i=0;i<len;i++) {
      if (*cp1++ != *cp2++) {
	 return 0;                /* not scrambled, leave as is */
      }
   }
   strncpy(pw2, cp1, MAX_PASSWORD_LEN);
   cp3 = getenv(PASSWORD_KEY_ENV_VAR);
   if (cp3==NULL) {
      cp3 = PASSWORD_DEFAULT_KEY;
   }
   obfDecodeByKey(pw2, cp3, unscrambled);
   strncpy(pw, unscrambled, MAX_PASSWORD_LEN);

   return 0;
}

/* 
 Scramble a password (for user passwords stored in the ICAT).
 Called internally.
 */
int
icatScramble(char *pw) {
   char *cp1;
   char newPw[MAX_PASSWORD_LEN+10];
   char scrambled[MAX_PASSWORD_LEN+10];

   cp1 = getenv(PASSWORD_KEY_ENV_VAR);
   if (cp1==NULL) {
      cp1 = PASSWORD_DEFAULT_KEY;
   }
   obfEncodeByKey(pw, cp1, scrambled);
   strncpy(newPw, PASSWORD_SCRAMBLE_PREFIX, MAX_PASSWORD_LEN);
   strncat(newPw, scrambled, MAX_PASSWORD_LEN);
   strncpy(pw, newPw, MAX_PASSWORD_LEN);
   return 0;
}

/*
 Open a connection to the database.  This has to be called first.  The
 server/agent and Rule-Engine Server call this when initializing.
 */
int chlOpen(char *DBUser, char *DBpasswd) {
   int i;
   if (logSQL) rodsLog(LOG_SQL, "chlOpen");
   icss.databaseUsername = DBUser;
   icss.databasePassword = DBpasswd;
   i = cmlOpen(&icss);
   if (i != 0) {
      rodsLog(LOG_NOTICE, "chlOpen cmlOpen failure %d",i);
   }
   else {
      icss.status=1;
   }
   return(i);
}

/*
 Close an open connection to the database.  
 Clean up and shutdown the connection.
 */
int chlClose() {
   int i;

   i = cmlClose(&icss);
   if (i == 0) icss.status=0;
   return(i);
}

int chlIsConnected() {
   if (logSQL) rodsLog(LOG_SQL, "chlIsConnected");
   return(icss.status);
}

/*
 This is used by the icatGeneralUpdate.c functions to get the icss
 structure.  icatGeneralUpdate.c and this (icatHighLevelRoutine.c)
 are actually one module but in two separate source files (as they
 got larger) so this is a 'glue' that binds them together.  So this
 is mostly an 'internal' function too.
 */
icatSessionStruct *
chlGetRcs()
{
   if (logSQL) rodsLog(LOG_SQL, "chlGetRcs");
   if (icss.status != 1) {
      return(NULL);
   }
   return(&icss);
}

/*
 Called internally to rollback current transaction after an error.
 */
int
_rollback(char *functionName) {
   int status;
#if ORA_ICAT
   status = 0; 
#else
   /* This type of rollback is needed for Postgres since the low-level
      now does an automatic 'begin' to create a sql block */
   status =  cmlExecuteNoAnswerSql("rollback", &icss);
   if (status == 0) {
      rodsLog(LOG_NOTICE,
	      "%s cmlExecuteNoAnswerSql(rollback) succeeded", functionName);
   }
   else {
      rodsLog(LOG_NOTICE,
	      "%s cmlExecuteNoAnswerSql(rollback) failure %d", 
	      functionName, status);
   }
#endif

   return(status);
}


/*
 Internal function to return the local zone (which is the default
 zone).  The first time it's called, it gets the zone from the DB and
 subsequent calls just return that value.
 */
int
getLocalZone()
{
   int status;
   if (localZone[0]=='\0') {
      if (logSQL) rodsLog(LOG_SQL, "getLocalZone SQL 1 ");
      status = cmlGetStringValueFromSql(
	   "select zone_name from R_ZONE_MAIN where zone_type_name=?",
	   localZone, MAX_NAME_LEN, "local", 0, &icss);
      if (status != 0) {
	 _rollback("getLocalZone");
	 rodsLog(LOG_NOTICE, "getLocalZone failure %d", status);
      }
      return(status);
   }
   return(0);
}

/* 
  External function to return the local zone name.
  Used by icatGeneralQuery.c
 */
char *
chlGetLocalZone() {
   getLocalZone();
   return(localZone);
}

/* 
 * chlModDataObjMeta - Modify the metadata of an existing data object. 
 * Input - rsComm_t *rsComm  - the server handle
 *         dataObjInfo_t *dataObjInfo - contains info about this copy of
 *	   a data object.
 *	   keyValPair_t *regParam - the keyword/value pair of items to be
 *	   modified. Valid keywords are given in char *dataObjCond[] in
 *	   rcGlobal.h. 
 *	   If the keyword ALL_REPL_STATUS_KW is used
 *	   the replStatus of the copy specified by dataObjInfo 
 *	   is marked NEWLY_CREATED_COPY and all other copies are
 *	   be marked OLD_COPY.  
 */
int chlModDataObjMeta(rsComm_t *rsComm, dataObjInfo_t *dataObjInfo,
		      keyValPair_t *regParam) {
   int i, j, status, upCols;
   rodsLong_t iVal;
   int status2;

   int mode=0;

   char logicalFileName[MAX_NAME_LEN];
   char logicalDirName[MAX_NAME_LEN];
   char *theVal;
   char replNum1[MAX_NAME_LEN];

   char *whereColsAndConds[10];
   char *whereValues[10];
   char idVal[MAX_NAME_LEN];
   int numConditions;
   char oldCopy[NAME_LEN];
   char newCopy[NAME_LEN];
   int adminMode;

   int maxCols=90;
   char *updateCols[90];
   char *updateVals[90];

   /* regParamNames has the argument names (in regParam) that this
      routine understands and colNames has the corresponding column
      names; one for one. */
   char *regParamNames[]={
      "replNum", "dataType", "dataSize",
      "rescName","filePath", "dataOwner", "dataOwnerZone", 
      "replStatus", "chksum", "dataExpiry",
      "dataComments", "dataCreate", "dataModify",  "rescGroupName",
      "dataMode", "END"
   };
   char *colNames[]={
      "data_repl_num", "data_type_name", "data_size",
      "resc_name", "data_path", "data_owner_name", "data_owner_zone",
      "data_is_dirty", "data_checksum", "data_expiry_ts",
      "r_comment", "create_ts", "modify_ts", "resc_group_name",
      "data_mode"
   };
   char objIdString[MAX_NAME_LEN];
   char *neededAccess;

   if (logSQL) rodsLog(LOG_SQL, "chlModDataObjMeta");

   if (regParam == NULL || dataObjInfo == NULL) {
      return (CAT_INVALID_ARGUMENT);
   }

   adminMode=0;
   theVal = getValByKey(regParam, IRODS_ADMIN_KW);
   if (theVal != NULL) adminMode=1;

   /* Set up the updateCols and updateVals arrays */
   for (i=0, j=0; i<maxCols; i++) {
      if (strcmp(regParamNames[i],"END")==0) break;
      theVal = getValByKey(regParam, regParamNames[i]);
      if (theVal != NULL) {
	 updateCols[j]=colNames[i];
	 updateVals[j]=theVal;
	 j++;
      }
   }
   upCols=j;

   /* If the only field is the chksum then the user only needs read
      access since we can trust that the server-side code is
      calculating it properly and checksum is a system-managed field.
      For example, when doing an irsync the server may calcuate a
      checksum and want to set it in the source copy.
   */
   neededAccess = ACCESS_MODIFY_METADATA;
   if (upCols==1 && strcmp(updateCols[0],"chksum")==0) {
      neededAccess = ACCESS_READ_OBJECT;
   }

   /* If dataExpiry is being updated, user needs to have 
      a greater access permission */
   theVal = getValByKey(regParam, "dataExpiry");
   if (theVal != NULL) {
      neededAccess = ACCESS_DELETE_OBJECT;
   }

   if (dataObjInfo->dataId <= 0) {
      status = splitPathByKey(dataObjInfo->objPath, 
			      logicalDirName, logicalFileName, '/');

      if (logSQL) rodsLog(LOG_SQL, "chlModDataObjMeta SQL 1 ");
      status = cmlGetIntegerValueFromSql(
	 "select coll_id from R_COLL_MAIN where coll_name=?", &iVal, 
	 logicalDirName, 0, 0, 0, 0, &icss);

      if (status) {
	 int i;
	 char errMsg[105];
	 snprintf(errMsg, 100, "collection '%s' is unknown", 
	       logicalDirName);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 _rollback("chlModDataObjMeta");
	 return(CAT_UNKNOWN_COLLECTION);
      }
      snprintf(objIdString, MAX_NAME_LEN, "%lld", iVal);

      if (logSQL) rodsLog(LOG_SQL, "chlModDataObjMeta SQL 2");
      status = cmlGetIntegerValueFromSql(
          "select data_id from R_DATA_MAIN where coll_id=? and data_name=?",
	  &iVal, objIdString, logicalFileName,  0, 0, 0, &icss);
      if (status) {
	 _rollback("chlModDataObjMeta");
	 return(CAT_UNKNOWN_FILE);
      }

      dataObjInfo->dataId = iVal;  /* return it for possible use next time, */
                                   /* and for use below */
   }

   snprintf(objIdString, MAX_NAME_LEN, "%lld", dataObjInfo->dataId);

   if (adminMode) {
      if (rsComm->clientUser.authInfo.authFlag != LOCAL_PRIV_USER_AUTH) {
	 return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
      }
   }
   else {
      status = cmlCheckDataObjId(objIdString, rsComm->clientUser.userName,
	      rsComm->clientUser.rodsZone, neededAccess, &icss);
      if (status) {
	 _rollback("chlModDataObjMeta");
	 return(CAT_NO_ACCESS_PERMISSION);
      } 
   }

   whereColsAndConds[0]="data_id=";
   snprintf(idVal, MAX_NAME_LEN, "%lld", dataObjInfo->dataId);
   whereValues[0]=idVal;
   numConditions=1;

   /* up here since this is usually called to modify the metadata of a
    * single repl */
   j = numConditions;
   whereColsAndConds[j]="data_repl_num=";
   snprintf(replNum1, MAX_NAME_LEN, "%d", dataObjInfo->replNum);
   whereValues[j]=replNum1;
   numConditions++;

   mode =0;
   if (getValByKey(regParam, ALL_REPL_STATUS_KW)) { 
      mode=1;
      /* mark this one as NEWLY_CREATED_COPY and others as OLD_COPY */
   }

   if (mode == 0) {
      if (logSQL) rodsLog(LOG_SQL, "chlModDataObjMeta SQL 4");
      status = cmlModifySingleTable("R_DATA_MAIN", updateCols, updateVals, 
				 whereColsAndConds, whereValues, upCols, 
				 numConditions, &icss);
   } else {
      /* mark this one as NEWLY_CREATED_COPY and others as OLD_COPY */
      j = upCols;
      updateCols[j]="data_is_dirty";
      snprintf(newCopy, NAME_LEN, "%d", NEWLY_CREATED_COPY);
      updateVals[j]=newCopy;
      upCols++;
      if (logSQL) rodsLog(LOG_SQL, "chlModDataObjMeta SQL 5");
      status = cmlModifySingleTable("R_DATA_MAIN", updateCols, updateVals, 
				 whereColsAndConds, whereValues, upCols, 
				 numConditions, &icss);
      if (status == 0) {
	 j = numConditions-1;
	 whereColsAndConds[j]="data_repl_num!=";
	 snprintf(replNum1, MAX_NAME_LEN, "%d", dataObjInfo->replNum);
	 whereValues[j]=replNum1;

         updateCols[0]="data_is_dirty";
         snprintf(oldCopy, NAME_LEN, "%d", OLD_COPY);
         updateVals[0]=oldCopy;
	 if (logSQL) rodsLog(LOG_SQL, "chlModDataObjMeta SQL 6");
         status2 = cmlModifySingleTable("R_DATA_MAIN", updateCols, updateVals,
                                       whereColsAndConds, whereValues, 1,
                               numConditions, &icss);

	 if (status2 != 0 && status2 != CAT_SUCCESS_BUT_WITH_NO_INFO) {
	    /* Ignore NO_INFO errors but not others */
	    rodsLog(LOG_NOTICE,
		    "chlModDataObjMeta cmlModifySingleTable failure for other replicas %d",
		    status2);
	    _rollback("chlModDataObjMeta");
	    return(status2);
	 }
      }
   }
   if (status != 0) {
      _rollback("chlModDataObjMeta");
      rodsLog(LOG_NOTICE,
	      "chlModDataObjMeta cmlModifySingleTable failure %d",
	      status);
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModDataObjMeta cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }

   return status;
}


/* 
 * chlRegDataObj - Register a new iRODS file (data object)
 * Input - rsComm_t *rsComm  - the server handle
 *         dataObjInfo_t *dataObjInfo - contains info about the data object.
 */
int chlRegDataObj(rsComm_t *rsComm, dataObjInfo_t *dataObjInfo) {
   char myTime[50];
   char logicalFileName[MAX_NAME_LEN];
   char logicalDirName[MAX_NAME_LEN];
   rodsLong_t seqNum;
   rodsLong_t iVal;
   char dataIdNum[MAX_NAME_LEN];
   char collIdNum[MAX_NAME_LEN];
   char dataReplNum[MAX_NAME_LEN];
   char dataSizeNum[MAX_NAME_LEN];
   char dataStatusNum[MAX_NAME_LEN];
   int status;

   if (logSQL) rodsLog(LOG_SQL, "chlRegDataObj");
   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (logSQL) rodsLog(LOG_SQL, "chlRegDataObj SQL 1 ");
   seqNum = cmlGetNextSeqVal(&icss);
   if (seqNum < 0) {
      rodsLog(LOG_NOTICE, "chlRegDataObj cmlGetNextSeqVal failure %d",
	      seqNum);
      _rollback("chlRegDataObj");
      return(seqNum);
   }
   snprintf(dataIdNum, MAX_NAME_LEN, "%lld", seqNum);
   dataObjInfo->dataId=seqNum;  /* store as output parameter */

   status = splitPathByKey(dataObjInfo->objPath, 
			   logicalDirName, logicalFileName, '/');


   /* Check that collection exists and user has write permission */
   iVal = cmlCheckDir(logicalDirName, rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone, 
		      ACCESS_MODIFY_OBJECT, &icss);
   if (iVal < 0) {
      int i;
      char errMsg[105];
      if (iVal==CAT_UNKNOWN_COLLECTION) {
	 snprintf(errMsg, 100, "collection '%s' is unknown", 
	       logicalDirName);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
      }
      if (iVal==CAT_NO_ACCESS_PERMISSION) {
	 snprintf(errMsg, 100, "no permission to update collection '%s'", 
		  logicalDirName);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
      }
      return (iVal);
   }
   snprintf(collIdNum, MAX_NAME_LEN, "%lld", iVal);

   /* Make sure no collection already exists by this name */
   if (logSQL) rodsLog(LOG_SQL, "chlRegDataObj SQL 4");
   status = cmlGetIntegerValueFromSql(
               "select coll_id from R_COLL_MAIN where coll_name=?",
	       &iVal, 
	       dataObjInfo->objPath, 0, 0, 0, 0, &icss);
   if (status == 0) {
      return(CAT_NAME_EXISTS_AS_COLLECTION);
   }

   if (logSQL) rodsLog(LOG_SQL, "chlRegDataObj SQL 5");
   status = cmlCheckNameToken("data_type", 
			      dataObjInfo->dataType, &icss);
   if (status !=0 ) {
      return(CAT_INVALID_DATA_TYPE);
   }

   snprintf(dataReplNum, MAX_NAME_LEN, "%d", dataObjInfo->replNum);
   snprintf(dataStatusNum, MAX_NAME_LEN, "%d", dataObjInfo->replStatus);
   snprintf(dataSizeNum, MAX_NAME_LEN, "%lld", dataObjInfo->dataSize);
   getNowStr(myTime);

   cllBindVars[0]=dataIdNum;
   cllBindVars[1]=collIdNum;
   cllBindVars[2]=logicalFileName;
   cllBindVars[3]=dataReplNum;
   cllBindVars[4]=dataObjInfo->version;
   cllBindVars[5]=dataObjInfo->dataType;
   cllBindVars[6]=dataSizeNum;
   cllBindVars[7]=dataObjInfo->rescGroupName;
   cllBindVars[8]=dataObjInfo->rescName;
   cllBindVars[9]=dataObjInfo->filePath;
   cllBindVars[10]=rsComm->clientUser.userName;
   cllBindVars[11]=rsComm->clientUser.rodsZone;
   cllBindVars[12]=dataStatusNum;
   cllBindVars[13]=dataObjInfo->dataMode;
   cllBindVars[14]=myTime;
   cllBindVars[15]=myTime;
   cllBindVarCount=16;
   if (logSQL) rodsLog(LOG_SQL, "chlRegDataObj SQL 6");
   status =  cmlExecuteNoAnswerSql(
       "insert into R_DATA_MAIN (data_id, coll_id, data_name, data_repl_num, data_version, data_type_name, data_size, resc_group_name, resc_name, data_path, data_owner_name, data_owner_zone, data_is_dirty, data_mode, create_ts, modify_ts) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", 
       &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegDataObj cmlExecuteNoAnswerSql failure %d",status);
      _rollback("chlRegDataObj");
      return(status);
   }

   cllBindVars[0]=dataIdNum;
   cllBindVars[1]=rsComm->clientUser.userName;
   cllBindVars[2]=rsComm->clientUser.rodsZone;
   cllBindVars[3]=ACCESS_OWN;
   cllBindVars[4]=myTime;
   cllBindVars[5]=myTime;
   cllBindVarCount=6;
   if (logSQL) rodsLog(LOG_SQL, "chlRegDataObj SQL 7");
   status =  cmlExecuteNoAnswerSql(
				   "insert into r_objt_access values (?, (select user_id from r_user_main where user_name=? and zone_name=?), (select token_id from R_TOKN_MAIN where token_namespace = 'access_type' and token_name = ?), ?, ?)",
				   &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegDataObj cmlExecuteNoAnswerSql insert access failure %d",
	      status);
      _rollback("chlRegDataObj");
      return(status);
   }

   status = cmlAudit3(AU_REGISTER_DATA_OBJ, dataIdNum,
		      rsComm->clientUser.userName, 
		      rsComm->clientUser.rodsZone, "", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegDataObj cmlAudit3 failure %d",
	      status);
      _rollback("chlRegDataObj");
      return(status);
   }


   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegDataObj cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }

   return(0);
}

/* 
 * chlRegReplica - Register a new iRODS replica file (data object)
 * Input - rsComm_t *rsComm  - the server handle
 *         srcDataObjInfo and dstDataObjInfo each contain information
 *         about the object.
 * The src dataId and replNum are used in a query, a few fields are updated
 * from dstDataObjInfo, and a new row inserted.
 */
int chlRegReplica(rsComm_t *rsComm, dataObjInfo_t *srcDataObjInfo,
		  dataObjInfo_t *dstDataObjInfo, keyValPair_t *condInput) {
   char myTime[50];
   char logicalFileName[MAX_NAME_LEN];
   char logicalDirName[MAX_NAME_LEN];
   rodsLong_t iVal;
   rodsLong_t status;
   char tSQL[MAX_SQL_SIZE];
   char *cVal[30];
   int i;
   int statementNumber;
   int nextReplNum;
   char nextRepl[30];
   char theColls[]="data_id, coll_id, data_name, data_repl_num, data_version, data_type_name, data_size, resc_group_name, resc_name, data_path, data_owner_name, data_owner_zone, data_is_dirty, data_status, data_checksum, data_expiry_ts, data_map_id, r_comment, create_ts, modify_ts";
   int IX_DATA_REPL_NUM=3;  /* index of data_repl_num in theColls */
   int IX_RESC_NAME=8;      /* index into theColls */
   int IX_RESC_GROUP_NAME=7;/* index into theColls */
   int IX_DATA_PATH=9;      /* index into theColls */
   int IX_CREATE_TS=18;
   int IX_MODIFY_TS=19;
   int nColumns=20;
   char objIdString[MAX_NAME_LEN];
   char replNumString[MAX_NAME_LEN];
   int adminMode;
   char *theVal;

   if (logSQL) rodsLog(LOG_SQL, "chlRegReplica");

   adminMode=0;
   if (condInput != NULL) {
      theVal = getValByKey(condInput, IRODS_ADMIN_KW);
      if (theVal != NULL) {
	 adminMode=1;
      }
   }

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   status = splitPathByKey(srcDataObjInfo->objPath, 
			   logicalDirName, logicalFileName, '/');

   if (adminMode) {
      if (rsComm->clientUser.authInfo.authFlag != LOCAL_PRIV_USER_AUTH) {
	 return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
      }
   }
   else {
      /* Check the access to the dataObj */
      if (logSQL) rodsLog(LOG_SQL, "chlRegReplica SQL 1 ");
      status = cmlCheckDataObjOnly(logicalDirName, logicalFileName,
				   rsComm->clientUser.userName, 
				   rsComm->clientUser.rodsZone, 
				   ACCESS_READ_OBJECT, &icss);
      if (status < 0) {
	 _rollback("chlRegReplica");
	 return(status);
      }
   }

   /* Get the next replica number */
   snprintf(objIdString, MAX_NAME_LEN, "%lld", srcDataObjInfo->dataId);
   if (logSQL) rodsLog(LOG_SQL, "chlRegReplica SQL 2");
   status = cmlGetIntegerValueFromSql(
        "select max(data_repl_num) from r_data_main where data_id = ?",
	&iVal, objIdString, 0, 0, 0, 0, &icss);

   if (status) {
      _rollback("chlRegReplica");
      return(status);
   }

   nextReplNum = iVal+1;
   snprintf(nextRepl, 30, "%d", nextReplNum);
   dstDataObjInfo->replNum = nextReplNum; /* return new replica number */
   snprintf(replNumString, MAX_NAME_LEN, "%d", srcDataObjInfo->replNum);
   snprintf(tSQL, MAX_SQL_SIZE,
	    "select %s from r_data_main where data_id = ? and data_repl_num = ?",
	    theColls);
   if (logSQL) rodsLog(LOG_SQL, "chlRegReplica SQL 3");
   status = cmlGetOneRowFromSqlV2(tSQL, cVal, nColumns, 
				   objIdString, replNumString, &icss);
   if (status < 0) {
      _rollback("chlRegReplica");
      return(status);
   }
   statementNumber = status;

   cVal[IX_DATA_REPL_NUM]=nextRepl;
   cVal[IX_RESC_NAME]=dstDataObjInfo->rescName;
   cVal[IX_RESC_GROUP_NAME]=dstDataObjInfo->rescGroupName;
   cVal[IX_DATA_PATH]=dstDataObjInfo->filePath;

   getNowStr(myTime);
   cVal[IX_MODIFY_TS]=myTime;
   cVal[IX_CREATE_TS]=myTime;

   for (i=0;i<nColumns;i++) {
      cllBindVars[i]=cVal[i];
   }
   cllBindVarCount = nColumns;
   snprintf(tSQL, MAX_SQL_SIZE, "insert into r_data_main ( %s ) values (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
	    theColls);
   if (logSQL) rodsLog(LOG_SQL, "chlRegReplica SQL 4");
   status = cmlExecuteNoAnswerSql(tSQL,  &icss);
   if (status < 0) {
      rodsLog(LOG_NOTICE, 
	      "chlRegReplica cmlExecuteNoAnswerSql(insert) failure %d",
	      status);
      _rollback("chlRegReplica");
      return(status);
   }

   cmlFreeStatement(statementNumber, &icss);
   if (status < 0) {
      rodsLog(LOG_NOTICE, "chlRegReplica cmlFreeStatement failure %d", status);
      return(status);
   }

   status = cmlAudit3(AU_REGISTER_DATA_REPLICA, objIdString,
		      rsComm->clientUser.userName, 
		      rsComm->clientUser.rodsZone, nextRepl, &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegDataReplica cmlAudit3 failure %d",
	      status);
      _rollback("chlRegReplica");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegReplica cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }

   return(0);
}

/*
 * unregDataObj - Unregister a data object
 * Input - rsComm_t *rsComm  - the server handle
 *         dataObjInfo_t *dataObjInfo - contains info about the data object.
 *	   keyValPair_t *condInput - used to specify a admin-mode.
 */
int chlUnregDataObj (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo, 
		 keyValPair_t *condInput) {
   char logicalFileName[MAX_NAME_LEN];
   char logicalDirName[MAX_NAME_LEN];
   rodsLong_t status;
   int i;
   char tSQL[MAX_SQL_SIZE];
   char replNumber[30];
   char dataObjNumber[30];
   char cVal[30];
   int adminMode;
   int trashMode;
   char *theVal;
   char checkPath[MAX_NAME_LEN];

   dataObjNumber[0]='\0';
   if (logSQL) rodsLog(LOG_SQL, "chlUnregDataObj");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   adminMode=0;
   trashMode=0;
   if (condInput != NULL) {
      theVal = getValByKey(condInput, IRODS_ADMIN_KW);
      if (theVal != NULL) {
	 adminMode=1;
      }
      theVal = getValByKey(condInput, IRODS_ADMIN_RMTRASH_KW);
      if (theVal != NULL) {
	 adminMode=1;
	 trashMode=1;
      }
   }

   status = splitPathByKey(dataObjInfo->objPath, 
			   logicalDirName, logicalFileName, '/');


   if (adminMode==0) {
      /* Check the access to the dataObj */
      if (logSQL) rodsLog(LOG_SQL, "chlUnregDataObj SQL 1 ");
      status = cmlCheckDataObjOnly(logicalDirName, logicalFileName,
				   rsComm->clientUser.userName, 
				   rsComm->clientUser.rodsZone, 
				   ACCESS_DELETE_OBJECT, &icss);
      if (status < 0) {
	 _rollback("chlUnregDataObj");
	 return(status);  /* convert long to int */
      }
      snprintf(dataObjNumber, 30, "%lld", status);
   }
   else {
      if (rsComm->clientUser.authInfo.authFlag != LOCAL_PRIV_USER_AUTH) {
	 return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
      }
      if (trashMode) {
	 int len;
	 status = getLocalZone();
	 if (status) return(status);
	 snprintf(checkPath, MAX_NAME_LEN, "/%s/trash", localZone);
	 len = strlen(checkPath);
	 if (strncmp(checkPath, logicalDirName, len) != 0) {
	    i = addRErrorMsg (&rsComm->rError, 0, 
	      "TRASH_KW but not zone/trash path");
	    return(CAT_INVALID_ARGUMENT);
	 }
	 if (dataObjInfo->dataId > 0) {
	    snprintf(dataObjNumber, 30, "%lld", dataObjInfo->dataId);
	 }
      }
      else {
	 if (dataObjInfo->replNum >= 0 && dataObjInfo->dataId >= 0) {
	    /* Check for a different replica */
	    snprintf(dataObjNumber, 30, "%lld", dataObjInfo->dataId);
	    snprintf(replNumber, 30, "%d", dataObjInfo->replNum);
	    if (logSQL) rodsLog(LOG_SQL, "chlUnregDataObj SQL 2");
	    status = cmlGetStringValueFromSql(
                  "select data_repl_num from r_data_main where data_id=? and data_repl_num!=?",
		  cVal,
                  30,
		  dataObjNumber,
		  replNumber,
		  &icss);
	    if (status) {
	       i = addRErrorMsg (&rsComm->rError, 0, 
		 "This is the last replica, removal by admin not allowed");
	       return(CAT_LAST_REPLICA);
	    }
	 }
	 else {
	    i = addRErrorMsg (&rsComm->rError, 0, 
			      "dataId and replNum required");
	    _rollback("chlUnregDataObj");
	    return (CAT_INVALID_ARGUMENT);
	 }
      }
   }

   cllBindVars[0]=logicalDirName;
   cllBindVars[1]=logicalFileName;
   if (dataObjInfo->replNum >= 0) {
      snprintf(replNumber, 30, "%d", dataObjInfo->replNum);
      cllBindVars[2]=replNumber;
      cllBindVarCount=3;
      if (logSQL) rodsLog(LOG_SQL, "chlUnregDataObj SQL 4");
      snprintf(tSQL, MAX_SQL_SIZE, 
	       "delete from r_data_main where coll_id=(select coll_id from R_COLL_MAIN where coll_name=?) and data_name=? and data_repl_num=?");
   }
   else {
      cllBindVarCount=2;
      if (logSQL) rodsLog(LOG_SQL, "chlUnregDataObj SQL 5");
      snprintf(tSQL, MAX_SQL_SIZE, 
	    "delete from r_data_main where coll_id=(select coll_id from R_COLL_MAIN where coll_name=?) and data_name=?");
   }
   status =  cmlExecuteNoAnswerSql(tSQL, &icss);
   if (status) {
      if (status == CAT_SUCCESS_BUT_WITH_NO_INFO) {
	 int i;
	 char errMsg[105];
	 status = CAT_UNKNOWN_FILE;  /* More accurate, in this case */
	 snprintf(errMsg, 100, "data object '%s' is unknown", 
	       logicalFileName);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 return(status);
      }
      _rollback("chlUnregDataObj");
      return(status);
   }

   /* delete the access rows, if we just deleted the last replica */
   if (dataObjNumber[0]!='\0') {
      cllBindVars[0]=dataObjNumber;
      cllBindVars[1]=dataObjNumber;
      cllBindVarCount=2;
      if (logSQL) rodsLog(LOG_SQL, "chlUnregDataObj SQL 3");
      status = cmlExecuteNoAnswerSql(
	       "delete from r_objt_access where object_id=? and not exists (select * from r_data_main where data_id=?)", &icss);
   }

   /* Audit */
   if (dataObjNumber[0]!='\0') {
      status = cmlAudit3(AU_UNREGISTER_DATA_OBJ, dataObjNumber,
		      rsComm->clientUser.userName, 
		      rsComm->clientUser.rodsZone, "", &icss);
   }
   else {
      status = cmlAudit3(AU_UNREGISTER_DATA_OBJ, "0",
			 rsComm->clientUser.userName, 
			 rsComm->clientUser.rodsZone,
			 dataObjInfo->objPath, &icss);
   }
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlUnRegDataObj cmlAudit3 failure %d",
	      status);
      _rollback("chlUnregDataObj");
      return(status);
   }

   
   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlUnregDataObj cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }

   return(status);

}

/* 
 * chlRegRuleExec - Register a new iRODS delayed rule execution object
 * Input - rsComm_t *rsComm  - the server handle
 *         ruleExecSubmitInp_t *ruleExecSubmitInp - contains info about the
 *             delayed rule.
 */
int chlRegRuleExec(rsComm_t *rsComm, 
		      ruleExecSubmitInp_t *ruleExecSubmitInp) {
   char myTime[50];
   rodsLong_t seqNum;
   char ruleExecIdNum[MAX_NAME_LEN];
   int status;

   if (logSQL) rodsLog(LOG_SQL, "chlRegRuleExec");
   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (logSQL) rodsLog(LOG_SQL, "chlRegRuleExec SQL 1 ");
   seqNum = cmlGetNextSeqVal(&icss);
   if (seqNum < 0) {
      rodsLog(LOG_NOTICE, "chlRegRuleExec cmlGetNextSeqVal failure %d",
	      seqNum);
      _rollback("chlRegRuleExec");
      return(seqNum);
   }
   snprintf(ruleExecIdNum, MAX_NAME_LEN, "%lld", seqNum);

   /* store as output parameter */
   strncpy(ruleExecSubmitInp->ruleExecId,ruleExecIdNum, NAME_LEN); 

   getNowStr(myTime);

   cllBindVars[0]=ruleExecIdNum;
   cllBindVars[1]=ruleExecSubmitInp->ruleName;
   cllBindVars[2]=ruleExecSubmitInp->reiFilePath;
   cllBindVars[3]=ruleExecSubmitInp->userName;
   cllBindVars[4]=ruleExecSubmitInp->exeAddress;
   cllBindVars[5]=ruleExecSubmitInp->exeTime;
   cllBindVars[6]=ruleExecSubmitInp->exeFrequency;
   cllBindVars[7]=ruleExecSubmitInp->priority;
   cllBindVars[8]=ruleExecSubmitInp->estimateExeTime;
   cllBindVars[9]=ruleExecSubmitInp->notificationAddr;
   cllBindVars[10]=myTime;
   cllBindVars[11]=myTime;

   cllBindVarCount=12;
   if (logSQL) rodsLog(LOG_SQL, "chlRegRuleExec SQL 2");
   status =  cmlExecuteNoAnswerSql(
				   "insert into R_RULE_EXEC (rule_exec_id, rule_name, rei_file_path, user_name, exe_address, exe_time, exe_frequency, priority, estimated_exe_time, notification_addr, create_ts, modify_ts) values (?,?,?,?,?,?,?,?,?,?,?,?)",
				   &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegRuleExec cmlExecuteNoAnswerSql(insert) failure %d",status);
      _rollback("chlRegRuleExec");
      return(status);

   }
 
   /* Audit */
   status = cmlAudit3(AU_REGISTER_DELAYED_RULE,  ruleExecIdNum,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      ruleExecSubmitInp->ruleName, &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegRuleExec cmlAudit3 failure %d",
	      status);
      _rollback("chlRegRuleExec");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegRuleExec cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }

   return(0);
}

/* 
 * chlModRuleExec - Modify the metadata of an existing (delayed) 
 * Rule Execution object. 
 * Input - rsComm_t *rsComm  - the server handle
 *         char *ruleExecId - the id of the object to change
 *	   keyValPair_t *regParam - the keyword/value pair of items to be
 *	   modified.
 */
int chlModRuleExec(rsComm_t *rsComm, char *ruleExecId,
		      keyValPair_t *regParam) {
   int i, j, status;

   char tSQL[MAX_SQL_SIZE];
   char *theVal;

   int maxCols=90;

   /* regParamNames has the argument names (in regParam) that this
      routine understands and colNames has the corresponding column
      names; one for one. */   
   char *regParamNames[]={ 
      RULE_NAME_KW, RULE_REI_FILE_PATH_KW, RULE_USER_NAME_KW, 
      RULE_EXE_ADDRESS_KW, RULE_EXE_TIME_KW,
      RULE_EXE_FREQUENCY_KW, RULE_PRIORITY_KW, RULE_ESTIMATE_EXE_TIME_KW,
      RULE_NOTIFICATION_ADDR_KW, RULE_LAST_EXE_TIME_KW, 
      RULE_EXE_STATUS_KW,
     "END"
   };
   char *colNames[]={
      "rule_name", "rei_file_path", "user_name",
      "exe_address", "exe_time", "exe_frequency", "priority",
      "estimated_exe_time", "notification_addr", 
      "last_exe_time", "exe_status",
      "create_ts", "modify_ts", 
   };

   if (logSQL) rodsLog(LOG_SQL, "chlModRuleExec");

   if (regParam == NULL || ruleExecId == NULL) {
      return (CAT_INVALID_ARGUMENT);
   }

   snprintf(tSQL, MAX_SQL_SIZE, "update R_RULE_EXEC set ");

   for (i=0, j=0; i<maxCols; i++) {
      if (strcmp(regParamNames[i],"END")==0) break;
      theVal = getValByKey(regParam, regParamNames[i]);
      if (theVal != NULL) {
	 if (j>0) rstrcat(tSQL, "," , MAX_SQL_SIZE);
	 rstrcat(tSQL, colNames[i] , MAX_SQL_SIZE);
	 rstrcat(tSQL, "=?", MAX_SQL_SIZE);
	 cllBindVars[j++]=theVal;
      }
   }

   if (j == 0) {
      return (CAT_INVALID_ARGUMENT);
   }

   rstrcat(tSQL, "where rule_exec_id=?", MAX_SQL_SIZE);
   cllBindVars[j++]=ruleExecId;
   cllBindVarCount=j;

   if (logSQL) rodsLog(LOG_SQL, "chlModRuleExec SQL 1 ");
   status =  cmlExecuteNoAnswerSql(tSQL, &icss);

   if (status != 0) {
      _rollback("chlModRuleExec");
      rodsLog(LOG_NOTICE,
	      "chlModRuleExec cmlExecuteNoAnswer(update) failure %d",
	      status);
      return(status);
   }

   /* Audit */
   status = cmlAudit3(AU_MODIFY_DELAYED_RULE,  ruleExecId,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      "", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModRuleExec cmlAudit3 failure %d",
	      status);
      _rollback("chlModRuleExec");
      return(status);
   }


   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModRuleExecMeta cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }

   return status;
}

/* delete a delayed rule execution entry */
int chlDelRuleExec(rsComm_t *rsComm, 
	       char *ruleExecId) {
   int status;

   if (logSQL) rodsLog(LOG_SQL, "chlDelRuleExec");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   cllBindVars[cllBindVarCount++]=ruleExecId;
   if (logSQL) rodsLog(LOG_SQL, "chlDelRuleExec SQL 1 ");
   status =  cmlExecuteNoAnswerSql(
			   "delete from r_rule_exec where rule_exec_id=?",
			   &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlDelRuleExec delete failure %d",
	      status);
      _rollback("chlDelRuleExec");
      return(status);
   }

   /* Audit */
   status = cmlAudit3(AU_DELETE_DELAYED_RULE,  ruleExecId,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      "", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlDelRuleExec cmlAudit3 failure %d",
	      status);
      _rollback("chlDelRuleExec");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlDelRuleExec cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }
   return(status);
}



/*
 This can be called to test other routines, passing in an actual
 rsComm structure.

 For example, in _rsSomething instead of:
    status = chlSomething (SomethingInp);
 change it to this:
    status = chlTest(rsComm, Something->name);
 and it tests the chlRegDataObj function.
 */
int chlTest(rsComm_t *rsComm, char *name) {
   dataObjInfo_t dataObjInfo;

   strcpy(dataObjInfo.objPath, name);
   dataObjInfo.replNum=1;
   strcpy(dataObjInfo.version, "12");
   strcpy(dataObjInfo.dataType, "URL");
   dataObjInfo.dataSize=42;

   strcpy(dataObjInfo.rescName, "resc A");

   strcpy(dataObjInfo.filePath, "/scratch/slocal/test1");

   dataObjInfo.replStatus=5;

   return (chlRegDataObj(rsComm, &dataObjInfo));
}

/* register a Resource */
int chlRegResc(rsComm_t *rsComm, 
	       rescInfo_t *rescInfo) {
   rodsLong_t seqNum;
   char idNum[MAX_SQL_SIZE];
   int status;
   char myTime[50];

   if (logSQL) rodsLog(LOG_SQL, "chlRegResc");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   if (logSQL) rodsLog(LOG_SQL, "chlRegResc SQL 1 ");
   seqNum = cmlGetNextSeqVal(&icss);
   if (seqNum < 0) {
      rodsLog(LOG_NOTICE, "chlRegResc cmlGetNextSeqVal failure %d",
	      seqNum);
      _rollback("chlRegResc");
      return(seqNum);
   }
   snprintf(idNum, MAX_SQL_SIZE, "%lld", seqNum);

   status = getLocalZone();
   if (status) return(status);

   if (rescInfo->zoneName != NULL && strlen(rescInfo->zoneName) > 0) {
      if (strcmp(rescInfo->zoneName, localZone) !=0) {
	 int i;
	 i = addRErrorMsg (&rsComm->rError, 0, 
			   "Currently, resources must be in the local zone");
	 return(CAT_INVALID_ZONE);
      }
   }

   if (logSQL) rodsLog(LOG_SQL, "chlRegResc SQL 2");
   status = cmlCheckNameToken("resc_type", rescInfo->rescType, &icss);
   if (status !=0 ) {
      int i;
      char errMsg[105];
      snprintf(errMsg, 100, "resource_type '%s' is not valid", 
	       rescInfo->rescType);
      i = addRErrorMsg (&rsComm->rError, 0, errMsg);
      return(CAT_INVALID_RESOURCE_TYPE);
   }

   if (logSQL) rodsLog(LOG_SQL, "chlRegResc SQL 3");
   status = cmlCheckNameToken("resc_class", rescInfo->rescClass, &icss);
   if (status !=0 ) {
      return(CAT_INVALID_RESOURCE_CLASS);
   }

   if (strlen(rescInfo->rescLoc)<1) {
      return(CAT_INVALID_RESOURCE_NET_ADDR);
   }

   if (strlen(rescInfo->rescVaultPath)<1) {
      return(CAT_INVALID_RESOURCE_VAULT_PATH);
   }

   status = getLocalZone();
   if (status) return(status);

   getNowStr(myTime);

   cllBindVars[0]=idNum;
   cllBindVars[1]=rescInfo->rescName;
   cllBindVars[2]=localZone;
   cllBindVars[3]=rescInfo->rescType;
   cllBindVars[4]=rescInfo->rescClass;
   cllBindVars[5]=rescInfo->rescLoc;
   cllBindVars[6]=rescInfo->rescVaultPath;
   cllBindVars[7]=myTime;
   cllBindVars[8]=myTime;

   cllBindVarCount=9;
   if (logSQL) rodsLog(LOG_SQL, "chlRegResc SQL 4");
   status =  cmlExecuteNoAnswerSql(
		"insert into R_RESC_MAIN (resc_id, resc_name, zone_name, resc_type_name, resc_class_name, resc_net, resc_def_path, create_ts, modify_ts) values (?,?,?,?,?,?,?,?,?)",
		&icss);

   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegResc cmlExectuteNoAnswerSql(insert) failure %d",
	      status);
      _rollback("chlRegResc");
      return(status);
   }

   /* Audit */
   status = cmlAudit3(AU_REGISTER_RESOURCE,  idNum,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      rescInfo->rescName, &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegResc cmlAudit3 failure %d",
	      status);
      _rollback("chlRegResc");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegResc cmlExecuteNoAnswerSql commit failure %d",status);
      return(status);
   }
   return(status);
}

/* delete a Resource */
int chlDelResc(rsComm_t *rsComm, 
	       rescInfo_t *rescInfo) {
   int status;
   rodsLong_t iVal;
   char rescId[MAX_NAME_LEN];

   if (logSQL) rodsLog(LOG_SQL, "chlDelResc");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   if (logSQL) rodsLog(LOG_SQL, "chlDelResc SQL 1 ");
   status = cmlGetIntegerValueFromSql(
	    "select data_id from r_data_main where resc_name=?",
	     &iVal, rescInfo->rescName, 0, 0, 0, 0, &icss);
   if (status != CAT_NO_ROWS_FOUND) {
      if (status == 0) {
	 int i;
	 char errMsg[105];
	 snprintf(errMsg, 100, 
		  "resource '%s' contains one or more dataObjects",
		  rescInfo->rescName);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 return(CAT_RESOURCE_NOT_EMPTY);
      }
      _rollback("chlDelResc");
      return(status);
   }

   status = getLocalZone();
   if (status) return(status);

   /* get rescId for possible audit call; won't be available after delete */
   rescId[0]='\0';
   if (logSQL) rodsLog(LOG_SQL, "chlDelResc SQL 2 ");
   status = cmlGetStringValueFromSql(
       "select resc_id from r_resc_main where resc_name=?",
       rescId, MAX_NAME_LEN, rescInfo->rescName, 0, &icss);
   if (status) {
      if (status == CAT_SUCCESS_BUT_WITH_NO_INFO) {
	 int i;
	 char errMsg[105];
	 snprintf(errMsg, 100, 
		  "resource '%s' does not exist",
		  rescInfo->rescName);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 return(status);
      }
      _rollback("chlDelResc");
      return(status);
   }

   cllBindVars[cllBindVarCount++]=rescInfo->rescName;
   if (logSQL) rodsLog(LOG_SQL, "chlDelResc SQL 3");
   status = cmlExecuteNoAnswerSql(
               "delete from r_resc_main where resc_name=?",
	       &icss);
   if (status) {
      if (status == CAT_SUCCESS_BUT_WITH_NO_INFO) {
	 int i;
	 char errMsg[105];
	 snprintf(errMsg, 100, 
		  "resource '%s' does not exist",
		  rescInfo->rescName);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 return(status);
      }
      _rollback("chlDelResc");
      return(status);
   }

   /* Audit */
   status = cmlAudit3(AU_DELETE_RESOURCE,  
		      rescId,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      rescInfo->rescName, 
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlDelResc cmlAudit3 failure %d",
	      status);
      _rollback("chlDelResc");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlDelResc cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }
   return(status);
}

/* 
 Issue a rollback command.

 If we don't do a commit, the updates will not be saved in the
 database but will still exist during the current connection.  Since
 iadmin connects once and then can issue multiple commands there are
 situations where we need to rollback.

 For example, if the user's zone is wrong the code will first remove the
 home collection and then fail when removing the user and we need to
 rollback or the next attempt will show the collection as missing.

*/
int chlRollback(rsComm_t *rsComm) {
   int status;
   if (logSQL) rodsLog(LOG_SQL, "chlRollback - SQL 1 ");
   status =  cmlExecuteNoAnswerSql("rollback", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRollback cmlExecuteNoAnswerSql failure %d",
	      status);
   }
   return(status);
}

/*
 Issue a commit command.
 This is called to commit changes to the database.
 Some of the chl functions also commit changes upon success but some
 do not, having the caller (microservice, perhaps) either commit or
 rollback.
 */
int chlCommit(rsComm_t *rsComm) {
   int status;
   if (logSQL) rodsLog(LOG_SQL, "chlCommit - SQL 1 ");
   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlCommit cmlExecuteNoAnswerSql failure %d",
	      status);
   }
   return(status);
}

/* Delete a User, Rule Engine version */
int chlDelUserRE(rsComm_t *rsComm, userInfo_t *userInfo) {
   int status;
   char iValStr[200];
   char zoneToUse[MAX_NAME_LEN];
   char userStr[200];
   char userName2[NAME_LEN];
   char zoneName[NAME_LEN];

   if (logSQL) rodsLog(LOG_SQL, "chlDelUserRE");

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   status = getLocalZone();
   if (status) return(status);

   strncpy(zoneToUse, localZone, MAX_NAME_LEN);
   if (strlen(userInfo->rodsZone)>0) {
      strncpy(zoneToUse, userInfo->rodsZone, MAX_NAME_LEN);
   }

   status = parseUserName(userInfo->userName, userName2, zoneName);
   if (zoneName[0]!='\0') {
      rstrcpy(zoneToUse, zoneName, NAME_LEN);
   }

   if (logSQL) rodsLog(LOG_SQL, "chlDelUserRE SQL 1 ");
   status = cmlGetStringValueFromSql(
            "select user_id from r_user_main where user_name=? and zone_name=?",
	    iValStr, 200, userName2, zoneToUse, &icss);
   if (status==CAT_SUCCESS_BUT_WITH_NO_INFO ||
       status==CAT_NO_ROWS_FOUND) {
      int i;
      i = addRErrorMsg (&rsComm->rError, 0, "Invalid user");
      return(CAT_INVALID_USER); 
   }
   if (status) {
      _rollback("chlDelUserRE");
      return(status);
   }

   cllBindVars[cllBindVarCount++]=userName2;
   cllBindVars[cllBindVarCount++]=zoneToUse;
   if (logSQL) rodsLog(LOG_SQL, "chlDelUserRE SQL 2");
   status = cmlExecuteNoAnswerSql(
	    "delete from r_user_main where user_name=? and zone_name=?",
	    &icss);
   if (status==CAT_SUCCESS_BUT_WITH_NO_INFO) return(CAT_INVALID_USER); 
   if (status) {
      _rollback("chlDelUserRE");
      return(status);
   }

   cllBindVars[cllBindVarCount++]=iValStr;
   if (logSQL) rodsLog(LOG_SQL, "chlDelUserRE SQL 3");
   status = cmlExecuteNoAnswerSql(
	    "delete from r_user_password where user_id=?",
	    &icss);
   if (status!=0 && status != CAT_SUCCESS_BUT_WITH_NO_INFO) {
      int i;
      char errMsg[MAX_NAME_LEN+40];
      rodsLog(LOG_NOTICE,
	      "chlDelUserRE delete password failure %d",
	      status);
      snprintf(errMsg, MAX_NAME_LEN+40, "Error removing password entry");
      i = addRErrorMsg (&rsComm->rError, 0, errMsg);
      _rollback("chlDelUserRE");
      return(status);
   }

   /* Remove both the special user_id = group_user_id entry and any
      other access entries for this user */
   cllBindVars[cllBindVarCount++]=iValStr;
   if (logSQL) rodsLog(LOG_SQL, "chlDelUserRE SQL 4");
   status = cmlExecuteNoAnswerSql(
	    "delete from r_user_group where user_id=?",
	    &icss);
   if (status!=0 && status != CAT_SUCCESS_BUT_WITH_NO_INFO) {
      int i;
      char errMsg[MAX_NAME_LEN+40];
      rodsLog(LOG_NOTICE,
	      "chlDelUserRE delete user_group entry failure %d",
	      status);
      snprintf(errMsg, MAX_NAME_LEN+40, "Error removing user_group entry");
      i = addRErrorMsg (&rsComm->rError, 0, errMsg);
      _rollback("chlDelUserRE");
      return(status);
   }

   /* Audit */
   snprintf(userStr, 190, "%s#%s",
	    userName2, zoneToUse);
   status = cmlAudit3(AU_DELETE_USER_RE,  
		      iValStr,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      userStr,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlDelUserRE cmlAudit3 failure %d",
	      status);
      _rollback("chlDelUserRE");
      return(status);
   }

   return(0);
}

/*
 Register a Collection by the admin.
 There are cases where the irods admin needs to create collections,
 for a new user, for example; thus the create user rule/microservices
 make use of this.
 */
int chlRegCollByAdmin(rsComm_t *rsComm, collInfo_t *collInfo)
{
   char myTime[50];
   char logicalEndName[MAX_NAME_LEN];
   char logicalParentDirName[MAX_NAME_LEN];
   rodsLong_t iVal;
   char collIdNum[MAX_NAME_LEN];
   char nextStr[MAX_NAME_LEN];
   char currStr[MAX_NAME_LEN];
   char currStr2[MAX_SQL_SIZE];
   int status;
   char tSQL[MAX_SQL_SIZE];
   char userName2[NAME_LEN];
   char zoneName[NAME_LEN];

   if (logSQL) rodsLog(LOG_SQL, "chlRegCollByAdmin");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   if (collInfo==0) {
      return(CAT_INVALID_ARGUMENT);
   }

   status = splitPathByKey(collInfo->collName, 
			   logicalParentDirName, logicalEndName, '/');

   if (strlen(logicalParentDirName)==0) {
      strcpy(logicalParentDirName, "/");
      strcpy(logicalEndName, collInfo->collName+1);
   }

   /* Check that the parent collection exists */
   if (logSQL) rodsLog(LOG_SQL, "chlRegCollByAdmin SQL 1 ");
   status = cmlGetIntegerValueFromSql(
             "select coll_id from R_COLL_MAIN where coll_name=?",
	     &iVal, logicalParentDirName, 0, 0, 0, 0, &icss);
   if (status < 0) {
      int i;
      char errMsg[MAX_NAME_LEN+40];
      if (status == CAT_NO_ROWS_FOUND) {
	 snprintf(errMsg, MAX_NAME_LEN+40, 
		  "collection '%s' is unknown, cannot create %s under it",
		  logicalParentDirName, logicalEndName);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 return(status);
      }
      _rollback("chlRegCollByAdmin");
      return(status);
   }

   snprintf(collIdNum, MAX_NAME_LEN, "%d", status);

   /* String to get next sequence item for objects */
   cllNextValueString("R_ObjectID", nextStr, MAX_NAME_LEN);

   if (logSQL) rodsLog(LOG_SQL, "chlRegCollByAdmin SQL 2");
   snprintf(tSQL, MAX_SQL_SIZE, 
	    "insert into R_COLL_MAIN (coll_id, parent_coll_name, coll_name, coll_owner_name, coll_owner_zone, coll_type, coll_info1, coll_info2, create_ts, modify_ts) values (%s, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
	    nextStr);

   getNowStr(myTime);

   status = getLocalZone();
   if (status) return(status);

   /* Parse input name into user and zone */
   status = parseUserName(collInfo->collOwnerName, userName2, zoneName);
   if (zoneName[0]=='\0') {
      rstrcpy(zoneName, localZone, NAME_LEN);
   }

   cllBindVars[cllBindVarCount++]=logicalParentDirName;
   cllBindVars[cllBindVarCount++]=collInfo->collName;
   cllBindVars[cllBindVarCount++]=userName2;
   if (strlen(collInfo->collOwnerZone)>0) {
      cllBindVars[cllBindVarCount++]=collInfo->collOwnerZone;
   }
   else {
      cllBindVars[cllBindVarCount++]=zoneName;
   }
   if (collInfo->collType != NULL) {
      cllBindVars[cllBindVarCount++]=collInfo->collType;
   }
   else {
      cllBindVars[cllBindVarCount++]=""; 
   }
   if (collInfo->collInfo1 != NULL) {
      cllBindVars[cllBindVarCount++]=collInfo->collInfo1;
   }
   else {
      cllBindVars[cllBindVarCount++]="";
   }
   if (collInfo->collInfo2 != NULL) {
      cllBindVars[cllBindVarCount++]=collInfo->collInfo2;
   }
   else {
      cllBindVars[cllBindVarCount++]="";
   }
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=myTime;
   if (logSQL) rodsLog(LOG_SQL, "chlRegCollByAdmin SQL 3");
   status =  cmlExecuteNoAnswerSql(tSQL,
				   &icss);
   if (status != 0) {
      int i;
      char errMsg[105];
      if (status == CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME) {
	 snprintf(errMsg, 100, "Error %d %s",
		  status,
		  "CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME"
		  );
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
      }

      rodsLog(LOG_NOTICE,
	      "chlRegCollByAdmin cmlExecuteNoAnswerSQL(insert) failure %d"
	      ,status);
      _rollback("chlRegCollByAdmin");
      return(status);
   }

   /* String to get current sequence item for objects */
   cllCurrentValueString("R_ObjectID", currStr, MAX_NAME_LEN);
   snprintf(currStr2, MAX_SQL_SIZE, " %s ", currStr);

   cllBindVars[cllBindVarCount++]=userName2;
   cllBindVars[cllBindVarCount++]=zoneName;
   cllBindVars[cllBindVarCount++]=ACCESS_OWN;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=myTime;

   snprintf(tSQL, MAX_SQL_SIZE, 
	    "insert into r_objt_access values (%s, (select user_id from r_user_main where user_name=? and zone_name=?), (select token_id from R_TOKN_MAIN where token_namespace = 'access_type' and token_name = ?), ?, ?)",
	    currStr2);
   if (logSQL) rodsLog(LOG_SQL, "chlRegCollByAdmin SQL 4");
   status =  cmlExecuteNoAnswerSql(tSQL, &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegCollByAdmin cmlExecuteNoAnswerSql(insert access) failure %d",
	      status);
      _rollback("chlRegCollByAdmin");
      return(status);
   }

   /* Audit */
   status = cmlAudit4(AU_REGISTER_COLL_BY_ADMIN,  
		      currStr2,
		      "",
		      userName2,
		      zoneName,
		      rsComm->clientUser.userName,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegCollByAdmin cmlAudit4 failure %d",
	      status);
      _rollback("chlRegCollByAdmin");
      return(status);
   }

   return(0);
}

/* 
 * chlRegColl - register a collection
 * Input -
 *   rcComm_t *conn - The client connection handle.
 *   collInfo_t *collInfo - generic coll input. Relevant items are:
 *      collName - the collection to be registered, and optionally
 *      collType, collInfo1 and/or collInfo2.
 *   We may need a kevValPair_t sometime, but currently not used.
 */
int chlRegColl(rsComm_t *rsComm, collInfo_t *collInfo) {
   char myTime[50];
   char logicalEndName[MAX_NAME_LEN];
   char logicalParentDirName[MAX_NAME_LEN];
   rodsLong_t iVal;
   char collIdNum[MAX_NAME_LEN];
   char nextStr[MAX_NAME_LEN];
   char currStr[MAX_NAME_LEN];
   char currStr2[MAX_SQL_SIZE];
   rodsLong_t status;
   char tSQL[MAX_SQL_SIZE];

   if (logSQL) rodsLog(LOG_SQL, "chlRegColl");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   status = splitPathByKey(collInfo->collName, 
			   logicalParentDirName, logicalEndName, '/');

   if (strlen(logicalParentDirName)==0) {
      strcpy(logicalParentDirName, "/");
      strcpy(logicalEndName, collInfo->collName+1);
   }

   /* Check that the parent collection exists and user has write permission,
      and get the collectionID */
   if (logSQL) rodsLog(LOG_SQL, "chlRegColl SQL 1 ");
   status = cmlCheckDir(logicalParentDirName, 
			rsComm->clientUser.userName, 
			rsComm->clientUser.rodsZone, 
			ACCESS_MODIFY_OBJECT, &icss);
   if (status < 0) {
      int i;
      char errMsg[105];
      if (status == CAT_UNKNOWN_COLLECTION) {
	 snprintf(errMsg, 100, "collection '%s' is unknown",
		  logicalParentDirName);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 return(status);
      }
      _rollback("chlRegColl");
      return(status);
   }
   snprintf(collIdNum, MAX_NAME_LEN, "%lld", status);

   /* Check that the path is not already a dataObj */
   if (logSQL) rodsLog(LOG_SQL, "chlRegColl SQL 2");
   status = cmlGetIntegerValueFromSql(
       "select data_id from R_DATA_MAIN where data_name=? and coll_id=?",
       &iVal, logicalEndName, collIdNum, 0, 0, 0, &icss);

   if (status == 0) {
      return(CAT_NAME_EXISTS_AS_DATAOBJ);
   }


   /* String to get next sequence item for objects */
   cllNextValueString("R_ObjectID", nextStr, MAX_NAME_LEN);

   getNowStr(myTime);

   cllBindVars[cllBindVarCount++]=logicalParentDirName;
   cllBindVars[cllBindVarCount++]=collInfo->collName;
   cllBindVars[cllBindVarCount++]=rsComm->clientUser.userName;
   cllBindVars[cllBindVarCount++]=rsComm->clientUser.rodsZone;
   if (collInfo->collType != NULL) {
      cllBindVars[cllBindVarCount++]=collInfo->collType;
   }
   else {
      cllBindVars[cllBindVarCount++]=""; 
   }
   if (collInfo->collInfo1 != NULL) {
      cllBindVars[cllBindVarCount++]=collInfo->collInfo1;
   }
   else {
      cllBindVars[cllBindVarCount++]="";
   }
   if (collInfo->collInfo2 != NULL) {
      cllBindVars[cllBindVarCount++]=collInfo->collInfo2;
   }
   else {
      cllBindVars[cllBindVarCount++]="";
   }
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=myTime;
   if (logSQL) rodsLog(LOG_SQL, "chlRegColl SQL 3");
   snprintf(tSQL, MAX_SQL_SIZE, 
	    "insert into R_COLL_MAIN (coll_id, parent_coll_name, coll_name, coll_owner_name, coll_owner_zone, coll_type, coll_info1, coll_info2, create_ts, modify_ts) values (%s, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
	    nextStr);
   status =  cmlExecuteNoAnswerSql(tSQL,
				   &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegColl cmlExecuteNoAnswerSql(insert) failure %d",status);
      _rollback("chlRegColl");
      return(status);
   }

   /* String to get current sequence item for objects */
   cllCurrentValueString("R_ObjectID", currStr, MAX_NAME_LEN);
   snprintf(currStr2, MAX_SQL_SIZE, " %s ", currStr);

   cllBindVars[cllBindVarCount++]=rsComm->clientUser.userName,
   cllBindVars[cllBindVarCount++]=rsComm->clientUser.rodsZone,
   cllBindVars[cllBindVarCount++]=ACCESS_OWN;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=myTime;
   snprintf(tSQL, MAX_SQL_SIZE, 
	    "insert into r_objt_access values (%s, (select user_id from r_user_main where user_name=? and zone_name=?), (select token_id from R_TOKN_MAIN where token_namespace = 'access_type' and token_name = ?), ?, ?)",
	    currStr2);
   if (logSQL) rodsLog(LOG_SQL, "chlRegColl SQL 4");
   status =  cmlExecuteNoAnswerSql(tSQL, &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegColl cmlExecuteNoAnswerSql(insert access) failure %d",
	      status);
      _rollback("chlRegColl");
      return(status);
   }

   /* Audit */
   status = cmlAudit4(AU_REGISTER_COLL,  
		      currStr2,
		      "",
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      collInfo->collName,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegColl cmlAudit4 failure %d",
	      status);
      _rollback("chlRegColl");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegColl cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }

   return(status);
}

/* 
 * chlModColl - modify attributes of a collection
 * Input -
 *   rcComm_t *conn - The client connection handle.
 *   collInfo_t *collInfo - generic coll input. Relevant items are:
 *      collName - the collection to be updated, and one or more of:
 *      collType, collInfo1 and/or collInfo2.
 *   We may need a kevValPair_t sometime, but currently not used.
 */
int chlModColl(rsComm_t *rsComm, collInfo_t *collInfo) {
   char myTime[50];
   rodsLong_t status;
   char tSQL[MAX_SQL_SIZE];
   int count;
   rodsLong_t iVal;
   char iValStr[60];

   if (logSQL) rodsLog(LOG_SQL, "chlModColl");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   /* Check that collection exists and user has write permission */
   iVal = cmlCheckDir(collInfo->collName,  rsComm->clientUser.userName,  
		      rsComm->clientUser.rodsZone, 
		      ACCESS_MODIFY_OBJECT, &icss);

   if (iVal < 0) {
      int i;
      char errMsg[105];
      if (iVal==CAT_UNKNOWN_COLLECTION) {
	 snprintf(errMsg, 100, "collection '%s' is unknown", 
	       collInfo->collName);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 return(CAT_UNKNOWN_COLLECTION);
      }
      if (iVal==CAT_NO_ACCESS_PERMISSION) {
	 snprintf(errMsg, 100, "no permission to update collection '%s'", 
		  collInfo->collName);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 return (CAT_NO_ACCESS_PERMISSION);
      }
      return(iVal);
   }

   if (collInfo==0) {
      return(CAT_INVALID_ARGUMENT);
   }

   strncpy(tSQL, "update r_coll_main set ", MAX_SQL_SIZE);
   count=0;
   if (collInfo->collType != NULL && strlen(collInfo->collType)>0) {
      if (strcmp(collInfo->collType,"NULL_SPECIAL_VALUE")==0) {
	 /* A special value to indicate NULL */
	 cllBindVars[cllBindVarCount++]="";
      }
      else {
	 cllBindVars[cllBindVarCount++]=collInfo->collType;
      }
      strncat(tSQL, "coll_type=? ", MAX_SQL_SIZE);
      count++;
   }
   if (collInfo->collInfo1 != NULL && strlen(collInfo->collInfo1)>0) {
      if (strcmp(collInfo->collInfo1,"NULL_SPECIAL_VALUE")==0) {
         /* A special value to indicate NULL */
         cllBindVars[cllBindVarCount++]="";
      } else {
         cllBindVars[cllBindVarCount++]=collInfo->collInfo1;
      }
      if (count>0)  strncat(tSQL, ",", MAX_SQL_SIZE);
      strncat(tSQL, "coll_info1=? ", MAX_SQL_SIZE);
      count++;
   }
   if (collInfo->collInfo2 != NULL && strlen(collInfo->collInfo2)>0) {
      if (strcmp(collInfo->collInfo2,"NULL_SPECIAL_VALUE")==0) {
         /* A special value to indicate NULL */
         cllBindVars[cllBindVarCount++]="";
      } else {
         cllBindVars[cllBindVarCount++]=collInfo->collInfo2;
      }
      if (count>0)  strncat(tSQL, ",", MAX_SQL_SIZE);
      strncat(tSQL, "coll_info2=? ", MAX_SQL_SIZE);
      count++;
   }
   if (count==0) return(CAT_INVALID_ARGUMENT);
   getNowStr(myTime);
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=collInfo->collName;
   strncat(tSQL, ", modify_ts=? where coll_name=?", MAX_SQL_SIZE);

   if (logSQL) rodsLog(LOG_SQL, "chlModColl SQL 1");
   status =  cmlExecuteNoAnswerSql(tSQL,
				   &icss);

   /* Audit */
   snprintf(iValStr, 50, "%lld", iVal);
   status = cmlAudit3(AU_REGISTER_COLL,  
		      iValStr,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      collInfo->collName,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModColl cmlAudit3 failure %d",
	      status);
      return(status);
   }


   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModColl cmlExecuteNoAnswerSQL(update) failure %d", status);
      return(status);
   }
   return(0);
}


/* register a Zone */
int chlRegZone(rsComm_t *rsComm, 
	       char *zoneName, char *zoneType, char *zoneConnInfo, 
	       char *zoneComment) {
   char nextStr[MAX_NAME_LEN];
   char tSQL[MAX_SQL_SIZE];
   int status;
   char myTime[50];

   if (logSQL) rodsLog(LOG_SQL, "chlRegZone");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   if (strncmp(zoneType, "remote", 6) != 0) {
      int i;
      i = addRErrorMsg (&rsComm->rError, 0, 
			"Currently, only zones of type 'remote' are allowed");
      return(CAT_INVALID_ARGUMENT);
   }

   /* String to get next sequence item for objects */
   cllNextValueString("R_ObjectID", nextStr, MAX_NAME_LEN);

   getNowStr(myTime);

   if (logSQL) rodsLog(LOG_SQL, "chlRegZone SQL 1 ");
   cllBindVars[cllBindVarCount++]=zoneName;
   cllBindVars[cllBindVarCount++]=zoneConnInfo;
   cllBindVars[cllBindVarCount++]=zoneComment;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=myTime;

   snprintf(tSQL, MAX_SQL_SIZE, 
	    "insert into R_ZONE_MAIN (zone_id, zone_name, zone_type_name, zone_conn_string, r_comment, create_ts, modify_ts) values (%s, ?, 'remote', ?, ?, ?, ?)",
	    nextStr);
   status =  cmlExecuteNoAnswerSql(tSQL,
				   &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegZone cmlExecuteNoAnswerSql(insert) failure %d",status);
      _rollback("chlRegZone");
      return(status);
   }

   /* Audit */
   status = cmlAudit3(AU_REGISTER_ZONE,  "0",
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      "", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegResc cmlAudit3 failure %d",
	      status);
      return(status);
   }


   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegZone cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }

   return(0);
}


/* Modify a Zone (certain fields) */
int chlModZone(rsComm_t *rsComm, char *zoneName, char *option,
		 char *optionValue) {
   int status, OK;
   char myTime[50];
   char zoneId[MAX_NAME_LEN];
   char commentStr[200];

   if (logSQL) rodsLog(LOG_SQL, "chlModZone");

   if (zoneName == NULL || option==NULL || optionValue==NULL) {
      return (CAT_INVALID_ARGUMENT);
   }

   if (*zoneName == '\0' || *option == '\0' || *optionValue=='\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   status = getLocalZone();
   if (status) return(status);

   zoneId[0]='\0';
   if (logSQL) rodsLog(LOG_SQL, "chlModZone SQL 1 ");
   status = cmlGetStringValueFromSql(
       "select zone_id from r_zone_main where zone_name=?",
       zoneId, MAX_NAME_LEN, zoneName, "", &icss);
   if (status != 0) {
      if (status==CAT_NO_ROWS_FOUND) return(CAT_INVALID_ZONE);
      return(status);
   }

   getNowStr(myTime);
   OK=0;
   if (strcmp(option, "comment")==0) {
      cllBindVars[cllBindVarCount++]=optionValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=zoneId;
      if (logSQL) rodsLog(LOG_SQL, "chlModZone SQL 3");
      status =  cmlExecuteNoAnswerSql(
	       "update r_zone_main set r_comment = ?, modify_ts=? where zone_id=?",
	       &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModZone cmlExecuteNoAnswerSql update failure %d",
		 status);
	 return(status);
      }
      OK=1;
   }
   if (strcmp(option, "conn")==0) {
      cllBindVars[cllBindVarCount++]=optionValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=zoneId;
      if (logSQL) rodsLog(LOG_SQL, "chlModZone SQL 5");
      status =  cmlExecuteNoAnswerSql(
		 "update r_zone_main set zone_conn_string = ?, modify_ts=? where zone_id=?",
		 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModZone cmlExecuteNoAnswerSql update failure %d",
		 status);
	 return(status);
      }
      OK=1;
   }
   if (strcmp(option, "name")==0) {
      if (strcmp(zoneName,localZone)==0) {
	 int i;
	 i = addRErrorMsg (&rsComm->rError, 0, 
			   "It is not valid to rename the local zone via chlModZone; iadmin should use acRenameLocalZone");
	 return (CAT_INVALID_ARGUMENT);
      }
      cllBindVars[cllBindVarCount++]=optionValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=zoneId;
      if (logSQL) rodsLog(LOG_SQL, "chlModZone SQL 5");
      status =  cmlExecuteNoAnswerSql(
		 "update r_zone_main set zone_name = ?, modify_ts=? where zone_id=?",
		 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModZone cmlExecuteNoAnswerSql update failure %d",
		 status);
	 return(status);
      }
      OK=1;
   }
   if (OK==0) {
      return (CAT_INVALID_ARGUMENT);
   }

   /* Audit */
   snprintf(commentStr, 190, "%s %s", option, optionValue);
   status = cmlAudit3(AU_MOD_ZONE,  
		      zoneId,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      commentStr,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModZone cmlAudit3 failure %d",
	      status);
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModZone cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }
   return(0);
}

/* rename a collection */
int chlRenameColl(rsComm_t *rsComm, char *oldCollName, char *newCollName) {
   int status;
   rodsLong_t status1;

   /* See if the input path is a collection and the user owns it,
      and, if so, get the collectionID */
   if (logSQL) rodsLog(LOG_SQL, "chlRenameColl SQL 1 ");

   status1 = cmlCheckDir(oldCollName,
			rsComm->clientUser.userName, 
			rsComm->clientUser.rodsZone, 
			ACCESS_OWN, 
			&icss);

   if (status1 < 0) {
      return(status1);
   }

   /* call chlRenameObject to rename */
   status = chlRenameObject(rsComm, status1, newCollName);
   return(status);
}


/* rename the local zone */
int chlRenameLocalZone(rsComm_t *rsComm, char *oldZoneName, char *newZoneName) {
   int status;
   char zoneId[MAX_NAME_LEN];
   char myTime[50];
   char commentStr[200];

   if (logSQL) rodsLog(LOG_SQL, "chlRenameLocalZone");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   if (logSQL) rodsLog(LOG_SQL, "chlRenameLocalZone SQL 1 ");
   getLocalZone();

   if (strcmp(localZone, oldZoneName) != 0) { /* not the local zone */
      return(CAT_INVALID_ARGUMENT);
   }

   /* check that the new zone does not exist */
   zoneId[0]='\0';
   if (logSQL) rodsLog(LOG_SQL, "chlRenameLocalZone SQL 2 ");
   status = cmlGetStringValueFromSql(
	     "select zone_id from r_zone_main where zone_name=?",
	     zoneId, MAX_NAME_LEN, newZoneName, "", &icss);
   if (status != CAT_NO_ROWS_FOUND) return(CAT_INVALID_ZONE);

   getNowStr(myTime);

   /* Audit */
   /* Do this first, before the userName-zone is made invalid;
      it will be rolledback if an error occurs */

   snprintf(commentStr, 190, "renamed local zone %s to %s",
	    oldZoneName, newZoneName);
   status = cmlAudit3(AU_MOD_ZONE,  
		      "0",
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      commentStr,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRenameLocalZone cmlAudit3 failure %d",
	      status);
      return(status);
   }

   /* update coll_owner_zone in r_coll_main */
   cllBindVars[cllBindVarCount++]=newZoneName;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=oldZoneName;
   if (logSQL) rodsLog(LOG_SQL, "chlRenameLocalZone SQL 3 ");
   status =  cmlExecuteNoAnswerSql(
      "update r_coll_main set coll_owner_zone = ?, modify_ts=? where coll_owner_zone=?",
      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRenameLocalZone cmlExecuteNoAnswerSql update failure %d",
	      status);
      return(status);
   }

   /* update data_owner_zone in r_data_main */
   cllBindVars[cllBindVarCount++]=newZoneName;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=oldZoneName;
   if (logSQL) rodsLog(LOG_SQL, "chlRenameLocalZone SQL 4 ");
   status =  cmlExecuteNoAnswerSql(
      "update r_data_main set data_owner_zone = ?, modify_ts=? where data_owner_zone=?",
      &icss);
   if (status != 0 && status != CAT_SUCCESS_BUT_WITH_NO_INFO) {
      rodsLog(LOG_NOTICE,
	      "chlRenameLocalZone cmlExecuteNoAnswerSql update failure %d",
	      status);
      return(status);
   }

   /* update zone_name in r_resc_main */
   cllBindVars[cllBindVarCount++]=newZoneName;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=oldZoneName;
   if (logSQL) rodsLog(LOG_SQL, "chlRenameLocalZone SQL 5 ");
   status =  cmlExecuteNoAnswerSql(
      "update r_resc_main set zone_name = ?, modify_ts=? where zone_name=?",
      &icss);
   if (status != 0 && status != CAT_SUCCESS_BUT_WITH_NO_INFO) {
      rodsLog(LOG_NOTICE,
	      "chlRenameLocalZone cmlExecuteNoAnswerSql update failure %d",
	      status);
      return(status);
   }

   /* update rule_owner_zone in r_rule_main */
   cllBindVars[cllBindVarCount++]=newZoneName;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=oldZoneName;
   if (logSQL) rodsLog(LOG_SQL, "chlRenameLocalZone SQL 6 ");
   status =  cmlExecuteNoAnswerSql(
      "update r_rule_main set rule_owner_zone=?, modify_ts=? where rule_owner_zone=?",
      &icss);
   if (status != 0 && status != CAT_SUCCESS_BUT_WITH_NO_INFO) {
      rodsLog(LOG_NOTICE,
	      "chlRenameLocalZone cmlExecuteNoAnswerSql update failure %d",
	      status);
      return(status);
   }

   /* update zone_name in r_user_main */
   cllBindVars[cllBindVarCount++]=newZoneName;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=oldZoneName;
   if (logSQL) rodsLog(LOG_SQL, "chlRenameLocalZone SQL 7 ");
   status =  cmlExecuteNoAnswerSql(
      "update r_user_main set zone_name=?, modify_ts=? where zone_name=?",
      &icss);
   if (status != 0 && status != CAT_SUCCESS_BUT_WITH_NO_INFO) {
      rodsLog(LOG_NOTICE,
	      "chlRenameLocalZone cmlExecuteNoAnswerSql update failure %d",
	      status);
      return(status);
   }

   /* update zone_name in r_zone_main */
   cllBindVars[cllBindVarCount++]=newZoneName;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=oldZoneName;
   if (logSQL) rodsLog(LOG_SQL, "chlRenameLocalZone SQL 8 ");
   status =  cmlExecuteNoAnswerSql(
      "update r_zone_main set zone_name=?, modify_ts=? where zone_name=?",
      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRenameLocalZone cmlExecuteNoAnswerSql update failure %d",
	      status);
      return(status);
   }

   return(0);
}

/* delete a Zone */
int chlDelZone(rsComm_t *rsComm, char *zoneName) {
   int status;
   char zoneType[MAX_NAME_LEN];

   if (logSQL) rodsLog(LOG_SQL, "chlDelZone");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   if (logSQL) rodsLog(LOG_SQL, "chlDelZone SQL 1 ");

   status = cmlGetStringValueFromSql(
       "select zone_type_name from r_zone_main where zone_name=?",
       zoneType, MAX_NAME_LEN, zoneName, 0, &icss);
   if (status != 0) {
      if (status==CAT_NO_ROWS_FOUND) return(CAT_INVALID_ZONE);
      return(status);
   }

   if (strcmp(zoneType, "remote") != 0) {
      int i;
      i = addRErrorMsg (&rsComm->rError, 0, 
          "It is not permitted to remove the local zone");
      return(CAT_INVALID_ARGUMENT);
   }

   cllBindVars[cllBindVarCount++]=zoneName;
   if (logSQL) rodsLog(LOG_SQL, "chlModRescGroup SQL 3");
   status =  cmlExecuteNoAnswerSql(
		"delete from r_zone_main where zone_name = ?",
		&icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlDelZone cmlExecuteNoAnswerSql delete failure %d",
	      status);
      return(status);
   }

   /* Audit */
   status = cmlAudit3(AU_DELETE_ZONE,
		      "0",
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      zoneName,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlDelZone cmlAudit3 failure %d",
	      status);
      _rollback("chlDelZone");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlDelZone cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }

   return(0);
}


/* Simple query

   This is used in cases where it is easier to do a straight-forward
   SQL query rather than go thru the generalQuery interface.  This is
   used this in the iadmin.c interface as it was easier for me (Wayne)
   to work in SQL for admin type ops as I'm thinking in terms of
   tables and columns and SQL anyway.

   For improved security, this is available only to admin users and
   the code checks that the input sql is one of the allowed forms.

   input: sql, up to for optional arguments (bind variables),
          and requested format, max text to return (maxOutBuf)
   output: text (outBuf) or error return
   input/output: control: on input if 0 request is starting, 
          returned non-zero if more rows
	  are available (and then it is the statement number);
          on input if positive it is the statement number (+1) that is being
          continued.
   format 1: column-name : column value, and with CR after each column
   format 2: column headings CR, rows and col values with CR

*/
int chlSimpleQuery(rsComm_t *rsComm, char *sql, 
		   char *arg1, char *arg2, char *arg3, char *arg4,
		   int format, int *control,
		   char *outBuf, int maxOutBuf) {
   int stmtNum, status, nCols, i, needToGet, didGet;
   int rowSize;
   int rows;

   int allowedSQLForms=19;
   char *allowedSQL[]={
"select token_name from r_tokn_main where token_namespace = 'token_namespace'",
"select token_name from r_tokn_main where token_namespace = ?",
"select * from r_tokn_main where token_namespace = ? and token_name like ?",
"select resc_name from r_resc_main",
"select * from r_resc_main where resc_name=?",
"select zone_name from r_zone_main",
"select * from r_zone_main where zone_name=?",
"select user_name from r_user_main where user_type_name='rodsgroup'",
"select user_name||'#'||zone_name from r_user_main, r_user_group where r_user_group.user_id=r_user_main.user_id and r_user_group.group_user_id=(select user_id from r_user_main where user_name=?)",
"select * from r_data_main where data_id=?",
"select data_name, data_id, data_repl_num from r_data_main where coll_id =(select coll_id from r_coll_main where coll_name=?)",
"select coll_name from r_coll_main where parent_coll_name=?",
"select * from r_user_main where user_name=?",
"select user_name||'#'||zone_name from r_user_main where user_type_name != 'rodsgroup'",
"select r_resc_group.resc_group_name, r_resc_group.resc_id, resc_name, r_resc_group.create_ts, r_resc_group.modify_ts from r_resc_main, r_resc_group where r_resc_main.resc_id = r_resc_group.resc_id and resc_group_name=?",
"select distinct resc_group_name from r_resc_group",
"select coll_id from r_coll_main where coll_name = ?",
"select * from r_user_main where user_name=? and zone_name=?",
"select user_name from r_user_main where zone_name=? and user_type_name != 'rodsgroup'",
"select zone_name from R_ZONE_MAIN where zone_type_name=?",
""
   };

   if (logSQL) rodsLog(LOG_SQL, "chlSimpleQuery");

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   /* check that the input sql is one of the allowed forms */
   for (i=0;i<allowedSQLForms;i++) {
      if (strcmp(allowedSQL[i], sql)==0) break;
   }
   if (i > allowedSQLForms) return(CAT_INVALID_ARGUMENT);

   /* done with multiple log calls so that each form will be checked
      via checkIcatLog.pl */
   if (i==0 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 1 ");
   if (i==1 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 2");
   if (i==2 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 3");
   if (i==3 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 4");
   if (i==4 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 5");
   if (i==5 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 6");
   if (i==6 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 7");
   if (i==7 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 8");
   if (i==8 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 9");
   if (i==9 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 10");
   if (i==10 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 11");
   if (i==11 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 12");
   if (i==12 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 13");
   if (i==13 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 14");
   if (i==14 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 15");
   if (i==15 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 16");
   if (i==16 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 17");
   if (i==17 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 18");
   if (i==18 && logSQL) rodsLog(LOG_SQL, "chlSimpleQuery SQL 19");

   outBuf[0]='\0';
   needToGet=1;
   didGet=0;
   rowSize=0;
   rows=0;
   if (*control==0) {
      status = cmlGetFirstRowFromSqlBV(sql, arg1, arg2, arg3, arg4,
				       &stmtNum, &icss);
      if (status < 0) {
	  if (status != CAT_NO_ROWS_FOUND) {
	     rodsLog(LOG_NOTICE,
		     "chlSimpleQuery cmlGetFirstRowFromSqlBV failure %d",
		     status);
	  }
	  return(status);
      }
      didGet=1;
      needToGet=0;
      *control = stmtNum+1;   /* control is always > 0 */
   } 
   else {
      stmtNum = *control - 1;
   }

   for (;;) {
      if (needToGet) {
	 status = cmlGetNextRowFromStatement(stmtNum, &icss);
	 if (status == CAT_NO_ROWS_FOUND) {
	    *control = 0;
	    if (didGet) {
	       if (format == 2) {
		  i = strlen(outBuf);
		  outBuf[i-1]='\0';  /* remove the last CR */
	       }
	       return(0);
	    }
	    return(status);
	 }
	 if (status < 0) {
	    rodsLog(LOG_NOTICE,
		    "chlSimpleQuery cmlGetNextRowFromStatement failure %d",
		    status);
	    return(status);
	 }
	 *control = stmtNum+1;   /* control is always > 0 */
	 didGet=1;
      }
      needToGet=1;
      nCols = icss.stmtPtr[stmtNum]->numOfCols;
      if (rows==0 && format==3) {
	 for (i = 0; i < nCols ; i++ ) {
	    rstrcat(outBuf, icss.stmtPtr[stmtNum]->resultColName[i],maxOutBuf);
	    rstrcat(outBuf, " ", maxOutBuf);
	 }
	 rstrcat(outBuf, "\n", maxOutBuf);
      }
      rows++;
      for (i = 0; i < nCols ; i++ ) {
	 if (format==1 || format==3) {
	    if (strlen(icss.stmtPtr[stmtNum]->resultValue[i])==0) {
	       rstrcat(outBuf, "- ", maxOutBuf);
	    }
	    else {
	       rstrcat(outBuf, icss.stmtPtr[stmtNum]->resultValue[i], 
		       maxOutBuf);
	       rstrcat(outBuf, " ", maxOutBuf);
	    }
	 }
	 if (format == 2) {
	    rstrcat(outBuf, icss.stmtPtr[stmtNum]->resultColName[i],maxOutBuf);
	    rstrcat(outBuf, ": ", maxOutBuf);
	    rstrcat(outBuf, icss.stmtPtr[stmtNum]->resultValue[i], maxOutBuf);
	    rstrcat(outBuf, "\n", maxOutBuf);
	 }
      }
      rstrcat(outBuf, "\n", maxOutBuf);
      if (rowSize==0) rowSize=strlen(outBuf);
      if (strlen(outBuf)+rowSize+20 > maxOutBuf) {
	 return(0); /* success so far, but more rows available */
      }
   }
}

/* Delete a Collection by Administrator, */
/* if it is empty. */
int chlDelCollByAdmin(rsComm_t *rsComm, collInfo_t *collInfo) {
   rodsLong_t iVal;
   char logicalEndName[MAX_NAME_LEN];
   char logicalParentDirName[MAX_NAME_LEN];
   int status;

   if (logSQL) rodsLog(LOG_SQL, "chlDelCollByAdmin");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   if (collInfo==0) {
      return(CAT_INVALID_ARGUMENT);
   }

   status = splitPathByKey(collInfo->collName, 
			   logicalParentDirName, logicalEndName, '/');

   if (strlen(logicalParentDirName)==0) {
      strcpy(logicalParentDirName, "/");
      strcpy(logicalEndName, collInfo->collName+1);
   }

   /* check that the collection is empty (both subdirs and files) */
   if (logSQL) rodsLog(LOG_SQL, "chlDelCollByAdmin SQL 1 ");
   status = cmlGetIntegerValueFromSql(
             "select coll_id from r_coll_main where parent_coll_name=? union select coll_id from r_data_main where coll_id=(select coll_id from R_COLL_MAIN where coll_name=?)",
	     &iVal, collInfo->collName, collInfo->collName, 0, 0, 0, &icss);
 
   if (status != CAT_NO_ROWS_FOUND) {
      if (status == 0) {
	 int i;
	 char errMsg[105];
	 snprintf(errMsg, 100, "collection '%s' is not empty",
		  collInfo->collName);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 return(CAT_COLLECTION_NOT_EMPTY);
      }
      _rollback("chlDelCollByAdmin");
      return(status);
   }

   /* remove any access rows */
   cllBindVars[cllBindVarCount++]=collInfo->collName;
   if (logSQL) rodsLog(LOG_SQL, "chlDelCollByAdmin SQL 2");
   status =  cmlExecuteNoAnswerSql(
		   "delete from r_objt_access where object_id=(select coll_id from R_COLL_MAIN where coll_name=?)",
		   &icss);
   if (status) {  /* error, but let it fall thru to below, 
		     probably doesn't exist */
      rodsLog(LOG_NOTICE,
	      "chlDelCollByAdmin delete access failure %d",
	      status);
      _rollback("chlDelCollByAdmin");
   }

   /* Audit (before it's deleted) */
   status = cmlAudit4(AU_DELETE_COLL_BY_ADMIN,  
		      "select coll_id from r_coll_main where coll_name=?",
		      collInfo->collName,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      collInfo->collName,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModColl cmlAudit4 failure %d",
	      status);
      _rollback("chlDelCollByAdmin");
      return(status);
   }


   /* delete the row if it exists */
   cllBindVars[cllBindVarCount++]=collInfo->collName;
   if (logSQL) rodsLog(LOG_SQL, "chlDelCollByAdmin SQL 3");
   status =  cmlExecuteNoAnswerSql("delete from r_coll_main where coll_name=?",
				   &icss);

   if (status) {
      int i;
      char errMsg[105];
      snprintf(errMsg, 100, "collection '%s' is unknown", 
	       collInfo->collName);
      i = addRErrorMsg (&rsComm->rError, 0, errMsg);
      _rollback("chlDelCollByAdmin");
      return(CAT_UNKNOWN_COLLECTION);
   }

   return(0);
}


/* Delete a Collection */
int chlDelColl(rsComm_t *rsComm, collInfo_t *collInfo) {

   int status;

   if (logSQL) rodsLog(LOG_SQL, "chlDelColl");

   status = _delColl(rsComm, collInfo);
   if (status) return(status);

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlDelColl cmlExecuteNoAnswerSql commit failure %d",
	      status);
      _rollback("chlDelColl");
      return(status);
   }
   return(0);
}

/* delCollection (internally called),
   does not do the commit.
*/
int _delColl(rsComm_t *rsComm, collInfo_t *collInfo) {
   rodsLong_t iVal;
   char logicalEndName[MAX_NAME_LEN];
   char logicalParentDirName[MAX_NAME_LEN];
   char collIdNum[MAX_NAME_LEN];
   char parentCollIdNum[MAX_NAME_LEN];
   rodsLong_t status;

   if (logSQL) rodsLog(LOG_SQL, "_delColl");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   status = splitPathByKey(collInfo->collName, 
			   logicalParentDirName, logicalEndName, '/');

   if (strlen(logicalParentDirName)==0) {
      strcpy(logicalParentDirName, "/");
      strcpy(logicalEndName, collInfo->collName+1);
   }

   /* Check that the parent collection exists and user has write permission,
      and get the collectionID */
   if (logSQL) rodsLog(LOG_SQL, "_delColl SQL 1 ");
   status = cmlCheckDir(logicalParentDirName, 
			rsComm->clientUser.userName, 
			rsComm->clientUser.rodsZone, 
			ACCESS_MODIFY_OBJECT, 
			&icss);
   if (status < 0) {
      int i;
      char errMsg[105];
      if (status == CAT_UNKNOWN_COLLECTION) {
	 snprintf(errMsg, 100, "collection '%s' is unknown",
		  logicalParentDirName);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 return(status);
      }
      _rollback("_chlDelColl");
      return(status);
   }
   snprintf(parentCollIdNum, MAX_NAME_LEN, "%lld", status);

   /* Check that the collection exists and user has DELETE or better 
      permission */
   if (logSQL) rodsLog(LOG_SQL, "_delColl SQL 2");
   status = cmlCheckDir(collInfo->collName, 
			rsComm->clientUser.userName, 
			rsComm->clientUser.rodsZone,
			ACCESS_DELETE_OBJECT, 
			&icss);
   if (status < 0) return(status);
   snprintf(collIdNum, MAX_NAME_LEN, "%lld", status);

   /* check that the collection is empty (both subdirs and files) */
   if (logSQL) rodsLog(LOG_SQL, "_delColl SQL 3");
   status = cmlGetIntegerValueFromSql(
       "select coll_id from r_coll_main where parent_coll_name=? union select coll_id from r_data_main where coll_id=(select coll_id from R_COLL_MAIN where coll_name=?)",
       &iVal, collInfo->collName, collInfo->collName, 0, 0, 0, &icss);
   if (status != CAT_NO_ROWS_FOUND) {
      return(CAT_COLLECTION_NOT_EMPTY);
   }

   /* delete the row if it exists and is owned by the user */
   cllBindVars[cllBindVarCount++]=collInfo->collName;
   cllBindVars[cllBindVarCount++]=rsComm->clientUser.userName;
   if (logSQL) rodsLog(LOG_SQL, "_delColl SQL 4");
   status =  cmlExecuteNoAnswerSql(
		   "delete from r_coll_main where coll_name=? and coll_owner_name=?",
		   &icss);
   if (status) {  /* error, odd one as everything checked above */
      rodsLog(LOG_NOTICE,
	      "_delColl cmlExecuteNoAnswerSql delete failure %d",
	      status);
      _rollback("_chlDelColl");
   }

   /* remove any access rows */
   cllBindVars[cllBindVarCount++]=collIdNum;
   if (logSQL) rodsLog(LOG_SQL, "_delColl SQL 5");
   status =  cmlExecuteNoAnswerSql(
		   "delete from r_objt_access where object_id=?",
		   &icss);
   if (status) {  /* error, odd one as everything checked above */
      rodsLog(LOG_NOTICE,
	      "_delColl cmlExecuteNoAnswerSql delete access failure %d",
	      status);
      _rollback("_chlDelColl");
   }


   /* Audit */
   status = cmlAudit3(AU_DELETE_COLL,
		      collIdNum,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      collInfo->collName,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModColl cmlAudit3 failure %d",
	      status);
      _rollback("_chlDelColl");
      return(status);
   }

   return(status);
}

/* Check an authentication response.
 
   Input is the challange, response, and username; the response is checked
   and if OK the userPrivLevel is set.  Temporary-one-time passwords are
   also checked and possibly removed.

   The clientPrivLevel is the privilege level for the client in
   the rsComm structure; this is used by servers when setting the
   authFlag.

   Called from rsAuthCheck.
*/
int chlCheckAuth(rsComm_t *rsComm, char *challenge, char *response,
		 char *username, 
		 int *userPrivLevel, int *clientPrivLevel) {

   int status;
   char md5Buf[CHALLENGE_LEN+MAX_PASSWORD_LEN+2];
   char digest[RESPONSE_LEN+2];
   MD5_CTX context;
   char *cp;
   int i, OK, k;
   char userType[MAX_NAME_LEN];
   static int prevFailure=0;
   char pwInfoArray[MAX_PASSWORD_LEN*MAX_PASSWORDS*3];
   char goodPw[MAX_PASSWORD_LEN+10];
   char goodPwExpiry[MAX_PASSWORD_LEN+10];
   char goodPwTs[MAX_PASSWORD_LEN+10];
   rodsLong_t expireTime;
   char *cpw;
   int nPasswords;
   char myTime[50];
   time_t nowTime;
   time_t pwExpireMaxCreateTime;
   char expireStr[50];
   char expireStrCreate[50];
   char myUserZone[MAX_NAME_LEN];
   char userName2[NAME_LEN+2];
   char userZone[NAME_LEN+2];

   if (logSQL) rodsLog(LOG_SQL, "chlCheckAuth");

   if (prevFailure > 1) {
      /* Somebody trying a dictionary attack? */
      if (prevFailure > 5) sleep(20);  /* at least, slow it down */
      sleep(2);
   }
   *userPrivLevel = NO_USER_AUTH;
   *clientPrivLevel = NO_USER_AUTH;

   status = parseUserName(username, userName2, userZone);
   if (userZone[0]=='\0') {
      status = getLocalZone();
      if (status) return(status);
      strncpy(myUserZone, localZone, MAX_NAME_LEN);
   }
   else {
      strncpy(myUserZone, userZone, MAX_NAME_LEN);
   }


   if (logSQL) rodsLog(LOG_SQL, "chlCheckAuth SQL 1 ");

   status = cmlGetMultiRowStringValuesFromSql(
	    "select rcat_password, pass_expiry_ts, r_user_password.create_ts from r_user_password, r_user_main where user_name=? and zone_name=? and r_user_main.user_id = r_user_password.user_id",
	    pwInfoArray, MAX_PASSWORD_LEN,
	    MAX_PASSWORDS*3,  /* three strings per password returned */
	    userName2, myUserZone, &icss);
   if (status < 3) {
      if (status == CAT_NO_ROWS_FOUND) {
	 status = CAT_INVALID_USER; /* Be a little more specific */
	 if (strncmp(ANONYMOUS_USER, userName2, NAME_LEN)==0) {
	    /* anonymous user, skip the pw check but do the rest */
	    goto checkLevel;
	 }
      } 
      return(status);
   }

   nPasswords=status/3;    /* three strings per password returned */
   goodPwExpiry[0]='\0';
   goodPwTs[0]='\0';

   cpw=pwInfoArray;
   for (k=0;k<MAX_PASSWORDS && k<nPasswords;k++) {
      memset(md5Buf, 0, sizeof(md5Buf));
      strncpy(md5Buf, challenge, CHALLENGE_LEN);
      icatDescramble(cpw);
      strncpy(md5Buf+CHALLENGE_LEN, cpw, MAX_PASSWORD_LEN);

      MD5Init (&context);
      MD5Update (&context, md5Buf, CHALLENGE_LEN+MAX_PASSWORD_LEN);
      MD5Final (digest, &context);

      for (i=0;i<RESPONSE_LEN;i++) {
	 if (digest[i]=='\0') digest[i]++;  /* make sure 'string' doesn't end
				       early (this matches client code) */
      }

      cp = response;
      OK=1;
      for (i=0;i<RESPONSE_LEN;i++) {
	 if (*cp++ != digest[i]) OK=0;
      }

      memset(md5Buf, 0, sizeof(md5Buf));
      if (OK==1) {
	 rstrcpy(goodPw, cpw, MAX_PASSWORD_LEN);
	 cpw+=MAX_PASSWORD_LEN;
	 rstrcpy(goodPwExpiry, cpw, MAX_PASSWORD_LEN);
	 cpw+=MAX_PASSWORD_LEN;
	 rstrcpy(goodPwTs, cpw, MAX_PASSWORD_LEN);
	 break;
      }
      cpw+=MAX_PASSWORD_LEN*3;
   }
   memset(pwInfoArray, 0, sizeof(pwInfoArray));

   if (OK==0) {
      prevFailure++;
      return(CAT_INVALID_AUTHENTICATION);
   }

   expireTime=atoll(goodPwExpiry);
   getNowStr(myTime);
   nowTime=atoll(myTime);
   if (expireTime < 1000) {

      /* in the form used by temporary, one-time passwords */

      time_t createTime;
      int returnExpired;

      /* check if it's expired */

      returnExpired=0;
      getNowStr(myTime);
      nowTime=atoll(myTime);
      createTime=atoll(goodPwTs);
      if (createTime==0 || nowTime==0 ) returnExpired=1;
      if (createTime+expireTime < nowTime) returnExpired=1;


      /* Remove this temporary, one-time password */

      cllBindVars[cllBindVarCount++]=goodPw;
      if (logSQL) rodsLog(LOG_SQL, "chlCheckAuth SQL 2");
      status =  cmlExecuteNoAnswerSql(
	    "delete from r_user_password where rcat_password=?",
	    &icss);
      if (status !=0) {
	 rodsLog(LOG_NOTICE,
		 "chlCheckAuth cmlExecuteNoAnswerSql delete failure %d",
		 status);
	 _rollback("chlCheckAuth");
	 return(status);
      }

      /* Also remove any expired temporary passwords */

      if (logSQL) rodsLog(LOG_SQL, "chlCheckAuth SQL 3");
      snprintf(expireStr, 50, "%d", TEMP_PASSWORD_TIME);
      cllBindVars[cllBindVarCount++]=expireStr; 

      pwExpireMaxCreateTime = nowTime-TEMP_PASSWORD_TIME;
      /* Not sure if casting to int is correct but seems OK & avoids warning:*/
      snprintf(expireStrCreate, 50, "%011d", (int)pwExpireMaxCreateTime); 
      cllBindVars[cllBindVarCount++]=expireStrCreate;

      status =  cmlExecuteNoAnswerSql(
            "delete from r_user_password where pass_expiry_ts = ? and create_ts < ?",
	    &icss);
      if (status != 0 && status != CAT_SUCCESS_BUT_WITH_NO_INFO) {
	 rodsLog(LOG_NOTICE,
		 "chlCheckAuth cmlExecuteNoAnswerSql delete2 failure %d",
		 status);
	 _rollback("chlCheckAuth");
	 return(status);
      }

      memset(goodPw, 0, MAX_PASSWORD_LEN);
      if (returnExpired) return(CAT_PASSWORD_EXPIRED);

      if (logSQL) rodsLog(LOG_SQL, "chlCheckAuth SQL 4");
      status =  cmlExecuteNoAnswerSql("commit", &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlCheckAuth cmlExecuteNoAnswerSql commit failure %d",
		 status);
	 return(status);
      }
      memset(goodPw, 0, MAX_PASSWORD_LEN);
      if (returnExpired) return(CAT_PASSWORD_EXPIRED);
   }

   /* Get the user type so privilege level can be set */
 checkLevel:

   if (logSQL) rodsLog(LOG_SQL, "chlCheckAuth SQL 5");
   status = cmlGetStringValueFromSql(
	    "select user_type_name from r_user_main where user_name=? and zone_name=?",
	    userType, MAX_NAME_LEN, userName2, myUserZone, &icss);
   if (status !=0) {
      if (status == CAT_NO_ROWS_FOUND) {
	 status = CAT_INVALID_USER; /* Be a little more specific */
      }
      else {
	 _rollback("chlCheckAuth");
      }
      return(status);
   }
   *userPrivLevel = LOCAL_USER_AUTH;
   if (strcmp(userType, "rodsadmin") == 0) {
      *userPrivLevel = LOCAL_PRIV_USER_AUTH;

      /* Since the user is admin, also get the client privilege level */
      if (strcmp( rsComm->clientUser.userName, userName2)==0 &&
	  strcmp( rsComm->clientUser.rodsZone, userZone)==0) {
	 *clientPrivLevel = LOCAL_PRIV_USER_AUTH; /* same user, no query req */
      }
      else {
	 if (logSQL) rodsLog(LOG_SQL, "chlCheckAuth SQL 6");
	 status = cmlGetStringValueFromSql(
	       "select user_type_name from r_user_main where user_name=? and zone_name=?",
	       userType, MAX_NAME_LEN, userName2,
	       myUserZone, &icss);

	 if (status !=0) {
	    if (status == CAT_NO_ROWS_FOUND) {
	       status = CAT_INVALID_CLIENT_USER; /* more specific */
	    }
	    else {
	       _rollback("chlCheckAuth");
	    }
	    return(status);
	 }
	 *clientPrivLevel = LOCAL_USER_AUTH;
	 if (strcmp(userType, "rodsadmin") == 0) {
	    *clientPrivLevel = LOCAL_PRIV_USER_AUTH;
	 }
      }
   }

   prevFailure=0;
   return(0);
}

/* Generate a temporary, one-time password.
   Input is the username from the rsComm structure.  
   Output is the pattern, that when hashed with the user's password,
   becomes the temporary password.  The temp password is also stored
   in the database.

   Called from rsGetTempPassword.
*/
int chlMakeTempPw(rsComm_t *rsComm, char *pwValueToHash) {
   int status;
   char md5Buf[100];
   unsigned char digest[RESPONSE_LEN+2];
   MD5_CTX context;
   int i;
   char password[MAX_PASSWORD_LEN+10];
   char newPw[MAX_PASSWORD_LEN+10];
   char myTime[50];
   char myTimeExp[50];
   char rBuf[200];
   char hashValue[50];
   int j=0;
   char tSQL[MAX_SQL_SIZE];

   if (logSQL) rodsLog(LOG_SQL, "chlMakeTempPw");

   if (logSQL) rodsLog(LOG_SQL, "chlMakeTempPw SQL 1 ");

   snprintf(tSQL, MAX_SQL_SIZE, 
            "select rcat_password from r_user_password, r_user_main where user_name=? and r_user_main.user_id = r_user_password.user_id and pass_expiry_ts != '%d'",
	    TEMP_PASSWORD_TIME);

   status = cmlGetStringValueFromSql(tSQL,
	    password, MAX_PASSWORD_LEN, 
	    rsComm->clientUser.userName, 0, &icss);
   if (status !=0) {
      if (status == CAT_NO_ROWS_FOUND) {
	 status = CAT_INVALID_USER; /* Be a little more specific */
      }
      else {
	 _rollback("chlMakeTempPw");
      }
      return(status);
   }

   icatDescramble(password);

   j=0;
   get64RandomBytes(rBuf);
   for (i=0;i<50 && j<MAX_PASSWORD_LEN-1;i++) {
      char c;
      c = rBuf[i] &0x7f;
      if (c < '0') c+='0';
      if ( (c > 'a' && c < 'z') || (c > 'A' && c < 'Z') ||
           (c > '0' && c < '9') ){
         hashValue[j++]=c;
      }
   }
   hashValue[j]='\0';
   printf("hashValue=%s\n", hashValue);

   /* calcuate the temp password (a hash of the user's main pw and
      the hashValue) */
   memset(md5Buf, 0, sizeof(md5Buf));
   strncpy(md5Buf, hashValue, 100);
   strncat(md5Buf, password, 100);

   MD5Init (&context);
   MD5Update (&context, md5Buf, 100);
   MD5Final (digest, &context);

   md5ToStr(digest, newPw);
   printf("newPw=%s\n", newPw);

   rstrcpy(pwValueToHash, hashValue, MAX_PASSWORD_LEN);


   /* Insert the temporay, one-time password */

   getNowStr(myTime);
   sprintf(myTimeExp, "%d", TEMP_PASSWORD_TIME);  /* seconds from create time
                                                     when it will expire */
   cllBindVars[cllBindVarCount++]=rsComm->clientUser.userName;
   cllBindVars[cllBindVarCount++]=rsComm->clientUser.rodsZone,
   cllBindVars[cllBindVarCount++]=newPw;
   cllBindVars[cllBindVarCount++]=myTimeExp;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=myTime;
   if (logSQL) rodsLog(LOG_SQL, "chlMakeTempPw SQL 2");
  status =  cmlExecuteNoAnswerSql(
              "insert into r_user_password (user_id, rcat_password, pass_expiry_ts,  create_ts, modify_ts) values ((select user_id from r_user_main where user_name=? and zone_name=?), ?, ?, ?, ?)",
	      &icss);
   if (status !=0) {
      rodsLog(LOG_NOTICE,
	      "chlMakeTempPw cmlExecuteNoAnswerSql insert failure %d",
	      status);
      _rollback("chlMakeTempPw");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlMakeTempPw cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }

   memset(newPw, 0, MAX_PASSWORD_LEN);
   return(0);
}

/*
 de-scramble a password sent from the client.
 This isn't real encryption, but does obfuscate the pw on the network.
 Called internally, from chlModUser.
 */
int decodePw(rsComm_t *rsComm, char *in, char *out) {
   int status;
   char *cp;
   char password[MAX_PASSWORD_LEN];
   char upassword[MAX_PASSWORD_LEN+10];
   char rand[]=
      "1gCBizHWbwIYyWLo";  /* must match clients */

   if (logSQL) rodsLog(LOG_SQL, "decodePw - SQL 1 ");
   status = cmlGetStringValueFromSql(
	    "select rcat_password from r_user_password, r_user_main where user_name=? and r_user_main.user_id = r_user_password.user_id",
	    password, MAX_PASSWORD_LEN, 
	    rsComm->clientUser.userName, 0, &icss);
   if (status !=0) {
      if (status == CAT_NO_ROWS_FOUND) {
	 status = CAT_INVALID_USER; /* Be a little more specific */
      }
      else {
	 _rollback("decodePw");
      }
      return(status);
   }

   icatDescramble(password);

   obfDecodeByKey(in, password, upassword);
   memset(password, 0, MAX_PASSWORD_LEN);

   cp = strstr(upassword, rand);
   if (cp !=NULL) *cp='\0';
   strcpy(out, upassword);
   memset(upassword, 0, MAX_PASSWORD_LEN);

   return(0);
}


/* Modify an existing user.
   Admin only.
   Called from rsGeneralAdmin which is used by iadmin */
int chlModUser(rsComm_t *rsComm, char *userName, char *option,
		 char *newValue) {
   int status;
   int opType;
   char decoded[MAX_PASSWORD_LEN+20];
   char tSQL[MAX_SQL_SIZE];
   char form1[]="update r_user_main set %s=?, modify_ts=? where user_name=? and zone_name=?";
   char form2[]="update r_user_main set %s=%s, modify_ts=? where user_name=? and zone_name=?";
   char form3[]="update r_user_password set rcat_password=?, modify_ts=? where user_id=?";
   char form4[]="insert into r_user_password (user_id, rcat_password, pass_expiry_ts,  create_ts, modify_ts) values ((select user_id from r_user_main where user_name=? and zone_name=?), ?, ?, ?, ?)";

   char myTime[50];
   rodsLong_t iVal;

   int auditId;
   char auditComment[110];
   char auditUserName[110];
   int userSettingOwnPassword;

   char userName2[NAME_LEN];
   char zoneName[NAME_LEN];

   if (logSQL) rodsLog(LOG_SQL, "chlModUser");

   if (userName == NULL || option == NULL || newValue==NULL) {
      return (CAT_INVALID_ARGUMENT);
   }

   if (*userName == '\0' || *option == '\0' || newValue=='\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   userSettingOwnPassword=0;
   if ( strcmp(option,"password")==0 &&
        strcmp(userName, rsComm->clientUser.userName)==0)  {
      userSettingOwnPassword=1;
   }

   if (userSettingOwnPassword==0) {
      if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
	 return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
      }
      if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
	 return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
      }
   }

   status = getLocalZone();
   if (status) return(status);

   tSQL[0]='\0';
   opType=0;

   getNowStr(myTime);

   auditComment[0]='\0';
   strncpy(auditUserName,userName,100);

   status = parseUserName(userName, userName2, zoneName);
   if (zoneName[0]=='\0') {
      rstrcpy(zoneName, localZone, NAME_LEN);
   }
   if (status) {
      return (CAT_INVALID_ARGUMENT);
   }

#if 0
	/* no longer allow modifying the user's name since it would 
      require moving the home and trash/home collections too */
   if (strcmp(option,"name" )==0 ||
       strcmp(option,"user_name" )==0) {
      snprintf(tSQL, MAX_SQL_SIZE, form1,
	       "user_name");
      cllBindVars[cllBindVarCount++]=newValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=userName2;
      cllBindVars[cllBindVarCount++]=zoneName;
      if (logSQL) rodsLog(LOG_SQL, "chlModUserSQLxx1x");
      auditId = AU_MOD_USER_NAME;
      strncpy(auditComment, userName, 100);
      strncpy(auditUserName,newValue,100);
   }
#endif
   if (strcmp(option,"type")==0 ||
       strcmp(option,"user_type_name")==0) {
      char tsubSQL[MAX_SQL_SIZE];
      snprintf(tsubSQL, MAX_SQL_SIZE, "(select token_name from r_tokn_main where token_namespace='user_type' and token_name=?)");
      cllBindVars[cllBindVarCount++]=newValue;
      snprintf(tSQL, MAX_SQL_SIZE, form2,
	       "user_type_name", tsubSQL);
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=userName2;
      cllBindVars[cllBindVarCount++]=zoneName;
      opType=1;
      if (logSQL) rodsLog(LOG_SQL, "chlModUser SQL 2");
      auditId = AU_MOD_USER_TYPE;
      strncpy(auditComment, newValue, 100);
   }
   if (strcmp(option,"zone")==0 ||
       strcmp(option,"zone_name")==0) {
      snprintf(tSQL, MAX_SQL_SIZE, form1, "zone_name");
      cllBindVars[cllBindVarCount++]=newValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=userName2;
      cllBindVars[cllBindVarCount++]=zoneName;
      if (logSQL) rodsLog(LOG_SQL, "chlModUser SQL 3");
      auditId = AU_MOD_USER_ZONE;
      strncpy(auditComment, newValue, 100);
      strncpy(auditUserName,userName,100);
   }
   if (strcmp(option,"DN")==0 ||
       strcmp(option,"user_distin_name")==0) {
      snprintf(tSQL, MAX_SQL_SIZE, form1,
	       "user_distin_name");
      cllBindVars[cllBindVarCount++]=newValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=userName2;
      cllBindVars[cllBindVarCount++]=zoneName;
      if (logSQL) rodsLog(LOG_SQL, "chlModUser SQL 4");
      auditId = AU_MOD_USER_DN;
      strncpy(auditComment, newValue, 100);
   }
   if (strcmp(option,"info")==0 ||
       strcmp(option,"user_info")==0) {
      snprintf(tSQL, MAX_SQL_SIZE, form1,
	       "user_info");
      cllBindVars[cllBindVarCount++]=newValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=userName2;
      cllBindVars[cllBindVarCount++]=zoneName;
      if (logSQL) rodsLog(LOG_SQL, "chlModUser SQL 5");
      auditId = AU_MOD_USER_INFO;
      strncpy(auditComment, newValue, 100);
   }
   if (strcmp(option,"comment")==0 ||
       strcmp(option,"r_comment")==0) {
      snprintf(tSQL, MAX_SQL_SIZE, form1,
	       "r_comment");
      cllBindVars[cllBindVarCount++]=newValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=userName2;
      cllBindVars[cllBindVarCount++]=zoneName;
      if (logSQL) rodsLog(LOG_SQL, "chlModUser SQL 6");
      auditId = AU_MOD_USER_COMMENT;
      strncpy(auditComment, newValue, 100);
   }
   if (strcmp(option,"password")==0) {
      int i;
      char userIdStr[MAX_NAME_LEN];
      i = decodePw(rsComm, newValue, decoded);

      icatScramble(decoded); 

      if (i) return(i);
      if (logSQL) rodsLog(LOG_SQL, "chlModUser SQL 7");
      i = cmlGetStringValueFromSql(
	       "select r_user_password.user_id from r_user_password, r_user_main where r_user_main.user_name=? and r_user_main.zone_name=? and r_user_main.user_id = r_user_password.user_id",
	       userIdStr, MAX_NAME_LEN, userName2, zoneName, &icss);
      if (i != 0 && i !=CAT_NO_ROWS_FOUND) return(i);
      if (i == 0) {
	 snprintf(tSQL, MAX_SQL_SIZE, form3);
	 cllBindVars[cllBindVarCount++]=decoded;
	 cllBindVars[cllBindVarCount++]=myTime;
	 cllBindVars[cllBindVarCount++]=userIdStr;
	 if (logSQL) rodsLog(LOG_SQL, "chlModUser SQL 8");
      }
      else {
	 opType=4;
	 snprintf(tSQL, MAX_SQL_SIZE, form4);
	 cllBindVars[cllBindVarCount++]=userName2;
	 cllBindVars[cllBindVarCount++]=zoneName;
	 cllBindVars[cllBindVarCount++]=decoded;
	 cllBindVars[cllBindVarCount++]="9999-12-31-23.59.01";
	 cllBindVars[cllBindVarCount++]=myTime;
	 cllBindVars[cllBindVarCount++]=myTime;
	 if (logSQL) rodsLog(LOG_SQL, "chlModUser SQL 9");
      }
      auditId = AU_MOD_USER_PASSWORD;
   }

   if (tSQL[0]=='\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   status =  cmlExecuteNoAnswerSql(tSQL, &icss);
   memset(decoded, 0, MAX_PASSWORD_LEN);

   if (status != 0 ) {  /* error */
      if (opType==1) { /* doing a type change, check if user_type problem */
	 int status2;
	 if (logSQL) rodsLog(LOG_SQL, "chlModUser SQL 10");
	 status2 = cmlGetIntegerValueFromSql(
             "select token_name from r_tokn_main where token_namespace='user_type' and token_name=?", 
	     &iVal, newValue, 0, 0, 0, 0, &icss);
	 if (status2) {
	    char errMsg[105];
	    int i;
	    snprintf(errMsg, 100, "user_type '%s' is not valid", 
		     newValue);
	    i = addRErrorMsg (&rsComm->rError, 0, errMsg);

	    rodsLog(LOG_NOTICE,
		    "chlModUser invalid user_type");
	    return(CAT_INVALID_USER_TYPE);
	 }
      }
      if (opType==4) { /* trying to insert password */
	 /* check if user exists */
	 int status2;
	 if (logSQL) rodsLog(LOG_SQL, "chlModUser SQL 11");
	 status2 = cmlGetIntegerValueFromSql(
           "select user_id from r_user_main where user_name=? and zone_name=?",
	   &iVal, userName2, zoneName, 0, 0, 0, &icss);
	 if (status2) {
	    rodsLog(LOG_NOTICE,
		    "chlModUser invalid user %s zone %s", userName2, zoneName);
	    return(CAT_INVALID_USER);
	 }
      }
      rodsLog(LOG_NOTICE,
	      "chlModUser cmlExecuteNoAnswerSql failure %d",
	      status);
      return(status);
   }

   status = cmlAudit1(auditId, rsComm->clientUser.userName, 
		      localZone, auditUserName, auditComment, &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModUser cmlAudit1 failure %d",
	      status);
      _rollback("chlModUser");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModUser cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }
   return(0);
}

/* Modify an existing group (membership). 
   Groups are also users in the schema, so chlModUser can also 
   modify other group attibutes. */
int chlModGroup(rsComm_t *rsComm, char *groupName, char *option,
		 char *userName, char *userZone) {
   int status, OK;
   char myTime[50];
   char userId[MAX_NAME_LEN];
   char groupId[MAX_NAME_LEN];
   char commentStr[100];
   char zoneToUse[MAX_NAME_LEN];

   char userName2[NAME_LEN];
   char zoneName[NAME_LEN];
      

   if (logSQL) rodsLog(LOG_SQL, "chlModGroup");

   if (groupName == NULL || option == NULL || userName==NULL) {
      return (CAT_INVALID_ARGUMENT);
   }

   if (*groupName == '\0' || *option == '\0' || userName=='\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   status = getLocalZone();
   if (status) return(status);

   strncpy(zoneToUse, localZone, MAX_NAME_LEN);
   if (userZone != NULL && *userZone != '\0') {
      strncpy(zoneToUse, userZone, MAX_NAME_LEN);
   }

   status = parseUserName(userName, userName2, zoneName);
   if (zoneName[0]!='\0') {
      rstrcpy(zoneToUse, zoneName, NAME_LEN);
   }

   userId[0]='\0';
   if (logSQL) rodsLog(LOG_SQL, "chlModGroup SQL 1 ");
   status = cmlGetStringValueFromSql(
            "select user_id from r_user_main where user_name=? and r_user_main.zone_name=? and user_type_name !='rodsgroup'",
	    userId, MAX_NAME_LEN, userName2, zoneToUse, &icss);
   if (status != 0) {
      if (status==CAT_NO_ROWS_FOUND) {
	 return(CAT_INVALID_USER);
      }
      _rollback("chlModGroup");
      return(status);
   }

   groupId[0]='\0';
   if (logSQL) rodsLog(LOG_SQL, "chlModGroup SQL 2");
   status = cmlGetStringValueFromSql(
              "select user_id from r_user_main where user_name=? and r_user_main.zone_name=? and user_type_name='rodsgroup'",
	      groupId, MAX_NAME_LEN, groupName, localZone, &icss);
   if (status != 0) {
      if (status==CAT_NO_ROWS_FOUND) {
	 return(CAT_INVALID_GROUP);
      }
      _rollback("chlModGroup");
      return(status);
   }
   OK=0;
   if (strcmp(option, "remove")==0) {
      if (logSQL) rodsLog(LOG_SQL, "chlModGroup SQL 3");
      cllBindVars[cllBindVarCount++]=groupId;
      cllBindVars[cllBindVarCount++]=userId;
      status =  cmlExecuteNoAnswerSql(
         "delete from r_user_group where group_user_id = ? and user_id = ?",
         &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModGroup cmlExecuteNoAnswerSql delete failure %d",
		 status);
	 _rollback("chlModGroup");
	 return(status);
      }
      OK=1;
   }

   if (strcmp(option, "add")==0) {
      getNowStr(myTime);
      cllBindVars[cllBindVarCount++]=groupId;
      cllBindVars[cllBindVarCount++]=userId;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=myTime;
      if (logSQL) rodsLog(LOG_SQL, "chlModGroup SQL 4");
      status =  cmlExecuteNoAnswerSql(
             "insert into r_user_group (group_user_id, user_id , create_ts, modify_ts) values (?, ?, ?, ?)",
	     &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModGroup cmlExecuteNoAnswerSql delete failure %d",
		 status);
	 _rollback("chlModGroup");
	 return(status);
      }
      OK=1;
   }

   if (OK==0) {
      return (CAT_INVALID_ARGUMENT);
   }

   /* Audit */
   snprintf(commentStr, 90, "%s %s", option, userId);
   status = cmlAudit3(AU_MOD_GROUP,  
		      groupId,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      commentStr,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModGroup cmlAudit3 failure %d",
	      status);
      _rollback("chlModGroup");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModGroup cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }
   return(0);
}

/* Modify a Resource (certain fields) */
int chlModResc(rsComm_t *rsComm, char *rescName, char *option,
		 char *optionValue) {
   int status, OK;
   char myTime[50];
   char rescId[MAX_NAME_LEN];
   char commentStr[200];

   if (logSQL) rodsLog(LOG_SQL, "chlModResc");

   if (rescName == NULL || option==NULL || optionValue==NULL) {
      return (CAT_INVALID_ARGUMENT);
   }

   if (*rescName == '\0' || *option == '\0' || *optionValue=='\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   status = getLocalZone();
   if (status) return(status);

   rescId[0]='\0';
   if (logSQL) rodsLog(LOG_SQL, "chlModResc SQL 1 ");
   status = cmlGetStringValueFromSql(
       "select resc_id from r_resc_main where resc_name=? and zone_name=?",
       rescId, MAX_NAME_LEN, rescName, localZone, &icss);
   if (status != 0) {
      if (status==CAT_NO_ROWS_FOUND) return(CAT_INVALID_RESOURCE);
      _rollback("chlModResc");
      return(status);
   }

   getNowStr(myTime);
   OK=0;
   if (strcmp(option, "info")==0) {
      cllBindVars[cllBindVarCount++]=optionValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=rescId;
      if (logSQL) rodsLog(LOG_SQL, "chlModResc SQL 2");
      status =  cmlExecuteNoAnswerSql(
	           "update r_resc_main set resc_info=?, modify_ts=? where resc_id=?",
		   &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModResc cmlExecuteNoAnswerSql update failure %d",
		 status);
	 _rollback("chlModResc");
	 return(status);
      }
      OK=1;
   }
   if (strcmp(option, "comment")==0) {
      cllBindVars[cllBindVarCount++]=optionValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=rescId;
      if (logSQL) rodsLog(LOG_SQL, "chlModResc SQL 3");
      status =  cmlExecuteNoAnswerSql(
	       "update r_resc_main set r_comment = ?, modify_ts=? where resc_id=?",
	       &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModResc cmlExecuteNoAnswerSql update failure %d",
		 status);
	 _rollback("chlModResc");
	 return(status);
      }
      OK=1;
   }
   if (strcmp(option, "freespace")==0) {
      cllBindVars[cllBindVarCount++]=optionValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=rescId;
      if (logSQL) rodsLog(LOG_SQL, "chlModResc SQL 4");
      status =  cmlExecuteNoAnswerSql(
		 "update r_resc_main set free_space = ?, free_space_ts = ?, modify_ts=? where resc_id=?",
		 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModResc cmlExecuteNoAnswerSql update failure %d",
		 status);
	 _rollback("chlModResc");
	 return(status);
      }
      OK=1;
   }
   if (strcmp(option, "host")==0) {
      cllBindVars[cllBindVarCount++]=optionValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=rescId;
      if (logSQL) rodsLog(LOG_SQL, "chlModResc SQL 5");
      status =  cmlExecuteNoAnswerSql(
		 "update r_resc_main set resc_net = ?, modify_ts=? where resc_id=?",
		 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModResc cmlExecuteNoAnswerSql update failure %d",
		 status);
	 _rollback("chlModResc");
	 return(status);
      }
      OK=1;
   }
   if (strcmp(option, "type")==0) {
      if (logSQL) rodsLog(LOG_SQL, "chlModResc SQL 6");
      status = cmlCheckNameToken("resc_type", optionValue, &icss);
      if (status !=0 ) {
	 int i;
	 char errMsg[105];
	 snprintf(errMsg, 100, "resource_type '%s' is not valid", 
		  optionValue);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 return(CAT_INVALID_RESOURCE_TYPE);
      }

      cllBindVars[cllBindVarCount++]=optionValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=rescId;
      if (logSQL) rodsLog(LOG_SQL, "chlModResc SQL 7");
      status =  cmlExecuteNoAnswerSql(
		 "update r_resc_main set resc_type_name = ?, modify_ts=? where resc_id=?",
		 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModResc cmlExecuteNoAnswerSql update failure %d",
		 status);
	 _rollback("chlModResc");
	 return(status);
      }
      OK=1;
   }

   if (strcmp(option, "class")==0) {
      if (logSQL) rodsLog(LOG_SQL, "chlModResc SQL 8");
      status = cmlCheckNameToken("resc_class", optionValue, &icss);
      if (status !=0 ) {
	 int i;
	 char errMsg[105];
	 snprintf(errMsg, 100, "resource_class '%s' is not valid", 
		  optionValue);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 return(CAT_INVALID_RESOURCE_CLASS);
      }

      cllBindVars[cllBindVarCount++]=optionValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=rescId;
      if (logSQL) rodsLog(LOG_SQL, "chlModResc SQL 9");
      status =  cmlExecuteNoAnswerSql(
		 "update r_resc_main set resc_class_name = ?, modify_ts=? where resc_id=?",
		 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModResc cmlExecuteNoAnswerSql update failure %d",
		 status);
	 _rollback("chlModResc");
	 return(status);
      }
      OK=1;
   }

   if (strcmp(option, "path")==0) {
      if (logSQL) rodsLog(LOG_SQL, "chlModResc SQL 10");
      cllBindVars[cllBindVarCount++]=optionValue;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=rescId;
      status =  cmlExecuteNoAnswerSql(
		 "update r_resc_main set resc_def_path=?, modify_ts=? where resc_id=?",
		 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModResc cmlExecuteNoAnswerSql update failure %d",
		 status);
	 _rollback("chlModResc");
	 return(status);
      }
      OK=1;
   }

   if (OK==0) {
      return (CAT_INVALID_ARGUMENT);
   }

   /* Audit */
   snprintf(commentStr, 190, "%s %s", option, optionValue);
   status = cmlAudit3(AU_MOD_RESC,  
		      rescId,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      commentStr,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModResc cmlAudit3 failure %d",
	      status);
      _rollback("chlModResc");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModResc cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }
   return(0);
}

/* Add or substract to the resource free_space */
int chlModRescFreeSpace(rsComm_t *rsComm, char *rescName, int updateValue) {
   int status;
   char myTime[50];
   char updateValueStr[MAX_NAME_LEN];

   if (logSQL) rodsLog(LOG_SQL, "chlModRescFreeSpace");

   if (rescName == NULL) {
      return (CAT_INVALID_ARGUMENT);
   }

   if (*rescName == '\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   /* The following checks may not be needed long term, but
      shouldn't hurt, for now.
    */

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   status = getLocalZone();
   if (status) return(status);

   getNowStr(myTime);

   snprintf(updateValueStr,MAX_NAME_LEN, "%d", updateValue);

   cllBindVars[cllBindVarCount++]=updateValueStr;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=rescName;
	       
   if (logSQL) rodsLog(LOG_SQL, "chlModRescFreeSpace SQL 1 ");
   status =  cmlExecuteNoAnswerSql(
               "update r_resc_main set free_space = ?, free_space_ts=? where resc_name=?",
	       &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModRescFreeSpace cmlExecuteNoAnswerSql update failure %d",
	      status);
      _rollback("chlModRescFreeSpace");
      return(status);
   }

   /* Audit */
   status = cmlAudit4(AU_MOD_RESC_FREE_SPACE,  
		      "select resc_id from r_resc_main where resc_name=?",
		      rescName,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      updateValueStr,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModRescFreeSpace cmlAudit4 failure %d",
	      status);
      _rollback("chlModRescFreeSpace");
      return(status);
   }

   return(0);
}

/* Add or remove a resource to/from a Resource Group */
int chlModRescGroup(rsComm_t *rsComm, char *rescGroupName, char *option,
		 char *rescName) {
   int status, OK;
   char myTime[50];
   char rescId[MAX_NAME_LEN];
   char commentStr[200];

   if (logSQL) rodsLog(LOG_SQL, "chlModRescGroup");

   if (rescGroupName == NULL || option==NULL || rescName==NULL) {
      return (CAT_INVALID_ARGUMENT);
   }

   if (*rescGroupName == '\0' || *option == '\0' || *rescName=='\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   status = getLocalZone();
   if (status) return(status);

   rescId[0]='\0';
   if (logSQL) rodsLog(LOG_SQL, "chlModRescGroup SQL 1 ");
   status = cmlGetStringValueFromSql(
	      "select resc_id from r_resc_main where resc_name=? and zone_name=?",
	      rescId, MAX_NAME_LEN, rescName, localZone, &icss);
   if (status != 0) {
      if (status==CAT_NO_ROWS_FOUND) return(CAT_INVALID_RESOURCE);
      _rollback("chlModRescGroup");
      return(status);
   }

   getNowStr(myTime);
   OK=0;
   if (strcmp(option, "add")==0) {
      cllBindVars[cllBindVarCount++]=rescGroupName;
      cllBindVars[cllBindVarCount++]=rescId;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=myTime;
      if (logSQL) rodsLog(LOG_SQL, "chlModRescGroup SQL 2");
      status =  cmlExecuteNoAnswerSql(
	       "insert into r_resc_group (resc_group_name, resc_id , create_ts, modify_ts) values (?, ?, ?, ?)",
	       &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModRescGroup cmlExecuteNoAnswerSql insert failure %d",
		 status);
	 _rollback("chlModRescGroup");
	 return(status);
      }
      OK=1;
   }

   if (strcmp(option, "remove")==0) {
      cllBindVars[cllBindVarCount++]=rescGroupName;
      cllBindVars[cllBindVarCount++]=rescId;
      if (logSQL) rodsLog(LOG_SQL, "chlModRescGroup SQL 3");
      status =  cmlExecuteNoAnswerSql(
         "delete from r_resc_group where resc_group_name=? and resc_id=?",
	 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModRescGroup cmlExecuteNoAnswerSql delete failure %d",
		 status);
	 _rollback("chlModRescGroup");
	 return(status);
      }
      OK=1;
   }

   if (OK==0) {
      return (CAT_INVALID_ARGUMENT);
   }

   /* Audit */
   snprintf(commentStr, 190, "%s %s", option, rescGroupName);
   status = cmlAudit3(AU_MOD_RESC_GROUP,  
		      rescId,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      commentStr,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModRescGroup cmlAudit3 failure %d",
	      status);
      _rollback("chlModRescGroup");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModRescGroup cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }
   return(0);
}



/* Register a User, RuleEngine version */
int chlRegUserRE(rsComm_t *rsComm, userInfo_t *userInfo) {
   char myTime[50];
   int status;
   char seqStr[MAX_NAME_LEN];
   char auditSQL[MAX_SQL_SIZE];
   char userZone[MAX_NAME_LEN];
   char zoneId[MAX_NAME_LEN];

   int zoneForm;
   char userName2[NAME_LEN];
   char zoneName[NAME_LEN];

   static char lastValidUserType[MAX_NAME_LEN]="";
   static char userTypeTokenName[MAX_NAME_LEN]="";

   if (logSQL) rodsLog(LOG_SQL, "chlRegUserRE");

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }
   if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (userInfo==0) {
      return(CAT_INVALID_ARGUMENT);
   }

   if (userInfo->userType==0) {
      return(CAT_INVALID_ARGUMENT);
   }

   /*
     Check if the user type is valid.
     This check is skipped if this process has already verified this type
     (iadmin doing a series of mkuser subcommands).
    */
   if ( *userInfo->userType=='\0' || 
        strcmp(userInfo->userType, lastValidUserType)!=0 ) {
      char errMsg[105];
      int i;
      if (logSQL) rodsLog(LOG_SQL, "chlRegUserRE SQL 1 ");
      status = cmlGetStringValueFromSql(
                "select token_name from r_tokn_main where token_namespace='user_type' and token_name=?", 
		userTypeTokenName, MAX_NAME_LEN, userInfo->userType, 0, &icss);
      if (status==0) {
	 strncpy(lastValidUserType, userInfo->userType, MAX_NAME_LEN);
      }
      else {
	 snprintf(errMsg, 100, "user_type '%s' is not valid", 
		  userInfo->userType);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 return(CAT_INVALID_USER_TYPE);
      }
   }

   status = getLocalZone();
   if (status) return(status);

   if (strlen(userInfo->rodsZone)>0) {
      zoneForm=1;
      strncpy(userZone, userInfo->rodsZone, MAX_NAME_LEN);
   }
   else {
      zoneForm=0;
      strncpy(userZone, localZone, MAX_NAME_LEN);
   }

   status = parseUserName(userInfo->userName, userName2, zoneName);
   if (zoneName[0]!='\0') {
      rstrcpy(userZone, zoneName, NAME_LEN);
      zoneForm=2;
   }
   if (status) {
      return (CAT_INVALID_ARGUMENT);
   }

   if (zoneForm) {
      /* check that the zone exists (if not defaulting to local) */
      zoneId[0]='\0';
      if (logSQL) rodsLog(LOG_SQL, "chlRegUserRE SQL 5 ");
      status = cmlGetStringValueFromSql(
		"select zone_id from r_zone_main where zone_name=?",
		zoneId, MAX_NAME_LEN, userZone, "", &icss);
      if (status != 0) {
	 if (status==CAT_NO_ROWS_FOUND) {
	    int i;
	    char errMsg[105];
	    snprintf(errMsg, 100, 
		  "zone '%s' does not exist",
		  userZone);
	    i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	    return(CAT_INVALID_ZONE);
	 }
	 return(status);
      }
   }

   if (logSQL) rodsLog(LOG_SQL, "chlRegUserRE SQL 2");
   status = cmlGetNextSeqStr(seqStr, MAX_NAME_LEN, &icss);
   if (status) {
      rodsLog(LOG_NOTICE, "chlRegUserRE cmlGetNextSeqStr failure %d",
	      status);
      return(status);
   }

   getNowStr(myTime);

   cllBindVars[cllBindVarCount++]=seqStr;
   cllBindVars[cllBindVarCount++]=userName2;
   cllBindVars[cllBindVarCount++]=userTypeTokenName;
   cllBindVars[cllBindVarCount++]=userZone;
   cllBindVars[cllBindVarCount++]=userInfo->authInfo.authStr;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=myTime;

   if (logSQL) rodsLog(LOG_SQL, "chlRegUserRE SQL 3");
   status =  cmlExecuteNoAnswerSql(
             "insert into r_user_main (user_id, user_name, user_type_name, zone_name, user_distin_name, create_ts, modify_ts) values (?, ?, ?, ?, ?, ?, ?)",
	     &icss);

   if (status) {
      if (status == CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME) {
	 char errMsg[105];
	 int i;
	 snprintf(errMsg, 100, "Error %d %s",
		  status,
		  "CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME"
		  );
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
      }
      _rollback("chlRegUserRE");
      rodsLog(LOG_NOTICE,
	      "chlRegUserRE insert failure %d",status);
      return(status);
   }


   cllBindVars[cllBindVarCount++]=seqStr;
   cllBindVars[cllBindVarCount++]=seqStr;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=myTime;

   if (logSQL) rodsLog(LOG_SQL, "chlRegUserRE SQL 4");
   status =  cmlExecuteNoAnswerSql(
             "insert into r_user_group (group_user_id, user_id, create_ts, modify_ts) values (?, ?, ?, ?)",
	     &icss);
   if (status) {
      rodsLog(LOG_NOTICE,
	      "chlRegUserRE insert into r_user_group failure %d",status);
      _rollback("chlRegUserRE");
      return(status);
   }

   /* Audit */
   snprintf(auditSQL, MAX_SQL_SIZE-1,
	    "select user_id from r_user_main where user_name=? and zone_name='%s'",
	    userZone);
   status = cmlAudit4(AU_REGISTER_USER_RE,  
		      auditSQL,
		      userName2,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      userZone,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlRegUserRE cmlAudit4 failure %d",
	      status);
      _rollback("chlRegUserRE");
      return(status);
   }


   return(status);
}

/*
Check object - get an object's ID and check that the user has access.
Called internally.
*/
rodsLong_t checkAndGetObjectId(rsComm_t *rsComm, char *type,
		    char *name, char *access) {
   int itype;
   char logicalEndName[MAX_NAME_LEN];
   char logicalParentDirName[MAX_NAME_LEN];
   rodsLong_t status;
   rodsLong_t objId;
   char userName[NAME_LEN];
   char userZone[NAME_LEN];


   if (logSQL) rodsLog(LOG_SQL, "checkAndGetObjectId");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (type == NULL || *type=='\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   if (name == NULL || *name=='\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   itype=0;
   if (strcmp(type, "-d") == 0) itype=1; /* dataObj */
   if (strcmp(type, "-c") == 0) itype=2; /* collection */
   if (strcmp(type, "-r") == 0) itype=3; /* resource */
   if (strcmp(type, "-u") == 0) itype=4; /* user */
   if (itype==0) return(CAT_INVALID_ARGUMENT);

   if (itype==1) {
      status = splitPathByKey(name,
			   logicalParentDirName, logicalEndName, '/');
      if (strlen(logicalParentDirName)==0) {
	 strcpy(logicalParentDirName, "/");
	 strcpy(logicalEndName, name);
      }
      if (logSQL) rodsLog(LOG_SQL, "checkAndGetObjectId SQL 1 ");
      status = cmlCheckDataObjOnly(logicalParentDirName, logicalEndName,
				   rsComm->clientUser.userName, 
				   rsComm->clientUser.rodsZone, 
				   access, &icss);
      if (status < 0) {
	 _rollback("checkAndGetObjectId");
	 return(status);
      }
      objId=status;
   }

   if (itype==2) {
   /* Check that the collection exists and user has create_metadata permission,
      and get the collectionID */
      if (logSQL) rodsLog(LOG_SQL, "checkAndGetObjectId SQL 2");
      status = cmlCheckDir(name,
			   rsComm->clientUser.userName, 
			   rsComm->clientUser.rodsZone,
			   access, &icss); 
      if (status < 0) {
	 int i;
	 char errMsg[105];
	 if (status == CAT_UNKNOWN_COLLECTION) {
	    snprintf(errMsg, 100, "collection '%s' is unknown",
		     name);
	    i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 }
	 return(status);
      }
      objId=status;
   }

   if (itype==3) {
      if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
	 return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
      }

      status = getLocalZone();
      if (status) return(status);

      objId=0;
      if (logSQL) rodsLog(LOG_SQL, "checkAndGetObjectId SQL 3");
      status = cmlGetIntegerValueFromSql(
                   "select resc_id from r_resc_main where resc_name=? and zone_name=?",
		   &objId, name, localZone, 0, 0, 0, &icss);
      if (status != 0) {
	 if (status==CAT_NO_ROWS_FOUND) return(CAT_INVALID_RESOURCE);
	 _rollback("checkAndGetObjectId");
	 return(status);
      }
   }

   if (itype==4) {
      if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
	 return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
      }

      status = parseUserName(name, userName, userZone);
      if (userZone[0]=='\0') {
	 status = getLocalZone();
	 if (status) return(status);
	 strncpy(userZone, localZone, NAME_LEN);
      }

      objId=0;
      if (logSQL) rodsLog(LOG_SQL, "checkAndGetObjectId SQL 4");
      status = cmlGetIntegerValueFromSql(
         "select user_id from r_user_main where user_name=? and zone_name=?",
	 &objId, userName, userZone, 0, 0, 0, &icss);
      if (status != 0) {
	 if (status==CAT_NO_ROWS_FOUND) return(CAT_INVALID_USER);
	 _rollback("checkAndGetObjectId");
	 return(status);
      }
   }
   return(objId);
}



/* Add an Attribute-Value [Units] pair/triple metadata item to an object */
int chlAddAVUMetadata(rsComm_t *rsComm, int adminMode, char *type, 
		  char *name, char *attribute, char *value,  char *units) {
   int itype;
   char myTime[50];
   char logicalEndName[MAX_NAME_LEN];
   char logicalParentDirName[MAX_NAME_LEN];
   rodsLong_t seqNum, iVal;
   char nextStr[MAX_NAME_LEN];
   rodsLong_t objId, status;
   char objIdStr[MAX_NAME_LEN];
   char seqNumStr[MAX_NAME_LEN];
   char userName[NAME_LEN];
   char userZone[NAME_LEN];

   if (logSQL) rodsLog(LOG_SQL, "chlAddAVUMetadata");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (type == NULL || *type=='\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   if (name == NULL || *name=='\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   if (attribute == NULL || *attribute=='\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   if (value == NULL || *value=='\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   if (adminMode==1) {
      if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
	 return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
      }
   }

   if (units == NULL) units="";

   itype=0;
   if (strcmp(type, "-d") == 0) itype=1; /* dataObj */
   if (strcmp(type, "-c") == 0) itype=2; /* collection */
   if (strcmp(type, "-r") == 0) itype=3; /* resource */
   if (strcmp(type, "-u") == 0) itype=4; /* user */
   if (itype==0) return(CAT_INVALID_ARGUMENT);


   if (itype==1) {
      status = splitPathByKey(name,
			   logicalParentDirName, logicalEndName, '/');
      if (strlen(logicalParentDirName)==0) {
	 strcpy(logicalParentDirName, "/");
	 strcpy(logicalEndName, name);
      }
      if (adminMode==1) {
	 if (logSQL) rodsLog(LOG_SQL, "chlAddAVUMetadata SQL 1 ");
	 status = cmlGetIntegerValueFromSql(
	       "select data_id from r_data_main DM, r_coll_main CM where DM.data_name=? and DM.coll_id=CM.coll_id and CM.coll_name=?",
	       &iVal, logicalEndName, logicalParentDirName, 0, 0, 0, &icss);
	 if (status==0) status=iVal; /*like cmlCheckDataObjOnly, status is objid */
      }
      else {
	 if (logSQL) rodsLog(LOG_SQL, "chlAddAVUMetadata SQL 2");
	 status = cmlCheckDataObjOnly(logicalParentDirName, logicalEndName,
				   rsComm->clientUser.userName, 
				   rsComm->clientUser.rodsZone, 
				   ACCESS_CREATE_METADATA, &icss);
      }
      if (status < 0) {
	 _rollback("chlAddAVUMetadata");
	 return(status);
      }
      objId=status;
   }

   if (itype==2) {
      if (adminMode==1) {
	 if (logSQL) rodsLog(LOG_SQL, "chlAddAVUMetadata SQL 3");
	 status = cmlGetIntegerValueFromSql(
            "select coll_id from R_COLL_MAIN where coll_name=?",
            &iVal, name, 0, 0, 0, 0, &icss);
	 if (status==0) status=iVal;/*like cmlCheckDir, status is objid*/
      }
      else {
	 /* Check that the collection exists and user has create_metadata 
	    permission, and get the collectionID */
	 if (logSQL) rodsLog(LOG_SQL, "chlAddAVUMetadata SQL 4");
	 status = cmlCheckDir(name,
			   rsComm->clientUser.userName, 
			   rsComm->clientUser.rodsZone,
			   ACCESS_CREATE_METADATA, &icss);
      }
      if (status < 0) {
	 int i;
	 char errMsg[105];
	 _rollback("chlAddAVUMetadata");
	 if (status == CAT_UNKNOWN_COLLECTION) {
	    snprintf(errMsg, 100, "collection '%s' is unknown",
		     name);
	    i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 } else {
	    _rollback("chlAddAVUMetadata");
	 }
	 return(status);
      }
      objId=status;
   }

   if (itype==3) {
      if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
	 return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
      }

      status = getLocalZone();
      if (status) return(status);

      objId=0;
      if (logSQL) rodsLog(LOG_SQL, "chlAddAVUMetadata SQL 5");
      status = cmlGetIntegerValueFromSql(
		 "select resc_id from r_resc_main where resc_name=? and zone_name=?",
		 &objId, name, localZone, 0, 0, 0, &icss);
      if (status != 0) {
	 _rollback("chlAddAVUMetadata");
	 if (status==CAT_NO_ROWS_FOUND) return(CAT_INVALID_RESOURCE);
	 return(status);
      }
   }

   if (itype==4) {
      if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
	 return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
      }

      status = parseUserName(name, userName, userZone);
      if (userZone[0]=='\0') {
	 status = getLocalZone();
	 if (status) return(status);
	 strncpy(userZone, localZone, NAME_LEN);
      }

      objId=0;
      if (logSQL) rodsLog(LOG_SQL, "chlAddAVUMetadata SQL 6");
      status = cmlGetIntegerValueFromSql(
              "select user_id from r_user_main where user_name=? and zone_name=?",
	      &objId, userName, userZone, 0, 0, 0, &icss);
      if (status != 0) {
	 _rollback("chlAddAVUMetadata");
	 if (status==CAT_NO_ROWS_FOUND) return(CAT_INVALID_USER);
	 return(status);
      }
   }

   iVal=0;
   if (*units!='\0') {
      if (logSQL) rodsLog(LOG_SQL, "chlAddAVUMetadata SQL 7");
      status = cmlGetIntegerValueFromSql(
            "select meta_id from r_meta_main where meta_attr_name=? and meta_attr_value=? and meta_attr_unit=?",
	    &iVal, attribute, value, units, 0, 0, &icss);
   }
   else {
      if (logSQL) rodsLog(LOG_SQL, "chlAddAVUMetadata SQL 8");
      status = cmlGetIntegerValueFromSql(
         "select meta_id from r_meta_main where meta_attr_name=? and meta_attr_value=? and meta_attr_unit IS NULL",
         &iVal, attribute, value, 0, 0, 0, &icss);
   }
   if (status == 0) {
      seqNum = iVal; /* use existing r_meta_main row */
   }

   else {
      if (logSQL) rodsLog(LOG_SQL, "chlAddAVUMetadata SQL 9");
      seqNum = cmlGetNextSeqVal(&icss);
      if (seqNum < 0) {
	 rodsLog(LOG_NOTICE, "chlAddAVUMetadata cmlGetNextSeqVal failure %d",
		 seqNum);
	 return(seqNum);
      }

      snprintf(nextStr, MAX_SQL_SIZE, "%lld", seqNum);

      getNowStr(myTime);

      cllBindVars[cllBindVarCount++]=nextStr;
      cllBindVars[cllBindVarCount++]=attribute;
      cllBindVars[cllBindVarCount++]=value;
      cllBindVars[cllBindVarCount++]=units;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=myTime;

      if (logSQL) rodsLog(LOG_SQL, "chlAddAVUMetadata SQL 10");
      status =  cmlExecuteNoAnswerSql(
             "insert into r_meta_main (meta_id, meta_attr_name, meta_attr_value, meta_attr_unit, create_ts, modify_ts) values (?, ?, ?, ?, ?, ?)",
	     &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlAddAVUMetadata cmlExecuteNoAnswerSql (insert) failure %d",
		 status);
	 _rollback("chlAddAVUMetadata");
	 return(status);
      }
   }

   getNowStr(myTime);

   snprintf(objIdStr, MAX_SQL_SIZE, "%lld", objId);
   snprintf(seqNumStr, MAX_SQL_SIZE, "%lld", seqNum);
   cllBindVars[cllBindVarCount++]=objIdStr;
   cllBindVars[cllBindVarCount++]=seqNumStr;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=myTime;

   if (logSQL) rodsLog(LOG_SQL, "chlAddAVUMetadata SQL 11");
   status =  cmlExecuteNoAnswerSql(
                 "insert into r_objt_metamap (object_id, meta_id, create_ts, modify_ts) values (?, ?, ?, ?)",
		 &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlAddAVUMetadata cmlExecuteNoAnswerSql insert failure %d",
	      status);
      _rollback("chlAddAVUMetadata");
      return(status);
   }

   /* Audit */
   status = cmlAudit3(AU_ADD_AVU_METADATA,  
		      objIdStr,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      type,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlAddAVUMetadata cmlAudit3 failure %d",
	      status);
      _rollback("chlAddAVUMetadata");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlAddAVUMetadata cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }

   return(status);
}

/* Delete an Attribute-Value [Units] pair/triple metadata item from an object*/
/* option is 0: normal, 1: use wildcards, 2: input is id not type,name,units */
int chlDeleteAVUMetadata(rsComm_t *rsComm, int option, char *type, 
		  char *name, char *attribute, char *value,  char *units) {
   int itype;
   char logicalEndName[MAX_NAME_LEN];
   char logicalParentDirName[MAX_NAME_LEN];
   rodsLong_t status;
   rodsLong_t objId;
   char objIdStr[MAX_NAME_LEN];
   int allowNullUnits;
   char userName[NAME_LEN];
   char userZone[NAME_LEN];

   if (logSQL) rodsLog(LOG_SQL, "chlDeleteAVUMetadata");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (type == NULL || *type=='\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   if (name == NULL || *name=='\0') {
      return (CAT_INVALID_ARGUMENT);
   }

   if (option != 2) {
      if (attribute == NULL || *attribute=='\0') {
	 return (CAT_INVALID_ARGUMENT);
      }

      if (value == NULL || *value=='\0') {
	 return (CAT_INVALID_ARGUMENT);
      }
   }

   if (units == NULL) units="";

   itype=0;
   if (strcmp(type, "-d") == 0) itype=1; /* dataObj */
   if (strcmp(type, "-c") == 0) itype=2; /* collection */
   if (strcmp(type, "-r") == 0) itype=3; /* resource */
   if (strcmp(type, "-u") == 0) itype=4; /* user */
   if (itype==0) return(CAT_INVALID_ARGUMENT);

   if (itype==1) {
      status = splitPathByKey(name,
			   logicalParentDirName, logicalEndName, '/');
      if (strlen(logicalParentDirName)==0) {
	 strcpy(logicalParentDirName, "/");
	 strcpy(logicalEndName, name);
      }
      if (logSQL) rodsLog(LOG_SQL, "chlDeleteAVUMetadata SQL 1 ");
      status = cmlCheckDataObjOnly(logicalParentDirName, logicalEndName,
				   rsComm->clientUser.userName, 
				   rsComm->clientUser.rodsZone, 
				   ACCESS_DELETE_METADATA, &icss);
      if (status < 0) {
	 _rollback("chlDeleteAVUMetadata");
	 return(status);
      }
      objId=status;
   }

   if (itype==2) {
   /* Check that the collection exists and user has delete_metadata permission,
      and get the collectionID */
      if (logSQL) rodsLog(LOG_SQL, "chlDeleteAVUMetadata SQL 2");
      status = cmlCheckDir(name,
			   rsComm->clientUser.userName, 
			   rsComm->clientUser.rodsZone,
			   ACCESS_DELETE_METADATA, &icss);
      if (status < 0) {
	 int i;
	 char errMsg[105];
	 if (status == CAT_UNKNOWN_COLLECTION) {
	    snprintf(errMsg, 100, "collection '%s' is unknown",
		     name);
	    i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 }
	 return(status);
      }
      objId=status;
   }

   if (itype==3) {
      if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
	 return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
      }

      status = getLocalZone();
      if (status) return(status);

      objId=0;
      if (logSQL) rodsLog(LOG_SQL, "chlDeleteAVUMetadata SQL 3");
      status = cmlGetIntegerValueFromSql(
                "select resc_id from r_resc_main where resc_name=? and zone_name=?",
		&objId, name, localZone, 0, 0, 0, &icss);
      if (status != 0) {
	 if (status==CAT_NO_ROWS_FOUND) return(CAT_INVALID_RESOURCE);
	 _rollback("chlDeleteAVUMetadata");
	 return(status);
      }
   }

   if (itype==4) {
      if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
	 return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
      }

      status = parseUserName(name, userName, userZone);
      if (userZone[0]=='\0') {
	 status = getLocalZone();
	 if (status) return(status);
	 strncpy(userZone, localZone, NAME_LEN);
      }

      objId=0;
      if (logSQL) rodsLog(LOG_SQL, "chlDeleteAVUMetadata SQL 4");
      status = cmlGetIntegerValueFromSql(
                 "select user_id from r_user_main where user_name=? and zone_name=?",
		 &objId, userName, userZone, 0, 0, 0, &icss);
      if (status != 0) {
	 if (status==CAT_NO_ROWS_FOUND) return(CAT_INVALID_USER);
	 _rollback("chlDeleteAVUMetadata");
	 return(status);
      }
   }

   snprintf(objIdStr, MAX_NAME_LEN, "%lld", objId);

   if (option==2) {
      cllBindVars[cllBindVarCount++]=objIdStr;
      cllBindVars[cllBindVarCount++]=attribute; /* attribute is really id */

      if (logSQL) rodsLog(LOG_SQL, "chlDeleteAVUMetadata SQL 9");
      status =  cmlExecuteNoAnswerSql(
	      "delete from r_objt_metamap where object_id=? and meta_id =?",
		&icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
	       "chlDeleteAVUMetadata cmlExecuteNoAnswerSql delete failure %d",
		 status);
	 _rollback("chlDeleteAVUMetadata");
	 return(status);
      }

      /* Audit */
      status = cmlAudit3(AU_DELETE_AVU_METADATA,  
			 objIdStr,
			 rsComm->clientUser.userName,
			 rsComm->clientUser.rodsZone,
			 type,
			 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlDeleteAVUMetadata cmlAudit3 failure %d",
		 status);
	 _rollback("chlDeleteAVUMetadata");
	 return(status);
      }

      status =  cmlExecuteNoAnswerSql("commit", &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		"chlDeleteAVUMetadata cmlExecuteNoAnswerSql commit failure %d",
		 status);
	 return(status);
      }
      return(status);
   }

   cllBindVars[cllBindVarCount++]=objIdStr;
   cllBindVars[cllBindVarCount++]=attribute;
   cllBindVars[cllBindVarCount++]=value;
   cllBindVars[cllBindVarCount++]=units;

   allowNullUnits=0;
   if (*units=='\0') {
      allowNullUnits=1;  /* null or empty-string units */
   }
   if (option==1 && *units=='%' && *(units+1)=='\0') {
      allowNullUnits=1; /* wildcard and just % */
   }

   if (allowNullUnits) {
      if (option==1) {  /* use wildcards ('like') */
	 if (logSQL) rodsLog(LOG_SQL, "chlDeleteAVUMetadata SQL 5");
	 status =  cmlExecuteNoAnswerSql(
		"delete from r_objt_metamap where object_id=? and meta_id IN (select meta_id from r_meta_main where meta_attr_name like ? and meta_attr_value like ? and (meta_attr_unit like ? or meta_attr_unit IS NULL) )",
		&icss);
      }
      else {
	 if (logSQL) rodsLog(LOG_SQL, "chlDeleteAVUMetadata SQL 6");
	 status =  cmlExecuteNoAnswerSql(
		"delete from r_objt_metamap where object_id=? and meta_id IN (select meta_id from r_meta_main where meta_attr_name = ? and meta_attr_value = ? and (meta_attr_unit = ? or meta_attr_unit IS NULL) )",
		&icss);
      }
   }
   else {
      if (option==1) {  /* use wildcards ('like') */
	 if (logSQL) rodsLog(LOG_SQL, "chlDeleteAVUMetadata SQL 7");
	 status =  cmlExecuteNoAnswerSql(
		"delete from r_objt_metamap where object_id=? and meta_id IN (select meta_id from r_meta_main where meta_attr_name like ? and meta_attr_value like ? and meta_attr_unit like ?)",
		&icss);
      }
      else {
	 if (logSQL) rodsLog(LOG_SQL, "chlDeleteAVUMetadata SQL 8");
	 status =  cmlExecuteNoAnswerSql(
		"delete from r_objt_metamap where object_id=? and meta_id IN (select meta_id from r_meta_main where meta_attr_name = ? and meta_attr_value = ? and meta_attr_unit = ?)",
		&icss);
      }
   }
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlDeleteAVUMetadata cmlExecuteNoAnswerSql delete failure %d",
	      status);
      _rollback("chlDeleteAVUMetadata");
      return(status);
   }

   /* Audit */
   status = cmlAudit3(AU_DELETE_AVU_METADATA,  
		      objIdStr,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      type,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlDeleteAVUMetadata cmlAudit3 failure %d",
	      status);
      _rollback("chlDeleteAVUMetadata");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlDeleteAVUMetadata cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }

   return(status);
}

/*
Copy an Attribute-Value [Units] pair/triple from one object to another  */
int chlCopyAVUMetadata(rsComm_t *rsComm, char *type1,  char *type2, 
		  char *name1, char *name2) {
   char myTime[50];
   int status;
   rodsLong_t objId1, objId2;
   char objIdStr1[MAX_NAME_LEN];
   char objIdStr2[MAX_NAME_LEN];

   if (logSQL) rodsLog(LOG_SQL, "chlCopyAVUMetadata");

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   if (logSQL) rodsLog(LOG_SQL, "chlCopyAVUMetadata SQL 1 ");
   objId1 = checkAndGetObjectId(rsComm, type1, name1, ACCESS_READ_METADATA);
   if (objId1 < 0) return(objId1);

   if (logSQL) rodsLog(LOG_SQL, "chlCopyAVUMetadata SQL 2");
   objId2 = checkAndGetObjectId(rsComm, type2, name2, ACCESS_CREATE_METADATA);

   if (objId2 < 0) return(objId2);

   snprintf(objIdStr1, MAX_NAME_LEN, "%lld", objId1);
   snprintf(objIdStr2, MAX_NAME_LEN, "%lld", objId2);

   getNowStr(myTime);
   cllBindVars[cllBindVarCount++]=objIdStr2;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=objIdStr1;
   if (logSQL) rodsLog(LOG_SQL, "chlCopyAVUMetadata SQL 3");
   status =  cmlExecuteNoAnswerSql(
                "insert into r_objt_metamap (object_id, meta_id, create_ts, modify_ts) select ?, meta_id, ?, ? from r_objt_metamap where object_id=?",
		&icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlCopyAVUMetadata cmlExecuteNoAnswerSql insert failure %d",
	      status);
      _rollback("chlCopyAVUMetadata");
      return(status);
   }

   /* Audit */
   status = cmlAudit3(AU_COPY_AVU_METADATA,  
		      objIdStr1,
		      rsComm->clientUser.userName,
		      rsComm->clientUser.rodsZone,
		      objIdStr2,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlCopyAVUMetadata cmlAudit3 failure %d",
	      status);
      _rollback("chlCopyAVUMetadata");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlCopyAVUMetadata cmlExecuteNoAnswerSql commit failure %d",
	      status);
      return(status);
   }

   return(status);
}

/* 
 * chlModAccessControl - Modify the Access Control information
 *         of an existing dataObj or collection.
 * "n" (null or none) used to remove access.
 */
int chlModAccessControl(rsComm_t *rsComm, int recursiveFlag,
			char* accessLevel, char *userName, char *zone, 
			char* pathName) {
   char *myAccessLev=NULL;
   char logicalEndName[MAX_NAME_LEN];
   char logicalParentDirName[MAX_NAME_LEN];
   char collIdStr[MAX_NAME_LEN];
   rodsLong_t objId=0;
   rodsLong_t status, status1, status2, status3;
   int rmFlag=0;
   rodsLong_t userId;
   char myTime[50];
   char *myZone;
   char userIdStr[MAX_NAME_LEN];
   char objIdStr[MAX_NAME_LEN];
   char pathStart[MAX_NAME_LEN];
   int len;
   char pathStartLen[20];

   if (logSQL) rodsLog(LOG_SQL, "chlModAccessControl");

   if (strcmp(accessLevel, AP_NULL)==0) {myAccessLev=ACCESS_NULL; rmFlag=1;}
   else if (strcmp(accessLevel,AP_READ)==0) {myAccessLev=ACCESS_READ_OBJECT;}
   else if (strcmp(accessLevel,AP_WRITE)==0){myAccessLev=ACCESS_MODIFY_OBJECT;}
   else if (strcmp(accessLevel,AP_OWN)==0) {myAccessLev=ACCESS_OWN;}
   else {
      int i;
      char errMsg[105];
      snprintf(errMsg, 100, "access level '%s' is invalid",
	       accessLevel);
      i = addRErrorMsg (&rsComm->rError, 0, errMsg);
      return(CAT_INVALID_ARGUMENT);
   }

   if (!icss.status) {
      return(CATALOG_NOT_CONNECTED);
   }

   /* See if the input path is a collection and the user owns it,
      and, if so, get the collectionID */
   if (logSQL) rodsLog(LOG_SQL, "chlModAccessControl SQL 1 ");
   status1 = cmlCheckDir(pathName,
			 rsComm->clientUser.userName, 
			 rsComm->clientUser.rodsZone,
			 ACCESS_OWN, 
			 &icss);
   if (status1 >= 0) {
      snprintf(collIdStr, MAX_NAME_LEN, "%lld", status1);
   }

   /* Not a collection with access, so see if the input path dataObj 
      exists and the user owns it, and, if so, get the objectID */
   if (status1 < 0) {
      status2 = splitPathByKey(pathName,
			       logicalParentDirName, logicalEndName, '/');
      if (strlen(logicalParentDirName)==0) {
	 strcpy(logicalParentDirName, "/");
	 strcpy(logicalEndName, pathName+1);
      }
      if (logSQL) rodsLog(LOG_SQL, "chlModAccessControl SQL 2");
      status2 = cmlCheckDataObjOnly(logicalParentDirName, logicalEndName,
				    rsComm->clientUser.userName, 
				    rsComm->clientUser.rodsZone, 
				    ACCESS_OWN, &icss);
      if (status2 > 0) objId=status2;
   }

   /* If both failed, it doesn't exist or there's no permission */
   if (status1 < 0 && status2 < 0) {
      int i;
      char errMsg[205];

      if (status1 == CAT_UNKNOWN_COLLECTION && status2 == CAT_UNKNOWN_FILE) {
	 snprintf(errMsg, 200, 
		  "Input path is not a collection and not a dataObj: %s",
		  pathName);
	 i = addRErrorMsg (&rsComm->rError, 0, errMsg);
	 return(CAT_INVALID_ARGUMENT);
      }
      if (status1 != CAT_UNKNOWN_COLLECTION) {
	 if (logSQL) rodsLog(LOG_SQL, "chlModAccessControl SQL 10");
	 status3 = cmlCheckDirOwn(pathName,
				  rsComm->clientUser.userName, 
				  rsComm->clientUser.rodsZone, 
				  &icss);
	 if (status3 < 0) return(status1);
	 snprintf(collIdStr, MAX_NAME_LEN, "%lld", status3);
      }
      else {
	 if (status2 == CAT_NO_ACCESS_PERMISSION) {
	    /* See if this user is the owner (with no access, but still
	       allowed to ichmod) */
	    if (logSQL) rodsLog(LOG_SQL, "chlModAccessControl SQL 11");
	    status3 = cmlCheckDataObjOwn(logicalParentDirName, logicalEndName,
					 rsComm->clientUser.userName,
					 rsComm->clientUser.rodsZone,
					 &icss);
	    if (status3 < 0) {
	       _rollback("chlModAccessControl");
	       return(status2);
	    }
	    objId = status3;
	 } else {
	    return(status2);
	 }
      }
   }

   /* Check that the receiving user exists and if so get the userId */
   status = getLocalZone();
   if (status) return(status);

   myZone=zone;
   if (zone == NULL || strlen(zone)==0) {
      myZone=localZone;
   }

   userId=0;
   if (logSQL) rodsLog(LOG_SQL, "chlModAccessControl SQL 3");
   status = cmlGetIntegerValueFromSql(
              "select user_id from r_user_main where user_name=? and r_user_main.zone_name=?",
	      &userId, userName, myZone, 0, 0, 0, &icss);
   if (status != 0) {
      if (status==CAT_NO_ROWS_FOUND) return(CAT_INVALID_USER);
      return(status);
   }

   snprintf(userIdStr, MAX_NAME_LEN, "%lld", userId);
   snprintf(objIdStr, MAX_NAME_LEN, "%lld", objId);

   rodsLog(LOG_NOTICE, "recursiveFlag %d",recursiveFlag);

   /* non-Recursive mode */
   if (recursiveFlag==0) {

      /* doing a dataObj */
      if (objId) { 
	 cllBindVars[cllBindVarCount++]=userIdStr;
	 cllBindVars[cllBindVarCount++]=objIdStr;
	 if (logSQL) rodsLog(LOG_SQL, "chlModAccessControl SQL 4");
	 status =  cmlExecuteNoAnswerSql(
                   "delete from r_objt_access where user_id=? and object_id=?",
		   &icss);
	 if (status != 0 && status != CAT_SUCCESS_BUT_WITH_NO_INFO) {
	    return(status);
	 }
	 if (rmFlag==0) {  /* if not just removing: */
	    getNowStr(myTime);
	    cllBindVars[cllBindVarCount++]=objIdStr;
	    cllBindVars[cllBindVarCount++]=userIdStr;
	    cllBindVars[cllBindVarCount++]=myAccessLev;
	    cllBindVars[cllBindVarCount++]=myTime;
	    cllBindVars[cllBindVarCount++]=myTime;
	    if (logSQL) rodsLog(LOG_SQL, "chlModAccessControl SQL 5");
	    status =  cmlExecuteNoAnswerSql(
			"insert into r_objt_access (object_id, user_id, access_type_id, create_ts, modify_ts)  values (?, ?, (select token_id from R_TOKN_MAIN where token_namespace = 'access_type' and token_name = ?), ?, ?)",
			&icss);
	    if (status) {
	       _rollback("chlModAccessControl");
	       return(status);
	    }
	 }

	 /* Audit */
	 status = cmlAudit5(AU_MOD_ACCESS_CONTROL_OBJ,
			    objIdStr,
			    userIdStr,
			    myAccessLev,
			    &icss);
	 if (status != 0) {
	    rodsLog(LOG_NOTICE,
		    "chlModAccessControl cmlAudit5 failure %d",
		    status);
	    _rollback("chlModAccessControl");
	    return(status);
	 }

	 status =  cmlExecuteNoAnswerSql("commit", &icss);
	 return(status);
      }

      /* doing a collection, non-recursive */
      cllBindVars[cllBindVarCount++]=userIdStr;
      cllBindVars[cllBindVarCount++]=collIdStr;
      if (logSQL) rodsLog(LOG_SQL, "chlModAccessControl SQL 6");
      status =  cmlExecuteNoAnswerSql(
	      "delete from r_objt_access where user_id=? and object_id=?",
  	      &icss);
      if (status != 0 && status != CAT_SUCCESS_BUT_WITH_NO_INFO) {
	 _rollback("chlModAccessControl");
	 return(status);
      }
      if (rmFlag) {  /* just removing */
	 /* Audit */
	 status = cmlAudit5(AU_MOD_ACCESS_CONTROL_COLL,
			    collIdStr,
			    userIdStr,
			    myAccessLev,
			    &icss);
	 if (status != 0) {
	    rodsLog(LOG_NOTICE,
		    "chlModAccessControl cmlAudit5 failure %d",
		    status);
	    _rollback("chlModAccessControl");
	    return(status);
	 }
	 status =  cmlExecuteNoAnswerSql("commit", &icss);
	 return(status);
      }

      getNowStr(myTime);
      cllBindVars[cllBindVarCount++]=collIdStr;
      cllBindVars[cllBindVarCount++]=userIdStr;
      cllBindVars[cllBindVarCount++]=myAccessLev;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=myTime;
      if (logSQL) rodsLog(LOG_SQL, "chlModAccessControl SQL 7");
      status =  cmlExecuteNoAnswerSql(
          "insert into r_objt_access (object_id, user_id, access_type_id, create_ts, modify_ts)  values (?, ?, (select token_id from R_TOKN_MAIN where token_namespace = 'access_type' and token_name = ?), ?, ?)",
	  &icss);

      if (status) {
	 _rollback("chlModAccessControl");
	 return(status);
      }
      /* Audit */
      status = cmlAudit5(AU_MOD_ACCESS_CONTROL_COLL,
			 collIdStr,
			 userIdStr,
			 myAccessLev,
			 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModAccessControl cmlAudit5 failure %d",
		 status);
	 _rollback("chlModAccessControl");
	 return(status);
      }

      status =  cmlExecuteNoAnswerSql("commit", &icss);
      return(status);
   }


   /* Recursive */
   if (objId) {
      int i;
      char errMsg[205];

      snprintf(errMsg, 200, 
            "Input path is not a collection and recursion was requested: %s",
	    pathName);
      i = addRErrorMsg (&rsComm->rError, 0, errMsg);
      return(CAT_INVALID_ARGUMENT);
   }

   snprintf(pathStart, MAX_NAME_LEN, "%s/", pathName);
   len = strlen(pathStart);
   snprintf(pathStartLen, 10, "%d", len);

   cllBindVars[cllBindVarCount++]=userIdStr;
   cllBindVars[cllBindVarCount++]=pathName;
   cllBindVars[cllBindVarCount++]=pathStartLen;
   cllBindVars[cllBindVarCount++]=pathStart;

   if (logSQL) rodsLog(LOG_SQL, "chlModAccessControl SQL 8");
   status =  cmlExecuteNoAnswerSql(
               "delete from r_objt_access where user_id=? and object_id in (select data_id from r_data_main where coll_id in (select coll_id from r_coll_main where coll_name = ? or substr(coll_name,1,?) = ?))",
	       &icss);
   if (status != 0 && status != CAT_SUCCESS_BUT_WITH_NO_INFO) {
      _rollback("chlModAccessControl");
      return(status);
   }
   if (rmFlag) {  /* just removing */

      /* Audit */
      status = cmlAudit5(AU_MOD_ACCESS_CONTROL_COLL_RECURSIVE,
			 collIdStr,
			 userIdStr,
			 myAccessLev,
			 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlModAccessControl cmlAudit5 failure %d",
		 status);
	 _rollback("chlModAccessControl");
	 return(status);
      }

      status =  cmlExecuteNoAnswerSql("commit", &icss);
      return(status);
   }

   getNowStr(myTime);
   snprintf(pathStart, MAX_NAME_LEN, "%s/", pathName);
   len = strlen(pathStart);
   snprintf(pathStartLen, 10, "%d", len);
   cllBindVars[cllBindVarCount++]=userIdStr;
   cllBindVars[cllBindVarCount++]=myAccessLev;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=pathName;
   cllBindVars[cllBindVarCount++]=pathStartLen;
   cllBindVars[cllBindVarCount++]=pathStart;
   if (logSQL) rodsLog(LOG_SQL, "chlModAccessControl SQL 9");
#if ORA_ICAT
   /* For Oracle cast is to integer, for Postgres to bigint */
   status =  cmlExecuteNoAnswerSql(
	         "insert into r_objt_access (object_id, user_id, access_type_id, create_ts, modify_ts)  (select distinct data_id, cast(? as integer), (select token_id from R_TOKN_MAIN where token_namespace = 'access_type' and token_name = ?), ?, ? from r_data_main where coll_id in (select coll_id from r_coll_main where coll_name = ? or substr(coll_name,1,?) = ?))",
		 &icss);
#else
   status =  cmlExecuteNoAnswerSql(
	         "insert into r_objt_access (object_id, user_id, access_type_id, create_ts, modify_ts)  (select distinct data_id, cast(? as bigint), (select token_id from R_TOKN_MAIN where token_namespace = 'access_type' and token_name = ?), ?, ? from r_data_main where coll_id in (select coll_id from r_coll_main where coll_name = ? or substr(coll_name,1,?) = ?))",
		 &icss);
#endif
   if (status) {
      _rollback("chlModAccessControl");
      return(status);
   }

   /* Audit */
   status = cmlAudit5(AU_MOD_ACCESS_CONTROL_COLL_RECURSIVE,
		      collIdStr,
		      userIdStr,
		      myAccessLev,
		      &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
	      "chlModAccessControl cmlAudit5 failure %d",
	      status);
      _rollback("chlModAccessControl");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   return(status);
}

/* 
 * chlRenameObject - Rename a dataObject or collection.
 */
int chlRenameObject(rsComm_t *rsComm, rodsLong_t objId,
			char* newName) {
   int status;
   rodsLong_t collId;
   rodsLong_t otherDataId;
   rodsLong_t otherCollId;
   char myTime[50];

   char parentCollName[MAX_NAME_LEN]="";
   char collName[MAX_NAME_LEN]="";
   char *cVal[3];
   int iVal[3];
   int pLen, newNameLen, cLen, len;
   int isRootDir=0;
   char objIdString[MAX_NAME_LEN];
   char collIdString[MAX_NAME_LEN];
   char collNameTmp[MAX_NAME_LEN];

   char pLenStr[MAX_NAME_LEN];
   char cLenStr[MAX_NAME_LEN];
   char collNameSlash[MAX_NAME_LEN];
   char collNameSlashLen[20];
   char slashNewName[MAX_NAME_LEN];

   if (logSQL) rodsLog(LOG_SQL, "chlRenameObject");

   if (strstr(newName, "/")) {
      return(CAT_INVALID_ARGUMENT);
   }

/* See if it's a dataObj and if so get the coll_id
   check the access permission at the same time */
   collId=0;

   snprintf(objIdString, MAX_NAME_LEN, "%lld", objId);
   if (logSQL) rodsLog(LOG_SQL, "chlRenameObject SQL 1 ");

   status = cmlGetIntegerValueFromSql(
	      "select coll_id from r_data_main DM, r_objt_access OA, r_user_group UG, r_user_main UM, r_tokn_main TM where DM.data_id=? and UM.user_name=? and UM.user_type_name!='rodsgroup' and UM.user_id = UG.user_id and OA.object_id = DM.data_id and UG.group_user_id = OA.user_id and OA.access_type_id >= TM.token_id and TM.token_namespace ='access_type' and TM.token_name = 'own'",
	      &collId, objIdString, rsComm->clientUser.userName, 0, 0, 0,
	      &icss);


   if (status == 0) {  /* it is a dataObj and user has access to it */

      /* check that no other dataObj exists with this name in this collection*/
      snprintf(collIdString, MAX_NAME_LEN, "%lld", collId);
      if (logSQL) rodsLog(LOG_SQL, "chlRenameObject SQL 2");
      status = cmlGetIntegerValueFromSql(
         "select data_id from r_data_main where data_name=? and coll_id=?",
	 &otherDataId, 
	 newName, collIdString, 0, 0, 0, &icss);
      if (status!=CAT_NO_ROWS_FOUND) {
	 return( CAT_NAME_EXISTS_AS_DATAOBJ);
      }

      /* check that no subcoll exists in this collection,
         with the newName */
      snprintf(collNameTmp, MAX_NAME_LEN, "/%s", newName);
      if (logSQL) rodsLog(LOG_SQL, "chlRenameObject SQL 3");
      status = cmlGetIntegerValueFromSql(
                 "select coll_id from r_coll_main where coll_name = ( select coll_name from r_coll_main where coll_id=? ) || ?",
		 &otherCollId, collIdString, collNameTmp, 0, 0, 0, &icss);
      if (status!=CAT_NO_ROWS_FOUND) {
	 return( CAT_NAME_EXISTS_AS_COLLECTION);
      }

      /* update the tables */
      getNowStr(myTime);
      cllBindVars[cllBindVarCount++]=newName;
      cllBindVars[cllBindVarCount++]=objIdString;
      if (logSQL) rodsLog(LOG_SQL, "chlRenameObject SQL 4");
      status =  cmlExecuteNoAnswerSql(
                   "update r_data_main set data_name = ? where data_id=?",
		   &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlRenameObject cmlExecuteNoAnswerSql update1 failure %d",
		 status);
	 _rollback("chlRenameObject");
	 return(status);
      }

      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=collIdString;
      if (logSQL) rodsLog(LOG_SQL, "chlRenameObject SQL 5");
      status =  cmlExecuteNoAnswerSql(
                   "update r_coll_main set modify_ts=? where coll_id=?",
		   &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlRenameObject cmlExecuteNoAnswerSql update2 failure %d",
		 status);
	 _rollback("chlRenameObject");
	 return(status);
      }

      /* Audit */
      status = cmlAudit3(AU_RENAME_DATA_OBJ,  
			 objIdString,
			 rsComm->clientUser.userName,
			 rsComm->clientUser.rodsZone,
			 newName,
			 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlRenameObject cmlAudit3 failure %d",
		 status);
	 _rollback("chlRenameObject");
	 return(status);
      }

      return(status);
   }

   /* See if it's a collection, and get the parentCollName and
     collName, and check permission at the same time */

   cVal[0]=parentCollName;
   iVal[0]=MAX_NAME_LEN;
   cVal[1]=collName;
   iVal[1]=MAX_NAME_LEN;

   snprintf(objIdString, MAX_NAME_LEN, "%lld", objId);
   if (logSQL) rodsLog(LOG_SQL, "chlRenameObject SQL 6");

   status = cmlGetStringValuesFromSql(
	    "select parent_coll_name, coll_name from r_coll_main CM, r_objt_access OA, r_user_group UG, r_user_main UM, r_tokn_main TM where CM.coll_id=? and UM.user_name=? and UM.user_type_name!='rodsgroup' and UM.user_id = UG.user_id and OA.object_id = CM.coll_id and UG.group_user_id = OA.user_id and OA.access_type_id >= TM.token_id and TM.token_namespace ='access_type' and TM.token_name = 'own'",
	    cVal, iVal, 2, objIdString, 
	    rsComm->clientUser.userName, &icss);
   if (status == 0) { 
      /* it is a collection and user has access to it */

      /* check that no other dataObj exists with this name in this collection*/
      if (logSQL) rodsLog(LOG_SQL, "chlRenameObject SQL 7");
      status = cmlGetIntegerValueFromSql(
           "select data_id from r_data_main where data_name=? and coll_id= (select coll_id from r_coll_main  where coll_name = ?)",
	   &otherDataId, newName, parentCollName, 0, 0, 0, &icss);
      if (status!=CAT_NO_ROWS_FOUND) {
	 return( CAT_NAME_EXISTS_AS_DATAOBJ);
      }

      /* check that no subcoll exists in the parent collection,
         with the newName */
      snprintf(collNameTmp, MAX_NAME_LEN, "%s/%s", parentCollName, newName);
      if (logSQL) rodsLog(LOG_SQL, "chlRenameObject SQL 8");
      status = cmlGetIntegerValueFromSql(
               "select coll_id from r_coll_main where coll_name = ?",
	       &otherCollId, collNameTmp, 0, 0, 0, 0, &icss);
      if (status!=CAT_NO_ROWS_FOUND) {
	 return( CAT_NAME_EXISTS_AS_COLLECTION);
      }

      /* update the table */
      pLen = strlen(parentCollName);
      cLen=strlen(collName);
      newNameLen = strlen(newName);
      if (pLen<=0 || cLen <=0) return(CAT_INVALID_ARGUMENT);  /* invalid 
            argument is not really right, but something is really wrong */

      if (pLen==1) {
	 if (strncmp(parentCollName, "/", 20) == 0) { /* just to be sure */
	    isRootDir=1;  /* need to treat a little special below */
	 }
      }

      /* set any collection names that are under this collection to
	 the new name, putting the string together from the the old upper
	 part, newName string, and then (if any for each row) the
	 tailing part of the name. 
	 (In the sql substr function, the index for sql is 1 origin.) */
      snprintf(pLenStr,MAX_NAME_LEN, "%d", pLen); /* formerly +1 but without is
		     correct, makes a difference in Oracle, and works
                     in postgres too. */
      snprintf(cLenStr,MAX_NAME_LEN, "%d", cLen+1);
      snprintf(collNameSlash, MAX_NAME_LEN, "%s/", collName);
      len = strlen(collNameSlash);
      snprintf(collNameSlashLen, 10, "%d", len);
      snprintf(slashNewName, MAX_NAME_LEN, "/%s", newName);
      if (isRootDir) {
	 snprintf(slashNewName, MAX_NAME_LEN, "%s", newName);
      }
      cllBindVars[cllBindVarCount++]=pLenStr;
      cllBindVars[cllBindVarCount++]=slashNewName;
      cllBindVars[cllBindVarCount++]=cLenStr;
      cllBindVars[cllBindVarCount++]=collNameSlashLen;
      cllBindVars[cllBindVarCount++]=collNameSlash;
      cllBindVars[cllBindVarCount++]=collName;
      if (logSQL) rodsLog(LOG_SQL, "chlRenameObject SQL 9");
      status =  cmlExecuteNoAnswerSql(
	           "update r_coll_main set coll_name = substr(coll_name,1,?) || ? || substr(coll_name, ?) where substr(parent_coll_name,1,?) = ? or parent_coll_name  = ?",
		   &icss);
      if (status != 0 && status != CAT_SUCCESS_BUT_WITH_NO_INFO) {
	 rodsLog(LOG_NOTICE,
		 "chlRenameObject cmlExecuteNoAnswerSql update failure %d",
		 status);
	 _rollback("chlRenameObject");
	 return(status);
      }

      /* like above, but for the parent_coll_name's */
      cllBindVars[cllBindVarCount++]=pLenStr;
      cllBindVars[cllBindVarCount++]=slashNewName;
      cllBindVars[cllBindVarCount++]=cLenStr;
      cllBindVars[cllBindVarCount++]=collNameSlashLen;
      cllBindVars[cllBindVarCount++]=collNameSlash;
      cllBindVars[cllBindVarCount++]=collName;
      if (logSQL) rodsLog(LOG_SQL, "chlRenameObject SQL 10");
      status =  cmlExecuteNoAnswerSql(
	          "update r_coll_main set parent_coll_name = substr(parent_coll_name,1,?) || ? || substr(parent_coll_name, ?) where substr(parent_coll_name,1,?) = ? or parent_coll_name  = ?",
		  &icss);
      if (status != 0 && status != CAT_SUCCESS_BUT_WITH_NO_INFO) {
	 rodsLog(LOG_NOTICE,
		 "chlRenameObject cmlExecuteNoAnswerSql update failure %d",
		 status);
	 _rollback("chlRenameObject");
	 return(status);
      }

      /* And now, update the row for this collection */
      getNowStr(myTime);
      snprintf(collNameTmp, MAX_NAME_LEN, "%s/%s", parentCollName, newName);
      if (isRootDir) {
	 snprintf(collNameTmp, MAX_NAME_LEN, "%s%s", parentCollName, newName);
      }
      cllBindVars[cllBindVarCount++]=collNameTmp;
      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=objIdString;
      if (logSQL) rodsLog(LOG_SQL, "chlRenameObject SQL 11");
      status =  cmlExecuteNoAnswerSql(
                    "update r_coll_main set coll_name=?, modify_ts=? where coll_id=?",
		    &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlRenameObject cmlExecuteNoAnswerSql update failure %d",
		 status);
	 _rollback("chlRenameObject");
	 return(status);
      }

      /* Audit */
      status = cmlAudit3(AU_RENAME_COLLECTION,  
			 objIdString,
			 rsComm->clientUser.userName,
			 rsComm->clientUser.rodsZone,
			 newName,
			 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlRenameObject cmlAudit3 failure %d",
		 status);
	 _rollback("chlRenameObject");
	 return(status);
      }

      return(status);

   }


   /* Both collection and dataObj failed, go thru the sql in smaller
      steps to return a specific error */

   snprintf(objIdString, MAX_NAME_LEN, "%lld", objId);
   if (logSQL) rodsLog(LOG_SQL, "chlRenameObject SQL 12");
   status = cmlGetIntegerValueFromSql(
                   "select coll_id from r_data_main where data_id=?",
		   &otherDataId, objIdString, 0, 0, 0, 0, &icss);
   if (status == 0) {
      /* it IS a data obj, must be permission error */
      return (CAT_NO_ACCESS_PERMISSION);
   }

   snprintf(collIdString, MAX_NAME_LEN, "%lld", objId);
   if (logSQL) rodsLog(LOG_SQL, "chlRenameObject SQL 12");
   status = cmlGetIntegerValueFromSql(
              "select coll_id from r_coll_main where coll_id=?",
	      &otherDataId, collIdString, 0, 0, 0, 0, &icss);
   if (status == 0) {
      /* it IS a collection, must be permission error */
      return (CAT_NO_ACCESS_PERMISSION);
   }

   return(CAT_NOT_A_DATAOBJ_AND_NOT_A_COLLECTION);
}

/* 
 * chlMoveObject - Move a dataObject or collection to another
 * collection.
 */
int chlMoveObject(rsComm_t *rsComm, rodsLong_t objId,
		  rodsLong_t targetCollId) {
   int status;
   rodsLong_t collId;
   rodsLong_t otherDataId;
   rodsLong_t otherCollId;
   char myTime[50];

   char dataObjName[MAX_NAME_LEN]="";
   char *cVal[3];
   int iVal[3];

   char parentCollName[MAX_NAME_LEN]="";
   char oldCollName[MAX_NAME_LEN]="";
   char endCollName[MAX_NAME_LEN]="";  /* for example: d1 portion of
					  /tempZone/home/d1  */

   char targetCollName[MAX_NAME_LEN]="";
   char parentTargetCollName[MAX_NAME_LEN]="";
   char newCollName[MAX_NAME_LEN]="";
   int pLen, newNameLen, ocLen;
   int i, OK, len;
   char *cp;
   char objIdString[MAX_NAME_LEN];
   char collIdString[MAX_NAME_LEN];
   char nameTmp[MAX_NAME_LEN];
   char ocLenStr[MAX_NAME_LEN];
   char collNameSlash[MAX_NAME_LEN];
   char collNameSlashLen[20];

   if (logSQL) rodsLog(LOG_SQL, "chlMoveObject");

   /* check that the target collection exists and user has write
      permission, and get the names while at it */
   cVal[0]=parentTargetCollName;
   iVal[0]=MAX_NAME_LEN;
   cVal[1]=targetCollName;
   iVal[1]=MAX_NAME_LEN;
   snprintf(objIdString, MAX_NAME_LEN, "%lld", targetCollId);
   if (logSQL) rodsLog(LOG_SQL, "chlMoveObject SQL 1 ");
   status = cmlGetStringValuesFromSql(
	    "select parent_coll_name, coll_name from r_coll_main CM, r_objt_access OA, r_user_group UG, r_user_main UM, r_tokn_main TM where CM.coll_id=? and UM.user_name=? and UM.user_type_name!='rodsgroup' and UM.user_id = UG.user_id and OA.object_id = CM.coll_id and UG.group_user_id = OA.user_id and OA.access_type_id >= TM.token_id and TM.token_namespace ='access_type' and TM.token_name = 'own'",
	      cVal, iVal, 2, objIdString, 
	      rsComm->clientUser.userName, &icss);

   snprintf(collIdString, MAX_NAME_LEN, "%lld", targetCollId);
   if (status != 0) {
      if (logSQL) rodsLog(LOG_SQL, "chlMoveObject SQL 2");
      status = cmlGetIntegerValueFromSql(
                    "select coll_id from r_coll_main where coll_id=?",
		    &collId, collIdString, 0, 0, 0, 0, &icss);
      if (status==0) {
	 return (CAT_NO_ACCESS_PERMISSION);  /* does exist, must be
						permission error */
      }
      return(CAT_UNKNOWN_COLLECTION);        /* isn't a coll */
   }


/* See if we're moving a dataObj and if so get the data_name;
   and at the same time check the access permission */
   snprintf(objIdString, MAX_NAME_LEN, "%lld", objId);
   if (logSQL) rodsLog(LOG_SQL, "chlMoveObject SQL 3");
   status = cmlGetStringValueFromSql(
	      "select data_name from r_data_main DM, r_objt_access OA, r_user_group UG, r_user_main UM, r_tokn_main TM where DM.data_id=? and UM.user_name=? and UM.user_type_name!='rodsgroup' and UM.user_id = UG.user_id and OA.object_id = DM.data_id and UG.group_user_id = OA.user_id and OA.access_type_id >= TM.token_id and TM.token_namespace ='access_type' and TM.token_name = 'own'",
	     dataObjName, MAX_NAME_LEN, objIdString, 
	     rsComm->clientUser.userName, &icss);
   snprintf(collIdString, MAX_NAME_LEN, "%lld", targetCollId);
   if (status == 0) {  /* it is a dataObj and user has access to it */

      /* check that no other dataObj exists with the ObjName in the
	 target collection */
      if (logSQL) rodsLog(LOG_SQL, "chlMoveObject SQL 4");
      status = cmlGetIntegerValueFromSql(
           "select data_id from r_data_main where data_name=? and coll_id=?",
	   &otherDataId, dataObjName, collIdString, 0, 0, 0, &icss);
      if (status!=CAT_NO_ROWS_FOUND) {
	 return( CAT_NAME_EXISTS_AS_DATAOBJ);
      }

      /* check that no subcoll exists in the target collection, with
         the name of the object */
/* //not needed, I think   snprintf(collIdString, MAX_NAME_LEN, "%d", collId); */
      snprintf(nameTmp, MAX_NAME_LEN, "/%s", dataObjName);
      if (logSQL) rodsLog(LOG_SQL, "chlMoveObject SQL 5");
      status = cmlGetIntegerValueFromSql(
                "select coll_id from r_coll_main where coll_name = ( select coll_name from r_coll_main where coll_id=? ) || ?",
		&otherCollId, collIdString, nameTmp, 0, 0, 0, &icss);
      if (status!=CAT_NO_ROWS_FOUND) {
	 return( CAT_NAME_EXISTS_AS_COLLECTION);
      }

      /* update the table */
      getNowStr(myTime);
      cllBindVars[cllBindVarCount++]=collIdString;
      cllBindVars[cllBindVarCount++]=objIdString;
      if (logSQL) rodsLog(LOG_SQL, "chlMoveObject SQL 6");
      status =  cmlExecuteNoAnswerSql(
	             "update r_data_main set coll_id=? where data_id=?",
		     &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlMoveObject cmlExecuteNoAnswerSql update1 failure %d",
		 status);
	 _rollback("chlMoveObject");
	 return(status);
      }


      cllBindVars[cllBindVarCount++]=myTime;
      cllBindVars[cllBindVarCount++]=collIdString;
      if (logSQL) rodsLog(LOG_SQL, "chlMoveObject SQL 7");
      status =  cmlExecuteNoAnswerSql(
	             "update r_coll_main set modify_ts=? where coll_id=?",
		     &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlMoveObject cmlExecuteNoAnswerSql update2 failure %d",
		 status);
	 _rollback("chlMoveObject");
	 return(status);
      }

      /* Audit */
      status = cmlAudit3(AU_MOVE_DATA_OBJ,  
			 objIdString,
			 rsComm->clientUser.userName,
			 rsComm->clientUser.rodsZone,
			 collIdString,
			 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlMoveObject cmlAudit3 failure %d",
		 status);
	 _rollback("chlMoveObject");
	 return(status);
      }

      return(status);
   }

   /* See if it's a collection, and get the parentCollName and
     oldCollName, and check permission at the same time */
   cVal[0]=parentCollName;
   iVal[0]=MAX_NAME_LEN;
   cVal[1]=oldCollName;
   iVal[1]=MAX_NAME_LEN;

   if (logSQL) rodsLog(LOG_SQL, "chlMoveObject SQL 8");
   status = cmlGetStringValuesFromSql(
	    "select parent_coll_name, coll_name from r_coll_main where coll_id=? and (select access_type_id from R_OBJT_ACCESS where object_id = coll_id and user_id = (select user_id from R_USER_MAIN where user_name=?)) >= (select token_id from R_TOKN_MAIN where token_namespace = 'access_type' and token_name = 'own')",
	       cVal, iVal, 2, objIdString, rsComm->clientUser.userName, &icss);
   if (status == 0) { 
      /* it is a collection and user has access to it */

      pLen = strlen(parentCollName);

      ocLen=strlen(oldCollName);
      if (pLen<=0 || ocLen <=0) return(CAT_INVALID_ARGUMENT);  /* invalid
            argument is not really the right error code, but something
            is really wrong */
      OK=0;
      for (i=ocLen;i>0;i--) {
	 if (oldCollName[i]=='/') {
	    OK=1;
	    strncpy(endCollName, (char*)&oldCollName[i+1], MAX_NAME_LEN);
	    break;
	 }
      }
      if (OK==0) return (CAT_INVALID_ARGUMENT); /* not really, but...*/

      /* check that no other dataObj exists with the ObjName in the
	 target collection */
      snprintf(collIdString, MAX_NAME_LEN, "%lld", targetCollId);
      if (logSQL) rodsLog(LOG_SQL, "chlMoveObject SQL 9");
      status = cmlGetIntegerValueFromSql(
         "select data_id from r_data_main where data_name=? and coll_id=?",
	 &otherDataId, endCollName, collIdString, 0, 0, 0, &icss);
      if (status!=CAT_NO_ROWS_FOUND) {
	 return( CAT_NAME_EXISTS_AS_DATAOBJ);
      }

      /* check that no subcoll exists in the target collection, with
         the name of the object */
      strncpy(newCollName, targetCollName, MAX_NAME_LEN);
      strncat(newCollName, "/", MAX_NAME_LEN);
      strncat(newCollName, endCollName, MAX_NAME_LEN);
      newNameLen = strlen(newCollName);

      if (logSQL) rodsLog(LOG_SQL, "chlMoveObject SQL 10");
      status = cmlGetIntegerValueFromSql(
		 "select coll_id from r_coll_main where coll_name = ?",
		 &otherCollId, newCollName, 0, 0, 0, 0, &icss);
      if (status!=CAT_NO_ROWS_FOUND) {
	 return( CAT_NAME_EXISTS_AS_COLLECTION);
      }


      /* Check that we're not moving the coll down into it's own
	 subtree (which would create a recursive loop) */
      cp = strstr(targetCollName, oldCollName);
      if (cp == targetCollName && 
	(targetCollName[strlen(oldCollName)] == '/' || 
	 targetCollName[strlen(oldCollName)] == '\0')) {
	 return(CAT_RECURSIVE_MOVE);
      }


      /* Update the table */

      /* First, set the collection name and parent collection to the
	 new strings */
      cllBindVars[cllBindVarCount++]=newCollName;
      cllBindVars[cllBindVarCount++]=targetCollName;
      cllBindVars[cllBindVarCount++]=objIdString;
      if (logSQL) rodsLog(LOG_SQL, "chlMoveObject SQL 11");
      status =  cmlExecuteNoAnswerSql(
  	           "update r_coll_main set coll_name = ?, parent_coll_name=? where coll_id = ?",
		   &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlMoveObject cmlExecuteNoAnswerSql update failure %d",
		 status);
	 _rollback("chlMoveObject");
	 return(status);
      }

      /* Now set any collection names that are under this collection to
	 the new name, putting the string together from the the new upper
	 part, endCollName string, and then (if any for each row) the
	 tailing part of the name. 
	 (In the sql substr function, the index for sql is 1 origin.) */
      snprintf(ocLenStr, MAX_NAME_LEN, "%d", ocLen+1);
      snprintf(collNameSlash, MAX_NAME_LEN, "%s/", oldCollName);
      len = strlen(collNameSlash);
      snprintf(collNameSlashLen, 10, "%d", len);
      cllBindVars[cllBindVarCount++]=newCollName;
      cllBindVars[cllBindVarCount++]=ocLenStr;
      cllBindVars[cllBindVarCount++]=newCollName;
      cllBindVars[cllBindVarCount++]=ocLenStr;
      cllBindVars[cllBindVarCount++]=collNameSlashLen;
      cllBindVars[cllBindVarCount++]=collNameSlash;
      cllBindVars[cllBindVarCount++]=oldCollName;
      if (logSQL) rodsLog(LOG_SQL, "chlMoveObject SQL 12");
      status =  cmlExecuteNoAnswerSql(
	             "update r_coll_main set parent_coll_name = ? || substr(parent_coll_name, ?), coll_name = ? || substr(coll_name, ?) where substr(parent_coll_name,1,?) = ? or parent_coll_name = ?",
		     &icss);
      if (status == CAT_SUCCESS_BUT_WITH_NO_INFO) status = 0;
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlMoveObject cmlExecuteNoAnswerSql update failure %d",
		 status);
	 _rollback("chlMoveObject");
	 return(status);
      }

      /* Audit */
      status = cmlAudit3(AU_MOVE_COLL,  
			 objIdString,
			 rsComm->clientUser.userName,
			 rsComm->clientUser.rodsZone,
			 targetCollName,
			 &icss);
      if (status != 0) {
	 rodsLog(LOG_NOTICE,
		 "chlMoveObject cmlAudit3 failure %d",
		 status);
	 _rollback("chlMoveObject");
	 return(status);
      }

      return(status);
   }


   /* Both collection and dataObj failed, go thru the sql in smaller
      steps to return a specific error */
   snprintf(objIdString, MAX_NAME_LEN, "%lld", objId);
   if (logSQL) rodsLog(LOG_SQL, "chlMoveObject SQL 13");
   status = cmlGetIntegerValueFromSql(
	      "select coll_id from r_data_main where data_id=?",
	      &otherDataId, objIdString, 0, 0, 0, 0, &icss);
   if (status == 0) {
      /* it IS a data obj, must be permission error */
      return (CAT_NO_ACCESS_PERMISSION);
   }

   if (logSQL) rodsLog(LOG_SQL, "chlMoveObject SQL 14");
   status = cmlGetIntegerValueFromSql(
       "select coll_id from r_coll_main where coll_id=?",
       &otherDataId, objIdString, 0, 0, 0, 0, &icss);
   if (status == 0) {
      /* it IS a collection, must be permission error */
      return (CAT_NO_ACCESS_PERMISSION);
   }

   return(CAT_NOT_A_DATAOBJ_AND_NOT_A_COLLECTION);
}

/* 
 * chlRegToken - Register a new token
 */
int chlRegToken(rsComm_t *rsComm, char *nameSpace, char *name, char *value,
                char *value2, char *value3, char *comment)
{
   int status;
   rodsLong_t objId;
   char *myValue1, *myValue2, *myValue3, *myComment;
   char myTime[50];
   rodsLong_t seqNum;
   char errMsg[205];
   int i;
   char seqNumStr[MAX_NAME_LEN];

   if (logSQL) rodsLog(LOG_SQL, "chlRegToken");

   if (nameSpace==NULL || strlen(nameSpace)==0) return (CAT_INVALID_ARGUMENT);
   if (name==NULL || strlen(name)==0) return (CAT_INVALID_ARGUMENT);

   if (logSQL) rodsLog(LOG_SQL, "chlRegToken SQL 1 ");
   status = cmlGetIntegerValueFromSql(
              "select token_id from r_tokn_main where token_namespace=? and token_name=?",
	      &objId, "token_namespace", nameSpace, 0, 0, 0, &icss);
   if (status != 0) {
      snprintf(errMsg, 200, 
	       "Token namespace '%s' does not exist",
	       nameSpace);
      i = addRErrorMsg (&rsComm->rError, 0, errMsg);
      return (CAT_INVALID_ARGUMENT);
   }

   if (logSQL) rodsLog(LOG_SQL, "chlRegToken SQL 2");
   status = cmlGetIntegerValueFromSql(
              "select token_id from r_tokn_main where token_namespace=? and token_name=?",
	      &objId, nameSpace, name, 0, 0, 0, &icss);
   if (status == 0) {
      snprintf(errMsg, 200, 
	       "Token '%s' already exists in namespace '%s'",
	       name, nameSpace);
      i = addRErrorMsg (&rsComm->rError, 0, errMsg);
      return (CAT_INVALID_ARGUMENT);
   }

   myValue1=value;
   if (myValue1==NULL) myValue1="";
   myValue2=value2;
   if (myValue2==NULL) myValue2="";
   myValue3=value3;
   if (myValue3==NULL) myValue3="";
   myComment=comment;
   if (myComment==NULL) myComment="";

   if (logSQL) rodsLog(LOG_SQL, "chlRegToken SQL 3");
   seqNum = cmlGetNextSeqVal(&icss);
   if (seqNum < 0) {
      rodsLog(LOG_NOTICE, "chlRegToken cmlGetNextSeqVal failure %d",
	      seqNum);
      return(seqNum);
   }

   getNowStr(myTime);
   snprintf(seqNumStr, MAX_SQL_SIZE, "%lld", seqNum);
   cllBindVars[cllBindVarCount++]=nameSpace;
   cllBindVars[cllBindVarCount++]=seqNumStr;
   cllBindVars[cllBindVarCount++]=name;
   cllBindVars[cllBindVarCount++]=myValue1;
   cllBindVars[cllBindVarCount++]=myValue2;
   cllBindVars[cllBindVarCount++]=myValue3;
   cllBindVars[cllBindVarCount++]=myComment;
   cllBindVars[cllBindVarCount++]=myTime;
   cllBindVars[cllBindVarCount++]=myTime;
   if (logSQL) rodsLog(LOG_SQL, "chlRegToken SQL 4");
   status =  cmlExecuteNoAnswerSql(
	    "insert into r_tokn_main values (?, ?, ?, ?, ?, ?, ?, ?, ?)",
	    &icss);
   if (status) {
      _rollback("chlRegToken");
      return(status);
   }

   /* Audit */
   status = cmlAudit3(AU_REG_TOKEN,  
			 seqNumStr,
			 rsComm->clientUser.userName,
			 rsComm->clientUser.rodsZone,
			 name,
			 &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
		 "chlRegToken cmlAudit3 failure %d",
	      status);
      _rollback("chlRegToken");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   return(status);
}


/* 
 * chlDelToken - Delete a token
 */
int chlDelToken(rsComm_t *rsComm, char *nameSpace, char *name)
{
   int status;
   rodsLong_t objId;
   char errMsg[205];
   int i;
   char objIdStr[60];

   if (logSQL) rodsLog(LOG_SQL, "chlDelToken");

   if (nameSpace==NULL || strlen(nameSpace)==0) return (CAT_INVALID_ARGUMENT);
   if (name==NULL || strlen(name)==0) return (CAT_INVALID_ARGUMENT);

   if (logSQL) rodsLog(LOG_SQL, "chlDelToken SQL 1 ");
   status = cmlGetIntegerValueFromSql(
                 "select token_id from r_tokn_main where token_namespace=? and token_name=?",
		 &objId, nameSpace, name, 0, 0, 0, &icss);
   if (status != 0) {
      snprintf(errMsg, 200, 
	       "Token '%s' does not exist in namespace '%s'",
	       name, nameSpace);
      i = addRErrorMsg (&rsComm->rError, 0, errMsg);
      return (CAT_INVALID_ARGUMENT);
   }

   if (logSQL) rodsLog(LOG_SQL, "chlDelToken SQL 2");
   cllBindVars[cllBindVarCount++]=nameSpace;
   cllBindVars[cllBindVarCount++]=name;
   status =  cmlExecuteNoAnswerSql(
	         "delete from r_tokn_main where token_namespace=? and token_name=?",
		 &icss);
   if (status) {
      _rollback("chlDelToken");
      return(status);
   }

   /* Audit */
   snprintf(objIdStr, 50, "%lld", objId);
   status = cmlAudit3(AU_DEL_TOKEN,  
			 objIdStr,
			 rsComm->clientUser.userName,
			 rsComm->clientUser.rodsZone,
			 name,
			 &icss);
   if (status != 0) {
      rodsLog(LOG_NOTICE,
		 "chlDelToken cmlAudit3 failure %d",
	      status);
      _rollback("chlDelToken");
      return(status);
   }

   status =  cmlExecuteNoAnswerSql("commit", &icss);
   return(status);
}
