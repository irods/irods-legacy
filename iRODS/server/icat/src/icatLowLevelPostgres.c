/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 

These are the Catalog Low Level (cll) routines for talking to postgresql.

For each of the supported database systems there is .c file like this
one with a set of routines by the same names.

Callable functions:
   cllOpenEnv
   cllCloseEnv
   cllConnect
   cllDisconnect
   cllGetRowCount
   cllExecSqlNoResult
   cllExecSqlWithResult
   cllDoneWithResult
   cllDoneWithDefaultResult
   cllGetRow
   cllGetRows
   cllGetNumberOfColumns
   cllGetColumnInfo
   cllNextValueString

Internal functions are those that do not begin with cll.
The external functions used are those that begin with SQL.

*/

#include "icatLowLevelPostgres.h"
int _cllFreeStatementColumns(icatSessionStruct *icss, int statementNumber);

int cllBindVarCount=0;
char *cllBindVars[MAX_BIND_VARS];
int cllBindVarCountPrev=0; /* cclBindVarCount earlier in processing */

/* for now: */
#define MAX_TOKEN 256

#define TMP_STR_LEN 1040

SQLINTEGER columnLength[MAX_TOKEN];  /* change me ! */

#include <stdio.h>
#include <pwd.h>


/*
  call SQLError to get error information and log it
 */
int
logPsgError(int level, HENV henv, HDBC hdbc, HSTMT hstmt)
{
   SQLCHAR         msg[SQL_MAX_MESSAGE_LENGTH + 10];
   SQLCHAR         sqlstate[ SQL_SQLSTATE_SIZE + 10];
   SQLINTEGER sqlcode;
   SQLSMALLINT length;
   int errorVal=-2;
   while (SQLError(henv, hdbc, hstmt, sqlstate, &sqlcode, msg,
		   SQL_MAX_MESSAGE_LENGTH + 1, &length) == SQL_SUCCESS) {
      if (strstr((char *)msg, "duplicate key")) {
         errorVal = CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME;
      }
      rodsLog(level,"SQLSTATE: %s", sqlstate);
      rodsLog(level,"SQLCODE: %ld", sqlcode);
      rodsLog(level,"SQL Error message: %s", msg);
   }
   return(errorVal);
}

/* 
 Allocate the environment structure for use by the SQL routines.
 */
int
cllOpenEnv(icatSessionStruct *icss) {
   RETCODE stat;

   HENV myHenv;

   stat = SQLAllocEnv(&myHenv);

   if (stat != SQL_SUCCESS) {
      rodsLog(LOG_ERROR, "cllOpenEnv: SQLAllocEnv failed");
      return (-1);
   }

   icss->environPtr=myHenv;
   return(0);
}


/* 
 Deallocate the environment structure.
 */
int
cllCloseEnv(icatSessionStruct *icss) {
   RETCODE stat;
   HENV myHenv;

   myHenv = icss->environPtr;
   stat =SQLFreeEnv(myHenv);

   if (stat != SQL_SUCCESS) {
      rodsLog(LOG_ERROR, "cllCloseEnv: SQLFreeEnv failed");
   }
   return(stat);
}

/*
 Connect to the DBMS.
 */
int 
cllConnect(icatSessionStruct *icss) {
   RETCODE stat;
   RETCODE stat2;

   SQLCHAR         buffer[SQL_MAX_MESSAGE_LENGTH + 1];
   SQLCHAR         sqlstate[SQL_SQLSTATE_SIZE + 1];
   SQLINTEGER      sqlcode;
   SQLSMALLINT     length;

   HDBC myHdbc;

   stat = SQLAllocConnect(icss->environPtr,
			  &myHdbc);
   if (stat != SQL_SUCCESS) {
      rodsLog(LOG_ERROR, "cllConnect: SQLAllocConnect failed: %d, stat");
      return (-1);
   }

   stat = SQLSetConnectOption(myHdbc,
			      SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF);
   if (stat != SQL_SUCCESS) {
      rodsLog(LOG_ERROR, "cllConnect: SQLSetConnectOption failed: %d", stat);
      return (-1);
   }

   stat = SQLConnect(myHdbc, (unsigned char *)POSTGRES_DATABASE_NAME, SQL_NTS, 
		     (unsigned char *)icss->databaseUsername, SQL_NTS, 
		     (unsigned char *)icss->databasePassword, SQL_NTS);
   if (stat != SQL_SUCCESS) {
      rodsLog(LOG_ERROR, "cllConnect: SQLConnect failed: %d", stat);
      rodsLog(LOG_ERROR, "cllConnect: SQLConnect failed:db=%s,user=%s,pass=%s\n",POSTGRES_DATABASE_NAME,icss->databaseUsername,  icss->databasePassword);
      while (SQLError(icss->environPtr,myHdbc , 0, sqlstate, &sqlcode, buffer,
                    SQL_MAX_MESSAGE_LENGTH + 1, &length) == SQL_SUCCESS) {
        rodsLog(LOG_ERROR, "cllConnect:          SQLSTATE: %s\n", sqlstate);
        rodsLog(LOG_ERROR, "cllConnect:  Native Error Code: %ld\n", sqlcode);
        rodsLog(LOG_ERROR, "cllConnect: %s \n", buffer);
    }

      stat2 = SQLDisconnect(myHdbc);
      stat2 = SQLFreeConnect(myHdbc);
      return (-1);
   }

   icss->connectPtr=myHdbc;
   return(0);
}

