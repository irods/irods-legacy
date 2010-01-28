/*** Copyright (c), The Unregents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsObjStat.c
 */

#include "objStat.h"
#include "rcMisc.h"
#include "genQuery.h"
#include "querySpecColl.h"
#include "objMetaOpr.h"
#include "rcGlobalExtern.h"
#include "rsGlobalExtern.h"
#include "dataObjClose.h"

int
rsObjStat (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
rodsObjStat_t **rodsObjStatOut)
{
    int status;

    status = __rsObjStat (rsComm, dataObjInp, 0, rodsObjStatOut);

    return (status);
}

/* __rsObjStat - internal version of __rsObjStat. Given the object path given
 * in dataObjInp->objPath, stat the path and put the output in rodsObjStatOut.
 * intenFlag specifies whether it is called internally instead of from the
 * client API. If it is called internally, (*rodsObjStatOut)->specColl
 * is from the Globsl cache and should not be freed.
 */

int
__rsObjStat (rsComm_t *rsComm, dataObjInp_t *dataObjInp, int intenFlag,
rodsObjStat_t **rodsObjStatOut)
{
    int status;
    rodsServerHost_t *rodsServerHost = NULL;

    *rodsObjStatOut = NULL;
    status = getAndConnRcatHost (rsComm, SLAVE_RCAT, dataObjInp->objPath,
      &rodsServerHost);
    if (status < 0) {
       return(status);
    }
    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
        status = _rsObjStat (rsComm, dataObjInp, rodsObjStatOut);
#else
        status = SYS_NO_RCAT_SERVER_ERR;
#endif
    } else {
	/* see if it is a sub path of a specColl cached locally. If it is,
	 * it will save time resolving it */
        status = statPathInSpecColl (rsComm, dataObjInp->objPath, 1,
          rodsObjStatOut);

	if (status < 0) {
	    if (status == SYS_SPEC_COLL_NOT_IN_CACHE) {
	        /* not in cache, need to do a remote call */
                status = rcObjStat (rodsServerHost->conn, dataObjInp, 
	          rodsObjStatOut);
		if (status >= 0 && (*rodsObjStatOut)->specColl != NULL) {
		    /* queue it in cache */
		    queueSpecCollCacheWithObjStat (*rodsObjStatOut);
		    if (intenFlag > 0) {
			specCollCache_t *specCollCache; 
			/* Internal call, use the global cache copy instead */
			specCollCache = matchSpecCollCache (
			  (*rodsObjStatOut)->specColl->collection);
			free ((*rodsObjStatOut)->specColl);
			(*rodsObjStatOut)->specColl = 
			  &specCollCache->specColl;
		    }
		}
	    }
	    return (status);
	}
    }

    if (intenFlag == 0 && status >= 0 && 
      (*rodsObjStatOut)->specColl != NULL) {
        /* replace specColl since the one given in rodsObjStatOut
         * is a cached one */
        specColl_t *specColl = malloc (sizeof (specColl_t));
        *specColl = *(*rodsObjStatOut)->specColl;
        (*rodsObjStatOut)->specColl = specColl;
    }

    return (status);
}

int
_rsObjStat (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
rodsObjStat_t **rodsObjStatOut)
{
    int status;
    char *tmpStr;
    specCollCache_t *specCollCache;

    /* do data first to catch data registered in spec Coll */ 
    if ((tmpStr = getValByKey (&dataObjInp->condInput, SEL_OBJ_TYPE_KW)) ==
      NULL || strcmp (tmpStr, "dataObj") == 0) {
        status = dataObjStat (rsComm, dataObjInp, rodsObjStatOut);
        if (status >= 0) return (status);
    }

    if (tmpStr == NULL || strcmp (tmpStr, "collection") == 0) {
        status = collStat (rsComm, dataObjInp, rodsObjStatOut);
	/* specColl may already been obtained from collStat */
        if (status >= 0 && (*rodsObjStatOut)->specColl == NULL) {
	    if (getSpecCollCache (rsComm, dataObjInp->objPath, 0,
              &specCollCache) >= 0) {
                (*rodsObjStatOut)->specColl = &specCollCache->specColl;
	    }
	    return (status);
	}
    }

    /* now check specColl */
    /* XXXX need to check a rule if it supports spec collection */
    status = statPathInSpecColl (rsComm, dataObjInp->objPath, 0,
      rodsObjStatOut);
    if (status < 0) status = USER_FILE_DOES_NOT_EXIST;
    return (status);
}

