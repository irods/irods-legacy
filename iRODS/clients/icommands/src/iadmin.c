/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* 
  Initial version of an administrator interface
*/

#include "rodsClient.h"
#include "parseCommandLine.h"

#define MAX_SQL 300
#define BIG_STR 200

/* The simpleQuery input sql is passed as an argument (along with up
   to 4 bind variables) so that is is clear what is going on.  But the
   server-side code checks the input sql against some pre-defined
   forms (to improve security a bit).
*/

int debug=0;
int veryVerbose=0;

rcComm_t *Conn;
rodsEnv myEnv;

int lastCommandStatus=0;

void usage(char *subOpt);

/* print the results of a simple query, converting time values if 
   necessary.  Called recursively.
*/
int
printSimpleQuery(char *buf) {
   char *cpTime, *endOfLine;
   char localTime[20];
   int fieldLen=10;
   cpTime = strstr(buf, "data_expiry_ts");
   if (cpTime!=NULL) {
      fieldLen=15;
   }
   else {
      cpTime = strstr(buf, "free_space_ts");
      if (cpTime!=NULL) {
	 fieldLen=14;
      }
      else {
	 cpTime = strstr(buf, "create_ts");
	 if (cpTime!=NULL) {
	    fieldLen=10;
	 }
	 else {
	    cpTime = strstr(buf, "modify_ts");
	    if (cpTime!=NULL) {
	       fieldLen=10;
	    }
	 }
      }
   }
   if (cpTime==NULL) {
      printf(buf);
      return(0);
   }
   endOfLine=strstr(cpTime,"\n");
   if (endOfLine-cpTime > 30) {
      printf(buf);
   }
   else {
      *endOfLine='\0';
      printf(buf);
      getLocalTimeFromRodsTime(cpTime+fieldLen, localTime);
      printf(" : %s\n", localTime);
      printSimpleQuery(endOfLine+1);
   }
   return(0);
}

int
doSimpleQuery(simpleQueryInp_t simpleQueryInp) {
   int status;
   simpleQueryOut_t *simpleQueryOut;
   char *mySubName;
   char *myName;

   status = rcSimpleQuery(Conn, &simpleQueryInp, &simpleQueryOut);
   lastCommandStatus = status;

   if (status ==  CAT_NO_ROWS_FOUND) {
      lastCommandStatus=0; /* success */
      printf("No rows found\n");
      return(status);
   }
   if (status < 0) {
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
      rodsLog (LOG_ERROR, "rcSimpleQuery failed with error %d %s %s",
	       status, myName, mySubName);
      return(status);
   }
   printSimpleQuery(simpleQueryOut->outBuf);
   if (debug) printf("control=%d\n",simpleQueryOut->control);
   if (simpleQueryOut->control > 0) {
      simpleQueryInp.control = simpleQueryOut->control;
      for (;simpleQueryOut->control > 0 && status==0;) {
	 status = rcSimpleQuery(Conn, &simpleQueryInp, &simpleQueryOut);
	 if (status < 0 && status != CAT_NO_ROWS_FOUND) {
	    myName = rodsErrorName(status, &mySubName);
	    rodsLog (LOG_ERROR, 
		     "rcSimpleQuery failed with error %d %s %s",
		     status, myName, mySubName);
	    return(status);
	 }
	 if (status == 0) {
	    printSimpleQuery(simpleQueryOut->outBuf);
	    if (debug) printf("control=%d\n",simpleQueryOut->control);
	 }
      }
   }
   return(status);
}

int
showToken(char *token, char *tokenName2) 
{
   simpleQueryInp_t simpleQueryInp;

   memset (&simpleQueryInp, 0, sizeof (simpleQueryInp_t));
   simpleQueryInp.control = 0;
   if (token==0 || *token=='\0') {
      simpleQueryInp.form = 1;
      simpleQueryInp.sql = 
		 "select token_name from r_tokn_main where token_namespace = 'token_namespace'";
      simpleQueryInp.maxBufSize = 1024;
   }
   else {
      if (tokenName2==0 || *tokenName2=='\0') {
	 simpleQueryInp.form = 1;
	 simpleQueryInp.sql = "select token_name from r_tokn_main where token_namespace = ?";
	 simpleQueryInp.arg1 = token;
	 simpleQueryInp.maxBufSize = 1024;
      }
      else {
	 simpleQueryInp.form = 2;
	 simpleQueryInp.sql = "select * from r_tokn_main where token_namespace = ? and token_name like ?";
	 simpleQueryInp.arg1 = token;
	 simpleQueryInp.arg2 = tokenName2;
	 simpleQueryInp.maxBufSize = 1024;
      }
   }
   return (doSimpleQuery(simpleQueryInp));
}

