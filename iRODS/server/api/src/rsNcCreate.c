/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See dataObjGet.h for a description of this API call.*/

#include "ncCreate.h"
#include "rodsLog.h"
#include "dataObjCreate.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "rsApiHandler.h"
#include "objMetaOpr.h"
#include "physPath.h"
#include "specColl.h"
#include "getRemoteZoneResc.h"

int
rsNcCreate (rsComm_t *rsComm, ncOpenInp_t *ncCreateInp, int **ncid)
{
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    specCollCache_t *specCollCache = NULL;
    int status;
    dataObjInp_t dataObjInp;
    int l1descInx, myncid;

    bzero (&dataObjInp, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, ncCreateInp->objPath, MAX_NAME_LEN);
    replKeyVal (&ncCreateInp->condInput, &dataObjInp.condInput);
    resolveLinkedPath (rsComm, dataObjInp.objPath, &specCollCache,
      &dataObjInp.condInput);
    remoteFlag = getAndConnRemoteZone (rsComm, &dataObjInp, &rodsServerHost,
      REMOTE_OPEN);

    if (remoteFlag < 0) {
        return (remoteFlag);
    } else if (remoteFlag == LOCAL_HOST) {
	addKeyVal (&dataObjInp.condInput, NO_OPEN_FLAG_KW, "");
	l1descInx = _rsDataObjCreate (rsComm, &dataObjInp);
	clearKeyVal (&dataObjInp.condInput);
        if (l1descInx < 0) return l1descInx;
	remoteFlag = resolveHostByDataObjInfo (L1desc[l1descInx].dataObjInfo, 
	  &rodsServerHost);
	if (remoteFlag < 0) {
            return (remoteFlag);
	} else if (remoteFlag == LOCAL_HOST) {
            status = nc_create (ncCreateInp->objPath, ncCreateInp->mode, 
	      &myncid);
	    if (status != NC_NOERR) {
		rodsLog (LOG_ERROR,
		  "rsNcCreate: nc_open %s error, status = %d, %s",
		  ncCreateInp->objPath, status, nc_strerror(status));
		freeL1desc (l1descInx);
		return (NETCDF_CREATE_ERR - status);
	    }
	} else {
            addKeyVal (&dataObjInp.condInput, DEST_RESC_NAME_KW,
              L1desc[l1descInx].dataObjInfo->rescInfo->rescName);
	    status = rcNcCreate (rodsServerHost->conn, ncCreateInp, &myncid);
	    if (status < 0) {
		rodsLog (LOG_ERROR,
                  "rsNcCreate: _rcNcCreate %s error, status = %d",
                  ncCreateInp->objPath, status);
                freeL1desc (l1descInx);
                return (status);
            }
	    L1desc[l1descInx].l3descInx = myncid;
	} 
    } else {
	addKeyVal (&dataObjInp.condInput, CROSS_ZONE_CREATE_KW, "");
        status = rcNcCreate (rodsServerHost->conn, ncCreateInp, &myncid);
        /* rm it to avoid confusion */
        rmKeyVal (&dataObjInp.condInput, CROSS_ZONE_CREATE_KW);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "rsNcCreate: _rcNcCreate %s error, status = %d",
              ncCreateInp->objPath, status);
            return (status);
        }
        l1descInx = allocAndSetL1descForZoneOpr (myncid, &dataObjInp,
          rodsServerHost, NULL);
    }
    L1desc[l1descInx].oprType = NC_CREATE;
    L1desc[l1descInx].l3descInx = myncid;
    *ncid = (int *) malloc (sizeof (int));
    *(*ncid) = l1descInx;

    return 0;
}

