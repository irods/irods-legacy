/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See dataObjGet.h for a description of this API call.*/

#include "ncOpen.h"
#include "rodsLog.h"
#include "dataObjOpen.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "rsApiHandler.h"
#include "objMetaOpr.h"
#include "physPath.h"
#include "specColl.h"
#include "getRemoteZoneResc.h"

int
rsNcOpen (rsComm_t *rsComm, ncOpenInp_t *ncOpenInp, int **ncid)
{
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    specCollCache_t *specCollCache = NULL;
    int status;
    dataObjInp_t dataObjInp;
    int l1descInx, myncid;

    if (getValByKey (&ncOpenInp->condInput, NATIVE_NETCDF_CALL_KW) != NULL) {
	/* just all nc_open with objPath as file nc file path */
	/* must to be called internally */
        if (rsComm->proxyUser.authInfo.authFlag < REMOTE_PRIV_USER_AUTH) {
            return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
	}
        status = nc_open (ncOpenInp->objPath, ncOpenInp->mode, &myncid);
        if (status == NC_NOERR) {
            *ncid = (int *) malloc (sizeof (int));
            *(*ncid) = myncid;
	    return 0;
	} else {
            rodsLog (LOG_ERROR,
              "rsNcOpen: nc_open %s error, status = %d, %s",
              ncOpenInp->objPath, status, nc_strerror(status));
            return (NETCDF_OPEN_ERR + status);
        } 
    }
    bzero (&dataObjInp, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, ncOpenInp->objPath, MAX_NAME_LEN);
    replKeyVal (&ncOpenInp->condInput, &dataObjInp.condInput);
    resolveLinkedPath (rsComm, dataObjInp.objPath, &specCollCache,
      &dataObjInp.condInput);
    remoteFlag = getAndConnRemoteZone (rsComm, &dataObjInp, &rodsServerHost,
      REMOTE_OPEN);

    if (remoteFlag < 0) {
        return (remoteFlag);
    } else if (remoteFlag == LOCAL_HOST) {
	addKeyVal (&dataObjInp.condInput, NO_OPEN_FLAG_KW, "");
        if (getValByKey (&ncOpenInp->condInput, NO_STAGING_KW) != NULL)
	    addKeyVal (&dataObjInp.condInput, NO_STAGING_KW, "");

	l1descInx = _rsDataObjOpen (rsComm, &dataObjInp);
	clearKeyVal (&dataObjInp.condInput);
        if (l1descInx < 0) return l1descInx;
	remoteFlag = resoAndConnHostByDataObjInfo (rsComm,
	  L1desc[l1descInx].dataObjInfo, &rodsServerHost);
	if (remoteFlag < 0) {
            return (remoteFlag);
	} else if (remoteFlag == LOCAL_HOST) {
#if 0
char myPath[MAX_NAME_LEN];
snprintf (myPath, MAX_NAME_LEN, "%s?stationID,stationName,longitude,latitude,time,dcp,sensor,AT,X,N,R&time>=2012-07-23", L1desc[l1descInx].dataObjInfo->filePath);
snprintf (myPath, MAX_NAME_LEN, "%s?longitude,latitude,station_id,altitude,time,sensor_id,air_temperature&time>=2012-07-19", L1desc[l1descInx].dataObjInfo->filePath);
status = nc_open (myPath, ncOpenInp->mode, &myncid);
#else
            status = nc_open (L1desc[l1descInx].dataObjInfo->filePath, 
	      ncOpenInp->mode, &myncid);
#endif
	    if (status != NC_NOERR) {
		rodsLog (LOG_ERROR,
		  "rsNcOpen: nc_open %s error, status = %d, %s",
		  ncOpenInp->objPath, status, nc_strerror(status));
		freeL1desc (l1descInx);
		return (NETCDF_OPEN_ERR + status);
	    }
	} else {
	    /* execute it remotely with dataObjInfo->filePath */
	    ncOpenInp_t myNcOpenInp;
	    bzero (&myNcOpenInp, sizeof (myNcOpenInp));
	    rstrcpy (myNcOpenInp.objPath, 
	      L1desc[l1descInx].dataObjInfo->filePath, MAX_NAME_LEN);
	    addKeyVal (&myNcOpenInp.condInput, NATIVE_NETCDF_CALL_KW, "");
	    status = rcNcOpen (rodsServerHost->conn, &myNcOpenInp, &myncid);
	    clearKeyVal (&myNcOpenInp.condInput);
	    if (status < 0) {
		rodsLog (LOG_ERROR,
                  "rsNcOpen: rcNcOpen %s error, status = %d",
                  myNcOpenInp.objPath, status);
                freeL1desc (l1descInx);
                return (status);
            }
	} 
	L1desc[l1descInx].l3descInx = myncid;
    } else {
        status = rcNcOpen (rodsServerHost->conn, ncOpenInp, &myncid);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "rsNcOpen: _rcNcOpen %s error, status = %d",
              ncOpenInp->objPath, status);
            return (status);
        }
        l1descInx = allocAndSetL1descForZoneOpr (myncid, &dataObjInp,
          rodsServerHost, NULL);
    }
    if (ncOpenInp->mode == NC_NOWRITE) {
        L1desc[l1descInx].oprType = NC_OPEN_FOR_READ;
    } else {
        L1desc[l1descInx].oprType = NC_OPEN_FOR_WRITE;
    }
    *ncid = (int *) malloc (sizeof (int));
    *(*ncid) = l1descInx;

    return 0;
}

