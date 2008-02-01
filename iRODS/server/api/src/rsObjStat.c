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

    status = irsObjStat (rsComm, dataObjInp, 0, rodsObjStatOut);

    return (status);
}

/* irsObjStat - internal version of irsObjStat. Mostly to deal with
 * specColl
 */

int
irsObjStat (rsComm_t *rsComm, dataObjInp_t *dataObjInp, int intenFlag,
rodsObjStat_t **rodsObjStatOut)
{
    int status;
    rodsServerHost_t *rodsServerHost = NULL;

    status = getAndConnRcatHost (rsComm, MASTER_RCAT, dataObjInp->objPath,
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
        status = querySubInSpecColl (rsComm, dataObjInp->objPath, 1,
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
			/* use the cache copy instead */
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
        if (status >= 0) {
	    if (getSpecCollCache (rsComm, dataObjInp->objPath, 0,
              &specCollCache) >= 0) {
#if 0   /* XXXXXX specColl is cached */
		(*rodsObjStatOut)->specColl = malloc (sizeof (specColl_t));
		*(*rodsObjStatOut)->specColl = specCollCache->specColl;
#endif
                (*rodsObjStatOut)->specColl = &specCollCache->specColl;
	    }
	    return (status);
	}
    }

#if 0
    if (dataObjInp->oprType == PUT_OPR) {
        /* don't call querySubInSpecColl if BUNDLE_COLL */
        if (getSpecCollCache (rsComm, dataObjInp->objPath, 0,
          &specCollCache) < 0 ||
          specCollCache->specColl.class == BUNDLE_COLL) {
            return (USER_FILE_DOES_NOT_EXIST);
        }
    }
#endif
    /* now check specColl */
    /* XXXX need to check a rule if it supports spec collection */
    status = querySubInSpecColl (rsComm, dataObjInp->objPath, 0,
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

#if 0	/* XXXXX not needed ? */
	    if (strlen (collType->value) > 0) {
		specColl_t *specColl;
    		specColl = (*rodsObjStatOut)->specColl =
     		 (specColl_t *) malloc (sizeof (specColl_t));
    		memset (specColl, 0, sizeof (specColl_t));
	        status = resolveSpecCollType (collType->value, 
		  dataObjInp->objPath, collInfo1->value, collInfo2->value, 
		  specColl);

		if (status < 0) return (status);
	    }
#endif
	}
#if 0
    } else {
        if (dataObjInp->oprType == PUT_OPR) { 
            specCollCache_t *specCollCache;
            /* don't call querySubInSpecColl if BUNDLE_COLL */
            if (getSpecCollCache (rsComm, dataObjInp->objPath, 0,
              &specCollCache) < 0 ||
              specCollCache->specColl.class == BUNDLE_COLL) { 
                return (USER_FILE_DOES_NOT_EXIST);
            }
        }
        /* XXXX need to check a rule if it supports spec collection */
        status = querySubInSpecColl (rsComm, dataObjInp->objPath, 0,
          rodsObjStatOut);
        if (status < 0) status = USER_FILE_DOES_NOT_EXIST;
#endif
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
            (*rodsObjStatOut)->numCopies = genQueryOut->rowCnt;

            for (i = 0;i < genQueryOut->rowCnt; i++) {
                if (atoi (&replStatus->value[replStatus->len * i]) > 0) {
                    rstrcpy ((*rodsObjStatOut)->dataId, 
		      &dataId->value[dataId->len * i], NAME_LEN);
                    (*rodsObjStatOut)->objSize =
                      strtoll (&dataSize->value[dataSize->len * i], 0, 0);
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
#if 0
    } else {
        if (dataObjInp->oprType == PUT_OPR) { 
	    specCollCache_t *specCollCache;
	    /* don't call querySubInSpecColl if BUNDLE_COLL */
    	    if (getSpecCollCache (rsComm, dataObjInp->objPath, 0, 
              &specCollCache) < 0 || 
	      specCollCache->specColl.class == BUNDLE_COLL) {  
		return (USER_FILE_DOES_NOT_EXIST);
	    }
	}

	/* XXXX need to check a rule if it supports spec collection */ 
	status = querySubInSpecColl (rsComm, dataObjInp->objPath, 0,
	  rodsObjStatOut);
	if (status < 0) 
            return (USER_FILE_DOES_NOT_EXIST);
#endif
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

    status = queueSpecCollCache (genQueryOut);
    freeGenQueryOut (&genQueryOut);

    if (status < 0) return (status);
    *specCollCache = SpecCollCacheHead;  /* queued at top */

    return (0);
}

int
querySubInSpecColl (rsComm_t *rsComm, char *objPath, 
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
              "querySubInSpecColl: getSpecCollCache for %s, status = %d",
              objPath, status);
	}
        return (status);
    }
 
    *rodsObjStatOut = (rodsObjStat_t *) malloc (sizeof (rodsObjStat_t));
    memset (*rodsObjStatOut, 0, sizeof (rodsObjStat_t));
#if 0	/* XXXXX use cached specColl */
    specColl = (*rodsObjStatOut)->specColl = 
     (specColl_t *) malloc (sizeof (specColl_t));
    *specColl = specCollCache->specColl;
#else
    specColl = (*rodsObjStatOut)->specColl = &specCollCache->specColl;
#endif
    rstrcpy ((*rodsObjStatOut)->dataId, specCollCache->collId, NAME_LEN);
    rstrcpy ((*rodsObjStatOut)->ownerName, specCollCache->ownerName, NAME_LEN);
    rstrcpy ((*rodsObjStatOut)->ownerZone, specCollCache->ownerZone, NAME_LEN);

    status = specCollSubStat (rsComm, specColl, objPath, &dataObjInfo);

    if (status < 0) {
	(*rodsObjStatOut)->objType = UNKNOWN_OBJ_T;
        rstrcpy ((*rodsObjStatOut)->createTime, specCollCache->createTime, 
	  NAME_LEN);
        rstrcpy ((*rodsObjStatOut)->modifyTime, specCollCache->modifyTime, 
	  NAME_LEN);
	freeAllDataObjInfo (dataObjInfo);
	return (0);
    } else {
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

int 
querySpecColl (rsComm_t *rsComm, char *objPath, genQueryOut_t **genQueryOut)
{
    genQueryInp_t genQueryInp;
    int status;
    char condStr[MAX_NAME_LEN];

    /* see if objPath is in the path of a spec collection */
    memset (&genQueryInp, 0, sizeof (genQueryInp));

    snprintf (condStr, MAX_NAME_LEN, "begin_of '%s'", objPath);
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

    if (status < 0) {
        return (status);
    }

    if ((*genQueryOut)->rowCnt != 1) {
        rodsLog (LOG_ERROR,
          "querySpecColl: Too many result rowCnt = %d for %s",
          (*genQueryOut)->rowCnt, objPath);
        freeGenQueryOut (genQueryOut);
        return (SYS_TOO_MANY_QUERY_RESULT);
    }

    return (0);
}

int
specCollSubStat (rsComm_t *rsComm, specColl_t *specColl, 
char *subPath, dataObjInfo_t **dataObjInfo)
{
    int status;
    int objType;
    rodsStat_t *rodsStat = NULL;

    if (specColl->class == MOUNTED_COLL) {
	dataObjInfo_t *myDataObjInfo;

	/* a mount point */
        myDataObjInfo = *dataObjInfo = 
	  (dataObjInfo_t *) malloc (sizeof (dataObjInfo_t));
        memset (myDataObjInfo, 0, sizeof (dataObjInfo_t));
#if 0	/* XXXXX use cached specColl */
	myDataObjInfo->specColl = (specColl_t *) malloc (sizeof (specColl_t));
        *myDataObjInfo->specColl = *specColl;
#else
        myDataObjInfo->specColl = specColl;
#endif

        status = resolveResc (specColl->resource, &myDataObjInfo->rescInfo);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "specCollSubStat: _getRescInfo error for %s, status = %d",
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
    } else if (getStructFileType (specColl) >= 0) {
	/* bundle */
	dataObjInp_t myDataObjInp;
	dataObjInfo_t *tmpDataObjInfo;

        rstrcpy (myDataObjInp.objPath, specColl->objPath, MAX_NAME_LEN);
        status = getDataObjInfo (rsComm, &myDataObjInp, dataObjInfo, NULL, 1);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "resolveSpecColl: getDataObjInfo error for %s, status = %d",
              myDataObjInp.objPath, status);
            *dataObjInfo = NULL;
            return (status);
        }

        /* screen out any stale copies */
        sortObjInfoForOpen (dataObjInfo, &myDataObjInp.condInput, 0);

	if (strlen (specColl->resource) > 0) {
	    if (requeDataObjInfoByResc (dataObjInfo, specColl->resource, 
	      0, 1) >= 0) {
		if (strcmp (specColl->resource, 
		  (*dataObjInfo)->rescName) != 0) {
                    rodsLog (LOG_ERROR,
                      "resolveSpecColl: %s in %s does not match cache resc %s",
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

#if 0	/* do just one */
        while (tmpDataObjInfo != NULL) {
            rstrcpy (tmpDataObjInfo->subPath, subPath, MAX_NAME_LEN);
            tmpDataObjInfo->specColl = specColl;
	      (specColl_t *) malloc (sizeof (specColl_t));
            *tmpDataObjInfo->specColl = *specColl;
	    rstrcpy (tmpDataObjInfo->specColl->resource, 
	      tmpDataObjInfo->rescName, NAME_LEN);
            rstrcpy (tmpDataObjInfo->specColl->phyPath, 
	      tmpDataObjInfo->filePath, MAX_NAME_LEN);
	    tmpDataObjInfo = tmpDataObjInfo->next;
        }
#endif
        if (strcmp ((*dataObjInfo)->subPath, specColl->collection) == 0) {
	    /* no need to go down */
	    return (COLL_OBJ_T);
	}
    } else {
       rodsLog (LOG_ERROR,
          "specCollSubStat: Unknown specColl class = %d",
          specColl->class);
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

int
queueSpecCollCache (genQueryOut_t *genQueryOut)
{
    specCollCache_t *tmpSpecCollCache;
    int status;
    sqlResult_t *dataId;
    sqlResult_t *ownerName;
    sqlResult_t *ownerZone;
    sqlResult_t *createTime;
    sqlResult_t *modifyTime;
    sqlResult_t *collType;
    sqlResult_t *collection;
    sqlResult_t *collInfo1;
    sqlResult_t *collInfo2;
    specColl_t *specColl;

    tmpSpecCollCache = malloc (sizeof (specCollCache_t));
    memset (tmpSpecCollCache, 0, sizeof (specCollCache_t));

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

    specColl = &tmpSpecCollCache->specColl;
    status = resolveSpecCollType (collType->value, collection->value,
      collInfo1->value, collInfo2->value, specColl);

    if (status < 0) return status;

    rstrcpy (tmpSpecCollCache->collId, dataId->value, NAME_LEN);
    rstrcpy (tmpSpecCollCache->ownerName, ownerName->value, NAME_LEN);
    rstrcpy (tmpSpecCollCache->ownerZone, ownerZone->value, NAME_LEN);
    rstrcpy (tmpSpecCollCache->createTime, createTime->value, NAME_LEN);
    rstrcpy (tmpSpecCollCache->modifyTime, modifyTime->value, NAME_LEN);

    tmpSpecCollCache->next = SpecCollCacheHead;
    SpecCollCacheHead = tmpSpecCollCache;

    return 0;
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

