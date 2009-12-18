/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* 
  User interface to display quota information.
*/

#include "rods.h"
#include "rodsClient.h"

#define MAX_SQL 300
#define BIG_STR 200

char cwd[BIG_STR];

int debug=0;

int printedFlag[3]={0,0,0};
char *header[3]={
   "User quotas:\n    Resource         User        Limit         Over     Time-set",
   "\nGroup quotas:       Group ",
   "Group quotas:\n    Resource        Group        Limit         Over     Time-set",
};
/*  123456789012|123456789012|123456789012|123456789012|123456789012| */

rcComm_t *Conn;
rodsEnv myEnv;


void usage();

/*
 Print a header if it hasn't already.
 */
int
printHdr(int ix) {
   if (printedFlag[ix]==0) {
      printedFlag[ix]=1;
      printf("%s\n", header[ix]);
      return(1);
   }
   return(0);
}

/* 
 print the results of a general query.
 */
int
printGenQueryResults(rcComm_t *Conn, int status, genQueryOut_t *genQueryOut, 
		     char *descriptions[])
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
	    for (j=0;j<genQueryOut->attriCnt;j++) {
	       char *tResult;
	       tResult = genQueryOut->sqlResult[j].value;
	       tResult += i*genQueryOut->sqlResult[j].len;
	       if (strstr(descriptions[j],"time")!=0) {
		  getLocalTimeFromRodsTime(tResult, localTime);
		  printf("%s: %s: %s\n", descriptions[j], tResult, 
			 localTime);
	       } 
	       else {
		  printf("%s: %s\n", descriptions[j], tResult);
	       }
	       printCount++;
	    }
	 }
      }
   }
   return(printCount);
}


