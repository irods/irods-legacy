/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* 
  This is an interface to the database objects.
*/

#include "rods.h"
#include "rodsClient.h"

#define MAX_SQL 4000
#define BIG_STR 200

char cwd[BIG_STR];
char lastResc[50]="";

int debug=0;

char zoneArgument[MAX_NAME_LEN+2]="";

rcComm_t *Conn;
rodsEnv myEnv;

int lastCommandStatus=0;
int printCount=0;

void usage(char *subOpt);

/* 
 Prompt for input and parse into tokens
*/
void
getInput(char *cmdToken[], int maxTokens) {
   int lenstr, i;
   static char ttybuf[BIG_STR];
   int nTokens;
   int tokenFlag; /* 1: start reg, 2: start ", 3: start ' */
   char *cpTokenStart;
   char *stat;

   memset(ttybuf, 0, BIG_STR);
   fputs("idbo>",stdout);
   stat = fgets(ttybuf, BIG_STR, stdin);
   if (stat==0) {
      printf("\n");
      rcDisconnect(Conn);
      if (lastCommandStatus != 0) exit(4);
      exit(0);
   }
   lenstr=strlen(ttybuf);
   for (i=0;i<maxTokens;i++) {
      cmdToken[i]="";
   }
   cpTokenStart = ttybuf;
   nTokens=0;
   tokenFlag=0;
   for (i=0;i<lenstr;i++) {
      if (ttybuf[i]=='\n') {
	 ttybuf[i]='\0';
	 cmdToken[nTokens++]=cpTokenStart;
	 return;
      }
      if (tokenFlag==0) {
	 if (ttybuf[i]=='\'') {
	    tokenFlag=3;
	    cpTokenStart++;
	 }
	 else if (ttybuf[i]=='"') {
	    tokenFlag=2;
	    cpTokenStart++;
	 }
	 else if (ttybuf[i]==' ') {
	    cpTokenStart++;
	 }
	 else {
	    tokenFlag=1;
	 }
      }
      else if (tokenFlag == 1) {
	 if (ttybuf[i]==' ') {
	    ttybuf[i]='\0';
	    cmdToken[nTokens++]=cpTokenStart;
	    cpTokenStart = &ttybuf[i+1];
	    tokenFlag=0;
	 }
      }
      else if (tokenFlag == 2) {
	 if (ttybuf[i]=='"') {
	    ttybuf[i]='\0';
	    cmdToken[nTokens++]=cpTokenStart;
	    cpTokenStart = &ttybuf[i+1];
	    tokenFlag=0;
	 }
      }
      else if (tokenFlag == 3) {
	 if (ttybuf[i]=='\'') {
	    ttybuf[i]='\0';
	    cmdToken[nTokens++]=cpTokenStart;
	    cpTokenStart = &ttybuf[i+1];
	    tokenFlag=0;
	 }
      }
   }
}

int
getDbInfo(char *dbRescName, char *dbObjIx) {
   databaseObjInfoInp_t databaseObjInfoInp;
   databaseObjInfoOut_t *databaseObjInfoOut;
   int status;
   char *myName;
   char *mySubName;
   int saveLastResc=1;
   if (dbRescName==NULL || strlen(dbRescName)==0) {
      if (strlen(lastResc)==0) {
	 printf("You need to include the resource name; see 'help info'.\n");
	 return(0);
      }
      else {
	 databaseObjInfoInp.dbrName = lastResc;
	 databaseObjInfoInp.objDesc = -1;
	 saveLastResc=0;
      }
   }
   else {
      databaseObjInfoInp.dbrName = dbRescName;
      if (dbRescName[0]<='9' && dbRescName[0]>='0') {
	 databaseObjInfoInp.dbrName = lastResc;
	 databaseObjInfoInp.objDesc = atoi(dbRescName);
	 saveLastResc=0;
      }
      else {
	 databaseObjInfoInp.objDesc = -1;
	 if (dbObjIx != NULL && strlen(dbObjIx)>0) {
	    databaseObjInfoInp.objDesc = atoi(dbObjIx);
	 }
      }
   }
   status = rcDatabaseObjInfo(Conn, &databaseObjInfoInp, &databaseObjInfoOut);
   if (status) {
      myName = rodsErrorName(status, &mySubName);
      rodsLog (LOG_ERROR, "rcDatabaseObjInfo failed with error %d %s %s",
	       status, myName, mySubName);
      if (databaseObjInfoOut != NULL &&
	  databaseObjInfoOut->outBuf != NULL &&
	  strlen(databaseObjInfoOut->outBuf)>0 ) {
	 if(status == DBO_NOT_COMPILED_IN) {
	    printf("Error message return by the iRODS server:\n");
	 }
	 else {
	    printf("Error message return by the DBMS:\n");
	 }
	 printf("%s\n",databaseObjInfoOut->outBuf);
      }
      return(status);
   }
   printf("%s\n",databaseObjInfoOut->outBuf);
   if (saveLastResc) strncpy(lastResc,dbRescName, sizeof(lastResc));
   return(status);
}

