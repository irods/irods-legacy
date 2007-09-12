/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
#include "bundleDriver.h"
#include "bunSubClose.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"

int
rsBunSubClose (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubCloseInp)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    remoteFlag = resolveHost (&bunSubCloseInp->addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsBunSubClose (rsComm, bunSubCloseInp);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteBunSubClose (rsComm, bunSubCloseInp,
          rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsBunSubClose: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remoteBunSubClose (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubCloseInp,
rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteBunSubClose: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcBunSubClose (rodsServerHost->conn, bunSubCloseInp);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "remoteBunSubClose: rcFileClose failed for fd %d",
         bunSubCloseInp->fd);
    }

    return status;
}

int
_rsBunSubClose (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubCloseInp)
{
    int status;

    status =  bunSubClose (bunSubCloseInp->type, rsComm,
      bunSubCloseInp->fd);

    return (status);
}

