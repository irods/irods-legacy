/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* 
  This is an interface to the Ticket management system.
*/

#include "rods.h"
#include "rodsClient.h"

#define MAX_SQL 300
#define BIG_STR 3000

extern int get64RandomBytes(char *buf);

char cwd[BIG_STR];

int debug=0;
int longMode=0; /* more detailed listing */

char zoneArgument[MAX_NAME_LEN+2]="";

rcComm_t *Conn;
rodsEnv myEnv;

int lastCommandStatus=0;
int printCount=0;

int usage(char *subOpt);

/* 
 print the results of a general query.
 */
void
printGenQueryResults(rcComm_t *Conn, int status, genQueryOut_t *genQueryOut, 
		     char *descriptions[])
{
   int i, j;
   char localTime[20];
   lastCommandStatus = status;
   if (status == CAT_NO_ROWS_FOUND) lastCommandStatus = 0;
   if (status!=0 && status != CAT_NO_ROWS_FOUND) {
      printError(Conn, status, "rcGenQuery");
   }
   else {
      if (status == CAT_NO_ROWS_FOUND) {
	 if (printCount==0) printf("No rows found\n");
      }
      else {
	 for (i=0;i<genQueryOut->rowCnt;i++) {
	    if (i>0) printf("----\n");
	    for (j=0;j<genQueryOut->attriCnt;j++) {
	       char *tResult;
	       tResult = genQueryOut->sqlResult[j].value;
	       tResult += i*genQueryOut->sqlResult[j].len;
	       if (*descriptions[j]!='\0') {
		  if (strstr(descriptions[j],"time")!=0) {
		     getLocalTimeFromRodsTime(tResult, localTime);
		     printf("%s: %s\n", descriptions[j], 
			    localTime);
		  } 
		  else {
		     printf("%s: %s\n", descriptions[j], tResult);
		     printCount++;
		  }
	       }
	    }
	 }
      }
   }
}


/*
Via a general query, show the Tickets for this user
*/
int
showThisUsersTickets()
{
   genQueryInp_t genQueryInp;
   genQueryOut_t *genQueryOut;
   int i1a[10];
   int i1b[10];
   int i2a[10];
   int i;
   char *condVal[10];
   int status;
   char *columnNames[]={"id", "obj id", "type", "limit", "count"};

   memset (&genQueryInp, 0, sizeof (genQueryInp_t));

   printCount=0;

   i=0;
   i1a[i]=COL_TICKET_ID;
   i1b[i++]=0;
   i1a[i]=COL_TICKET_OBJECT_ID;
   i1b[i++]=0;
   i1a[i]=COL_TICKET_OBJECT_TYPE;
   i1b[i++]=0;
   i1a[i]=COL_TICKET_USES_LIMIT;
   i1b[i++]=0;
   i1a[i]=COL_TICKET_USES_COUNT;
   i1b[i++]=0;
   genQueryInp.selectInp.inx = i1a;
   genQueryInp.selectInp.value = i1b;
   genQueryInp.selectInp.len = i;

   genQueryInp.sqlCondInp.inx = i2a;
   genQueryInp.sqlCondInp.value = condVal;
   genQueryInp.sqlCondInp.len=0;


   genQueryInp.maxRows=10;
   genQueryInp.continueInx=0;
   genQueryInp.condInput.len=0;

   if (zoneArgument[0]!='\0') {
      addKeyVal (&genQueryInp.condInput, ZONE_KW, zoneArgument);
   }

   status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
   if (status == CAT_NO_ROWS_FOUND) {
      i1a[0]=COL_USER_COMMENT;
      genQueryInp.selectInp.len = 1;
      status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
      if (status==0) {
	 printf("None\n");
	 return(0);
      }
      if (status == CAT_NO_ROWS_FOUND) {
	 printf("None\n");
	 return(0);
      }
   }

   printGenQueryResults(Conn, status, genQueryOut, columnNames);

   while (status==0 && genQueryOut->continueInx > 0) {
      genQueryInp.continueInx=genQueryOut->continueInx;
      status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
      if (genQueryOut->rowCnt>0) printf("----\n");
      printGenQueryResults(Conn, status, genQueryOut, 
					columnNames);
   }

   return (0);
}

