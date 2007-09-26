/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/**************************************************************************

  This file contains midLevel functions that can be used to get
  information from the ICAT database. These functions should not have
  any intelligence encoded in them.  These functions use the 'internal
  interface to database' library calls to access the ICAT
  database. Hence these functions can be viewed as providing higher
  level calls to the other routines. These functions do not call any
  of the high-level functions, but do sometimes call each other as well
  as the low-level functions.

**************************************************************************/


#include "icatMidLevelRoutines.h"
#include "icatLowLevel.h"
#include "icatMidLevelHelpers.h"

int logSQL_CML=0;

int cmlDebug(int mode) {
   logSQL_CML = mode;
   return(0);
}

int cmlOpen( icatSessionStruct *icss) {
   int i;

   /* Initialize the icss statement pointers */
   for (i=0; i<MAX_NUM_OF_CONCURRENT_STMTS; i++) {
      icss->stmtPtr[i]=0;
   }

   /* Open Environment */
   i = cllOpenEnv(icss);
   if (i != 0) return(CAT_ENV_ERR);

   /* Connect to the DBMS */
   i = cllConnect(icss);
   if (i != 0) return(CAT_CONNECT_ERR);

   return(0);
}

int cmlClose( icatSessionStruct *icss) {
   int status, stat2;

   status = cllDisconnect(icss);

   stat2 = cllCloseEnv(icss);

   if (status) {
      return(CAT_DISCONNECT_ERR);
   }
   if (stat2) {
      return(CAT_CLOSE_ENV_ERR);
   }
   return(0);
}


int cmlExecuteNoAnswerSql( char *sql, 
			   icatSessionStruct *icss)
{
  int i;
  
  i = cllExecSqlNoResult(icss, sql);
  if (i) { 
     if (i <= CAT_ENV_ERR) return(i); /* already an iRODS error code */
     return(CAT_SQL_ERR);
  }
  return(0);

}

int cmlGetOneRowFromSqlBV (char *sql, 
		   char *cVal[], 
		   int cValSize[], 
		   int numOfCols,
		   char *bindVar1,
		   char *bindVar2,
		   char *bindVar3,
		   char *bindVar4,
		   icatSessionStruct *icss)
{
    int i,j, stmtNum, ii;
    
    i = cllExecSqlWithResultBV(icss, &stmtNum, sql,
				 bindVar1,bindVar2,bindVar3,bindVar4,0,0);
    if (i != 0) {
      if (i <= CAT_ENV_ERR) return(i); /* already an iRODS error code */
      return (CAT_SQL_ERR);
    }
    i = cllGetRow(icss,stmtNum);
    if (i != 0)  {
      ii = cllFreeStatement(icss,stmtNum);
      return(CAT_GET_ROW_ERR);
    }
    if (icss->stmtPtr[stmtNum]->numOfCols == 0) {
      ii = cllFreeStatement(icss,stmtNum);
      return(CAT_NO_ROWS_FOUND);
    }
    for (j = 0; j < numOfCols && j < icss->stmtPtr[stmtNum]->numOfCols ; j++ ) 
      rstrcpy(cVal[j],icss->stmtPtr[stmtNum]->resultValue[j],cValSize[j]);

    i = cllFreeStatement(icss,stmtNum);
    return(j);

}

int cmlGetOneRowFromSql (char *sql, 
		   char *cVal[], 
		   int cValSize[], 
		   int numOfCols,
		   icatSessionStruct *icss)
{
    int i,j, stmtNum, ii;
    
    i = cllExecSqlWithResultBV(icss, &stmtNum, sql,
				 0,0,0,0,0,0);
    if (i != 0) {
      if (i <= CAT_ENV_ERR) return(i); /* already an iRODS error code */
      return (CAT_SQL_ERR);
    }
    i = cllGetRow(icss,stmtNum);
    if (i != 0)  {
      ii = cllFreeStatement(icss,stmtNum);
      return(CAT_GET_ROW_ERR);
    }
    if (icss->stmtPtr[stmtNum]->numOfCols == 0) {
      ii = cllFreeStatement(icss,stmtNum);
      return(CAT_NO_ROWS_FOUND);
    }
    for (j = 0; j < numOfCols && j < icss->stmtPtr[stmtNum]->numOfCols ; j++ ) 
      rstrcpy(cVal[j],icss->stmtPtr[stmtNum]->resultValue[j],cValSize[j]);

    i = cllFreeStatement(icss,stmtNum);
    return(j);

}

