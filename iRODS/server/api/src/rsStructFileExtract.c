/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to subStructFiles in the COPYRIGHT directory ***/
#include "structFileDriver.h"
#include "structFileExtract.h" 
#include "miscServerFunct.h"
#include "dataObjOpr.h"

int
rsStructFileExtract (rsComm_t *rsComm, structFileOprInp_t *structFileOprInp)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    remoteFlag = resolveHost (&structFileOprInp->addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsStructFileExtract (rsComm, structFileOprInp);
    } else if (remoteFlag == REMOTE_HOST) {
        status = 
	  remoteStructFileExtract (rsComm, structFileOprInp, rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
             "rsStructFileExtract: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remoteStructFileExtract (rsComm_t *rsComm, 
structFileOprInp_t *structFileOprInp, rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteStructFileExtract: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcStructFileExtract (rodsServerHost->conn, structFileOprInp);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "remoteStructFileExtract: rcStructFileExtract failed for %s, status = %d",
          structFileOprInp->specColl->collection, status);
    }

    return status;
}

int
_rsStructFileExtract (rsComm_t *rsComm, structFileOprInp_t *structFileOprInp)
{
    int status;

    status = structFileExtract (rsComm, structFileOprInp);

    return (status);
}

