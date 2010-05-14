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

//#include "icatHighLevelRoutines.h"

#define DBO_CONFIG_FILE "dbo.config"
//#define DBO_ACCESS_ATTRIBUTE "DBO_ACCESS"
#define DBO_ODBC_ENTRY_PREFIX "IRODS_DBO_"
#define MAX_ODBC_ENTRY_NAME 100

#define BUF_LEN 500

#define MAX_DBO_NAME_LEN 200
#define BIG_STR 200

#define RESULT_BUF1_SIZE 2000

#define MAX_SESSIONS 10
static char openDboName[MAX_DBO_NAME_LEN+2]="";

int dboLogSQL=0;
int readDboConfig(char *dboName, char **DBUser, char**DBPasswd);


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



int dboOpen(char *dboName) {
#if defined(DBO) 
   int i;
   int status;
   char *DBUser;
   char *DBPasswd;
   int icss_index;
   char odbcEntryName[MAX_ODBC_ENTRY_NAME+10];

   if (dboLogSQL) rodsLog(LOG_SQL, "dboOpen");

#if 0
   if (strcmp(openDboName, dboName)==0) {
      return(0); /* already open */
   }
#endif
   icss_index = -1;
   for (i=0;i<MAX_SESSIONS;i++) {
      if (dbo_icss[i].status==0) {
	 icss_index=i;
	 break;
      }
   }
   if (icss_index==-1) return (DBO_MAX_SESSIONS_REACHED);

   status =  readDboConfig(dboName, &DBUser, &DBPasswd);
   if (status) return(status);

   rodsLog(LOG_NOTICE, "dboOpen DBUser %s",DBUser);
   rodsLog(LOG_NOTICE, "dboOpen DBPasswd %s",DBPasswd);

   dbo_icss[icss_index].databaseUsername = DBUser;
   dbo_icss[icss_index].databasePassword = DBPasswd;

   /* Initialize the dbo_icss statement pointers */
   for (i=0; i<MAX_NUM_OF_CONCURRENT_STMTS; i++) {
      dbo_icss[icss_index].stmtPtr[i]=0;
   }

   /* Open Environment */
   i = cllOpenEnv(&dbo_icss[icss_index]);
   if (i != 0) {
      rodsLog(LOG_NOTICE, "dboOpen cllOpen failure %d",i);
      return(DBO_ENV_ERR);
   }

   /* Connect to the DBMS */
   strncpy((char *)&odbcEntryName, DBO_ODBC_ENTRY_PREFIX, 
	   MAX_ODBC_ENTRY_NAME);
   strncat((char *)&odbcEntryName, dboName,
	   MAX_ODBC_ENTRY_NAME);
   i = cllConnectDbo(&dbo_icss[icss_index], odbcEntryName);
   if (i != 0) {
      rodsLog(LOG_NOTICE, "dboOpen cllConnectDbo failure %d",i);
      return(DBO_CONNECT_ERR);
   }

   dbo_icss[icss_index].status=1;
   strncpy(openDboName, dboName, MAX_DBO_NAME_LEN);

   return(0);
#else
   openDboName[0]='\0'; /* avoid warning */
   return(DBO_NOT_COMPILED_IN);
#endif
}

int dboClose(int icss_index) {
#if defined(DBO) 
   int status, stat2;

   status = cllDisconnect(&dbo_icss[icss_index]);

   stat2 = cllCloseEnv(&dbo_icss[icss_index]);

   openDboName[0]='\0';

   if (status) {
      return(DBO_DISCONNECT_ERR);
   }
   if (stat2) {
      return(DBO_CLOSE_ENV_ERR);
   }
   dbo_icss[icss_index].status=0;
   return(0);
#else
   return(DBO_NOT_COMPILED_IN);
#endif
}

int dboCommit(int icss_index) {
#if defined(DBO) 
   int status;

   status = cllExecSqlNoResult(&dbo_icss[icss_index], "commit");
   return(status);
#else
   return(DBO_NOT_COMPILED_IN);
#endif
}