int
execDbo(char *dbRescName, char *dbObjIx, char *dboName) {
   databaseObjInfoInp_t databaseObjInfoInp;
   databaseObjInfoOut_t *databaseObjInfoOut;
   int status;
   char *myName;
   char *mySubName;
   int saveLastResc=1;
   char fullName[BIG_STR];

   if (dbRescName==NULL || strlen(dbRescName)==0) {
      if (strlen(lastResc)==0) {
	 printf("You need to include the resource name; see 'help info'.\n");
	 return(0);
      }
      else {
	 databaseObjInfoInp.dbrName = lastResc;
	 databaseObjInfoInp.objDesc = -1;
	 saveLastResc=0;
      }
   }
   else {
      databaseObjInfoInp.dbrName = dbRescName;
      if (dbRescName[0]<='9' && dbRescName[0]>='0') {
	 databaseObjInfoInp.dbrName = lastResc;
	 databaseObjInfoInp.objDesc = atoi(dbRescName);
	 saveLastResc=0;
      }
      else {
	 databaseObjInfoInp.objDesc = -1;
	 if (dbObjIx != NULL && strlen(dbObjIx)>0) {
	    databaseObjInfoInp.objDesc = atoi(dbObjIx);
	 }
      }
   }
   databaseObjInfoInp.option = "execute";

   if(dboName[0]=='/') {
      strncpy(fullName, dboName, BIG_STR);
   }
   else {
      strncpy(fullName, cwd, BIG_STR);
      strncat(fullName, "/", BIG_STR);
      strncat(fullName, dboName, BIG_STR);
   }
   databaseObjInfoInp.optionArg = fullName;


   status = rcDatabaseObjInfo(Conn, &databaseObjInfoInp, &databaseObjInfoOut);
   if (status) {
      myName = rodsErrorName(status, &mySubName);
      rodsLog (LOG_ERROR, "rcDatabaseObjInfo failed with error %d %s %s",
	       status, myName, mySubName);
      if (databaseObjInfoOut != NULL &&
	  databaseObjInfoOut->outBuf != NULL &&
	  strlen(databaseObjInfoOut->outBuf)>0 ) {
	 if(status == DBO_NOT_COMPILED_IN) {
	    printf("Error message return by the iRODS server:\n");
	 }
	 else {
	    printf("Error message return by the DBMS:\n");
	 }
	 printf("%s\n",databaseObjInfoOut->outBuf);
      }
      return(status);
   }
   printf("%s\n",databaseObjInfoOut->outBuf);
   if (saveLastResc) strncpy(lastResc,dbRescName, sizeof(lastResc));
   return(status);
}

int
openDatabaseObj(char *dbRescName, char *dbObjName) {
   databaseObjOpenInp_t databaseObjOpnInp;
   int status;
   char *myName;
   char *mySubName;
   databaseObjOpnInp.dbrName = dbRescName;
   databaseObjOpnInp.dboName = dbObjName;
   status = rcDatabaseObjOpen(Conn, &databaseObjOpnInp);
   if (status<0) {
      myName = rodsErrorName(status, &mySubName);
      rodsLog (LOG_ERROR, "rcDatabaseObjOpen failed with error %d %s %s",
	       status, myName, mySubName);
      return(status);
   }
   printf("open object-descriptor (index to open database)=%d\n",
	  status);
   strncpy(lastResc,dbRescName, sizeof(lastResc));
   return(status);
}

