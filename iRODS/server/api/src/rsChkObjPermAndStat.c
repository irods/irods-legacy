/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsChkObjPermAndStat.c
 */

#include "chkObjPermAndStat.h"
#include "apiHeaderAll.h"
#include "icatHighLevelRoutines.h"
#include "icatHighLevelRoutines.h"
int
rsChkObjPermAndStat (rsComm_t *rsComm,
chkObjPermAndStat_t *chkObjPermAndStatInp)
{
    int status;
    rodsServerHost_t *rodsServerHost = NULL;

    status = getAndConnRcatHost (rsComm, SLAVE_RCAT, 
      chkObjPermAndStatInp->objPath, &rodsServerHost);
    if (status < 0) {
       return(status);
    }
    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
        status = _rsChkObjPermAndStat (rsComm, chkObjPermAndStatInp);
#else
        status = SYS_NO_RCAT_SERVER_ERR;
#endif
    } else {
        status = rcChkObjPermAndStat (rodsServerHost->conn, 
	  chkObjPermAndStatInp);
    }

    return (status);
}

int
_rsChkObjPermAndStat (rsComm_t *rsComm,
chkObjPermAndStat_t *chkObjPermAndStatInp)
{
#ifdef RODS_CAT
    int status;

    if ((chkObjPermAndStatInp->flags & CHK_COLL_FOR_BUNDLE_OPR) != 0) {
	status = chkCollForBundleOpr (rsComm, chkObjPermAndStatInp);
    } else {
	rodsLog (LOG_ERROR,
          "_rsChkObjPermAndStat: rsChkObjPermAndStat of %s error. flags = %d",
      chkObjPermAndStatInp->objPath, chkObjPermAndStatInp->flags);
	return SYS_OPR_FLAG_NOT_SUPPORT;
    }
    return status;
#else
    return (SYS_NO_RCAT_SERVER_ERR);
#endif
}


int
chkCollForBundleOpr (rsComm_t *rsComm,
chkObjPermAndStat_t *chkObjPermAndStatInp)
{
#ifdef RODS_CAT
    int status;
    openCollInp_t openCollInp;
    collEnt_t *collEnt = NULL;
    collEnt_t *curCollEnt = NULL;
    int handleInx;
    int curCopyGood = False;
    char *resource;
    rodsLong_t myId;
    char myPath[MAX_NAME_LEN];

    if ((resource = getValByKey (&chkObjPermAndStatInp->condInput, 
      RESC_NAME_KW)) == NULL) {
        rodsLog (LOG_ERROR,
          "chkCollForBundleOpr: RESC_NAME_KW not specified for %s",
	    chkObjPermAndStatInp->objPath);
	return SYS_INVALID_RESC_INPUT;
    }
	  
    memset (&openCollInp, 0, sizeof (openCollInp));
    rstrcpy (openCollInp.collName, chkObjPermAndStatInp->objPath, MAX_NAME_LEN);
    openCollInp.flags = 
      RECUR_QUERY_FG | LONG_METADATA_FG | NO_TRIM_REPL_FG;
    handleInx = rsOpenCollection (rsComm, &openCollInp);
    if (handleInx < 0) {
        rodsLog (LOG_ERROR,
          "chkCollForBundleOpr: rsOpenCollection of %s error. status = %d",
          openCollInp.collName, handleInx);
        return (handleInx);
    }

    while ((status = rsReadCollection (rsComm, &handleInx, &collEnt)) >= 0) {
        if (collEnt->specColl.collClass != NO_SPEC_COLL) {
	    if (strcmp (resource, collEnt->specColl.resource) != 0) {
                rodsLog (LOG_ERROR,
                  "chkCollForBundleOpr: specColl resc %s does not match %s",
                  collEnt->specColl.resource, resource);
                rsCloseCollection (rsComm, &handleInx);
                return SYS_COPY_NOT_EXIST_IN_RESC;
	    }
	    /* check permission */
            myId = checkAndGetObjectId (rsComm, "-c", 
	      collEnt->specColl.collection, ACCESS_READ_OBJECT);
            if (myId < 0) {
                status = myId;
                rodsLog (LOG_ERROR,
                 "chkCollForBundleOpr: no accPerm to specColl %s. status = %d",
                  collEnt->specColl.collection, status);
                rsCloseCollection (rsComm, &handleInx);
                return status;
	    }
	    free (collEnt);
	    collEnt = NULL;
	    continue;
	}

	if (collEnt->objType == DATA_OBJ_T) {
            if (curCollEnt == NULL) {
	        curCollEnt = collEnt;
                if (collEnt->replStatus > 0 &&
                  strcmp (resource, collEnt->resource) == 0) {
                    curCopyGood = True;
                }
	    } else {
	        if (strcmp (curCollEnt->dataName, collEnt->dataName) == 0 &&
		  strcmp (curCollEnt->collName, collEnt->collName) == 0) {
		    if (collEnt->replStatus > 0 &&
		      strcmp (resource, collEnt->resource) == 0) {
			/* a good copy */
			free (curCollEnt);
			curCopyGood = True;
			curCollEnt = collEnt;
		    }
		} else {
		    /* encounter a new data obj */
                    snprintf (myPath, MAX_NAME_LEN, "%s/%s", 
		      curCollEnt->collName,
                      curCollEnt->dataName);
                    free (curCollEnt);
                    curCollEnt = NULL;
                    if (curCopyGood == False) {
                        rodsLog (LOG_ERROR,
                         "chkCollForBundleOpr: %s does not have a good copy in %s",
                          myPath, resource);
			rsCloseCollection (rsComm, &handleInx);
                        return SYS_COPY_NOT_EXIST_IN_RESC;
                    }
                    /* we have a good copy. Check the permission */
                    myId = checkAndGetObjectId (rsComm, "-d", myPath,
                      ACCESS_READ_OBJECT);
                    if (myId < 0 && myId != CAT_UNKNOWN_FILE) {
                        /* could return CAT_UNKNOWN_FILE if mounted files */
			status = myId;
			rodsLog (LOG_ERROR,
                         "chkCollForBundleOpr: no accPerm to %s. status = %d",
                          myPath, status);
			rsCloseCollection (rsComm, &handleInx);
                        return status;
		    } else {
			/* copy is OK */
                        curCollEnt = collEnt;
			collEnt = NULL;
                        if (curCollEnt->replStatus > 0 &&
                          strcmp (resource, curCollEnt->resource) == 0) {
                            /* a good copy */
                            curCopyGood = True;
			}
		    }
		}
	    }
	}	
    }

    /* handle what's left */
    if (curCollEnt != NULL && curCopyGood == False) {
        rodsLog (LOG_ERROR,
         "chkCollForBundleOpr: a file in %s does not have a good copy in %s",
          chkObjPermAndStatInp->objPath, resource);
        status = SYS_COPY_NOT_EXIST_IN_RESC;
    } else {
	status = 0;
    }

    rsCloseCollection (rsComm, &handleInx);

    return (0);
#else
    return (SYS_NO_RCAT_SERVER_ERR);
#endif
}

