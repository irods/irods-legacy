/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
#include "bundleDriver.h"
#include "subStructFileFstat.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"

int
rsBunSubFstat (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubFstatInp,
rodsStat_t **bunSubStatOut)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    remoteFlag = resolveHost (&bunSubFstatInp->addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsBunSubFstat (rsComm, bunSubFstatInp, bunSubStatOut);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteBunSubFstat (rsComm, bunSubFstatInp, bunSubStatOut,
          rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsBunSubFstat: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remoteBunSubFstat (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubFstatInp,
rodsStat_t **bunSubStatOut, rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteBunSubFstat: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcBunSubFstat (rodsServerHost->conn, bunSubFstatInp,
      bunSubStatOut);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "remoteBunSubFstat: rcFileFstat failed for fd %d",
         bunSubFstatInp->fd);
    }

    return status;

}

int
_rsBunSubFstat (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubFstatInp,
rodsStat_t **bunSubStatOut)
{
    int status;

    *bunSubStatOut = (rodsStat_t *) malloc (sizeof (rodsStat_t));
    memset (*bunSubStatOut, 0, sizeof (rodsStat_t));
    status = bunSubFstat (bunSubFstatInp->type, rsComm, bunSubFstatInp->fd,
      *bunSubStatOut);

    return (status);
}

