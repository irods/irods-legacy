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
#include "rsApiHandler.h"

/* rsCollRepl - The Api handler of the rcCollRepl call - Replicate
 * a data object.
 * Input -
 *    rsComm_t *rsComm
 *     dataObjInp_t *collReplInp - The replication input
 *    collOprStat_t **collOprStat - transfer stat output. If it is an 
 *     internal server call, collOprStat must be NULL
 */

int
rsCollRepl (rsComm_t *rsComm, dataObjInp_t *collReplInp,
collOprStat_t **collOprStat)
{
    int status;
    openCollInp_t openCollInp;
    collEnt_t *collEnt;
    int handleInx;
    transStat_t myTransStat;
    int totalFileCnt;
    int savedStatus = 0;
    int fileCntPerStatOut = FILE_CNT_PER_STAT_OUT;

    if (collOprStat != NULL) *collOprStat = NULL;

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

    if (collOprStat != NULL) {
        *collOprStat = malloc (sizeof (collOprStat_t));
        memset (*collOprStat, 0, sizeof (collOprStat_t));
    }

    if (CollHandle[handleInx].rodsObjStat->specColl != NULL) {
        rodsLog (LOG_ERROR,
          "rsCollRepl: unable to replicate mounted collection %s",
          openCollInp.collName);
        rsCloseCollection (rsComm, &handleInx);
        return (0);
    }

    while ((status = rsReadCollection (rsComm, &handleInx, &collEnt)) >= 0) {
        if (collEnt->objType == DATA_OBJ_T) {
	    if (totalFileCnt == 0) totalFileCnt = 
		CollHandle[handleInx].dataObjSqlResult.totalRowCount;

            snprintf (collReplInp->objPath, MAX_NAME_LEN, "%s/%s",
              collEnt->collName, collEnt->dataName);

    	    memset (&myTransStat, 0, sizeof (myTransStat));
            status = rsDataObjReplWithOutDataObj (rsComm, collReplInp,
	      &myTransStat, NULL);

            if (status == SYS_COPY_ALREADY_IN_RESC) {
		savedStatus = status;
                status = 0;
            }

            if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                  "rsCollRepl: rsDataObjRepl failed for %s. status = %d",
                  collReplInp->objPath, status);
		savedStatus = status;
                break;
            } else {
		if (collOprStat != NULL) {
		    (*collOprStat)->bytesWritten += myTransStat.bytesWritten;
		    (*collOprStat)->filesCnt ++; 
		}
	    }
	    if (collOprStat != NULL &&
	      (*collOprStat)->filesCnt >= fileCntPerStatOut) {
	        rstrcpy ((*collOprStat)->lastObjPath, collReplInp->objPath,
	          MAX_NAME_LEN);
	        (*collOprStat)->totalFileCnt = totalFileCnt;
	        status = svrSendCollOprStat (rsComm, *collOprStat);
	        if (status < 0) {
                    rodsLogError (LOG_ERROR, status,
                      "rsCollRepl: svrSendCollOprStat failed for %s. status = %d",
                      collReplInp->objPath, status);
		    *collOprStat = NULL;
	            savedStatus = status;
	            break;
	        }
                 *collOprStat = malloc (sizeof (collOprStat_t));
                 memset (*collOprStat, 0, sizeof (collOprStat_t));
	    }
        }
	free (collEnt);	    /* just free collEnt but not content */
    }
    rsCloseCollection (rsComm, &handleInx);

    return (savedStatus);
}