/* like cmlGetOneRowFromSql but cVal uses space from query
   and then caller frees it later (via cmlFreeStatement).
   This is simplier for the caller, in some cases.   */
int cmlGetOneRowFromSqlV2 (char *sql, 
		   char *cVal[], 
		   int maxCols,
		   char *bindVar1,
		   char *bindVar2,
		   icatSessionStruct *icss)
{
    int i,j, stmtNum, ii;
    
    i = cllExecSqlWithResultBV(icss, &stmtNum, sql,
				 bindVar1, bindVar2,0,0,0,0);

    if (i != 0) {
      if (i <= CAT_ENV_ERR) return(i); /* already an iRODS error code */
      return (CAT_SQL_ERR);
    }
    i = cllGetRow(icss,stmtNum);
    if (i != 0)  {
      ii = cllFreeStatement(icss,stmtNum);
      return(CAT_GET_ROW_ERR);
    }
    if (icss->stmtPtr[stmtNum]->numOfCols == 0)
      return(CAT_NO_ROWS_FOUND);
    for (j = 0; j < maxCols && j < icss->stmtPtr[stmtNum]->numOfCols ; j++ ) 
       cVal[j] = icss->stmtPtr[stmtNum]->resultValue[j];

    return(stmtNum);  /* 0 or positive is the statement number */
}

int cmlFreeStatement(int statementNumber, icatSessionStruct *icss) 
{
   int i;
   i = cllFreeStatement(icss,statementNumber);
   return(i);
}


int cmlGetFirstRowFromSql (char *sql, 
		   int *statement,
		   int skipCount,
		   icatSessionStruct *icss)
{
    int i, stmtNum, ii;

    *statement=0;
    
    i = cllExecSqlWithResult(icss, &stmtNum, sql);

    if (i != 0) {
      if (i <= CAT_ENV_ERR) return(i); /* already an iRODS error code */
      return (CAT_SQL_ERR);
    }

#ifdef ORA_ICAT
    if (skipCount > 0) {
       for (j=0;j<skipCount;j++) {
	  i = cllGetRow(icss,stmtNum);
	  if (i != 0)  {
	     ii = cllFreeStatement(icss,stmtNum);
	     return(CAT_GET_ROW_ERR);
	  }
	  if (icss->stmtPtr[stmtNum]->numOfCols == 0) {
	     i = cllFreeStatement(icss,stmtNum);
	     return(CAT_NO_ROWS_FOUND);
	  }
       }
    }
#endif

    i = cllGetRow(icss,stmtNum);
    if (i != 0)  {
      ii = cllFreeStatement(icss,stmtNum);
      return(CAT_GET_ROW_ERR);
    }
    if (icss->stmtPtr[stmtNum]->numOfCols == 0) {
      i = cllFreeStatement(icss,stmtNum);
      return(CAT_NO_ROWS_FOUND);
    }

    *statement = stmtNum;
    return(0);
}

/* with bind-variables */
int cmlGetFirstRowFromSqlBV (char *sql, 
	           char *arg1, char *arg2, char *arg3, char *arg4,
		   int *statement,
		   icatSessionStruct *icss)
{
    int i, stmtNum, ii;

    *statement=0;
    
   i = cllExecSqlWithResultBV(icss, &stmtNum, sql,
				 arg1,arg2,arg3,arg4,0,0);

    if (i != 0) {
      if (i <= CAT_ENV_ERR) return(i); /* already an iRODS error code */
      return (CAT_SQL_ERR);
    }
    i = cllGetRow(icss,stmtNum);
    if (i != 0)  {
      ii = cllFreeStatement(icss,stmtNum);
      return(CAT_GET_ROW_ERR);
    }
    if (icss->stmtPtr[stmtNum]->numOfCols == 0) {
      i = cllFreeStatement(icss,stmtNum);
      return(CAT_NO_ROWS_FOUND);
    }

    *statement = stmtNum;
    return(0);
}

int cmlGetNextRowFromStatement (int stmtNum, 
		   icatSessionStruct *icss)
{
    int i, ii;
    
    i = cllGetRow(icss,stmtNum);
    if (i != 0)  {
      ii = cllFreeStatement(icss,stmtNum);
      return(CAT_GET_ROW_ERR);
    }
    if (icss->stmtPtr[stmtNum]->numOfCols == 0) {
       i = cllFreeStatement(icss,stmtNum);
      return(CAT_NO_ROWS_FOUND);
    }
    return(0);
}

