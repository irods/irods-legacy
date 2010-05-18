/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* 
  This is an interface to the database objects.
*/

#include "rods.h"
#include "rodsClient.h"

#define MAX_SQL 300
#define BIG_STR 200

char cwd[BIG_STR];
char lastResc[50]="";

int debug=0;
int testMode=0; /* some some particular internal tests */

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
getDboInfo(char *dbRescName, char *dbObjIx) {
   databaseObjInfoInp_t databaseObjInfoInp;
   databaseObjInfoOut_t *databaseObjInfoOut;
   int status;
   char *myName;
   char *mySubName;
   int saveLastResc=1;
   if (dbRescName==NULL || strlen(dbRescName)==0) {
      if (strlen(lastResc)==0) {
	 printf("You need to include the resource name; see 'help ls'.\n");
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
   printf("open object-descriptor (index to open database-object)=%d\n",
	  status);
   strncpy(lastResc,dbRescName, sizeof(lastResc));
   return(status);
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

   if (strcmp(cmdToken[0],"ls") == 0) {
      getDboInfo(cmdToken[1], cmdToken[2]);
      return(0);
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
" open ResourceName DatabaseObjectName", 
" ls [ResourceName] [Object-descriptor]",
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
"open  ResourceName DatabaseObjectName", 
"Open the specified database object on the specified database resource. ",
""};
   char *lsMsgs[]={
"ls [ResourceName] [database-object-descriptor]",
"list information about the database-resource or a particular (open)",
"database-object on that resource.",
"In the first case, it will list the configured database-objects on that",
"database-resource.  In the later, it will list the tables in the DBO.",
"If ResourceName is not included, it will use the last one used.",
"Examples:",
"$ idbo",
"idbo>ls demoResc",
"DBName1 DBName2",
"idbo>open demoResc DBName1",
"open object-descriptor (index to open database-object)=0",
"idbo>ls 0",
"Schema|Name|Type|Owner|",
"public|r_table1|table|schroeder|",
"public|r_table2|table|schroeder|",
"public|cadc_config_archive_case|table|schroeder|",
"public|cadc_config_compression|table|schroeder|",
""};
   char *helpMsgs[]={
" help (or h) [command] (general help, or more details on a command)",
" If you specify a command, a brief description of that command",
" will be displayed.",
""};


   char *subCmds[]={"open", "ls",
		    "help", "h",
		    ""};

   char **pMsgs[]={ openMsgs, lsMsgs,
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
