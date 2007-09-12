/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
#include "bundleDriver.h"
#include "bunSubOpen.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"

int
rsBunSubOpen (rsComm_t *rsComm, subFile_t *subFile)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int fd;

    remoteFlag = resolveHost (&subFile->addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        fd = _rsBunSubOpen (rsComm, subFile);
    } else if (remoteFlag == REMOTE_HOST) {
        fd = remoteBunSubOpen (rsComm, subFile, rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsBunSubOpen: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (fd);
}

int
remoteBunSubOpen (rsComm_t *rsComm, subFile_t *subFile,
rodsServerHost_t *rodsServerHost)
{
    int fd;
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteBunSubOpen: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    fd = rcBunSubOpen (rodsServerHost->conn, subFile);

    if (fd < 0) {
        rodsLog (LOG_NOTICE,
         "remoteBunSubOpen: rcBunSubOpen failed for %s, status = %d",
          subFile->subFilePath, fd);
    }

    return fd;
}

int
_rsBunSubOpen (rsComm_t *rsComm, subFile_t *subFile)
{
    int fd;

    fd = bunSubOpen (rsComm, subFile);

    return (fd);
}


