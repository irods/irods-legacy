/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
#include "bundleDriver.h"
#include "bunSubReaddir.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"

int
rsBunSubReaddir (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubReaddirInp,
rodsDirent_t **rodsDirent)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    remoteFlag = resolveHost (&bunSubReaddirInp->addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsBunSubReaddir (rsComm, bunSubReaddirInp, rodsDirent);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteBunSubReaddir (rsComm, bunSubReaddirInp, rodsDirent,
          rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsBunSubReaddir: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remoteBunSubReaddir (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubReaddirInp,
rodsDirent_t **rodsDirent, rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteBunSubReaddir: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcBunSubReaddir (rodsServerHost->conn, bunSubReaddirInp,
      rodsDirent);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "remoteBunSubReaddir: rcFileReaddir failed for fd %d",
         bunSubReaddirInp->fd);
    }

    return status;

}

int
_rsBunSubReaddir (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubReaddirInp,
rodsDirent_t **rodsDirent)
{
    int status;

    *rodsDirent = (rodsDirent_t *) malloc (sizeof (rodsDirent_t));
    memset (*rodsDirent, 0, sizeof (rodsDirent_t));
    status = bunSubReaddir (bunSubReaddirInp->type, rsComm, 
      bunSubReaddirInp->fd, *rodsDirent);

    return (status);
}

