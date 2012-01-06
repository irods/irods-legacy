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

int
rsNcOpen (rsComm_t *rsComm, ncOpenInp_t *ncOpenInp, int **ncid)
{
    int status;
    int myncid;

    status = nc_open (ncOpenInp->objPath, NC_NOWRITE, &myncid);

    return status;
}