void
getInputLine(char *prompt, char *result, int max_result_len) {
   char *stat;
   int i;
   fputs(prompt,stdout);
   stat = fgets(result, max_result_len, stdin);
   for (i=0;i<max_result_len;i++) {
      if (result[i]=='\n') {
	 result[i]='\0';
	 break;
      }
   }
}

/* add a DBO-SQL-Object */
int
addSql() {
   int status;
   databaseObjectAdminInp_t databaseObjectAdminInp;
   databaseObjectAdminOut_t *databaseObjectAdminOut;
   char *myName;
   char *mySubName;
   char rescName[BIG_STR];
   char objName[BIG_STR];
   char objNameFromUser[BIG_STR];
   char desc[BIG_STR];
   char sql[MAX_SQL];
   getInputLine("Resource name:", rescName, BIG_STR);
   getInputLine("Object name or full path:", objNameFromUser, BIG_STR);
   if(objNameFromUser[0]=='/') {
      strncpy(objName, objNameFromUser, BIG_STR);
   }
   else {
      strncpy(objName, cwd, BIG_STR);
      strncat(objName, "/", BIG_STR);
      strncat(objName, objNameFromUser, BIG_STR);
   }
   getInputLine("Description:", desc, BIG_STR);
   getInputLine("SQL:", sql, MAX_SQL);

   memset((void *)&databaseObjectAdminInp, 0, sizeof(databaseObjectAdminInp));
   databaseObjectAdminInp.dbrName = rescName;
   databaseObjectAdminInp.dboName = objName;
   databaseObjectAdminInp.description = desc;
   databaseObjectAdminInp.sql = sql;

   databaseObjectAdminInp.option = DBObjAdmin_Add;

   status = rcDatabaseObjectAdmin(Conn, &databaseObjectAdminInp, 
				  &databaseObjectAdminOut);
   if (status) {
      myName = rodsErrorName(status, &mySubName);
      rodsLog (LOG_ERROR, "rcDatabaseObjectAdmin failed with error %d %s %s",
	       status, myName, mySubName);
      return(status);
   }
   return(status);
}

/* remove a DBO-SQL-Object */
int
rmSql() {
   int status;
   databaseObjectAdminInp_t databaseObjectAdminInp;
   databaseObjectAdminOut_t *databaseObjectAdminOut;
   char *myName;
   char *mySubName;
/*   char rescName[BIG_STR]; */
   char objName[BIG_STR];
   char objNameFromUser[BIG_STR];

   memset((void *)&databaseObjectAdminInp, 0, sizeof(databaseObjectAdminInp));

/*   getInputLine("Resource name:", rescName, BIG_STR); */
   getInputLine("Object name or full path:", objNameFromUser, BIG_STR);
   if(objNameFromUser[0]=='/') {
      strncpy(objName, objNameFromUser, BIG_STR);
   }
   else {
      strncpy(objName, cwd, BIG_STR);
      strncat(objName, "/", BIG_STR);
      strncat(objName, objNameFromUser, BIG_STR);
   }


/*   databaseObjectAdminInp.dbrName = rescName; */
   databaseObjectAdminInp.dboName = objName;

   databaseObjectAdminInp.option = DBObjAdmin_Remove;

   status = rcDatabaseObjectAdmin(Conn, &databaseObjectAdminInp, 
				  &databaseObjectAdminOut);
   if (status) {
      myName = rodsErrorName(status, &mySubName);
      rodsLog (LOG_ERROR, "rcDatabaseObjectAdmin failed with error %d %s %s",
	       status, myName, mySubName);
      return(status);
   }
   return(status);
}

/* 
 print the results of a general query.
 */
