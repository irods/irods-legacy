/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
#include "bundleDriver.h"
#include "subStructFileRename.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"

int
rsBunSubRename (rsComm_t *rsComm, bunSubRenameInp_t *bunSubRenameInp)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    remoteFlag = resolveHost (&bunSubRenameInp->subFile.addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsBunSubRename (rsComm, bunSubRenameInp);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteBunSubRename (rsComm, bunSubRenameInp, rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsBunSubRename: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remoteBunSubRename (rsComm_t *rsComm, bunSubRenameInp_t *bunSubRenameInp,
rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteBunSubRename: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcBunSubRename (rodsServerHost->conn, bunSubRenameInp);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "remoteBunSubRename: rcBunSubRename failed for %s, status = %d",
          bunSubRenameInp->subFile.subFilePath, status);
    }

    return status;
}

int
_rsBunSubRename (rsComm_t *rsComm, bunSubRenameInp_t *bunSubRenameInp)
{
    int status;

    status = bunSubRename (rsComm, &bunSubRenameInp->subFile,
      bunSubRenameInp->newSubFilePath);

    return (status);
}