/*
 Disconnect from the DBMS.
*/
int
cllDisconnect(icatSessionStruct *icss) {
   RETCODE stat;
   HDBC myHdbc;

   myHdbc = icss->connectPtr;

   stat = SQLDisconnect(myHdbc);
   if (stat != SQL_SUCCESS) {
      rodsLog(LOG_ERROR, "cllDisconnect: SQLDisconnect failed: %d", stat);
      return(-1);
   }

   stat = SQLFreeConnect(myHdbc);
   if (stat != SQL_SUCCESS) {
      rodsLog(LOG_ERROR, "cllDisconnect: SQLFreeConnect failed: %d", stat);
      return(-2);
   }

   icss->connectPtr = myHdbc;
   return(0);
}

/*
 Execute a SQL command which has no resulting table.  Examples include
 insert, delete, update, or ddl.
 */
int
cllExecSqlNoResult(icatSessionStruct *icss, char *sql)
{
   return (cllExecSqlNoResultBV(icss, sql,0,0,0));
}

/*
 Log the bind variables from the global array (after an error)
*/
void
logTheBindVariables(int level)
{
   int i;
   char tmpStr[TMP_STR_LEN+2];
   for (i=0;i<cllBindVarCountPrev;i++) {
      snprintf(tmpStr, TMP_STR_LEN, "bindVar[%d]=%s", i+1, cllBindVars[i]);
      rodsLog(level, tmpStr);
   }
}

/*
 Bind variables from the global array.
 */
int
bindTheVariables(HSTMT myHstmt, char *sql) {
   int myBindVarCount;
   RETCODE stat;
   int i;
   char tmpStr[TMP_STR_LEN+2];

   myBindVarCount = cllBindVarCount;
   cllBindVarCountPrev=cllBindVarCount; /* save in case we need to log error */
   cllBindVarCount = 0; /* reset for next call */

   if (myBindVarCount > 0) {
      rodsLogSql("SQLPrepare");
      stat = SQLPrepare(myHstmt,  (unsigned char *)sql, SQL_NTS);
      if (stat != SQL_SUCCESS) {
	 rodsLog(LOG_ERROR, "bindTheVariables: SQLPrepare failed: %d",
		 stat);
	 return(-1);
      }

      for (i=0;i<myBindVarCount;i++) {
	 stat = SQLBindParameter(myHstmt, i+1, SQL_PARAM_INPUT, SQL_C_CHAR,
				 SQL_C_CHAR, 0, 0, cllBindVars[i], 0, 0);
	 snprintf(tmpStr, TMP_STR_LEN, "bindVar[%d]=%s", i+1, cllBindVars[i]);
	 rodsLogSql(tmpStr);
	 if (stat != SQL_SUCCESS) {
	    rodsLog(LOG_ERROR, 
		    "bindTheVariables: SQLBindParameter failed: %d", stat);
	    return(-1);
	 }
      }

      if (stat != SQL_SUCCESS) {
	 rodsLog(LOG_ERROR, "bindTheVariables: SQLAllocStmt failed: %d",
		 stat);
	 return(-1);
      }
   }
   return(0);
}

/*
 Execute a SQL command which has no resulting table.  With optional
 bind variables.
 */
