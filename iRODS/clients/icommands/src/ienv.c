/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 
 * ienv - The irods print environment utility
 */

#include "rodsClient.h"
#include "parseCommandLine.h"

void usage ();

int
main(int argc, char **argv) {
	int status;
	rodsEnv myEnv;
	rodsArguments_t myRodsArgs;
	char *optStr;
	
	
	optStr = "hZ";
	
	status = parseCmdLineOpt (argc, argv, optStr, 1, &myRodsArgs);
	
	if (status < 0) {
		printf("Use -h for help\n");
		exit (1);
	}

	if (myRodsArgs.help==True) {
		usage();
		exit(0);
	}

	rodsLogLevel(LOG_NOTICE);

	rodsLog (LOG_NOTICE, "Release Version = %s, API Version = %s",
	  RODS_REL_VERSION, RODS_API_VERSION);

	status = getRodsEnv (&myEnv);
	
	if (status < 0) {
		rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
		exit (1);
	}
	
	exit(0);
}

void
usage () {
   char *msgs[]={
"Usage : ienv [-h]",
"Display current irods environment. Equivalent to iinit -l.",
"Options are:",
" -h  this help",
" ",
"For this version of iRODS, for many i-commands, environment variables",
"can also be passed on the command line via '--' options and if the four",
"key ones are input, no other user environment is needed (for example",
"the ~/.irods/.irodsEnv file). These are:",
"--irodsHost --irodsPort --irodsUserName --irodsZone",
"Also available are:",
"--irodsHome --irodsCwd --irodsDefResource",
"These are input with a blank and the associated value, for example,",
"'--irodsHost pivo.ucsd.edu'",
"These option values are placed into the process environment.",
"For an example, try running: ienv --irodsHost somename",
" ",
"For more information, see https://wiki.irods.org/index.php/user_environment",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) break;
      printf("%s\n",msgs[i]);
   }
   printReleaseInfo("ienv");
}