int cmlGetStringValueFromSql (char *sql, 
			   char *cVal,
			   int cValSize,
                           char *bindVar1,
                           char *bindVar2,
			   icatSessionStruct *icss)
{
    int i;
    char *cVals[2];
    int iVals[2];

    cVals[0]=cVal;
    iVals[0]=cValSize;

    i = cmlGetOneRowFromSqlBV (sql, cVals, iVals, 1, 
			       bindVar1, bindVar2, 0, 0, icss);
    if (i == 1)
      return(0);
    else
      return(i);

}

int cmlGetStringValuesFromSql (char *sql, 
			   char *cVal[], 
			   int cValSize[],
		           int numberOfStringsToGet,
                           char *bindVar1,
                           char *bindVar2,
			   icatSessionStruct *icss)
{
    int i;

    i = cmlGetOneRowFromSqlBV (sql, cVal, cValSize, numberOfStringsToGet,
			       bindVar1, bindVar2, 0, 0, icss);
    if (i == numberOfStringsToGet)
      return(0);
    else
      return(i);

}

int cmlGetMultiRowStringValuesFromSql (char *sql, 
				       char *returnedStrings,  
			      int maxStringLen,
			      int maxNumberOfStringsToGet, 
			      char *bindVar1,
			      char *bindVar2,
 		              icatSessionStruct *icss) {

    int i,j, stmtNum, ii;
    int tsg; /* total strings gotten */
    char *pString;
    
    if (maxNumberOfStringsToGet <= 0) return(CAT_INVALID_ARGUMENT);

    i = cllExecSqlWithResultBV(icss, &stmtNum, sql,
				 bindVar1,bindVar2,0,0,0,0);
    if (i != 0) {
      if (i <= CAT_ENV_ERR) return(i); /* already an iRODS error code */
      return (CAT_SQL_ERR);
    }
    tsg = 0;
    pString = returnedStrings;
    for (;;) {
       i = cllGetRow(icss,stmtNum);
       if (i != 0)  {
	  ii = cllFreeStatement(icss,stmtNum);
	  if (tsg > 0) return(tsg);
	  return(CAT_GET_ROW_ERR);
       }
       if (icss->stmtPtr[stmtNum]->numOfCols == 0) {
	  ii = cllFreeStatement(icss,stmtNum);
	  if (tsg > 0) return(tsg);
	  return(CAT_NO_ROWS_FOUND);
       }
       for (j = 0; j < icss->stmtPtr[stmtNum]->numOfCols;j++) {
	  rstrcpy(pString, icss->stmtPtr[stmtNum]->resultValue[j],
		  maxStringLen);
	  tsg++;
	  pString+=maxStringLen;
	  if (tsg >= maxNumberOfStringsToGet) {
	     i = cllFreeStatement(icss,stmtNum);
	     return(tsg);
	  }
       }
    }
}


int cmlGetIntegerValueFromSql (char *sql, 
			       rodsLong_t *iVal,
			       char *bindVar1,
			       char *bindVar2,
			       char *bindVar3,
			       char *bindVar4,
			       icatSessionStruct *icss)
{
  int i, cValSize;
  char *cVal[2];
  char cValStr[MAX_INTEGER_SIZE+10];

  cVal[0]=cValStr;
  cValSize = MAX_INTEGER_SIZE;

  i = cmlGetOneRowFromSqlBV (sql, cVal, &cValSize, 1, 
			   bindVar1, bindVar2, bindVar3, bindVar4, icss);
  if (i == 1) {
     if (*cVal[0]=='\0') {
	return(CAT_NO_ROWS_FOUND);
     }
    *iVal = strtoll(*cVal, NULL, 0);
    return(0);
  }
  return(i);
}

int cmlCheckNameToken(char *nameSpace, char *tokenName, icatSessionStruct *icss)
{

  rodsLong_t iVal;
  int status;

  if (logSQL_CML) rodsLog(LOG_SQL, "cmlCheckNameToken SQL 1 ");
  status = cmlGetIntegerValueFromSql (
  "select token_id from  R_TOKN_MAIN where token_namespace=? and token_name=?",
      &iVal, nameSpace, tokenName, 0, 0, icss);
  return(status);

}

