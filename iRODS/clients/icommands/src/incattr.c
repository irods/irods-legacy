/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 
 * incattr - The irods physical move utility
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "ncattrUtil.h"
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
    

    optStr = "hlMq:rvZ";
   
    status = parseCmdLineOpt (argc, argv, optStr, 1, &myRodsArgs);

    if (status < 0) {
        printf("Use -h for help.\n");
        exit (1);
    }

    if (myRodsArgs.help==True) {
       usage();
       exit(0);
    }

    if (argc - optind <= 0) {
        rodsLog (LOG_ERROR, "incattr: no input");
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
        exit (2);
    }
   
    status = clientLogin(conn);
    if (status != 0) {
        rcDisconnect(conn);
        exit (7);
    }

    status = ncattrUtil (conn, &myEnv, &myRodsArgs, &rodsPathInp);

    printErrorStack(conn->rError);

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
"Usage : incattr [-hlMrv] [--attr attrName] [--reg] [--remove] [-q \"key Op value [...]\"]",
"dataObj|collection ... ",
"By default, the attribute name (key) associated with the data is listed",
" ",
"Netcdf attribute operations.",
"Options are:",
" -l  list values for all attributes",
" -M  admin - admin user uses this option to phymove other users files", 
" -q \"attr Op value\" - list all data files that match the query",
" -r  recursive operation on the collction",
" -v  verbose mode",
"--attr attrName - given the atrribute name, get the attribute value",
"--reg - extract and register the global attributes",
"--remove - unregister the global attributes",
" -h  this help",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) break;
      printf("%s\n",msgs[i]);
   }
   printReleaseInfo("incattr");
}
