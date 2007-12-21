/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
#include "bundleDriver.h"
#include "subStructFileStat.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"


int
rsBunSubStat (rsComm_t *rsComm, subFile_t *subFile, rodsStat_t **bunSubStatOut)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    remoteFlag = resolveHost (&subFile->addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsBunSubStat (rsComm, subFile, bunSubStatOut);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteBunSubStat (rsComm, subFile, bunSubStatOut,
	  rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsBunSubStat: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remoteBunSubStat (rsComm_t *rsComm, subFile_t *subFile,
rodsStat_t **bunSubStatOut, rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteBunSubStat: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcBunSubStat (rodsServerHost->conn, subFile, bunSubStatOut);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "remoteBunSubStat: rcBunSubStat failed for %s, status = %d",
          subFile->subFilePath, status);
    }

    return status;
}

int
_rsBunSubStat (rsComm_t *rsComm, subFile_t *subFile, rodsStat_t **bunSubStatOut)
{
    int status;

    *bunSubStatOut = (rodsStat_t *) malloc (sizeof (rodsStat_t));
    memset (*bunSubStatOut, 0, sizeof (rodsStat_t));
    status = bunSubStat (rsComm, subFile, *bunSubStatOut);
    if (status < 0) {
	free (*bunSubStatOut);
	*bunSubStatOut = NULL;
    } 

    return (status);
}

