/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
#include "bundleDriver.h"
#include "bunSubLseek.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"

int
rsBunSubLseek (rsComm_t *rsComm, bunSubLseekInp_t *bunSubLseekInp,
fileLseekOut_t **bunSubLseekOut)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    remoteFlag = resolveHost (&bunSubLseekInp->addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsBunSubLseek (rsComm, bunSubLseekInp, bunSubLseekOut);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteBunSubLseek (rsComm, bunSubLseekInp, bunSubLseekOut,
          rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsBunSubLseek: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remoteBunSubLseek (rsComm_t *rsComm, bunSubLseekInp_t *bunSubLseekInp,
fileLseekOut_t **bunSubLseekOut, rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteBunSubLseek: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcBunSubLseek (rodsServerHost->conn, bunSubLseekInp,
      bunSubLseekOut);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "remoteBunSubLseek: rcFileLseek failed for fd %d",
         bunSubLseekInp->fd);
    }

    return status;
}

int
_rsBunSubLseek (rsComm_t *rsComm, bunSubLseekInp_t *bunSubLseekInp,
fileLseekOut_t **bunSubLseekOut)
{
    rodsLong_t lStatus;
    int status;

    *bunSubLseekOut = (fileLseekOut_t *) malloc (sizeof (fileLseekOut_t));
    memset (*bunSubLseekOut, 0, sizeof (fileLseekOut_t));
    lStatus = bunSubLseek (bunSubLseekInp->type, rsComm, bunSubLseekInp->fd,
      bunSubLseekInp->offset, bunSubLseekInp->whence);

    if (lStatus < 0) {
        status = lStatus;
        rodsLog (LOG_ERROR,
          "rsBunSubLseek: bunSubLseek failed for %d, status = %d",
          bunSubLseekInp->fd, status);
        return (status);
    } else {
        *bunSubLseekOut = (fileLseekOut_t *) malloc (sizeof (fileLseekOut_t));
        memset (*bunSubLseekOut, 0, sizeof (fileLseekOut_t));
        (*bunSubLseekOut)->offset = lStatus;
        status = 0;
    }

    return (status);
}

