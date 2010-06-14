/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsProcStat.c - server routine that handles the the ProcStat
 * API
 */

/* script generated code */
#include "procStat.h"
#include "objMetaOpr.h"
#include "miscServerFunct.h"
#include "rodsLog.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"

int
rsProcStat (rsComm_t *rsComm, procStatInp_t *procStatInp,
genQueryOut_t **procStatOut)
{
    int status;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;

    if (*procStatInp->rodsZone != '\0') {
        remoteFlag = getRcatHost (MASTER_RCAT, procStatInp->rodsZone, 
	  &rodsServerHost);
        if (remoteFlag < 0) {
            rodsLog (LOG_ERROR,
             "rsProcStat: getRcatHost() failed. erro=%d", remoteFlag);
            return (remoteFlag);
        }
        if (rodsServerHost->localFlag == REMOTE_HOST) {
	    status = remoteProcStat (rsComm, procStatInp, procStatOut,
	      rodsServerHost);
	} else {
	    status = _rsProcStat (rsComm, procStatInp, procStatOut);
	}
    } else {
	status = _rsProcStat (rsComm, procStatInp, procStatOut);
    }
    return status;
}

int
_rsProcStat (rsComm_t *rsComm, procStatInp_t *procStatInp,
genQueryOut_t **procStatOut)
{
    int status;
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    rodsHostAddr_t addr;
    procStatInp_t myProcStatInp;
    char *tmpStr;

    if (getValByKey (&procStatInp->condInput, ALL_KW) != NULL) {
	status = _rsProcStatAll (rsComm, procStatInp, procStatOut); 
	return status;
    }
    if (getValByKey (&procStatInp->condInput, EXEC_LOCALLY_KW) != NULL) {
        status = localProcStat (rsComm, procStatInp, procStatOut);
        return status;
    }

    bzero (&addr, sizeof (addr));
    bzero (&myProcStatInp, sizeof (myProcStatInp));
    if (*procStatInp->addr != '\0') {	/* given input addr */
        rstrcpy (addr.hostAddr, procStatInp->addr, LONG_NAME_LEN);
        remoteFlag = resolveHost (&addr, &rodsServerHost);
    } else if ((tmpStr = getValByKey (&procStatInp->condInput, RESC_NAME_KW)) 
      != NULL) {
	rescGrpInfo_t *rescGrpInfo = NULL;
        status = _getRescInfo (rsComm, tmpStr, &rescGrpInfo);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "_rsProcStat: _getRescInfo of %s error. stat = %d",
              tmpStr, status);
            return status;
        }
        rstrcpy (addr.hostAddr, rescGrpInfo->rescInfo->rescLoc, NAME_LEN);
        remoteFlag = resolveHost (&addr, &rodsServerHost);
    } else {
	/* do the IES server */
        remoteFlag = getRcatHost (MASTER_RCAT, NULL, &rodsServerHost);
        if (remoteFlag < 0) {
            rodsLog (LOG_ERROR,
             "_rsProcStat: getRcatHost() failed. erro=%d", remoteFlag);
            return (remoteFlag);
        }
    }
    if (remoteFlag == REMOTE_HOST) {
	addKeyVal (&myProcStatInp.condInput, EXEC_LOCALLY_KW, "");
	status = remoteProcStat (rsComm, &myProcStatInp, procStatOut,
          rodsServerHost);
    } else {
	status = localProcStat (rsComm, procStatInp, procStatOut);
    }
    return status;
}

int
_rsProcStatAll (rsComm_t *rsComm, procStatInp_t *procStatInp,
genQueryOut_t **procStatOut)
{

    return 0;
}

int
localProcStat (rsComm_t *rsComm, procStatInp_t *procStatInp,
genQueryOut_t **procStatOut)
{
    int numProc;

    numProc = getNumFilesInDir (ProcLogDir);

    if (numProc <= 0) {
	/* empty */
	initProcStatOut (procStatOut, 1);
	return numProc;
    }
    initProcStatOut (procStatOut, numProc);

    return 0;
}