void
makeFullPath(char *inName, char **outName) {
   static char fullName[LONG_NAME_LEN];
   strcpy(fullName, "");
   if (strlen(inName)>0) {
      strncpy(fullName, cwd, LONG_NAME_LEN);
      if (*inName=='/') {
	 strncpy(fullName, inName, LONG_NAME_LEN);
      }
      else {
	 rstrcat(fullName, "/", LONG_NAME_LEN);
	 rstrcat(fullName, inName, LONG_NAME_LEN);
      }
   }
   *outName=fullName;
}

/*
 Create, modify, or delete a ticket
 */
int
doTicketOp(char *arg1, char *arg2, char *arg3, char *arg4, char *arg5) {
   userAdminInp_t userAdminInp;
   int status;
   char *mySubName;
   char *myName;

   userAdminInp.arg0 = "ticket";
   userAdminInp.arg1 = arg1;
   userAdminInp.arg2 = arg2;
   userAdminInp.arg3 = arg3;
   userAdminInp.arg4 = arg4;
   userAdminInp.arg5 = arg5;
   userAdminInp.arg6 ="";
   userAdminInp.arg7 ="";
   userAdminInp.arg8 ="";
   userAdminInp.arg9 ="";

   status = rcUserAdmin(Conn, &userAdminInp);
   lastCommandStatus = status;

   if (status < 0 ) {
      if (Conn->rError) {
	 rError_t *Err;
         rErrMsg_t *ErrMsg;
	 int i, len;
	 Err = Conn->rError;
	 len = Err->len;
	 for (i=0;i<len;i++) {
	    ErrMsg = Err->errMsg[i];
	    rodsLog(LOG_ERROR, "Level %d: %s",i, ErrMsg->msg);
	 }
      }
      myName = rodsErrorName(status, &mySubName);
      rodsLog (LOG_ERROR, "rcUserAdmin failed with error %d %s %s",
	       status, myName, mySubName);
   }
   return(status);
}

void
makeTicket(char *newTicket) {
   int characterSet_len;
   int characterSet[26+26+10];
   char buf1[100], buf2[20];
   get64RandomBytes(buf1);
   int i, ix, j;

/*
 Set up an array of characters that are allowed in the result.
*/
   characterSet_len=26+26+10;
   j=0;
   for (i=0;i<26;i++) characterSet[j++]=(int)'A' + i;
   for (i=0;i<26;i++) characterSet[j++]=(int)'a' + i;
   for (i=0;i<10;i++) characterSet[j++]=(int)'0' + i;

   for (i=0,j=0;j<15;i++) {
      ix = (int)buf1[i];
      ix = ix & 0x3f;
      if (ix < characterSet_len-1) {
	 buf2[j++]=(char)characterSet[ix];
      }
      else {
      }
   }
   buf2[j++]='\0';
   strncpy(newTicket, buf2, 20);
   printf("ticket:%s\n",buf2);
}

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
   fputs("iticket>",stdout);
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

/*
 Detect a 'l' in a '-' option and if present, set a mode flag and
 remove it from the string (to simplify other processing).
 */
