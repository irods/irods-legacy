/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 
 * iput - The irods put utility
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "putUtil.h"
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
    

    optStr = "aD:fhkKn:N:p:rR:UvVX:";
   
    status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);

    if (status < 0) {
	printf("use -h for help.\n");
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
      UNKNOWN_FILE_T, UNKNOWN_OBJ_T, 0, &rodsPathInp);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: parseCmdLinePath error. "); 
	printf("use -h for help.\n");
        exit (1);
    }

    conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
      myEnv.rodsZone, 1, &errMsg);

    if (conn == NULL) {
        exit (2);
    }
   
    status = clientLogin(conn);
    if (status != 0) {
       rcDisconnect(conn);
        exit (7);
    }

    status = putUtil (conn, &myEnv, &myRodsArgs, &rodsPathInp);

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
"Usage : iput [-fkKrUvV] [-D dataType] [-N numThreads] [-n replNum]",
"             [-p physicalPath] [-R resource] [-X restartFile]", 
"		localSrcFile|localSrcDir ...  destDataObj|destColl",
"Usage : iput [-fkKUvV] [-D dataType] [-N numThreads] [-n replNum] ",
"             [-p physicalPath] [-R resource] [-X restartFile] localSrcFile",
" ",
"Store a file into iRODS.  If the destination data-object or collection are",
"not provided, the current irods directory and the input file name are used.",
"The -X option specifies that the restart option is on and the restartFile",
"input specifies a local file that contains the restart info. If the ",
"restartFile does not exist, it will be created and used for recording ",
"subsequent restart info. If it exists and is not empty, the restart info",
"contained in this file will be used for restarting the operation.",
"Note that the restart operation only works for uploading directories and",
"the path input must be identical to the one that generated the restart file", 
" ",
"If the options -f is used to overwrite an existing data-object, the copy",
"in the resource specified by the -R option will be picked if it exists.",
"Otherwise, one of the copy in the other resources will be picked for the",
"overwrite. Note that a copy will not be made in the specified resource",
"if a copy in the specified resource does not already exist. The irepl",
"command should be used to make a replica of an existing copy.", 
" ",
"Options are:",
" -D  dataType - the data type string",
" -f  force - write data-object even it exists already; overwrite it",
" -k  checksum - calculate a checksum on the data",
" -K  verify checksum - calculate and verify the checksum on the data",
" -N  numThreads - the number of thread to use for the transfer. A value of",
"       0 means no threading. By default (-N option not used) the server ",
"       decides the number of threads to use.",
" -R  resource - specifies the resource to store to. This can also be specified",
"     in your environment or via a rule set up by the administrator.",
" -r  recursive - store the whole subdirectory",
" -U  use RBUDP protocol for the data transfer",
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