int
remoteProcStat (rsComm_t *rsComm, procStatInp_t *procStatInp,
genQueryOut_t **procStatOut, rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_ERROR,
          "remoteProcStat: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcProcStat (rodsServerHost->conn, procStatInp, procStatOut);

    return status;
}

int 
initProcStatOut (genQueryOut_t **procStatOut, int numProc)
{
    genQueryOut_t *myProcStatOut;

    if (procStatOut == NULL || numProc <= 0) return USER__NULL_INPUT_ERR;

    myProcStatOut = *procStatOut = malloc (sizeof (genQueryOut_t));
    bzero (myProcStatOut, sizeof (genQueryOut_t));

    myProcStatOut->continueInx = -1;

    myProcStatOut->attriCnt = 9;

    myProcStatOut->sqlResult[0].attriInx = PID_INX;
    myProcStatOut->sqlResult[0].len = NAME_LEN;
    myProcStatOut->sqlResult[0].value =
      malloc (NAME_LEN * numProc);
    bzero (myProcStatOut->sqlResult[0].value, NAME_LEN * numProc);

    myProcStatOut->sqlResult[1].attriInx = STARTTIME_INX;
    myProcStatOut->sqlResult[1].len = NAME_LEN;
    myProcStatOut->sqlResult[1].value =
      malloc (NAME_LEN * numProc);
    bzero (myProcStatOut->sqlResult[1].value, NAME_LEN * numProc);

    myProcStatOut->sqlResult[2].attriInx = CLIENT_NAME_INX;
    myProcStatOut->sqlResult[2].len = NAME_LEN;
    myProcStatOut->sqlResult[2].value =
      malloc (NAME_LEN * numProc);
    bzero (myProcStatOut->sqlResult[2].value, NAME_LEN * numProc);

    myProcStatOut->sqlResult[3].attriInx = CLIENT_ZONE_INX;
    myProcStatOut->sqlResult[3].len = NAME_LEN;
    myProcStatOut->sqlResult[3].value =
      malloc (NAME_LEN * numProc);
    bzero (myProcStatOut->sqlResult[3].value, NAME_LEN * numProc);

    myProcStatOut->sqlResult[4].attriInx = PROXY_NAME_INX;
    myProcStatOut->sqlResult[4].len = NAME_LEN;
    myProcStatOut->sqlResult[4].value =
      malloc (NAME_LEN * numProc);
    bzero (myProcStatOut->sqlResult[4].value, NAME_LEN * numProc);

    myProcStatOut->sqlResult[5].attriInx = PROXY_ZONE_INX;
    myProcStatOut->sqlResult[5].len = NAME_LEN;
    myProcStatOut->sqlResult[5].value =
      malloc (NAME_LEN * numProc);
    bzero (myProcStatOut->sqlResult[5].value, NAME_LEN * numProc);

    myProcStatOut->sqlResult[6].attriInx = REMOTE_ADDR_INX;
    myProcStatOut->sqlResult[6].len = NAME_LEN;
    myProcStatOut->sqlResult[6].value =
      malloc (NAME_LEN * numProc);
    bzero (myProcStatOut->sqlResult[6].value, NAME_LEN * numProc);

    myProcStatOut->sqlResult[7].attriInx = PROG_NAME_INX;
    myProcStatOut->sqlResult[7].len = NAME_LEN;
    myProcStatOut->sqlResult[7].value =
      malloc (NAME_LEN * numProc);
    bzero (myProcStatOut->sqlResult[7].value, NAME_LEN * numProc);

    myProcStatOut->sqlResult[8].attriInx = SERVER_ADDR_INX;
    myProcStatOut->sqlResult[8].len = NAME_LEN;
    myProcStatOut->sqlResult[8].value =
      malloc (NAME_LEN * numProc);
    bzero (myProcStatOut->sqlResult[8].value, NAME_LEN * numProc);


    return 0;    
}

