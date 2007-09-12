/*** Copyright (c), The Regents of the University of California            ***
 *** For more inforeplation please refer to files in the COPYRIGHT directory ***/
/* 
 * irepl - The irods repl utility
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "replUtil.h"
void usage ();


main(int argc, char **argv) {
    int status;
    rodsEnv myEnv;
    rErrMsg_t errMsg;
    rcComm_t *conn;
    rodsArguments_t myRodsArgs;
    char *optStr;
    rodsPathInp_t rodsPathInp;
    

    optStr = "aBMhrvVn:R:S:X:";
   
    status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);

    if (status < 0) {
        printf("Use -h for help.\n");
        exit (1);
    }

    if (myRodsArgs.help==True) {
       usage();
       exit(0);
    }

    if (argc - optind <= 0) {
        rodsLog (LOG_ERROR, "irepl: no input");
        printf("Use -h for help.\n");
        exit (2);
    }

    status = getRodsEnv (&myEnv);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
        exit (1);
    }

    status = parseCmdLinePath (argc, argv, optind, &myEnv,
      UNKNOWN_OBJ_T, NO_INPUT_T, 0, &rodsPathInp);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: parseCmdLinePath error. ");
        printf("Use -h for help.\n");
        exit (1);
    }

    conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
      myEnv.rodsZone, 1, &errMsg);

    if (conn == NULL) {
        rodsLogError (LOG_ERROR, errMsg.status, "rcConnect failure %s",
	       errMsg.msg);
        exit (2);
    }
   
    status = clientLogin(conn);
    if (status != 0) {
        rcDisconnect(conn);
        exit (7);
    }

    status = replUtil (conn, &myEnv, &myRodsArgs, &rodsPathInp);

    rcDisconnect(conn);

    if (status < 0) {
	exit (3);
    } else {
        exit(0);
    }

}

void 
usage ()
{

   char *msgs[]={
"Usage : irepl [-aBMrvV] [-n replNum] [-R destResource] [-S srcResource]",
"[-X restartFile]  dataObj|collection ... ",
" ",
"Replicate a file in iRODS to another storage resource.",
" ",
"The -X option specifies that the restart option is on and the restartFile",
"input specifies a local file that contains the restart info. If the ",
"restartFile does not exist, it will be created and used for recording ",
"subsequent restart info. If it exists and is not empty, the restart info",
"contained in this file will be used for restarting the operation.",
"Note that the restart operation only works for uploading directories and",
"the path input must be identical to the one that generated the restart file",
" ",
"Options are:",
" -a  all - only meaningful if input resource [-R resource] is a resource group.",
"     Replicate to all the resources in the resource group.",
" -B  Backup mode - if a good copy already exists in this",
"     resource, don't make another copy.",
" -M  admin - admin user uses this option to backup/replicate other users files",
" -r  recursive - copy the whole subtree",
" -n  replNum  - the replica to copy, typically not needed",
" -R  destResource - specifies the destination resource to store to.",
"     This can also be specified in your environment or via a rule set up", 
"     by the administrator.",
" -S  srcResource - specifies the source resource of the data object to be",
"     replicated. If specified, only copies stored in this resource will",
"     be replicated. Otherwise, one of the copy will be replicated",
" -v  verbose",
" -V  Very verbose",
" -X  restartFile - specifies that the restart option is on and the",
"     restartFile input specifies a local file that contains the restart info.",
" -h  this help",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) return;
      printf("%s\n",msgs[i]);
   }
}