/*
  Show user quota information
*/
int
showUserLimits(char *userName, int userOrGroup, int rescOrGlobal) 
{
   genQueryInp_t genQueryInp;
   genQueryOut_t *genQueryOut;
   int inputInx[20];
   int inputVal[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   int inputCond[20];
   char *condVal[10];
   char v1[BIG_STR];
   char v2[BIG_STR];
   char v3[BIG_STR];
   int i, j, k, status;
   char localTime[20];
   int printCount;

   char *pad[13]={"            ",
		  "           ",
		  "          ",
		  "         ",
		  "        ",
		  "       ",
		  "      ",
		  "     ",
		  "    ",
		  "   ",
		  "  ",
		  " ",
		  ""};

   memset (&genQueryInp, 0, sizeof (genQueryInp_t));
   printCount=0;
   i=0;
   if (rescOrGlobal==0) {
      inputInx[i++]=COL_QUOTA_RESC_NAME;
   }
   else {
      inputInx[i++]=COL_QUOTA_RESC_ID;
   }
   inputInx[i++]=COL_QUOTA_USER_NAME;
   inputInx[i++]=COL_QUOTA_LIMIT;
   inputInx[i++]=COL_QUOTA_OVER;
   inputInx[i++]=COL_QUOTA_MODIFY_TIME;

   genQueryInp.selectInp.inx = inputInx;
   genQueryInp.selectInp.value = inputVal;
   genQueryInp.selectInp.len = i;

   genQueryInp.sqlCondInp.len=0;
   if (userName[0]!='\0') {
      inputCond[0]=COL_QUOTA_USER_NAME;
      sprintf(v1,"='%s'",userName);
      condVal[0]=v1;
      genQueryInp.sqlCondInp.len++;
   }
   inputCond[genQueryInp.sqlCondInp.len] = COL_QUOTA_USER_TYPE;
   if (userOrGroup==0) {
      sprintf(v2,"!='%s'","rodsgroup");
   }
   else {
      sprintf(v2,"='%s'","rodsgroup");
   }
   condVal[genQueryInp.sqlCondInp.len] = v2;
   genQueryInp.sqlCondInp.len++;

   if (rescOrGlobal==1) {
      inputCond[genQueryInp.sqlCondInp.len] = COL_QUOTA_RESC_ID;
      sprintf(v3, "='%s'", "0");
      condVal[genQueryInp.sqlCondInp.len] = v3;
      genQueryInp.sqlCondInp.len++;
   }


   genQueryInp.sqlCondInp.inx = inputCond;
   genQueryInp.sqlCondInp.value = condVal;

   genQueryInp.condInput.len=0;

   genQueryInp.maxRows=MAX_SQL_ROWS;

   genQueryInp.continueInx=0;
   status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
   if (status == CAT_NO_ROWS_FOUND) {
      if (rescOrGlobal==0) {
      if (userOrGroup==1) {
	 if (userName[0]!='\0') {
	    printf("No quotas set for group %s\n", userName);
	 }
	 else {
	    printf("No group quotas set\n");
	 }
      }
      else {
	 if (userName[0]!='\0') {
	    printf("No quotas set for user %s\n", userName);
	 }
	 else {
	    printf("No user quotas set\n");
	 }
      }
      }
      return(0);
   }
   if (status!=0) {
      printError(Conn, status, "rcGenQuery");
      return(status);
   }

   if (userOrGroup==0) { /* user */
      printHdr(0);
   }
   else {  /* group */
      if (printedFlag[0]==1) {
	 printHdr(1);
      }
      else {
	 printHdr(2);
      }
   }

   printCount=0;
   for (i=0;i<genQueryOut->rowCnt;i++) {
      for (j=0;j<genQueryOut->attriCnt;j++) {
	 char *tResult;
	 tResult = genQueryOut->sqlResult[j].value;
	 tResult += i*genQueryOut->sqlResult[j].len;
	 if (j==4) {
	    getLocalTimeFromRodsTime(tResult, localTime);
	    printf("%s ", localTime);
	 }
	 else {
	    if (rescOrGlobal==1 && j==0) {
	       tResult="All";
	    }
	    k = strlen(tResult);
	    if (k < 12) printf("%s",pad[k]);
	    printf("%s ",tResult);
	    printCount++;
	 }
      }
      printf("\n");
   }
   return (0);
}

/*
  Show user quota information
*/
int
showUserQuotas(char *userName, int mode) 
{
   genQueryInp_t genQueryInp;
   genQueryOut_t *genQueryOut;
   int inputInx[20];
   int inputVal[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   int inputCond[20];
   char *condVal[10];
   char v1[BIG_STR];
   int i, j, status;
   char localTime[20];
   int printCount;
   char *tResult;
/*
   char *columnNames[3][8]={
      {"resc", "user", "usage", "limit", "over", "modify time", ""},
      {"resc", "user", "user-type", "limit", "over", "modify time", ""},
      {"resc", "user", "user-type", "usage", "modify time", "", ""}
   };
*/
   char *header[3]={
      "h 1",
   "    Resource   User/Group    User-type        Limit         Over      Time-set",
      "Resource User    User-type  Usage"
   };
   char *pad[13]={"            ",
		  "           ",
		  "          ",
		  "         ",
		  "        ",
		  "       ",
		  "      ",
		  "     ",
		  "    ",
		  "   ",
		  "  ",
		  " ",
		  ""};

   memset (&genQueryInp, 0, sizeof (genQueryInp_t));
   printCount=0;
   i=0;
   if (mode ==0 ) {
      inputInx[i++]=COL_QUOTA_MODIFY_TIME;
      inputInx[i++]=COL_QUOTA_RESC_NAME;
      inputInx[i++]=COL_QUOTA_USER_NAME;
      inputInx[i++]=COL_QUOTA_USAGE;
      inputInx[i++]=COL_QUOTA_LIMIT;
      inputInx[i++]=COL_QUOTA_OVER;
   }
   if (mode ==1 ) {
      inputInx[i++]=COL_QUOTA_RESC_NAME;
      inputInx[i++]=COL_QUOTA_USER_NAME;
      inputInx[i++]=COL_QUOTA_USER_TYPE;
      inputInx[i++]=COL_QUOTA_LIMIT;
      inputInx[i++]=COL_QUOTA_OVER;
      inputInx[i++]=COL_QUOTA_MODIFY_TIME;
   }
   if (mode ==2 ) {
      inputInx[i++]=COL_QUOTA_USAGE_MODIFY_TIME;
      inputInx[i++]=COL_QUOTA_RESC_NAME;
      inputInx[i++]=COL_QUOTA_USER_NAME;
      inputInx[i++]=COL_QUOTA_USER_TYPE;
      inputInx[i++]=COL_QUOTA_USAGE;
   }

   genQueryInp.selectInp.inx = inputInx;
   genQueryInp.selectInp.value = inputVal;
   genQueryInp.selectInp.len = i;

   if (userName[0]!='\0') {
      inputCond[0]=COL_QUOTA_USER_NAME;
      sprintf(v1,"='%s'",userName);
      condVal[0]=v1;

      genQueryInp.sqlCondInp.inx = inputCond;
      genQueryInp.sqlCondInp.value = condVal;
      genQueryInp.sqlCondInp.len=1; //
   }
   else {
      genQueryInp.sqlCondInp.len=0;
   }

   genQueryInp.condInput.len=0;

   genQueryInp.maxRows=20;
   genQueryInp.continueInx=0;
   status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
   if (status == CAT_NO_ROWS_FOUND) {
      printf("No quotas set for user %s\n", userName);
      return(0);
   }
   if (status!=0) {
      printError(Conn, status, "rcGenQuery");
      return(status);
   }

   if (mode==1) {
      printf("%s\n", header[mode]);
      printCount=0;
      for (i=0;i<genQueryOut->rowCnt;i++) {
	 for (j=0;j<genQueryOut->attriCnt;j++) {
	    char *tResult;
	    int k;
	    tResult = genQueryOut->sqlResult[j].value;
	    tResult += i*genQueryOut->sqlResult[j].len;
	    if (j==5) {
	       getLocalTimeFromRodsTime(tResult, localTime);
	       printf("%s ", localTime);
       	    }
	    else {
	       k = strlen(tResult);
	       if (k < 12) printf("%s",pad[k]);
	       printf("%s ",tResult);
	       printCount++;
	    }
	 }
	 printf("\n");
      }
   }
   if (mode==2) {
      tResult = genQueryOut->sqlResult[0].value;
      getLocalTimeFromRodsTime(tResult, localTime);
      printf("Time usages set: %s\n", localTime);
      printf("%s\n", header[mode]);
      printCount=0;
      for (i=0;i<genQueryOut->rowCnt;i++) {
	 for (j=1;j<genQueryOut->attriCnt;j++) {
	    char *tResult;
	    tResult = genQueryOut->sqlResult[j].value;
	    tResult += i*genQueryOut->sqlResult[j].len;
	    printf("%s ",tResult);
	    printCount++;
	 }
	 printf("\n");
      }
   }

   return (0);
}

int
main(int argc, char **argv) {
   int status, nArgs;
   rErrMsg_t errMsg;
   char userName[NAME_LEN];
   int mode;

   rodsArguments_t myRodsArgs;

   rodsLogLevel(LOG_ERROR);

   status = parseCmdLineOpt (argc, argv, "au:vVh", 0, &myRodsArgs);
   if (status) {
      printf("Use -h for help.\n");
      exit(1);
   }
   if (myRodsArgs.help==True) {
      usage();
      exit(0);
   }

   status = getRodsEnv (&myEnv);
   if (status < 0) {
      rodsLog (LOG_ERROR, "main: getRodsEnv error. status = %d",
	       status);
      exit (1);
   }

   Conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
                     myEnv.rodsZone, 0, &errMsg);

   if (Conn == NULL) {
      exit (2);
   }

   status = clientLogin(Conn);
   if (status != 0) {
      if (!debug) exit (3);
   }

   strncpy(userName, myEnv.rodsUserName, NAME_LEN);
   if (myRodsArgs.user)  strncpy(userName, myRodsArgs.userString, NAME_LEN);
   if (myRodsArgs.all) userName[0]='\0';

   nArgs = argc - myRodsArgs.optind;

   mode = 0;
   if (nArgs > 0) {
      if ((strncmp(argv[myRodsArgs.optind],"limits",6)==0) ||
	  strncmp(argv[myRodsArgs.optind],"limit",5)==0) {
	 mode=1;
	 status = showUserLimits(userName, 0, 0); /* users, resc */
	 status = showUserLimits(userName, 0, 1); /* users, global */
	 status = showUserLimits("", 1, 0);       /* all groups, resc */
	 status = showUserLimits("", 1, 1);       /* all groups, global */
      }
      if (strncmp(argv[myRodsArgs.optind],"usage",5)==0) {
	 mode=2;
	 status = showUserQuotas(userName, mode);
      }
   }
   
   rcDisconnect(Conn);

   exit(0);
}

/*
Print the main usage/help information.
 */
void usage()
{
   char *msgs[]={
"Usage: iquota [-uavVh] [UserName] [usage] [limits]", 
" ", 
"Show information iRODS iquotas (if any)", 
"By default, information is displayed for the current iRODS user",
"Options",
"-a All users",
"-u UserName  - for the specified user",
"usage  - show only usage information",
"limits - show only the defined limits",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) break;
      printf("%s\n",msgs[i]);
   }
   printReleaseInfo("iquota");
}
