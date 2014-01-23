/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See dataObjRead.h for a description of this API call.*/

#include <sys/types.h>
#ifndef windows_platform
#include <sys/wait.h>
#else
#include "Unix2Nt.h"
#endif
#include "execCmd.h"
#include "objMetaOpr.h"
#include "dataObjOpr.h"
#include "miscServerFunct.h"
#include "rodsLog.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"

#ifdef USE_BOOST
#include <boost/thread/mutex.hpp>
boost::mutex ExecCmdMutex;
int initExecCmdMutex () {
    return (0);
}
#else
#ifndef windows_platform
#include <pthread.h>
pthread_mutex_t ExecCmdMutex;

int
initExecCmdMutex ()
{
    pthread_mutex_init (&ExecCmdMutex, NULL);
    return (0);
}
#endif	/* windows_platform */
#endif

int
rsExecCmd241 (rsComm_t *rsComm, execCmd241_t *execCmd241Inp, 
execCmdOut_t **execCmdOut)
{
    execCmd_t execCmdInp;
    int status;

    rstrcpy (execCmdInp.cmd, execCmd241Inp->cmd, LONG_NAME_LEN);
    rstrcpy (execCmdInp.cmdArgv, execCmd241Inp->cmdArgv, HUGE_NAME_LEN);
    rstrcpy (execCmdInp.execAddr, execCmd241Inp->execAddr, LONG_NAME_LEN);
    rstrcpy (execCmdInp.hintPath, execCmd241Inp->hintPath, LONG_NAME_LEN);
    execCmdInp.addPathToArgv = execCmd241Inp->addPathToArgv;
    execCmdInp.dummy = 0;
    bzero (&execCmdInp.condInput, sizeof (keyValPair_t));

    status = rsExecCmd (rsComm, &execCmdInp, execCmdOut);
    return (status);
}

int
rsExecCmd (rsComm_t *rsComm, execCmd_t *execCmdInp, execCmdOut_t **execCmdOut)
{
    int status;
    dataObjInfo_t *dataObjInfoHead = NULL;
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    rodsHostAddr_t addr;
    ruleExecInfo_t rei;

    initReiWithDataObjInp (&rei, rsComm, NULL);
    char *args[4];
    args[0] = execCmdInp->cmd;
    args[1] = execCmdInp->cmdArgv == NULL? (char *) "" : execCmdInp->cmdArgv;
    args[2] = execCmdInp->execAddr == NULL ? (char *) "" : execCmdInp->execAddr;
    args[3] = execCmdInp->hintPath == NULL ? (char *) "" : execCmdInp->hintPath;
    status = applyRuleArg ("acPreProcForExecCmd", args, 4, &rei, NO_SAVE_REI);
    if (status < 0) {
        rodsLog (LOG_ERROR,
                 "initAgent: acPreProcForExecCmd error, status = %d", status);
    	return (status);
    }

    /* some sanity check on the cmd path */
    if (strchr (execCmdInp->cmd, '/') != NULL) {
	rodsLog (LOG_ERROR,
	  "rsExecCmd: bad cmd path %s", execCmdInp->cmd);
	return (BAD_EXEC_CMD_PATH);
    }

    /* Also check for anonymous.  As an additional safety precaution,
       by default, do not allow the anonymous user (if defined) to
       execute commands via rcExecCmd.  If your site needs to allow
       this for some particular feature, you can remove the
       following check.
    */
    if (strncmp(ANONYMOUS_USER, rsComm->clientUser.userName,NAME_LEN) == 0) {
        return(USER_NOT_ALLOWED_TO_EXEC_CMD);
    }

    memset (&addr, 0, sizeof (addr));
    if (*execCmdInp->hintPath != '\0') {
	dataObjInp_t dataObjInp;
	memset (&dataObjInp, 0, sizeof (dataObjInp));
	rstrcpy (dataObjInp.objPath, execCmdInp->hintPath, MAX_NAME_LEN);
        status = getDataObjInfo (rsComm, &dataObjInp, &dataObjInfoHead,
          ACCESS_READ_OBJECT, 0);

        if (status < 0) {
	    rodsLog (LOG_ERROR,
              "rsExecCmd: getDataObjInfo error for hintPath %s", 
	      execCmdInp->hintPath);  
            return (status);
	}
        status = sortObjInfoForOpen (rsComm, &dataObjInfoHead, 
	  &execCmdInp->condInput, 0);
	if (status < 0) return status;

	if (execCmdInp->addPathToArgv > 0) {
	    char tmpArgv[LONG_NAME_LEN];
	    rstrcpy (tmpArgv, execCmdInp->cmdArgv, HUGE_NAME_LEN);
	    snprintf (execCmdInp->cmdArgv, HUGE_NAME_LEN, "%s %s",
	      dataObjInfoHead->filePath, tmpArgv);
	}
	rstrcpy (addr.zoneName, dataObjInfoHead->rescInfo->zoneName, 
	  NAME_LEN);
	rstrcpy (addr.hostAddr, dataObjInfoHead->rescInfo->rescLoc, 
	  LONG_NAME_LEN);
	/* just in case we have to do it remote */
	*execCmdInp->hintPath = '\0';	/* no need for hint */
        rstrcpy (execCmdInp->execAddr, dataObjInfoHead->rescInfo->rescLoc,
          LONG_NAME_LEN);
	freeAllDataObjInfo (dataObjInfoHead);
        remoteFlag = resolveHost (&addr, &rodsServerHost);
    } else if (*execCmdInp->execAddr  != '\0') {
	rstrcpy (addr.hostAddr, execCmdInp->execAddr, LONG_NAME_LEN);
        remoteFlag = resolveHost (&addr, &rodsServerHost);
    } else {
	rodsServerHost = LocalServerHost;
	remoteFlag = LOCAL_HOST;
    }
    
    if (remoteFlag == LOCAL_HOST) {
	status = _rsExecCmd (rsComm, execCmdInp, execCmdOut);
    } else if (remoteFlag == REMOTE_HOST) {
	status = remoteExecCmd (rsComm, execCmdInp, execCmdOut, 
	  rodsServerHost);
    } else {
       rodsLog (LOG_NOTICE,
         "rsFileOpenByHost: resolveHost of %s error, status = %d",
          addr.hostAddr, remoteFlag);
        status = SYS_UNRECOGNIZED_REMOTE_FLAG;
    }
     
    return (status);
}

