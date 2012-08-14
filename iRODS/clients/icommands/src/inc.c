/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 
 * inc - The irods physical move utility
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "ncUtil.h"
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
    

    optStr = "ho:Z";
   
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
        rodsLog (LOG_ERROR, "inc: no input");
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

    status = ncUtil (conn, &myEnv, &myRodsArgs, &rodsPathInp);

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
"Usage : inc [-hr] [--header] [--dim] [--ascitime] [--var]|[-o outFile]",
"dataObj|collection ... ",
" ",
"Perform NETCDF operations on the input data objects. The data objects must",
"be in NETCDF file format.",
"The -o option specifies the variables values will be extracted and put",
"into the file given by outFile in NETCDF format. The --header option can be",
"used with the -o option to extract the attributes info and add them to the",
"NETCDF output file.",

"If the -o option is not used, the output will be in plain text.",
"If no option is specified, the header and dimension info will be output.",
" ",

" ",
"Options are:",
"-o outFile - the variables values will be extracted and put into the file",
"      given by outFile in NETCDF format.",
" -r  recursive operation on the collction",
"--ascitime - For 'time' variable, output time in asci GMT time instead of ",
"      integer. e.g., 2006-05-01T08:30:00Z.",
"--header - output the header info (info on atrributes, dimensions and",
"      variables).",
"--dim - output the values of dimension variables.", 
"--var - output the values of variables.",
" -h  this help",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) break;
      printf("%s\n",msgs[i]);
   }
   printReleaseInfo("inc");
}
