/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsBulkDataObjPut.c. See bulkDataObjReg.h for a description of
 * this API call.*/

#include "apiHeaderAll.h"
#include "objMetaOpr.h"
#include "dataObjOpr.h"
#include "miscServerFunct.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"

int
rsBulkDataObjPut (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
bytesBuf_t *dataObjInpBBuf)
{
    int status;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    specCollCache_t *specCollCache = NULL;

    resolveLinkedPath (rsComm, dataObjInp->objPath, &specCollCache,
      &dataObjInp->condInput);

    remoteFlag = getAndConnRemoteZone (rsComm, dataObjInp, &rodsServerHost,
      REMOTE_CREATE);

    if (remoteFlag < 0) {
        return (remoteFlag);
    } else if (remoteFlag == LOCAL_HOST) {
        status = _rsBulkDataObjPut (rsComm, dataObjInp, dataObjInpBBuf);
    } else {
        status = rcBulkDataObjPut (rodsServerHost->conn, dataObjInp,
          dataObjInpBBuf);
    }
    return status;
}

int
_rsBulkDataObjPut (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
bytesBuf_t *dataObjInpBBuf)
{
    int status;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    rescInfo_t *rescInfo;
    char *inpRescGrpName;
    rescGrpInfo_t *myRescGrpInfo = NULL;

    inpRescGrpName = getValByKey (&dataObjInp->condInput, RESC_GROUP_NAME_KW);

    /* query rcat for resource info and sort it */

    status = getRescGrpForCreate (rsComm, dataObjInp, &myRescGrpInfo);
    if (status < 0) return status;

    /* just take the top one */
    rescInfo = myRescGrpInfo->rescInfo;

    remoteFlag = resolveHostByRescInfo (rescInfo, &rodsServerHost);

    if (remoteFlag == REMOTE_HOST) {
        addKeyVal (&dataObjInp->condInput, DEST_RESC_NAME_KW,
          rescInfo->rescName);
	if (inpRescGrpName == NULL && 
	  strlen (myRescGrpInfo->rescGroupName) > 0) {
            addKeyVal (&dataObjInp->condInput, RESC_GROUP_NAME_KW,
              myRescGrpInfo->rescGroupName);
	}
        if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
            return status;
        }
        status = rcBulkDataObjPut (rodsServerHost->conn, dataObjInp, 
          dataObjInpBBuf);
    }
    return status;
}


