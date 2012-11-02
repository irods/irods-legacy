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

int
rsNcArchTimeSeries (rsComm_t *rsComm,
ncArchTimeSeriesInp_t *ncArchTimeSeriesInp) 
{
    int status;
    int myncid;
    specCollCache_t *specCollCache = NULL;

    if (getValByKey (&ncArchTimeSeriesInp->condInput, NATIVE_NETCDF_CALL_KW) 
      != NULL) {
        /* must to be called internally */
        if (rsComm->proxyUser.authInfo.authFlag < REMOTE_PRIV_USER_AUTH) {
            return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
        }
        status = _rsNcArchTimeSeries (rsComm, ncArchTimeSeriesInp);
        return status;
    }
    return status;
}

int
_rsNcArchTimeSeries (rsComm_t *rsComm,
ncArchTimeSeriesInp_t *ncArchTimeSeriesInp) 
{
    int status;

    return status;
}