int
collStat (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
rodsObjStat_t **rodsObjStatOut)
{
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    int status;
    char condStr[MAX_NAME_LEN];
    sqlResult_t *dataId;
    sqlResult_t *ownerName;
    sqlResult_t *ownerZone;
    sqlResult_t *createTime;
    sqlResult_t *modifyTime;
    sqlResult_t *collType;
    sqlResult_t *collInfo1;
    sqlResult_t *collInfo2;

    /* see if objPath is a collection */
    memset (&genQueryInp, 0, sizeof (genQueryInp));

    snprintf (condStr, MAX_NAME_LEN, "='%s'", dataObjInp->objPath);
    addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, condStr);

    addInxIval (&genQueryInp.selectInp, COL_COLL_ID, 1);
    /* XXXX COL_COLL_NAME added for queueSpecColl */
    addInxIval (&genQueryInp.selectInp, COL_COLL_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_OWNER_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_OWNER_ZONE, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_CREATE_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_MODIFY_TIME, 1);
    /* XXXX may want to do this if spec coll is supported */
    addInxIval (&genQueryInp.selectInp, COL_COLL_TYPE, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_INFO1, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_INFO2, 1);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

    if (status >= 0) {
	*rodsObjStatOut = (rodsObjStat_t *) malloc (sizeof (rodsObjStat_t));
	memset (*rodsObjStatOut, 0, sizeof (rodsObjStat_t));
	(*rodsObjStatOut)->objType = status = COLL_OBJ_T;
        if ((dataId = getSqlResultByInx (genQueryOut,
          COL_COLL_ID)) == NULL) {
            rodsLog (LOG_ERROR,
              "_rsObjStat: getSqlResultByInx for COL_COLL_ID failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((ownerName = getSqlResultByInx (genQueryOut,
          COL_COLL_OWNER_NAME)) == NULL) {
            rodsLog (LOG_ERROR,
             "_rsObjStat:getSqlResultByInx for COL_COLL_OWNER_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((ownerZone = getSqlResultByInx (genQueryOut,
          COL_COLL_OWNER_ZONE)) == NULL) {
            rodsLog (LOG_ERROR,
             "_rsObjStat:getSqlResultByInx for COL_COLL_OWNER_ZONE failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((createTime = getSqlResultByInx (genQueryOut,
          COL_COLL_CREATE_TIME)) == NULL) {
            rodsLog (LOG_ERROR,
             "_rsObjStat:getSqlResultByInx for COL_COLL_CREATE_TIME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((modifyTime = getSqlResultByInx (genQueryOut,
          COL_COLL_MODIFY_TIME)) == NULL) {
            rodsLog (LOG_ERROR,
             "_rsObjStat:getSqlResultByInx for COL_COLL_MODIFY_TIME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((collType = getSqlResultByInx (genQueryOut,
          COL_COLL_TYPE)) == NULL) {
            rodsLog (LOG_ERROR,
             "_rsObjStat:getSqlResultByInx for COL_COLL_TYPE failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((collInfo1 = getSqlResultByInx (genQueryOut,
          COL_COLL_INFO1)) == NULL) {
            rodsLog (LOG_ERROR,
             "_rsObjStat:getSqlResultByInx for COL_COLL_INFO1 failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((collInfo2 = getSqlResultByInx (genQueryOut,
          COL_COLL_INFO2)) == NULL) {
            rodsLog (LOG_ERROR,
             "_rsObjStat:getSqlResultByInx for COL_COLL_INFO2 failed");
            return (UNMATCHED_KEY_OR_INDEX);
	} else {
            rstrcpy ((*rodsObjStatOut)->dataId, dataId->value, NAME_LEN);
            rstrcpy ((*rodsObjStatOut)->ownerName, ownerName->value,
              NAME_LEN);
            rstrcpy ((*rodsObjStatOut)->ownerZone, ownerZone->value,
              NAME_LEN);
            rstrcpy ((*rodsObjStatOut)->createTime, createTime->value,
              NAME_LEN);
            rstrcpy ((*rodsObjStatOut)->modifyTime, modifyTime->value,
              NAME_LEN);

	    if (strlen (collType->value) > 0) {
		specCollCache_t *specCollCache;

		if ((specCollCache = 
		  matchSpecCollCache (dataObjInp->objPath)) != NULL) {
		    (*rodsObjStatOut)->specColl = &specCollCache->specColl;
		} else {
    		    status = queueSpecCollCache (genQueryOut, 
		      dataObjInp->objPath);
    		    if (status < 0) return (status);
    		    (*rodsObjStatOut)->specColl = &SpecCollCacheHead->specColl;
		}
	    }
	}
    } 
    clearGenQueryInp (&genQueryInp);
    freeGenQueryOut (&genQueryOut);

    return (status);
}

