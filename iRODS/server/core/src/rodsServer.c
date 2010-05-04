/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "rodsServer.h"

#include <syslog.h>

#ifndef windows_platform
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef windows_platform
#include "irodsntutil.h"
#endif

uint ServerBootTime;

#ifndef windows_platform   /* all UNIX */
int
main(int argc, char **argv)
#else   /* Windows */
int irodsWinMain(int argc, char **argv)
#endif
{
    int c;
    int uFlag = 0;
    char tmpStr1[100], tmpStr2[100];
    char *logDir = NULL;


    ProcessType = SERVER_PT;	/* I am a server */

    rodsLogLevel (LOG_NOTICE);

#ifdef IRODS_SYSLOG
/* Open a connection to syslog */
	openlog("rodsServer",LOG_ODELAY|LOG_PID,LOG_DAEMON);
#endif
    
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

    /* start of irodsReServer has been moved to serverMain */
#ifndef _WIN32
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
#ifdef osx_platform
    signal(SIGINT, (void *) serverExit);
    signal(SIGHUP, (void *)serverExit);
    signal(SIGTERM, (void *)serverExit);
#else
    signal(SIGINT, serverExit);
    signal(SIGHUP, serverExit);
    signal(SIGTERM, serverExit);
#endif
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

#ifdef windows_platform
	if(iRODSNtServerRunningConsoleMode())
		return;
#endif

    getLogfileName (&logFile, logDir, RODS_LOGFILE);

#ifndef windows_platform
#ifdef IRODS_SYSLOG
    LogFd = 0;
#else
    LogFd = open (logFile, O_CREAT|O_WRONLY|O_APPEND, 0644);
#endif
#else
    LogFd = iRODSNt_open(logFile, O_CREAT|O_APPEND|O_WRONLY, 1);
#endif

    if (LogFd < 0) {
        rodsLog (LOG_NOTICE, "logFileOpen: Unable to open %s. errno = %d",
          logFile, errno);
        return (-1);
    }

#ifndef windows_platform
    if (fork()) {	/* parent */
        exit (0);
    } else {	/* child */
        if (setsid() < 0) {
            rodsLog (LOG_NOTICE, 
	     "serverize: setsid failed, errno = %d\n", errno);
            exit(1);
	}
#ifndef IRODS_SYSLOG
        (void) dup2 (LogFd, 0);
        (void) dup2 (LogFd, 1);
        (void) dup2 (LogFd, 2);
        close (LogFd);
        LogFd = 2;
#endif
    }
#else
	_close(LogFd);
#endif

#ifdef IRODS_SYSLOG
    return (0);
#else
    return (LogFd);
#endif
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
    int loopCnt = 0;
    int acceptErrCnt = 0;
    rodsServerHost_t *reServerHost = NULL;


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
    svrComm.sock = sockOpenForInConn (&svrComm, &svrComm.myEnv.rodsPort, NULL,
      SOCK_STREAM);

    if (svrComm.sock < 0) {
        rodsLog (LOG_NOTICE, "serverMain: sockOpenForInConn error. status = %d",
          svrComm.sock);
        exit (1);
    }

    listen (svrComm.sock, MAX_LISTEN_QUE);

    FD_ZERO(&sockMask);

    rodsLog (LOG_NOTICE, 
     "rodsServer Release version %s - API Version %s is up", 
     RODS_REL_VERSION, RODS_API_VERSION);

    /* Record port, pid, and cwd into a well-known file */
    recordServerProcess(&svrComm);

    /* start the irodsReServer */
#ifndef windows_platform   /* tempoarily set Windows don't need to to have reServer */
#if 0	/* use reHost in server.config instead */
#ifdef RODS_CAT
    if (getenv ("reServerOnIes") != NULL) 
#else
    if (getenv ("reServerOnThisServer") != NULL) 
#endif
#endif
    getReHost (&reServerHost);
    if (reServerHost != NULL && reServerHost->localFlag == LOCAL_HOST) {
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
#endif

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
#ifdef SYS_TIMING
	initSysTiming ("irodsServer", "recv connection", 0);
#endif

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
#ifdef SYS_TIMING
	printSysTiming ("irodsServer", "read StartupPack", 0);
#endif
	if (startupPack->connectCnt > MAX_SVR_SVR_CONNECT_CNT) {
	    sendVersion (newSock, SYS_EXCEED_CONNECT_CNT, 0, NULL, 0);
	    close (newSock);
            continue;
        }

	status = spawnAgent (newSock, startupPack, &agentProcHead);

#ifndef windows_platform
	close (newSock);
#endif
        if (status < 0) {
            rodsLog (LOG_NOTICE, 
	     "spawnAgent error for puser=%s and cuser=%s from %s, status = %d",
              startupPack->proxyUser, startupPack->clientUser,
	      inet_ntoa (svrComm.remoteAddr.sin_addr), status);
	        free  (startupPack);
            continue;
        } else {
            rodsLog (LOG_NOTICE,
	     "Agent process %d started for puser=%s and cuser=%s from %s",
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
    return(0); /* to avoid warning */
}

void
#if defined(linux_platform) || defined(aix_platform) || defined(solaris_platform) || defined(linux_platform) || defined(osx_platform)
serverExit (int sig)
#else
serverExit ()
#endif
{
#ifndef windows_platform
    rodsLog (LOG_NOTICE, "rodsServer caught signal %d, exiting", sig);
#else
	rodsLog (LOG_NOTICE, "rodsServer is exiting.");
#endif
    recordServerProcess(NULL); /* unlink the process id file */
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
	    rodsLog (LOG_NOTICE, "Agent process %s exited", childPid);
	    free (tmpAgentProc);
	} else {
	    rodsLog (LOG_NOTICE, "Agent process %s exited but not in queue",
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

#ifndef windows_platform
    childPid = RODS_FORK ();

    if (childPid == 0) {	/* child */
#ifdef SYS_TIMING
        printSysTiming ("irodsAent", "after fork", 0);
        initSysTiming ("irodsAent", "after fork", 1);
#endif
	execAgent (newSock, startupPack);
    } else {			/* parent */
#ifdef SYS_TIMING
	printSysTiming ("irodsServer", "fork agent", 0);
#endif
	queAgentProc (childPid, startupPack, agentProcHead);
    }
#else
	childPid = execAgent (newSock, startupPack);
	queAgentProc (childPid, startupPack, agentProcHead);
#endif

    return (childPid);
}

int
execAgent (int newSock, startupPack_t *startupPack)
{
#if windows_platform
	char *myArgv[3];
	char console_buf[20];
#else
    char *myArgv[2];
#endif
    char buf[NAME_LEN];
    char *myBuf;

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

#ifdef windows_platform  /* windows */
	iRODSNtGetAgentExecutableWithPath(buf, AGENT_EXE);
	myArgv[0] = buf;
	if(iRODSNtServerRunningConsoleMode())
	{
		strcpy(console_buf, "console");
		myArgv[1]= console_buf;
		myArgv[2] = NULL;
	}
	else
	{
		myArgv[1] = NULL;
		myArgv[2] = NULL;
	}
#else
    rstrcpy (buf, AGENT_EXE, NAME_LEN);
    myArgv[0] = buf;
    myArgv[1] = NULL;
#endif

#if windows_platform  /* windows */
	return (int)_spawnv(_P_NOWAIT,myArgv[0], myArgv);
#else
    execv(myArgv[0], myArgv);
    return (0);
#endif
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

#ifdef windows_platform
	status = startWinsock();
	if(status !=0)
	{
		rodsLog (LOG_NOTICE, "initServer: startWinsock() failed. status=%d", status);
		return -1;
	}
#endif

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

/* record the server process number and other information into
   a well-known file.  If svrComm is Null and this has created a file
   before, just unlink the file. */
int
recordServerProcess(rsComm_t *svrComm) {
#ifndef windows_platform
	int myPid;
    FILE *fd;
    DIR  *dirp;
    static char filePath[100]="";
    char cwd[1000];
    char stateFile[]="irodsServer";
    char *tmp;
    char *cp;

	 if (svrComm == NULL) {
		 if (filePath[0]!='\0') {
			 unlink(filePath);
		 }
		 return 0;
	 }
    rodsEnv *myEnv = &(svrComm->myEnv);
    
    /* Use /usr/tmp if it exists, /tmp otherwise */
    dirp = opendir("/usr/tmp");
    if (dirp!=NULL) {
       tmp="/usr/tmp";
       (void)closedir( dirp );
    }
    else {
       tmp="/tmp";
    }

    sprintf (filePath, "%s/%s.%d", tmp, stateFile, myEnv->rodsPort);

    unlink(filePath);

    myPid=getpid();
    cp = getcwd(cwd, 1000);
    if (cp != NULL) {
       fd = fopen (filePath, "w");
       if (fd != NULL) {
	  fprintf(fd, "%d %s\n", myPid, cwd);
	  fclose(fd);
	  chmod(filePath, 0664);
       }
    }
#endif
    return 0;
}