int
printGenQueryResults(rcComm_t *Conn, int status, genQueryOut_t *genQueryOut, 
		     char *descriptions[], int doDashes)
{
   int printCount;
   int i, j;
   char localTime[20];
   printCount=0;
   if (status!=0) {
      printError(Conn, status, "rcGenQuery");
   }
   else {
      if (status !=CAT_NO_ROWS_FOUND) {
	 for (i=0;i<genQueryOut->rowCnt;i++) {
	    if (i>0 && doDashes) printf("----\n");
	    for (j=0;j<genQueryOut->attriCnt;j++) {
	       char *tResult;
	       tResult = genQueryOut->sqlResult[j].value;
	       tResult += i*genQueryOut->sqlResult[j].len;
	       if (descriptions !=0 && *descriptions[j]!='\0') {
		  if (strstr(descriptions[j],"_ts")!=0) {
		     getLocalTimeFromRodsTime(tResult, localTime);
		     if (atoll(tResult)==0) rstrcpy(localTime, "None", 20);
		     printf("%s: %s: %s\n", descriptions[j], tResult, 
			    localTime);
		  } 
		  else {
		     printf("%s: %s\n", descriptions[j], tResult);
		  }
	       }
	       else {
		  printf("%s\n", tResult);
	       }
	       printCount++;
	    }
	 }
      }
   }
   return(printCount);
}

/*
 do a Query on a specific DBO
 */
int
doQueryDbo(char *objName) {
   genQueryInp_t genQueryInp;
   genQueryOut_t *genQueryOut;
   int i1a[30];
   int i1b[30]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   int i2a[30];
   char *condVal[10];
   char v1[MAX_NAME_LEN+10];
   char v2[MAX_NAME_LEN+10];
   char fullName[BIG_STR];
   char myDirName[BIG_STR];
   char myFileName[BIG_STR];
   int status;
   int printCount;

   char *columnNames[]={"attribute", "value", "", "" , "", ""};
   if(*objName=='/') {
      strncpy(fullName, objName, BIG_STR);
   }
   else {
      strncpy(fullName, cwd, BIG_STR);
      strncat(fullName, "/", BIG_STR);
      strncat(fullName, objName, BIG_STR);
   }
   memset (&genQueryInp, 0, sizeof (genQueryInp_t));

   printf("Data-object %s:\n",objName);
   printCount=0;
   i1a[0]=COL_META_DATA_ATTR_NAME;
   i1b[0]=0;
   i1a[1]=COL_META_DATA_ATTR_VALUE;
   i1b[1]=0;
   genQueryInp.selectInp.inx = i1a;
   genQueryInp.selectInp.value = i1b;
   genQueryInp.selectInp.len = 2;



   status = splitPathByKey(fullName, 
			   myDirName, myFileName, '/');
   sprintf(v1,"='%s'",myDirName);
   i2a[0]=COL_COLL_NAME;
   condVal[0]=v1;
   sprintf(v2,"='%s'",myFileName);
   i2a[1]=COL_DATA_NAME;
   condVal[1]=v2;

   genQueryInp.sqlCondInp.inx = i2a;
   genQueryInp.sqlCondInp.value = condVal;
   genQueryInp.sqlCondInp.len=2;

   genQueryInp.maxRows=10;
   genQueryInp.continueInx=0;
   genQueryInp.condInput.len=0;

   if (zoneArgument[0]!='\0') {
      addKeyVal (&genQueryInp.condInput, ZONE_KW, zoneArgument);
   }

   status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
   if (status == CAT_NO_ROWS_FOUND) {
      i1a[0]=COL_D_DATA_PATH;
      genQueryInp.selectInp.len = 1;
      status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
      if (status==0) {
	 printf("Is not a database-object\n");
	 return(0);
      }
      if (status == CAT_NO_ROWS_FOUND) {
	 printf("Dataobject %s does not exist.\n", fullName);
	 return(0);
      }
   }
   else {
      int i, j, printNext;
      for (i=0;i<genQueryOut->rowCnt;i++) {
	 printNext=0;
	 for (j=0;j<genQueryOut->attriCnt;j++) {
	    char *tResult;
	    tResult = genQueryOut->sqlResult[j].value;
	    tResult += i*genQueryOut->sqlResult[j].len;
	    if (strcmp(tResult,DBO_SQL)==0) {
		printf("%s:", DBO_SQL);
		printNext=1;
	    }
	    if (strcmp(tResult,DBO_DESC)==0) {
		printf("%s:", DBO_DESC);
		printNext=1;
	    }
	    if (strcmp(tResult,DBO_RESC)==0) {
		printf("%s:", DBO_RESC);
		printNext=1;
	    }
	    if (j==1) {
	       if (printNext==1) {
		  printf("%s\n", tResult);
		  printCount++;
	       }
	       printNext=0;
	    }
	 }
	 if (printCount==0) {
	    printf("Is not a database-object\n");
	 }
      }
   }

   while (status==0 && genQueryOut->continueInx > 0) {
      genQueryInp.continueInx=genQueryOut->continueInx;
      status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
      if (genQueryOut->rowCnt>0) printf("----\n");
      printGenQueryResults(Conn, status, genQueryOut, 
			   columnNames, 0);
   }

   return (0);
}