int
dataObjStat (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
rodsObjStat_t **rodsObjStatOut)
{
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    int status;
    char myColl[MAX_NAME_LEN], myData[MAX_NAME_LEN];
    char condStr[MAX_NAME_LEN];
    sqlResult_t *dataSize;
    sqlResult_t *dataMode;
    sqlResult_t *replStatus;
    sqlResult_t *dataId;
    sqlResult_t *chksum;
    sqlResult_t *ownerName;
    sqlResult_t *ownerZone;
    sqlResult_t *createTime;
    sqlResult_t *modifyTime;

    /* see if objPath is a dataObj */

    memset (myColl, 0, MAX_NAME_LEN);
    memset (myData, 0, MAX_NAME_LEN);

    if ((status = splitPathByKey (
      dataObjInp->objPath, myColl, myData, '/')) < 0) {
        return (USER_FILE_DOES_NOT_EXIST);
    }

    memset (&genQueryInp, 0, sizeof (genQueryInp));

    snprintf (condStr, MAX_NAME_LEN, "='%s'", myColl);
    addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, condStr);
    snprintf (condStr, MAX_NAME_LEN, "='%s'", myData);
    addInxVal (&genQueryInp.sqlCondInp, COL_DATA_NAME, condStr);

    addInxIval (&genQueryInp.selectInp, COL_D_DATA_ID, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_SIZE, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_MODE, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_REPL_STATUS, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_DATA_CHECKSUM, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_OWNER_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_OWNER_ZONE, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_CREATE_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_MODIFY_TIME, 1);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

    clearGenQueryInp (&genQueryInp);

    if (status >= 0) {
        if ((dataSize = getSqlResultByInx (genQueryOut, COL_DATA_SIZE))
          == NULL) {
            rodsLog (LOG_ERROR,
              "_rsObjStat: getSqlResultByInx for COL_DATA_SIZE failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((dataMode = getSqlResultByInx (genQueryOut,
          COL_DATA_MODE)) == NULL) {
            rodsLog (LOG_ERROR,
              "_rsObjStat: getSqlResultByInx for COL_DATA_MODE failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((replStatus = getSqlResultByInx (genQueryOut,
          COL_D_REPL_STATUS)) == NULL) {
            rodsLog (LOG_ERROR,
              "_rsObjStat: getSqlResultByInx for COL_D_REPL_STATUS failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((dataId = getSqlResultByInx (genQueryOut,
          COL_D_DATA_ID)) == NULL) {
            rodsLog (LOG_ERROR,
              "_rsObjStat: getSqlResultByInx for COL_D_DATA_ID failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((chksum = getSqlResultByInx (genQueryOut,
          COL_D_DATA_CHECKSUM)) == NULL) {
            rodsLog (LOG_ERROR,
             "_rsObjStat:getSqlResultByInx for COL_D_DATA_CHECKSUM failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((ownerName = getSqlResultByInx (genQueryOut,
          COL_D_OWNER_NAME)) == NULL) {
            rodsLog (LOG_ERROR,
             "_rsObjStat:getSqlResultByInx for COL_D_OWNER_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((ownerZone = getSqlResultByInx (genQueryOut,
          COL_D_OWNER_ZONE)) == NULL) {
            rodsLog (LOG_ERROR,
             "_rsObjStat:getSqlResultByInx for COL_D_OWNER_ZONE failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((createTime = getSqlResultByInx (genQueryOut,
          COL_D_CREATE_TIME)) == NULL) {
            rodsLog (LOG_ERROR,
             "_rsObjStat:getSqlResultByInx for COL_D_CREATE_TIME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else if ((modifyTime = getSqlResultByInx (genQueryOut,
          COL_D_MODIFY_TIME)) == NULL) {
            rodsLog (LOG_ERROR,
             "_rsObjStat:getSqlResultByInx for COL_D_MODIFY_TIME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else {
            int i;

            *rodsObjStatOut = (rodsObjStat_t *) malloc (sizeof (rodsObjStat_t));
            memset (*rodsObjStatOut, 0, sizeof (rodsObjStat_t));
            (*rodsObjStatOut)->objType = status = DATA_OBJ_T;
            /* XXXXXX . dont have numCopies anymore. Replaced by dataMode 
	     * (*rodsObjStatOut)->numCopies = genQueryOut->rowCnt; */

            for (i = 0;i < genQueryOut->rowCnt; i++) {
                if (atoi (&replStatus->value[replStatus->len * i]) > 0) {
                    rstrcpy ((*rodsObjStatOut)->dataId, 
		      &dataId->value[dataId->len * i], NAME_LEN);
                    (*rodsObjStatOut)->objSize =
                      strtoll (&dataSize->value[dataSize->len * i], 0, 0);
                    (*rodsObjStatOut)->dataMode =
                      atoi (&dataMode->value[dataMode->len * i]);
                    rstrcpy ((*rodsObjStatOut)->chksum,
                      &chksum->value[chksum->len * i], NAME_LEN);
		    rstrcpy ((*rodsObjStatOut)->ownerName,
                      &ownerName->value[ownerName->len * i], NAME_LEN);
                    rstrcpy ((*rodsObjStatOut)->ownerZone,
                      &ownerZone->value[ownerZone->len * i], NAME_LEN);
                    rstrcpy ((*rodsObjStatOut)->createTime,
                      &createTime->value[createTime->len * i], TIME_LEN);
                    rstrcpy ((*rodsObjStatOut)->modifyTime,
                      &modifyTime->value[modifyTime->len * i], TIME_LEN);
                    break;
                }
            }

            if (strlen ((*rodsObjStatOut)->dataId) == 0) {
                /* just use the first one */
                rstrcpy ((*rodsObjStatOut)->dataId, dataId->value, NAME_LEN);
                (*rodsObjStatOut)->objSize = strtoll (dataSize->value, 0, 0);
                rstrcpy ((*rodsObjStatOut)->chksum, chksum->value, NAME_LEN);
                rstrcpy ((*rodsObjStatOut)->ownerName, ownerName->value, 
		  NAME_LEN);
                rstrcpy ((*rodsObjStatOut)->ownerZone, ownerZone->value, 
		  NAME_LEN);
                rstrcpy ((*rodsObjStatOut)->createTime, createTime->value, 
		  NAME_LEN);
                rstrcpy ((*rodsObjStatOut)->modifyTime, modifyTime->value, 
		  NAME_LEN);
            }
        }
        freeGenQueryOut (&genQueryOut);
    }

    return (status);
}

