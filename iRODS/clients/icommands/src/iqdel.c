/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* 
  A user interface for deleting delayed execution rules
*/

#include "rodsClient.h"
#include "parseCommandLine.h"

#define MAX_SQL 300
#define BIG_STR 200

void usage();

int debug=0;

rcComm_t *Conn;
rodsEnv myEnv;

int
rmDelayedRule(char *ruleId) {
   int status;
   char *mySubName;
   char *myName;

   ruleExecDelInp_t ruleExecDelInp;
   
   strncpy(ruleExecDelInp.ruleExecId, ruleId, NAME_LEN);
   status = rcRuleExecDel(Conn, &ruleExecDelInp);

   if (status == CAT_SUCCESS_BUT_WITH_NO_INFO) {
      printf("No rule found with id %s\n",ruleId);
   }
   if (status < 0) {
      printError(Conn, status, "rcRuleExecDel");
   }
   return(status);
}

int
main(int argc, char **argv) {
   int status;
   rErrMsg_t errMsg;

   rodsArguments_t myRodsArgs;
   int argOffset;
   int i;

   rodsLogLevel(LOG_ERROR);  /* This should be the default someday */

   status = parseCmdLineOpt (argc, argv, "vVh", 0, &myRodsArgs);
   if (status) {
      printf("Use -h for help\n");
      exit(1);
   }
   if (myRodsArgs.help==True) {
      usage();
      exit(0);
   }
   argOffset = myRodsArgs.optind;

   status = getRodsEnv (&myEnv);
   if (status < 0) {
      rodsLog (LOG_ERROR, "main: getRodsEnv error. status = %d",
	       status);
      exit (1);
   }

   if (argc <= argOffset) {
      usage();
      exit(-1);
   }
   Conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
                     myEnv.rodsZone, 0, &errMsg);
   if (Conn == NULL) {
      char *mySubName;
      char *myName;
      myName = rodsErrorName(errMsg.status, &mySubName);
      rodsLogError(LOG_ERROR, errMsg.status, "rcConnect failure");
      rodsLog(LOG_ERROR, "rcConnect failure %s (%s) (%d) %s",
	      myName,
	      mySubName,
	      errMsg.status,
	      errMsg.msg);
      exit (2);
   }

   status = clientLogin(Conn);
   if (status != 0) {
      printError(Conn, status, "clientLogin");
      if (!debug) exit (3);
   }

   for (i=argOffset;i<argc;i++) {
      status = rmDelayedRule(argv[i]);
   }

   rcDisconnect(Conn);

   if (status) exit(4);
   exit(0);
}

void usage()
{
   printf("Usage: iqdel [-vVh] ruleId [...]\n");
   printf("\n");
   printf(" iqdel removes delayed rules from the queue.\n");
   printf(" multiple ruleIds may be included on the command line.\n");
   printf("\n");
   printf("Also see iqstat and iqmod.\n");

}
