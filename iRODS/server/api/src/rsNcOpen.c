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
	l1descInx = _rsDataObjOpen (rsComm, &dataObjInp);
	clearKeyVal (&dataObjInp.condInput);
        if (l1descInx < 0) return l1descInx;
	remoteFlag = resolveHostByDataObjInfo (L1desc[l1descInx].dataObjInfo, 
	  &rodsServerHost);
	if (remoteFlag < 0) {
            return (remoteFlag);
	} else if (remoteFlag == LOCAL_HOST) {
            status = nc_open (ncOpenInp->objPath, NC_NOWRITE, &myncid);
	    if (status != NC_NOERR) {
		rodsLog (LOG_ERROR,
		  "rsNcOpen: nc_open %s error, status = %d, %s",
		  ncOpenInp->objPath, status, nc_strerror(status));
		freeL1desc (l1descInx);
		return (NETCDF_OPEN_ERR - status);
	    }
	} else {
	    status = rcNcOpen (rodsServerHost->conn, ncOpenInp, &myncid);
	    if (status < 0) {
		rodsLog (LOG_ERROR,
                  "rsNcOpen: _rcNcOpen %s error, status = %d",
                  ncOpenInp->objPath, status);
                freeL1desc (l1descInx);
                return (status);
            }
	    L1desc[l1descInx].l3descInx = myncid;
	} 
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
    L1desc[l1descInx].l3descInx = myncid;
    *ncid = (int *) malloc (sizeof (int));
    *(*ncid) = l1descInx;

    return 0;
}

