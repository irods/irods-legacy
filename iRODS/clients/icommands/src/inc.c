/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 
 * inc - The irods NETCDF utility
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
"Usage : inc [-hr] [--header] [--dim] [--ascitime] [--noattr] [-o outFile]",
"[--var 'var1 var2 ...'] [--subset 'dimName1[start%stride%end] ...']|",
"[--SUBSET dimName1[startVal%stride%endVal]...] dataObj|collection ... ",
" ",
"Perform NETCDF operations on the input data objects. The data objects must",
"be in NETCDF file format.",
"The -o option specifies the extracted variable values will be put into the", 
"given outFile in NETCDF format instead of text format to the terminal."
"The -o option is only allowable if the client is built with the NETCDF_API",
"switched on (compile and linked without the NETCDF C library).",
"The --noattr specifies that attributes will not be saved or displayed.",
"By default, the attributes info is automatically extracted and saved to the",
"NETCDF output file if the -o option is used or displayed to the terminal",
"if the --header option is used.", 

"If the -o option is not used, the output will be in plain text.",
"If no option is specified, the header info will be output.",
" ",
"The --var option can be used to specify a list of variables for data output",
"to the terminal in text format or to the outFile in NETCDF format if the -o",
"option is used. e.g., ",
"   inc --var 'pressure temperature current' myfile.nc",
"A value of 'all' for --var means all variables. In addition, if the ",
"-o option is used, no --var input also means all variables.", 
" ",
"The --subset option can be used to specify a list of subsetting conditions.", 
"Each subetting condition must be given in the form dimName[start%stride%end]",
"where 'start' and 'end' are the starting and ending indices of the dimension",
"array. e.g.,",
"   inc --var pressure --subset 'longitude[2%1%8] latitude[4%1%5] time[2%1%4]'",
"   myfile.nc",
" ",
"Alternatively, the --SUBSET option can be used to specify subsetting",
"conditions using the values of the dimension valuable instead of using",
"the dimension indeces. e.g.,",
"   inc --SUBSET 'latitude[37.87%1%45.34] time[1146515400%1%1146533400]' myfile.nc",
"or using ASCI value for time:",
"   inc --SUBSET 'latitude[37.87%1%45.34] time[2006-05-01T13:30:00%1%200-05-01T18:30:00]' myfile.nc",
"Options are:",
"-o outFile - the extracted variable values will be put into the given",
"      outFile in NETCDF format. Only allowable if the client is built",
"      with the NETCDF_API switched on (compile and linked without the",
"      NETCDF C library)",
" -r  recursive operation on the collction",
"--ascitime - For 'time' variable, output time in asci local time instead of ",
"      integer. e.g., 2006-05-01T08:30:00.",
"--header - output the header info (info on atrributes, dimensions and",
"      variables).",
"--dim - output the values of dimension variables.", 
"--noattr - attributes will not be saved nor displayed.",
"--var 'var1 var2 ...' - list of variables or data output. A value of 'all'",
"      means all variables",
"--subset 'dimName1[start%stride%end] ...' - list of subsetting conditions in",
"      the form dimName[start%stride%end] where 'start' and 'end' are the",
"      starting and ending indices of the dimension array",
"--SUBSET 'dimName1[startVal%stride%endVal] ...' - list of subsetting",
"      conditions in the form dimName[startVal%stride%endVal] where 'startVal'",
"      and 'endVal' are the starting and ending values of the dimension array",
" -h  this help",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) break;
      printf("%s\n",msgs[i]);
   }
   printReleaseInfo("inc");
}
