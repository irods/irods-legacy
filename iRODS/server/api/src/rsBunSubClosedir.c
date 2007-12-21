/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
#include "bundleDriver.h"
#include "subStructFileClosedir.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"

int
rsBunSubClosedir (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubClosedirInp)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    remoteFlag = resolveHost (&bunSubClosedirInp->addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsBunSubClosedir (rsComm, bunSubClosedirInp);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteBunSubClosedir (rsComm, bunSubClosedirInp,
          rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsBunSubClosedir: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remoteBunSubClosedir (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubClosedirInp,
rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteBunSubClosedir: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcBunSubClosedir (rodsServerHost->conn, bunSubClosedirInp);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "remoteBunSubClosedir: rcFileClosedir failed for fd %d",
         bunSubClosedirInp->fd);
    }

    return status;
}

int
_rsBunSubClosedir (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubClosedirInp)
{
    int status;

    status =  bunSubClosedir (bunSubClosedirInp->type, rsComm,
      bunSubClosedirInp->fd);

    return (status);
}