int
showResc(char *resc) 
{
   simpleQueryInp_t simpleQueryInp;

   memset (&simpleQueryInp, 0, sizeof (simpleQueryInp_t));
   simpleQueryInp.control = 0;
   if (resc==0 || *resc=='\0') {
      simpleQueryInp.form = 1;
      simpleQueryInp.sql =
	 "select resc_name from r_resc_main";
      simpleQueryInp.maxBufSize = 1024;
   }
   else {
      simpleQueryInp.form = 2;
      simpleQueryInp.sql = "select * from r_resc_main where resc_name=?";
      simpleQueryInp.arg1 = resc;
      simpleQueryInp.maxBufSize = 1024;
   }
   return (doSimpleQuery(simpleQueryInp));
}

int
showZone(char *zone) 
{
   simpleQueryInp_t simpleQueryInp;

   memset (&simpleQueryInp, 0, sizeof (simpleQueryInp_t));
   simpleQueryInp.control = 0;
   if (zone==0 || *zone=='\0') {
      simpleQueryInp.form = 1;
      simpleQueryInp.sql =
	 "select zone_name from r_zone_main";
      simpleQueryInp.maxBufSize = 1024;
   }
   else {
      simpleQueryInp.form = 2;
      simpleQueryInp.sql = "select * from r_zone_main where zone_name=?";
      simpleQueryInp.arg1 = zone;
      simpleQueryInp.maxBufSize = 1024;
   }
   return (doSimpleQuery(simpleQueryInp));
}

int
showGroup(char *group) 
{
   simpleQueryInp_t simpleQueryInp;

   memset (&simpleQueryInp, 0, sizeof (simpleQueryInp_t));
   simpleQueryInp.control = 0;
   if (group==0 || *group=='\0') {
      simpleQueryInp.form = 1;
      simpleQueryInp.sql =
	 "select user_name from r_user_main where user_type_name='rodsgroup'";
      simpleQueryInp.maxBufSize = 1024;
   }
   else {
      printf("Members of group %s:\n",group);
      simpleQueryInp.form = 1;
      simpleQueryInp.sql = 
	 "select user_name from r_user_main, r_user_group where r_user_group.user_id=r_user_main.user_id and r_user_group.group_user_id=(select user_id from r_user_main where user_name=?)";
      simpleQueryInp.arg1 = group;
      simpleQueryInp.maxBufSize = 1024;
   }
   return (doSimpleQuery(simpleQueryInp));
}

int
showFile(char *file) 
{
   simpleQueryInp_t simpleQueryInp;

   memset (&simpleQueryInp, 0, sizeof (simpleQueryInp_t));
   simpleQueryInp.control = 0;
   if (file==0 || *file=='\0') {
      printf("Need to specify a data_id number\n");
      return(USER__NULL_INPUT_ERR);
   }
   simpleQueryInp.form = 2;
   simpleQueryInp.sql = "select * from r_data_main where data_id=?";
   simpleQueryInp.arg1 = file;
   simpleQueryInp.maxBufSize = 1024;
   return (doSimpleQuery(simpleQueryInp));
}

int
showDir(char *dir) 
{
   simpleQueryInp_t simpleQueryInp;
   int status;

   memset (&simpleQueryInp, 0, sizeof (simpleQueryInp_t));
   simpleQueryInp.control = 0;

   if (dir==0 || *dir=='\0') {
      dir="/";
   }
   printf("Contents of collection %s\n", dir);

   simpleQueryInp.form = 1;
   simpleQueryInp.sql = "select data_name, data_id, data_repl_num from r_data_main where coll_id =(select coll_id from r_coll_main where coll_name=?)";
   simpleQueryInp.arg1 = dir;
   simpleQueryInp.maxBufSize = 1024;
   if (debug) simpleQueryInp.maxBufSize = 20;

   printf("Files (data objects) (name, data_id, repl_num):\n");
   status = doSimpleQuery(simpleQueryInp);

   simpleQueryInp.form = 1;
   simpleQueryInp.sql = 
         "select coll_name from r_coll_main where parent_coll_name=?";
   simpleQueryInp.arg1 = dir;
   simpleQueryInp.maxBufSize = 1024;
   printf("Subcollections:\n");
   return (doSimpleQuery(simpleQueryInp));
}

int
showUser(char *user)
{
   simpleQueryInp_t simpleQueryInp;

   memset (&simpleQueryInp, 0, sizeof (simpleQueryInp_t));
   simpleQueryInp.control = 0;
   if (*user!='\0') {
      simpleQueryInp.form = 2;
      simpleQueryInp.sql = "select * from r_user_main where user_name=?";
      simpleQueryInp.arg1 = user;
      simpleQueryInp.maxBufSize = 1024;
   }
   else {
      simpleQueryInp.form = 1;
      simpleQueryInp.sql = "select user_name from r_user_main where user_type_name != 'rodsgroup'";
      simpleQueryInp.maxBufSize = 1024;
   }
   return (doSimpleQuery(simpleQueryInp));
}