int
getSpecCollCache (rsComm_t *rsComm, char *objPath, 
int inCachOnly, specCollCache_t **specCollCache)
{
    int status;
    genQueryOut_t *genQueryOut = NULL;

    if ((*specCollCache = matchSpecCollCache (objPath)) != NULL) {
	return (0);
    } else if (inCachOnly > 0) {
	return (SYS_SPEC_COLL_NOT_IN_CACHE); 
    }

    status = querySpecColl (rsComm, objPath, &genQueryOut);
    if (status < 0) return (status);

    status = queueSpecCollCache (genQueryOut, objPath);
    freeGenQueryOut (&genQueryOut);

    if (status < 0) return (status);
    *specCollCache = SpecCollCacheHead;  /* queued at top */

    return (0);
}

/* statPathInSpecColl - stat the path given in objPath assuming it is
 * in the path of a special collection. The inCachOnly flag asks it to
 * check the specColl in the global cache only. The output of the 
 * stat is given in rodsObjStatOut.
 *
 */

int
statPathInSpecColl (rsComm_t *rsComm, char *objPath, 
int inCachOnly, rodsObjStat_t **rodsObjStatOut)
{
    int status;
    dataObjInfo_t *dataObjInfo = NULL;
    specColl_t *specColl;
    specCollCache_t *specCollCache;
    
    if ((status = getSpecCollCache (rsComm, objPath, inCachOnly,
      &specCollCache)) < 0) {
	if (status != SYS_SPEC_COLL_NOT_IN_CACHE && 
	  status != CAT_NO_ROWS_FOUND){
            rodsLog (LOG_ERROR,
              "statPathInSpecColl: getSpecCollCache for %s, status = %d",
              objPath, status);
	}
        return (status);
    }
 
    if (*rodsObjStatOut == NULL)
        *rodsObjStatOut = (rodsObjStat_t *) malloc (sizeof (rodsObjStat_t));
    memset (*rodsObjStatOut, 0, sizeof (rodsObjStat_t));
    specColl = (*rodsObjStatOut)->specColl = &specCollCache->specColl;
    rstrcpy ((*rodsObjStatOut)->dataId, specCollCache->collId, NAME_LEN);
    rstrcpy ((*rodsObjStatOut)->ownerName, specCollCache->ownerName, NAME_LEN);
    rstrcpy ((*rodsObjStatOut)->ownerZone, specCollCache->ownerZone, NAME_LEN);

    status = specCollSubStat (rsComm, specColl, objPath, UNKNOW_COLL_PERM,
      &dataObjInfo);

    if (status < 0) {
	(*rodsObjStatOut)->objType = UNKNOWN_OBJ_T;
        rstrcpy ((*rodsObjStatOut)->createTime, specCollCache->createTime, 
	  NAME_LEN);
        rstrcpy ((*rodsObjStatOut)->modifyTime, specCollCache->modifyTime, 
	  NAME_LEN);
        if (specColl->collClass == LINKED_COLL && dataObjInfo != NULL) {
            rstrcpy ((*rodsObjStatOut)->specColl->objPath,
              dataObjInfo->objPath, MAX_NAME_LEN);
	} else {
	    (*rodsObjStatOut)->specColl->objPath[0] = '\0';
	}
	freeAllDataObjInfo (dataObjInfo);
	/* XXXXX 0 return is creating a problem for fuse */
	return (0);
    } else {
	if (specColl->collClass == LINKED_COLL) {
            rstrcpy ((*rodsObjStatOut)->ownerName, dataObjInfo->dataOwnerName, 
	      NAME_LEN);
            rstrcpy ((*rodsObjStatOut)->ownerZone, dataObjInfo->dataOwnerZone, 
	      NAME_LEN);
            snprintf ((*rodsObjStatOut)->dataId, NAME_LEN, "%lld", 
	      dataObjInfo->dataId);
	    (*rodsObjStatOut)->specColl = dataObjInfo->specColl;
	    /* save the linked path here */
            rstrcpy ((*rodsObjStatOut)->specColl->objPath, 
	      dataObjInfo->objPath, MAX_NAME_LEN);
	}
	(*rodsObjStatOut)->objType = status;
	(*rodsObjStatOut)->objSize = dataObjInfo->dataSize;
        rstrcpy ((*rodsObjStatOut)->createTime, dataObjInfo->dataCreate, 
	  NAME_LEN);
        rstrcpy ((*rodsObjStatOut)->modifyTime, dataObjInfo->dataModify,
	  NAME_LEN);
	freeAllDataObjInfo (dataObjInfo);
    }

    return (status);
}

/* querySpecColl - The query can produce multiple answer and only one
 * is correct. e.g., objPath = /x/yabc can produce answers:
 * /x/y, /x/ya, /x/yabc, etc. The calling subroutine need to screen
 * /x/y, /x/ya out 
 * check queueSpecCollCache () for screening.
 */ 
