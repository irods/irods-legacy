/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
#include "bundleDriver.h"
#include "subStructFileRmdir.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"

int
rsBunSubRmdir (rsComm_t *rsComm, subFile_t *subFile)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int fd;

    remoteFlag = resolveHost (&subFile->addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        fd = _rsBunSubRmdir (rsComm, subFile);
    } else if (remoteFlag == REMOTE_HOST) {
        fd = remoteBunSubRmdir (rsComm, subFile, rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsBunSubRmdir: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (fd);
}

int
remoteBunSubRmdir (rsComm_t *rsComm, subFile_t *subFile,
rodsServerHost_t *rodsServerHost)
{
    int fd;
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteBunSubRmdir: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    fd = rcBunSubRmdir (rodsServerHost->conn, subFile);

    if (fd < 0) {
        rodsLog (LOG_NOTICE,
         "remoteBunSubRmdir: rcBunSubRmdir failed for %s, status = %d",
          subFile->subFilePath, fd);
    }

    return fd;
}

int
_rsBunSubRmdir (rsComm_t *rsComm, subFile_t *subFile)
{
    int fd;

    fd = bunSubRmdir (rsComm, subFile);

    return (fd);
}

