/* This file handles the libcf subsetting. */

#include "nccfGetVara.h"
#include "rodsLog.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "rsApiHandler.h"
#include "objMetaOpr.h"
#include "physPath.h"
#include "specColl.h"
#include "getRemoteZoneResc.h"

int
rsNccfGetVara (rsComm_t *rsComm,  nccfGetVarInp_t * nccfGetVarInp,
nccfGetVarOut_t ** nccfGetVarOut)
{
    int status;
    int nlat, nlon;
    void *data;

    status = nccf_get_vara (nccfGetVarInp->ncid, nccfGetVarInp->varid,
      nccfGetVarInp->latRange, &nlat, nccfGetVarInp->lonRange, &nlon,
      nccfGetVarInp->lvlIndex,  nccfGetVarInp->timestep, data);

    return status;
}