void
handleMinusL(char *token) {
   char *cptr, *cptr2;
   if (*token!='-') return;
   for (cptr=token++;*cptr!='\0';cptr++) {
      if (*cptr=='l') {
	 longMode=1;
	 for (cptr2=cptr;*cptr2!='\0';cptr2++) {
	    *cptr2=*(cptr2+1);
	 }
	 return;
      }
   }
   longMode=0;
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

   handleMinusL(cmdToken[1]);
   handleMinusL(cmdToken[2]);

   if (strcmp(cmdToken[0],"create") == 0 
       || strcmp(cmdToken[0],"make") == 0 
       || strcmp(cmdToken[0],"mk") == 0 
      ) {
      static char myTicket[30];
      char *fullPath=0;
      if (strlen(cmdToken[3])>0) {
	 strncpy(myTicket, cmdToken[3], 30);
      }
      else {
	 makeTicket(myTicket);
      }
      makeFullPath(cmdToken[2], &fullPath);
      doTicketOp("create", myTicket, cmdToken[1], fullPath,
		cmdToken[3]);
      return(0);
   }


   if (strcmp(cmdToken[0],"delete") == 0 ) {
      doTicketOp("delete", cmdToken[1], cmdToken[2], 
		cmdToken[3], cmdToken[4]);
      return(0);
   }


   if (strcmp(cmdToken[0],"mod") == 0) {
      doTicketOp("mod", cmdToken[1], cmdToken[2], 
		cmdToken[3], cmdToken[4]);
      return(0);
   }

   if (strcmp(cmdToken[0],"ls") == 0) {
      showThisUsersTickets();
      return(0);
   }

   if (*cmdToken[0] != '\0') {
      printf("unrecognized command, try 'help'\n");
      return(-2);
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

   status = parseCmdLineOpt (argc, argv, "vVhgrcGRCdulz:", 0, &myRodsArgs);
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

   if (myRodsArgs.longOption) {
      longMode=1;
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

#if defined(linux_platform)
   /*
     imeta cp -d TestFile1 -d TestFile3
     comes in as:     -d -d cp TestFile1 TestFile3
     so switch it to: cp -d -d TestFile1 TestFile3
   */
   if (cmdToken[0]!=NULL && *cmdToken[0]=='-') {
      /* args were toggled, switch them back */
      if (cmdToken[1]!=NULL && *cmdToken[1]=='-') {
	 cmdToken[0]=argv[argOffset+2];
	 cmdToken[1]=argv[argOffset];
	 cmdToken[2]=argv[argOffset+1];
      }
      else {
	 cmdToken[0]=argv[argOffset+1];
	 cmdToken[1]=argv[argOffset];
      }
   }
#else
   /* tested on Solaris, not sure other than Linux/Solaris */
   /*
     imeta cp -d TestFile1 -d TestFile3
     comes in as:     cp -d TestFile1 -d TestFile3
     so switch it to: cp -d -d TestFile1 TestFile3
   */
   if (cmdToken[0]!=NULL && cmdToken[1]!=NULL && *cmdToken[1]=='-' &&
       cmdToken[2]!=NULL && cmdToken[3]!=NULL && *cmdToken[3]=='-') {
      /* two args */
      cmdToken[2]=argv[argOffset+3];
      cmdToken[3]=argv[argOffset+2];
   }

#endif

   if (strcmp(cmdToken[0],"help")==0 ||
	      strcmp(cmdToken[0],"h") == 0) {
      usage(cmdToken[1]);
      exit(0);
   }

   if (strcmp(cmdToken[0],"spass") ==0) {
      char scrambled[MAX_PASSWORD_LEN+100];
      if (strlen(cmdToken[1])>MAX_PASSWORD_LEN-2) {
	 printf("Password exceeds maximum length\n");
      }
      else {
	 obfEncodeByKey(cmdToken[1], cmdToken[2], scrambled);
	 printf("Scrambled form is:%s\n", scrambled);
      }
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

/*
Print the main usage/help information.
 */
void usageMain()
{
   char *msgs[]={
"Usage: iticket [-h] [command]", 
" -h This help",
"Commands are:", 
" create read/write Object-Name [string] (create a new ticket)",
" mod Ticket_string-or-id uses/expire string-or-none  (modify restrictions)",
" mod Ticket_string-or-id add/remove host/user string (modify restrictions)",
" ls [-l] [-u user] [-a] list tickets (by default, just your own)",
" delete ticket_string-or-id",
" quit", 
" ", 
"Tickets are another way to provide access to iRODS data-objects (files) or",
"collections (directories or folders).  The 'iticket' command allows you",
"to create, modify, list, and delete tickets.  When you create a ticket",
"it's 16 character string it given to you which you can share with others.",
"This is less secure than normal iRODS login-based access control, but",
"is useful in some situations.  See the 'ticket-based access' page on",
"irods.org for more information.",
" ", 
"A blank execute line invokes the interactive mode, where iticket", 
"prompts and executes commands until 'quit' or 'q' is entered.", 
"Like other unix utilities, a series of commands can be piped into it:",
"'cat file1 | iticket' (maintaining one connection for all commands).",
" ",
"Use 'help command' for more help on a specific command.",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) break;
      printf("%s\n",msgs[i]);
   }
   printReleaseInfo("iticket");
}

/*
Print either main usage/help information, or some more specific
information on particular commands.
 */
int
usage(char *subOpt)
{
   int i;
   if (*subOpt=='\0') {
      usageMain();
   }
   else {
      if (strcmp(subOpt,"create")==0) {
	 char *msgs[]={
" create read/write Object-Name (create a new ticket)",
"Create a new ticket for Object-Name, which is either a data-object (file)",
"or a collection (directory). ",
"Example: create read myFile",
"The ticket string, which can be used for access, will be displayed.",
""};
	 for (i=0;;i++) {
	    if (strlen(msgs[i])==0) return(0);
	    printf("%s\n",msgs[i]);
	 }
      }
      if (strcmp(subOpt,"mod")==0) {
	 char *msgs[]={
"   mod Ticket-id uses/expire string-or-none",
"or mod Ticket-id add/remove host/user string (modify restrictions)",
"Modify a ticket to use one of the specialized options.  By default a",
"ticket can be used by anyone (and 'anonymous'), from any host, and any",
"number of times, and for all time (until deleted).  You can modify it to",
"add (or remove) these types of restrictions.",
" ",
" 'mod Ticket-id uses integer-or-none' will make the ticket only valid",
"the specified number of times.  Use 'none' to remove this restriction.",
" ",
" 'mod Ticket-id add/remove user Username' will make the ticket only valid",
"when used by that particular iRODS user.  You can use multiple mod commands",
"to add more users to the allowed list.",
" ",
" 'mod Ticket-id add/remove host Hostname' will make the ticket only valid",
"when used from that particular host computer.  Hostname will be converted to",
"the IP address for use in the internal checks.  You can use multiple mod",
"commands to add more hosts to the list.",
" ",
" 'mod Ticket-id expire date.time-or-0' will make the ticket only valid",
"before the specified date-time.  You can cancel this expiration by using",
"'0'.  The time is year-mo-da.hr:min:sec, for example: 2012-05-07.23:00:00",
" ",
" The Ticket-id is either the ticket object number or the ticket-string",
""};
	 for (i=0;;i++) {
	    if (strlen(msgs[i])==0) return(0);
	    printf("%s\n",msgs[i]);
	 }
      }
      if (strcmp(subOpt,"delete")==0) {
	 char *msgs[]={
" delete Ticket-string", 
"Remove a ticket from the system.  Access will no longer be allowed",
"via the ticket-string.",
""};
	 for (i=0;;i++) {
	    if (strlen(msgs[i])==0) return(0);
	    printf("%s\n",msgs[i]);
	 }
      }

      if (strcmp(subOpt,"ls")==0) {
	 char *msgs[]={
" ls [-l] [-u user] [-a] list tickets",

"By default, 'ls' will briefly list the tickets owned by you.",
"The -l option will list them in long form.",
"The -a (all) option will list all the tickets (all users).",
"The -u username option will cause it to list tickets owned by that user.",
""};
	 for (i=0;;i++) {
	    if (strlen(msgs[i])==0) return(0);
	    printf("%s\n",msgs[i]);
	 }
      }
      printf("Sorry, either %s is an invalid command or the help has not been written yet\n",
	     subOpt);
   }
   return(0);
}
