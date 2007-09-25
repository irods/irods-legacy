/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See dataObjRead.h for a description of this API call.*/

#include <sys/types.h>
#include <sys/wait.h>
#include "execCmd.h"
#include "objMetaOpr.h"
#include "miscServerFunct.h"
#include "rodsLog.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"


int
rsExecCmd (rsComm_t *rsComm, execCmd_t *execCmdInp, execCmdOut_t **execCmdOut)
{
    int status;
    dataObjInfo_t *dataObjInfoHead = NULL;
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    rodsHostAddr_t addr;

    /* some sanity check on the cmd path */
    if (strchr (execCmdInp->cmd, '/') != NULL) {
	rodsLog (LOG_ERROR,
	  "rsExecCmd: bad cmd path %s", execCmdInp->cmd);
	return (BAD_EXEC_CMD_PATH);
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
        sortObjInfoForOpen (&dataObjInfoHead, &execCmdInp->condInput, 0);

	if (execCmdInp->addPathToArgv > 0) {
	    char tmpArgv[LONG_NAME_LEN];
	    rstrcpy (tmpArgv, execCmdInp->cmdArgv, LONG_NAME_LEN);
	    snprintf (execCmdInp->cmdArgv, LONG_NAME_LEN, "%s %s",
	      dataObjInfoHead->filePath, tmpArgv);
	}
	rstrcpy (addr.rodsZone, dataObjInfoHead->rescInfo->zoneName, 
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
    
    if (pipe (stdoutFd) < 0) {
        rodsLog (LOG_ERROR,
         "_rsExecCmd: pipe create failed. errno = %d", errno);
	return (SYS_PIPE_ERROR - errno);
    }

    if (pipe (stderrFd) < 0) {
        rodsLog (LOG_ERROR,
         "_rsExecCmd: pipe create failed. errno = %d", errno);
        return (SYS_PIPE_ERROR - errno);
    }

    if (pipe (statusFd) < 0) {
        rodsLog (LOG_ERROR,
         "_rsExecCmd: pipe create failed. errno = %d", errno);
        return (SYS_PIPE_ERROR - errno);
    }

    childPid = RODS_FORK ();

    if (childPid == 0) {
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

    /* the parent gets here */
    close (stdoutFd[1]);
    close (stderrFd[1]);
    close (statusFd[1]);

    myExecCmdOut = *execCmdOut = malloc (sizeof (execCmdOut_t));
    memset (myExecCmdOut, 0, sizeof (execCmdOut_t));

    readToByteBuf (stdoutFd[0], &myExecCmdOut->stdoutBuf);
    readToByteBuf (stderrFd[0], &myExecCmdOut->stderrBuf);
    memset (&statusBuf, 0, sizeof (statusBuf));
    readToByteBuf (statusFd[0], &statusBuf);
    if (statusBuf.len == sizeof (int) + 1) {
        myExecCmdOut->status = *((int *)statusBuf.buf);
	free (statusBuf.buf);
    }
    status = waitpid (childPid, &childStatus, WNOHANG);
    if (status >= 0 && myExecCmdOut->status >= 0 && childStatus != 0) {
        myExecCmdOut->status = EXEC_CMD_ERROR;
    }
    return (myExecCmdOut->status);
}

int
readToByteBuf (int fd, bytesBuf_t *bytesBuf)
{
    int toRead, buflen, nbytes;
    char *bufptr;

    bytesBuf->buf = bufptr = malloc (MAX_NAME_LEN * 5);
    bytesBuf->len = 0;
    buflen = toRead = MAX_NAME_LEN * 5;
    while (1) {
        nbytes = myRead (fd, bufptr, toRead, SOCK_TYPE, NULL);
	if (nbytes == toRead) {	/* more */
	    toRead = buflen;
	    buflen = 2 * buflen;
	    if (buflen > MAX_SZ_FOR_SINGLE_BUF) {
		close (fd);
		return (EXEC_CMD_OUTPUT_TOO_LARGE);
	    } 
	    bytesBuf->buf = malloc (buflen);
	    memcpy (bytesBuf->buf, bufptr, toRead);
	    free (bufptr);
	    bufptr = (char *) bytesBuf->buf + toRead;
	    bytesBuf->len += nbytes;
	} else {
	    if (nbytes > 0) {
		bytesBuf->len += nbytes;
		bufptr += nbytes;
	    }
	    if (bytesBuf->len > 0) {
		/* add NULL termination */
                *bufptr = '\0';
	        bytesBuf->len++;
	    } else {
		free (bytesBuf->buf);
		bytesBuf->buf = NULL;
	    }
	    break;
        }
    }
    close (fd);

    if (nbytes < 0) {
	return (nbytes);
    } else {
	return (0);
    }
}

int
execCmd (execCmd_t *execCmdInp, int stdOutFd, int stdErrFd)
{
    char cmdPath[LONG_NAME_LEN];
    char *av[LONG_NAME_LEN];
    int status;

    snprintf (cmdPath, LONG_NAME_LEN, "%s/%s", CMD_DIR, execCmdInp->cmd); 

    initCmdArg (av, execCmdInp->cmdArgv, cmdPath);

    closeAllL1desc (ThisComm);

    /* set up the pipe as the stdin, stdout and stderr */
  
    close (0);
    close (1);
    close (2);

    dup2 (stdOutFd, 0);
    dup2 (stdOutFd, 1);
    dup2 (stdErrFd, 2);
    close (stdOutFd);
    close (stdErrFd);

    status = execv (av[0], av);
    return (status);
}

int
initCmdArg (char *av[], char *cmdArgv, char *cmdPath)
{
    int avInx = 0;
    char *startPtr, *curPtr;
    int quoteCnt, curLen;
    char tmpCmdArgv[MAX_NAME_LEN];

    av[avInx] = strdup (cmdPath);
    avInx++;

    /* parse cmdArgv */

    if (strlen (cmdArgv) > 0) {
	rstrcpy (tmpCmdArgv, cmdArgv, MAX_NAME_LEN);
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

    return (0);
}

