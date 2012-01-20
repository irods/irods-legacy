/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See dataObjGet.h for a description of this API call.*/

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

int
_rsNcGetVarsByType (int ncid, ncGetVarInp_t *ncGetVarInp,
ncGetVarOut_t **ncGetVarOut)
{
    int status;
    size_t start[NC_MAX_DIMS], count[NC_MAX_DIMS];
    ptrdiff_t stride[NC_MAX_DIMS];
    int i;
    int len = 1;
    int hasStride = 0;

    if (ncGetVarInp == NULL || ncGetVarOut == NULL) return USER__NULL_INPUT_ERR;

    for (i = 0; i < ncGetVarInp->ndim; i++) {
	start[i] = ncGetVarInp->start[i];
	count[i] = ncGetVarInp->count[i];
	stride[i] = ncGetVarInp->stride[i];
	if (count[i] <= 0) return 0;
	/* cal dataLen */
	if (stride[i] <= 0) {
	    stride[i] = 1;
	} else if (stride[i] > 1) {
	    hasStride = 1;
	}
	len = len * ((count[i] - 1) / stride[i] + 1);
    }
    if (len <= 0) return 0;
    *ncGetVarOut = (ncGetVarOut_t *) calloc (1, sizeof (ncGetVarOut_t));
    (*ncGetVarOut)->dataLen = len;
    (*ncGetVarOut)->varid = ncGetVarInp->varid;

    switch (ncGetVarInp->dataType) {
      case NC_FLOAT:
	(*ncGetVarOut)->data = calloc (1, sizeof (float) * len);
	rstrcpy ((*ncGetVarOut)->dataType_PI, INT_PI, NAME_LEN);
	if (hasStride != 0) {
            status = nc_get_vars_float (ncid, ncGetVarInp->varid, start, count,
	      stride, (float *) (*ncGetVarOut)->data);
	} else {
            status = nc_get_vara_float (ncid, ncGetVarInp->varid, start, count,
	      (float *) (*ncGetVarOut)->data);
	}
	break;
      default:
        rodsLog (LOG_ERROR,
          "_rsNcGetVarsByType: Unknow dataType %d", ncGetVarInp->dataType);
        return (NETCDF_INVALID_DATA_TYPE);
    }

    if (status != NC_NOERR) {
	if ((*ncGetVarOut)->data != NULL) free ((*ncGetVarOut)->data);
	free (*ncGetVarOut);
	*ncGetVarOut = NULL;
        rodsLog (LOG_ERROR,
          "_rsNcGetVarsByType:  nc_get_vars err varid %d dataType %d. %s ", 
	  ncGetVarInp->varid, ncGetVarInp->dataType, nc_strerror(status));
        status = NETCDF_GET_VARS_ERR - status;
    }
    return status;
}
