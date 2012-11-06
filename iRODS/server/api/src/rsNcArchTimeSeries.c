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
#include "ncClose.h"
#include "ncInq.h"

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
    int dimInx, varInx;
    rescGrpInfo_t *myRescGrpInfo = NULL;
    rescGrpInfo_t *tmpRescGrpInfo;
    rescInfo_t *tmpRescInfo;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    dataObjInp_t dataObjInp;
    int l1descInx;
    unsigned int endTime;
    ncOpenInp_t ncOpenInp;
    ncCloseInp_t ncCloseInp;
    int *ncid = NULL;
    ncInqInp_t ncInqInp;
    ncInqOut_t *ncInqOut = NULL;
    ncAggInfo_t *ncAggInfo = NULL;
    rodsLong_t startTimeInx, endTimeInx;

    bzero (&dataObjInp, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, ncArchTimeSeriesInp->aggCollection,
      MAX_NAME_LEN);
    replKeyVal (&ncArchTimeSeriesInp->condInput, &dataObjInp.condInput);
    
    status = getRescGrpForCreate (rsComm, &dataObjInp, &myRescGrpInfo);
    clearKeyVal (&dataObjInp.condInput);
    
    /* pick the first local host */
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
    bzero (&ncCloseInp, sizeof (ncCloseInp_t));
    ncCloseInp.ncid = *ncid;
    for (dimInx = 0; dimInx < ncInqOut->ndims; dimInx++) {
        if (strcasecmp (ncInqOut->dim[dimInx].name, "time") == 0) break;
    }
    if (dimInx >= ncInqOut->ndims) {
        /* no match */
        rodsLog (LOG_ERROR,
          "_rsNcArchTimeSeries: 'time' dim does not exist for %s",
          ncOpenInp.objPath);
        rsNcClose (rsComm, &ncCloseInp);
        return NETCDF_DIM_MISMATCH_ERR;
    }
    for (varInx = 0; varInx < ncInqOut->nvars; varInx++) {
        if (strcmp (ncInqOut->dim[dimInx].name, ncInqOut->var[varInx].name) 
          == 0) {
            break;
        }
    }
    if (varInx >= ncInqOut->nvars) {
        /* no match */
        rodsLog (LOG_ERROR,
          "_rsNcArchTimeSeries: 'time' var does not exist for %s",
          ncOpenInp.objPath);
        rsNcClose (rsComm, &ncCloseInp);
        return NETCDF_DIM_MISMATCH_ERR;
    }

    if (ncInqOut->var[varInx].nvdims != 1) {
        rodsLog (LOG_ERROR,
          "_rsNcArchTimeSeries: 'time' .nvdims = %d is not 1 for %s",
          ncInqOut->var[varInx].nvdims, ncOpenInp.objPath);
        rsNcClose (rsComm, &ncCloseInp);
        return NETCDF_DIM_MISMATCH_ERR;
    }

    if (getValByKey (&ncArchTimeSeriesInp->condInput, NEW_NETCDF_ARCH_KW) !=
      NULL) {
        /* this is a new archive */
        startTimeInx = 0;
    } else {
        status = readAggInfo (rsComm, ncArchTimeSeriesInp->aggCollection,
          NULL, &ncAggInfo);
        if (status < 0) {
            rsNcClose (rsComm, &ncCloseInp);
            return status;
        }
        endTime = ncAggInfo->ncAggElement[ncAggInfo->numFiles - 1].endTime;

        status = getTimeInxForArch (rsComm, *ncid, ncInqOut, dimInx, varInx, 
          endTime, &startTimeInx);
        if (status < 0) {
            rsNcClose (rsComm, &ncCloseInp);
            return status;
        }
    }
    endTimeInx = ncInqOut->dim[dimInx].arrayLen;

    addKeyVal (&dataObjInp.condInput, NO_OPEN_FLAG_KW, "");
    getNextAggEleObjPath (ncAggInfo, ncArchTimeSeriesInp->aggCollection, 
      dataObjInp.objPath);
    l1descInx = _rsDataObjCreateWithRescInfo (rsComm, &dataObjInp,
      tmpRescInfo, myRescGrpInfo->rescGroupName);
    if (l1descInx < 0) {
        freeAllRescGrpInfo (myRescGrpInfo);
        return l1descInx;
    }

    return status;
}

int
getTimeInxForArch (rsComm_t *rsComm, int ncid, ncInqOut_t *ncInqOut,
int dimInx, int varInx, unsigned int prevEndTime, rodsLong_t *startTimeInx)
{
    rodsLong_t start[1], count[1], stride[1];
    rodsLong_t timeArrayLen, timeArrayRemain, readCount;
    ncGetVarInp_t ncGetVarInp;
    ncGetVarOut_t *ncGetVarOut = NULL;
    void *bufPtr;
    int i, status;
    unsigned int myTime;


    /* read backward, READ_TIME_SIZE at a time until it is <= prevEndTime */
    timeArrayLen = timeArrayRemain = ncInqOut->dim[dimInx].arrayLen;
    if (timeArrayRemain <= READ_TIME_SIZE) {
        readCount = timeArrayRemain;
    } else {
        readCount = READ_TIME_SIZE;
    }
    bzero (&ncGetVarInp, sizeof (ncGetVarInp));
    ncGetVarInp.dataType = ncInqOut->var[varInx].dataType;
    ncGetVarInp.ncid = ncid;
    ncGetVarInp.varid =  ncInqOut->var[varInx].id;
    ncGetVarInp.ndim =  ncInqOut->var[varInx].nvdims;
    ncGetVarInp.start = start;
    ncGetVarInp.count = count;
    ncGetVarInp.stride = stride;

    while (timeArrayRemain > 0) {
        int goodInx = -1;
        timeArrayRemain -= readCount;
        start[0] = timeArrayRemain;
        count[0] = readCount;
        stride[0] = 1;

        status = rsNcGetVarsByType (rsComm, &ncGetVarInp, &ncGetVarOut);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "dumpNcInqOut: rcNcGetVarsByType error for %s",
              ncInqOut->var[varInx].name);
              return status;
        }
        bufPtr = ncGetVarOut->dataArray->buf;
        
        for (i = 0; i < ncGetVarOut->dataArray->len; i++) {
            myTime = ncValueToInt (ncGetVarOut->dataArray->type, &bufPtr);
            if (myTime < 0) {
                /* XXXX close and clear */
                return myTime;
            }
            if (myTime >= prevEndTime) break;
            goodInx = i;
        }
        if (goodInx >= 0) {
            *startTimeInx = timeArrayRemain + 1;
            return 0;
        }
        if (timeArrayRemain <= READ_TIME_SIZE) {
            readCount = timeArrayRemain;
        } else {
            readCount = READ_TIME_SIZE;
        }
    }
    *startTimeInx = 0;
    return NETCDF_DIM_MISMATCH_ERR;
}