int 
querySpecColl (rsComm_t *rsComm, char *objPath, genQueryOut_t **genQueryOut)
{
    genQueryInp_t genQueryInp;
    int status;
    char condStr[MAX_NAME_LEN];

    /* see if objPath is in the path of a spec collection */
    memset (&genQueryInp, 0, sizeof (genQueryInp));

    snprintf (condStr, MAX_NAME_LEN, "parent_of '%s'", objPath);
    addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, condStr);
    rstrcpy (condStr, "like '_%'", MAX_NAME_LEN);
    addInxVal (&genQueryInp.sqlCondInp, COL_COLL_TYPE, condStr);

    addInxIval (&genQueryInp.selectInp, COL_COLL_ID, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_OWNER_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_OWNER_ZONE, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_CREATE_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_MODIFY_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_TYPE, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_INFO1, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_INFO2, 1);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, &genQueryInp, genQueryOut);

    clearGenQueryInp (&genQueryInp);

    if (status < 0) {
        return (status);
    }

    return (0);
}

/* specCollSubStat - Given specColl and the object path (subPath),
 * returns a dataObjInfo struct with dataObjInfo->specColl != NULL.
 * Returns COLL_OBJ_T if the path is a collection or DATA_OBJ_T if the
 * path is a data oobject.
 */