int
showRescGroup(char *group)
{
   simpleQueryInp_t simpleQueryInp;

   memset (&simpleQueryInp, 0, sizeof (simpleQueryInp_t));
   simpleQueryInp.control = 0;
   if (*group!='\0') {
      simpleQueryInp.form = 2;
      simpleQueryInp.sql = 
           "select r_resc_group.resc_group_name, r_resc_group.resc_id, resc_name, r_resc_group.create_ts, r_resc_group.modify_ts from r_resc_main, r_resc_group where r_resc_main.resc_id = r_resc_group.resc_id and resc_group_name=?";
      simpleQueryInp.arg1 = group;
      simpleQueryInp.maxBufSize = 1024;
   }
   else {
      simpleQueryInp.form = 1;
      simpleQueryInp.sql = "select distinct resc_group_name from r_resc_group";
      simpleQueryInp.maxBufSize = 1024;
   }
   return (doSimpleQuery(simpleQueryInp));
}

int
simpleQueryCheck()
{
   int status;
   simpleQueryInp_t simpleQueryInp;
   simpleQueryOut_t *simpleQueryOut;

   memset (&simpleQueryInp, 0, sizeof (simpleQueryInp_t));

   simpleQueryInp.control = 0;
   simpleQueryInp.form = 2;
   simpleQueryInp.sql = "select * from r_resc_main where resc_name=?";
   simpleQueryInp.arg1 = "foo";
   simpleQueryInp.maxBufSize = 1024;

   status = rcSimpleQuery(Conn, &simpleQueryInp, &simpleQueryOut);

   if (status==CAT_NO_ROWS_FOUND) status=0; /* success */

   return(status);
}

int
generalAdmin(char *arg0, char *arg1, char *arg2, char *arg3, 
	     char *arg4, char *arg5, char *arg6, char *arg7) {
   generalAdminInp_t generalAdminInp;
   int status;
   char *mySubName;
   char *myName;

   generalAdminInp.arg0 = arg0;
   generalAdminInp.arg1 = arg1;
   generalAdminInp.arg2 = arg2;
   generalAdminInp.arg3 = arg3;
   generalAdminInp.arg4 = arg4;
   generalAdminInp.arg5 = arg5;
   generalAdminInp.arg6 = arg6;
   generalAdminInp.arg7 = arg7;
   generalAdminInp.arg8 ="";
   generalAdminInp.arg9 ="";

   status = rcGeneralAdmin(Conn, &generalAdminInp);
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
      rodsLog (LOG_ERROR, "rcGeneralAdmin failed with error %d %s %s",
	       status, myName, mySubName);
      if (status == CAT_INVALID_USER_TYPE) {
	 printf("See 'lt user_type' for a list of valid user types.\n");
      }
   }
   return(status);
}

/* 
 Prompt for input and parse into tokens
*/
int
getInput(char *cmdToken[], int maxTokens) {
   int lenstr, i;
   static char ttybuf[BIG_STR];
   int nTokens;
   int tokenFlag; /* 1: start reg, 2: start ", 3: start ' */
   char *cpTokenStart;

   memset(ttybuf, 0, BIG_STR);
   fputs("iadmin>",stdout);
   fgets(ttybuf, BIG_STR, stdin);
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
	 return(0);
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
   return(0);
}

/* handle a command,
   return code is 0 if the command was (at least partially) valid,
   -1 for quitting,
   -2 for if invalid
   -3 if empty.
 */
