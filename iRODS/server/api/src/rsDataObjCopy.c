/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See dataObjCopy.h for a description of this API call.*/

#include "dataObjCopy.h"
#include "rodsLog.h"
#include "objMetaOpr.h"
#include "dataObjOpen.h"
#include "dataObjCreate.h"
#include "dataObjRepl.h"
#include "regDataObj.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"

int
rsDataObjCopy (rsComm_t *rsComm, dataObjCopyInp_t *dataObjCopyInp,
transStat_t **transStat)
{
    dataObjInp_t *srcDataObjInp, *destDataObjInp;
    int srcL1descInx, destL1descInx;
    int status;
    int existFlag;

    *transStat = malloc (sizeof (transStat_t));
    memset (*transStat, 0, sizeof (transStat_t));

    srcDataObjInp = &dataObjCopyInp->srcDataObjInp;
    destDataObjInp = &dataObjCopyInp->destDataObjInp;

    if (strcmp (srcDataObjInp->objPath, destDataObjInp->objPath) == 0) {
        rodsLog (LOG_ERROR,
          "rsDataObjCopy: same src and dest objPath %s not allowed",
	  srcDataObjInp->objPath);
	return (USER_INPUT_PATH_ERR);
    }
	
#if 0
    if (dataObjExist (rsComm, destDataObjInp)) {
        if (getValByKey (&destDataObjInp->condInput, FORCE_FLAG_KW) == NULL) {
	    return (OVERWITE_WITHOUT_FORCE_FLAG);
	}
	existFlag = 1;
    } else {
	existFlag = 0;
	/* have to remove FORCE_FLAG_KW because it is used later to 
	 * determine wether the file exists */
	rmKeyVal (&destDataObjInp->condInput, FORCE_FLAG_KW);
    }
#endif

    srcL1descInx = _rsDataObjOpen (rsComm, srcDataObjInp, PHYOPEN_BY_SIZE);

    if (srcL1descInx < 0)
        return srcL1descInx;

    /* have to set L1desc[srcL1descInx].dataSize because open set this to -1 */
    destDataObjInp->dataSize = L1desc[srcL1descInx].dataSize =
     L1desc[srcL1descInx].dataObjInfo->dataSize;

    L1desc[srcL1descInx].oprType = COPY_SRC;

    if (L1desc[srcL1descInx].l3descInx <= 2) {
        /* dataSingleBuf */
        addKeyVal (&destDataObjInp->condInput, NO_OPEN_FLAG_KW, "");
    }

    destL1descInx = rsDataObjCreate (rsComm, destDataObjInp);

#if 0
    if (existFlag > 0) {
        int phyOpenFlag;
	if (L1desc[srcL1descInx].l3descInx <= 2) {
            /* dataSingleBuf */
            phyOpenFlag = DO_NOT_PHYOPEN;
        } else {
            phyOpenFlag = DO_PHYOPEN;
        }
	destDataObjInp->openFlags |= (O_TRUNC | O_WRONLY);
	destL1descInx = _rsDataObjOpen (rsComm, destDataObjInp, phyOpenFlag);
    } else {
        if (L1desc[srcL1descInx].l3descInx <= 2) {
            /* dataSingleBuf */
            addKeyVal (&destDataObjInp->condInput, NO_OPEN_FLAG_KW, "");
        }
	destL1descInx = rsDataObjCreate (rsComm, destDataObjInp);
    }
#endif

    if (destL1descInx < 0) {
	return (destL1descInx);
    }

    if (L1desc[destL1descInx].replStatus == NEWLY_CREATED_COPY) {
	existFlag = 0;
    } else {
	existFlag = 1;
    }
	
    L1desc[destL1descInx].oprType = COPY_DEST;

    L1desc[destL1descInx].srcL1descInx = srcL1descInx;

    rstrcpy (L1desc[destL1descInx].dataObjInfo->dataType, 

    L1desc[srcL1descInx].dataObjInfo->dataType, NAME_LEN);
    /* set dataSize for verification in _rsDataObjClose */

    L1desc[destL1descInx].dataSize = 
      L1desc[srcL1descInx].dataObjInfo->dataSize;

    (*transStat)->bytesWritten = L1desc[srcL1descInx].dataObjInfo->dataSize;

    status = _rsDataObjCopy (rsComm, destL1descInx, existFlag);

    if (status >= 0) {
        (*transStat)->numThreads = destDataObjInp->numThreads;
    }

    return (status);
}

int
_rsDataObjCopy (rsComm_t *rsComm, int destL1descInx, int existFlag)
{
    dataObjInp_t *srcDataObjInp, *destDataObjInp;
    dataObjCloseInp_t dataObjCloseInp;
    dataObjInfo_t *srcDataObjInfo, *destDataObjInfo;
    int srcL1descInx;
    int status;

    destDataObjInp = L1desc[destL1descInx].dataObjInp;
    destDataObjInfo = L1desc[destL1descInx].dataObjInfo;
    srcL1descInx = L1desc[destL1descInx].srcL1descInx;

    srcDataObjInp = L1desc[srcL1descInx].dataObjInp;
    srcDataObjInfo = L1desc[srcL1descInx].dataObjInfo;

    if (L1desc[srcL1descInx].l3descInx <= 2) {
        /* no physical file was opened */
        status = l3DataCopySingleBuf (rsComm, destL1descInx);
	/* has not been registered yet because of NO_OPEN_FLAG_KW */
	if (status >= 0 && existFlag == 0 && 
	  destDataObjInfo->specColl == NULL) {
            status = svrRegDataObj (rsComm, destDataObjInfo);
            if (status < 0) {
                rodsLog (LOG_NOTICE,
                  "_rsDataObjCopy: svrRegDataObj for %s failed, status = %d",
                  destDataObjInfo->objPath, status);
                return (status);
	    }
	}
    } else {
        destDataObjInp->numThreads = getNumThreads (rsComm,
	 srcDataObjInfo->dataSize, destDataObjInp->numThreads, NULL);
        status = dataObjCopy (rsComm, destL1descInx);
    }

    memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));

    dataObjCloseInp.l1descInx = destL1descInx;
    if (status >= 0) {
        dataObjCloseInp.bytesWritten = srcDataObjInfo->dataSize;
    }

    rsDataObjClose (rsComm, &dataObjCloseInp);

    return (status);
}