int
specCollSubStat (rsComm_t *rsComm, specColl_t *specColl, 
char *subPath, specCollPerm_t specCollPerm, dataObjInfo_t **dataObjInfo)
{
    int status;
    int objType;
    rodsStat_t *rodsStat = NULL;
    dataObjInfo_t *myDataObjInfo = NULL;;

    if (dataObjInfo == NULL) return USER__NULL_INPUT_ERR;
    *dataObjInfo = NULL;
    if (specColl->collClass == MOUNTED_COLL) {

	/* a mount point */
        myDataObjInfo = *dataObjInfo = 
	  (dataObjInfo_t *) malloc (sizeof (dataObjInfo_t));
        memset (myDataObjInfo, 0, sizeof (dataObjInfo_t));
        myDataObjInfo->specColl = specColl;

        status = resolveResc (specColl->resource, &myDataObjInfo->rescInfo);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "specCollSubStat: resolveResc error for %s, status = %d",
              specColl->resource, status);
	    freeDataObjInfo (myDataObjInfo);
	    *dataObjInfo = NULL;
            return (status);
        }

	rstrcpy (myDataObjInfo->objPath, subPath, MAX_NAME_LEN);
	rstrcpy (myDataObjInfo->subPath, subPath, MAX_NAME_LEN);
        rstrcpy (myDataObjInfo->rescName, specColl->resource, NAME_LEN);
        rstrcpy (myDataObjInfo->dataType, "generic", NAME_LEN);

	status = getMountedSubPhyPath (specColl->collection,
	  specColl->phyPath, subPath, myDataObjInfo->filePath);
	if (status < 0) {
            freeDataObjInfo (myDataObjInfo);
            *dataObjInfo = NULL;
	    return (status);
        }
    } else if (specColl->collClass == LINKED_COLL) {
        /* a link point */
	specCollCache_t *specCollCache = NULL;
        char newPath[MAX_NAME_LEN];
	specColl_t *curSpecColl;
	char *accessStr;
        dataObjInp_t myDataObjInp;
	rodsObjStat_t *rodsObjStatOut = NULL;

        *dataObjInfo = NULL;
        curSpecColl = specColl;

        status = getMountedSubPhyPath (curSpecColl->collection,
          curSpecColl->phyPath, subPath, newPath);
        if (status < 0) {
            return (status);
        }

	status = resolveLinkedPath (rsComm, newPath, &specCollCache);
	if (status < 0) return status;
#if 0
	while (getSpecCollCache (rsComm, newPath, 0,  &specCollCache) >= 0) {
	    if (linkCnt++ >= MAX_LINK_CNT) {
                rodsLog (LOG_ERROR,
                  "specCollSubStat: linkCnt for %s exceeds %d",
                  subPath, MAX_LINK_CNT);
                return SYS_LINK_CNT_EXCEEDED_ERR;
            }

	    curSpecColl = &specCollCache->specColl;
	    rstrcpy (prevNewPath, newPath, MAX_NAME_LEN);
            status = getMountedSubPhyPath (curSpecColl->collection,
              curSpecColl->phyPath, prevNewPath, newPath);
            if (status < 0) {
                return (status);
            }
	}
#endif
        bzero (&myDataObjInp, sizeof (myDataObjInp));
        rstrcpy (myDataObjInp.objPath, newPath, MAX_NAME_LEN);

        status = collStat (rsComm, &myDataObjInp, &rodsObjStatOut);
	if (status >= 0) {	/* a collection */
            myDataObjInfo = *dataObjInfo =
              (dataObjInfo_t *) malloc (sizeof (dataObjInfo_t));
            memset (myDataObjInfo, 0, sizeof (dataObjInfo_t));
	    myDataObjInfo->specColl = curSpecColl;
            rstrcpy (myDataObjInfo->objPath, newPath, MAX_NAME_LEN);
	    myDataObjInfo->dataId = strtoll (rodsObjStatOut->dataId, 0, 0);
	    rstrcpy (myDataObjInfo->dataOwnerName, rodsObjStatOut->ownerName, 
	      NAME_LEN);
	    rstrcpy (myDataObjInfo->dataOwnerZone, rodsObjStatOut->ownerZone, 
	      NAME_LEN);
	    rstrcpy (myDataObjInfo->dataCreate, rodsObjStatOut->createTime,
	      TIME_LEN);
	    rstrcpy (myDataObjInfo->dataModify, rodsObjStatOut->modifyTime,
	      TIME_LEN);
	    free (rodsObjStatOut);
	    return COLL_OBJ_T;
	}

	/* data object */
	if (specCollPerm == READ_COLL_PERM) {
	    accessStr = ACCESS_READ_OBJECT;
	} else if (specCollPerm == WRITE_COLL_PERM) {
	    accessStr = ACCESS_DELETE_OBJECT;
	} else {
	    accessStr = NULL;
	}

	status = getDataObjInfo (rsComm, &myDataObjInp, dataObjInfo,
          accessStr, 0);
        if (status < 0) {
            myDataObjInfo = *dataObjInfo =
              (dataObjInfo_t *) malloc (sizeof (dataObjInfo_t));
            memset (myDataObjInfo, 0, sizeof (dataObjInfo_t));
	    myDataObjInfo->specColl = curSpecColl;
            rstrcpy (myDataObjInfo->objPath, newPath, MAX_NAME_LEN);
            rodsLog (LOG_DEBUG,
              "specCollSubStat: getDataObjInfo error for %s, status = %d",
              newPath, status);
            return (status);
	} else {
	    (*dataObjInfo)->specColl = curSpecColl;
	    return DATA_OBJ_T;
	}
    } else if (getStructFileType (specColl) >= 0) {
	/* bundle */
	dataObjInp_t myDataObjInp;
	dataObjInfo_t *tmpDataObjInfo;

	bzero (&myDataObjInp, sizeof (myDataObjInp));
        rstrcpy (myDataObjInp.objPath, specColl->objPath, MAX_NAME_LEN);
        status = getDataObjInfo (rsComm, &myDataObjInp, dataObjInfo, NULL, 1);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "specCollSubStat: getDataObjInfo error for %s, status = %d",
              myDataObjInp.objPath, status);
            *dataObjInfo = NULL;
            return (status);
        }

        /* screen out any stale copies */
        status = sortObjInfoForOpen (rsComm, dataObjInfo, 
	  &myDataObjInp.condInput, 0);
	if (status < 0) {
	    rodsLog (LOG_ERROR,
              "specCollSubStat: sortObjInfoForOpen error for %s. status = %d",
              myDataObjInp.objPath, status);
	    return status;
	}

	if (strlen (specColl->resource) > 0) {
	    if (requeDataObjInfoByResc (dataObjInfo, specColl->resource, 
	      0, 1) >= 0) {
		if (strcmp (specColl->resource, 
		  (*dataObjInfo)->rescName) != 0) {
                    rodsLog (LOG_ERROR,
                      "specCollSubStat: %s in %s does not match cache resc %s",
                      myDataObjInp.objPath, (*dataObjInfo)->rescName,
		      specColl->resource);
                    freeAllDataObjInfo (*dataObjInfo);
                    *dataObjInfo = NULL;
                    return (SYS_CACHE_STRUCT_FILE_RESC_ERR);
                }
	    } else {
                rodsLog (LOG_ERROR,
                  "specCollSubStat: requeDataObjInfoByResc %s, resc %s error",
                  myDataObjInp.objPath, specColl->resource);
	        freeAllDataObjInfo (*dataObjInfo);
                *dataObjInfo = NULL;
                return (SYS_CACHE_STRUCT_FILE_RESC_ERR);
	    }
        }

        /* free all the other dataObjInfo */
        if ((*dataObjInfo)->next != NULL) {
            freeAllDataObjInfo ((*dataObjInfo)->next);
            (*dataObjInfo)->next = NULL;
        }

        /* fill in DataObjInfo */
	tmpDataObjInfo = *dataObjInfo;
        tmpDataObjInfo->specColl = specColl;
        rstrcpy (specColl->resource,
          tmpDataObjInfo->rescName, NAME_LEN);
        rstrcpy (specColl->phyPath,
          tmpDataObjInfo->filePath, MAX_NAME_LEN);
        rstrcpy (tmpDataObjInfo->subPath, subPath, MAX_NAME_LEN);
	specColl->replNum = tmpDataObjInfo->replNum;

        if (strcmp ((*dataObjInfo)->subPath, specColl->collection) == 0) {
	    /* no need to go down */
	    return (COLL_OBJ_T);
	}
    } else {
       rodsLog (LOG_ERROR,
          "specCollSubStat: Unknown specColl collClass = %d",
          specColl->collClass);
        return (SYS_UNKNOWN_SPEC_COLL_CLASS);
    }

    status = l3Stat (rsComm, *dataObjInfo, &rodsStat);
    if (status < 0) return status;

    if (rodsStat->st_ctim != 0) {
        snprintf ((*dataObjInfo)->dataCreate, NAME_LEN, "%d",
          rodsStat->st_ctim);
        snprintf ((*dataObjInfo)->dataModify, NAME_LEN, "%d",
          rodsStat->st_mtim);
    }

    if (rodsStat->st_mode & S_IFDIR) {
        objType = COLL_OBJ_T;
    } else {
        objType = DATA_OBJ_T;
        (*dataObjInfo)->dataSize = rodsStat->st_size;
    }
    free (rodsStat);

    return (objType);
}

