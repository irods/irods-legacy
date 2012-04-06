/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See ncInq.h for a description of this API call.*/
#include "ncInq.h"
#include "rodsLog.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "rsApiHandler.h"
#include "objMetaOpr.h"
#include "physPath.h"
#include "specColl.h"
#include "getRemoteZoneResc.h"


int
rsNcInq (rsComm_t *rsComm, ncInqIdInp_t *ncInqInp, ncInqOut_t **ncInqOut)
{
    int remoteFlag;
    rodsServerHost_t *rodsServerHost = NULL;
    int l1descInx;
    ncInqIdInp_t mysNcInqInp;
    int status = 0;

    if (getValByKey (&ncInqInp->condInput, NATIVE_NETCDF_CALL_KW) !=
      NULL) {
        /* just do nc_inq */
        status = _rsNcInq (rsComm, ncInqInp->ncid, ncInqOut);
        return status;
    }
    l1descInx = ncInqInp->ncid;
    if (l1descInx < 2 || l1descInx >= NUM_L1_DESC) {
        rodsLog (LOG_ERROR,
          "rsNcInq: l1descInx %d out of range",
          l1descInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }
    if (L1desc[l1descInx].inuseFlag != FD_INUSE) return BAD_INPUT_DESC_INDEX;
    if (L1desc[l1descInx].remoteZoneHost != NULL) {
        bzero (&mysNcInqInp, sizeof (mysNcInqInp));
        mysNcInqInp.ncid = L1desc[l1descInx].remoteL1descInx;

        /* cross zone operation */
        status = rcNcInq (L1desc[l1descInx].remoteZoneHost->conn,
          &mysNcInqInp, ncInqOut);
    } else {
        remoteFlag = resoAndConnHostByDataObjInfo (rsComm,
          L1desc[l1descInx].dataObjInfo, &rodsServerHost);
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else if (remoteFlag == LOCAL_HOST) {
            status = _rsNcInq (rsComm, L1desc[l1descInx].l3descInx, 
	      ncInqOut);
            if (status < 0) {
                return status;
            }
        } else {
            /* execute it remotely */
            bzero (&mysNcInqInp, sizeof (mysNcInqInp));
            mysNcInqInp.ncid = L1desc[l1descInx].l3descInx;
            addKeyVal (&mysNcInqInp.condInput, NATIVE_NETCDF_CALL_KW, "");
            status = rcNcInq (rodsServerHost->conn, &mysNcInqInp,
              ncInqOut);
            clearKeyVal (&mysNcInqInp.condInput);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "rsNcInq: rcsNcInq %d for %s error, status = %d",
                  L1desc[l1descInx].l3descInx,
                  L1desc[l1descInx].dataObjInfo->objPath, status);
                return (status);
            }
        }
    }
    return status;
}

int
_rsNcInq (rsComm_t *rsComm, int ncid, ncInqOut_t **ncInqOut)
{
    int ndims, nvars, ngatts, unlimdimid, format;
    int status, i;
    ncGenDimOut_t *dim;
    ncGenVarOut_t *var;
    ncGenAttOut_t *gatt;
    size_t mylong = 0;
    int intArray[NC_MAX_VAR_DIMS];
    int myndim;

    *ncInqOut = NULL;
    status = nc_inq (ncid, &ndims, &nvars, &ngatts, &unlimdimid);
    if (status != NC_NOERR) {
        rodsLog (LOG_ERROR,
          "_rsNcInq: nc_inq error.  %s ", nc_strerror(status));
        status = NETCDF_INQ_ERR - status;
	return status;
    }

    status = nc_inq_format (ncid, &format);
    if (status != NC_NOERR) {
        rodsLog (LOG_ERROR,
          "_rsNcInq: nc_inq_format error.  %s ", nc_strerror(status));
        status = NETCDF_INQ_FORMAT_ERR - status;
        return status;
    }
    initNcInqOut (ndims, nvars, ngatts, unlimdimid, format, ncInqOut);

    /* inq dimension */
    dim = (*ncInqOut)->dim;
    for (i = 0; i < ndims; i++) {
	status = nc_inq_dim (ncid, i, dim[i].name, &mylong);
        if (status == NC_NOERR) {
	    dim[i].id = i;
	     dim[i].arrayLen = mylong;
        } else {
            rodsLog (LOG_ERROR,
              "_rsNcInq: nc_inq_dim error.  %s ", nc_strerror(status));
            status = NETCDF_INQ_DIM_ERR - status;
	    clearNcInqOut (ncInqOut);
            return status;
	}
    }

    /* inq variables */
    var = (*ncInqOut)->var;
    for (i = 0; i < nvars; i++) {
        status = nc_inq_var (ncid, i, var[i].name, &var[i].dataType, &myndim, 
          intArray, &var[i].natts);
        if (status == NC_NOERR) {
            var[i].id = i;
	    /* fill in att */
        } else {
            rodsLog (LOG_ERROR,
              "_rsNcInq: nc_inq_var error.  %s ", nc_strerror(status));
            status = NETCDF_INQ_VARS_ERR - status;
            clearNcInqOut (ncInqOut);
            return status;
        }
    }

    /* inq attributes */
    gatt = (*ncInqOut)->gatt;
    for (i = 0; i < ngatts; i++) {
    }

    return status;
}