int cmlModifySingleTable (char *tableName, 
		       char *updateCols[],
		       char *updateValues[],
		       char *whereColsAndConds[],
		       char *whereValues[],
		       int numOfUpdates, 
		       int numOfConds, 
		       icatSessionStruct *icss)
{
  char tsql[MAX_SQL_SIZE];
  int i, l;
  char *rsql;

  if (logSQL_CML) rodsLog(LOG_SQL, "cmlModifySingleTable SQL 1 ");

  snprintf(tsql, MAX_SQL_SIZE, "update %s set ", tableName);
  l = strlen(tsql);
  rsql = tsql + l;

  cmlArraysToStrWithBind ( rsql, "", updateCols,updateValues, numOfUpdates, " = ", ", ",MAX_SQL_SIZE - l);
  l = strlen(tsql);
  rsql = tsql + l;

  cmlArraysToStrWithBind(rsql, " where ", whereColsAndConds, whereValues, numOfConds, "", " and ", MAX_SQL_SIZE - l);
    
  i = cmlExecuteNoAnswerSql( tsql, icss);
  return(i);

}

#define STR_LEN 100
rodsLong_t
cmlGetNextSeqVal(icatSessionStruct *icss) {
   char nextStr[STR_LEN];
   char sql[STR_LEN];
   int status;
   rodsLong_t iVal;

   if (logSQL_CML) rodsLog(LOG_SQL, "cmlGetNextSeqVal SQL 1 ");

   nextStr[0]='\0';

   cllNextValueString("R_ObjectID", nextStr, STR_LEN);
      /* R_ObjectID is created in icatSysTables.sql as
         the sequence item for objects */

#ifdef ORA_ICAT
   /* For Oracle, use the built-in one-row table */
   snprintf(sql, STR_LEN, "select %s from DUAL", nextStr);
#else
   /* Postgres can just get the next value without a table */
   snprintf(sql, STR_LEN, "select %s", nextStr);
#endif

   status = cmlGetIntegerValueFromSql (sql, &iVal, 0, 0, 0, 0, icss);
   if (status < 0) {
      rodsLog(LOG_NOTICE, 
	      "cmlGetNextSeqVal cmlGetIntegerValueFromSql failure %d", status);
      return(status);
   }
   return(iVal);
}

int 
cmlGetNextSeqStr(char *seqStr, int maxSeqStrLen, icatSessionStruct *icss) {
   char nextStr[STR_LEN];
   char sql[STR_LEN];
   int status;

   if (logSQL_CML) rodsLog(LOG_SQL, "cmlGetNextSeqStr SQL 1 ");

   nextStr[0]='\0';
   cllNextValueString("R_ObjectID", nextStr, STR_LEN);
      /* R_ObjectID is created in icatSysTables.sql as
         the sequence item for objects */

#ifdef ORA_ICAT
   snprintf(sql, STR_LEN, "select %s from DUAL", nextStr);
#else
   snprintf(sql, STR_LEN, "select %s", nextStr);
#endif

   status = cmlGetStringValueFromSql (sql, seqStr, maxSeqStrLen, 0, 0, icss);
   if (status < 0) {
      rodsLog(LOG_NOTICE, 
	      "cmlGetNextSeqStr cmlGetStringValueFromSql failure %d", status);
   }
   return(status);
}

/* modifed for various tests */
int cmlTest( icatSessionStruct *icss) {
  int i, cValSize;
  char *cVal[2];
  char cValStr[MAX_INTEGER_SIZE+10];
  char sql[100];

  icss->databaseUsername="schroede";
  icss->databasePassword="";
  i = cmlOpen(icss);
  if (i != 0) return(i);
  
  cVal[0]=cValStr;
  cValSize = MAX_INTEGER_SIZE;
  snprintf(sql,100, "select coll_id from R_COLL_MAIN where coll_name='a'");

  i = cmlGetOneRowFromSql (sql, cVal, &cValSize, 1, icss);
  if (i == 1) {
    printf("result = %s\n",cValStr);
    i = 0;
  }
  else {
     return(i);
  }

  snprintf(sql,100, "select data_id from R_DATA_MAIN where coll_id='1' and data_name='a'");
  i = cmlGetOneRowFromSql (sql, cVal, &cValSize, 1, icss);
  if (i == 1) {
    printf("result = %s\n",cValStr);
    i = 0;
  }

  return(i);

}


