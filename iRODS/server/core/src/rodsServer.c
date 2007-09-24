/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "rodsServer.h"

uint ServerBootTime;

int
main(int argc, char **argv)
{
    int status;
    int c;
    int uFlag = 0;
    char tmpStr1[100], tmpStr2[100];
    char *logDir = NULL;

    ProcessType = SERVER_PT;	/* I am a server */

    rodsLogLevel (LOG_NOTICE);
    
    ServerBootTime = time (0);
    while ((c = getopt(argc, argv,"uvVqsh")) != EOF) {
        switch (c) {
            case 'u':		/* user command level. without serverized */
		uFlag = 1;
                break;
            case 'D':   /* user specified a log directory */
                logDir = strdup (optarg);
                break;
	    case 'v':		/* verbose Logging */
		snprintf(tmpStr1,100,"%s=%d",SP_LOG_LEVEL, LOG_NOTICE);
		putenv(tmpStr1);
	        rodsLogLevel(LOG_NOTICE);
                break;
  	    case 'V':		/* very Verbose */
		snprintf(tmpStr1,100,"%s=%d",SP_LOG_LEVEL, LOG_DEBUG1);
		putenv(tmpStr1);
	        rodsLogLevel(LOG_DEBUG1);
                break;
	    case 'q':           /* quiet (only errors and above) */
		snprintf(tmpStr1,100,"%s=%d",SP_LOG_LEVEL, LOG_ERROR);
		putenv(tmpStr1);
	        rodsLogLevel(LOG_ERROR);
                break;
  	    case 's':		/* log SQL commands */
		snprintf(tmpStr2,100,"%s=%d",SP_LOG_SQL, 1);
		putenv(tmpStr2);
                break;
	    case 'h':		/* help */
                usage (argv[0]);
                exit (0);
            default:
                usage (argv[0]);
                exit (1);
        }
    }

    if (uFlag == 0) {
	if (serverize (logDir) < 0) {
	    exit (1);
	}
    }

#ifdef RODS_CAT
    if (getenv ("reServerOnIes") != NULL) {
#else
    if (getenv ("reServerOnThisServer") != NULL) {
#endif
	char *reServerOption;

        if (RODS_FORK () == 0) {  /* child */
            char *reServerOption = NULL;
            char *av[NAME_LEN];

            memset (av, 0, sizeof (av));
            reServerOption = getenv ("reServerOption");
            setExecArg (reServerOption, av);
	    rodsLog(LOG_NOTICE, "Starting irodsReServer");
            av[0] = "irodsReServer";
            execv(av[0], av);
            exit(1);
        }
    }

    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
#ifdef osx_platform
    signal(SIGINT, (void *) serverExit);
    signal(SIGHUP, (void *)serverExit);
    signal(SIGTERM, (void *)serverExit);
#else
    signal(SIGINT, serverExit);
    signal(SIGHUP, serverExit);
    signal(SIGTERM, serverExit);
#endif
/* log the server timeout */

#ifndef _WIN32
    /* Initialize ServerTimeOut */
    if (getenv ("ServerTimeOut") != NULL) {
        int serverTimeOut;
        serverTimeOut = atoi (getenv ("ServerTimeOut"));
        if (serverTimeOut < MIN_AGENT_TIMEOUT_TIME) {
            rodsLog (LOG_NOTICE,
             "ServerTimeOut %d is less than min of %d",
             serverTimeOut, MIN_AGENT_TIMEOUT_TIME);
            serverTimeOut = MIN_AGENT_TIMEOUT_TIME;
        }
        rodsLog (LOG_NOTICE,
         "ServerTimeOut has been set to %d seconds",
         serverTimeOut);
    }
#endif

    serverMain (logDir);
    exit (0);
}

int 
serverize (char *logDir)
{
    char *logFile = NULL;
    int status;

    getLogfileName (&logFile, logDir, RODS_LOGFILE);

    LogFd = open (logFile, O_CREAT|O_WRONLY|O_APPEND, 0644);
    if (LogFd < 0) {
        rodsLog (LOG_NOTICE, "logFileOpen: Unable to open %s. errno = %d",
          logFile, errno);
        return (-1);
    }

#ifndef _WIN32
    if (fork()) {	/* parent */
        exit (0);
    } else {	/* child */
        if (setsid() < 0) {
            rodsLog (LOG_NOTICE, 
	     "serverize: setsid failed, errno = %d\n", errno);
            exit(1);
	}
        (void) dup2 (LogFd, 0);
        (void) dup2 (LogFd, 1);
        (void) dup2 (LogFd, 2);
        close (LogFd);
        LogFd = 2;
    }
#endif
    return (LogFd);
}

int 
serverMain (char *logDir)
{
    int status;
    rsComm_t svrComm;
    fd_set sockMask;
    int numSock;
    int newSock;
    startupPack_t *startupPack;
    agentProc_t *agentProcHead = NULL;
    int loopCnt;
    int acceptErrCnt = 0;


    memset (&svrComm, 0, sizeof (svrComm));

    status = getRodsEnv (&svrComm.myEnv);

    if (status < 0) {
        rodsLog (LOG_NOTICE, "serverMain: getRodsEnv error. status = %d",
          status);
        exit (1);
    }
	
    setRsCommFromRodsEnv (&svrComm);

    status = initServer (&svrComm);

    if (status < 0) {
        rodsLog (LOG_NOTICE, "serverMain: initServer error. status = %d",
          status);
        exit (1);
    }

    /* open  a socket an listen for connection */ 
    svrComm.sock = sockOpenForInConn (&svrComm, &svrComm.myEnv.rodsPort, NULL);

    if (svrComm.sock < 0) {
        rodsLog (LOG_NOTICE, "serverMain: sockOpenForInConn error. status = %d",
          svrComm.sock);
        exit (1);
    }

    listen (svrComm.sock, MAX_LISTEN_QUE);

    FD_ZERO(&sockMask);

    rodsLog (LOG_NOTICE, "rodsServer version %s is up", RODS_REL_VERSION);

    while (1) {		/* infinite loop */
        FD_SET(svrComm.sock, &sockMask);
        while ((numSock = select (svrComm.sock + 1, &sockMask, 
	  (fd_set *) NULL, (fd_set *) NULL, (struct timeval *) NULL)) < 0) {

            if (errno == EINTR) {
		rodsLog (LOG_NOTICE, "serverMain: select() interrupted");
                FD_SET(svrComm.sock, &sockMask);
		continue;
	    } else {
		rodsLog (LOG_NOTICE, "serverMain: select() error, errno = %d",
		  errno);
	        return (-1);
	    }
	}

	procChildren (&agentProcHead);

	newSock = rsAcceptConn (&svrComm);

	if (newSock < 0) {
	    acceptErrCnt ++;
	    if (acceptErrCnt > MAX_ACCEPT_ERR_CNT) {
	        rodsLog (LOG_ERROR, 
		  "serverMain: Too many socket accept error. Exiting");
		break;
	    } else {
	        rodsLog (LOG_NOTICE, 
	          "serverMain: acceptConn () error, errno = %d", errno);
	        continue;
	    }
	}

        acceptErrCnt = 0;

	status = readStartupPack (newSock, &startupPack);

	if (status < 0) {
	    rodsLog (LOG_NOTICE, "readStartupPack error from %s, status = %d", 
	      inet_ntoa (svrComm.remoteAddr.sin_addr), status);
	    sendVersion (newSock, status, 0, NULL, 0);
	    close (newSock);
	    continue;
	}

	if (startupPack->connectCnt > MAX_SVR_SVR_CONNECT_CNT) {
	    sendVersion (newSock, SYS_EXCEED_CONNECT_CNT, 0, NULL, 0);
	    close (newSock);
            continue;
        }

	status = spawnAgent (newSock, startupPack, &agentProcHead);

	close (newSock);
        if (status < 0) {
            rodsLog (LOG_NOTICE, 
	     "spawnAgent error for puser=%s and cuser=%s from %s, status = %d",
              startupPack->proxyUser, startupPack->clientUser,
	      inet_ntoa (svrComm.remoteAddr.sin_addr), status);
	    free  (startupPack);
            continue;
        } else {
            rodsLog (LOG_NOTICE,
	     "Agent proccess %d started for for puser=%s and cuser=%s from %s",
              agentProcHead->pid, startupPack->proxyUser, 
	      startupPack->clientUser, 
	      inet_ntoa (svrComm.remoteAddr.sin_addr));
	    free  (startupPack);
	    loopCnt++;
#ifndef _WIN32
	    if (loopCnt >= LOGFILE_CHK_CNT) {
		chkLogfileName (logDir, RODS_LOGFILE);
	    }
#endif
	}
    }		/* infinite loop */

    /* not reached - return (status); */
}

void
#if defined(linux_platform) || defined(aix_platform) || defined(solaris_platform) || defined(linux_platform) || defined(osx_platform)
serverExit (int sig)
#else
serverExit ()
#endif
{
    rodsLog (LOG_NOTICE, "rodsServer caught signal %d, exiting", sig);
    exit (1);
}

void
usage (char *prog)
{
    printf ("Usage: %s [-uvVqs]\n", prog);
    printf (" -u  user command level, remain attached to the tty\n");
    printf (" -v  verbose (LOG_NOTICE)\n");
    printf (" -V  very verbose (LOG_DEBUG1)\n");
    printf (" -q  quiet (LOG_ERROR)\n");
    printf (" -s  log SQL commands\n");

}

int
procChildren (agentProc_t **agentProcHead)
{
    int childPid;
    agentProc_t *tmpAgentProc;
    int status;


#ifndef _WIN32
    while ((childPid = waitpid (-1, &status, WNOHANG | WUNTRACED)) > 0) {
	tmpAgentProc = getAgentProcByPid (childPid, agentProcHead);
	if (tmpAgentProc != NULL) {
	    rodsLog (LOG_NOTICE, "Agent proccess %s exited", childPid);
	    free (tmpAgentProc);
	} else {
	    rodsLog (LOG_NOTICE, "Agent proccess %s exited but not in queue",
	      childPid); 
	}
    }
#endif

    return (0);
}


agentProc_t *
getAgentProcByPid (int childPid, agentProc_t **agentProcHead) 
{
    agentProc_t *tmpAgentProc, *prevAgentProc;
 
    tmpAgentProc = *agentProcHead;
    prevAgentProc = NULL;

    while (tmpAgentProc != NULL) {
	if (childPid == tmpAgentProc->pid) {
	    if (prevAgentProc == NULL) {
		*agentProcHead = tmpAgentProc->next;
	    } else {
		prevAgentProc->next = tmpAgentProc->next;
	    }
	    break;
	}
	prevAgentProc = tmpAgentProc;
	tmpAgentProc = tmpAgentProc->next;
    }
    return (tmpAgentProc);
}


int
spawnAgent (int newSock, startupPack_t *startupPack, 
agentProc_t **agentProcHead)
{
    int childPid;

    childPid = RODS_FORK ();

    if (childPid == 0) {	/* child */
	execAgent (newSock, startupPack);
    } else {			/* parent */
	queAgentProc (childPid, startupPack, agentProcHead);
    }
    return (childPid);
}

int 
execAgent (int newSock, startupPack_t *startupPack)
{
    char *myArgv[2];
    char buf[NAME_LEN];
    char *myBuf;
    int i = 0;

    myBuf = malloc (NAME_LEN * 2);
    snprintf (myBuf, NAME_LEN * 2, "%s=%d", SP_NEW_SOCK, newSock);
    putenv (myBuf);

    myBuf = malloc (NAME_LEN * 2);
    snprintf (myBuf, NAME_LEN * 2, "%s=%d", SP_PROTOCOL,
      startupPack->irodsProt);
    putenv (myBuf);

    myBuf = malloc (NAME_LEN * 2);
    snprintf (myBuf, NAME_LEN * 2, "%s=%d", SP_RECONN_FLAG,
      startupPack->reconnFlag);
    putenv (myBuf);

    myBuf = malloc (NAME_LEN * 2);
    snprintf (myBuf, NAME_LEN * 2, "%s=%d", SP_CONNECT_CNT,
      startupPack->connectCnt);
    putenv (myBuf);

    myBuf = malloc (NAME_LEN * 2);
    snprintf (myBuf, NAME_LEN * 2, "%s=%s", SP_PROXY_USER, 
      startupPack->proxyUser);
    putenv (myBuf);

    myBuf = malloc (NAME_LEN * 2);
    snprintf (myBuf, NAME_LEN * 2, "%s=%s", SP_PROXY_RODS_ZONE, 
      startupPack->proxyRodsZone);
    putenv (myBuf);

    myBuf = malloc (NAME_LEN * 2);
    snprintf (myBuf, NAME_LEN * 2, "%s=%s", SP_CLIENT_USER, 
      startupPack->clientUser);
    putenv (myBuf);

    myBuf = malloc (NAME_LEN * 2);
    snprintf (myBuf, NAME_LEN * 2, "%s=%s", SP_CLIENT_RODS_ZONE, 
      startupPack->clientRodsZone);
    putenv (myBuf);

    myBuf = malloc (NAME_LEN * 2);
    snprintf (myBuf, NAME_LEN * 2, "%s=%s", SP_REL_VERSION, 
      startupPack->relVersion);
    putenv (myBuf);

    myBuf = malloc (NAME_LEN * 2);
    snprintf (myBuf, NAME_LEN * 2, "%s=%s", SP_API_VERSION,
      startupPack->apiVersion);
    putenv (myBuf);

    myBuf = malloc (NAME_LEN * 2);
    snprintf (myBuf, NAME_LEN * 2, "%s=%s", SP_OPTION, 
      startupPack->option);
    putenv (myBuf);

    myBuf = malloc (NAME_LEN * 2);
    snprintf (myBuf, NAME_LEN * 2, "%s=%d", SERVER_BOOT_TIME,
      ServerBootTime);
    putenv (myBuf);

    rstrcpy (buf, AGENT_EXE, NAME_LEN); 
    myArgv[0] = buf;
    myArgv[1] = NULL;
    
    execv(myArgv[0], myArgv);
    return (0);
}

int
queAgentProc (int childPid, startupPack_t *startupPack, 
agentProc_t **agentProcHead)
{
    agentProc_t *tmpAagentProc;

    tmpAagentProc = (agentProc_t *) malloc (sizeof (agentProc_t));
    memset (tmpAagentProc, 0, sizeof (agentProc_t));

    tmpAagentProc->next = *agentProcHead;
    tmpAagentProc->pid = childPid;
    rstrcpy (tmpAagentProc->proxyUser, startupPack->proxyUser, NAME_LEN);
    rstrcpy (tmpAagentProc->clientUser, startupPack->clientUser, NAME_LEN);
    *agentProcHead = tmpAagentProc;

    return (0);
}

int
initServer ( rsComm_t *svrComm)
{
    int status;
    rodsServerHost_t *rodsServerHost = NULL;

    status = initServerInfo (svrComm);
    if (status < 0) {
        rodsLog (LOG_NOTICE,
          "initServer: initServerInfo error, status = %d",
          status);
        return (status);
    }

    printLocalResc ();

    printZoneInfo ();

    status = getRcatHost (MASTER_RCAT, NULL, &rodsServerHost);

    if (status < 0) {
        return (status);
    }

    if (rodsServerHost->localFlag == LOCAL_HOST) {
#if RODS_CAT
        disconnectRcat (svrComm);
#endif
    } else {
	if (rodsServerHost->conn != NULL) {
	    rcDisconnect (rodsServerHost->conn);
	    rodsServerHost->conn = NULL;
	}
    } 

    return (status);
}

int
setRsCommFromRodsEnv (rsComm_t *rsComm)
{
    rodsEnv *myEnv = &rsComm->myEnv;

    rstrcpy (rsComm->proxyUser.userName, myEnv->rodsUserName, NAME_LEN);
    rstrcpy (rsComm->clientUser.userName, myEnv->rodsUserName, NAME_LEN);

    rstrcpy (rsComm->proxyUser.rodsZone, myEnv->rodsZone, NAME_LEN);
    rstrcpy (rsComm->clientUser.rodsZone, myEnv->rodsZone, NAME_LEN);

    return (0);
}