int
doQuery(char *objName) {
   genQueryInp_t genQueryInp;
   genQueryOut_t *genQueryOut;
   int i1a[30];
   int i1b[30]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   int i2a[30];
   char *condVal[10];
   char v1[MAX_NAME_LEN+10];
   int i, status;
   int printCount;

   char *columnNames[]={
   "data_name",
   "coll_name",
   "desc",
   "other"};

   if (strlen(objName)>0) {
      return(doQueryDbo(objName));
   }
   memset (&genQueryInp, 0, sizeof (genQueryInp_t));
   printCount=0;

   i=0;
   i1a[i++]=COL_DATA_NAME;
   i1a[i++]=COL_COLL_NAME;
   i1a[i++]=COL_META_DATA_ATTR_VALUE;
   
   i2a[0]=COL_META_DATA_ATTR_NAME;
   sprintf(v1,"='%s'", DBO_DESC);
   condVal[0]=v1;

   genQueryInp.sqlCondInp.inx = i2a;
   genQueryInp.sqlCondInp.value = condVal;
   genQueryInp.sqlCondInp.len=1;

   genQueryInp.selectInp.inx = i1a;
   genQueryInp.selectInp.value = i1b;
   genQueryInp.selectInp.len = i;

   genQueryInp.maxRows=50;
   genQueryInp.continueInx=0;
   status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
   if (status == CAT_NO_ROWS_FOUND) {
      printf("DataObject %s does not exist.\n", objName);
      return(0);
   }
   printCount+= printGenQueryResults(Conn, status, genQueryOut, columnNames, 
				     1);
   return(printCount);
}

/*
 do a Query on a specific DBO
 */
int
doTest(char *inStr) {
   genQueryInp_t genQueryInp;
   genQueryOut_t *genQueryOut;
   int i1a[30];
   int i1b[30]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   int i2a[30];
   char *condVal[10];
   char v1[MAX_NAME_LEN+10];
   char v2[MAX_NAME_LEN+10];
   int status;
   int printCount;

   memset (&genQueryInp, 0, sizeof (genQueryInp_t));

   i1a[0]=COL_META_DATA_ATTR_NAME;
   i1b[0]=0;
   i1a[1]=COL_META_DATA_ATTR_VALUE;
   i1b[1]=0;
   genQueryInp.selectInp.inx = i1a;
   genQueryInp.selectInp.value = i1b;
   genQueryInp.selectInp.len = 2;

   sprintf(v1,"='%s'",inStr);
   i2a[0]=COL_D_DATA_ID;
   condVal[0]=v1;

   i2a[1]=COL_META_DATA_ATTR_NAME;
   sprintf(v2,"='%s'", DBO_SQL);
   condVal[1]=v2;

   genQueryInp.sqlCondInp.inx = i2a;
   genQueryInp.sqlCondInp.value = condVal;
   genQueryInp.sqlCondInp.len=2;

   genQueryInp.maxRows=10;
   genQueryInp.continueInx=0;
   genQueryInp.condInput.len=0;

   if (zoneArgument[0]!='\0') {
      addKeyVal (&genQueryInp.condInput, ZONE_KW, zoneArgument);
   }

   status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
   if (status == CAT_NO_ROWS_FOUND) {
      i1a[0]=COL_D_DATA_PATH;
      genQueryInp.selectInp.len = 1;
      status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
      if (status==0) {
	 printf("Is not a database-object\n");
	 return(0);
      }
      if (status == CAT_NO_ROWS_FOUND) {
	 printf("Dataobject does not exist.\n");
	 return(0);
      }
   }
   else {
      int i, j, printNext;
      for (i=0;i<genQueryOut->rowCnt;i++) {
	 printNext=0;
	 for (j=0;j<genQueryOut->attriCnt;j++) {
	    char *tResult;
	    tResult = genQueryOut->sqlResult[j].value;
	    tResult += i*genQueryOut->sqlResult[j].len;
	    if (strcmp(tResult,DBO_SQL)==0) {
		printf("%s:", DBO_SQL);
		printNext=1;
	    }
	    if (j==1) {
	       if (printNext==1) {
		  printf("%s\n", tResult);
		  printCount++;
	       }
	       printNext=0;
	    }
	 }
	 if (printCount==0) {
	    printf("Is not a database-object\n");
	 }
      }
   }

#if 0
   while (status==0 && genQueryOut->continueInx > 0) {
      genQueryInp.continueInx=genQueryOut->continueInx;
      status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
      if (genQueryOut->rowCnt>0) printf("----\n");
      printGenQueryResults(Conn, status, genQueryOut, 
			   columnNames, 0);
   }
#endif

   return (0);
}