int
cllExecSqlNoResultBV(icatSessionStruct *icss, char *sql,
		   char *bindVar1, char *bindVar2, char *bindVar3) {
   RETCODE stat;
   HDBC myHdbc;
   HSTMT myHstmt;
   int result;
   char *status;
   SQLINTEGER rowCount;
#ifdef NEW_ODBC
   int i;
#endif
   myHdbc = icss->connectPtr;
   rodsLog(LOG_DEBUG1, sql);
   stat = SQLAllocStmt(myHdbc, &myHstmt); 
   if (stat != SQL_SUCCESS) {
      rodsLog(LOG_ERROR, "cllExecSqlNoResultBV: SQLAllocStmt failed: %d", stat);
      return(-1);
   }

   /*   // used? */
   if (bindVar1 != 0 && *bindVar1 != '\0') {
      stat = SQLBindParameter(myHstmt, 1, SQL_PARAM_INPUT, SQL_C_SBIGINT,
			      SQL_C_SBIGINT, 0, 0, 0, 0, 0);
      if (stat != SQL_SUCCESS) {
	 rodsLog(LOG_ERROR, "cllExecSqlNoResultBV: SQLAllocStmt failed: %d",
		 stat);
	 return(-1);
      }
   }

   if (bindTheVariables(myHstmt, sql) != 0) return(-1);

   rodsLogSql(sql);
   stat = SQLExecDirect(myHstmt, (unsigned char *)sql, SQL_NTS);
   status = "UNKNOWN";
   if (stat == SQL_SUCCESS) status= "SUCCESS";
   if (stat == SQL_SUCCESS_WITH_INFO) status="SUCCESS_WITH_INFO";
   if (stat == SQL_NO_DATA_FOUND) status="NO_DATA";
   if (stat == SQL_ERROR) status="SQL_ERROR";
   if (stat == SQL_INVALID_HANDLE) status="HANDLE_ERROR";
   rodsLogSqlResult(status);

   if (stat == SQL_SUCCESS || stat == SQL_SUCCESS_WITH_INFO ||
      stat == SQL_NO_DATA_FOUND) {
      result = 0;
      if (stat == SQL_NO_DATA_FOUND) result = CAT_SUCCESS_BUT_WITH_NO_INFO;
#ifdef NEW_ODBC
      /* Doesn't seem to return SQL_NO_DATA_FOUND, so check */
      i = SQLRowCount (myHstmt, (SQLINTEGER *)&rowCount);
      if (i) {
	 /* error getting rowCount???, just call it no_info */
	 result = CAT_SUCCESS_BUT_WITH_NO_INFO;
      }
      if (rowCount==0) result = CAT_SUCCESS_BUT_WITH_NO_INFO;
#else
      rowCount=0; /* avoid compiler warning */
#endif
   }
   else {
      logTheBindVariables(LOG_NOTICE);
      rodsLog(LOG_NOTICE, "cllExecSqlNoResult: SQLExecDirect error: %d sql:%s",
	      stat, sql);
      result = logPsgError(LOG_NOTICE, icss->environPtr, myHdbc, myHstmt);
   }

   stat = SQLFreeStmt(myHstmt, SQL_DROP);
   if (stat != SQL_SUCCESS) {
      rodsLog(LOG_ERROR, "cllExecSqlNoResult: SQLFreeStmt error: %d", stat);
   }

   return(result);
}

