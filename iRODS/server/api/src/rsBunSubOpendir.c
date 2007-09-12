/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
#include "bundleDriver.h"
#include "bunSubOpendir.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"

int
rsBunSubOpendir (rsComm_t *rsComm, subFile_t *subFile)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int fd;

    remoteFlag = resolveHost (&subFile->addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        fd = _rsBunSubOpendir (rsComm, subFile);
    } else if (remoteFlag == REMOTE_HOST) {
        fd = remoteBunSubOpendir (rsComm, subFile, rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsBunSubOpendir: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (fd);
}

int
remoteBunSubOpendir (rsComm_t *rsComm, subFile_t *subFile,
rodsServerHost_t *rodsServerHost)
{
    int fd;
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteBunSubOpendir: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    fd = rcBunSubOpendir (rodsServerHost->conn, subFile);

    if (fd < 0) {
        rodsLog (LOG_NOTICE,
         "remoteBunSubOpendir: rcBunSubOpendir failed for %s, status = %d",
          subFile->subFilePath, fd);
    }

    return fd;
}

int
_rsBunSubOpendir (rsComm_t *rsComm, subFile_t *subFile)
{
    int fd;

    fd = bunSubOpendir (rsComm, subFile);

    return (fd);
}

