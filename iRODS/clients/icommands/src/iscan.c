/*** Copyright (c), CCIN2P3            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* Written by Jean-Yves Nief of CCIN2P3 */
#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "scanUtil.h" 
void usage ();

int
main(int argc, char **argv) {
    int status;
    rodsEnv myEnv;
    rErrMsg_t errMsg;
    rcComm_t *conn;
    rodsArguments_t myRodsArgs;
    char *optStr, hostname[LONG_NAME_LEN];
    rodsPathInp_t rodsPathInp;
    

    optStr = "hr";
   
    status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);

    if (status < 0) {
        printf("Use -h for help\n");
        exit (1);
    }
    if (myRodsArgs.help==True) {
       usage();
       exit(0);
    }

    status = getRodsEnv (&myEnv);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
        exit (1);
    }

    status = parseCmdLinePath (argc, argv, optind, &myEnv,
      UNKNOWN_FILE_T, NO_INPUT_T, 0, &rodsPathInp);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: parseCmdLinePath error. ");
	usage ();
        exit (1);
    }

    conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
      myEnv.rodsZone, 0, &errMsg);

    if (conn == NULL) {
        exit (2);
    }

    if (strcmp (myEnv.rodsUserName, PUBLIC_USER_NAME) != 0) { 
        status = clientLogin(conn);
        if (status != 0) {
           rcDisconnect(conn);
           exit (7);
	}
    }
   
	status = gethostname(hostname, LONG_NAME_LEN);
	if ( status < 0 ) {
		printf ("cannot resolve server name, aborting!\n");
		exit(4);
	}
	
	status = scanObj (conn, &myRodsArgs, &rodsPathInp, hostname);

    rcDisconnect(conn);

    exit(status);

}

void
usage () {
   char *msgs[]={
"Usage : iscan [-rh] srcPhysicalFile|srcPhysicalDirectory ... ",
"Check if a local data object or a local collection content is registered in irods.",
"It allows to detect orphan files, srcPhysicalFile or srcPhysicalDirectory must be a full path name.",
"Options are:",
" -r  recursive - scan local subdirectories",
" -h  this help",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) break;
      printf("%s\n",msgs[i]);
   }
   printReleaseInfo("iscan");
}