/* queueSpecCollCache - queue the specColl given in genQueryOut.
 * genQueryOut may contain multiple answer and only one
 * is correct. e.g., objPath = /x/yabc can produce answers:
 * /x/y, /x/ya, /x/yabc, etc. The calling subroutine need to screen
 * /x/y, /x/ya out 
 */

int
queueSpecCollCache (genQueryOut_t *genQueryOut, char *objPath)
{
    specCollCache_t *tmpSpecCollCache;
    int status;
    int i;
    sqlResult_t *dataId;
    sqlResult_t *ownerName;
    sqlResult_t *ownerZone;
    sqlResult_t *createTime;
    sqlResult_t *modifyTime;
    sqlResult_t *collType;
    sqlResult_t *collection;
    sqlResult_t *collInfo1;
    sqlResult_t *collInfo2;
    char *tmpDataId, *tmpOwnerName, *tmpOwnerZone, *tmpCreateTime, 
      *tmpModifyTime, *tmpCollType, *tmpCollection, *tmpCollInfo1,
      *tmpCollInfo2;
    specColl_t *specColl;

    if ((dataId = getSqlResultByInx (genQueryOut, COL_COLL_ID)) == NULL) {
        rodsLog (LOG_ERROR,
          "queueSpecCollCache: getSqlResultByInx for COL_COLL_ID failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else if ((ownerName = getSqlResultByInx (genQueryOut,
      COL_COLL_OWNER_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
         "queueSpecCollCache:getSqlResultByInx for COL_COLL_OWNER_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else if ((ownerZone = getSqlResultByInx (genQueryOut,
      COL_COLL_OWNER_ZONE)) == NULL) {
        rodsLog (LOG_ERROR,
         "queueSpecCollCache:getSqlResultByInx for COL_COLL_OWNER_ZONE failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else if ((createTime = getSqlResultByInx (genQueryOut,
      COL_COLL_CREATE_TIME)) == NULL) {
        rodsLog (LOG_ERROR,
         "queueSpecCollCache:getSqlResultByInx for COL_COLL_CREATE_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else if ((modifyTime = getSqlResultByInx (genQueryOut,
      COL_COLL_MODIFY_TIME)) == NULL) {
        rodsLog (LOG_ERROR,
         "queueSpecCollCache:getSqlResultByInx for COL_COLL_MODIFY_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else if ((collType = getSqlResultByInx (genQueryOut,
      COL_COLL_TYPE)) == NULL) {
        rodsLog (LOG_ERROR,
         "queueSpecCollCache:getSqlResultByInx for COL_COLL_TYPE failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else if ((collection = getSqlResultByInx (genQueryOut,
      COL_COLL_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
         "queueSpecCollCache:getSqlResultByInx for COL_COLL_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else if ((collInfo1 = getSqlResultByInx (genQueryOut,
      COL_COLL_INFO1)) == NULL) {
        rodsLog (LOG_ERROR,
         "queueSpecCollCache:getSqlResultByInx for COL_COLL_INFO1 failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else if ((collInfo2 = getSqlResultByInx (genQueryOut,
      COL_COLL_INFO2)) == NULL) {
        rodsLog (LOG_ERROR,
         "queueSpecCollCache:getSqlResultByInx for COL_COLL_INFO2 failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    for (i = 0; i <= genQueryOut->rowCnt; i++) {
	int len; 
	char *tmpPtr;

	tmpCollection = &collection->value[collection->len * i];

        len = strlen (tmpCollection);
	tmpPtr = objPath + len;

	if (*tmpPtr == '\0' || *tmpPtr == '/') { 
    	    tmpSpecCollCache = malloc (sizeof (specCollCache_t));
    	    memset (tmpSpecCollCache, 0, sizeof (specCollCache_t));

	    tmpDataId = &dataId->value[dataId->len * i];
	    tmpOwnerName = &ownerName->value[ownerName->len * i];
	    tmpOwnerZone = &ownerZone->value[ownerZone->len * i];
	    tmpCreateTime = &createTime->value[createTime->len * i];
	    tmpModifyTime = &modifyTime->value[modifyTime->len * i];
	    tmpCollType = &collType->value[collType->len * i];
	    tmpCollInfo1 = &collInfo1->value[collInfo1->len * i];
	    tmpCollInfo2 = &collInfo2->value[collInfo2->len * i];

	    specColl = &tmpSpecCollCache->specColl;
	    status = resolveSpecCollType (tmpCollType, tmpCollection,
	      tmpCollInfo1, tmpCollInfo2, specColl);
	    if (status < 0) return status;

            rstrcpy (tmpSpecCollCache->collId, tmpDataId, NAME_LEN);
            rstrcpy (tmpSpecCollCache->ownerName, tmpOwnerName, NAME_LEN);
            rstrcpy (tmpSpecCollCache->ownerZone, tmpOwnerZone, NAME_LEN);
            rstrcpy (tmpSpecCollCache->createTime, tmpCreateTime, NAME_LEN);
            rstrcpy (tmpSpecCollCache->modifyTime, tmpModifyTime, NAME_LEN);
            tmpSpecCollCache->next = SpecCollCacheHead;
            SpecCollCacheHead = tmpSpecCollCache;
	    return 0;
	}
    }

    return CAT_NO_ROWS_FOUND;
}