/*
  Check that a collection exists and user has 'accessLevel' permission.
  Return code is either an iRODS error code (< 0) or the collectionId.
*/
rodsLong_t
cmlCheckDir( char *dirName, char *userName, char *accessLevel,
		 icatSessionStruct *icss)
{
   int status;
   rodsLong_t iVal;

   if (logSQL_CML) rodsLog(LOG_SQL, "cmlCheckDir SQL 1 ");

   status = cmlGetIntegerValueFromSql(
	    "select coll_id from R_COLL_MAIN where coll_name=? and (select access_type_id from R_OBJT_ACCESS where object_id = coll_id and user_id = (select user_id from R_USER_MAIN where user_name=?)) >= (select token_id from R_TOKN_MAIN where token_namespace = 'access_type' and token_name = ?)",
	      &iVal, dirName, userName, accessLevel, 0, icss);
   if (status) { 
      /* There was an error, so do another sql to see which 
         of the two likely cases is problem. */

      if (logSQL_CML) rodsLog(LOG_SQL, "cmlCheckDir SQL 2 ");

      status = cmlGetIntegerValueFromSql(
		 "select coll_id from R_COLL_MAIN where coll_name=?",
		 &iVal, dirName, 0, 0, 0, icss);
      if (status) {
	 return(CAT_UNKNOWN_COLLECTION);
      }
      return (CAT_NO_ACCESS_PERMISSION);
   }

   return(iVal);

}

/*
  Check that a collection exists and user has 'accessLevel' permission.
  Return code is either an iRODS error code (< 0) or the collectionId.
*/
rodsLong_t
cmlCheckDirId( char *dirId, char *userName, char *accessLevel,
		 icatSessionStruct *icss)
{
   int status;
   rodsLong_t iVal;

   if (logSQL_CML) rodsLog(LOG_SQL, "cmlCheckDirId SQL 1 ");

   status = cmlGetIntegerValueFromSql(
	    "select coll_id from R_COLL_MAIN where coll_id=? and (select access_type_id from R_OBJT_ACCESS where object_id = coll_id and user_id = (select user_id from R_USER_MAIN where user_name=?)) >= (select token_id from R_TOKN_MAIN where token_namespace = 'access_type' and token_name = ?)",
	      &iVal, dirId, userName, accessLevel, 0, icss);
   if (status) { 
      /* There was an error, so do another sql to see which 
         of the two likely cases is problem. */

      if (logSQL_CML) rodsLog(LOG_SQL, "cmlCheckDirId SQL 2 ");

      status = cmlGetIntegerValueFromSql(
		 "select coll_id from R_COLL_MAIN where coll_id=?",
		 &iVal, dirId, 0, 0, 0, icss);
      if (status) {
	 return(CAT_UNKNOWN_COLLECTION);
      }
      return (CAT_NO_ACCESS_PERMISSION);
   }

   return(0);
}

/*
  Check that a collection exists and user owns it
*/
rodsLong_t
cmlCheckDirOwn( char *dirName, char *userName, 
			icatSessionStruct *icss)
{
   int status; 
   rodsLong_t iVal; 

   if (logSQL_CML) rodsLog(LOG_SQL, "cmlCheckDirOwn SQL 1 ");

   status = cmlGetIntegerValueFromSql(
	    "select coll_id from R_COLL_MAIN where coll_name=? and coll_owner_name=?",
	    &iVal, dirName, userName, 0, 0, icss);
   if (status < 0) return(status);
   return(iVal);
}