int
remoteExecCmd (rsComm_t *rsComm, execCmd_t *execCmdInp, 
execCmdOut_t **execCmdOut, rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteExecCmd: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcExecCmd (rodsServerHost->conn, execCmdInp, execCmdOut);

    if (status < 0) {
        rodsLog (LOG_ERROR,
         "remoteExecCmd: rcExecCmd failed for %s. status = %d",
          execCmdInp->cmd, status);
    } else if (status > 0 &&
      getValByKey (&execCmdInp->condInput, STREAM_STDOUT_KW) != NULL) {
	int fileInx = status;
        (*execCmdOut)->status = bindStreamToIRods (rodsServerHost, fileInx);
        if ((*execCmdOut)->status < 0) {
	    fileCloseInp_t remFileCloseInp;
            rodsLog (LOG_ERROR,
             "remoteExecCmd: bindStreamToIRods failed. status = %d",
              (*execCmdOut)->status);
            memset (&remFileCloseInp, 0, sizeof (remFileCloseInp));
            remFileCloseInp.fileInx = fileInx;
            rcFileClose (rodsServerHost->conn, &remFileCloseInp);
	}
	status = (*execCmdOut)->status;
    } else {
	status = 0;
    }
    return status;
}

int
_rsExecCmd (rsComm_t *rsComm, execCmd_t *execCmdInp, execCmdOut_t **execCmdOut)
{
    int childPid;
    int stdoutFd[2];
    int stderrFd[2];
    int statusFd[2];
    execCmdOut_t *myExecCmdOut;
    bytesBuf_t statusBuf;
    int status, childStatus;
#ifdef windows_platform
	int pipe_buf_size = META_STR_LEN;
#endif
    
#ifndef windows_platform    /* UNIX */
    #ifdef USE_BOOST
    ExecCmdMutex.lock();
    #else
    pthread_mutex_lock (&ExecCmdMutex);
    #endif
    if (pipe (stdoutFd) < 0) 
#else
	if(_pipe(stdoutFd, pipe_buf_size, O_BINARY) < 0)
#endif
	{
        rodsLog (LOG_ERROR,
         "_rsExecCmd: pipe create failed. errno = %d", errno);
	return (SYS_PIPE_ERROR - errno);
    }

#ifndef windows_platform    /* UNIX */
    if (pipe (stderrFd) < 0) 
#else
	if(_pipe(stderrFd, pipe_buf_size, O_BINARY) < 0)
#endif
	{
        rodsLog (LOG_ERROR,
         "_rsExecCmd: pipe create failed. errno = %d", errno);
        return (SYS_PIPE_ERROR - errno);
    }

#ifndef windows_platform    /* UNIX */
    if (pipe (statusFd) < 0) 
#else
	if(_pipe(statusFd, pipe_buf_size, O_BINARY) < 0)
#endif
	{
        rodsLog (LOG_ERROR,
         "_rsExecCmd: pipe create failed. errno = %d", errno);
        return (SYS_PIPE_ERROR - errno);
    }

#ifndef windows_platform   /* UNIX */
    /* use fork instead of vfork to handle mylti-thread */
    childPid = fork ();

    #ifdef USE_BOOST
    ExecCmdMutex.unlock();
    #else
    pthread_mutex_unlock (&ExecCmdMutex);
    #endif
    if (childPid == 0) {
	char *tmpStr;
	/* Indicate that the call came from internal rule */
	if ((tmpStr = getValByKey (&execCmdInp->condInput, EXEC_CMD_RULE_KW))
	  != NULL) {
	    char *myStr = (char*)malloc (NAME_LEN + 20);
	    snprintf (myStr, NAME_LEN + 20, "%s=%s", EXEC_CMD_RULE_KW, tmpStr);
	    putenv(myStr);
//		free(myStr);	// don't free or environment is lost
	}
	close (stdoutFd[0]);
	close (stderrFd[0]);
	close (statusFd[0]);
	status = execCmd (execCmdInp, stdoutFd[1], stderrFd[1]);
	if (status < 0) {
	    status = EXEC_CMD_ERROR - errno;
	}
	/* send the status back to parent */
	write (statusFd[1], &status, 4);
	/* gets here. must be bad */
	exit(1);
    } else if (childPid < 0) { 
        rodsLog (LOG_ERROR,
         "_rsExecCmd: RODS_FORK failed. errno = %d", errno);
	return (SYS_FORK_ERROR);
    }
#else  /* Windows */
	status = execCmd (execCmdInp, stdoutFd[1], stderrFd[1]);
	if (status < 0) {
	    status = EXEC_CMD_ERROR - errno;
	}
	if(status < 0)
	{
		rodsLog (LOG_ERROR, "_rsExecCmd: RODS_FORK failed. errno = %d", errno);
		return (SYS_FORK_ERROR);
	}
#endif


    /* the parent gets here */
#ifndef windows_platform
    close (stdoutFd[1]);
    close (stderrFd[1]);
    close (statusFd[1]);
#endif


    myExecCmdOut = *execCmdOut = (execCmdOut_t*)malloc (sizeof (execCmdOut_t));
    memset (myExecCmdOut, 0, sizeof (execCmdOut_t));

    readToByteBuf (stdoutFd[0], &myExecCmdOut->stdoutBuf);
    if (getValByKey (&execCmdInp->condInput, STREAM_STDOUT_KW) != NULL &&
      myExecCmdOut->stdoutBuf.len >= MAX_SZ_FOR_EXECMD_BUF) {
	/* more to come. don't close stdoutFd. close stderrFd and statusFd
	 * because the child is not done */ 
	close (stderrFd[0]);
	close (statusFd[0]);
	myExecCmdOut->status = bindStreamToIRods (LocalServerHost, stdoutFd[0]);
	if (myExecCmdOut->status < 0) {
            rodsLog (LOG_ERROR,
             "_rsExecCmd: bindStreamToIRods failed. status = %d", 
	      myExecCmdOut->status);
	    close (stdoutFd[0]);
	}
    } else {
	close (stdoutFd[0]);
        readToByteBuf (stderrFd[0], &myExecCmdOut->stderrBuf);
	close (stderrFd[0]);
        memset (&statusBuf, 0, sizeof (statusBuf));
        readToByteBuf (statusFd[0], &statusBuf);
	close (statusFd[0]);
        if (statusBuf.len == sizeof (int) + 1) {
            myExecCmdOut->status = *((int *)statusBuf.buf);
	    free (statusBuf.buf);
        }
        childStatus = 0;

#ifndef windows_platform   /* UNIX */
        status = waitpid (childPid, &childStatus, 0);
        if (status >= 0 && myExecCmdOut->status >= 0 && childStatus != 0) {
            rodsLog (LOG_ERROR,
             "_rsExecCmd: waitpid status = %d, myExecCmdOut->status = %d, childStatus = %d", status, myExecCmdOut->status, childStatus);
            myExecCmdOut->status = EXEC_CMD_ERROR;
	}
    }
#endif
    return (myExecCmdOut->status);
}

