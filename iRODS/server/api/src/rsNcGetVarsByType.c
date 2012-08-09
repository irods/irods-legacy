/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See ncGetVarsByType.h for a description of this API call.*/

#include "ncGetVarsByType.h"
#include "rodsLog.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "rsApiHandler.h"
#include "objMetaOpr.h"
#include "physPath.h"
#include "specColl.h"
#include "getRemoteZoneResc.h"

int
rsNcGetVarsByType (rsComm_t *rsComm, ncGetVarInp_t *ncGetVarInp,
ncGetVarOut_t **ncGetVarOut)
{
    int remoteFlag;
    rodsServerHost_t *rodsServerHost = NULL;
    int l1descInx;
    ncGetVarInp_t myNcGetVarInp;
    int status = 0;

    if (getValByKey (&ncGetVarInp->condInput, NATIVE_NETCDF_CALL_KW) != 
      NULL) {
	/* just do nc_inq_YYYY */
	status = _rsNcGetVarsByType (ncGetVarInp->ncid, ncGetVarInp,
	  ncGetVarOut);
        return status;
    }
    l1descInx = ncGetVarInp->ncid;
    if (l1descInx < 2 || l1descInx >= NUM_L1_DESC) {
        rodsLog (LOG_ERROR,
          "rsNcGetVarsByType: l1descInx %d out of range",
          l1descInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }
    if (L1desc[l1descInx].inuseFlag != FD_INUSE) return BAD_INPUT_DESC_INDEX;
    if (L1desc[l1descInx].remoteZoneHost != NULL) {
	myNcGetVarInp = *ncGetVarInp;
	myNcGetVarInp.ncid = L1desc[l1descInx].remoteL1descInx;

        /* cross zone operation */
	status = rcNcGetVarsByType (L1desc[l1descInx].remoteZoneHost->conn,
	  &myNcGetVarInp, ncGetVarOut);
    } else {
        remoteFlag = resoAndConnHostByDataObjInfo (rsComm,
	  L1desc[l1descInx].dataObjInfo, &rodsServerHost);
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else if (remoteFlag == LOCAL_HOST) {
	    status = _rsNcGetVarsByType (L1desc[l1descInx].l3descInx,  
	      ncGetVarInp, ncGetVarOut);
            if (status < 0) {
                return status;
            }
        } else {
	    /* execute it remotely */
	    myNcGetVarInp = *ncGetVarInp;
	    myNcGetVarInp.ncid = L1desc[l1descInx].l3descInx;
	    addKeyVal (&myNcGetVarInp.condInput, NATIVE_NETCDF_CALL_KW, "");
            status = rcNcGetVarsByType (rodsServerHost->conn, &myNcGetVarInp, 
	      ncGetVarOut);
	    clearKeyVal (&myNcGetVarInp.condInput);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "rsNcGetVarsByType: rcNcGetVarsByType %d for %s error, status = %d",
                  L1desc[l1descInx].l3descInx,
                  L1desc[l1descInx].dataObjInfo->objPath, status);
                return (status);
            }
	}
    }
    return status;
}

/* _rsNcGetVarsByType has been moved to the client because clients need it */