int
queueSpecCollCacheWithObjStat (rodsObjStat_t *rodsObjStatOut)
{
    specCollCache_t *tmpSpecCollCache;

    tmpSpecCollCache = malloc (sizeof (specCollCache_t));
    memset (tmpSpecCollCache, 0, sizeof (specCollCache_t));

    tmpSpecCollCache->specColl = *rodsObjStatOut->specColl;

    rstrcpy (tmpSpecCollCache->collId, rodsObjStatOut->dataId, NAME_LEN);
    rstrcpy (tmpSpecCollCache->ownerName, rodsObjStatOut->ownerName, NAME_LEN);
    rstrcpy (tmpSpecCollCache->ownerZone, rodsObjStatOut->ownerZone, NAME_LEN);
    rstrcpy (tmpSpecCollCache->createTime, rodsObjStatOut->createTime, 
      NAME_LEN);
    rstrcpy (tmpSpecCollCache->modifyTime, rodsObjStatOut->modifyTime, 
      NAME_LEN);

    tmpSpecCollCache->next = SpecCollCacheHead;
    SpecCollCacheHead = tmpSpecCollCache;

    return 0;

}

specCollCache_t *
matchSpecCollCache (char *objPath)
{
    specCollCache_t *tmpSpecCollCache = SpecCollCacheHead;

    while (tmpSpecCollCache != NULL) {
	int len = strlen (tmpSpecCollCache->specColl.collection);
	if (strncmp (tmpSpecCollCache->specColl.collection, objPath, len) 
          == 0) {
	    char *tmpPtr = objPath + len;

	    if (*tmpPtr == '\0' || *tmpPtr == '/') {
		return (tmpSpecCollCache);
	    }
	}
	tmpSpecCollCache = tmpSpecCollCache->next;
    }
    return (NULL);
}

/* resolvePathInSpecColl - given the object path in dataObjInp->objPath, see if
 * it is in the path of a special collection (mounted or structfile).
 * If it is not in a special collection, returns a -ive value.
 * The inCachOnly flag asks it to check the specColl in the global cache only
 * If it is, returns a dataObjInfo struct with dataObjInfo->specColl != NULL.
 * Returns COLL_OBJ_T if the path is a collection or DATA_OBJ_T if the
 * path is a data oobject.
 */
int
resolvePathInSpecColl (rsComm_t *rsComm, char *objPath,
specCollPerm_t specCollPerm, int inCachOnly, dataObjInfo_t **dataObjInfo)
{
    specCollCache_t *specCollCache;
    specColl_t *cachedSpecColl;
    int status;
    char *accessStr;

    if (objPath == NULL) {
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if ((status = getSpecCollCache (rsComm, objPath, inCachOnly,
      &specCollCache)) < 0) {
        return (status);
    } else {
        cachedSpecColl = &specCollCache->specColl;
    }

    if (specCollPerm != UNKNOW_COLL_PERM) {
        if (specCollPerm == WRITE_COLL_PERM) {
            accessStr = ACCESS_DELETE_OBJECT;
        } else {
            accessStr = ACCESS_READ_OBJECT;
        }

        if (specCollCache->perm < specCollPerm) {
            status = checkCollAccessPerm (rsComm, cachedSpecColl->collection,
              accessStr);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "resolveSpecColl: checkCollAccessPerm err for %s, stat = %d",
                  cachedSpecColl->collection, status);
                return (status);
            } else {
                specCollCache->perm = specCollPerm;
            }
        }
    }

    status = specCollSubStat (rsComm, cachedSpecColl, objPath,
      specCollPerm, dataObjInfo);

#if 0
    if (*dataObjInfo != NULL && getStructFileType ((*dataObjInfo)->specColl)
      >= 0) {
        dataObjInp->numThreads = NO_THREADING;
    }
#endif

    if (status < 0) {
        if (*dataObjInfo != NULL) {
           /* does not exist. return the dataObjInfo anyway */
            return (SYS_SPEC_COLL_OBJ_NOT_EXIST);
        }
        rodsLog (LOG_ERROR,
          "resolveSpecColl: specCollSubStat error for %s, status = %d",
          objPath, status);
        return (status);
    } else {
        if (*dataObjInfo != NULL) {
            if (specCollPerm == WRITE_COLL_PERM)
                (*dataObjInfo)->writeFlag = 1;
        }
    }

    return (status);
}

int
resolveLinkedPath (rsComm_t *rsComm, char *objPath, 
specCollCache_t **specCollCache)
{
    *specCollCache = NULL;
    int linkCnt = 0;
    specColl_t *curSpecColl;
    char prevNewPath[MAX_NAME_LEN];
    int status;

    while (getSpecCollCache (rsComm, objPath, 0,  specCollCache) >= 0) {
        if (linkCnt++ >= MAX_LINK_CNT) {
            rodsLog (LOG_ERROR,
              "resolveLinkedPath: linkCnt for %s exceeds %d",
              objPath, MAX_LINK_CNT);
            return SYS_LINK_CNT_EXCEEDED_ERR;
        }

        curSpecColl = &(*specCollCache)->specColl;
        rstrcpy (prevNewPath, objPath, MAX_NAME_LEN);
        status = getMountedSubPhyPath (curSpecColl->collection,
          curSpecColl->phyPath, prevNewPath, objPath);
        if (status < 0) {
            return (status);
        }
    }
    return 0;
}