/* 
  Execute a SQL command that returns a result table, and
  and bind the default row.
  This version now uses the global array of bind variables.
*/
int
cllExecSqlWithResult(icatSessionStruct *icss, int *stmtNum, char *sql) {

   RETCODE stat;
   HDBC myHdbc;
   HSTMT hstmt;	
   SQLSMALLINT numColumns;

   SQLCHAR         colName[MAX_TOKEN];
   SQLSMALLINT     colType;
   SQLSMALLINT     colNameLen;
   SQLUINTEGER     precision;
   SQLSMALLINT     scale;
   SQLINTEGER      displaysize;
   static SQLINTEGER resultDataSize;

   icatStmtStrct *myStatement;

   int i;
   int statementNumber;
   char *status;

   myHdbc = icss->connectPtr;
   rodsLog(LOG_DEBUG1, sql);
   stat = SQLAllocStmt(myHdbc, &hstmt); 
   if (stat != SQL_SUCCESS) {
      rodsLog(LOG_ERROR, "cllExecSqlWithResult: SQLAllocStmt failed: %d",
	      stat);
      return(-1);
   }

   statementNumber=-1;
   for (i=0;i<MAX_NUM_OF_CONCURRENT_STMTS && statementNumber<0;i++) {
      if (icss->stmtPtr[i]==0) {
	 statementNumber=i;
      }
   }
   if (statementNumber<0) {
      rodsLog(LOG_ERROR, 
	      "cllExecSqlWithResult: too many concurrent statements");
      return(-2);
   }

   myStatement = (icatStmtStrct *)malloc(sizeof(icatStmtStrct));
   icss->stmtPtr[statementNumber]=myStatement;

   myStatement->stmtPtr=hstmt;

   if (bindTheVariables(hstmt, sql) != 0) return(-1);

   rodsLogSql(sql);
   stat = SQLExecDirect(hstmt, (unsigned char *)sql, SQL_NTS);
   status = "UNKNOWN";
   if (stat == SQL_SUCCESS) status= "SUCCESS";
   if (stat == SQL_SUCCESS_WITH_INFO) status="SUCCESS_WITH_INFO";
   if (stat == SQL_NO_DATA_FOUND) status="NO_DATA";
   if (stat == SQL_ERROR) status="SQL_ERROR";
   if (stat == SQL_INVALID_HANDLE) status="HANDLE_ERROR";
   rodsLogSqlResult(status);

   if (stat == SQL_SUCCESS ||
       stat == SQL_SUCCESS_WITH_INFO || 
       stat == SQL_NO_DATA_FOUND) {
   }
   else {
      logTheBindVariables(LOG_NOTICE);
      rodsLog(LOG_NOTICE, 
	      "cllExecSqlWithResult: SQLExecDirect error: %d, sql:%s",
	      stat, sql);
      logPsgError(LOG_NOTICE, icss->environPtr, myHdbc, hstmt);
      return(-1);
   }

   stat =  SQLNumResultCols(hstmt, &numColumns);
   if (stat != SQL_SUCCESS) {
      rodsLog(LOG_ERROR, "cllExecSqlWithResult: SQLNumResultCols failed: %d",
	      stat);
      return(-2);
   }
   myStatement->numOfCols=numColumns;

   for (i = 0; i<numColumns; i++) {
      stat = SQLDescribeCol(hstmt, i+1, colName, sizeof(colName),
			    &colNameLen, &colType, &precision, &scale, NULL);
      if (stat != SQL_SUCCESS) {
	 rodsLog(LOG_ERROR, "cllExecSqlWithResult: SQLDescribeCol failed: %d",
	      stat);
	 return(-3);
      }
      /*  printf("colName='%s' precision=%d\n",colName, precision); */
      columnLength[i]=precision;
      stat = SQLColAttributes(hstmt, i+1, SQL_COLUMN_DISPLAY_SIZE, 
			      NULL, 0, NULL, &displaysize);
      if (stat != SQL_SUCCESS) {
	 rodsLog(LOG_ERROR, 
		 "cllExecSqlWithResult: SQLColAttributes failed: %d",
		 stat);
	 return(-3);
      }

      if (displaysize > (strlen((char *) colName))) {
	 columnLength[i] = displaysize + 1;
      }
      else {
	 columnLength[i] = strlen((char *) colName) + 1;
      }
      /*      printf("columnLength[%d]=%d\n",i,columnLength[i]); */
 
      myStatement->resultValue[i] = malloc((int)columnLength[i]);

      strcpy((char *)myStatement->resultValue[i],"");

#if NEW_ODBC
      stat = SQLBindCol(hstmt, i+1, SQL_C_CHAR, myStatement->resultValue[i], 
			columnLength[i], NULL);
      /* The last argument could be resultDataSize (a SQLINTEGER
         location), which will be returned later via the SQLFetch.
         Since unused now, passing in NULL tells ODBC to skip it */
#else
      /* The old ODBC needs a non-NULL value */
      stat = SQLBindCol(hstmt, i+1, SQL_C_CHAR, myStatement->resultValue[i], 
			columnLength[i], &resultDataSize);
#endif

      if (stat != SQL_SUCCESS) {
	 rodsLog(LOG_ERROR, 
		 "cllExecSqlWithResult: SQLColAttributes failed: %d",
		 stat);
	 return(-4);
      }


      myStatement->resultColName[i] = malloc((int)columnLength[i]);
      strncpy(myStatement->resultColName[i], (char *)colName, columnLength[i]);

   }
   *stmtNum = statementNumber;
   return(0);
}

