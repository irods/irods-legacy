/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 
 * ireg - The irods reg utility
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "regUtil.h"
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
    int nArgv;
    

    optStr = "D:hCbmR:vV";
   
    status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);

    if (status < 0) {
	printf("use -h for help.\n");
        exit (1);
    }

    if (myRodsArgs.help==True) {
       usage();
       exit(0);
    }

    nArgv = argc - optind;

    if (nArgv != 2) {      /* must have 2 inputs */
        usage (argv[0]);
        exit (1);
    }

    status = getRodsEnv (&myEnv);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
        exit (1);
    }

    if ((*argv[optind] != '/' && strcmp (argv[optind], UNMOUNT_STR) != 0) || 
      *argv[optind + 1] != '/') { 
	rodsLog (LOG_ERROR,
	 "Input path must be absolute");
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
        rodsLogError (LOG_ERROR, errMsg.status, "rcConnect failure %s",
	       errMsg.msg);
        exit (2);
    }
   
    status = clientLogin(conn);
    if (status != 0) {
       rcDisconnect(conn);
        exit (7);
    }

    status = regUtil (conn, &myEnv, &myRodsArgs, &rodsPathInp);

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
"Usage : ireg [-hCvV] [-D dataType] [-R resource] physicalFilePath",
"               irodsPath",
"Usage : ireg -m [-hvV] [-R resource] mountDirectory irodsCollection",
"Usage : ireg -m unmount irodsCollection",
"Usage : ireg -b [-R resource] irodsStructuredFilePath irodsCollection",
"NOTE: ireg -b has NOT yet been implemented.",
" ",
"Register a file or a directory of files and subdirectory into iRODS.",
"The file or the directory of files must already exist on the server where",
"the resource is located. Full path must be supplied for both physicalFilePath",
"and irodsPath",
" ",
"With the -C option, the entire content beneath the physicalFilePath",
"(files and subdirectories) will be recursively registered beneath the",
"irodsPath. For example, the command:",
" ",
"    ireg -C /tmp/src1 /tempZone/home/myUser/src1",
" ",
"grafts all files and subdirectories beneath the directory /tmp/src1 to",
"the collection /tempZone/home/myUser/src1", 
" ",
"The -m option can be used to associate (mount) an iRods collection with a",
"a physical directory (e.g.,a UNIX directory). Unlike the -C option, files",
"and subdirectories beneath the directory will not be registered. Only",
"the top level collection/directory will be registered. The content of",
"the registered directory can be accessed through the irods server.", 
"This is simlilar to a mounted file system where a UNIX directory is",
"mounted to a mount point (the registered collection).",
" ",
"For security reason, file permissions are checked to make sure that",
"the client has the proper permission for the registration.",   
" ",
"To disassociate (unmount) an iRods collection with a physical directory," 
"use the 'unmount' string as the physicalFilePath. For example, the following",
"command unmounts the /tempZone/home/myUser/mymount collection:",
"    ireg -m unmount /tempZone/home/myUser/mymount",
" ",
"The -b option can be used to associate a structured file with a collection.",
"The irods files stored in this collection will be stored in",
"the structured file. The [-R resource] option is used to specify the resource",
"to create the structured file in case it does not exist.",
"The 'ireg -m unmount' command can be used to disassociate a structured from a",
"collection",
" ",
"Options are:",
" -R  resource - specifies the resource to store to. This can also be specified",
"     in your environment or via a rule set up by the administrator.",
" -C  the specified path is a directory. The default assumes the path is a file.",
" -m  mount a directory to a collection by registering a physical directory",
"     and a resource with a collection.",
" -b  associated a structured file with a collection",
" -v  verbose",
" -V  Very verbose",
" -h  this help",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) return;
      printf("%s\n",msgs[i]);
   }
}
