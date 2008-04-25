/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "rods.h"
#include "parseCommandLine.h"

void usage (char *prog);

int
main(int argc, char **argv)
{
    int i, ix, status;
    int echoFlag=0;
    char *password;
    rodsEnv myEnv;
    rcComm_t *Conn;
    rErrMsg_t errMsg;
    rodsArguments_t myRodsArgs;
    int useGsi=0;
    int doPassword;

    status = parseCmdLineOpt(argc, argv, "ehvVl", 0, &myRodsArgs);
    if (status != 0) {
       printf("Use -h for help.\n");
       exit(1);
    }

    if (myRodsArgs.echo==True) {
       echoFlag=1;
    }
    if (myRodsArgs.help==True) {
       usage(argv[0]);
       exit(0);
    }

    if (myRodsArgs.longOption==True) {
	rodsLogLevel(LOG_NOTICE);
    }
 
    ix = myRodsArgs.optind;

    password="";
    if (ix < argc) {
       password = argv[ix];
    }

    status = getRodsEnv (&myEnv);  /* Need to get irodsAuthFileName (if set) */
    if (status < 0) {
       rodsLog (LOG_ERROR, "main: getRodsEnv error. status = %d",
		status);
       exit (1);
    }

    if (myRodsArgs.longOption==True) {
	/* just list the env */
	exit (0);
    }

    doPassword=1;
#if defined(GSI_AUTH)
    if (strncmp("GSI",myEnv.rodsAuthScheme,3)==0) {
       useGsi=1;
       doPassword=0;
    }
#endif

    if (strcmp(myEnv.rodsUserName, ANONYMOUS_USER)==0) {
       doPassword=0;
    }
    if (useGsi==1) {
       printf("Using GSI, attempting connection/authentication\n");
    }
    if (doPassword==1) {
       if (myRodsArgs.verbose==True) {
	  i = obfSavePw(echoFlag, 1, 1, password);
       }
       else {
	  i = obfSavePw(echoFlag, 0, 0, password);
       }

       if (i != 0) {
	  rodsLogError(LOG_ERROR, i, "Save Password failure");
	  exit(1);
       }
    }

    /* Connect... */ 
    Conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
      myEnv.rodsZone, 0, &errMsg);
    if (Conn == NULL) {
       rodsLog (LOG_ERROR, "rcConnect failure %d %s",errMsg.status,
		errMsg.msg);
       exit(2);
    }

    /* and check that the user/password is OK */
    status = clientLogin(Conn);
    if (status != 0) {
       rcDisconnect(Conn);
       exit (7);
    }

    rcDisconnect(Conn);

    exit (0);
}


void usage (char *prog)
{
  fprintf(stderr, "Creates a file containing your iRODS password in a scrambled form,\n");
  fprintf(stderr, "to be used automatically by the icommands.\n");
  fprintf(stderr, "Usage: %s [-ehvVl]\n", prog);
  fprintf(stderr, " -e  echo the password as you enter it (normally there is no echo)\n");
  fprintf(stderr, " -l  list the iRODS environment variables (only)\n");
  fprintf(stderr, " -v  verbose\n");
  fprintf(stderr, " -V  Very verbose\n");
  fprintf(stderr, " -h  this help\n");
}

