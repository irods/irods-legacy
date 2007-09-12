/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
#include "bundleDriver.h"
#include "bunSubWrite.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"

int
rsBunSubWrite (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubWriteInp,
bytesBuf_t *bunSubWriteOutBBuf)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    remoteFlag = resolveHost (&bunSubWriteInp->addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsBunSubWrite (rsComm, bunSubWriteInp, bunSubWriteOutBBuf);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteBunSubWrite (rsComm, bunSubWriteInp, bunSubWriteOutBBuf,
          rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsBunSubWrite: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remoteBunSubWrite (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubWriteInp,
bytesBuf_t *bunSubWriteOutBBuf, rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteBunSubWrite: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcBunSubWrite (rodsServerHost->conn, bunSubWriteInp,
      bunSubWriteOutBBuf);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "remoteBunSubWrite: rcFileWrite failed for fd %d",  
         bunSubWriteInp->fd);
    }

    return status;
}

int
_rsBunSubWrite (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubWriteInp,
bytesBuf_t *bunSubWriteOutBBuf)
{
    int status;

    status =  bunSubWrite (bunSubWriteInp->type, rsComm,
      bunSubWriteInp->fd, bunSubWriteOutBBuf->buf, bunSubWriteInp->len);

    return (status);
}