/* logBindVars 
   For when an error occurs, log the bind variables which were used
   with the sql.
*/
void
logBindVars(int level, char *bindVar1, char *bindVar2, char *bindVar3,
		char *bindVar4, char *bindVar5, char *bindVar6) {
   if (bindVar1 != 0 && *bindVar1 != '\0') {
      rodsLog(level,"bindVar1=%s", bindVar1);
   }
   if (bindVar2 != 0 && *bindVar2 != '\0') {
      rodsLog(level,"bindVar2=%s", bindVar2);
   }
   if (bindVar3 != 0 && *bindVar3 != '\0') {
      rodsLog(level,"bindVar3=%s", bindVar3);
   }
   if (bindVar4 != 0 && *bindVar4 != '\0') {
      rodsLog(level,"bindVar4=%s", bindVar4);
   }
}


/* 
  Execute a SQL command that returns a result table, and
  and bind the default row; and allow optional bind variables.
*/
int
cllExecSqlWithResultBV(icatSessionStruct *icss, int *stmtNum, char *sql,
 			 char *bindVar1, char *bindVar2, char *bindVar3,
			 char *bindVar4, char *bindVar5, char *bindVar6) {

   RETCODE stat;
   HDBC myHdbc;
   HSTMT hstmt;	
   SQLSMALLINT numColumns;

   SQLCHAR         colName[MAX_TOKEN];
   SQLSMALLINT     colType;
   SQLSMALLINT     colNameLen;
   SQLUINTEGER     precision;
   SQLSMALLINT     scale;
   SQLINTEGER      displaysize;
   static SQLINTEGER resultDataSize;

   icatStmtStrct *myStatement;

   int i;
   int statementNumber;
   char *status;
   char tmpStr[TMP_STR_LEN+2];

   myHdbc = icss->connectPtr;
   rodsLog(LOG_DEBUG1, sql);
   stat = SQLAllocStmt(myHdbc, &hstmt); 
   if (stat != SQL_SUCCESS) {
      rodsLog(LOG_ERROR, "cllExecSqlWithResultBV: SQLAllocStmt failed: %d",
	      stat);
      return(-1);
   }

   statementNumber=-1;
   for (i=0;i<MAX_NUM_OF_CONCURRENT_STMTS && statementNumber<0;i++) {
      if (icss->stmtPtr[i]==0) {
	 statementNumber=i;
      }
   }
   if (statementNumber<0) {
      rodsLog(LOG_ERROR, 
	      "cllExecSqlWithResultBV: too many concurrent statements");
      return(-2);
   }

   myStatement = (icatStmtStrct *)malloc(sizeof(icatStmtStrct));
   icss->stmtPtr[statementNumber]=myStatement;

   myStatement->stmtPtr=hstmt;

   if ((bindVar1 != 0 && *bindVar1 != '\0')  ||
       (bindVar2 != 0 && *bindVar2 != '\0')  ||
       (bindVar3 != 0 && *bindVar3 != '\0')  ||
       (bindVar4 != 0 && *bindVar4 != '\0')) {

      rodsLogSql("SQLPrepare");
      stat = SQLPrepare(hstmt,  (unsigned char *)sql, SQL_NTS);
      if (stat != SQL_SUCCESS) {
	 rodsLog(LOG_ERROR, "cllExecSqlNoResult: SQLPrepare failed: %d",
		 stat);
	 return(-1);
      }

      if (bindVar1 != 0 && *bindVar1 != '\0') {
	  stat = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
	 			 SQL_C_CHAR, 0, 0, bindVar1, 0, 0);
	  snprintf(tmpStr, TMP_STR_LEN, 
		   "bindVar1=%s", bindVar1);
	  rodsLogSql(tmpStr);
	  if (stat != SQL_SUCCESS) {
	     rodsLog(LOG_ERROR, 
		     "cllExecSqlNoResult: SQLBindParameter failed: %d", stat);
	     return(-1);
	 }
      }
      if (bindVar2 != 0 && *bindVar2 != '\0') {
	 stat = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR,
				 SQL_C_CHAR, 0, 0, bindVar2, 0, 0);
	 snprintf(tmpStr, TMP_STR_LEN, 
		  "bindVar2=%s", bindVar2);
	 rodsLogSql(tmpStr);
	 if (stat != SQL_SUCCESS) {
	    rodsLog(LOG_ERROR, 
		    "cllExecSqlNoResult: SQLBindParameter failed: %d", stat);
	    return(-1);
	 }
      }
      if (bindVar3 != 0 && *bindVar3 != '\0') {
	 stat = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR,
				 SQL_C_CHAR, 0, 0, bindVar3, 0, 0);
	 snprintf(tmpStr, TMP_STR_LEN, 
		   "bindVar3=%s", bindVar3);
	 rodsLogSql(tmpStr);
	 if (stat != SQL_SUCCESS) {
	    rodsLog(LOG_ERROR, "cllExecSqlNoResult: SQLBindParameter failed: %d",
		    stat);
	    return(-1);
	 }
      }
      if (bindVar4 != 0 && *bindVar4 != '\0') {
	 stat = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR,
				 SQL_C_CHAR, 0, 0, bindVar4, 0, 0);
	 snprintf(tmpStr, TMP_STR_LEN, 
		   "bindVar4=%s", bindVar4);
	 rodsLogSql(tmpStr);
	 if (stat != SQL_SUCCESS) {
	    rodsLog(LOG_ERROR, "cllExecSqlNoResult: SQLBindParameter failed: %d",
		    stat);
	    return(-1);
	 }
      }
      rodsLogSql(sql);
      stat = SQLExecute(hstmt);
   }
   else {
      rodsLogSql(sql);
      stat = SQLExecDirect(hstmt, (unsigned char *)sql, SQL_NTS);
   }

   status = "UNKNOWN";
   if (stat == SQL_SUCCESS) status= "SUCCESS";
   if (stat == SQL_SUCCESS_WITH_INFO) status="SUCCESS_WITH_INFO";
   if (stat == SQL_NO_DATA_FOUND) status="NO_DATA";
   if (stat == SQL_ERROR) status="SQL_ERROR";
   if (stat == SQL_INVALID_HANDLE) status="HANDLE_ERROR";
   rodsLogSqlResult(status);

   if (stat == SQL_SUCCESS ||
       stat == SQL_SUCCESS_WITH_INFO || 
       stat == SQL_NO_DATA_FOUND) {
   }
   else {
      logBindVars(LOG_NOTICE, bindVar1, bindVar2, bindVar3, bindVar4,
		  bindVar5, bindVar6);
      rodsLog(LOG_NOTICE, 
	      "cllExecSqlWithResultBV: SQLExecDirect error: %d, sql:%s",
	      stat, sql);
      logPsgError(LOG_NOTICE, icss->environPtr, myHdbc, hstmt);
      return(-1);
   }

   stat =  SQLNumResultCols(hstmt, &numColumns);
   if (stat != SQL_SUCCESS) {
      rodsLog(LOG_ERROR, "cllExecSqlWithResultBV: SQLNumResultCols failed: %d",
	      stat);
      return(-2);
   }
   myStatement->numOfCols=numColumns;

   for (i = 0; i<numColumns; i++) {
      stat = SQLDescribeCol(hstmt, i+1, colName, sizeof(colName),
			    &colNameLen, &colType, &precision, &scale, NULL);
      if (stat != SQL_SUCCESS) {
	 rodsLog(LOG_ERROR, "cllExecSqlWithResultBV: SQLDescribeCol failed: %d",
	      stat);
	 return(-3);
      }
      /*  printf("colName='%s' precision=%d\n",colName, precision); */
      columnLength[i]=precision;
      stat = SQLColAttributes(hstmt, i+1, SQL_COLUMN_DISPLAY_SIZE, 
			      NULL, 0, NULL, &displaysize);
      if (stat != SQL_SUCCESS) {
	 rodsLog(LOG_ERROR, 
		 "cllExecSqlWithResultBV: SQLColAttributes failed: %d",
		 stat);
	 return(-3);
      }

      if (displaysize > (strlen((char *) colName))) {
	 columnLength[i] = displaysize + 1;
      }
      else {
	 columnLength[i] = strlen((char *) colName) + 1;
      }
      /*      printf("columnLength[%d]=%d\n",i,columnLength[i]); */
 
      myStatement->resultValue[i] = malloc((int)columnLength[i]);

      strcpy((char *)myStatement->resultValue[i],"");

#ifdef NEW_ODBC
      stat = SQLBindCol(hstmt, i+1, SQL_C_CHAR, myStatement->resultValue[i], 
			columnLength[i], NULL);
      /* The last argument could be resultDataSize (a SQLINTEGER
         location), which will be returned later via the SQLFetch.
         Since unused now, passing in NULL tells ODBC to skip it */
#else
      /* The old ODBC needs a non-NULL value */
      stat = SQLBindCol(hstmt, i+1, SQL_C_CHAR, myStatement->resultValue[i], 
			columnLength[i], &resultDataSize);
#endif

      if (stat != SQL_SUCCESS) {
	 rodsLog(LOG_ERROR, 
		 "cllExecSqlWithResultBV: SQLColAttributes failed: %d",
		 stat);
	 return(-4);
      }


      myStatement->resultColName[i] = malloc((int)columnLength[i]);
      strncpy(myStatement->resultColName[i], (char *)colName, columnLength[i]);

   }
   *stmtNum = statementNumber;
   return(0);
}

