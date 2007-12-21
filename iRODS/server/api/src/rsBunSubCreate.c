/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
#include "bundleDriver.h"
#include "subStructFileCreate.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"

int
rsBunSubCreate (rsComm_t *rsComm, subFile_t *subFile)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int fd;

    remoteFlag = resolveHost (&subFile->addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        fd = _rsBunSubCreate (rsComm, subFile);
    } else if (remoteFlag == REMOTE_HOST) {
        fd = remoteBunSubCreate (rsComm, subFile, rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsBunSubCreate: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (fd);
}

int
remoteBunSubCreate (rsComm_t *rsComm, subFile_t *subFile,
rodsServerHost_t *rodsServerHost)
{
    int fd;
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteBunSubCreate: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    fd = rcBunSubCreate (rodsServerHost->conn, subFile);

    if (fd < 0) {
        rodsLog (LOG_NOTICE,
         "remoteBunSubCreate: rcBunSubCreate failed for %s, status = %d",
          subFile->subFilePath, fd);
    }

    return fd;
}

int
_rsBunSubCreate (rsComm_t *rsComm, subFile_t *subFile)
{
    int fd;

    fd = bunSubCreate (rsComm, subFile);

    return (fd);
}