/* handle a command,
   return code is 0 if the command was (at least partially) valid,
   -1 for quitting,
   -2 for if invalid
   -3 if empty.
 */
int
doCommand(char *cmdToken[]) {
   if (strcmp(cmdToken[0],"help")==0 ||
	      strcmp(cmdToken[0],"h") == 0) {
      usage(cmdToken[1]);
      return(0);
   }
   if (strcmp(cmdToken[0],"quit")==0 ||
	      strcmp(cmdToken[0],"q") == 0) {
      return(-1);
   }

   if (strcmp(cmdToken[0],"open") == 0) {
      openDatabaseObj(cmdToken[1], cmdToken[2]);
      return(0);
   }

   if (strcmp(cmdToken[0],"info") == 0) {
      getDbInfo(cmdToken[1], cmdToken[2]);
      return(0);
   }

   if (strcmp(cmdToken[0],"addsql") == 0) {
      addSql();
      return(0);
   }
   if (strcmp(cmdToken[0],"rmsql") == 0) {
      rmSql();
      return(0);
   }
   if (strcmp(cmdToken[0],"ls") == 0) {
      doQuery(cmdToken[1]);
      return(0);
   }
   if (strcmp(cmdToken[0],"test") == 0) {
      doTest(cmdToken[1]);
      return(0);
   }
   if (strcmp(cmdToken[0],"exec") == 0) {
      execDbo(cmdToken[1], cmdToken[2], cmdToken[3]);
      return(0);
   }
   if (strlen(cmdToken[0])>0) {
      printf("Unrecognized command\n");
   }
   return(-3);
}

int
main(int argc, char **argv) {
   int status, i, j;
   rErrMsg_t errMsg;

   rodsArguments_t myRodsArgs;

   char *mySubName;
   char *myName;

   int argOffset;

   int maxCmdTokens=20;
   char *cmdToken[20];
   int keepGoing;
   int firstTime;

   rodsLogLevel(LOG_ERROR);

   status = parseCmdLineOpt (argc, argv, "vVhrcRCduz:", 0, &myRodsArgs);
   if (status) {
      printf("Use -h for help.\n");
      exit(1);
   }
   if (myRodsArgs.help==True) {
      usage("");
      exit(0);
   }

   if (myRodsArgs.zone==True) {
      strncpy(zoneArgument, myRodsArgs.zoneName, MAX_NAME_LEN);
   }

   argOffset = myRodsArgs.optind;
   if (argOffset > 1) {
      if (argOffset > 2) {
	 if (*argv[1]=='-' && *(argv[1]+1)=='z') {
	    if (*(argv[1]+2)=='\0') {
	       argOffset=3;  /* skip -z zone */
	    }
	    else {
	       argOffset=2;  /* skip -zzone */
	    }
	 }
	 else {
	    argOffset=1; /* Ignore the parseCmdLineOpt parsing 
			    as -d etc handled  below*/
	 }
      }
      else {
	 argOffset=1; /* Ignore the parseCmdLineOpt parsing 
			 as -d etc handled  below*/
      }
   }

   status = getRodsEnv (&myEnv);
   if (status < 0) {
      rodsLog (LOG_ERROR, "main: getRodsEnv error. status = %d",
	       status);
      exit (1);
   }
   strncpy(cwd,myEnv.rodsCwd,BIG_STR);
   if (strlen(cwd)==0) {
      strcpy(cwd,"/");
   }

   for (i=0;i<maxCmdTokens;i++) {
      cmdToken[i]="";
   }
   j=0;
   for (i=argOffset;i<argc;i++) {
      cmdToken[j++]=argv[i];
   }

   if (strcmp(cmdToken[0],"help")==0 ||
	      strcmp(cmdToken[0],"h") == 0) {
      usage(cmdToken[1]);
      exit(0);
   }

   Conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
                     myEnv.rodsZone, 0, &errMsg);

   if (Conn == NULL) {
      myName = rodsErrorName(errMsg.status, &mySubName);
      rodsLog(LOG_ERROR, "rcConnect failure %s (%s) (%d) %s",
	      myName,
	      mySubName,
	      errMsg.status,
	      errMsg.msg);

      exit (2);
   }

   status = clientLogin(Conn);
   if (status != 0) {
      if (!debug) exit (3);
   }

   keepGoing=1;
   firstTime=1;
   while (keepGoing) {
      int status;
      status=doCommand(cmdToken);
      if (status==-1) keepGoing=0;
      if (firstTime) {
	 if (status==0) keepGoing=0;
	 if (status==-2) {
	    keepGoing=0;
	    lastCommandStatus=-1;
	 }
	 firstTime=0;
      }
      if (keepGoing) {
	 getInput(cmdToken, maxCmdTokens);
      }
   }

   rcDisconnect(Conn);

   if (lastCommandStatus != 0) exit(4);
   exit(0);
}