/*
  Return a row from a previous cllExecSqlWithResult call.
 */
int
cllGetRow(icatSessionStruct *icss, int statementNumber) {
   HSTMT hstmt;
   RETCODE stat;
   int nCols, i;
   icatStmtStrct *myStatement;

   myStatement=icss->stmtPtr[statementNumber];
   hstmt = myStatement->stmtPtr;
   nCols = myStatement->numOfCols;

   for (i=0;i<nCols;i++) {
      strcpy((char *)myStatement->resultValue[i],"");
   }
   stat =  SQLFetch(hstmt);
   if (stat != SQL_SUCCESS && stat != SQL_NO_DATA_FOUND) {
      rodsLog(LOG_ERROR, "cllGetRow: SQLFetch failed: %d", stat);
      return(-1);
   }
   if (stat == SQL_NO_DATA_FOUND) {
      _cllFreeStatementColumns(icss,statementNumber);
      myStatement->numOfCols=0;
   }
   return(0);
}

/* 
  Return the string needed to get the next value in a sequence item.
  The syntax varies between RDBMSes, so it is here, in the DBMS-specific code.
 */
int
cllNextValueString(char *itemName, char *outString, int maxSize) {
   snprintf(outString, maxSize, "nextval('%s')", itemName);
   return 0;
}