int
execCmd (execCmd_t *execCmdInp, int stdOutFd, int stdErrFd)
{
    char cmdPath[LONG_NAME_LEN];
    char *av[LONG_NAME_LEN];
    int status;
    char *cmdDir;

    cmdDir = getenv("irodsServerCmdDir");
    if (cmdDir) {
        if (cmdDir[strlen(cmdDir)-1] == '/') {
            cmdDir[strlen(cmdDir)-1] = 0;
        }
    }
    else {
        cmdDir = CMD_DIR;
    }

    snprintf (cmdPath, LONG_NAME_LEN, "%s/%s", cmdDir, execCmdInp->cmd); 

    rodsLog(LOG_NOTICE, "execCmd:%s argv:%s", cmdPath, execCmdInp->cmdArgv);
    initCmdArg (av, execCmdInp->cmdArgv, cmdPath);

    closeAllL1desc (ThisComm);

#ifndef windows_platform
    /* set up the pipe as the stdin, stdout and stderr */
  
    close (0);
    close (1);
    close (2);

    dup2 (stdOutFd, 0);
    dup2 (stdOutFd, 1);
    dup2 (stdErrFd, 2);
    close (stdOutFd);
    close (stdErrFd);

#ifdef RUN_SERVER_AS_ROOT
    /* if we're running with root real uid, drop all root
       privilege and setuid() to the irods service user
       before running the request command */
    status = dropRootPrivilege();
    if (status < 0) {
        return (status);
    }
#endif

    status = execv (av[0], av);

#else /* Windows: Can Windows redirect the stdin, etc, to a pipe? */
	status = _spawnv(_P_NOWAIT, av[0], av);
#endif

    return (status);
}