void
printMsgs(char *msgs[]) {
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) return;
      printf("%s\n",msgs[i]);
   }
}

/*
Print the main usage/help information.
 */
void usageMain()
{
   char *msgs[]={
"This is an interface to the new database-objects, under development.",
"Currently, only a few test commands are implemented.",
"Usage: idbo [-vVhz] [command]", 
" -v verbose",
" -V Very verbose",
" -z Zonename  work with the specified Zone",
" -h This help",
"Commands are:", 
" open ResourceName DatabaseName", 
" info [ResourceName] [Object-descriptor]",
" ls [objName] (list information about defined DBOs)"
" quit",
" ",
"Try 'help command' for more help on a specific command.",
""};
   printMsgs(msgs);
   printReleaseInfo("idbo");
}

/*
Print either main usage/help information, or some more specific
information on particular commands.
 */
void
usage(char *subOpt)
{
   char *openMsgs[]={
"open  ResourceName DatabaseName", 
"Open the specified database object on the specified database resource. ",
""};
   char *infoMsgs[]={
"info [ResourceName] [database-descriptor]",
"list information about the database-resource or a particular (open)",
"database on that resource.",
"In the first case, it will list the configured databases on that",
"database-resource.  In the later, it will list the tables in the database.",
"If ResourceName is not included, it will use the last one used.",
"Examples:",
"$ idbo",
"idbo>info demoResc",
"DBName1 DBName2",
"idbo>open demoResc DBName1",
"open object-descriptor (index to open database)=0",
"idbo>info 0",
"Schema|Name|Type|Owner|",
"public|r_table1|table|schroeder|",
"public|r_table2|table|schroeder|",
"public|cadc_config_archive_case|table|schroeder|",
"public|cadc_config_compression|table|schroeder|",
""};
   char *lsMsgs[]={
"ls [dataObj]", 
"List information about all defined database objects or, if provided,",
"on the one database object.  You can also use 'ils' for other information.",
""};
   char *helpMsgs[]={
" help (or h) [command] (general help, or more details on a command)",
" If you specify a command, a brief description of that command",
" will be displayed.",
""};


   char *subCmds[]={"open", "info", "ls",
		    "help", "h",
		    ""};

   char **pMsgs[]={ openMsgs, infoMsgs, lsMsgs,
		    helpMsgs, helpMsgs };

   if (*subOpt=='\0') {
      usageMain();
   }
   else {
      int i;
      for (i=0;;i++) {
	 if (strlen(subCmds[i])==0) break;
	 if (strcmp(subOpt,subCmds[i])==0) {
	    printMsgs(pMsgs[i]);
	 }
      }
   }
}
