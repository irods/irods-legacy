/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* rodsAgent.c - The main code for rodsAgent
 */

#include <syslog.h>
#include "rodsAgent.h"
#include "rsApiHandler.h"
#include "icatHighLevelRoutines.h"
#ifdef windows_platform
#include "rsLog.h"
static void NtAgentSetEnvsFromArgs(int ac, char **av);
#endif

/* #define SERVER_DEBUG 1   */
int
main(int argc, char *argv[])
{
    int status;
    rsComm_t rsComm;
    struct allowedUser *allowedUserHead = NULL;
    char *tmpStr;

    ProcessType = AGENT_PT;

#ifdef windows_platform
	iRODSNtAgentInit(argc, argv);
#endif

#ifndef windows_platform
    signal(SIGINT, signalExit);
    signal(SIGHUP, signalExit);
    signal(SIGTERM, signalExit);
    /* set to SIG_DFL as recommended by andy.salnikov so that system()
     * call returns real values instead of 1 */
    signal(SIGCHLD, SIG_DFL);
    signal(SIGUSR1, signalExit);
    signal(SIGPIPE, rsPipSigalHandler);
#endif

#ifndef windows_platform
#ifdef SERVER_DEBUG
    if (isPath ("/tmp/rodsdebug"))
        sleep (20);
#endif
#endif

#ifdef SYS_TIMING
    rodsLogLevel(LOG_NOTICE);
    printSysTiming ("irodsAgent", "exec", 1);
#endif

    memset (&rsComm, 0, sizeof (rsComm));

    status = initRsCommWithStartupPack (&rsComm, NULL);

    if (status < 0) {
	sendVersion (rsComm.sock, status, 0, NULL, 0);
        cleanupAndExit (status);
    }

    /* Handle option to log sql commands */
    tmpStr = getenv (SP_LOG_SQL);
    if (tmpStr != NULL) {
#ifdef IRODS_SYSLOG
       int j = atoi(tmpStr);
       rodsLogSqlReq(j);
#else
       rodsLogSqlReq(1);
#endif
    }

    /* Set the logging level */
    tmpStr = getenv (SP_LOG_LEVEL);
    if (tmpStr != NULL) {
       int i;
       i = atoi(tmpStr);
       rodsLogLevel(i);
    } else {
       rodsLogLevel(LOG_NOTICE); /* default */
    }

#ifdef IRODS_SYSLOG
/* Open a connection to syslog */
    openlog("rodsAgent",LOG_ODELAY|LOG_PID,LOG_DAEMON);
#endif

    status = getRodsEnv (&rsComm.myEnv);

    if (status < 0) {
	sendVersion (rsComm.sock, SYS_AGENT_INIT_ERR, 0, NULL, 0);
        cleanupAndExit (status);
    }

#if RODS_CAT
    if (strstr(rsComm.myEnv.rodsDebug, "CAT") != NULL) {
       chlDebug(rsComm.myEnv.rodsDebug);
    }
#endif

    setAllowedUser (&allowedUserHead);

    status = chkAllowedUser (allowedUserHead, rsComm.clientUser.userName,
     rsComm.clientUser.rodsZone);

    if (status <= 0) {
	sendVersion (rsComm.sock, SYS_USER_NOT_ALLOWED_TO_CONN, 0, NULL, 0);
	cleanupAndExit (SYS_USER_NOT_ALLOWED_TO_CONN);
    }

    status = initAgent (&rsComm);
#ifdef SYS_TIMING
    printSysTiming ("irodsAgent", "initAgent", 0);
#endif

    if (status < 0) {
	sendVersion (rsComm.sock, SYS_AGENT_INIT_ERR, 0, NULL, 0);
        cleanupAndExit (status);
    }

    /* send the server version and atatus as part of the protocol. Put
     * rsComm.reconnPort as the status */

    status = sendVersion (rsComm.sock, status, rsComm.reconnPort,
      rsComm.reconnAddr, rsComm.cookie);

    if (status < 0) {
	sendVersion (rsComm.sock, SYS_AGENT_INIT_ERR, 0, NULL, 0);
        cleanupAndExit (status);
    }
#ifdef SYS_TIMING
    printSysTiming ("irodsAgent", "sendVersion", 0);
#endif

    status = agentMain (&rsComm);

    cleanupAndExit (status);

    return (status);
}