int
cllGetRowCount(icatSessionStruct *icss, int statementNumber) {
   int i;
   HSTMT hstmt;
   icatStmtStrct *myStatement;
   SQLINTEGER   RowCount;

   myStatement=icss->stmtPtr[statementNumber];
   hstmt = myStatement->stmtPtr;

   i = SQLRowCount (hstmt, (SQLINTEGER *)&RowCount);
   if (i) return(i);
   return(RowCount);
}

int
cllCurrentValueString(char *itemName, char *outString, int maxSize) {
   snprintf(outString, maxSize, "currval('%s')", itemName);
   return 0;
}

/* 
  Free a statement (from a previous cllExecSqlWithResult call) and the
  corresponding resultValue array.
*/
int
cllFreeStatement(icatSessionStruct *icss, int statementNumber) {
   HSTMT hstmt;
   RETCODE stat;
   int i;

   icatStmtStrct *myStatement;

   myStatement=icss->stmtPtr[statementNumber];
   if (myStatement==NULL) { /* already freed */
      return(0);
   }
   hstmt = myStatement->stmtPtr;

   for (i=0;i<myStatement->numOfCols;i++) {
      free(myStatement->resultValue[i]);
      free(myStatement->resultColName[i]);
   }

   stat = SQLFreeStmt(hstmt, SQL_DROP);
   if (stat != SQL_SUCCESS) {
      rodsLog(LOG_ERROR, "cllFreeStatement SQLFreeStmt error: %d", stat);
   }

   free(myStatement);

   icss->stmtPtr[statementNumber]=0;  /* indicate that the statement is free */

   return (0);
}

/* 
  Free the statement columns (from a previous cllExecSqlWithResult call),
  but not the whole statement.
*/
int
_cllFreeStatementColumns(icatSessionStruct *icss, int statementNumber) {
   HSTMT hstmt;
   int i;

   icatStmtStrct *myStatement;

   myStatement=icss->stmtPtr[statementNumber];
   hstmt = myStatement->stmtPtr;

   for (i=0;i<myStatement->numOfCols;i++) {
      free(myStatement->resultValue[i]);
      free(myStatement->resultColName[i]);
   }
   return (0);
}

/*
 A few tests to verify basic functionality (including talking with
 the database via ODBC). 
 */