/*
  Check that a dataObj (iRODS file) exists and user has specified permission
  (but don't check the collection access, only its existance).
  Return code is either an iRODS error code (< 0) or the dataId.
*/
rodsLong_t
cmlCheckDataObjOnly( char *dirName, char *dataName, char *userName, 
		     char *accessLevel, icatSessionStruct *icss)
{
   int status;
   rodsLong_t iVal; 
   rodsLong_t collId;
   char collIdStr[MAX_NAME_LEN];

   if (logSQL_CML) rodsLog(LOG_SQL, "cmlCheckDataObjOnly SQL 1 ");

   status = cmlGetIntegerValueFromSql(
	    "select coll_id from R_COLL_MAIN where coll_name=?",
	    &iVal, dirName, 0, 0, 0, icss);
   if (status < 0) return(status);
   collId = iVal;
   snprintf(collIdStr, MAX_NAME_LEN, "%lld", collId);

   if (logSQL_CML) rodsLog(LOG_SQL, "cmlCheckDataObjOnly SQL 2 ");

   status = cmlGetIntegerValueFromSql(
	         "select data_id from R_DATA_MAIN where data_name=? and coll_id=? and (select access_type_id from R_OBJT_ACCESS where object_id = data_id and user_id = (select user_id from R_USER_MAIN where user_name=?)) >= (select token_id from R_TOKN_MAIN where token_namespace = 'access_type' and token_name = ?)",
		 &iVal, dataName, collIdStr, userName, accessLevel, icss);

   if (status) {
      /* There was an error, so do another sql to see if the user has
         group access.  Could try to combine this with the above sql
         to be a bit faster in some cases, but for now do this
         separate check. */
      if (logSQL_CML) rodsLog(LOG_SQL, "cmlCheckDataObjOnly SQL 3 ");
      status = cmlGetIntegerValueFromSql( 
	       "select data_id from R_DATA_MAIN where data_name=? and coll_id=? and (select access_type_id from R_OBJT_ACCESS where object_id = data_id and user_id = (select group_user_id from r_user_group where user_id = (select user_id from R_USER_MAIN where user_name=?))) >= (select token_id from R_TOKN_MAIN where token_namespace = 'access_type' and token_name = ?)",
	       &iVal, dataName, collIdStr, userName, accessLevel, icss);
   }
   if (status) { 
      /* There was an error, so do another sql to see which 
         of the two likely cases is problem. */
      if (logSQL_CML) rodsLog(LOG_SQL, "cmlCheckDataObjOnly SQL 4 ");

      status = cmlGetIntegerValueFromSql(
	    "select data_id from R_DATA_MAIN where data_name=? and coll_id=? ",
	    &iVal, dataName, collIdStr, 0, 0, icss);
      if (status) {
	 return(CAT_UNKNOWN_FILE);
      }
      return (CAT_NO_ACCESS_PERMISSION);
   }

   return(iVal);

}

/*
  Check that a dataObj (iRODS file) exists and user owns it
*/
rodsLong_t
cmlCheckDataObjOwn( char *dirName, char *dataName, char *userName, 
			icatSessionStruct *icss)
{
   int status;
   rodsLong_t iVal, collId;
   char collIdStr[MAX_NAME_LEN];

   if (logSQL_CML) rodsLog(LOG_SQL, "cmlCheckDataObjOwn SQL 1 ");
   status = cmlGetIntegerValueFromSql(
	    "select coll_id from R_COLL_MAIN where coll_name=?",
	    &iVal, dirName, 0, 0, 0, icss);
   if (status < 0) return(status);
   collId = iVal;
   snprintf(collIdStr, MAX_NAME_LEN, "%lld", collId);

   if (logSQL_CML) rodsLog(LOG_SQL, "cmlCheckDataObjOwn SQL 2 ");
   status = cmlGetIntegerValueFromSql(
	         "select data_id from R_DATA_MAIN where data_name=? and coll_id=? and data_owner_name=?",
		 &iVal, dataName, collIdStr, userName, 0, icss);

   if (status) {
      return (status);
   }
   return(iVal);
}

/*
  Check that a user has the specified permission or better to a dataObj.
  Return value is either an iRODS error code (< 0) or success (0).
*/
int cmlCheckDataObjId( char *dataId, char *userName,  char *zoneName, 
		     char *accessLevel, icatSessionStruct *icss)
{
   int status;
   rodsLong_t iVal; 

   if (logSQL_CML) rodsLog(LOG_SQL, "cmlCheckDataObjId SQL 1 ");

   iVal=0;
   status = cmlGetIntegerValueFromSql(
	    "select object_id from r_objt_access where object_id=? and user_id = (select user_id from r_user_main where user_name=? and zone_name=?) and access_type_id >= (select token_id from R_TOKN_MAIN where token_namespace = 'access_type' and token_name = ?)",
	    &iVal,
	    dataId,
	    userName,
	    zoneName,
	    accessLevel,
	    icss);

   if (status) {
      /* There was an error, so do another sql to see if the user has
         group access.  Could try to combine this with the above sql
         to be a bit faster in some cases, but for now do this
         separate check. */
      if (logSQL_CML) rodsLog(LOG_SQL, "cmlCheckDataObjId SQL 2 ");
      status = cmlGetIntegerValueFromSql(
	       "select object_id from r_objt_access where object_id=? and user_id = (select group_user_id from r_user_group where user_id = (select user_id from r_user_main where user_name=? and zone_name=?)) and access_type_id >= (select token_id from R_TOKN_MAIN where token_namespace = 'access_type' and token_name = ?)",
	       &iVal, 
	       dataId,
	       userName,
	       zoneName,
	       accessLevel,
	       icss);
   }
   if (status != 0) return (CAT_NO_ACCESS_PERMISSION);
   if (iVal==0)  return (CAT_NO_ACCESS_PERMISSION);
   return(status);
}