#if 0	/* moved to initServer.c */
int 
agentMain (rsComm_t *rsComm)
{
    int status = 0;
    int retryCnt = 0;

    while (1) {

        if (rsComm->gsiRequest==1) {
	    status = igsiServersideAuth(rsComm) ;
	    rsComm->gsiRequest=0; 
        }
        if (rsComm->gsiRequest==2) {
	    status = ikrbServersideAuth(rsComm) ;
	    rsComm->gsiRequest=0; 
        }

	status = readAndProcClientMsg (rsComm, READ_HEADER_TIMEOUT);
#if 0
	status = readAndProcClientMsg (rsComm, 0);
#endif

	if (status >= 0) {
	    retryCnt = 0;
	    continue;
	} else {
	    if (status == DISCONN_STATUS) {
		status = 0;
		break;
	    } else {
                break;
	    }
	}
    }
    return (status);
}

int
setAllowedUser (struct allowedUser **allowedUserHead)
{
    struct allowedUser *tmpAllowedUser;

    char *conFile;
    char *configDir;
    FILE *file;
    char buf[LONG_NAME_LEN * 5];
    int len; 
    char *bufPtr;
    int status;

    *allowedUserHead = NULL;

    configDir = getConfigDir ();
    len = strlen (configDir) + strlen(MAINTENENCE_CONFIG_FILE) + 2;
;

    conFile = (char *) malloc(len);

    snprintf (conFile, len, "%s/%s", configDir, MAINTENENCE_CONFIG_FILE);
    file = fopen(conFile, "r");

    if (file == NULL) {
#ifdef DEBUG_MAINTENENCE
            fprintf (stderr, "Unable to open MAINTENENCE_CONFIG_FILE file %s\n",
             conFile);
#endif
        free (conFile);
        return (0);
    }

    free (conFile);

    while (fgets (buf, LONG_NAME_LEN * 5, file) != NULL) {
        char myuser[NAME_LEN];
        char myZone[NAME_LEN];
	char myInput[NAME_LEN * 2];

	if (*buf == '#')	/* a comment */
	    continue;

	bufPtr = buf;

	while (copyStrFromBuf (&bufPtr, myInput, NAME_LEN * 2) > 0) {
	    status = parseUserName (myInput, myuser, myZone);
	    if (status >= 0) {
                tmpAllowedUser = malloc (sizeof (struct allowedUser));
                memset (tmpAllowedUser, 0, sizeof (struct allowedUser));
                tmpAllowedUser->userName = strdup (myuser);
                tmpAllowedUser->rodsZone = strdup (myZone);
                /* queue it */

                if (*allowedUserHead == NULL) {
                    *allowedUserHead = tmpAllowedUser;
                } else {
                    tmpAllowedUser->next = *allowedUserHead;
                    *allowedUserHead = tmpAllowedUser;
		}
            } else {
		rodsLog (LOG_NOTICE, 
		  "setAllowedUser: cannot parse input %s. status = %d",
		  myInput, status);
	    }
        }
    }

    fclose (file);
    if (*allowedUserHead == NULL) {
        /* make an empty struct --> noone is allowed to use */
        *allowedUserHead = malloc (sizeof (struct allowedUser));
        memset (*allowedUserHead, 0, sizeof (struct allowedUser));
    }
    return (0);
}

int
chkAllowedUser (struct allowedUser *allowedUserHead, char *userName,
char *rodsZone)
{
    struct allowedUser *tmpAllowedUser;

    if (allowedUserHead == NULL) {
        /* no limit */
        return (1);
    }

    if (userName == NULL || rodsZone == 0) {
        return (0);
    }

    /* noone is allowed */
    if (allowedUserHead->next == NULL &&
      allowedUserHead->userName == NULL) {
        return (0);
    }

    tmpAllowedUser = allowedUserHead;
    while (tmpAllowedUser != NULL) {
        if (tmpAllowedUser->userName != NULL &&
         strcmp (tmpAllowedUser->userName, userName) == 0 &&
         tmpAllowedUser->rodsZone != NULL &&
         strcmp (tmpAllowedUser->rodsZone, rodsZone) == 0) {
            /* we have a match */
            break;
        }
        tmpAllowedUser = tmpAllowedUser->next;
    }
    if (tmpAllowedUser == NULL) {
        /* no match */
        return (0);
    } else {
        return (1);
    }
}
#endif