int
doCommand(char *cmdToken[]) {
   char buf0[MAX_PASSWORD_LEN+10];
   char buf1[MAX_PASSWORD_LEN+10];
   char buf2[MAX_PASSWORD_LEN+10];
   if (veryVerbose) {
      int i;
      printf("executing command:");
      for (i=0;i<20 && strlen(cmdToken[i])>0;i++) {
	 printf(" %s",cmdToken[i]);
      }
      printf("\n");
      fflush(stderr);
      fflush(stdout);
   }

   if (strcmp(cmdToken[0],"help")==0 ||
	      strcmp(cmdToken[0],"h") == 0) {
      usage(cmdToken[1]);
      return(0);
   }
   if (strcmp(cmdToken[0],"quit")==0 ||
	      strcmp(cmdToken[0],"q") == 0) {
      return(-1);
   }
   if (strcmp(cmdToken[0],"lu") == 0) {
      showUser(cmdToken[1]);
      return(0);
   }
   if (strcmp(cmdToken[0],"lt") == 0) {
      showToken(cmdToken[1], cmdToken[2]);
      return(0);
   }
   if (strcmp(cmdToken[0],"lr") == 0) {
      showResc(cmdToken[1]);
      return(0);
   }
   if (strcmp(cmdToken[0],"ls") == 0) {
      showDir(cmdToken[1]);
      return(0);
   }
   if (strcmp(cmdToken[0],"lf") == 0) {
      showFile(cmdToken[1]);
      return(0);
   }
   if (strcmp(cmdToken[0],"lz") == 0) {
      showZone(cmdToken[1]);
      return(0);
   }
   if (strcmp(cmdToken[0],"lg") == 0) {
      showGroup(cmdToken[1]);
      return(0);
   }
   if (strcmp(cmdToken[0],"lgd") == 0) {
      if (*cmdToken[1]=='\0') { 
	 printf("You must specify a group with the lgd command\n");
      }
      else {
	 showUser(cmdToken[1]);
      }
      return(0);
   }
   if (strcmp(cmdToken[0],"lrg") == 0) {
      showRescGroup(cmdToken[1]);
      return(0);
   }
   if (strcmp(cmdToken[0],"mkuser") == 0) {
      /*
      int status;
      No more, at least for now:
      char userName[NAME_LEN];
      char zoneName[NAME_LEN];
      status = parseUserName(cmdToken[1], userName, zoneName);
      if (status) {
	 printf("Invalid user name format");
	 return(0);
      }
      */
      generalAdmin("add", "user", cmdToken[1], cmdToken[2], 
      		   cmdToken[3], cmdToken[4], cmdToken[5], cmdToken[6]);
      return(0);
   }
   if (strcmp(cmdToken[0],"moduser") == 0) {
      if (strcmp(cmdToken[2],"password") ==0) {
	 int i, len, lcopy;
	 struct stat statbuf;
	 /* this is a random string used to pad, arbitrary, but must match
	    the server side: */
	 char rand[]="1gCBizHWbwIYyWLoysGzTe6SyzqFKMniZX05faZHWAwQKXf6Fs"; 

	 strncpy(buf0, cmdToken[3], MAX_PASSWORD_LEN);
	 len = strlen(cmdToken[3]);
	 lcopy = MAX_PASSWORD_LEN-10-len;
	 if (lcopy > 15) {  /* server will look for 15 characters of random string */
	    strncat(buf0, rand, lcopy);
	 }
   	 i = obfGetPw(buf1);
	 if (i !=0) {
#ifndef _WIN32
	    if (stat ("/bin/stty", &statbuf) == 0) {
	       system("/bin/stty -echo");
	    }
	    printf("Enter your current iRODS password:");
	    fgets(buf1, MAX_PASSWORD_LEN, stdin);
	    system("/bin/stty echo");
	    printf("\n");
	    buf1[strlen(buf1)-1]='\0'; /* remove trailing \n */
#else
	    printf("Error %d getting password\n", i);
	    return(0);
#endif
	 }
	 obfEncodeByKey(buf0, buf1, buf2);
	 cmdToken[3]=buf2;
      }
      generalAdmin("modify", "user", cmdToken[1], cmdToken[2],
		  cmdToken[3], cmdToken[4], cmdToken[5], cmdToken[6]);
      return(0);
   }
   if (strcmp(cmdToken[0],"mkdir") == 0) {
      generalAdmin("add", "dir", cmdToken[1], cmdToken[2], 
		  cmdToken[3], cmdToken[4], cmdToken[5], cmdToken[6]);
      return(0);
   }

   if (strcmp(cmdToken[0],"mkresc") ==0) {
      generalAdmin("add", "resource", cmdToken[1], cmdToken[2], 
		  cmdToken[3], cmdToken[4], cmdToken[5], cmdToken[6]);
      /* (add resource name type class host path zone) */
      return(0);
   }
   if (strcmp(cmdToken[0],"modresc") ==0) {
      generalAdmin("modify", "resource", cmdToken[1], cmdToken[2], 
		  cmdToken[3], "", "", "");
      return(0);
   }
   if (strcmp(cmdToken[0],"mkzone") == 0) {
      generalAdmin("add", "zone", cmdToken[1], cmdToken[2], 
		  cmdToken[3], cmdToken[4], "", "");
      return(0);
   }
   if (strcmp(cmdToken[0],"modzone") == 0) {
      generalAdmin("modify", "zone", cmdToken[1], cmdToken[2], 
		  cmdToken[3], "", "", "");
      return(0);
   }
   if (strcmp(cmdToken[0],"rmzone") == 0) {
      generalAdmin("rm", "zone", cmdToken[1], "",
		   "", "", "", "");
      return(0);
   }

   if (strcmp(cmdToken[0],"mkgroup") == 0) {
      generalAdmin("add", "user", cmdToken[1], "rodsgroup",
		   myEnv.rodsZone, "", "", "");
      return(0);
   }
   if (strcmp(cmdToken[0],"rmgroup") == 0) {
      generalAdmin("rm", "user", cmdToken[1], 
		   myEnv.rodsZone, "", "", "", "");
      return(0);
   }

   if (strcmp(cmdToken[0],"atg") == 0) {
      generalAdmin("modify", "group", cmdToken[1], "add", cmdToken[2],
		   cmdToken[3], "", "");
      return(0);
   }

   if (strcmp(cmdToken[0],"rfg") == 0) {
      generalAdmin("modify", "group", cmdToken[1], "remove", cmdToken[2],
		   cmdToken[3], "", "");
      return(0);
   }

   if (strcmp(cmdToken[0],"atrg") == 0) {
      generalAdmin("modify", "resourcegroup", cmdToken[1], "add", cmdToken[2],
		   "", "", "");
      return(0);
   }

   if (strcmp(cmdToken[0],"rfrg") == 0) {
      generalAdmin("modify", "resourcegroup", cmdToken[1], "remove", 
		   cmdToken[2],  "", "", "");
      return(0);
   }

   if (strcmp(cmdToken[0],"rmresc") ==0) {
      generalAdmin("rm", "resource", cmdToken[1], cmdToken[2], 
		  cmdToken[3], cmdToken[4], cmdToken[5], cmdToken[6]);
      return(0);
   }
   if (strcmp(cmdToken[0],"rmdir") ==0) {
      generalAdmin("rm", "dir", cmdToken[1], cmdToken[2], 
		  cmdToken[3], cmdToken[4], cmdToken[5], cmdToken[6]);
      return(0);
   }
   if (strcmp(cmdToken[0],"rmuser") ==0) {
      int status;
      char userName[NAME_LEN];
      char zoneName[NAME_LEN];
      status = parseUserName(cmdToken[1], userName, zoneName);
      if (status) {
	 printf("Invalid user name format");
	 return(0);
      }
      generalAdmin("rm", "user", userName, zoneName, 
	 cmdToken[2], cmdToken[3], cmdToken[4], cmdToken[5]); 
      return(0);
   }
   if (strcmp(cmdToken[0],"at") == 0) {
      generalAdmin("add", "token", cmdToken[1], cmdToken[2], 
		   cmdToken[3], cmdToken[4], cmdToken[5], cmdToken[6]);
      return(0);
   }
   if (strcmp(cmdToken[0],"rt") == 0) {
      generalAdmin("rm", "token", cmdToken[1], cmdToken[2],
		   cmdToken[3], cmdToken[4], cmdToken[5], cmdToken[6]);
      return(0);
   }
   if (strcmp(cmdToken[0],"spass") ==0) {
      char scrambled[MAX_PASSWORD_LEN+100];
      if (strlen(cmdToken[1])>MAX_PASSWORD_LEN-2) {
	 printf("Password exceeds maximum length\n");
      }
      else {
	 if (strlen(cmdToken[2])==0) {
	    printf("Warning, scramble key is null\n");
	 }
	 obfEncodeByKey(cmdToken[1], cmdToken[2], scrambled);
	 printf("Scrambled form is:%s\n", scrambled);
      }
      return(0);
   }
   if (strcmp(cmdToken[0],"dspass")==0) {
      char unscrambled[MAX_PASSWORD_LEN+100];
      if (simpleQueryCheck() != 0) {
	 exit(-1); /* not authorized */
      }
      if (strlen(cmdToken[1])>MAX_PASSWORD_LEN-2) {
	 printf("Scrambled password exceeds maximum length\n");
      }
      else {
	 if (strlen(cmdToken[2])==0) {
	    printf("Warning, scramble key is null\n");
	 }
	 obfDecodeByKey(cmdToken[1], cmdToken[2], unscrambled);
	 printf("Unscrambled form is:%s\n", unscrambled);
      }
      return(0);
   }
   if (strcmp(cmdToken[0],"pv") == 0) {
      generalAdmin("pvacuum", cmdToken[1], cmdToken[2], "", ""
		   "", "", "", "");
      return(0);
   }
   if (strcmp(cmdToken[0],"ctime") == 0) {
      char myString[20];
      if (strcmp(cmdToken[1],"str") == 0) {
	 int status;
	 status=checkDateFormat(cmdToken[2]);
	 if (status) {
	    rodsLogError(LOG_ERROR, status, "ctime str:checkDateFormat error");
	 }
	 printf("Converted to local iRODS integer time: %s\n", cmdToken[2]);
	 return(0);
      }
      if (strcmp(cmdToken[1],"now") == 0) {
	 char nowString[100];
	 getNowStr(nowString);
	 printf("Current time as iRODS integer time: %s\n", nowString);
	 return(0);
      }
      getLocalTimeFromRodsTime(cmdToken[1], myString);
      printf("Converted to local time: %s\n", myString);
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

   status = parseCmdLineOpt (argc, argv, "vVh", 0, &myRodsArgs);
   if (status) {
      printf("Use -h for help.\n");
      exit(2);
   }
   if (myRodsArgs.help==True) {
      usage("");
      exit(0);
   }
   argOffset = myRodsArgs.optind;

   status = getRodsEnv (&myEnv);
   if (status < 0) {
      rodsLog (LOG_ERROR, "main: getRodsEnv error. status = %d",
	       status);
      exit (1);
   }

   if (myRodsArgs.veryVerbose==True) {
      veryVerbose=1;
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

   if (strcmp(cmdToken[0],"spass") ==0) {
      char scrambled[MAX_PASSWORD_LEN+100];
      if (strlen(cmdToken[1])>MAX_PASSWORD_LEN-2) {
	 printf("Password exceeds maximum length\n");
      }
      else {
	 if (strlen(cmdToken[2])==0) {
	    printf("Warning, scramble key is null\n");
	 }
	 obfEncodeByKey(cmdToken[1], cmdToken[2], scrambled);
	 printf("Scrambled form is:%s\n", scrambled);
      }
      exit(0);
   }

   if (strcmp(cmdToken[0],"ctime") == 0) {
      char myString[20];
      if (strcmp(cmdToken[1],"str") == 0) {
	 int status;
	 status=checkDateFormat(cmdToken[2]);
	 if (status) {
	    rodsLogError(LOG_ERROR, status, "ctime str:checkDateFormat error");
	 }
	 printf("Converted to local iRODS integer time: %s\n", cmdToken[2]);
	 exit(0);
      }
      if (strcmp(cmdToken[1],"now") == 0) {
	 char nowString[100];
	 getNowStr(nowString);
	 printf("Current time as iRODS integer time: %s\n", nowString);
	 exit(0);
      }
      getLocalTimeFromRodsTime(cmdToken[1], myString);
      printf("Converted to local time: %s\n", myString);
      exit(0);
   }

   /* need to copy time convert commands up here too */

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
	 if (status==-2) keepGoing=0;
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

void usageMain()
{
   char *Msgs[]={
"Usage: iadmin [-hvV] [command]",
"A blank execute line invokes the interactive mode, where it",
"prompts and executes commands until 'quit' or 'q' is entered.",
"Single or double quotes can be used to enter items with blanks.",
"Commands are:",
" lu [name] (list user info; details if name entered)",
" lt [name] [subname] (list token info)",
" lr [name] (list resource info)",
" ls [name] (list directory: subdirs and files)",
" lz [name] (list zone info)",
" lg [name] (list group info (user member list))",
" lgd name  (list group details)",
" lrg [name] (list resource group info)",
" lf DataId (list file details; DataId is the number (from ls))",
" mkuser Name Type [zone] [DN] (make user)",
" moduser Name [ type | zone | DN | comment | info | password ] newValue",
" rmuser Name (remove user, where userName: name[@department][#zone])",
" mkdir Name [username] (make directory(collection))",
" rmdir Name (remove directory) ",
" mkresc Name Type Class Host Path (make Resource)",
" modresc Name [type, class, host, path, comment, info, freespace] Value (mod Resc)",
" rmresc Name (remove resource)",
" mkzone Name Type(remote) [Connection-info] [Comment] (make zone)",
" modZone Name [ name | conn | comment ] newValue  (modify zone)",
" rmZone Name (remove zone)",
" mkgroup Name (make group)",
" rmgroup Name (remove group)",
" atg groupName userName [userZone] (add to group - add a user to a group)",
" rfg groupName userName [userZone] (remove from group - remove a user from a group)",
" atrg resourceGroupName resourceName (add (resource) to resource group)",
" rfrg resourceGroupName resourceName (remove (resource) from resource group)",
" at tokenNamespace Name [Value1] [Value2] [Value3] (add token) ",
" rt tokenNamespace Name [Value1] (remove token) ",
" spass Password Key (print a scrambled form of a password for DB)",
" dspass Password Key (descramble a password and print it)",
" pv [date-time] [repeat-time(minutes)] (initiate a periodic rule to vacuum the DB)",
" ctime Time (convert an iRODS time (integer) to local time; & other forms)",
" help (or h) [command] (this help, or more details on a command)",
"Also see 'irmtrash -M -u user' for the admin mode of removing trash.",
""};
   printMsgs(Msgs);
}

void
usage(char *subOpt)
{
   char *luMsgs[]={
"lu [name] (list user info; details if name entered)",
"list user information.  ",
"Just 'lu' will briefly list currently defined local users.",
"If you include a user name, more detailed information is provided.",
"Also see the iuserinfo command.",
""};
   char *ltMsgs[]={
"lt [name] [subname]",
"list token information.  ",
"Just 'lt' lists the types of tokens that are defined",
"If you include a tokenname, it will list the values that are",
"allowed for the token type.  For details, lt name subname, ",
"for example: lt data_type email",
"The sql wildcard character % can be used on the subname,",
"for example: lt data_type %DLL",
""};
   char *lrMsgs[]={
"lr [name] (list resource info)",
"Just 'lr' briefly lists the defined resources.",
"If you include a resource name, it will list more detailed information.",
""};
   char *lsMsgs[]={
"ls [name] (list directory: subdirs and files)",
"This was a test function used before we had the ils command.",
"It lists collections and data-objects in a somewhat different",
"way than ils.  This is seldom of value but has been left in for now.",
""};
   char *lzMsgs[]={
" lz [name] (list zone info)",
"Just 'lz' briefly lists the defined zone(s).",
"If you include a zone name, it will list more detailed information.",
""};
   char *lgMsgs[]={
" lg [name] (list group info (user member list))",
"Just 'lg' briefly lists the defined groups.",
"If you include a group name, it will list users who are",
"members of that group.",
""};

   char *lgdMsgs[]={
" lgd name (list group details)",
"Lists some details about the user group.",
""};
   char *lrgMsgs[]={
" lrg [name] (list resource group info)",
"Just 'lrg' briefly lists the defined resource groups.",
"If you include a resource group name, it will list resources that are",
"members of that resource group.",
""};
   char *lfMsgs[]={
" lf DataId (list file details; DataId is the number (from ls))",
"This was a test function used before we had the ils command.",
"It lists data-objects in a somewhat different",
"way than ils.  This is seldom of value but has been left in for now.",

""};
   char *mkuserMsgs[]={
" mkuser Name Type [Zone] [DN] (make user)",
"Create a new iRODS user in the ICAT database",
" ",
"Name is the user name to create",
"Type is the user type (see 'lt user_type' for a list)",
"Zone is the user's zone (optional for local zone users)",
"DN is the Distinguished Name for GSI authentication (optional)",
" ",
"Tip: Use moduser to set a password, DN or other attributes of the user account.",
""};

   char *atrgMsgs[]={
" atrg resourceGroupName resourceName",
"Add a resource to a resourceGroup",
"If a resourceGroup by that name does not exist, it is created",
""};

   char *rfrgMsgs[]={
" rfrg resourceGroupName resourceName (remove (resource) from resource group)",
"Remove a resource to a resourceGroup",
"If this is the last resource in the group, the resourceGroup is removed",
""};

   char *spassMsgs[]={
" spass Password Key (print a scrambled form of a password for DB)",
"Scramble a password, using the input password and key.",
"This is used during the installation for a little additional security",
""};

   char *dspassMsgs[]={
" dspass Password Key (descramble a password and print it)",
"Descramble a password, using the input scrambled password and key",
""};


   char *moduserMsgs[]={
" moduser Name [ type | zone | DN | comment | info | password ] newValue",
"Modifies a field of an existing user definition.",
"For GSI authentication, the DN can also be entered via mkuser.",
"For password authentication, use moduser to set the password.",
"(The password is transferred in a scrambled form to be more secure.)",
"Long forms of the field names may also be used:",
"user_name, user_type_name, zone_name, user_distin_name, user_info, or ",
"r_comment",
"These are the names listed by 'lu' (and are the database table column names).",
"Modifying the user's name (user_name) is not allowed; instead remove the user",
"and create a new one.  rmuser/mkuser will remove (if empty) and create the needed",
"collections too.",
""};
   char *rmuserMsgs[]={
" rmuser Name (remove user, where userName: name[@department][#zone])",
" Remove an irods.",
""};

   char *mkdirMsgs[]={
" mkdir Name (make directory(collection))",
"This is similar to imkdir but is used during the installation process.",
"There is also a form 'mkdir Name Username' which makes a collection",
"that is owned by user Username.",
""};

   char *rmdirMsgs[]={
" rmdir Name (remove directory) ",
"This is similar to 'irm -f'.",
""};

   char *mkrescMsgs[]={
" mkresc Name Type Class Host Path (make Resource)",
"Create (register) a new storage resource.",
" ",
"Name is the name of the new resource.",
"Type is the resource type (see 'lt resc_type' for a list).",
"Class is the usage class of the resource (see 'lt resc_class').",
"Host is the DNS host name.",
"And Path is the defaultPath for the vault.",
" ",
"Tip: Also see the lt command for Type and Class token information.",
""};

   char *modrescMsgs[]={
" modresc Name [type, class, host, path, comment, info, or freespace] Value",
"         (modify Resource)",
"Change some attribute of a resource.  For example:",
"    modresc demoResc comment 'test resource'",
"The 'host' field is the DNS host name, for example 'datastar.sdsc.edu',",
"this is displayed as 'resc_net', the resource network address.",
""};

   char *rmrescMsgs[]={
" rmresc Name (remove resource)",
"Remove a storage resource.",
""};

   char *mkzoneMsgs[]={
" mkzone Name Type(remote) [Connection-info] [Comment] (make zone)",
"Create a new zone definition.  Type must be 'remote' as the local zone",
"must previously exist and there can be only one local zone definition.",
"Connection-info (hostname:port) and a Comment field are optional.",
"Also see modzone, rmzone, and lz.",
""};

   char *modzoneMsgs[]={
" modZone Name [ name | conn | comment ] newValue  (modify zone)",
"Modify values in a zone definition, either the name, conn (connection-info),",
"or comment.  Connection-info is the DNS host string:port, for example:",
"zuri.unc.edu:1247",
"The name of the local zone cannot currently be changed as it would require",
"many other changes to various tables and to user-environment and",
"configuration files.",
""};

   char *rmzoneMsgs[]={
" rmZone Name (remove zone)",
"Remove a zone definition.",
"Only remote zones can be removed.",
""};

   char *mkgroupMsgs[]={
" mkgroup Name (make group)"
"Create a user group.",
"Also see atg, rfg, and rmgroup.",
""};

   char *rmgroupMsgs[]={
" rmgroup Name (remove group)",
"Remove a user group.",
"Also see mkgroup, atg, and rfg.",
""};

   char *atgMsgs[]={
" atg groupName userName [userZone] (add to group - add a user to a group)",
"For remote-zone users, include the userZone.",
"Also see mkgroup, rfg and rmgroup.",
" ",
""};

   char *rfgMsgs[]={
" rfg groupName userName [userZone] (remove from group - remove a user from a group)",
"For remote-zone users, include the userZone.",
"Also see mkgroup, afg and rmgroup.",
""};

   char *atMsgs[]={
" at tokenNamespace Name [Value1] [Value2] [Value3] [comment] (add token) ",
"Add a new token.  The most common use of this is to add",
"data_type or user_type tokens.  See lt to display currently defined tokens.",
""};

   char *rtMsgs[]={
" rt tokenNamespace Name [Value] (remove token) ",
"Remove a token.  The most common use of this is to remove",
"data_type or user_type tokens.  See lt to display currently defined tokens.",
""};

   char *pvMsgs[]={
" pv [date-time] [minutes] (initiate a periodic rule to vacuum the DB)",
"The pv command will shutdown your irods Servers (if they have been",
"inactive a while), perform a db vacuum, and then restart them.",
"The date-time value is the time of day to run the first time,",
"for example 2008-05-07.23:00:00 .",
"The minutes value is the time between each subsequent run.",
"For example, you would use 1440 (24*60) to run it daily at the same time.",
"'pv 2008-05-07.23:59:00 1440' will run the pv rule/script each night one",
"minute before midnight.",
"With no arguments, pv will run the rule now and only once.",
"Without a minutes argument, pv will run the rule only once.",
"Run iqstat to view the queued rule.",
"See the vacuumdb.pl script (which is run by the rule) for details.",
""};

   char *ctimeMsgs[]={
" ctime Time (convert a iRODSTime value (integer) to local time",
"Time values (modify times, access times) are stored in the database",
"as a Unix Time value.  This is the number of seconds since 1970 and",
"is the same in all time zones (basically, Coordinated Universal Time).",
"ils and other utilities will convert it before displaying it, but iadmin",
"displays the actual value in the database.  You can enter the value to",
"the ctime command to convert it to your local time.  The following two",
"additional forms can also be used:",
" ",
" ctime now   convert a current time to an irods time integer value.",
" ",
" ctime str Timestr  - convert a string time string (YYYY-MM-DD.hh:mm:ss)",
" to an irods integer value time.",
" ",
""};

   char *helpMsgs[]={
" help (or h) [command] (general help, or more details on a command)",
" If you specify a command, a brief description of that command",
" will be displayed.",
""};

   /*
   char *noMsgs[]={
      "Help has not been written yet",
      ""};
   */

   char *subCmds[]={"lu", "lt", "lr", "ls", "lz",
		    "lg", "lgd", "lrg", "lf", "mkuser",
		    "moduser", "rmuser", "mkdir", "rmdir", "mkresc",
		    "modresc", "rmresc", 
		    "mkzone", "modzone", "rmzone",
		    "mkgroup", "rmgroup", "atg",
		    "rfg", "atrg", "rfrg", "at", "rt", "spass", "dspass", 
		    "pv", "ctime", "help", "h",
		    ""};

   char **pMsgs[]={ luMsgs, ltMsgs, lrMsgs, lsMsgs, lzMsgs, 
		    lgMsgs, lgdMsgs, lrgMsgs, lfMsgs, mkuserMsgs, 
		    moduserMsgs, rmuserMsgs, mkdirMsgs, rmdirMsgs, mkrescMsgs, 
		    modrescMsgs, rmrescMsgs, 
		    mkzoneMsgs, modzoneMsgs, rmzoneMsgs,
		    mkgroupMsgs, rmgroupMsgs,atgMsgs, 
		    rfgMsgs, atrgMsgs, rfrgMsgs, atMsgs, rtMsgs, spassMsgs,
		    dspassMsgs, pvMsgs, ctimeMsgs, helpMsgs, helpMsgs };

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
