/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* rodsAgent.c - The main code for rodsAgent
 */

#include "rodsAgent.h"
#include "rsApiHandler.h"

#define SERVER_DEBUG 1
int
main(int argc, char *argv[])
{
    int status;
    rsComm_t rsComm;
    struct allowedUser *allowedUserHead = NULL;
    char *tmpStr;

    ProcessType = AGENT_PT;

#ifndef _WIN32
    signal(SIGINT, signalExit);
    signal(SIGHUP, signalExit);
    signal(SIGTERM, signalExit);
    signal(SIGUSR1, signalExit);
    signal(SIGPIPE, rsPipSigalHandler);
#endif

#ifndef _WIN32
#ifdef SERVER_DEBUG
    if (isPath ("/tmp/rodsdebug"))
        sleep (20);
#endif
#endif

    memset (&rsComm, 0, sizeof (rsComm));

    status = setRsCommFromStartupPack (&rsComm);

    if (status < 0) {
	sendVersion (rsComm.sock, status, 0, NULL, 0);
        cleanupAndExit (status);
    }

    /* Handle option to log sql commands */
    tmpStr = getenv (SP_LOG_SQL);
    if (tmpStr != NULL) {
       rodsLogSqlReq(1);
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

    status = agentMain (&rsComm);

    cleanupAndExit (status);

    return (status);
}

int 
agentMain (rsComm_t *rsComm)
{
    int status = 0;
    int retryCnt = 0;

    while (1) {
	status = readAndProcClientMsg (rsComm, 0);

	if (status >= 0) {
	    retryCnt = 0;
	    continue;
	} else {
	    if (status == DISCONN_STATUS) {
		status = 0;
		break;
	    } else {
#if 0	/* client most likely gone */
                if (retryCnt >= MAX_MSG_READ_RETRY) {
                    rodsLog (LOG_NOTICE,
                      "agentMain: readMsg error. status = %d", status);
               } else {
                    retryCnt ++;
                    rodsSleep (READ_RETRY_SLEEP_TIME, 0);
                    continue;
                }
		/* attempt to send the error msg */
                sendVersion (rsComm->sock, status, 0, NULL);
#endif
                break;
	    }
	}
    }
    return (status);
}

int
setRsCommFromStartupPack (rsComm_t *rsComm)
{
    char *tmpStr;

    /* always use NATIVE_PROT as a client. e.g., server to server comm */
    tmpStr = malloc (NAME_LEN * 2);
    snprintf (tmpStr, NAME_LEN * 2, "%s=%d", IRODS_PROT, NATIVE_PROT);
    putenv (tmpStr);

    tmpStr = getenv (SP_NEW_SOCK);
    if (tmpStr == NULL) {
	rodsLog (LOG_NOTICE,
          "getstartupPackFromEnv: env %s does not exist", SP_NEW_SOCK);
	return (SYS_GETSTARTUP_PACK_ERR);
    }
    rsComm->sock = atoi (tmpStr);

    tmpStr = getenv (SP_CONNECT_CNT);
    if (tmpStr == NULL) {
        rodsLog (LOG_NOTICE,
          "getstartupPackFromEnv: env %s does not exist", SP_CONNECT_CNT);
        return (SYS_GETSTARTUP_PACK_ERR);
    }
    rsComm->connectCnt = atoi (tmpStr) + 1;
 
    tmpStr = getenv (SP_PROTOCOL);
    if (tmpStr == NULL) {
        rodsLog (LOG_NOTICE,
          "getstartupPackFromEnv: env %s does not exist", SP_PROTOCOL);
        return (SYS_GETSTARTUP_PACK_ERR);
    }
    rsComm->irodsProt = atoi (tmpStr);

    tmpStr = getenv (SP_RECONN_FLAG);
    if (tmpStr == NULL) {
        rodsLog (LOG_NOTICE,
          "getstartupPackFromEnv: env %s does not exist", SP_RECONN_FLAG);
        return (SYS_GETSTARTUP_PACK_ERR);
    }
    rsComm->reconnFlag = atoi (tmpStr);

    tmpStr = getenv (SP_PROXY_USER);
    if (tmpStr == NULL) {
        rodsLog (LOG_NOTICE,
          "getstartupPackFromEnv: env %s does not exist", SP_PROXY_USER);
        return (SYS_GETSTARTUP_PACK_ERR);
    }
    rstrcpy (rsComm->proxyUser.userName, tmpStr, NAME_LEN);
    if (strcmp (tmpStr, PUBLIC_USER_NAME) == 0) {
	rsComm->proxyUser.authInfo.authFlag = PUBLIC_USER_AUTH;
    }

    tmpStr = getenv (SP_PROXY_RODS_ZONE);
    if (tmpStr == NULL) {
        rodsLog (LOG_NOTICE,
          "getstartupPackFromEnv: env %s does not exist", 
	  SP_PROXY_RODS_ZONE);
        return (SYS_GETSTARTUP_PACK_ERR);
    }
    rstrcpy (rsComm->proxyUser.rodsZone, tmpStr, NAME_LEN);

    tmpStr = getenv (SP_CLIENT_USER);
    if (tmpStr == NULL) {
        rodsLog (LOG_NOTICE,
          "getstartupPackFromEnv: env %s does not exist", 
          SP_CLIENT_USER);
        return (SYS_GETSTARTUP_PACK_ERR);
    }
    rstrcpy (rsComm->clientUser.userName, tmpStr, NAME_LEN);
    if (strcmp (tmpStr, PUBLIC_USER_NAME) == 0) {
        rsComm->clientUser.authInfo.authFlag = PUBLIC_USER_AUTH;
    }

    tmpStr = getenv (SP_CLIENT_RODS_ZONE);
    if (tmpStr == NULL) {
        rodsLog (LOG_NOTICE,
          "getstartupPackFromEnv: env %s does not exist",
          SP_CLIENT_RODS_ZONE);
        return (SYS_GETSTARTUP_PACK_ERR);
    }
    rstrcpy (rsComm->clientUser.rodsZone, tmpStr, NAME_LEN);

    tmpStr = getenv (SP_REL_VERSION);
    if (tmpStr == NULL) {
        rodsLog (LOG_NOTICE,
          "getstartupPackFromEnv: env %s does not exist",
          SP_REL_VERSION);
        return (SYS_GETSTARTUP_PACK_ERR);
    }
    rstrcpy (rsComm->cliVersion.relVersion, tmpStr, NAME_LEN);

    tmpStr = getenv (SP_API_VERSION);
    if (tmpStr == NULL) {
        rodsLog (LOG_NOTICE,
          "getstartupPackFromEnv: env %s does not exist",
          SP_API_VERSION);
        return (SYS_GETSTARTUP_PACK_ERR);
    }
    rstrcpy (rsComm->cliVersion.apiVersion, tmpStr, NAME_LEN);

    tmpStr = getenv (SP_OPTION);
    if (tmpStr == NULL) {
        rodsLog (LOG_NOTICE,
          "getstartupPackFromEnv: env %s does not exist",
          SP_OPTION);
        return (SYS_GETSTARTUP_PACK_ERR);
    }
    rstrcpy (rsComm->option, tmpStr, NAME_LEN);

    setLocalAddr (rsComm->sock, &rsComm->localAddr);
    setRemoteAddr (rsComm->sock, &rsComm->remoteAddr);

    return (0);
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

