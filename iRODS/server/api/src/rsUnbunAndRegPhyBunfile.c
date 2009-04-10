/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsStructFileExtAndReg.c. See structFileExtAndReg.h for a description of 
 * this API call.*/

#include "unbunAndRegPhyBunfile.h"
#include "apiHeaderAll.h"
#include "miscServerFunct.h"
#include "objMetaOpr.h"
#include "dataObjOpr.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"

int
rsUnbunAndRegPhyBunfile (rsComm_t *rsComm, dataObjInp_t *dataObjInp)
{
    int status;
    char *rescName;
    rescGrpInfo_t *rescGrpInfo = NULL;

    if ((rescName = getValByKey (&dataObjInp->condInput, DEST_RESC_NAME_KW)) 
      == NULL) {
        return USER_NO_RESC_INPUT_ERR;
    }

    status = resolveAndQueResc (rescName, NULL, &rescGrpInfo);
    if (status < 0) {
        rodsLog (LOG_NOTICE,
          "rsUnbunAndRegPhyBunfile: resolveAndQueRescerror for %s, status = %d",
          rescName, status);
        return (status);
    }

    status = _rsUnbunAndRegPhyBunfile (rsComm, dataObjInp, 
      rescGrpInfo->rescInfo);

    return (status);
}

int
_rsUnbunAndRegPhyBunfile (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
rescInfo_t *rescInfo)
{
    int status;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;

    remoteFlag = resolveHostByRescInfo (rescInfo, &rodsServerHost);

    if (remoteFlag == REMOTE_HOST) {
        addKeyVal (&dataObjInp->condInput, DEST_RESC_NAME_KW, 
	  rescInfo->rescName);
	status = remoteUnbunAndRegPhyBunfile (rsComm, dataObjInp, 
	  rodsServerHost);
    }

    return (status);
}

int
remoteUnbunAndRegPhyBunfile (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteUnbunAndRegPhyBunfile: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcUnbunAndRegPhyBunfile (rodsServerHost->conn, dataObjInp);

    return status;
}

