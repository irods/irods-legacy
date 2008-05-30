/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* rsOpenCollection.c - server handling routine for rcOpenCollection
 */


#include "openCollection.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"

int
rsOpenCollection (rsComm_t *rsComm, openCollInp_t *openCollInp)
{
    int status;
    int handleInx;
    collHandle_t *collHandle;
    rodsObjStat_t *rodsObjStatOut = NULL;

    handleInx = allocCollHandle ();

    if (handleInx < 0) return (handleInx);

    collHandle = &CollHandle[handleInx];

    status = rsInitQueryHandle (&collHandle->queryHandle, rsComm);
   
    if (status < 0) return status;

    rstrcpy (collHandle->dataObjInp.objPath, openCollInp->collName, 
      MAX_NAME_LEN);
    status = rsObjStat (rsComm, &collHandle->dataObjInp, &rodsObjStatOut);


    if (status < 0) return status;

    collHandle->dataObjInp.specColl = rodsObjStatOut->specColl;

    free (rodsObjStatOut);

    collHandle->state = COLL_OPENED;
    collHandle->flags = openCollInp->flags;
    /* the collection exist. now query the data in it */
    return (handleInx);
}


