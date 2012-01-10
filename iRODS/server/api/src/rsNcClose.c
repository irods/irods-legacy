/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See dataObjGet.h for a description of this API call.*/

#include "ncClose.h"
#include "rodsLog.h"
#include "dataObjClose.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "rsApiHandler.h"
#include "objMetaOpr.h"
#include "physPath.h"
#include "specColl.h"
#include "getRemoteZoneResc.h"

int
rsNcClose (rsComm_t *rsComm, int *ncid)
{
    int remoteFlag;
    rodsServerHost_t *rodsServerHost = NULL;
    int l1descInx;
    int status = 0;

    l1descInx = *ncid;
    if (l1descInx < 2 || l1descInx >= NUM_L1_DESC) {
        rodsLog (LOG_ERROR,
          "rsNcClose: l1descInx %d out of range",
          l1descInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }
    if (L1desc[l1descInx].inuseFlag != FD_INUSE) return BAD_INPUT_DESC_INDEX;
    if (L1desc[l1descInx].remoteZoneHost != NULL) {
        /* cross zone operation */
	status = rcNcClose (L1desc[l1descInx].remoteZoneHost->conn,
	  L1desc[l1descInx].remoteL1descInx);
	/* the local resc will do the registration */
	freeL1desc (l1descInx);
    } else {
        remoteFlag = resolveHostByDataObjInfo (L1desc[l1descInx].dataObjInfo,
          &rodsServerHost);
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else if (remoteFlag == LOCAL_HOST) {
      	    openedDataObjInp_t dataObjCloseInp;
            status = nc_close (L1desc[l1descInx].l3descInx);
            if (status != NC_NOERR) {
                rodsLog (LOG_ERROR,
                  "rsNcClose: nc_close %d for %s error, status = %d, %s",
                  L1desc[l1descInx].l3descInx, 
		  L1desc[l1descInx].dataObjInfo->objPath, status, 
		  nc_strerror(status));
                freeL1desc (l1descInx);
                return (NETCDF_CLOSE_ERR - status);
            }
            bzero (&dataObjCloseInp, sizeof (dataObjCloseInp));
            dataObjCloseInp.l1descInx = l1descInx;

            status = rsDataObjClose (rsComm, &dataObjCloseInp);
        } else {
            status = rcNcClose (rodsServerHost->conn, 
	      L1desc[l1descInx].l3descInx);
	    freeL1desc (l1descInx);
	}
    }
    return status;
}