int dboRollback(int icss_index) {
#if defined(DBO) 
   int status;

   status = cllExecSqlNoResult(&dbo_icss[icss_index], "rollback");
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

int dboSqlWithResults(char *sql, char *parm[], int nParms, char **outBuf) {
#if defined(DBO_NOTYET) 
   int i, ii;
   int statement;
   int rowCount, nCols, rowSize=0;
   int maxOutBuf;
   char *myBuf, *pbuf;
   static char resultBuf1[RESULT_BUF1_SIZE+10];
   int totalRows, toMalloc;

   for (i=0;i<nParms;i++) {
      cllBindVars[i]=parm[i];
   }
   cllBindVarCount=nParms;
   i = cllExecSqlWithResult(&dbo_icss, &statement, sql);
   printf("i=%d\n",i);
   /* ?   if (i==CAT_SUCCESS_BUT_WITH_NO_INFO) return(0); */
   if (i) return(DBO_SQL_ERR);

   myBuf=resultBuf1; /* Initially, use this buffer */
   maxOutBuf=RESULT_BUF1_SIZE;
   memset(myBuf, 0, maxOutBuf);

   for (rowCount=0;;rowCount++) {
      i = cllGetRow(&dbo_icss, statement);
      if (i != 0)  {
	 ii = cllFreeStatement(&dbo_icss, statement);
	 if (rowCount==0) return(CAT_GET_ROW_ERR);
	 *outBuf=myBuf;
	 return(0);
      }

      if (dbo_icss.stmtPtr[statement]->numOfCols == 0) {
	 i = cllFreeStatement(&dbo_icss,statement);
	 if (rowCount==0) return(CAT_NO_ROWS_FOUND);
	 *outBuf=myBuf;
	 return(0);
      }

      nCols = dbo_icss.stmtPtr[statement]->numOfCols;
      if (rowCount==0) {
	 for (i=0; i<nCols ; i++ ) {
	    rstrcat(myBuf, dbo_icss.stmtPtr[statement]->resultColName[i],
		    maxOutBuf);
	    rstrcat(myBuf, "|", maxOutBuf);
	 }
	 rstrcat(myBuf, "\n", maxOutBuf);
      }
      for (i=0; i<nCols ; i++ ) {
	 rstrcat(myBuf, dbo_icss.stmtPtr[statement]->resultValue[i], 
		 maxOutBuf);
	 rstrcat(myBuf, "|", maxOutBuf);
      }
      rstrcat(myBuf, "\n", maxOutBuf);
      if (rowSize==0) {
	 rowSize=strlen(myBuf);
	 totalRows=cllGetRowCount(&dbo_icss, statement);
	 printf("rowSize=%d, totalRows=%d\n",rowSize, totalRows);
	 if (totalRows < 0) return(totalRows);
	 if (totalRows == 0) {
	    /* Unknown number of rows available (Oracle) */
	    totalRows=10; /* to start with */
	 }
	 toMalloc=((totalRows+1)*rowSize)*3;
	 myBuf=malloc(toMalloc);
	 if (myBuf <=0) return(SYS_MALLOC_ERR);
	 maxOutBuf=toMalloc;
	 memset(myBuf, 0, maxOutBuf);
	 rstrcpy(myBuf, resultBuf1, RESULT_BUF1_SIZE);
      }
      if (rowCount > totalRows) {
	 int oldTotalRows;
	 oldTotalRows = totalRows;
	 if (totalRows < 1000) {
	    totalRows = totalRows*2;
	 }
	 else {
	    totalRows = totalRows+500;
	 }
	 pbuf = myBuf;
	 toMalloc=((totalRows+1)*rowSize)*3;
	 myBuf=malloc(toMalloc);
	 if (myBuf <=0) return(SYS_MALLOC_ERR);
	 maxOutBuf=toMalloc;
	 memset(myBuf, 0, maxOutBuf);
	 strcpy(myBuf, pbuf);
	 free(pbuf);
      }
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
				    strlen(DBO_CONFIG_FILE) + 24));

   sprintf (dboConfigFile, "%s/%s", getDboConfigDir(), 
	    DBO_CONFIG_FILE);

   fptr = fopen (dboConfigFile, "r");

   if (fptr == NULL) {
      rodsLog (LOG_NOTICE, 
	       "Cannot open DBO_CONFIG_FILE file %s. errno = %d\n",
          dboConfigFile, errno);
      free (dboConfigFile);
      return (DBO_CONFIG_FILE_ERR);
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

   return(DBO_NAME_NOT_FOUND);
}