int
initCmdArg (char *av[], char *cmdArgv, char *cmdPath)
{
  int avInx = 0, i;
    char *startPtr, *curPtr;
    int quoteCnt, curLen;
    char tmpCmdArgv[HUGE_NAME_LEN];

    av[avInx] = strdup (cmdPath);
    avInx++;

    /* parse cmdArgv */

    if (strlen (cmdArgv) > 0) {
	rstrcpy (tmpCmdArgv, cmdArgv, HUGE_NAME_LEN);
	startPtr = curPtr = tmpCmdArgv;
	quoteCnt = curLen = 0;
	while (*curPtr != '\0') {
	    if ((*curPtr == ' ' && quoteCnt == 0 && curLen > 0) || 
	      quoteCnt == 2) {
		/* end of a argv */
		*curPtr = '\0';		/* mark the end of argv */
		curPtr++;
		av[avInx] = strdup (startPtr); 
		avInx++;
		startPtr = curPtr;
		quoteCnt = curLen = 0;
	    } else if (*curPtr == ' ' && quoteCnt <= 1 && curLen == 0) {
		/* skip over a leading blank */
		curPtr++;
		startPtr = curPtr;
    /**  Added by Raja to take care of escaped quotes Oct 28 09 */
	    } else if ( (*curPtr == '\'' || *curPtr == '\"') 
			&& (*(curPtr-1) == '\\') ) {
	      curPtr++;
	      curLen++;
    /**  Added by Raja to take care of escaped quotes Oct 28 09 */
	    } else if (*curPtr == '\'' || *curPtr == '\"') {
		quoteCnt++;
		/* skip over the quote */
		if (quoteCnt == 1) {
		    curPtr++;
		    startPtr = curPtr;
		}
	    } else {
		/* just normal char */
		curPtr++;
		curLen++;
	    }
        }
	/* The last one */
	if (curLen > 0) {
	    av[avInx] = strdup (startPtr);
	    avInx++;
	}
    }
		
    /* put a NULL on the last one */

    av[avInx] = NULL;

    /**  Added by Raja to take care of escaped quotes Oct 28 09 */
    for (i = 0; i < avInx ; i++) {
      curPtr = av[i];
      startPtr = curPtr;
      while (*curPtr != '\0') {
	if (*curPtr == '\\' && (*(curPtr+1) == '\'' || *(curPtr+1) == '\"')) {
          curPtr++;
	}
	*startPtr = *curPtr;
	curPtr++;
	startPtr++;
      }
      *startPtr = '\0';
    }
    /**  Added by Raja to take care of escaped quotes Oct 28 09 */

    return (0);
}

