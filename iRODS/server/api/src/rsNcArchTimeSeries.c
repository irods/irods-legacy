/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See ncArchTimeSeries.h for a description of this API call.*/

#include "ncArchTimeSeries.h"
#include "rodsLog.h"
#include "dataObjOpen.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "rsApiHandler.h"
#include "objMetaOpr.h"
#include "physPath.h"
#include "specColl.h"
#include "getRemoteZoneResc.h"
#include "miscServerFunct.h"
#include "dataObjCreate.h"
#include "ncGetAggInfo.h"

int
rsNcArchTimeSeries (rsComm_t *rsComm,
ncArchTimeSeriesInp_t *ncArchTimeSeriesInp) 
{
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    int status;
    dataObjInp_t dataObjInp;
    specCollCache_t *specCollCache = NULL;

    if (getValByKey (&ncArchTimeSeriesInp->condInput, NATIVE_NETCDF_CALL_KW) 
      != NULL) {
        /* must to be called internally */
        if (rsComm->proxyUser.authInfo.authFlag < REMOTE_PRIV_USER_AUTH) {
            return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
        }
        status = _rsNcArchTimeSeries (rsComm, ncArchTimeSeriesInp);
    } else {
        resolveLinkedPath (rsComm, ncArchTimeSeriesInp->objPath, 
          &specCollCache, &ncArchTimeSeriesInp->condInput);
        bzero (&dataObjInp, sizeof (dataObjInp));
        rstrcpy (dataObjInp.objPath, ncArchTimeSeriesInp->objPath, 
          MAX_NAME_LEN);
        replKeyVal (&ncArchTimeSeriesInp->condInput, &dataObjInp.condInput);
        remoteFlag = getAndConnRemoteZone (rsComm, &dataObjInp, &rodsServerHost,
          REMOTE_CREATE);
        clearKeyVal (&dataObjInp.condInput);
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else if (remoteFlag == LOCAL_HOST) {
            status = _rsNcArchTimeSeries (rsComm, ncArchTimeSeriesInp);
        } else {
            if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0)
                return status;
            status = rcNcArchTimeSeries (rodsServerHost->conn, 
              ncArchTimeSeriesInp);
        }
    }
    return status;
}

int
_rsNcArchTimeSeries (rsComm_t *rsComm,
ncArchTimeSeriesInp_t *ncArchTimeSeriesInp) 
{
    int status;
    rescGrpInfo_t *myRescGrpInfo = NULL;
    rescGrpInfo_t *tmpRescGrpInfo;
    rescInfo_t *tmpRescInfo;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    dataObjInp_t dataObjInp;
    int l1descInx;
    unsigned int endTime;
    char myDir[MAX_NAME_LEN], myFile[MAX_NAME_LEN], basePath[MAX_NAME_LEN];
    ncOpenInp_t ncOpenInp;
    int *ncid = NULL;
    ncInqInp_t ncInqInp;
    ncInqOut_t *ncInqOut = NULL;
    ncAggInfo_t *ncAggInfo = NULL;

    bzero (&dataObjInp, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, ncArchTimeSeriesInp->aggCollection,
      MAX_NAME_LEN);
    replKeyVal (&ncArchTimeSeriesInp->condInput, &dataObjInp.condInput);
    
    status = getRescGrpForCreate (rsComm, &dataObjInp, &myRescGrpInfo);
    clearKeyVal (&dataObjInp.condInput);
    
    /* pick this first local host */
    tmpRescGrpInfo = myRescGrpInfo;
    while (tmpRescGrpInfo != NULL) {
        tmpRescInfo = tmpRescGrpInfo->rescInfo;
        if (isLocalHost (tmpRescInfo->rescLoc)) break; 
        tmpRescGrpInfo = tmpRescGrpInfo->next;
    }
    if (tmpRescGrpInfo == NULL) {
        /* we don't have a local resource */
        if (getValByKey (&ncArchTimeSeriesInp->condInput, NATIVE_NETCDF_CALL_KW)
          != NULL) {
            rodsLog (LOG_ERROR,
              "_rsNcArchTimeSeries: No local resc for NATIVE_NETCDF_CALL of %s",
              ncArchTimeSeriesInp->objPath);
            return SYS_INVALID_RESC_INPUT;
        } else {
            remoteFlag = resolveHostByRescInfo (myRescGrpInfo->rescInfo,
              &rodsServerHost);
            if (remoteFlag < 0) {
                return (remoteFlag);
            } else if (remoteFlag == REMOTE_HOST) {
                addKeyVal (&ncArchTimeSeriesInp->condInput, 
                  NATIVE_NETCDF_CALL_KW, "");
                if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0)
                    return status;
                status = rcNcArchTimeSeries (rodsServerHost->conn,
                  ncArchTimeSeriesInp);
                return status;
            }
        }
    }
    /* get here when tmpRescGrpInfo != NULL. Will do it locally */ 

    status = readAggInfo (rsComm, ncArchTimeSeriesInp->aggCollection,
      NULL, &ncAggInfo);
    if (status < 0) return status;
    endTime = ncAggInfo->ncAggElement[ncAggInfo->numFiles - 1].endTime;
    addKeyVal (&dataObjInp.condInput, NO_OPEN_FLAG_KW, "");
    if ((status = splitPathByKey (ncArchTimeSeriesInp->aggCollection, 
      myDir, myFile, '/')) < 0) {
        rodsLogError (LOG_ERROR, status,
          "_rsNcArchTimeSeries: splitPathByKey error for %s",
          ncArchTimeSeriesInp->aggCollection);
        return status;
    }
    snprintf (basePath, MAX_NAME_LEN, "%s/%s", 
      ncArchTimeSeriesInp->aggCollection, myFile);
    getNextAggEleObjPath (ncAggInfo, basePath, dataObjInp.objPath);
    l1descInx = _rsDataObjCreateWithRescInfo (rsComm, &dataObjInp,
      tmpRescInfo, myRescGrpInfo->rescGroupName);
    if (l1descInx < 0) {
        freeAllRescGrpInfo (myRescGrpInfo);
        return l1descInx;
    }
    bzero (&ncOpenInp, sizeof (ncOpenInp_t));
    rstrcpy (ncOpenInp.objPath, ncArchTimeSeriesInp->objPath, MAX_NAME_LEN);
#ifdef NETCDF4_API
    ncOpenInp.mode = NC_NOWRITE|NC_NETCDF4;
#else
    ncOpenInp.mode = NC_NOWRITE;
#endif
    addKeyVal (&ncOpenInp.condInput, NO_STAGING_KW, "");

    status = rsNcOpen (rsComm, &ncOpenInp, &ncid);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "_rsNcArchTimeSeries: rsNcOpen error for %s", ncOpenInp.objPath);
        return status;
    }

    bzero (&ncInqInp, sizeof (ncInqInp));
    ncInqInp.ncid = *ncid;
    free (ncid);
    ncInqInp.paramType = NC_ALL_TYPE;
    ncInqInp.flags = NC_ALL_FLAG;
    status = rsNcInq (rsComm, &ncInqInp, &ncInqOut);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "_rsNcArchTimeSeries: rcNcInq error for %s", ncOpenInp.objPath);
        return status;
    }
    return status;
}

