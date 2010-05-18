/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* See databaseObjInfo.h for a description of this API call.*/

#include "databaseObjInfo.h"
#include "objMetaOpr.h"
#include "miscServerFunct.h"
#include "dboHighLevelRoutines.h"

int
remoteDatabaseObjInfo(rsComm_t *rsComm,
		      databaseObjInfoInp_t *databaseObjInfoInp,
		      databaseObjInfoOut_t **databaseObjInfoOut,
		      rodsServerHost_t *rodsServerHost) {
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteDatabaseInfo: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcDatabaseObjInfo(rodsServerHost->conn,
			       databaseObjInfoInp, databaseObjInfoOut);
    return status;
}

int
rsDatabaseObjInfo (rsComm_t *rsComm, databaseObjInfoInp_t *databaseObjInfoInp,
		      databaseObjInfoOut_t **databaseObjInfoOut)
{
    rodsServerHost_t *rodsServerHost;
    int status;
    int remoteFlag;
    rodsHostAddr_t rescAddr;
    rescGrpInfo_t *rescGrpInfo = NULL;

    status = _getRescInfo (rsComm, databaseObjInfoInp->dbrName, &rescGrpInfo);
    if (status < 0) {
	 rodsLog (LOG_ERROR,
		  "rsDatabaseObjInfo: _getRescInfo of %s error, stat = %d",
		  databaseObjInfoInp->dbrName, status);
	return status;
    }

    bzero (&rescAddr, sizeof (rescAddr));
    rstrcpy (rescAddr.hostAddr, rescGrpInfo->rescInfo->rescLoc, NAME_LEN);
    remoteFlag = resolveHost (&rescAddr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
       status = _rsDatabaseObjInfo(rsComm, databaseObjInfoInp,
				   databaseObjInfoOut);
    } else if (remoteFlag == REMOTE_HOST) {
       status = remoteDatabaseObjInfo(rsComm,
				      databaseObjInfoInp, 
				      databaseObjInfoOut,
				      rodsServerHost );
    } else if (remoteFlag < 0) {
       status = remoteFlag;
    }

    if (status < 0 ) { 
        rodsLog (LOG_NOTICE,
		 "rsDatabaseObjInfo: rcDatabaseObjInfo failed, status = %d",
		 status);
    }
    return (status);
}


int
_rsDatabaseObjInfo (rsComm_t *rsComm, databaseObjInfoInp_t *databaseObjInfoInp,
		    databaseObjInfoOut_t **databaseObjInfoOut) {
#ifdef DBO
    int status;

    int maxBufSize;
    char *outBuf;

    databaseObjInfoOut_t *myObjInfoOut;

    maxBufSize = 1024*50;

    outBuf = malloc(maxBufSize);
    *outBuf='\0';

    if (databaseObjInfoInp->objDesc < 0) {
       status = dboReadConfigItems(outBuf, maxBufSize);
    }
    else {
       status = dboGetInfo(databaseObjInfoInp->objDesc, outBuf, maxBufSize);
    }

    if (status < 0 ) { 
       rodsLog (LOG_NOTICE, 
		"_rsDatabaseObjInfo: databaseObjInfo status = %d",
		status);
       return (status);
    }

    myObjInfoOut = malloc(sizeof(databaseObjInfoOut_t));
    myObjInfoOut = malloc(100);
    myObjInfoOut->outBuf = outBuf;

    *databaseObjInfoOut = myObjInfoOut;

    return (status);
#else
    return(DBO_NOT_COMPILED_IN);
#endif
}
