/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/**************************************************************************

  This file contains the DBO (Data-base objects) high level functions.
  These constitute the API between the rest of iRODS and the DBO
  database.

  See the 'Database Resources' page on the irods web site for more
  information.

  These routines, like the icatHighLevelRoutines, layer on top of
  either icatLowLevelPostgres or icatLowLevelOracle.  DBO is not ICAT,
  but they both use the shared low level interface to the databases.
  DBO can be built as part of either an ICAT-enabled server or a
  non-ICAT-Enabled server.

**************************************************************************/

#include "rods.h"
#include "rcMisc.h"

#include "dboHighLevelRoutines.h"
#include "icatLowLevel.h"

#include "dataObjOpen.h"
#include "dataObjRead.h"
#include "dataObjClose.h"

#define DBR_CONFIG_FILE "dbr.config"
/*#define DBO_ACCESS_ATTRIBUTE "DBO_ACCESS" */
#define DBR_ODBC_ENTRY_PREFIX "IRODS_DBR_"
#define MAX_ODBC_ENTRY_NAME 100

#define BUF_LEN 500
#define MAX_SQL 4000

#define MAX_DBO_NAME_LEN 200
#define BIG_STR 200

#define MAX_SESSIONS 10
static char openDbrName[MAX_SESSIONS][MAX_DBO_NAME_LEN+2]={"","","","","","","","","",""};

int dboLogSQL=0;
int readDboConfig(char *dbrname, char **DBUser, char**DBPasswd);

