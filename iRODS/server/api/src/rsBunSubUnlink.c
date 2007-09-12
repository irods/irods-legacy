/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
#include "bundleDriver.h"
#include "bunSubUnlink.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"

int
rsBunSubUnlink (rsComm_t *rsComm, subFile_t *subFile)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    remoteFlag = resolveHost (&subFile->addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsBunSubUnlink (rsComm, subFile);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteBunSubUnlink (rsComm, subFile, rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsBunSubUnlink: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remoteBunSubUnlink (rsComm_t *rsComm, subFile_t *subFile,
rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteBunSubUnlink: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcBunSubUnlink (rodsServerHost->conn, subFile);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "remoteBunSubUnlink: rcBunSubUnlink failed for %s, status = %d",
          subFile->subFilePath, status);
    }

    return status;
}

int
_rsBunSubUnlink (rsComm_t *rsComm, subFile_t *subFile)
{
    int status;

    status = bunSubUnlink (rsComm, subFile);

    return (status);
}

