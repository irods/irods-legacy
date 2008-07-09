/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsStructFileBundle.c. See structFileBundle.h for a description of 
 * this API call.*/

#include "structFileBundle.h"
#include "apiHeaderAll.h"
#include "objMetaOpr.h"
#include "dataObjOpr.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"

int
rsStructFileBundle (rsComm_t *rsComm,
structFileExtAndRegInp_t *structFileBundleInp)
{
    int status;
    dataObjInp_t dataObjInp;
    dataObjCloseInp_t dataObjCloseInp;
    structFileOprInp_t structFileOprInp;
    int l1descInx;

    /* XXXXXX need to check permission of all files in collection */

    /* open the structured file */
    memset (&dataObjInp, 0, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, structFileBundleInp->objPath, 
      MAX_NAME_LEN);
 
    /* replicate the condInput. may have resource input */
    replKeyVal (&structFileBundleInp->condInput, &dataObjInp.condInput);

    dataObjInp.openFlags = O_CREAT | O_WRONLY;  
    l1descInx = _rsDataObjOpen (rsComm, &dataObjInp, DO_NOT_PHYOPEN);

    if (l1descInx < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileBundle: _rsDataObjOpen of %s error. status = %d",
          dataObjInp.objPath, l1descInx);
        return (l1descInx);
    }

    status = initStructFileOprInp (rsComm, &structFileOprInp, 
      structFileBundleInp, L1desc[l1descInx].dataObjInfo);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileBundle: initStructFileOprInp of %s error. stat = %d",
          dataObjInp.objPath, status);
        dataObjCloseInp.l1descInx = l1descInx;
        rsDataObjClose (rsComm, &dataObjCloseInp);
        return (status);
    }

    structFileOprInp.oprType = NO_REG_COLL_INFO;
    structFileOprInp.specColl->cacheDirty = 1;

    status = rsStructFileSync (rsComm, &structFileOprInp);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileBundle: rsStructFileSync of %s error. stat = %d",
          dataObjInp.objPath, status);
    } else {
	/* mark it was written so the size would be adjusted */
	L1desc[l1descInx].bytesWritten = 1;
    }

    dataObjCloseInp.l1descInx = l1descInx;
    rsDataObjClose (rsComm, &dataObjCloseInp);

    return (status);
}