icatSessionStruct dbo_icss[MAX_SESSIONS]={{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};

int dboDebug(char *debugMode) {
   if (strstr(debugMode, "SQL")) {
      dboLogSQL=1;
   }
   else {
      dboLogSQL=0;
   }
   return(0);
}

int
getOpenDbrIndex(char *dbrName) {
   int ix, i;
   ix=-1;
   for (i=0;i<MAX_SESSIONS;i++) {
      if (strcmp(openDbrName[i], dbrName)==0) {
	 ix = i;
	 break;
      }
   }
   return(ix);
}


int dbrOpen(char *dbrName) {
#if defined(DBO) 
   int i;
   int status;
   char *DBUser;
   char *DBPasswd;
   int icss_index;
   char odbcEntryName[MAX_ODBC_ENTRY_NAME+10];

   i = getOpenDbrIndex(dbrName);
   if (i>=0) return(DBR_ALREADY_OPEN);

   icss_index = -1;
   for (i=0;i<MAX_SESSIONS;i++) {
      if (dbo_icss[i].status==0) {
	 icss_index=i;
	 break;
      }
   }
   if (icss_index==-1) return (DBR_MAX_SESSIONS_REACHED);

   status =  readDboConfig(dbrName, &DBUser, &DBPasswd);
   if (status) return(status);

   rodsLog(LOG_NOTICE, "dbrOpen DBUser %s",DBUser);
   rodsLog(LOG_NOTICE, "dbrOpen DBPasswd %s",DBPasswd);

   dbo_icss[icss_index].databaseUsername = DBUser;
   dbo_icss[icss_index].databasePassword = DBPasswd;

   /* Initialize the dbo_icss statement pointers */
   for (i=0; i<MAX_NUM_OF_CONCURRENT_STMTS; i++) {
      dbo_icss[icss_index].stmtPtr[i]=0;
   }

   /* Open Environment */
   i = cllOpenEnv(&dbo_icss[icss_index]);
   if (i != 0) {
      rodsLog(LOG_NOTICE, "dbrOpen cllOpen failure %d",i);
      return(DBO_ENV_ERR);
   }

   /* Connect to the DBMS */
   strncpy((char *)&odbcEntryName, DBR_ODBC_ENTRY_PREFIX, 
	   MAX_ODBC_ENTRY_NAME);
   strncat((char *)&odbcEntryName, dbrName,
	   MAX_ODBC_ENTRY_NAME);
   i = cllConnectDbo(&dbo_icss[icss_index], odbcEntryName);
   if (i != 0) {
      rodsLog(LOG_NOTICE, "dbrOpen cllConnectDbo failure %d",i);
      return(DBO_CONNECT_ERR);
   }

   dbo_icss[icss_index].status=1;
   strncpy(openDbrName[icss_index], dbrName, MAX_DBO_NAME_LEN);

   return(icss_index);
#else
   openDbrName[0][0]='\0'; /* avoid warning */
   return(DBO_NOT_COMPILED_IN);
#endif
}


int _dbrClose(int icss_index) {
#if defined(DBO) 
   int status, stat2;

   status = cllDisconnect(&dbo_icss[icss_index]);

   stat2 = cllCloseEnv(&dbo_icss[icss_index]);

   openDbrName[icss_index][0]='\0';

   if (status) {
      return(DBO_DISCONNECT_ERR);
   }
   if (stat2) {
      return(DBO_CLOSE_ENV_ERR);
   }
   dbo_icss[icss_index].status=0;
   openDbrName[icss_index][0] = '\0';
   return(0);
#else
   return(DBO_NOT_COMPILED_IN);
#endif
}

int dbrClose(char *dbrName) {
#if defined(DBO) 
   int i;
   for (i=0;i<MAX_SESSIONS;i++) {
      if (strcmp(openDbrName[i], dbrName)==0) {
	 return(_dbrClose(i));
      }
   }
   return(DBR_NOT_OPEN);
#else
   return(DBO_NOT_COMPILED_IN);
#endif
}

int dbrCommit(rsComm_t *rsComm, char *dbrName) {
#if defined(DBO) 
   int status, ix;

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   ix = getOpenDbrIndex(dbrName);

   if (ix<0) return(DBR_NOT_OPEN);

   status = cllExecSqlNoResult(&dbo_icss[ix], "commit");
   return(status);
#else
   return(DBO_NOT_COMPILED_IN);
#endif
}

int dbrRollback(rsComm_t *rsComm, char *dbrName) {
#if defined(DBO) 
   int status, ix;

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   ix = getOpenDbrIndex(dbrName);

   if (ix<0) return(DBR_NOT_OPEN);

   status = cllExecSqlNoResult(&dbo_icss[ix], "rollback");
   return(status);
#else
   return(DBO_NOT_COMPILED_IN);
#endif
}

int dboIsConnected(int icss_index) {
#if defined(DBO) 
   if (dboLogSQL) rodsLog(LOG_SQL, "dboIsConnected");
   return(dbo_icss[icss_index].status);
#else
   return(DBO_NOT_COMPILED_IN);
#endif
}

/*
  Check to see if the client user has access to this DBO
  by querying the user AVU.
 */
int dboCheckAccess(char *dboName, rsComm_t *rsComm) {
#if defined(DBO_NOTYET) 
   genQueryInp_t genQueryInp;
   genQueryOut_t genQueryOut;
   int iAttr[10];
   int iAttrVal[10]={0,0,0,0,0};
   int iCond[10];
   char *condVal[10];
   char v1[BIG_STR];
   char v2[BIG_STR];
   char v3[BIG_STR];
   int status;

   memset (&genQueryInp, 0, sizeof (genQueryInp_t));

   iAttr[0]=COL_META_USER_ATTR_VALUE;
   genQueryInp.selectInp.inx = iAttr;
   genQueryInp.selectInp.value = iAttrVal;
   genQueryInp.selectInp.len = 1;

   iCond[0]=COL_USER_NAME;
   sprintf(v1,"='%s'", rsComm->clientUser.userName);
   condVal[0]=v1;

   iCond[1]=COL_META_USER_ATTR_NAME;
   sprintf(v2,"='%s'", DBO_ACCESS_ATTRIBUTE);
   condVal[1]=v2;

   iCond[2]=COL_META_USER_ATTR_VALUE;
   sprintf(v3,"='%s'", dboName);
   condVal[2]=v3;

   genQueryInp.sqlCondInp.inx = iCond;
   genQueryInp.sqlCondInp.value = condVal;
   genQueryInp.sqlCondInp.len=3;

   genQueryInp.maxRows=10;
   genQueryInp.continueInx=0;
   genQueryInp.condInput.len=0;
   status = chlGenQuery(genQueryInp, &genQueryOut);
   if (status == CAT_NO_ROWS_FOUND) {
      return(DBO_ACCESS_PROHIBITED);
   }

   return (status);  /* any error, no access */
#else
   return(DBO_NOT_COMPILED_IN);
#endif
}


int dboSqlNoResults(char *sql, char *parm[], int nParms) {
#if defined(DBO_NOTYET) 
   int i;
   for (i=0;i<nParms;i++) {
      cllBindVars[i]=parm[i];
   }
   cllBindVarCount=nParms;
   i = cllExecSqlNoResult(&dbo_icss, sql);
   /*   if (i <= CAT_ENV_ERR) return(i); ? already an iRODS error code */
   printf("i=%d\n",i);
   if (i==CAT_SUCCESS_BUT_WITH_NO_INFO) return(0);
   if (i) return(DBO_SQL_ERR);
   return(0);
#else
   return(DBO_NOT_COMPILED_IN);
#endif
}

int dboSqlWithResults(int fd, char *sql, char *parm[], int nParms, 
		      char *outBuf, int maxOutBuf) {
#if defined(DBO) 
   int i, ii;
   int statement;
   int rowCount, nCols;

   for (i=0;i<nParms;i++) {
      cllBindVars[i]=parm[i];
   }
   cllBindVarCount=nParms;
   i = cllExecSqlWithResult(&dbo_icss[fd], &statement, sql);
   if (i==CAT_SUCCESS_BUT_WITH_NO_INFO) return(CAT_SUCCESS_BUT_WITH_NO_INFO);
   if (i) {
      cllGetLastErrorMessage(outBuf, maxOutBuf);
      if (i <= CAT_ENV_ERR) return(i); /* already an iRODS error code */
      return (CAT_SQL_ERR);
   }

   for (rowCount=0;;rowCount++) {
      i = cllGetRow(&dbo_icss[fd], statement);
      if (i != 0)  {
	 ii = cllFreeStatement(&dbo_icss[fd], statement);
	 if (rowCount==0) return(CAT_GET_ROW_ERR);
	 return(0);
      }

      if (dbo_icss[fd].stmtPtr[statement]->numOfCols == 0) {
	 i = cllFreeStatement(&dbo_icss[fd],statement);
	 if (rowCount==0) return(CAT_NO_ROWS_FOUND);
	 return(0);
      }

      nCols = dbo_icss[fd].stmtPtr[statement]->numOfCols;
      if (rowCount==0) {
	 for (i=0; i<nCols ; i++ ) {
	    rstrcat(outBuf, dbo_icss[fd].stmtPtr[statement]->resultColName[i],
		    maxOutBuf);
	    rstrcat(outBuf, "|", maxOutBuf);
	 }
	 rstrcat(outBuf, "\n", maxOutBuf);
      }
      for (i=0; i<nCols ; i++ ) {
	 rstrcat(outBuf, dbo_icss[fd].stmtPtr[statement]->resultValue[i], 
		 maxOutBuf);
	 rstrcat(outBuf, "|", maxOutBuf);
      }
      rstrcat(outBuf, "\n", maxOutBuf);
   }

   return(0);  /* never reached */
#else
   return(DBO_NOT_COMPILED_IN);
#endif
}

char *
getDboConfigDir()
{
    char *myDir;

    if ((myDir = (char *) getenv("irodsConfigDir")) != (char *) NULL) {
        return (myDir);
    }
    return (DEF_CONFIG_DIR);
}

int 
readDboConfig(char *dboName, char **DBUser, char**DBPasswd) {
   FILE *fptr;
   char buf[BUF_LEN];
   char *fchar;
   char *key;
   char *dboConfigFile;
   static char foundLine[BUF_LEN];

   dboConfigFile =  (char *) malloc((strlen (getDboConfigDir()) +
				    strlen(DBR_CONFIG_FILE) + 24));

   sprintf (dboConfigFile, "%s/%s", getDboConfigDir(), 
	    DBR_CONFIG_FILE);

   fptr = fopen (dboConfigFile, "r");

   if (fptr == NULL) {
      rodsLog (LOG_NOTICE, 
	       "Cannot open DBR_CONFIG_FILE file %s. errno = %d\n",
          dboConfigFile, errno);
      free (dboConfigFile);
      return (DBR_CONFIG_FILE_ERR);
   }
   free (dboConfigFile);

   buf[BUF_LEN-1]='\0';
   fchar = fgets(buf, BUF_LEN-1, fptr);
   for(;fchar!='\0';) {
      if (buf[0]=='#' || buf[0]=='/') {
	 buf[0]='\0'; /* Comment line, ignore */
      }
      key=strstr(buf, dboName);
      if (key == buf) {
	 int state, i;
	 char *DBKey=0;
	 rstrcpy(foundLine, buf, BUF_LEN);
	 state=0;
	 for (i=0;i<BUF_LEN;i++) {
	    if (foundLine[i]==' ' || foundLine[i]=='\n') {
	       int endOfLine;
	       endOfLine=0;
	       if (foundLine[i]=='\n') endOfLine=1;
	       foundLine[i]='\0';
	       if (endOfLine && state<6) return(0);
	       if (state==0) state=1;
	       if (state==2) state=3;
	       if (state==4) state=5;
	       if (state==6) {
		  static char unscrambledPw[NAME_LEN];
		  obfDecodeByKey(*DBPasswd, DBKey, unscrambledPw);
		  *DBPasswd=unscrambledPw;
		  return(0);
	       }
	    }
	    else {
	       if (state==1) {
		  state=2;
		  *DBUser=&foundLine[i];
	       }
	       if (state==3) {
		  state=4;
		  *DBPasswd=&foundLine[i];
	       }
	       if (state==5) {
		  state=6;
		  DBKey=&foundLine[i];
	       }
	    }
	 }
      }
      fchar = fgets(buf, BUF_LEN-1, fptr);
   }
   fclose (fptr);

   return(DBR_NAME_NOT_FOUND);
}

/*
Read the config file and return a list of the defined DBOs.
 */
int 
dboReadConfigItems(char *dboList, int maxSize) {
   FILE *fptr;
   char buf[BUF_LEN];
   char *fchar;
   char *dboConfigFile;
   static char foundLine[BUF_LEN];

   dboConfigFile =  (char *) malloc((strlen (getDboConfigDir()) +
				    strlen(DBR_CONFIG_FILE) + 24));

   sprintf (dboConfigFile, "%s/%s", getDboConfigDir(), 
	    DBR_CONFIG_FILE);

   fptr = fopen (dboConfigFile, "r");

   if (fptr == NULL) {
      rodsLog (LOG_NOTICE, 
	       "Cannot open DBR_CONFIG_FILE file %s. errno = %d\n",
          dboConfigFile, errno);
      free (dboConfigFile);
      return (DBR_CONFIG_FILE_ERR);
   }
   free (dboConfigFile);

   dboList[0]='\0';
   foundLine[0]='\0';
   buf[BUF_LEN-1]='\0';
   fchar = fgets(buf, BUF_LEN-1, fptr);
   for(;fchar!='\0';) {
      int state, i;
      if (buf[0]=='#' || buf[0]=='/') {
	 buf[0]='\0'; /* Comment line, ignore */
      }
      rstrcpy(foundLine, buf, BUF_LEN);
      state=0;
      for (i=0;i<BUF_LEN;i++) {
	 if (foundLine[i]==' ') {
	    foundLine[i]='\0';
	    if(dboList[0]!='\0') rstrcat(dboList," ", maxSize);
	    rstrcat(dboList,foundLine, maxSize);
	    break;
	 }
      }
      fchar = fgets(buf, BUF_LEN-1, fptr);
   }
   fclose (fptr);

   return(0);
}
int
dboGetInfo(int fd, char *outBuf, int maxOutBuf) {
   int status;
#ifdef ORA_DBO
   /* Oracle vesion */
   char tableSql[]="select TABLE_NAME from tabs";
#else
   /* Postgres sql that returns table list, like '\d' in psql.  
      Found on the web.
   */
   char tableSql[]="SELECT n.nspname as \"Schema\", c.relname as \"Name\", CASE c.relkind WHEN 'r' THEN 'table' WHEN 'v' THEN 'view' WHEN 'i' THEN 'index' WHEN 'S' THEN 'sequence' WHEN 's' THEN 'special' END as \"Type\", u.usename as \"Owner\" FROM pg_catalog.pg_class c LEFT JOIN pg_catalog.pg_user u ON u.usesysid = c.relowner LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace WHERE c.relkind IN ('r','') AND n.nspname NOT IN ('pg_catalog', 'pg_toast') AND pg_catalog.pg_table_is_visible(c.oid) ORDER BY 1,2";
#endif

   if (fd>MAX_SESSIONS || fd< 0 || dbo_icss[fd].status!=1) {
      strcpy(outBuf, "DBR is not open");
      return(DBO_INVALID_OBJECT_DESCRIPTOR);
   }

   status =  dboSqlWithResults(fd, tableSql, NULL, 0, 
			       outBuf, maxOutBuf);
   return(status);
}

int
getDboSql( rsComm_t *rsComm, char *fullName, char *dboSQL) {
   openedDataObjInp_t dataObjReadInp;
   openedDataObjInp_t dataObjCloseInp;
   bytesBuf_t *readBuf;
   int status;
   int objID;
   char *cp1;
   int bytesRead;
   dataObjInp_t dataObjInp;

   memset (&dataObjInp, 0, sizeof(dataObjInp_t));
   strncpy(dataObjInp.objPath, fullName, MAX_NAME_LEN);

   if ((objID=rsDataObjOpen(rsComm, &dataObjInp)) < 0) {
      if (objID == CAT_NO_ROWS_FOUND) return (DBO_DOES_NOT_EXIST);
      return (objID);
   }

   /* read buffer init */
   readBuf = (bytesBuf_t *)malloc(sizeof(bytesBuf_t));
   readBuf->len = 5*1024;	        /* just 5K should do it */
   readBuf->buf = (char *)malloc(readBuf->len);
   memset (readBuf->buf, '\0', readBuf->len);

  /* read SQL data */
   memset (&dataObjReadInp, 0, sizeof (dataObjReadInp));
   dataObjReadInp.l1descInx = objID;
   dataObjReadInp.len = readBuf->len;

   bytesRead = rsDataObjRead (rsComm, &dataObjReadInp, readBuf);
   if (bytesRead < 0) return(bytesRead);
   
   cp1 = readBuf->buf;
   while (*cp1 == '!' || *cp1 == '#') {
      cp1++;
      while (*cp1 != '\n') cp1++;
      cp1++;
   }
   strncpy(dboSQL, cp1, MAX_SQL);

   memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
   dataObjCloseInp.l1descInx = objID;
	
   status = rsDataObjClose (rsComm, &dataObjCloseInp);
   if (status) return(status);

   return(0);
}

int
dboExecute(rsComm_t *rsComm, char *dbrName, char *dboName, char *outBuf,
	   int maxOutBuf) {
   int status;
   char dboSQL[MAX_SQL];
   int i, ix;
   int didOpen=0;

   if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
   }

   ix=getOpenDbrIndex(dbrName);

   if (ix < 0) {
      ix = dbrOpen(dbrName);
      if (ix) return(ix);
      didOpen=1;
   }

   status = getDboSql(rsComm, dboName, dboSQL);
   if (status) return(status);

   if (dboLogSQL) rodsLog(LOG_SQL, "dboExecute SQL: %s", dboSQL);

   if (status) return(status);

   status =  dboSqlWithResults(ix, dboSQL, NULL, 0, 
			       outBuf, maxOutBuf);

   if (didOpen) {  /* DBR was not originally open */
      i = dbrClose(dbrName);
      if (i) return(i);
   }
   return(0);
}

