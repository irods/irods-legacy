/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See ncGetAggInfo.h for a description of this API call.*/

#include "ncGetAggInfo.h"
#include "ncInq.h"
#include "rodsLog.h"
#include "dataObjOpen.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "rsApiHandler.h"
#include "objMetaOpr.h"
#include "physPath.h"
#include "specColl.h"
#include "openCollection.h"
#include "readCollection.h"
#include "closeCollection.h"
#include "closeCollection.h"

int
rsNcGetAggInfo (rsComm_t *rsComm, ncOpenInp_t *ncOpenInp,
ncAggInfo_t **ncAggInfo)
{
    specCollCache_t *specCollCache = NULL;
    collInp_t collInp;
    int handleInx;
    collEnt_t *collEnt;
    int status = 0;
    int status2 = 0;
    ncOpenInp_t childNcOpenInp;
    ncAggElement_t *ncAggElement = NULL;

    bzero (&collInp, sizeof (collInp));
    rstrcpy (collInp.collName, ncOpenInp->objPath, MAX_NAME_LEN);
    resolveLinkedPath (rsComm, collInp.collName, &specCollCache,
      &ncOpenInp->condInput);
    collInp.flags = VERY_LONG_METADATA_FG; 
    handleInx = rsOpenCollection (rsComm, &collInp);
    if (handleInx < 0) {
        rodsLog (LOG_ERROR,
          "rsNcGetAggInfo: rsOpenCollection of %s error. status = %d",
          collInp.collName, handleInx);
        return (handleInx);
    }
    bzero (&childNcOpenInp, sizeof (childNcOpenInp));
    *ncAggInfo = (ncAggInfo_t *) calloc (1, sizeof (ncAggInfo_t));
    rstrcpy ((*ncAggInfo)->ncObjectName, ncOpenInp->objPath, MAX_NAME_LEN);
    while ((status2 = rsReadCollection (rsComm, &handleInx, &collEnt)) >= 0) {
        if (collEnt->objType != DATA_OBJ_T ||
          strcmp (collEnt->dataType, "netcdf") != 0) {
            free (collEnt);
            continue;
        } 
        snprintf (childNcOpenInp.objPath, MAX_NAME_LEN, "%s/%s",
          collInp.collName, collEnt->dataName);
        status = rsNcGetAggElement (rsComm, &childNcOpenInp, &ncAggElement);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "rsNcGetAggInfo: rsNcGetAggElement of %s error.",
              childNcOpenInp.objPath);
            free (collEnt);
            break;
        } else {
            status = addNcAggElement (ncAggElement, *ncAggInfo);
            free (ncAggElement);
            if (status < 0) {
                free (collEnt);
                break;
            }
        }
        free (collEnt);
    }
    rsCloseCollection (rsComm, &handleInx);
    if (status2 < 0 && status2 != CAT_NO_ROWS_FOUND && status >= 0) {
        return status2;
    } else {
        return status;
    }
}

