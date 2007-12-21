/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
#include "bundleDriver.h"
#include "subStructFileRead.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"

int
rsBunSubRead (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubReadInp,
bytesBuf_t *bunSubReadOutBBuf)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    remoteFlag = resolveHost (&bunSubReadInp->addr, &rodsServerHost);

    if (bunSubReadInp->len > 0) {
        if (bunSubReadOutBBuf->buf == NULL)
            bunSubReadOutBBuf->buf = malloc (bunSubReadInp->len);
    } else {
        return (0);
    }

    if (remoteFlag == LOCAL_HOST) {
        status = _rsBunSubRead (rsComm, bunSubReadInp, bunSubReadOutBBuf);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteBunSubRead (rsComm, bunSubReadInp, bunSubReadOutBBuf, 
	  rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsBunSubRead: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remoteBunSubRead (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubReadInp, 
bytesBuf_t *bunSubReadOutBBuf, rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteBunSubRead: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcBunSubRead (rodsServerHost->conn, bunSubReadInp,
      bunSubReadOutBBuf);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "remoteBunSubRead: rcFileRead failed for fd %d",  bunSubReadInp->fd);
    }

    return status;

}

int
_rsBunSubRead (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubReadInp,
bytesBuf_t *bunSubReadOutBBuf)
{
    int status;

    status =  bunSubRead (bunSubReadInp->type, rsComm,
      bunSubReadInp->fd, bunSubReadOutBBuf->buf, bunSubReadInp->len);

    if (status > 0) {
        bunSubReadOutBBuf->len = status;
    } else {
	bunSubReadOutBBuf->len = 0;
    }
    return (status);
}

