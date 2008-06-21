/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 
 * icp - The irods cp utility
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "cpUtil.h"
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
    

    optStr = "hfkKN:p:rR:vVX:";
   
    status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);
    if (status) {
       printf("Use -h for help.\n");
       exit(1);
    }
    if (myRodsArgs.help==True) {
       usage();
       exit(0);
    }

    if (argc - optind <= 1) {
        rodsLog (LOG_ERROR, "icp: no input");
	printf("Use -h for help.\n");
        exit (2);
    }

    status = getRodsEnv (&myEnv);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
        exit (1);
    }

    status = parseCmdLinePath (argc, argv, optind, &myEnv,
      UNKNOWN_OBJ_T, UNKNOWN_OBJ_T, 0, &rodsPathInp);

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
   
    status = clientLogin(conn);
    if (status != 0) {
       rcDisconnect(conn);
        exit (7);
    }

    status = cpUtil (conn, &myEnv, &myRodsArgs, &rodsPathInp);

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
   int i;

   char *msgs[]={
"Usage : icp [-fkKrvV] [-N numThreads] [-p physicalPath] [-R resource]", 
"-X restartFile] srcDataObj|srcColl ...  destDataObj|destColl",
"icp copies an irods data-object (file) or collection (directory) to another",
"data-object or collection.",  
"The -X option specifies that the restart option is on and the restartFile",
"input specifies a local file that contains the restart info. If the ",
"restartFile does not exist, it will be created and used for recording ",
"subsequent restart info. If it exists and is not empty, the restart info",
"contained in this file will be used for restarting the operation.",
"Note that the restart operation only works for uploading directories and",
"the path input must be identical to the one that generated the restart file",
"Options are:",
"-f force - write data-object even it exists already; overwrite it",
"-k checksum - calculate a checksum on the data",
"-K verify checksum - calculate and verify the checksum on the data",
"-N number  specifies the number of I/O threads to use, by default a rule",
"           is used to determine the best value.",
"-p physicalPath  specifies the path on the storage resource on which to store.",
"       Normally, you let the irods system automatically determine this.",
"-R resource - specifies the resource to store to. This can also be specified",
"       in your environment or via a rule set up by the administrator.",
"-r recursive - copy the whole subtree",
"-v verbose - display various messages while processing",
"-V very verbose",
" -X  restartFile - specifies that the restart option is on and the",
"     restartFile input specifies a local file that contains the restart info.",
" -h  this help",
""};
    for (i=0;;i++) {
       if (strlen(msgs[i])==0) break;
       printf("%s\n",msgs[i]);
    }
}

