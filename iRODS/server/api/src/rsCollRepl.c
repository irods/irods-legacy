/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See dataObjRepl.h for a description of this API call.*/

#include "collRepl.h"
#include "dataObjOpr.h"
#include "rodsLog.h"
#include "objMetaOpr.h"
#include "reGlobalsExtern.h"
#include "reDefines.h"
#include "openCollection.h"
#include "readCollection.h"
#include "closeCollection.h"
#include "dataObjRepl.h"

/* rsCollRepl - The Api handler of the rcCollRepl call - Replicate
 * a data object.
 * Input -
 *    rsComm_t *rsComm
 *    collRepl_t *collRepl - The replication input
 *    transStat_t **transStat - transfer stat output
 */

int
rsCollRepl (rsComm_t *rsComm, dataObjInp_t *collReplInp,
transStat_t **transStat)
{
    int status;
    openCollInp_t openCollInp;
    collEnt_t *collEnt;
    int handleInx;
    transStat_t myTransStat;
    int savedStatus = 0;

    *transStat = NULL;

    memset (&openCollInp, 0, sizeof (openCollInp));
    rstrcpy (openCollInp.collName, collReplInp->objPath, MAX_NAME_LEN);
    openCollInp.flags = RECUR_QUERY_FG;
    handleInx = rsOpenCollection (rsComm, &openCollInp);
    if (handleInx < 0) {
        rodsLog (LOG_ERROR,
          "rsCollRepl: rsOpenCollection of %s error. status = %d",
          openCollInp.collName, handleInx);
        return (handleInx);
    }


    *transStat = malloc (sizeof (transStat_t));
    memset (*transStat, 0, sizeof (transStat_t));
    memset (&myTransStat, 0, sizeof (transStat_t));

    if (CollHandle[handleInx].rodsObjStat->specColl != NULL) {
        rodsLog (LOG_ERROR,
          "rsCollRepl: unable to replicate mounted collection %s",
          openCollInp.collName);
        rsCloseCollection (rsComm, &handleInx);
        return (0);
    }

    while ((status = rsReadCollection (rsComm, &handleInx, &collEnt)) >= 0) {
        if (collEnt->objType == DATA_OBJ_T) {
            snprintf (collReplInp->objPath, MAX_NAME_LEN, "%s/%s",
              collEnt->collName, collEnt->dataName);

            status = rsDataObjReplWithOutDataObj (rsComm, collReplInp,
	      &myTransStat, NULL);

            if (status == SYS_COPY_ALREADY_IN_RESC) {
		savedStatus = 0;
                status = 0;
            }

            if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                  "rsCollRepl: rsDataObjRepl failed for %s. status = %d",
                  collReplInp->objPath, status);
                break;
            } else {
		(*transStat)->bytesWritten += myTransStat.bytesWritten;
		(*transStat)->numThreads ++;  /* use numThreads for num files */
	    }
        }
	free (collEnt);	    /* just free collEnt but not content */
    }
    rsCloseCollection (rsComm, &handleInx);

    return (savedStatus);
}

