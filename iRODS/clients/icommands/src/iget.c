/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 
 * iget - The irods get utility
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "getUtil.h"
void usage ();

int
main(int argc, char **argv) {
    int status;
    rodsEnv myEnv;
    rErrMsg_t errMsg;
    rcComm_t *conn;
    rodsArguments_t myRodsArgs;
    char *optStr;
    rodsPathInp_t rodsPathInp;
    

    optStr = "hfKN:n:rUvVX:R:";
   
    status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);

    if (status < 0) {
        printf("Use -h for help.\n");
        exit (1);
    }
    if (myRodsArgs.help==True) {
       usage();
       exit(0);
    }

    status = getRodsEnv (&myEnv);

    if (status < 0) {
        rodsLogError(LOG_ERROR, status, "main: getRodsEnv error. ");
        exit (1);
    }

    status = parseCmdLinePath (argc, argv, optind, &myEnv,
      UNKNOWN_OBJ_T, UNKNOWN_FILE_T, 0, &rodsPathInp);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: parseCmdLinePath error. ");
        printf("Use -h for help.\n");
        exit (1);
    }

    conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
      myEnv.rodsZone, 1, &errMsg);

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

    status = getUtil (conn, &myEnv, &myRodsArgs, &rodsPathInp);

    rcDisconnect(conn);

    if (status < 0) {
	exit (3);
    } else {
        exit(0);
    }

}

void
usage () {
   char *msgs[]={
"Usage: iget [-fKrUvV] [-n replNumber] [-N numThreads] [-X restartFile]",
"[-R resource] srcDataObj|srcCollection ... destLocalFile|destLocalDir",
"Usage : iget [-fKUvV] [-n replNumber] [-N numThreads] [-X restartFile]",
"[-R resource] srcDataObj|srcCollection",
"Usage : iget [-fKUvV] [-n replNumber] [-N numThreads] [-X restartFile]",
"[-R resource] srcDataObj ... -",
"Get data-objects or collections from irods space, either to the specified",
"local area or to the current working directory.",
" ",
"If the destLocalFile is '-', the files read from the server will be ",
"written to the standard output (stdout). Similar to the UNIX 'cat'",
"command, multiple source files can be specified.",
" ",
"The -X option specifies that the restart option is on and the restartFile",
"input specifies a local file that contains the restart info. If the ",
"restartFile does not exist, it will be created and used for recording ",
"subsequent restart info. If it exists and is not empty, the restart info",
"contained in this file will be used for restarting the operation.",
"Note that the restart operation only works for uploading directories and",
"the path input must be identical to the one that generated the restart file",
" ",
"The -U option specifies the use of the RBUDP transfer mechanism which uses",
"the UDP protocol for data transfer. The UDP protocol is very efficient",
"if the network is very robust with few packet losses. Two environment",
"variables - rbudpSendRate and rbudpPackSize are used to tune the RBUDP",
"data transfer. rbudpSendRate is used to throttle the send rate in ",
"kbits/sec. The default rbudpSendRate is 600,000. rbudpPackSize is used",
"to set the packet size. The dafault rbudpPackSize is 8192.",
" ",
"Options are:",

" -f  force - write local files even it they exist already (overwrite them)",
" -K  verify the checksum",
" -n  replNumber - retrieve the copy with the specified replica number ",
" -N  numThreads - the number of thread to use for the transfer. A value of",
"       0 means no threading. By default (-N option not used) the server ",
"       decides the number of threads to use.", 
" -r  recursive - retrieve subcollections",
" -R  resource - the preferred resource",
" -U  use RBUDP protocol for the data transfer",
" -v  verbose",
" -V  Very verbose",
"     restartFile input specifies a local file that contains the restart info.",
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
