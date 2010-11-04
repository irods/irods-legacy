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
execDbo(char *dbRescName, char *dboName) {
   databaseObjControlInp_t databaseObjControlInp;
   databaseObjControlOut_t *databaseObjControlOut;
   int status;
   char *myName;
   char *mySubName;
   char fullName[BIG_STR];

   memset((void *)&databaseObjControlInp, 0, sizeof(databaseObjControlInp));

   if (dbRescName==NULL || strlen(dbRescName)==0) {
      printf("You need to include the resource name; see 'help control'.\n");
      return(0);
   }

   databaseObjControlInp.dbrName = dbRescName;

   databaseObjControlInp.option = DBO_EXECUTE;

   if(dboName[0]=='/') {
      strncpy(fullName, dboName, BIG_STR);
   }
   else {
      strncpy(fullName, cwd, BIG_STR);
      strncat(fullName, "/", BIG_STR);
      strncat(fullName, dboName, BIG_STR);
   }

   databaseObjControlInp.dboName = fullName;

   status = rcDatabaseObjControl(Conn, &databaseObjControlInp, 
				 &databaseObjControlOut);
   if (status) {
      myName = rodsErrorName(status, &mySubName);
      rodsLog (LOG_ERROR, "rcDatabaseObjControl failed with error %d %s %s",
	       status, myName, mySubName);
      if (databaseObjControlOut != NULL &&
	  databaseObjControlOut->outBuf != NULL &&
	  strlen(databaseObjControlOut->outBuf)>0 ) {
	 if(status == DBO_NOT_COMPILED_IN) {
	    printf("Error message return by the iRODS server:\n");
	 }
	 else {
	    printf("Error message return by the DBMS or DBO interface:\n");
	 }
	 printf("%s\n",databaseObjControlOut->outBuf);
      }
      return(status);
   }
   if (*databaseObjControlOut->outBuf != '\0') {
      printf("%s\n",databaseObjControlOut->outBuf);
   }
   return(status);
}

/* commit or rollback */
int
dbrControl(char *dbRescName, int option) {
   databaseObjControlInp_t databaseObjControlInp;
   databaseObjControlOut_t *databaseObjControlOut;
   int status;
   char *myName;
   char *mySubName;

   memset((void *)&databaseObjControlInp, 0, sizeof(databaseObjControlInp));

   if (dbRescName==NULL || strlen(dbRescName)==0) {
      printf("You need to include the resource name; see 'help control'.\n");
      return(0);
   }

   databaseObjControlInp.dbrName = dbRescName;

   databaseObjControlInp.option = option;

   status = rcDatabaseObjControl(Conn, &databaseObjControlInp, 
				 &databaseObjControlOut);
   if (status) {
      myName = rodsErrorName(status, &mySubName);
      rodsLog (LOG_ERROR, "rcDatabaseObjControl failed with error %d %s %s",
	       status, myName, mySubName);
      if (databaseObjControlOut != NULL &&
	  databaseObjControlOut->outBuf != NULL &&
	  strlen(databaseObjControlOut->outBuf)>0 ) {
	 if(status == DBO_NOT_COMPILED_IN) {
	    printf("Error message return by the iRODS server:\n");
	 }
	 else {
	    printf("Error message return by the DBMS or DBO interface:\n");
	 }
	 printf("%s\n",databaseObjControlOut->outBuf);
      }
      return(status);
   }
   if (*databaseObjControlOut->outBuf != '\0') {
      printf("%s\n",databaseObjControlOut->outBuf);
   }
   return(status);
}

int
openDatabaseResc(char *dbRescName) {
   databaseRescOpenInp_t databaseRescOpenInp;
   int status;
   char *myName;
   char *mySubName;
   databaseRescOpenInp.dbrName = dbRescName;
   status = rcDatabaseRescOpen(Conn, &databaseRescOpenInp);
   if (status<0) {
      myName = rodsErrorName(status, &mySubName);
      rodsLog (LOG_ERROR, "rcDatabaseRescOpen failed with error %d %s %s",
	       status, myName, mySubName);
      return(status);
   }
/*   printf("open object-descriptor (index to open database)=%d\n",
     status);  Not generally needed anymore */
   return(status);
}

int
closeDatabaseResc(char *dbRescName) {
   databaseRescCloseInp_t databaseRescCloseInp;
   int status;
   char *myName;
   char *mySubName;
   databaseRescCloseInp.dbrName = dbRescName;
   status = rcDatabaseRescClose(Conn, &databaseRescCloseInp);
   if (status<0) {
      myName = rodsErrorName(status, &mySubName);
      rodsLog (LOG_ERROR, "rcDatabaseRescClose failed with error %d %s %s",
	       status, myName, mySubName);
      return(status);
   }
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
doTest(char *inStr) {
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
      openDatabaseResc(cmdToken[1]);
      return(0);
   }

   if (strcmp(cmdToken[0],"close") == 0) {
      closeDatabaseResc(cmdToken[1]);
      return(0);
   }

   if (strcmp(cmdToken[0],"test") == 0) {
      doTest(cmdToken[1]);
      return(0);
   }
   if (strcmp(cmdToken[0],"exec") == 0) {
      execDbo(cmdToken[1], cmdToken[2]);
      return(0);
   }
   if (strcmp(cmdToken[0],"commit") == 0) {
      dbrControl(cmdToken[1], DBR_COMMIT);
      return(0);
   }
   if (strcmp(cmdToken[0],"rollback") == 0) {
      dbrControl(cmdToken[1], DBR_ROLLBACK);
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
"This is an interface to the new database resource (DBR)/database",
"object (DBO) capabilities, under development.",
" ",
"A single command can be entered on the command line or, if blank, it",
"will prompt go into interactive mode and prompt for commands.",
"Commands are:",
"  open DBR    (open a database resource)",
"  close DBR   (close a database resource)",
"  exec DBR DBO (execute a DBO on a DBR)",
"  commit DBR   (commit updates to a DBR (done via a DBO))",
"  rollback DBR (rollback updates instead)",
"  close DBR    (close a DBR)",
"  help  (this help)",
"  quit  (exit idbo)",
"Where DBR and DBO are the names of a Database Resource and Database Object.",
" ",
"You can exectute a DBO without first opening the DBR (in which case the",
"server will open and close it), so you can run a DBO from the command",
"line: 'idbo DBR DBO'",
" ",
"See 'Database Resources' on the irods web site for more information.",
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