int cllTest(char *userArg, char *pwArg) {
   int i;
   int j, k;
   int OK;
   int stmt;
   int numOfCols;
   char userName[500];
   int ival;

   struct passwd *ppasswd;
   icatSessionStruct icss;

   icss.stmtPtr[0]=0;
   rodsLogSqlReq(1);
   OK=1;
   i = cllOpenEnv(&icss);
   if (i != 0) OK=0;

   if (userArg==0 || *userArg=='\0') {
      ppasswd = getpwuid(getuid());    /* get user passwd entry             */
      strcpy(userName,ppasswd->pw_name);  /* get user name                  */
   }
   else {
      strncpy(userName, userArg, 500);
   }
   printf("userName=%s\n",userName);
   printf("password=%s\n",pwArg);

   icss.databaseUsername=userName;
   if (pwArg==0 || *pwArg=='\0') {
      icss.databasePassword="";
   }
   else {
      icss.databasePassword=pwArg;
   }

   i = cllConnect(&icss);
   if (i != 0) exit(-1);

   i = cllExecSqlNoResult(&icss,"create table test (i integer, j integer, a varchar(32))");
   if (i != 0 && i != CAT_SUCCESS_BUT_WITH_NO_INFO) OK=0;

   i = cllExecSqlNoResult(&icss, "insert into test values (2, 3, 'a')");
   if (i != 0) OK=0;

   i = cllExecSqlNoResult(&icss, "commit");
   if (i != 0) OK=0;

   i = cllExecSqlNoResult(&icss, "bad sql");
   if (i == 0) OK=0;   /* should fail, if not it's not OK */

   i = cllExecSqlNoResult(&icss, "delete from test where i = 1");
   if (i != 0 && i != CAT_SUCCESS_BUT_WITH_NO_INFO) OK=0;

   i = cllExecSqlNoResult(&icss, "commit");
   if (i != 0) OK=0;

   i = cllExecSqlWithResultBV(&icss, &stmt, 
				"select * from test where a = ?",
				"a",0 ,0,0,0,0);
   if (i != 0) OK=0;

   if (i == 0) {
      numOfCols = 1;
      for (j=0;j<10 && numOfCols>0;j++) {
	 i = cllGetRow(&icss, stmt);
	 if (i != 0) {
	    OK=0;
	 }
	 else {
	    numOfCols = icss.stmtPtr[stmt]->numOfCols;
	    if (numOfCols == 0) {
	       printf("No more rows returned\n");
	       i = cllFreeStatement(&icss,stmt);
	    }
	    else {
	       for (k=0; k<numOfCols || k < icss.stmtPtr[stmt]->numOfCols; k++){
		  printf("resultValue[%d]=%s\n",k,
			 icss.stmtPtr[stmt]->resultValue[k]);
	       }
	    }
	 }
      }
   }

   ival=2;
   i = cllExecSqlWithResultBV(&icss, &stmt, 
				"select * from test where i = ?",
				"2",0,0,0,0,0);
   if (i != 0) OK=0;

   if (i == 0) {
      numOfCols = 1;
      for (j=0;j<10 && numOfCols>0;j++) {
	 i = cllGetRow(&icss, stmt);
	 if (i != 0) {
	    OK=0;
	 }
	 else {
	    numOfCols = icss.stmtPtr[stmt]->numOfCols;
	    if (numOfCols == 0) {
	       printf("No more rows returned\n");
	       i = cllFreeStatement(&icss,stmt);
	    }
	    else {
	       for (k=0; k<numOfCols || k < icss.stmtPtr[stmt]->numOfCols; k++){
		  printf("resultValue[%d]=%s\n",k,
			 icss.stmtPtr[stmt]->resultValue[k]);
	       }
	    }
	 }
      }
   }

   i = cllExecSqlNoResult(&icss,"drop table test;");
   if (i != 0 && i != CAT_SUCCESS_BUT_WITH_NO_INFO) OK=0;

   i = cllExecSqlNoResultBV(&icss, "commit",0,0,0);
   if (i != 0) OK=0;

   i = cllDisconnect(&icss);
   if (i != 0) OK=0;

   i = cllCloseEnv(&icss);
   if (i != 0) OK=0;

   if (OK) {
      printf("The tests all completed normally\n");
      return(0);
   }
   else {
      printf("One or more tests DID NOT complete normally\n");
      return(-1);
   }
}
