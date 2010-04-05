/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* objMetaOpr.c - metadata operation at the object level */

#ifndef windows_platform
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif
#include "objMetaOpr.h"
#include "modDataObjMeta.h"
#include "ruleExecSubmit.h"
#include "ruleExecDel.h"
#include "genQuery.h"
#include "icatHighLevelRoutines.h"
#include "reSysDataObjOpr.h"
#include "miscUtil.h"
#include "rodsClient.h"
#include "rsIcatOpr.h"

int
getRescInfo (rsComm_t *rsComm, char *defaultResc, keyValPair_t *condInput, 
rescGrpInfo_t **rescGrpInfo)
{
    char *rescName;
    int status;

    if ((rescName = getValByKey (condInput, BACKUP_RESC_NAME_KW)) == NULL &&
      (rescName = getValByKey (condInput, DEST_RESC_NAME_KW)) == NULL &&
      (rescName = getValByKey (condInput, DEF_RESC_NAME_KW)) == NULL &&
      (rescName = getValByKey (condInput, RESC_NAME_KW)) == NULL &&
      ((rescName = defaultResc) == NULL || strcmp (defaultResc, "null") == 0)) {
        return (USER_NO_RESC_INPUT_ERR);
    }
    status = _getRescInfo (rsComm, rescName, rescGrpInfo);

    return (status);
}

int
getRescStatus (rsComm_t *rsComm, char *inpRescName, keyValPair_t *condInput)
{
    char *rescName;
    int status;
    rescGrpInfo_t *rescGrpInfo = NULL;

    if (inpRescName == NULL) {
        if ((rescName = getValByKey (condInput, BACKUP_RESC_NAME_KW)) == NULL &&
          (rescName = getValByKey (condInput, DEST_RESC_NAME_KW)) == NULL &&
          (rescName = getValByKey (condInput, DEF_RESC_NAME_KW)) == NULL) { 
            return (INT_RESC_STATUS_DOWN);
	}
    } else {
	rescName = inpRescName;
    }
    status = _getRescInfo (rsComm, rescName, &rescGrpInfo);

    freeAllRescGrpInfo (rescGrpInfo);

    if (status < 0) {
	return INT_RESC_STATUS_DOWN;
    } else {
	return INT_RESC_STATUS_UP;
    }
}

/* _getRescInfo - get the rescInfo. rescGroupName can be a resource
 * or a resource group.
 */

int
_getRescInfo (rsComm_t *rsComm, char *rescGroupName, 
rescGrpInfo_t **rescGrpInfo)
{
    int status;

    *rescGrpInfo = NULL;

    /* a resource ? */

    status = resolveAndQueResc (rescGroupName, NULL, rescGrpInfo);
    if (status >= 0 || status == SYS_RESC_IS_DOWN) {
	return (status);
    }
    /* assume it is a rescGrp */

    status = resolveRescGrp (rsComm, rescGroupName, rescGrpInfo);

    return (status);
}

/* resolveRescGrp - get the rescInfo for the rescGroupName.
 * rescGroupName can only be a resource group.
 */
 
int 
resolveRescGrp (rsComm_t *rsComm, char *rescGroupName, 
rescGrpInfo_t **rescGrpInfo)
{
    rescGrpInfo_t *tmpRescGrpInfo;
#if 0
    genQueryOut_t *genQueryOut = NULL;
    sqlResult_t *rescName;
    char *rescNameStr;
#endif
    int status;

    if ((status = initRescGrp (rsComm)) < 0) return status;

    /* see if it is in cache */

    tmpRescGrpInfo = CachedRescGrpInfo;

    while (tmpRescGrpInfo != NULL) {
	if (strcmp (rescGroupName, tmpRescGrpInfo->rescGroupName)
	  == 0) {
	    replRescGrpInfo (tmpRescGrpInfo, rescGrpInfo);
	    /* *rescGrpInfo = tmpRescGrpInfo; */
	    return (0);
	}
	tmpRescGrpInfo = tmpRescGrpInfo->cacheNext;
    }

    return CAT_NO_ROWS_FOUND;

#if 0	/* replaced with initRescGrp */
    /* not in cache, query it */

    status = queryRescInRescGrp (rsComm, rescGroupName, &genQueryOut);
    if (status < 0) {
	return (status);
    }

    if ((rescName = getSqlResultByInx (genQueryOut, COL_R_RESC_NAME)) ==
      NULL) {
        rodsLog (LOG_NOTICE,
          "resolveRescGrp: getSqlResultByInx for COL_R_RESC_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    tmpRescGrpInfo = NULL;
    for (i = 0;i < genQueryOut->rowCnt; i++) {
        rescNameStr = &rescName->value[rescName->len * i];
        status = resolveAndQueResc (rescNameStr, rescGroupName, 
	  &tmpRescGrpInfo);
	if (status < 0) {
            rodsLog (LOG_NOTICE,
              "resolveRescGrp: resolveAndQueResc error for %s. status = %d",
	      rescNameStr, status);
	    freeGenQueryOut (&genQueryOut);
	    return (status);
	}
    }

    freeGenQueryOut (&genQueryOut);


    if (tmpRescGrpInfo != NULL) {
        replRescGrpInfo (tmpRescGrpInfo, rescGrpInfo);
        /* query it in cache */
	tmpRescGrpInfo->cacheNext = CachedRescGrpInfo;
	CachedRescGrpInfo = tmpRescGrpInfo;
    }

    return (status);
#endif
}

int
replRescGrpInfo (rescGrpInfo_t *srcRescGrpInfo, rescGrpInfo_t **destRescGrpInfo)
{
    rescGrpInfo_t *tmpSrcRescGrpInfo, *tmpDestRescGrpInfo, 
      *lastDestRescGrpInfo;

    *destRescGrpInfo = lastDestRescGrpInfo = NULL;

    tmpSrcRescGrpInfo = srcRescGrpInfo;
    while (tmpSrcRescGrpInfo != NULL) {
	tmpDestRescGrpInfo = (rescGrpInfo_t *) malloc (sizeof (rescGrpInfo_t));
	memset (tmpDestRescGrpInfo, 0, sizeof (rescGrpInfo_t));
	tmpDestRescGrpInfo->rescInfo = tmpSrcRescGrpInfo->rescInfo; 
	rstrcpy (tmpDestRescGrpInfo->rescGroupName, 
	  tmpSrcRescGrpInfo->rescGroupName, NAME_LEN);
	if (*destRescGrpInfo == NULL) {
	    *destRescGrpInfo = tmpDestRescGrpInfo;
	} else {
	    lastDestRescGrpInfo->next = tmpDestRescGrpInfo;
	}
	lastDestRescGrpInfo = tmpDestRescGrpInfo;
	tmpSrcRescGrpInfo = tmpSrcRescGrpInfo->next;
    }

    return (0);
}

int
queryRescInRescGrp (rsComm_t *rsComm, char *rescGroupName, 
genQueryOut_t **genQueryOut)
{
    genQueryInp_t genQueryInp;
    char tmpStr[NAME_LEN];
    int status;

    memset (&genQueryInp, 0, sizeof (genQueryInp_t));

    snprintf (tmpStr, NAME_LEN, "='%s'", rescGroupName);
    addInxVal (&genQueryInp.sqlCondInp, COL_RESC_GROUP_NAME, tmpStr);
    addInxIval (&genQueryInp.selectInp, COL_R_RESC_NAME, 1);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, &genQueryInp, genQueryOut);

    clearGenQueryInp (&genQueryInp);

    return (status);

}

int
resolveAndQueResc (char *rescName, char *rescGroupName, 
rescGrpInfo_t **rescGrpInfo)
{
    rescInfo_t *myRescInfo;
    int status;

    status = resolveResc (rescName, &myRescInfo);

    if (status < 0) {
	return (status);
    } else if (myRescInfo->rescStatus == INT_RESC_STATUS_DOWN) {
	return SYS_RESC_IS_DOWN;
    } else {
	queResc (myRescInfo, rescGroupName, rescGrpInfo, BY_TYPE_FLAG);
	return (0);
    }
}

int
resolveResc (char *rescName, rescInfo_t **rescInfo)
{
    rescInfo_t *myRescInfo;
    rescGrpInfo_t *tmpRescGrpInfo;


    *rescInfo = NULL;

    /* search the global RescGrpInfo */

    tmpRescGrpInfo = RescGrpInfo;

    while (tmpRescGrpInfo != NULL) {
        myRescInfo = tmpRescGrpInfo->rescInfo;
        if (strcmp (rescName, myRescInfo->rescName) == 0) {
	    *rescInfo = myRescInfo;
            return (0);
        }
        tmpRescGrpInfo = tmpRescGrpInfo->next;
    }
    /* no match */ 
    rodsLog (LOG_DEBUG1,
      "resolveResc: resource %s not configured in RCAT", rescName);
    return (SYS_INVALID_RESC_INPUT);
}

int
getNumResc (rescGrpInfo_t *rescGrpInfo)
{
    rescGrpInfo_t *tmpRescGrpInfo;
    int numResc = 0;

    tmpRescGrpInfo = rescGrpInfo;
    while (tmpRescGrpInfo != NULL) {
	numResc++;
	tmpRescGrpInfo = tmpRescGrpInfo->next;
    }
    return (numResc);
}

int
sortResc (rsComm_t *rsComm, rescGrpInfo_t **rescGrpInfo, 
keyValPair_t *condInput, char *sortScheme)
{
    int numResc;

    if (sortScheme == NULL) {
        return (0);
    }

    if (rescGrpInfo == NULL || *rescGrpInfo == NULL) {
	return (0);
    }

    numResc = getNumResc (*rescGrpInfo);

    if (numResc <= 1) {
	return (0);
    }

    if (sortScheme == NULL) {
	return (0);
    }

    if (strcmp (sortScheme, "random") == 0) {
	sortRescRandom (rescGrpInfo);
#if 0
        order = random() % numResc;
	if (order == 0) {
	    return (0);
	}

        tmpRescGrpInfo = *rescGrpInfo;
	for (i = 0; i < order; i++) {
	    tmpRescGrpInfo = tmpRescGrpInfo->next;
	}
	    
	/* exchange rescInfo with the head */

        tmpRescInfo = tmpRescGrpInfo->rescInfo;
        tmpRescGrpInfo->rescInfo = (*rescGrpInfo)->rescInfo;
        (*rescGrpInfo)->rescInfo = tmpRescInfo;
#endif
    } else if (strcmp (sortScheme, "byRescClass") == 0) {
	sortRescByType (rescGrpInfo);
	} else if (strcmp (sortScheme, "byLoad") == 0) {
	sortRescByLoad (rsComm, rescGrpInfo);
    } else {
	    rodsLog (LOG_ERROR,
	      "sortResc: unknown sortScheme %s", sortScheme);
    }

    return (0);
}

int
sortRescRandom (rescGrpInfo_t **rescGrpInfo)
{
    int *randomArray;
    int numResc;
    int i, j, status;
    rescGrpInfo_t *tmpRescGrpInfo;
    rescInfo_t **rescInfoArray;


    tmpRescGrpInfo = *rescGrpInfo;
    numResc = getNumResc (tmpRescGrpInfo);

    if (numResc <= 1) {
        return (0);
    }
    if ((status = getRandomArray (&randomArray, numResc)) < 0) return status;
    rescInfoArray = malloc (numResc * sizeof (rescInfo_t *));
    for (i = 0; i < numResc; i++) {
	j = randomArray[i] - 1;
	rescInfoArray[j] = tmpRescGrpInfo->rescInfo;
	tmpRescGrpInfo = tmpRescGrpInfo->next;
    }
    tmpRescGrpInfo = *rescGrpInfo;
    for (i = 0; i < numResc; i++) {
        tmpRescGrpInfo->rescInfo = rescInfoArray[i];
        tmpRescGrpInfo = tmpRescGrpInfo->next;
    }
    free (rescInfoArray);
    free (randomArray);
    return 0;
}

int
getRandomArray (int **randomArray, int size)
{
    int *myArray;
    int i, j, k;

    if (size < 0) {
        *randomArray = NULL;
        return -1;
    }

    myArray = (int *) malloc (size * sizeof (int));
    bzero (myArray, size * sizeof (int));
    for (i = size ; i > 0; i --) {
	int ranNum;
	/* get a number between 0 and i-1 */
        ranNum = (random() >> 2) % i;
        k = 0;
	/* find the ranNum th empty slot  */
        for (j = 0; j < size ; j ++) {
            if (myArray[j] == 0) {
                k++;
            }
	    if (k > ranNum) break;
        }
        myArray[j] = i;
    }
    *randomArray = myArray;

    return (0);
}

int 
sortRescByType (rescGrpInfo_t **rescGrpInfo)
{
    rescGrpInfo_t *tmpRescGrpInfo, *tmp1RescGrpInfo;
    rescInfo_t *tmpRescInfo, *tmp1RescInfo;

    tmpRescGrpInfo = *rescGrpInfo;
 
    /* float CACHE_CL to top */

    while (tmpRescGrpInfo != NULL) {
        tmpRescInfo = tmpRescGrpInfo->rescInfo;
        if (RescClass[tmpRescInfo->rescClassInx].classType == CACHE_CL) {
            /* find a slot to exchange rescInfo */
	    tmp1RescGrpInfo = *rescGrpInfo;
	    while (tmp1RescGrpInfo != NULL) {
		if (tmp1RescGrpInfo == tmpRescGrpInfo) break;
	        tmp1RescInfo = tmp1RescGrpInfo->rescInfo;
		if (RescClass[tmp1RescInfo->rescClassInx].classType > CACHE_CL) {
                    tmpRescGrpInfo->rescInfo = tmp1RescInfo;
		    tmp1RescGrpInfo->rescInfo = tmpRescInfo;
		    break;
		}
		tmp1RescGrpInfo = tmp1RescGrpInfo->next;
	    }
        }
        tmpRescGrpInfo = tmpRescGrpInfo->next;
    }

    /* float ARCHIVAL_CL to second */

    while (tmpRescGrpInfo != NULL) {
        tmpRescInfo = tmpRescGrpInfo->rescInfo;
        if (RescClass[tmpRescInfo->rescClassInx].classType == ARCHIVAL_CL) {
            /* find a slot to exchange rescInfo */
            tmp1RescGrpInfo = *rescGrpInfo;
            while (tmp1RescGrpInfo != NULL) {
                if (tmp1RescGrpInfo == tmpRescGrpInfo) break;
                tmp1RescInfo = tmp1RescGrpInfo->rescInfo;
                if (RescClass[tmp1RescInfo->rescClassInx].classType > ARCHIVAL_CL) {
                    tmpRescGrpInfo->rescInfo = tmp1RescInfo;
                    tmp1RescGrpInfo->rescInfo = tmpRescInfo;
                    break;
                }
                tmp1RescGrpInfo = tmp1RescGrpInfo->next;
            }
        }
        tmpRescGrpInfo = tmpRescGrpInfo->next;
    }

    return 0;
}

int 
sortRescByLoad (rsComm_t *rsComm, rescGrpInfo_t **rescGrpInfo)
{
	int i, j, loadmin, loadList[MAX_NSERVERS], nresc, status, timeList[MAX_NSERVERS];
	char rescList[MAX_NSERVERS][MAX_NAME_LEN], *tResult;
	rescGrpInfo_t *tmpRescGrpInfo, *tmp1RescGrpInfo;
	rescInfo_t *tmpRescInfo;
	genQueryInp_t genQueryInp;
	genQueryOut_t *genQueryOut = NULL;
	time_t timenow;
	
	/* query the database in order to retrieve the information on the resources' load */
	memset(&genQueryInp, 0, sizeof (genQueryInp));
	addInxIval(&genQueryInp.selectInp, COL_SLD_RESC_NAME, 1);
	addInxIval(&genQueryInp.selectInp, COL_SLD_LOAD_FACTOR, 1);
	addInxIval(&genQueryInp.selectInp, COL_SLD_CREATE_TIME, SELECT_MAX);
	genQueryInp.maxRows = MAX_SQL_ROWS;
	status =  rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
	if ( status == 0 ) {
		nresc = genQueryOut->rowCnt;
		for (i=0; i<genQueryOut->attriCnt; i++) {
           for (j=0; j<nresc; j++) {
				tResult = genQueryOut->sqlResult[i].value;
				tResult += j*genQueryOut->sqlResult[i].len;
				switch (i) {
					case 0:
						rstrcpy(rescList[j], tResult, genQueryOut->sqlResult[i].len);
						break;
					case 1:
						loadList[j] = atoi(tResult);
						break;
					case 2:
						timeList[j] = atoi(tResult);
						break;
				}
			}
        }
	}
	else {
		return (0);
	}
	clearGenQueryInp(&genQueryInp);
	freeGenQueryOut(&genQueryOut);
	
	/* find the least loaded resource among the ones for which the information is available */
	tmpRescGrpInfo = *rescGrpInfo;
	tmpRescInfo = tmpRescGrpInfo->rescInfo;
	tmp1RescGrpInfo = tmpRescGrpInfo;
	loadmin = 100;
	/* retrieve local time in order to check if the load information is up to date, ie less than MAX_ELAPSE_TIME seconds old */
	(void) time(&timenow);
    while (tmpRescGrpInfo != NULL) {
		for (i=0; i<nresc; i++) {
			if ( strcmp(rescList[i], tmpRescGrpInfo->rescInfo->rescName) == 0 ) {
				if ( loadList[i] >= 0 && loadmin > loadList[i] && (timenow - timeList[i]) < MAX_ELAPSE_TIME ) {
					loadmin = loadList[i];
					tmpRescInfo = tmpRescGrpInfo->rescInfo;
					tmp1RescGrpInfo = tmpRescGrpInfo;
				}
			}
		}
		tmpRescGrpInfo = tmpRescGrpInfo->next;
	}
	/* exchange rescInfo with the head */
	tmp1RescGrpInfo->rescInfo = (*rescGrpInfo)->rescInfo;
	(*rescGrpInfo)->rescInfo = tmpRescInfo;

	return 0;
}

/* sortRescByLocation - float LOCAL_HOST resources to the top */
int
sortRescByLocation (rescGrpInfo_t **rescGrpInfo)
{
    rescGrpInfo_t *tmpRescGrpInfo, *tmp1RescGrpInfo;
    rescInfo_t *tmpRescInfo, *tmp1RescInfo;

    tmpRescGrpInfo = *rescGrpInfo;

    /* float CACHE_CL to top */

    while (tmpRescGrpInfo != NULL) {
	int tmpClass, tmp1Class;
        tmpRescInfo = tmpRescGrpInfo->rescInfo;
        if (isLocalHost (tmpRescInfo->rescLoc)) {
	    tmpClass = RescClass[tmpRescInfo->rescClassInx].classType;
            /* find a slot to exchange rescInfo */
            tmp1RescGrpInfo = *rescGrpInfo;
            while (tmp1RescGrpInfo != NULL) {
                if (tmp1RescGrpInfo == tmpRescGrpInfo) break;
                tmp1RescInfo = tmp1RescGrpInfo->rescInfo;
		tmp1Class = RescClass[tmp1RescInfo->rescClassInx].classType;
                if (tmp1Class > tmpClass ||
		 (tmp1Class == tmpClass && 
		  isLocalHost (tmp1RescInfo->rescLoc) == 0)) {
                    tmpRescGrpInfo->rescInfo = tmp1RescInfo;
                    tmp1RescGrpInfo->rescInfo = tmpRescInfo;
                    break;
                }
                tmp1RescGrpInfo = tmp1RescGrpInfo->next;
            }
        }
        tmpRescGrpInfo = tmpRescGrpInfo->next;
    }

    return 0;
}

int
checkCollAccessPerm (rsComm_t *rsComm, char *collection, char *accessPerm)
{
    char accStr[LONG_NAME_LEN];
    char condStr[MAX_NAME_LEN];
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    int status;

    if (collection == NULL || accessPerm == NULL) {
	return SYS_INTERNAL_NULL_INPUT_ERR;
    }

    memset (&genQueryInp, 0, sizeof (genQueryInp));

    snprintf (accStr, LONG_NAME_LEN, "%s", rsComm->clientUser.userName);
    addKeyVal (&genQueryInp.condInput, USER_NAME_CLIENT_KW, accStr);

    snprintf (accStr, LONG_NAME_LEN, "%s", rsComm->clientUser.rodsZone);
    addKeyVal (&genQueryInp.condInput, RODS_ZONE_CLIENT_KW, accStr);

    snprintf (accStr, LONG_NAME_LEN, "%s", accessPerm);
    addKeyVal (&genQueryInp.condInput, ACCESS_PERMISSION_KW, accStr);

    snprintf (condStr, MAX_NAME_LEN, "='%s'", collection);
    addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, condStr);

    addInxIval (&genQueryInp.selectInp, COL_COLL_ID, 1);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

    clearGenQueryInp (&genQueryInp);
    if (status >= 0) {
        freeGenQueryOut (&genQueryOut);
    }

    return (status);
}

int
getDataObjInfo (rsComm_t *rsComm, dataObjInp_t *dataObjInp, 
dataObjInfo_t **dataObjInfoHead,char *accessPerm, int ignoreCondInput)
{
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    int i, status;
    dataObjInfo_t *dataObjInfo;
    char condStr[MAX_NAME_LEN]; 
    char *tmpStr;
    sqlResult_t *dataId, *collId, *replNum, *version, *dataType, *dataSize,
      *rescGroupName, *rescName, *filePath, *dataOwnerName, *dataOwnerZone,
      *replStatus, *statusString, *chksum, *dataExpiry, *dataMapId, 
      *dataComments, *dataCreate, *dataModify, *dataMode, *dataName, *collName;
    char *tmpDataId, *tmpCollId, *tmpReplNum, *tmpVersion, *tmpDataType, 
      *tmpDataSize, *tmpRescGroupName, *tmpRescName, *tmpFilePath, 
      *tmpDataOwnerName, *tmpDataOwnerZone, *tmpReplStatus, *tmpStatusString, 
      *tmpChksum, *tmpDataExpiry, *tmpDataMapId, *tmpDataComments, 
      *tmpDataCreate, *tmpDataModify, *tmpDataMode, *tmpDataName, *tmpCollName;
    char accStr[LONG_NAME_LEN];
    int qcondCnt;
    int writeFlag;

    *dataObjInfoHead = NULL;

    qcondCnt = initDataObjInfoQuery (dataObjInp, &genQueryInp, 
      ignoreCondInput);

    if (qcondCnt < 0) {
	return (qcondCnt);
    }

    /* need to do RESC_NAME_KW here because not all query need this */

    if (ignoreCondInput == 0 && (tmpStr =
      getValByKey (&dataObjInp->condInput, RESC_NAME_KW)) != NULL) {
        snprintf (condStr, NAME_LEN, "='%s'", tmpStr);
        addInxVal (&genQueryInp.sqlCondInp, COL_D_RESC_NAME, condStr);
	qcondCnt++;
    }

    addInxIval (&genQueryInp.selectInp, COL_D_DATA_ID, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_COLL_ID, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_REPL_NUM, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_VERSION, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_TYPE_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_SIZE, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_RESC_GROUP_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_RESC_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_DATA_PATH, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_OWNER_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_OWNER_ZONE, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_REPL_STATUS, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_DATA_STATUS, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_DATA_CHECKSUM, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_EXPIRY, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_MAP_ID, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_COMMENTS, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_CREATE_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_D_MODIFY_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_MODE, 1);

    if (accessPerm != NULL) {
        snprintf (accStr, LONG_NAME_LEN, "%s", rsComm->clientUser.userName);
        addKeyVal (&genQueryInp.condInput, USER_NAME_CLIENT_KW, accStr);

        snprintf (accStr, LONG_NAME_LEN, "%s", rsComm->clientUser.rodsZone);
        addKeyVal (&genQueryInp.condInput, RODS_ZONE_CLIENT_KW, accStr);

        snprintf (accStr, LONG_NAME_LEN, "%s", accessPerm);
        addKeyVal (&genQueryInp.condInput, ACCESS_PERMISSION_KW, accStr);
    }

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

    clearGenQueryInp (&genQueryInp);

    if (status < 0) {
        if (status !=CAT_NO_ROWS_FOUND) {
            rodsLog (LOG_NOTICE,
              "getDataObjInfo: rsGenQuery error, status = %d",
              status);
        }
        return (status);
    }

    if (genQueryOut == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: NULL genQueryOut");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if ((dataOwnerName =
      getSqlResultByInx (genQueryOut, COL_D_OWNER_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_D_OWNER_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataName =
      getSqlResultByInx (genQueryOut, COL_DATA_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_DATA_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((collName =
      getSqlResultByInx (genQueryOut, COL_COLL_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_COLL_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataId = getSqlResultByInx (genQueryOut, COL_D_DATA_ID)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_D_DATA_ID failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((collId = getSqlResultByInx (genQueryOut, COL_D_COLL_ID)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_D_COLL_ID failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((replNum = getSqlResultByInx (genQueryOut, COL_DATA_REPL_NUM)) == 
     NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_DATA_REPL_NUM failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((version = getSqlResultByInx (genQueryOut, COL_DATA_VERSION)) == 
      NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_DATA_VERSION failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataType = getSqlResultByInx (genQueryOut, COL_DATA_TYPE_NAME)) == 
      NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_DATA_TYPE_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataSize = getSqlResultByInx (genQueryOut, COL_DATA_SIZE)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_DATA_SIZE failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((rescGroupName = 
      getSqlResultByInx ( genQueryOut, COL_D_RESC_GROUP_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo:getSqlResultByInx for COL_D_RESC_GROUP_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((rescName = getSqlResultByInx (genQueryOut, COL_D_RESC_NAME)) == 
      NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_D_RESC_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((filePath = getSqlResultByInx (genQueryOut, COL_D_DATA_PATH)) == 
      NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_D_DATA_PATH failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataOwnerZone = 
      getSqlResultByInx (genQueryOut, COL_D_OWNER_ZONE)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_D_OWNER_ZONE failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((replStatus = 
      getSqlResultByInx (genQueryOut, COL_D_REPL_STATUS)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_D_REPL_STATUS failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((statusString = 
      getSqlResultByInx (genQueryOut, COL_D_DATA_STATUS)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_D_DATA_STATUS failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((chksum = 
      getSqlResultByInx (genQueryOut, COL_D_DATA_CHECKSUM)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_D_DATA_CHECKSUM failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataExpiry = 
      getSqlResultByInx (genQueryOut, COL_D_EXPIRY)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_D_EXPIRY failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataMapId =
      getSqlResultByInx (genQueryOut, COL_D_MAP_ID)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_D_MAP_ID failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataComments = 
      getSqlResultByInx (genQueryOut, COL_D_COMMENTS)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_D_COMMENTS failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataCreate = 
      getSqlResultByInx (genQueryOut, COL_D_CREATE_TIME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_D_CREATE_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataModify =
      getSqlResultByInx (genQueryOut, COL_D_MODIFY_TIME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_D_MODIFY_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataMode =
      getSqlResultByInx (genQueryOut, COL_DATA_MODE)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getDataObjInfo: getSqlResultByInx for COL_DATA_MODE failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    writeFlag = getWriteFlag (dataObjInp->openFlags);

   for (i = 0;i < genQueryOut->rowCnt; i++) {
        dataObjInfo = (dataObjInfo_t *) malloc (sizeof (dataObjInfo_t));
        memset (dataObjInfo, 0, sizeof (dataObjInfo_t));

        tmpDataId = &dataId->value[dataId->len * i];
        tmpCollId = &collId->value[collId->len * i];
        tmpReplNum = &replNum->value[replNum->len * i];
        tmpVersion = &version->value[version->len * i];
        tmpDataType = &dataType->value[dataType->len * i];
        tmpDataSize = &dataSize->value[dataSize->len * i];
        tmpRescGroupName = &rescGroupName->value[rescGroupName->len * i];
        tmpRescName = &rescName->value[rescName->len * i];
        tmpFilePath = &filePath->value[filePath->len * i];
        tmpDataOwnerName = &dataOwnerName->value[dataOwnerName->len * i];
        tmpDataOwnerZone = &dataOwnerZone->value[dataOwnerZone->len * i];
        tmpReplStatus = &replStatus->value[replStatus->len * i];
        tmpStatusString = &statusString->value[statusString->len * i];
        tmpChksum = &chksum->value[chksum->len * i];
        tmpDataExpiry = &dataExpiry->value[dataExpiry->len * i];
        tmpDataMapId = &dataMapId->value[dataMapId->len * i];
        tmpDataComments = &dataComments->value[dataComments->len * i];
        tmpDataCreate = &dataCreate->value[dataCreate->len * i];
        tmpDataModify = &dataModify->value[dataModify->len * i];
        tmpDataMode = &dataMode->value[dataMode->len * i];
        tmpDataName = &dataName->value[dataName->len * i];
        tmpCollName = &collName->value[collName->len * i];

        snprintf (dataObjInfo->objPath, MAX_NAME_LEN, "%s/%s",
	  tmpCollName, tmpDataName);
	rstrcpy (dataObjInfo->rescName, tmpRescName, NAME_LEN);
        status = resolveResc (tmpRescName, &dataObjInfo->rescInfo);
	if (status < 0) {
	    rodsLog (LOG_DEBUG,
              "getDataObjInfo: resolveResc error for %s, status = %d",
	      tmpRescName, status);
#if 0	/* this could happen for remote zone resource */
	    return (status);
#endif
	}
	rstrcpy (dataObjInfo->rescGroupName, tmpRescGroupName, NAME_LEN);
	rstrcpy (dataObjInfo->dataType, tmpDataType, NAME_LEN);
	dataObjInfo->dataSize = strtoll (tmpDataSize, 0, 0);
	rstrcpy (dataObjInfo->chksum, tmpChksum, NAME_LEN);
	rstrcpy (dataObjInfo->version, tmpVersion, NAME_LEN);
	rstrcpy (dataObjInfo->filePath, tmpFilePath, MAX_NAME_LEN);
	rstrcpy (dataObjInfo->dataOwnerName, tmpDataOwnerName, NAME_LEN);
	rstrcpy (dataObjInfo->dataOwnerZone, tmpDataOwnerZone, NAME_LEN);
	dataObjInfo->replNum = atoi (tmpReplNum);
	dataObjInfo->replStatus = atoi (tmpReplStatus);
	rstrcpy (dataObjInfo->statusString, tmpStatusString, LONG_NAME_LEN);
	dataObjInfo->dataId = strtoll (tmpDataId, 0, 0);
	dataObjInfo->collId = strtoll (tmpCollId, 0, 0);
	dataObjInfo->dataMapId = atoi (tmpDataMapId);
	rstrcpy (dataObjInfo->dataComments, tmpDataComments, LONG_NAME_LEN);
	rstrcpy (dataObjInfo->dataExpiry, tmpDataExpiry, NAME_LEN);
	rstrcpy (dataObjInfo->dataCreate, tmpDataCreate, NAME_LEN);
	rstrcpy (dataObjInfo->dataModify, tmpDataModify, NAME_LEN);
	rstrcpy (dataObjInfo->dataMode, tmpDataMode, NAME_LEN);
	dataObjInfo->writeFlag = writeFlag;

	queDataObjInfo (dataObjInfoHead, dataObjInfo, 1, 0);
    }
    
    freeGenQueryOut (&genQueryOut);

    return (qcondCnt);
}

int
sortObjInfo (dataObjInfo_t **dataObjInfoHead, 
dataObjInfo_t **currentArchInfo, dataObjInfo_t **currentCacheInfo, 
dataObjInfo_t **oldArchInfo, dataObjInfo_t **oldCacheInfo, 
dataObjInfo_t **downCurrentInfo, dataObjInfo_t **downOldInfo)
{
    dataObjInfo_t *tmpDataObjInfo, *nextDataObjInfo;
    int rescClassInx;
    int topFlag;
    dataObjInfo_t *currentBundleInfo = NULL;
    dataObjInfo_t *oldBundleInfo = NULL;
    dataObjInfo_t *currentCompInfo = NULL;
    dataObjInfo_t *oldCompInfo = NULL;

    *currentArchInfo = *currentCacheInfo = *oldArchInfo = *oldCacheInfo = NULL;
    *downCurrentInfo = *downOldInfo = NULL;

    tmpDataObjInfo = *dataObjInfoHead;

    while (tmpDataObjInfo != NULL) {

        nextDataObjInfo = tmpDataObjInfo->next;
        tmpDataObjInfo->next = NULL;

	    
	if (tmpDataObjInfo->rescInfo == NULL || 
	 tmpDataObjInfo->rescInfo->rodsServerHost == NULL) {
	    topFlag = 0;
	} else if (tmpDataObjInfo->rescInfo->rescStatus == 
	  INT_RESC_STATUS_DOWN) {
	    /* the resource is down */
	    if (tmpDataObjInfo->replStatus > 0) {
		queDataObjInfo (downCurrentInfo, tmpDataObjInfo, 1, 1);
	    } else {
		queDataObjInfo (downOldInfo, tmpDataObjInfo, 1, 1);
	    }
	    tmpDataObjInfo = nextDataObjInfo;
	    continue;
	} else {
	    rodsServerHost_t *rodsServerHost =
	      (rodsServerHost_t *) tmpDataObjInfo->rescInfo->rodsServerHost;
	    
	    if (rodsServerHost->localFlag != LOCAL_HOST) {
	        topFlag = 0;
	    } else {
	        /* queue local host at the head */
	        topFlag = 1;
	    }
	}
	rescClassInx = tmpDataObjInfo->rescInfo->rescClassInx;
        if (tmpDataObjInfo->replStatus > 0) {
            if (RescClass[rescClassInx].classType == ARCHIVAL_CL) {
		queDataObjInfo (currentArchInfo, tmpDataObjInfo, 1, topFlag);
	    } else if (RescClass[rescClassInx].classType == COMPOUND_CL) {
                queDataObjInfo (&currentCompInfo, tmpDataObjInfo, 1, topFlag);
	    } else if (RescClass[rescClassInx].classType == BUNDLE_CL) {
                queDataObjInfo (&currentBundleInfo, tmpDataObjInfo, 1, topFlag);
	    } else {
                queDataObjInfo (currentCacheInfo, tmpDataObjInfo, 1, topFlag);
	    }
	} else {
            if (RescClass[rescClassInx].classType == ARCHIVAL_CL) {
                queDataObjInfo (oldArchInfo, tmpDataObjInfo, 1, topFlag);
            } else if (RescClass[rescClassInx].classType == COMPOUND_CL) {
                queDataObjInfo (&oldCompInfo, tmpDataObjInfo, 1, topFlag);
            } else if (RescClass[rescClassInx].classType == BUNDLE_CL) {
                queDataObjInfo (&oldBundleInfo, tmpDataObjInfo, 1, topFlag);
            } else {
                queDataObjInfo (oldCacheInfo, tmpDataObjInfo, 1, topFlag);
            }
	}
	tmpDataObjInfo = nextDataObjInfo;
    }
    /* combine ArchInfo and CompInfo. COMPOUND_CL before BUNDLE_CL */
    queDataObjInfo (oldArchInfo, oldCompInfo, 0, 0);
    queDataObjInfo (oldArchInfo, oldBundleInfo, 0, 0);
    queDataObjInfo (currentArchInfo, currentCompInfo, 0, 0);
    queDataObjInfo (currentArchInfo, currentBundleInfo, 0, 0);

    return (0);
}

/* sortObjInfoForOpen - Sort the dataObjInfo in dataObjInfoHead for open.
 * If it is for read (writeFlag == 0), discard old copies, then cache first,
 * archval second.
 * If it is for write, (writeFlag > 0), resource in DEST_RESC_NAME_KW first,
 * then current cache, current archival, old cache and old archival.
 */ 
int
sortObjInfoForOpen (rsComm_t *rsComm, dataObjInfo_t **dataObjInfoHead, 
keyValPair_t *condInput, int writeFlag)
{
    dataObjInfo_t *currentArchInfo, *currentCacheInfo, *oldArchInfo, 
     *oldCacheInfo, *downCurrentInfo, *downOldInfo;
    int status = 0;

    sortObjInfo (dataObjInfoHead, &currentArchInfo, &currentCacheInfo,
      &oldArchInfo, &oldCacheInfo, &downCurrentInfo, &downOldInfo);

    *dataObjInfoHead = currentCacheInfo;
    queDataObjInfo (dataObjInfoHead, currentArchInfo, 0, 0);
    if (writeFlag == 0) {
	/* For read only */
	if (*dataObjInfoHead != NULL) {
	    /* we already have a good copy */
	    freeAllDataObjInfo (oldCacheInfo);
	    freeAllDataObjInfo (oldArchInfo);
	} else if (downCurrentInfo != NULL) {
	    /* don't have a good copy */
	    /* the old current copy is not available */
            freeAllDataObjInfo (oldCacheInfo);
            freeAllDataObjInfo (oldArchInfo);
	    status = SYS_RESC_IS_DOWN;
	} else {
	    /* don't have a good copy. use the old one */
            queDataObjInfo (dataObjInfoHead, oldCacheInfo, 0, 0);
            queDataObjInfo (dataObjInfoHead, oldArchInfo, 0, 0);
	}
        freeAllDataObjInfo (downCurrentInfo);
    	freeAllDataObjInfo (downOldInfo);
    } else {	/* write */
	char *rescName; 
        queDataObjInfo (dataObjInfoHead, oldCacheInfo, 0, 0);
        queDataObjInfo (dataObjInfoHead, oldArchInfo, 0, 0);
	if (*dataObjInfoHead == NULL) {
	    /* no working copy. */
	    if (getRescStatus (rsComm, NULL, condInput) == 
	      INT_RESC_STATUS_DOWN) {
                freeAllDataObjInfo (downCurrentInfo);
                freeAllDataObjInfo (downOldInfo);
		status = SYS_RESC_IS_DOWN;
	    } else {
                queDataObjInfo (dataObjInfoHead, downCurrentInfo, 0, 0);
                queDataObjInfo (dataObjInfoHead, downOldInfo, 0, 0);
	    }
	} else {
            freeAllDataObjInfo (downCurrentInfo);
    	    freeAllDataObjInfo (downOldInfo);
            if ((rescName = 
	      getValByKey (condInput, DEST_RESC_NAME_KW)) != NULL ||
	      (rescName = 
	      getValByKey (condInput, DEF_RESC_NAME_KW)) != NULL ||
	      (rescName = 
	      getValByKey (condInput, BACKUP_RESC_NAME_KW)) != NULL) {
	        requeDataObjInfoByResc (dataObjInfoHead, rescName, writeFlag,1);
	    }
	}
    }
    return (status);
}

int
getNumDataObjInfo (dataObjInfo_t *dataObjInfoHead) 
{
    dataObjInfo_t *tmpDataObjInfo;
    int numInfo = 0;

    tmpDataObjInfo = dataObjInfoHead;
    while (tmpDataObjInfo != NULL) {
	numInfo++;
	tmpDataObjInfo = tmpDataObjInfo->next;
    }
    return (numInfo);
}

int
sortDataObjInfoRandom (dataObjInfo_t **dataObjInfoHead)
{
    dataObjInfo_t *myDataObjInfo[50];
    dataObjInfo_t *tmpDataObjInfo;
    int i, j, tmpCnt, order;
    int numInfo = getNumDataObjInfo (*dataObjInfoHead);

    if (numInfo <= 1) {
        return (0);
    }

    if (numInfo > 50) {
        rodsLog (LOG_NOTICE,
          "sortDataObjInfoRandom: numInfo %d > 50, setting it to 50", numInfo);
        numInfo = 50;
    }

    memset (myDataObjInfo, 0, numInfo * sizeof (rescGrpInfo_t *));
    /* fill the array randomly */

    tmpCnt = numInfo;
    tmpDataObjInfo = *dataObjInfoHead;
    while (tmpDataObjInfo != NULL) {
        if (tmpCnt > 1) {
            order = random() % tmpCnt;
        } else {
            order = 0;
        }
        for (i = 0, j = 0; i < numInfo; i ++) {
            if (myDataObjInfo[i] == NULL) {
                if (order <= j) {
                    myDataObjInfo[i] = tmpDataObjInfo;
                    break;
                }
                j ++;
            }
        }
        tmpCnt --;
        tmpDataObjInfo = tmpDataObjInfo->next;
    }

    /* now que the array in order */

    *dataObjInfoHead = NULL;
    for (i = 0; i < numInfo; i ++) {
        queDataObjInfo (dataObjInfoHead, myDataObjInfo[i], 1, 1);
    }

    return (0);
}

/* requeDataObjInfoByResc - requeue the dataObjInfo in the 
 * dataObjInfoHead by putting dataObjInfo stored in preferredResc
 * at the top of the queue.
 * return 0 if dataObjInfo with preferredResc exiists.
 * Otherwise, return -1.
 */

int
requeDataObjInfoByResc (dataObjInfo_t **dataObjInfoHead, 
char *preferredResc, int writeFlag, int topFlag)
{
    dataObjInfo_t *tmpDataObjInfo, *prevDataObjInfo;
    int status = -1;

    if (preferredResc == NULL || *dataObjInfoHead == NULL) {
	return (0);
    }

    tmpDataObjInfo = *dataObjInfoHead;
    if (tmpDataObjInfo->next == NULL) {
	/* just one */
	if (strcmp (preferredResc, tmpDataObjInfo->rescInfo->rescName) 
	  == 0) {
	    return (0);
	} else {
	    return (-1);
	}
    }
    prevDataObjInfo = NULL;
    while (tmpDataObjInfo != NULL) {
	if (tmpDataObjInfo->rescInfo != NULL) {
	    if (strcmp (preferredResc, tmpDataObjInfo->rescInfo->rescName) 
	      == 0 || strcmp (preferredResc, tmpDataObjInfo->rescGroupName) 
	      == 0) {
		if (writeFlag > 0 || tmpDataObjInfo->replStatus > 0) {
		    if (prevDataObjInfo != NULL) {
		        prevDataObjInfo->next = tmpDataObjInfo->next;
			queDataObjInfo (dataObjInfoHead, tmpDataObjInfo, 1,
			 topFlag);
		    }
		    if (topFlag > 0) {		    
		        return (0);
		    } else {
			status = 0;
		    }
		}
	    }
	}
	prevDataObjInfo = tmpDataObjInfo;
	tmpDataObjInfo = tmpDataObjInfo->next;
    }

    return (status);
}

int
requeDataObjInfoByReplNum (dataObjInfo_t **dataObjInfoHead, int replNum)
{
    dataObjInfo_t *tmpDataObjInfo, *prevDataObjInfo;
    int status = -1;

    if (dataObjInfoHead == NULL || *dataObjInfoHead == NULL) {
	return (-1);
    }

    tmpDataObjInfo = *dataObjInfoHead;
    if (tmpDataObjInfo->next == NULL) {
	/* just one */
	if (replNum == tmpDataObjInfo->replNum) {
	    return (0);
	} else {
	    return (-1);
	}
    }
    prevDataObjInfo = NULL;
    while (tmpDataObjInfo != NULL) {
        if (replNum == tmpDataObjInfo->replNum) {
            if (prevDataObjInfo != NULL) {
                prevDataObjInfo->next = tmpDataObjInfo->next;
                queDataObjInfo (dataObjInfoHead, tmpDataObjInfo, 1, 1);
	    }
	    status = 0;
	    break;
	}
	prevDataObjInfo = tmpDataObjInfo;
	tmpDataObjInfo = tmpDataObjInfo->next;
    }

    return (status);
}

dataObjInfo_t *
chkCopyInResc (dataObjInfo_t *dataObjInfoHead, rescGrpInfo_t *myRescGrpInfo)
{
    rescGrpInfo_t *tmpRescGrpInfo;
    rescInfo_t *tmpRescInfo;
    dataObjInfo_t *tmpDataObjInfo;

    tmpDataObjInfo = dataObjInfoHead;
    while (tmpDataObjInfo != NULL) {
        tmpRescGrpInfo = myRescGrpInfo;
        while (tmpRescGrpInfo != NULL) {
            tmpRescInfo = tmpRescGrpInfo->rescInfo;
	    if (strcmp (tmpDataObjInfo->rescInfo->rescName,
	      tmpRescInfo->rescName) == 0) { 
		return (tmpDataObjInfo);
	    }
	    tmpRescGrpInfo = tmpRescGrpInfo->next;
	}
	tmpDataObjInfo = tmpDataObjInfo->next;
    }
    return (NULL);
}

/* matchAndTrimRescGrp - check for matching rescName in dataObjInfoHead
 * and rescGrpInfoHead. If there is a match, unlink and free the 
 * rescGrpInfo in rescGrpInfoHead so that they wont be replicated 
 * If trimjFlag - set what to trim. Valid input are : TRIM_MATCHED_RESC_INFO, 
 * TRIM_MATCHED_OBJ_INFO and TRIM_UNMATCHED_OBJ_INFO
 */

int
matchAndTrimRescGrp (dataObjInfo_t **dataObjInfoHead, 
rescGrpInfo_t **rescGrpInfoHead, int trimjFlag, 
dataObjInfo_t **trimmedDataObjInfo)
{
    rescGrpInfo_t *tmpRescGrpInfo;
    rescGrpInfo_t *prevRescGrpInfo;
    rescInfo_t *tmpRescInfo;
    dataObjInfo_t *tmpDataObjInfo, *prevDataObjInfo, *nextDataObjInfo;
    int matchFlag; 
    char rescGroupName[NAME_LEN];

    if (trimmedDataObjInfo != NULL) *trimmedDataObjInfo = NULL;

    if (*rescGrpInfoHead != NULL) {
	rstrcpy (rescGroupName, (*rescGrpInfoHead)->rescGroupName, NAME_LEN);
    } else {
	rescGroupName[0] = '\0';
    } 

    tmpDataObjInfo = *dataObjInfoHead;
    prevDataObjInfo = NULL;
    while (tmpDataObjInfo != NULL) {
	matchFlag = 0;
	nextDataObjInfo = tmpDataObjInfo->next;
        tmpRescGrpInfo = *rescGrpInfoHead;
	prevRescGrpInfo = NULL;
        while (tmpRescGrpInfo != NULL) {
            tmpRescInfo = tmpRescGrpInfo->rescInfo;
            if (strcmp (tmpDataObjInfo->rescInfo->rescName,
              tmpRescInfo->rescName) == 0) {
		matchFlag = 1;
		break;
            } 
	    prevRescGrpInfo = tmpRescGrpInfo;
            tmpRescGrpInfo = tmpRescGrpInfo->next;
        }
	if (matchFlag == 1) {
            if (trimjFlag & TRIM_MATCHED_RESC_INFO) {
                if (tmpRescGrpInfo == *rescGrpInfoHead) {
                    *rescGrpInfoHead = tmpRescGrpInfo->next;
                } else {
                    prevRescGrpInfo->next = tmpRescGrpInfo->next;
                }
                free (tmpRescGrpInfo);
            } else if (trimjFlag & REQUE_MATCHED_RESC_INFO) {
		if (tmpRescGrpInfo->next != NULL) {
		    /* queue to bottom */
                    if (tmpRescGrpInfo == *rescGrpInfoHead) {
                        *rescGrpInfoHead = tmpRescGrpInfo->next;
                    } else {
                        prevRescGrpInfo->next = tmpRescGrpInfo->next;
                    }
		    queRescGrp (rescGrpInfoHead, tmpRescGrpInfo, BOTTOM_FLAG);
		}
	    }

            if (trimjFlag & TRIM_MATCHED_OBJ_INFO) {
                if (tmpDataObjInfo == *dataObjInfoHead) {
                    *dataObjInfoHead = tmpDataObjInfo->next;
                } else {
                    prevDataObjInfo->next = tmpDataObjInfo->next;
                }
                if (trimmedDataObjInfo != NULL) {
                    queDataObjInfo (trimmedDataObjInfo, tmpDataObjInfo, 1, 0);
                } else {
                    free (tmpDataObjInfo); 
		}
            } else {
		prevDataObjInfo = tmpDataObjInfo;
	    }
	} else {
	    /* no match */
	    if (trimjFlag & TRIM_UNMATCHED_OBJ_INFO ||
	      ((trimjFlag & TRIM_MATCHED_OBJ_INFO) && 
	      strlen (rescGroupName) > 0 &&
	      strcmp (tmpDataObjInfo->rescGroupName, rescGroupName) == 0)) {
		/* take it out */
                if (tmpDataObjInfo == *dataObjInfoHead) {
                    *dataObjInfoHead = tmpDataObjInfo->next;
                } else {
                    prevDataObjInfo->next = tmpDataObjInfo->next;
                }
                if (trimmedDataObjInfo != NULL) {
                    queDataObjInfo (trimmedDataObjInfo, tmpDataObjInfo, 1, 0);
                } else {
		    free (tmpDataObjInfo);
		}
	    } else {
	        prevDataObjInfo = tmpDataObjInfo;
	    }
	}
        tmpDataObjInfo = nextDataObjInfo;
    }

    return (0);
}

/* sortObjInfoForRepl - sort the data object given in dataObjInfoHead.
 * Put the current copies in dataObjInfoHead. Delete old copies if
 * deleteOldFlag allowed or put them in oldDataObjInfoHead for further
 * process if not allowed.
 */

int
sortObjInfoForRepl (dataObjInfo_t **dataObjInfoHead, 
dataObjInfo_t **oldDataObjInfoHead, int deleteOldFlag)
{
    dataObjInfo_t *currentArchInfo, *currentCacheInfo, *oldArchInfo,
     *oldCacheInfo, *downCurrentInfo, *downOldInfo;
    sortObjInfo (dataObjInfoHead, &currentArchInfo, &currentCacheInfo,
      &oldArchInfo, &oldCacheInfo, &downCurrentInfo, &downOldInfo);

    freeAllDataObjInfo (downOldInfo);
    *dataObjInfoHead = currentCacheInfo;
    queDataObjInfo (dataObjInfoHead, currentArchInfo, 0, 0);
    queDataObjInfo (dataObjInfoHead, downCurrentInfo, 0, 0);
    if (*dataObjInfoHead != NULL) {
        if (deleteOldFlag == 0) {  
	    /* multi copy not allowed. have to keep old copy in  
	     * oldDataObjInfoHead for further processing */
            *oldDataObjInfoHead = oldCacheInfo;
            queDataObjInfo (oldDataObjInfoHead, oldArchInfo, 0, 0);
	} else {
            freeAllDataObjInfo (oldCacheInfo);
            freeAllDataObjInfo (oldArchInfo);
	}
    } else {
        queDataObjInfo (dataObjInfoHead, oldCacheInfo, 0, 0);
        queDataObjInfo (dataObjInfoHead, oldArchInfo, 0, 0);
    }
    if (*dataObjInfoHead == NULL) 
	return SYS_RESC_IS_DOWN;
    else
        return (0);
}

/* dataObjExist - check whether the data object given in dataObjInp exist.
   Returns 1 if exists, 0 ==> does not exist
 */

int
dataObjExist (rsComm_t *rsComm, dataObjInp_t *dataObjInp)
{
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    int status;

    status = initDataObjInfoQuery (dataObjInp, &genQueryInp, 0);

    if (status < 0) {
        return (status);
    }

    addInxIval (&genQueryInp.selectInp, COL_D_DATA_ID, 1);
    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

    clearGenQueryInp (&genQueryInp);
    freeGenQueryOut (&genQueryOut);

    if (status < 0) {
	return (0);
    } else {
	return (1);
    }
}

/* initDataObjInfoQuery - initialize the genQueryInp based on dataObjInp.
 * returns the qcondCnt - count of sqlCondInp based on condition input.
 */
 	 
int
initDataObjInfoQuery (dataObjInp_t *dataObjInp, genQueryInp_t *genQueryInp,
int ignoreCondInput)
{
    char myColl[MAX_NAME_LEN], myData[MAX_NAME_LEN];
    char condStr[MAX_NAME_LEN];
    char *tmpStr;
    int status;
    int qcondCnt = 0;

    memset (genQueryInp, 0, sizeof (genQueryInp_t));

    if ((tmpStr = getValByKey (&dataObjInp->condInput, QUERY_BY_DATA_ID_KW)) 
      == NULL) {
        memset (myColl, 0, MAX_NAME_LEN);
        memset (myData, 0, MAX_NAME_LEN);

        if ((status = splitPathByKey (
          dataObjInp->objPath, myColl, myData, '/')) < 0) {
            rodsLog (LOG_NOTICE,
              "initDataObjInfoQuery: splitPathByKey for %s error, status = %d",
              dataObjInp->objPath, status);
            return (status);
        }
        snprintf (condStr, MAX_NAME_LEN, "='%s'", myColl);
        addInxVal (&genQueryInp->sqlCondInp, COL_COLL_NAME, condStr);
        snprintf (condStr, MAX_NAME_LEN, "='%s'", myData);
        addInxVal (&genQueryInp->sqlCondInp, COL_DATA_NAME, condStr);
    } else {
        snprintf (condStr, MAX_NAME_LEN, "='%s'", tmpStr);
        addInxVal (&genQueryInp->sqlCondInp, COL_D_DATA_ID, condStr);
    }

    if (ignoreCondInput == 0 && (tmpStr =
      getValByKey (&dataObjInp->condInput, REPL_NUM_KW)) != NULL) {
        snprintf (condStr, NAME_LEN, "='%s'", tmpStr);
        addInxVal (&genQueryInp->sqlCondInp, COL_DATA_REPL_NUM, condStr);
	qcondCnt++;
    }

    return (qcondCnt);
}

/* chkOrphanFile - check whether a filePath is a orphan file. 
 *    return - 1 - the file is orphan.
 *	       0 - 0 the file is not an orphan.
 *	       -ive - query error.
 *	       If it is not orphan and dataObjInfo != NULL, output ObjID, 
 *	       replNum and objPath to dataObjInfo.
 */

int
chkOrphanFile (rsComm_t *rsComm, char *filePath, char *rescName,
dataObjInfo_t *dataObjInfo)
{
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    int status;
    char condStr[MAX_NAME_LEN];

    memset (&genQueryInp, 0, sizeof (genQueryInp_t));

    snprintf (condStr, MAX_NAME_LEN, "='%s'", filePath);
    addInxVal (&genQueryInp.sqlCondInp, COL_D_DATA_PATH, condStr);
    snprintf (condStr, MAX_NAME_LEN, "='%s'", rescName);
    addInxVal (&genQueryInp.sqlCondInp, COL_D_RESC_NAME, condStr);

    addInxIval (&genQueryInp.selectInp, COL_D_DATA_ID, 1);
    addInxIval (&genQueryInp.selectInp, COL_COLL_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_REPL_NUM, 1);
    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

    if (status < 0) {
	if (status == CAT_NO_ROWS_FOUND) {
            rsComm->perfStat.orphanCnt ++;
	    return (1);
	} else {
            rodsLog (LOG_ERROR,
             "chkOrphanFile: rsGenQuery error for %s, status = %d",
             filePath, status);
	    /* we have unexpected query error. Assume the file is not
	     * orphan */ 
	    return (status);
	}
    } else {
        sqlResult_t *dataId, *replNum, *dataName, *collName;
	rsComm->perfStat.nonOrphanCnt ++;

        if ((collName =
          getSqlResultByInx (genQueryOut, COL_COLL_NAME)) == NULL) {
            rodsLog (LOG_NOTICE,
              "chkOrphanFile: getSqlResultByInx for COL_COLL_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        if ((dataName = getSqlResultByInx (genQueryOut, COL_DATA_NAME)) 
	  == NULL) {
            rodsLog (LOG_NOTICE,
              "chkOrphanFile: getSqlResultByInx for COL_DATA_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        if ((dataId = getSqlResultByInx (genQueryOut, COL_D_DATA_ID)) == NULL) {
            rodsLog (LOG_NOTICE,
              "chkOrphanFile: getSqlResultByInx for COL_D_DATA_ID failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        if ((replNum = getSqlResultByInx (genQueryOut, COL_DATA_REPL_NUM)) ==
         NULL) {
            rodsLog (LOG_NOTICE,
              "chkOrphanFile: getSqlResultByInx for COL_DATA_REPL_NUM failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

	if (dataObjInfo != NULL) {
	    dataObjInfo->dataId = strtoll (dataId->value, 0, 0);
	    dataObjInfo->replNum = atoi (replNum->value);
	    snprintf (dataObjInfo->objPath, MAX_NAME_LEN, "%s/%s",
	      collName->value, dataName->value);
        }

        clearGenQueryInp (&genQueryInp);
        freeGenQueryOut (&genQueryOut);

	return (0);
    }
}

/* chkOrphanDir - check whether a filePath is a orphan directory.
 *    return - 1 - the dir is orphan.
 *             0 - 0 the dir is not an orphan.
 *             -ive - query error.
 */

int
chkOrphanDir (rsComm_t *rsComm, char *dirPath, char *rescName)
{
    DIR *dirPtr;
    struct dirent *myDirent;
    struct stat statbuf;
    char subfilePath[MAX_NAME_LEN];
    int savedStatus = 1;
    int status = 0;

    dirPtr = opendir (dirPath);
    if (dirPtr == NULL) {
        rodsLog (LOG_ERROR,
        "chkOrphanDir: opendir error for %s, errno = %d",
         dirPath, errno);
        return (UNIX_FILE_OPENDIR_ERR - errno);
    }
    while ((myDirent = readdir (dirPtr)) != NULL) {
        if (strcmp (myDirent->d_name, ".") == 0 ||
          strcmp (myDirent->d_name, "..") == 0) {
            continue;
        }
        snprintf (subfilePath, MAX_NAME_LEN, "%s/%s",
          dirPath, myDirent->d_name);

        status = stat (subfilePath, &statbuf);

        if (status != 0) {
            rodsLog (LOG_ERROR,
              "chkOrphanDir: stat error for %s, errno = %d",
              subfilePath, errno);
	    savedStatus = UNIX_FILE_STAT_ERR - errno;
            continue;
        }
        if ((statbuf.st_mode & S_IFDIR) != 0) {
	    status = chkOrphanDir (rsComm, subfilePath, rescName);
	} else if ((statbuf.st_mode & S_IFREG) != 0) {
	    status = chkOrphanFile (rsComm, subfilePath, rescName, NULL);
	}
	if (status == 0) {
	     /* not orphan */
	    closedir (dirPtr);
	    return status;    
	} else if (status < 0) {
	    savedStatus = status;
	}
    }
    closedir (dirPtr);
    return savedStatus;
}

int 
getReInfo (rsComm_t *rsComm, genQueryOut_t **genQueryOut)
{
    genQueryInp_t genQueryInp;
    int status;

    *genQueryOut = NULL;
    memset (&genQueryInp, 0, sizeof (genQueryInp_t));

    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_ID, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_REI_FILE_PATH, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_USER_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_ADDRESS, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_FREQUENCY, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_PRIORITY, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_ESTIMATED_EXE_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_NOTIFICATION_ADDR, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_LAST_EXE_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_STATUS, 1);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, &genQueryInp, genQueryOut);

    clearGenQueryInp (&genQueryInp);
    /* take care of mem leak */
    if (status < 0 && *genQueryOut != NULL) {
	free (*genQueryOut);
	*genQueryOut = NULL;
    }
    return (status);
}

int 
getReInfoById (rsComm_t *rsComm, char *ruleExecId, genQueryOut_t **genQueryOut)
{
    genQueryInp_t genQueryInp;
    char tmpStr[NAME_LEN];
    int status;

    memset (&genQueryInp, 0, sizeof (genQueryInp_t));

    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_ID, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_REI_FILE_PATH, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_USER_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_ADDRESS, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_FREQUENCY, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_PRIORITY, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_ESTIMATED_EXE_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_LAST_EXE_TIME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RULE_EXEC_STATUS, 1);

    snprintf (tmpStr, NAME_LEN, "='%s'", ruleExecId);
    addInxVal (&genQueryInp.sqlCondInp, COL_RULE_EXEC_ID, tmpStr);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, &genQueryInp, genQueryOut);

    clearGenQueryInp (&genQueryInp);

    return (status);
}

/* getNextQueuedRuleExec - get the next RuleExec in queue to run
 * jobType -   0 - run exeStatus = RE_IN_QUEUE and RE_RUNNING
 * 		  RE_FAILED_STATUS -  run the RE_FAILED too
 */
  
int
getNextQueuedRuleExec (rsComm_t *rsComm, genQueryOut_t **inGenQueryOut, 
int startInx, ruleExecSubmitInp_t *queuedRuleExec, 
reExec_t *reExec, int jobType)
{
    sqlResult_t *ruleExecId, *ruleName, *reiFilePath, *userName, *exeAddress,
      *exeTime, *exeFrequency, *priority, *lastExecTime, *exeStatus,
      *estimateExeTime, *notificationAddr;
    int i, status;
    genQueryOut_t *genQueryOut;

    if (inGenQueryOut == NULL || *inGenQueryOut == NULL ||
      queuedRuleExec == NULL || queuedRuleExec->packedReiAndArgBBuf == NULL ||
      queuedRuleExec->packedReiAndArgBBuf->buf == NULL) {
        rodsLog (LOG_ERROR,
          "getNextQueuedRuleExec: NULL input");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    genQueryOut = *inGenQueryOut;
    if ((ruleExecId = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_ID)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for EXEC_ID failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((ruleName = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for EXEC_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((reiFilePath = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_REI_FILE_PATH)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for REI_FILE_PATH failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((userName = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_USER_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for USER_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((exeAddress = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_ADDRESS)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for EXEC_ADDRESS failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((exeTime = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_TIME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for EXEC_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((exeFrequency = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_FREQUENCY)) == NULL) {
        rodsLog (LOG_NOTICE,
         "getNextQueuedRuleExec:getResultByInx for RULE_EXEC_FREQUENCY failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((priority = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_PRIORITY)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for PRIORITY failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((lastExecTime = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_LAST_EXE_TIME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for LAST_EXE_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((exeStatus = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_STATUS)) == NULL) {
        rodsLog (LOG_NOTICE,
          "getNextQueuedRuleExec: getSqlResultByInx for EXEC_STATUS failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((estimateExeTime = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_ESTIMATED_EXE_TIME)) == NULL) {
        rodsLog (LOG_NOTICE,
         "getNextQueuedRuleExec: getResultByInx for ESTIMATED_EXE_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((notificationAddr = getSqlResultByInx (genQueryOut,
     COL_RULE_EXEC_NOTIFICATION_ADDR)) == NULL) {
        rodsLog (LOG_NOTICE,
         "getNextQueuedRuleExec:getResultByInx for NOTIFICATION_ADDR failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    for (i = startInx; i < genQueryOut->rowCnt; i++) {
	char *exeStatusStr, *exeTimeStr, *ruleExecIdStr;
        struct stat statbuf;
        int fd;
	

	exeStatusStr = &exeStatus->value[exeStatus->len * i];
	exeTimeStr = &exeTime->value[exeTime->len * i];
	ruleExecIdStr =  &ruleExecId->value[ruleExecId->len * i];
	
	if ((jobType & RE_FAILED_STATUS) == 0 && 
	  strcmp (exeStatusStr, RE_FAILED) == 0) {
	    /* failed request */
	    continue;
	} else if (atoi (exeTimeStr) > time (0)) {
	    /* not time yet */
	    continue;
        } else if (strcmp (exeStatusStr, RE_RUNNING) == 0) {
            /* is already running */
	    if (reExec->maxRunCnt > 1 &&	/* multiProc */
	     matchRuleExecId (reExec, ruleExecIdStr, RE_PROC_RUNNING)) {
	        /* the job is running in multiProc env */
		continue;
	    } else {
                rodsLog (LOG_NOTICE,
                 "getNextQueuedRuleExec: reId %s in RUNNING state. Run again",
                  ruleExecIdStr);
	    }
	}

        rstrcpy (queuedRuleExec->reiFilePath,
          &reiFilePath->value[reiFilePath->len * i], MAX_NAME_LEN);
	if (stat (queuedRuleExec->reiFilePath, &statbuf) < 0) {
	    status = UNIX_FILE_STAT_ERR - errno;
	    rodsLog (LOG_ERROR,
             "getNextQueuedRuleExec: stat error for rei file %s, status = %d",
	     queuedRuleExec->reiFilePath, status);
	    continue;
	}

        if (statbuf.st_size > queuedRuleExec->packedReiAndArgBBuf->len) {
	    free (queuedRuleExec->packedReiAndArgBBuf->buf);
	    queuedRuleExec->packedReiAndArgBBuf->buf = 
	      malloc ((int) statbuf.st_size);
	    queuedRuleExec->packedReiAndArgBBuf->len = statbuf.st_size;
	}

	fd = open (queuedRuleExec->reiFilePath, O_RDONLY,0);
	if (fd < 0) {
            status = UNIX_FILE_OPEN_ERR - errno;
            rodsLog (LOG_ERROR,
             "getNextQueuedRuleExec: open error for rei file %s, status = %d",
             queuedRuleExec->reiFilePath, status);
            return (status);
        }
 
	status = read (fd, queuedRuleExec->packedReiAndArgBBuf->buf, 
	  queuedRuleExec->packedReiAndArgBBuf->len);

	close (fd);
        if (status != statbuf.st_size) {
	    if (status < 0) {
                status = UNIX_FILE_READ_ERR - errno;
                rodsLog (LOG_ERROR,
                 "getNextQueuedRuleExec: read error for file %s, status = %d",
                 queuedRuleExec->reiFilePath, status);
	    } else {
                rodsLog (LOG_ERROR,
                 "getNextQueuedRuleExec:read error for %s,toRead %d, read %d",
                  queuedRuleExec->reiFilePath, 
	          queuedRuleExec->packedReiAndArgBBuf->len, status);
                return (SYS_COPY_LEN_ERR);
	    }
        }

	rstrcpy (queuedRuleExec->exeTime, exeTimeStr, NAME_LEN);
	rstrcpy (queuedRuleExec->exeStatus, exeStatusStr, NAME_LEN);
        rstrcpy (queuedRuleExec->ruleExecId, ruleExecIdStr, NAME_LEN);

	rstrcpy (queuedRuleExec->ruleName, 
	  &ruleName->value[ruleName->len * i], META_STR_LEN);
	rstrcpy (queuedRuleExec->userName, 
	  &userName->value[userName->len * i], NAME_LEN);
	rstrcpy (queuedRuleExec->exeAddress, 
	  &exeAddress->value[exeAddress->len * i], NAME_LEN);
	rstrcpy (queuedRuleExec->exeFrequency, 
	  &exeFrequency->value[exeFrequency->len * i], NAME_LEN);
	rstrcpy (queuedRuleExec->priority, 
	  &priority->value[priority->len * i], NAME_LEN);
	rstrcpy (queuedRuleExec->estimateExeTime, 
	  &estimateExeTime->value[estimateExeTime->len * i], NAME_LEN);
	rstrcpy (queuedRuleExec->notificationAddr, 
	  &notificationAddr->value[notificationAddr->len * i], NAME_LEN);
	return (i);
    }
    return (-1);
}

int
modExeInfoForRepeat(rsComm_t *rsComm, char *ruleExecId, char* pastTime, 
		      char *delay, int opStatus) {
    keyValPair_t *regParam;
    int status, status1;
    char myTimeNow[200];
    char myTimeNext[200];
	ruleExecModInp_t ruleExecModInp;
    ruleExecDelInp_t ruleExecDelInp;

    if (opStatus > 0) opStatus = 0;
  
    rstrcpy (myTimeNext, pastTime, 200);
    getOffsetTimeStr((char*)&myTimeNow, "                      ");

    status1 = getNextRepeatTime(myTimeNow, delay,myTimeNext);

    /***
    if (status != 0)
      return(status);
    rDelay = (atol(delay) * 60)  + atol(myTimeNow);
    sprintf(myTimeNext,"%lld", rDelay);
    ***/
    rodsLog (LOG_NOTICE,"modExeInfoForRepeat: rulId=%s,opStatus=%d,nextRepeatStatus=%d",ruleExecId,opStatus,status1);
    regParam = &(ruleExecModInp.condInput);
    rstrcpy (ruleExecModInp.ruleId, ruleExecId, NAME_LEN);
    memset (regParam, 0, sizeof (keyValPair_t));
    if (status1 == 0) {
      addKeyVal (regParam, RULE_EXE_STATUS_KW, "");
      addKeyVal (regParam, RULE_LAST_EXE_TIME_KW, myTimeNow);
      addKeyVal (regParam, RULE_EXE_TIME_KW, myTimeNext);
      status = rsRuleExecMod(rsComm, &ruleExecModInp);
    }
    else if (status1 == 1) {
      if (opStatus == 0) {
	/* entry remove  */
	rstrcpy (ruleExecDelInp.ruleExecId, ruleExecId, NAME_LEN);
	status = rsRuleExecDel (rsComm, &ruleExecDelInp);
      }
      else {
	addKeyVal (regParam, RULE_EXE_STATUS_KW, "");
	addKeyVal (regParam, RULE_LAST_EXE_TIME_KW, myTimeNow);
	addKeyVal (regParam, RULE_EXE_TIME_KW, myTimeNext);
	status = rsRuleExecMod(rsComm, &ruleExecModInp);
      }
    }
    else if (status1 == 2 ) {
      /* entry remove  */
      rstrcpy (ruleExecDelInp.ruleExecId, ruleExecId, NAME_LEN);
      status = rsRuleExecDel (rsComm, &ruleExecDelInp);
    }
    else if (status1 == 3 ) {
	addKeyVal (regParam, RULE_EXE_STATUS_KW, "");
	addKeyVal (regParam, RULE_LAST_EXE_TIME_KW, myTimeNow);
	addKeyVal (regParam, RULE_EXE_TIME_KW, myTimeNext);
	addKeyVal (regParam, RULE_EXE_FREQUENCY_KW,delay);
	status = rsRuleExecMod(rsComm, &ruleExecModInp);
    }
    else if (status1 == 4 ) {
      if (opStatus == 0) {
	/* entry remove  */
	rstrcpy (ruleExecDelInp.ruleExecId, ruleExecId, NAME_LEN);
	status = rsRuleExecDel (rsComm, &ruleExecDelInp);
      }
      else {
	addKeyVal (regParam, RULE_EXE_STATUS_KW, "");
	addKeyVal (regParam, RULE_LAST_EXE_TIME_KW, myTimeNow);
	addKeyVal (regParam, RULE_EXE_TIME_KW, myTimeNext);
	addKeyVal (regParam, RULE_EXE_FREQUENCY_KW,delay);
	status = rsRuleExecMod(rsComm, &ruleExecModInp);
      }
    }
    if (regParam->len > 0)
      clearKeyVal (regParam); 

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "modExeInfoForRepeat: rsRuleExecMod/rsRuleExecDel Error of id %s failed, status = %d",
          ruleExecId, status);
    }
    else {
      if (status1 == 3 || (status1 != 2 && opStatus != 0))
        rodsLog (LOG_NOTICE,
	      "Rule id %s set to run again at %s (frequency %s seconds)", 
		 ruleExecId, myTimeNext, delay);
    }

    return (status);
}

int
regExeStatus (rsComm_t *rsComm, char *ruleExecId, char *exeStatus)
{
    keyValPair_t *regParam;
    ruleExecModInp_t ruleExecModInp;
    int status;
    /*** RAJA July 24, 2007 changed chl call to rs call ***/
    regParam = &(ruleExecModInp.condInput);
    memset (regParam, 0, sizeof (keyValPair_t));
    rstrcpy (ruleExecModInp.ruleId, ruleExecId, NAME_LEN);
    addKeyVal (regParam, RULE_EXE_STATUS_KW, exeStatus);
    status = rsRuleExecMod(rsComm, &ruleExecModInp);


    clearKeyVal (regParam);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "regExeStatus: rsRuleExecMod of id %s failed, status = %d",
          ruleExecId, status);
    }

    return (status);
}

/* runQueuedRuleExec - given the job queue given in genQueryOut (from
 * a getReInfo call), run the jobs with the input jobType.
 * Valid jobType = 0 ==> normal job. 
 * jobType = RE_FAILED_STATUS ==> job have failed as least once
 */

int
runQueuedRuleExec (rsComm_t *rsComm, reExec_t *reExec, 
genQueryOut_t **genQueryOut, time_t endTime, int jobType)
{
    int inx, status;
    ruleExecSubmitInp_t *myRuleExecInp;
    int runCnt = 0;
    int thrInx;

    inx = -1;
    while (time(NULL) <= endTime && (thrInx = allocReThr (reExec)) >= 0) {
	myRuleExecInp = &reExec->reExecProc[thrInx].ruleExecSubmitInp;
        if ((inx = getNextQueuedRuleExec (rsComm, genQueryOut, inx + 1,
          myRuleExecInp, reExec, jobType)) < 0) {
	    /* no job to run */
	    freeReThr (reExec, thrInx);
	    break;
	} else {
	    reExec->reExecProc[thrInx].jobType = jobType;
	}

        /* mark running */
        status = regExeStatus (rsComm, myRuleExecInp->ruleExecId, 
	  RE_RUNNING);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "runQueuedRuleExec: regExeStatus of id %s failed,stat = %d",
              myRuleExecInp->ruleExecId, status);
	    freeReThr (reExec, thrInx);
            continue;
        }

	runCnt ++;
	if (reExec->maxRunCnt == 1) {
	    /* single thread. Just call runRuleExec */  
	    status = runRuleExec (&reExec->reExecProc[thrInx]);
            postProcRunRuleExec (rsComm, &reExec->reExecProc[thrInx]);
            freeReThr (reExec, thrInx);
            continue;
	} else {
	    if ((reExec->reExecProc[thrInx].pid = fork()) == 0) {
		/* child. need to disconnect Rcat */ 
		rodsServerHost_t *rodsServerHost = NULL;

                if ((status = resetRcatHost (rsComm, MASTER_RCAT,
                 rsComm->myEnv.rodsZone)) == LOCAL_HOST) {
#ifdef RODS_CAT
                    resetRcat (rsComm);
#endif
		}
        	if ((status = getAndConnRcatHost (rsComm, MASTER_RCAT,
                 rsComm->myEnv.rodsZone, &rodsServerHost)) == LOCAL_HOST) {
#ifdef RODS_CAT
                    status = connectRcat (rsComm);
                    if (status < 0) {
                        rodsLog (LOG_ERROR,
                          "runQueuedRuleExec: connectRcat error. status=%d",
			  status);
        	    }
#endif
    		}
		status = runRuleExec (&reExec->reExecProc[thrInx]);
                postProcRunRuleExec (rsComm, &reExec->reExecProc[thrInx]);
#ifdef RE_SERVER_DEBUG
		rodsLog (LOG_NOTICE,
		  "runQueuedRuleExec: process %d exiting", getpid ());
#endif
		if (reExec->reExecProc[thrInx].status >= 0) {
		    exit (0);
		} else {
		    exit (1);
		}
	    } else { 
#ifdef RE_SERVER_DEBUG
		rodsLog (LOG_NOTICE,
		  "runQueuedRuleExec: started proc %d, thrInx %d",
		  reExec->reExecProc[thrInx].pid, thrInx); 
#endif
	        /* parent fall through here */
	        reExec->runCnt++;
	        continue;
	    }
	}
    }
    if (reExec->maxRunCnt > 1) {
	/* wait for all jobs to finish */
	while (reExec->runCnt + 1 >= reExec->maxRunCnt && 
	  waitAndFreeReThr (reExec) >= 0);
    }

    return (runCnt);
}

int
svrCloseQueryOut (rsComm_t *rsComm, genQueryOut_t *genQueryOut)
{
    genQueryInp_t genQueryInp;
    genQueryOut_t *junk = NULL;
    int status;

    if (genQueryOut->continueInx <= 0) {
        return (0);
    }

    memset (&genQueryInp, 0, sizeof (genQueryInp_t));

    /* maxRows = 0 specifies that the genQueryOut should be closed */
    genQueryInp.maxRows = 0;;
    genQueryInp.continueInx = genQueryOut->continueInx;

    status =  rsGenQuery (rsComm, &genQueryInp, &junk);

    return (status);
}

/* resolveSingleReplCopy - given the dataObjInfoHead (up-to-date copies) 
 * and oldDataObjInfoHead (stale copies) and the destRescGrpInfo,
 * sort through the single copy reuirement for repl.
 * If there is a good copy in every resc in the rescGroup, return 
 * HAVE_GOOD_COPY. Otherwise, trim the resc in the rescGroup so only the one 
 * with no copies are left. The copies required to be overwritten are
 * placed in destDataObjInfo. The old dataObjInfo that do not need to
 * be overwritten are placed in oldDataObjInfoHead which may be needed
 * for compound resource staging.
 * A returned value of CAT_NO_ROWS_FOUND mean there is condition in
 * condInput but none in dataObjInfoHead or oldDataObjInfoHead match
 * the condition. i.e., no source for the replication
 */ 
int 
resolveSingleReplCopy ( dataObjInfo_t **dataObjInfoHead, 
dataObjInfo_t **oldDataObjInfoHead, rescGrpInfo_t **destRescGrpInfo,
dataObjInfo_t **destDataObjInfo, keyValPair_t *condInput)
{
    int status;
    dataObjInfo_t *matchedDataObjInfo = NULL;
    dataObjInfo_t *matchedOldDataObjInfo = NULL;

    /* see if dataObjInfoHead and oldDataObjInfoHead matches the condInput */
    status = matchDataObjInfoByCondInput (dataObjInfoHead, oldDataObjInfoHead,
      condInput, &matchedDataObjInfo, &matchedOldDataObjInfo);

    if (status < 0) {
	return status;
    }

    if (matchedDataObjInfo != NULL) {
	/* que the matched one on top */
	queDataObjInfo (dataObjInfoHead, matchedDataObjInfo, 0, 1);
	queDataObjInfo (oldDataObjInfoHead, matchedOldDataObjInfo, 0, 1);
    } else if (matchedOldDataObjInfo != NULL) {
	/* The source of replication is an old copy. Queue dataObjInfoHead 
	 * to oldDataObjInfoHead */ 
	queDataObjInfo (oldDataObjInfoHead, *dataObjInfoHead, 0, 1);
	*dataObjInfoHead = matchedOldDataObjInfo;
    }

    if ((*destRescGrpInfo)->next == NULL ||
      strlen ((*destRescGrpInfo)->rescGroupName) == 0) {
        /* single target resource */
        if ((*destDataObjInfo = chkCopyInResc (*dataObjInfoHead, 
	  *destRescGrpInfo)) != NULL) {
            /* have a good copy already */
            return (HAVE_GOOD_COPY);
	}
    } else {
        /* target resource is a resource group with multi resources */
        matchAndTrimRescGrp (dataObjInfoHead, destRescGrpInfo, 
	  TRIM_MATCHED_RESC_INFO, NULL);
        if (*destRescGrpInfo == NULL) {
            /* have a good copy in all resc in resc group */
            return (HAVE_GOOD_COPY);
        }
    }
    /* handle the old dataObj */
    if (getValByKey (condInput, ALL_KW) != NULL) {
	dataObjInfo_t *trimmedDataObjInfo = NULL;
	/* replicate to all resc. trim the resc that has a match and
	 * the DataObjInfo that does not have a match */ 
	matchAndTrimRescGrp (oldDataObjInfoHead, destRescGrpInfo, 
	  TRIM_MATCHED_RESC_INFO|TRIM_UNMATCHED_OBJ_INFO, &trimmedDataObjInfo);
	*destDataObjInfo = *oldDataObjInfoHead;
	*oldDataObjInfoHead = trimmedDataObjInfo;
    } else {
        *destDataObjInfo = chkCopyInResc (*oldDataObjInfoHead, 
	  *destRescGrpInfo);
        if (*destDataObjInfo != NULL) {
            /* see if there is any resc that is not used */
            matchAndTrimRescGrp (oldDataObjInfoHead, destRescGrpInfo, 
	      TRIM_MATCHED_RESC_INFO, NULL);
            if (*destRescGrpInfo != NULL) {
                /* just creat a new one in myRescGrpInfo */
                *destDataObjInfo = NULL;
            }
	}
    }
    return (NO_GOOD_COPY);
}

int
resolveInfoForPhymv (dataObjInfo_t **dataObjInfoHead,
dataObjInfo_t **oldDataObjInfoHead, rescGrpInfo_t **destRescGrpInfo,
keyValPair_t *condInput, int multiCopyFlag)
{
    int status;
    dataObjInfo_t *matchedDataObjInfo = NULL;
    dataObjInfo_t *matchedOldDataObjInfo = NULL;


    status = matchDataObjInfoByCondInput (dataObjInfoHead, oldDataObjInfoHead,
      condInput, &matchedDataObjInfo, &matchedOldDataObjInfo);

    if (status < 0) {
        return status;
    }

    if (matchedDataObjInfo != NULL) {
	/* put the matched in oldDataObjInfoHead. should not be anything in
	 * oldDataObjInfoHead */
	*oldDataObjInfoHead = *dataObjInfoHead;
	*dataObjInfoHead = matchedDataObjInfo;
    }

    if (multiCopyFlag) {
        matchAndTrimRescGrp (dataObjInfoHead, destRescGrpInfo,
          REQUE_MATCHED_RESC_INFO, NULL);
        matchAndTrimRescGrp (oldDataObjInfoHead, destRescGrpInfo,
          REQUE_MATCHED_RESC_INFO, NULL);
    } else {
        matchAndTrimRescGrp (dataObjInfoHead, destRescGrpInfo, 
          TRIM_MATCHED_RESC_INFO|TRIM_MATCHED_OBJ_INFO, NULL);
        matchAndTrimRescGrp (oldDataObjInfoHead, destRescGrpInfo, 
          TRIM_MATCHED_RESC_INFO, NULL);
    }

    if (*destRescGrpInfo == NULL) {
	if (*dataObjInfoHead == NULL) {
	    return (CAT_NO_ROWS_FOUND);
	} else {
            /* have a good copy in all resc in resc group */
	    rodsLog (LOG_ERROR,
	      "resolveInfoForPhymv: %s already have copy in the resc",
	      (*dataObjInfoHead)->objPath);
            return (SYS_COPY_ALREADY_IN_RESC);
	}
    } else {
	return (0);
    }
}

int
isData(rsComm_t *rsComm, char *objName, rodsLong_t *dataId)
{
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    char tmpStr[MAX_NAME_LEN];
    char logicalEndName[MAX_NAME_LEN];
    char logicalParentDirName[MAX_NAME_LEN];
    int status;

    status = splitPathByKey(objName,
			    logicalParentDirName, logicalEndName, '/');
    memset (&genQueryInp, 0, sizeof (genQueryInp_t));
    snprintf (tmpStr, MAX_NAME_LEN, "='%s'", logicalEndName);
    addInxVal (&genQueryInp.sqlCondInp, COL_DATA_NAME, tmpStr);
    addInxIval (&genQueryInp.selectInp, COL_D_DATA_ID, 1);
    snprintf (tmpStr, MAX_NAME_LEN, "='%s'", logicalParentDirName);
    addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, tmpStr);
    addInxIval (&genQueryInp.selectInp, COL_COLL_ID, 1);
    genQueryInp.maxRows = 2;
    status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
    if (status >= 0) {
        sqlResult_t *dataIdRes;

        if ((dataIdRes = getSqlResultByInx (genQueryOut, COL_D_DATA_ID)) ==
          NULL) {
            rodsLog (LOG_ERROR,
              "isData: getSqlResultByInx for COL_D_DATA_ID failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }
	if (dataId != NULL) {
            *dataId = strtoll (dataIdRes->value, 0, 0);
	}
    }

    clearGenQueryInp (&genQueryInp);
    return(status);
}

int
isColl(rsComm_t *rsComm, char *objName, rodsLong_t *collId)
{
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    char tmpStr[MAX_NAME_LEN];
    int status;

    memset (&genQueryInp, 0, sizeof (genQueryInp_t));
    snprintf (tmpStr, MAX_NAME_LEN, "='%s'", objName);
    addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, tmpStr);
    addInxIval (&genQueryInp.selectInp, COL_COLL_ID, 1);
    genQueryInp.maxRows = 2;
    status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
    if (status >= 0) {
        sqlResult_t *collIdRes;

        if ((collIdRes = getSqlResultByInx (genQueryOut, COL_COLL_ID)) ==
          NULL) {
            rodsLog (LOG_ERROR,
              "isColl: getSqlResultByInx for COL_D_DATA_ID failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

	if (collId != NULL) {
            *collId = strtoll (collIdRes->value, 0, 0);
	}
    }

    clearGenQueryInp (&genQueryInp);
    return(status);
}

int
isUser(rsComm_t *rsComm, char *objName)
{
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    char tmpStr[NAME_LEN];
    int status;

    memset (&genQueryInp, 0, sizeof (genQueryInp_t));
    snprintf (tmpStr, NAME_LEN, "='%s'", objName);
    addInxVal (&genQueryInp.sqlCondInp, COL_USER_NAME, tmpStr);
    addInxIval (&genQueryInp.selectInp, COL_USER_ID, 1);
    genQueryInp.maxRows = 2;
    status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
    clearGenQueryInp (&genQueryInp);
    return(status);
}


int
isResc(rsComm_t *rsComm, char *objName)
{
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    char tmpStr[NAME_LEN];
    int status;

    memset (&genQueryInp, 0, sizeof (genQueryInp_t));
    snprintf (tmpStr, NAME_LEN, "='%s'", objName);
    addInxVal (&genQueryInp.sqlCondInp, COL_R_RESC_NAME, tmpStr);
    addInxIval (&genQueryInp.selectInp, COL_R_RESC_ID, 1);
    genQueryInp.maxRows = 2;
    status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
    clearGenQueryInp (&genQueryInp);
    return(status);
}

int
isMeta(rsComm_t *rsComm, char *objName)
{
    /* needs to be filled in later */
    return(INVALID_OBJECT_TYPE);
}

int
isToken(rsComm_t *rsComm, char *objName)
{
    /* needs to be filled in later */
    return(INVALID_OBJECT_TYPE);
}

int
getObjType(rsComm_t *rsComm, char *objName, char * objType)
{
    if (isData(rsComm, objName, NULL) >= 0)
      strcpy(objType,"-d");
    else if (isColl(rsComm, objName, NULL) >= 0)
      strcpy(objType,"-c");
    else if (isResc(rsComm, objName) == 0)
      strcpy(objType,"-r");
    else if (isUser(rsComm, objName) == 0)
      strcpy(objType,"-u");
    else if (isMeta(rsComm, objName) == 0)
      strcpy(objType,"-m");
    else if (isToken(rsComm, objName) == 0)
      strcpy(objType,"-t");
    else
      return(INVALID_OBJECT_TYPE);
    return (0);
}

int
addAVUMetadataFromKVPairs (rsComm_t *rsComm, char *objName, char *inObjType,
			   keyValPair_t *kVP)
{
  int i,j;
  char  objType[10];
  modAVUMetadataInp_t modAVUMetadataInp;

  bzero (&modAVUMetadataInp, sizeof (modAVUMetadataInp));
  if (strcmp(inObjType,"-1")) {
    strcpy(objType,inObjType);
  }
  else {
    i = getObjType(rsComm, objName,objType);
    if (i < 0)
      return(i);
  }

  modAVUMetadataInp.arg0 = "add";
  for (i = 0; i < kVP->len ; i++) {
     /* Call rsModAVUMetadata to call chlAddAVUMetadata.
        rsModAVUMetadata connects to the icat-enabled server if the
        local host isn't.
     */
    modAVUMetadataInp.arg1 = objType;
    modAVUMetadataInp.arg2 = objName;
    modAVUMetadataInp.arg3 = kVP->keyWord[i];
    modAVUMetadataInp.arg4 = kVP->value[i];
    modAVUMetadataInp.arg5 = "";
    j = rsModAVUMetadata (rsComm, &modAVUMetadataInp);
    if (j < 0)
      return(j);
  }
  return(0);
}

int
rsQueryDataObjInCollReCur (rsComm_t *rsComm, char *collection,
genQueryInp_t *genQueryInp, genQueryOut_t **genQueryOut, char *accessPerm,
int singleFlag)
{
    char collQCond[MAX_NAME_LEN*2];
    int status;
    char accStr[LONG_NAME_LEN];

    if (collection == NULL || genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    memset (genQueryInp, 0, sizeof (genQueryInp_t));

    genAllInCollQCond (collection, collQCond);

    addInxVal (&genQueryInp->sqlCondInp, COL_COLL_NAME, collQCond);

    addInxIval (&genQueryInp->selectInp, COL_D_DATA_ID, 1);
    addInxIval (&genQueryInp->selectInp, COL_COLL_NAME, 1);
    addInxIval (&genQueryInp->selectInp, COL_DATA_NAME, 1);
    if (singleFlag == 0) {
        addInxIval (&genQueryInp->selectInp, COL_DATA_REPL_NUM, 1);
        addInxIval (&genQueryInp->selectInp, COL_D_RESC_NAME, 1);
        addInxIval (&genQueryInp->selectInp, COL_D_DATA_PATH, 1);
    }

    if (accessPerm != NULL) {
        snprintf (accStr, LONG_NAME_LEN, "%s", rsComm->clientUser.userName);
        addKeyVal (&genQueryInp->condInput, USER_NAME_CLIENT_KW, accStr);

        snprintf (accStr, LONG_NAME_LEN, "%s", rsComm->clientUser.rodsZone);
        addKeyVal (&genQueryInp->condInput, RODS_ZONE_CLIENT_KW, accStr);

        snprintf (accStr, LONG_NAME_LEN, "%s", accessPerm);
        addKeyVal (&genQueryInp->condInput, ACCESS_PERMISSION_KW, accStr);
	/* have to set it to 1 because it only check the first one */  
        genQueryInp->maxRows = 1;
    } else {
        genQueryInp->maxRows = MAX_SQL_ROWS;
    } 

    status =  rsGenQuery (rsComm, genQueryInp, genQueryOut);

    return (status);

}

int
rsQueryCollInColl (rsComm_t *rsComm, char *collection,
genQueryInp_t *genQueryInp, genQueryOut_t **genQueryOut)
{
    char collQCond[MAX_NAME_LEN];
    int status;

    if (collection == NULL || genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    memset (genQueryInp, 0, sizeof (genQueryInp_t));

    snprintf (collQCond, MAX_NAME_LEN, "='%s'", collection);

    addInxVal (&genQueryInp->sqlCondInp, COL_COLL_PARENT_NAME, collQCond);

    addInxIval (&genQueryInp->selectInp, COL_COLL_NAME, 1);

    genQueryInp->maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, genQueryInp, genQueryOut);

    return (status);
}

/* matchDataObjInfoByCondInput - given dataObjInfoHead and oldDataObjInfoHead
 * put all DataObjInfo that match condInput into matchedDataObjInfo and
 * matchedOldDataObjInfo. The unmatch one stay in dataObjInfoHead and
 * oldDataObjInfoHead.
 * A CAT_NO_ROWS_FOUND means there is condition in CondInput, but none 
 * in dataObjInfoHead or oldDataObjInfoHead matches the condition 
 */ 

int
matchDataObjInfoByCondInput (dataObjInfo_t **dataObjInfoHead,
dataObjInfo_t **oldDataObjInfoHead, keyValPair_t *condInput,
dataObjInfo_t **matchedDataObjInfo, dataObjInfo_t **matchedOldDataObjInfo)

{
    int replNumCond;
    int replNum;
    int rescCond;
    char *tmpStr, *rescName;
    dataObjInfo_t *prevDataObjInfo, *nextDataObjInfo, *tmpDataObjInfo;

    if (dataObjInfoHead == NULL || *dataObjInfoHead == NULL ||
      oldDataObjInfoHead == NULL || matchedDataObjInfo == NULL ||
      matchedOldDataObjInfo == NULL) {
        rodsLog (LOG_ERROR,
          "requeDataObjInfoByCondInput: NULL dataObjInfo input");
        return (USER__NULL_INPUT_ERR);
    }

    if ((tmpStr = getValByKey (condInput, REPL_NUM_KW)) != NULL) {
	replNum = atoi (tmpStr);
	replNumCond = 1;
    } else {
	replNumCond = 0;
    }

    if ((rescName = getValByKey (condInput, RESC_NAME_KW)) != NULL) {
        rescCond = 1;
    } else {
        rescCond = 0;
    }

    if (replNumCond + rescCond == 0) {
	return (0);
    }

    *matchedDataObjInfo = NULL;
    *matchedOldDataObjInfo = NULL;

    tmpDataObjInfo = *dataObjInfoHead;
    prevDataObjInfo = NULL;
    while (tmpDataObjInfo != NULL) {
	nextDataObjInfo = tmpDataObjInfo->next;
	if (replNumCond == 1 && replNum == tmpDataObjInfo->replNum) {
            if (prevDataObjInfo != NULL) {
                prevDataObjInfo->next = tmpDataObjInfo->next;
	    } else {
		*dataObjInfoHead = (*dataObjInfoHead)->next;
            }
            queDataObjInfo (matchedDataObjInfo, tmpDataObjInfo, 1, 0);
	} else if (rescCond == 1 && 
	  (strcmp (rescName, tmpDataObjInfo->rescGroupName) == 0 ||
	  strcmp (rescName, tmpDataObjInfo->rescName) == 0)) {
            if (prevDataObjInfo != NULL) {
                prevDataObjInfo->next = tmpDataObjInfo->next;
            } else {
                *dataObjInfoHead = (*dataObjInfoHead)->next;
            }
	    /* que single to the bottom */
            queDataObjInfo (matchedDataObjInfo, tmpDataObjInfo, 1, 0);
	} else {
	    prevDataObjInfo = tmpDataObjInfo;
	}
	tmpDataObjInfo = nextDataObjInfo;
    }

    tmpDataObjInfo = *oldDataObjInfoHead;
    prevDataObjInfo = NULL;
    while (tmpDataObjInfo != NULL) {
        nextDataObjInfo = tmpDataObjInfo->next;
        if (replNumCond == 1 && replNum == tmpDataObjInfo->replNum) {
            if (prevDataObjInfo != NULL) {
                prevDataObjInfo->next = tmpDataObjInfo->next;
            } else {
                *oldDataObjInfoHead = (*oldDataObjInfoHead)->next;
            }
            queDataObjInfo (matchedOldDataObjInfo, tmpDataObjInfo, 1, 0);
        } else if (rescCond == 1 &&
          (strcmp (rescName, tmpDataObjInfo->rescGroupName) == 0 ||
          strcmp (rescName, tmpDataObjInfo->rescName)) == 0) {
            if (prevDataObjInfo != NULL) {
                prevDataObjInfo->next = tmpDataObjInfo->next;
            } else {
                *oldDataObjInfoHead = (*oldDataObjInfoHead)->next;
            }
            queDataObjInfo (matchedOldDataObjInfo, tmpDataObjInfo, 1, 0);
        } else {
            prevDataObjInfo = tmpDataObjInfo;
        }
        tmpDataObjInfo = nextDataObjInfo;
    }
    
    if (*matchedDataObjInfo == NULL && *matchedOldDataObjInfo == NULL) {
	return (CAT_NO_ROWS_FOUND);
    } else {
	return (replNumCond + rescCond);
    }
}

int
resolveInfoForTrim (dataObjInfo_t **dataObjInfoHead,
keyValPair_t *condInput)
{
    int i, status;
    dataObjInfo_t *matchedDataObjInfo = NULL;
    dataObjInfo_t *matchedOldDataObjInfo = NULL;
    dataObjInfo_t *oldDataObjInfoHead = NULL;
    dataObjInfo_t *tmpDataObjInfo, *prevDataObjInfo;
    int matchedInfoCnt, unmatchedInfoCnt, matchedOldInfoCnt, 
     unmatchedOldInfoCnt;
    int minCnt;
    char *tmpStr;
    int condFlag;
    int toTrim;

    sortObjInfoForRepl (dataObjInfoHead, &oldDataObjInfoHead, 0);

    status = matchDataObjInfoByCondInput (dataObjInfoHead, &oldDataObjInfoHead,
      condInput, &matchedDataObjInfo, &matchedOldDataObjInfo);

    if (status < 0) {
	freeAllDataObjInfo (*dataObjInfoHead);
	freeAllDataObjInfo (oldDataObjInfoHead);
	*dataObjInfoHead = NULL;
	if (status == CAT_NO_ROWS_FOUND) {
	    return 0;
	} else {
            return status;
	}
    }
    condFlag = status;	/* cond exist if condFlag > 0 */

    if (matchedDataObjInfo == NULL && matchedOldDataObjInfo == NULL) {
	if (dataObjInfoHead != NULL && condFlag == 0) {
	    /* at least have some good copies */
	    /* see if we can trim some old copies */
	    matchedOldDataObjInfo = oldDataObjInfoHead;
	    oldDataObjInfoHead = NULL;
	    /* also trim good copy too since there is no condiftion 12/1/09 */
            matchedDataObjInfo = *dataObjInfoHead;
            *dataObjInfoHead = NULL;
	} else {
	    /* don't trim anything */
            freeAllDataObjInfo (*dataObjInfoHead);
            freeAllDataObjInfo (oldDataObjInfoHead);
            *dataObjInfoHead = NULL;
            return (0);
	}
    }

    matchedInfoCnt = getDataObjInfoCnt (matchedDataObjInfo);
    unmatchedInfoCnt = getDataObjInfoCnt (*dataObjInfoHead);
    unmatchedOldInfoCnt = getDataObjInfoCnt (oldDataObjInfoHead);

    /* free the unmatched one first */

    freeAllDataObjInfo (*dataObjInfoHead);
    freeAllDataObjInfo (oldDataObjInfoHead);
    *dataObjInfoHead = oldDataObjInfoHead = NULL;

    if ((tmpStr = getValByKey (condInput, COPIES_KW)) != NULL) {
	minCnt = atoi (tmpStr);
	if (minCnt <= 0) {
	    minCnt = DEF_MIN_COPY_CNT;
	}
    } else {
	minCnt = DEF_MIN_COPY_CNT;
    }

    toTrim = unmatchedInfoCnt + matchedInfoCnt - minCnt;
    if (toTrim > matchedInfoCnt) {	/* cannot trim more than match */
        toTrim = matchedInfoCnt;
    }

    if (toTrim >= 0) {
        /* trim all old */
        *dataObjInfoHead = matchedOldDataObjInfo;

        /* take some off the bottom - since cache are queued at top. Want
         * to trim them first */
        for (i = 0; i < matchedInfoCnt - toTrim; i++) {
            prevDataObjInfo = NULL;
            tmpDataObjInfo = matchedDataObjInfo;
            while (tmpDataObjInfo != NULL) {
                if (tmpDataObjInfo->next == NULL) {
                    if (prevDataObjInfo == NULL) {
                        matchedDataObjInfo = NULL;
                    } else {
                        prevDataObjInfo->next = NULL;
                    }
                    freeDataObjInfo (tmpDataObjInfo);
                    break;
                }
                prevDataObjInfo = tmpDataObjInfo;
                tmpDataObjInfo = tmpDataObjInfo->next;
            }
        }
        queDataObjInfo (dataObjInfoHead, matchedDataObjInfo, 0, 1);
    } else {
	/* negative toTrim. see if we can trim some matchedOldDataObjInfo */
        freeAllDataObjInfo (matchedDataObjInfo);
	matchedOldInfoCnt = getDataObjInfoCnt (matchedOldDataObjInfo);
	toTrim = matchedOldInfoCnt + unmatchedOldInfoCnt + toTrim;
	if (toTrim > matchedOldInfoCnt)
	    toTrim = matchedOldInfoCnt;

	if (toTrim <= 0) {
	    freeAllDataObjInfo (matchedOldDataObjInfo);
	} else {
            /* take some off the bottom - since cache are queued at top. Want
             * to trim them first */
            for (i = 0; i < matchedOldInfoCnt - toTrim; i++) {
                prevDataObjInfo = NULL;
                tmpDataObjInfo = matchedOldDataObjInfo;
                while (tmpDataObjInfo != NULL) {
                    if (tmpDataObjInfo->next == NULL) {
                        if (prevDataObjInfo == NULL) {
                            matchedOldDataObjInfo = NULL;
                        } else {
                            prevDataObjInfo->next = NULL;
                        }
                        freeDataObjInfo (tmpDataObjInfo);
                        break;
                    }
                    prevDataObjInfo = tmpDataObjInfo;
                    tmpDataObjInfo = tmpDataObjInfo->next;
                }
            }
            queDataObjInfo (dataObjInfoHead, matchedOldDataObjInfo, 0, 1);
	}
    }
    return (0);
}

int
requeDataObjInfoByDestResc (dataObjInfo_t **dataObjInfoHead,
keyValPair_t *condInput, int writeFlag, int topFlag)
{
    char *rescName;
    int status = -1; 

   if ((rescName = getValByKey (condInput, DEST_RESC_NAME_KW)) != NULL || 
      (rescName = getValByKey (condInput, BACKUP_RESC_NAME_KW)) != NULL ||
      (rescName = getValByKey (condInput, DEF_RESC_NAME_KW)) != NULL) { 
	status = requeDataObjInfoByResc (dataObjInfoHead, rescName, 
	  writeFlag, topFlag);
    }
    return (status);
}

int
requeDataObjInfoBySrcResc (dataObjInfo_t **dataObjInfoHead,
keyValPair_t *condInput, int writeFlag, int topFlag)
{
    char *rescName;

      if ((rescName = getValByKey (condInput, RESC_NAME_KW)) != NULL) {
        requeDataObjInfoByResc (dataObjInfoHead, rescName, writeFlag, topFlag);
    }
    return (0);
}

#if 0	/* replaced by resolvePathInSpecColl */
int
resolveSpecColl (rsComm_t *rsComm, dataObjInp_t *dataObjInp, 
dataObjInfo_t **dataObjInfo, int writeFlag)
{
    specCollCache_t *specCollCache;
    specColl_t *cachedSpecColl;
    int status;
    char *accessStr;
    specCollPerm_t specCollPerm;

    if (dataObjInp == NULL) {
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if ((status = getSpecCollCache (rsComm, dataObjInp->objPath, 0,
      &specCollCache)) < 0) {
        return (status);
    } else {
	cachedSpecColl = &specCollCache->specColl;
    }

    if (writeFlag > 0) {
	accessStr = ACCESS_DELETE_OBJECT;
	specCollPerm = WRITE_COLL_PERM;
    } else {
        accessStr = ACCESS_READ_OBJECT;
        specCollPerm = READ__COLL_PERM;
    }

    if (specCollCache->perm < specCollPerm) {
	status = checkCollAccessPerm (rsComm, cachedSpecColl->collection,
	  accessStr);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "resolveSpecColl: checkCollAccessPerm error for %s, stat = %d",
              cachedSpecColl->collection, status);
	    return (status);
        } else {
	    specCollCache->perm = specCollPerm;
	}
    }

    status = specCollSubStat (rsComm, cachedSpecColl, dataObjInp->objPath,
      dataObjInfo);

    if (*dataObjInfo != NULL && getStructFileType ((*dataObjInfo)->specColl) >= 0) {
        dataObjInp->numThreads = NO_THREADING;
    }

    if (status < 0) {
        if (*dataObjInfo != NULL) {
           /* does not exist. return the dataObjInfo anyway */
            return (SYS_SPEC_COLL_OBJ_NOT_EXIST);
        }
        rodsLog (LOG_ERROR,
          "resolveSpecColl: specCollSubStat error for %s, status = %d",
          dataObjInp->objPath, status);
        return (status);
    } else {
        if (*dataObjInfo != NULL) {
	    (*dataObjInfo)->writeFlag = writeFlag;
	}
    }

    return (status);
}
#endif

int
getStructFileType (specColl_t *specColl)
{
    if (specColl == NULL) {
	return (-1);
    }

    if (specColl->collClass == STRUCT_FILE_COLL) {  
	return ((int) specColl->type);
    } else {
	return (-1);
    }
}

int
getDataObjInfoIncSpecColl (rsComm_t *rsComm, dataObjInp_t *dataObjInp, 
dataObjInfo_t **dataObjInfo)
{
    int status, writeFlag;
    specCollPerm_t specCollPerm;

    writeFlag = getWriteFlag (dataObjInp->openFlags);
    if (writeFlag > 0) {
        specCollPerm = WRITE_COLL_PERM;
        if (rsComm->clientUser.authInfo.authFlag <= PUBLIC_USER_AUTH) {
            rodsLog (LOG_NOTICE,
             "getDataObjInfoIncSpecColl:open for write not allowed for user %s",
              rsComm->clientUser.userName);
            return (SYS_NO_API_PRIV);
        }
    } else {
        specCollPerm = READ_COLL_PERM;
    }

    if (dataObjInp->specColl != NULL && 
      dataObjInp->specColl->collClass != NO_SPEC_COLL &&
      dataObjInp->specColl->collClass != LINKED_COLL) {
	/* if it is linked, it already has been resolved */
        status = resolvePathInSpecColl (rsComm, dataObjInp->objPath,
          specCollPerm, 0, dataObjInfo);
        if (status == SYS_SPEC_COLL_OBJ_NOT_EXIST &&
          dataObjInfo != NULL) {
            freeDataObjInfo (*dataObjInfo);
            dataObjInfo = NULL;
        }
    } else if ((status = resolvePathInSpecColl (rsComm, dataObjInp->objPath,
      specCollPerm, 1, dataObjInfo)) >= 0) {
        /* check if specColl in cache. May be able to save one query */
    } else if (getValByKey (&dataObjInp->condInput,
      IRODS_ADMIN_RMTRASH_KW) != NULL &&
      rsComm->proxyUser.authInfo.authFlag == LOCAL_PRIV_USER_AUTH) {
        status = getDataObjInfo (rsComm, dataObjInp, dataObjInfo,
          NULL, 0);
    } else if (writeFlag > 0 && dataObjInp->oprType != REPLICATE_OPR) {
        status = getDataObjInfo (rsComm, dataObjInp, dataObjInfo,
          ACCESS_DELETE_OBJECT, 0);
    } else {
        status = getDataObjInfo (rsComm, dataObjInp, dataObjInfo,
          ACCESS_READ_OBJECT, 0);
    }

    if (status < 0 && dataObjInp->specColl == NULL) {
        int status2;
        status2 = resolvePathInSpecColl (rsComm, dataObjInp->objPath,
          specCollPerm, 0, dataObjInfo);
        if (status2 < 0) {
            if (status2 == SYS_SPEC_COLL_OBJ_NOT_EXIST &&
              dataObjInfo != NULL) {
                freeDataObjInfo (*dataObjInfo);
                *dataObjInfo = NULL;
            }
        }
	if (status2 >= 0) status = 0;
#if 0
	if (status2 ==CAT_NO_ROWS_FOUND) return(status);
        return(status2);
#endif
    }
    if (status >= 0) {
        if ((*dataObjInfo)->specColl != NULL) {
            if ((*dataObjInfo)->specColl->collClass == LINKED_COLL) {
		/* already been tranlated */
		rstrcpy (dataObjInp->objPath, (*dataObjInfo)->objPath,
		  MAX_NAME_LEN);
		free ((*dataObjInfo)->specColl);
	        (*dataObjInfo)->specColl = NULL;
	    } else if (getStructFileType ((*dataObjInfo)->specColl) >= 0) {
                dataObjInp->numThreads = NO_THREADING;
	    }
	}
    }
    return (status);
}

int
modCollInfo2 (rsComm_t *rsComm, specColl_t *specColl, int clearFlag)
{
    int status;
    char collInfo2[MAX_NAME_LEN];
    collInp_t modCollInp;

    memset (&modCollInp, 0, sizeof (modCollInp));
    rstrcpy (modCollInp.collName, specColl->collection, MAX_NAME_LEN);
    addKeyVal (&modCollInp.condInput, COLLECTION_TYPE_KW,
      TAR_STRUCT_FILE_STR); /* need this or rsModColl fail */
    if (clearFlag > 0) {
	rstrcpy (collInfo2, "NULL_SPECIAL_VALUE", MAX_NAME_LEN);
    } else {
        makeCachedStructFileStr (collInfo2, specColl);
    }
    addKeyVal (&modCollInp.condInput, COLLECTION_INFO2_KW, collInfo2);
    status = rsModColl (rsComm, &modCollInp);
    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "tarSubStructFileWrite:rsModColl error for Coll %s,stat=%d",
         modCollInp.collName, status);
    }
    return status;
}

int
regNewObjSize (rsComm_t *rsComm, char *objPath, int replNum,
rodsLong_t newSize)
{
    dataObjInfo_t dataObjInfo;
    keyValPair_t regParam;
    modDataObjMeta_t modDataObjMetaInp;
    char tmpStr[MAX_NAME_LEN]; 
    int status;

    if (objPath == NULL) return USER__NULL_INPUT_ERR;

    memset (&dataObjInfo, 0, sizeof (dataObjInfo));
    memset (&regParam, 0, sizeof (regParam));
    memset (&modDataObjMetaInp, 0, sizeof (modDataObjMetaInp));

    rstrcpy (dataObjInfo.objPath, objPath, MAX_NAME_LEN);
    dataObjInfo.replNum = replNum;
    snprintf (tmpStr, MAX_NAME_LEN, "%lld", newSize);
    addKeyVal (&regParam, DATA_SIZE_KW, tmpStr);

    modDataObjMetaInp.dataObjInfo = &dataObjInfo;
    modDataObjMetaInp.regParam = &regParam;
    status = rsModDataObjMeta (rsComm, &modDataObjMetaInp);
    if (status < 0) {
       rodsLog (LOG_ERROR,
          "regNewObjSize: rsModDataObjMeta error for %s, status = %d",
          objPath, status);
    }

    return (status);
}

int
isCollEmpty (rsComm_t *rsComm, char *collection)
{
    collInp_t openCollInp;
    collEnt_t *collEnt;
    int handleInx;
    int entCnt = 0;

    if (rsComm == NULL || collection == NULL) {
        rodsLog (LOG_ERROR,
	  "isCollEmpty: Input rsComm or collection is NULL");
	return True;
    }

    memset (&openCollInp, 0, sizeof (openCollInp));
    rstrcpy (openCollInp.collName, collection, MAX_NAME_LEN);
    /* cannot query recur because collection is sorted in wrong order */
    openCollInp.flags = 0;
    handleInx = rsOpenCollection (rsComm, &openCollInp);
    if (handleInx < 0) {
        rodsLog (LOG_ERROR,
          "isCollEmpty: rsOpenCollection of %s error. status = %d",
          openCollInp.collName, handleInx);
	return (True);
    }

    while (rsReadCollection (rsComm, &handleInx, &collEnt) >= 0) {
	entCnt++;
	free (collEnt);     /* just free collEnt but not content */
    }

    rsCloseCollection (rsComm, &handleInx);

    if (entCnt > 0) 
	return False;
    else 
	return True;
}

int
removeAVUMetadataFromKVPairs (rsComm_t *rsComm, char *objName, char *inObjType,
                           keyValPair_t *kVP)
{
  int i,j;
  char  objType[10];
  modAVUMetadataInp_t modAVUMetadataInp;

  if (strcmp(inObjType,"-1")) {
    strcpy(objType,inObjType);
  }
  else {
    i = getObjType(rsComm, objName,objType);
    if (i < 0)
      return(i);
  }

  modAVUMetadataInp.arg0 = "rm";
  for (i = 0; i < kVP->len ; i++) {
     /* Call rsModAVUMetadata to call chlAddAVUMetadata.
        rsModAVUMetadata connects to the icat-enabled server if the
        local host isn't.
     */
    modAVUMetadataInp.arg1 = objType;
    modAVUMetadataInp.arg2 = objName;
    modAVUMetadataInp.arg3 = kVP->keyWord[i];
    modAVUMetadataInp.arg4 = kVP->value[i];
    modAVUMetadataInp.arg5 = "";
    j = rsModAVUMetadata (rsComm, &modAVUMetadataInp);
    if (j < 0)
      return(j);
  }
  return(0);
}

#ifndef windows_platform
int
initReExec (rsComm_t *rsComm, reExec_t *reExec)
{
    int i;
    ruleExecInfo_t rei;
    int status;

    if (reExec == NULL) return SYS_INTERNAL_NULL_INPUT_ERR;

    bzero (reExec, sizeof (reExec_t));
    bzero (&rei, sizeof (ruleExecInfo_t)); /* RAJA ADDED June 17. 2009 */
    
    rei.rsComm = rsComm;
    status = applyRule ("acSetReServerNumProc", NULL, &rei, NO_SAVE_REI);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "initReExec: rule acSetReServerNumProc error, status = %d",
          status);
	reExec->maxRunCnt = DEF_NUM_RE_PROCS;
    } else {
	reExec->maxRunCnt = rei.status;
	if (reExec->maxRunCnt > MAX_RE_PROCS) {
	    reExec->maxRunCnt = MAX_RE_PROCS;
        } else if (reExec->maxRunCnt <= 0) {
            reExec->maxRunCnt = DEF_NUM_RE_PROCS;
        }
    }
    for (i = 0; i < reExec->maxRunCnt; i++) {
	reExec->reExecProc[i].procExecState = RE_PROC_IDLE;
        reExec->reExecProc[i].ruleExecSubmitInp.packedReiAndArgBBuf =
          (bytesBuf_t *) malloc (sizeof (bytesBuf_t));
        reExec->reExecProc[i].ruleExecSubmitInp.packedReiAndArgBBuf->buf = 
	  malloc (REI_BUF_LEN);
        reExec->reExecProc[i].ruleExecSubmitInp.packedReiAndArgBBuf->len = 
	  REI_BUF_LEN;

        /* init reComm */
        reExec->reExecProc[i].reComm.proxyUser = rsComm->proxyUser;
        reExec->reExecProc[i].reComm.myEnv = rsComm->myEnv;
    }
    return 0;
}
#endif

int 
allocReThr (reExec_t *reExec)
{
    int i;
    int thrInx = SYS_NO_FREE_RE_THREAD;

    if (reExec == NULL) return SYS_INTERNAL_NULL_INPUT_ERR;

    if (reExec->maxRunCnt == 1) {
	/* single thread */
	reExec->runCnt = 1;
	return 0;	
    }

    reExec->runCnt = 0;		/* reset each time */
    for (i = 0; i < reExec->maxRunCnt; i++) {
	if (reExec->reExecProc[i].procExecState == RE_PROC_IDLE) {
	    if (thrInx == SYS_NO_FREE_RE_THREAD) {
		thrInx = i;
	    }
	} else {
	    reExec->runCnt++;
	}
    }
    if (thrInx == SYS_NO_FREE_RE_THREAD) {
	thrInx = waitAndFreeReThr (reExec);
    }
    if (thrInx >= 0) 
        reExec->reExecProc[thrInx].procExecState = RE_PROC_RUNNING;

    return (thrInx);
}

int
waitAndFreeReThr (reExec_t *reExec)
{
    pid_t childPid;
    int status = 0;
    int thrInx = SYS_NO_FREE_RE_THREAD;

    childPid = waitpid (-1, &status, WUNTRACED);
    if (childPid < 0) {
	if (reExec->runCnt > 0) {
	    int i;
            rodsLog (LOG_NOTICE,
             "waitAndFreeReThr: no outstanding child. but runCnt=%d",
              reExec->runCnt);
	    for (i = 0; i < reExec->maxRunCnt; i++) {
		if (reExec->reExecProc[i].procExecState != RE_PROC_IDLE) {
		    freeReThr (reExec, i);
		}
	    }
	    reExec->runCnt = 0;
	    thrInx = 0;
	}
    } else {
        thrInx = matchPidInReExec (reExec, childPid);
        if (thrInx >= 0) freeReThr (reExec, thrInx);
    }
    return thrInx;
}

int
matchPidInReExec (reExec_t *reExec, pid_t pid)
{
    int i;

    for (i = 0; i < reExec->maxRunCnt; i++) {
	if (reExec->reExecProc[i].pid == pid) return i;
    }
    rodsLog (LOG_ERROR,
      "matchPidInReExec: no match for pid %d", pid);

    return SYS_NO_FREE_RE_THREAD;
}

int
freeReThr (reExec_t *reExec, int thrInx)
{
    bytesBuf_t *packedReiAndArgBBuf;

#ifdef RE_SERVER_DEBUG
    rodsLog (LOG_NOTICE,
      "freeReThr: thrInx %d, pid %d",thrInx, reExec->reExecProc[thrInx].pid);
#endif
    if (reExec == NULL) return SYS_INTERNAL_NULL_INPUT_ERR;
    if (thrInx < 0 || thrInx >= reExec->maxRunCnt) {
        rodsLog (LOG_ERROR, "freeReThr: Bad input thrInx %d", thrInx);
	return (SYS_BAD_RE_THREAD_INX);
    }
    reExec->runCnt--;
    reExec->reExecProc[thrInx].procExecState = RE_PROC_IDLE;
    reExec->reExecProc[thrInx].status = 0;
    reExec->reExecProc[thrInx].jobType = 0;
    reExec->reExecProc[thrInx].pid = 0;
    /* save the packedReiAndArgBBuf */
    packedReiAndArgBBuf = 
    reExec->reExecProc[thrInx].ruleExecSubmitInp.packedReiAndArgBBuf;

    bzero (packedReiAndArgBBuf->buf, REI_BUF_LEN);
    bzero (&reExec->reExecProc[thrInx].ruleExecSubmitInp, 
      sizeof (ruleExecSubmitInp_t));
    reExec->reExecProc[thrInx].ruleExecSubmitInp.packedReiAndArgBBuf = 
      packedReiAndArgBBuf;

    return 0;
}

int
runRuleExec (reExecProc_t *reExecProc)
{
    ruleExecSubmitInp_t *myRuleExec;
    ruleExecInfoAndArg_t *reiAndArg = NULL;
    rsComm_t *reComm;

    if (reExecProc == NULL) {
	rodsLog (LOG_ERROR, "runRuleExec: NULL reExecProc input");
	reExecProc->status = SYS_INTERNAL_NULL_INPUT_ERR;
	return reExecProc->status;
    }
	
    reComm = &reExecProc->reComm;
    myRuleExec = &reExecProc->ruleExecSubmitInp;

    reExecProc->status = unpackReiAndArg (reComm, &reiAndArg,
      myRuleExec->packedReiAndArgBBuf);

    if (reExecProc->status < 0) {
        rodsLog (LOG_ERROR,
          "runRuleExec: unpackReiAndArg of id %s failed, status = %d",
              myRuleExec->ruleExecId, reExecProc->status);
        return reExecProc->status;
    }

    /* execute the rule */
    reExecProc->status = applyRule (myRuleExec->ruleName,
      reiAndArg->rei->msParamArray,
      reiAndArg->rei, SAVE_REI);

    if (reiAndArg->rei->status < 0) {
        reExecProc->status = reiAndArg->rei->status;
    }
    freeRuleExecInfoStruct (reiAndArg->rei, FREE_MS_PARAM | FREE_DOINP);
    free (reiAndArg);

    return reExecProc->status;
}

int
postProcRunRuleExec (rsComm_t *rsComm, reExecProc_t *reExecProc)
{
    int status;
    int savedStatus = 0;
    ruleExecDelInp_t ruleExecDelInp;
    ruleExecSubmitInp_t *myRuleExecInp;

    myRuleExecInp = &reExecProc->ruleExecSubmitInp;

    if (strlen (myRuleExecInp->exeFrequency) > 0 ) {
        rodsLog(LOG_NOTICE, "postProcRunRuleExec: exec of freq: %s",
          myRuleExecInp->exeFrequency);

        savedStatus = modExeInfoForRepeat (rsComm, myRuleExecInp->ruleExecId,
          myRuleExecInp->exeTime, myRuleExecInp->exeFrequency, 
	  reExecProc->status);
    }
    else if (reExecProc->status < 0) {
        rodsLog (LOG_ERROR,
          "postProcRunRuleExec: ruleExec of id %s failed, status = %d",
          myRuleExecInp->ruleExecId, reExecProc->status);
        if ((reExecProc->jobType & RE_FAILED_STATUS) == 0) {
            /* first time. just mark it RE_FAILED */
            regExeStatus (rsComm, myRuleExecInp->ruleExecId, RE_FAILED);
        } else {

            /* failed once already. delete the ruleExecId */
             rodsLog (LOG_ERROR,
               "postProcRunRuleExec: ruleExec of %s: %s failed again.Removed",
                   myRuleExecInp->ruleExecId, myRuleExecInp->ruleName);
             rstrcpy (ruleExecDelInp.ruleExecId, myRuleExecInp->ruleExecId,
                   NAME_LEN);
             status = rsRuleExecDel (rsComm, &ruleExecDelInp);
             if (status < 0) {
                 rodsLog (LOG_ERROR,
                  "postProcRunRuleExec: rsRuleExecDel failed for %s, stat=%d",
                   myRuleExecInp->ruleExecId, status);
             }
        }
    } else {
        rstrcpy (ruleExecDelInp.ruleExecId, myRuleExecInp->ruleExecId,
                 NAME_LEN);
        rodsLog (LOG_NOTICE,
          "postProcRunRuleExec: exec of %s done", myRuleExecInp->ruleExecId);
        status = rsRuleExecDel (rsComm, &ruleExecDelInp);
        if (status < 0) {
           rodsLog (LOG_ERROR,
            "postProcRunRuleExec: rsRuleExecDel failed for %s, status = %d",
             myRuleExecInp->ruleExecId, status);
        }
    }
    if (status >= 0 && savedStatus < 0) return savedStatus;

    return status;
}

int
matchRuleExecId (reExec_t *reExec, char *ruleExecIdStr, 
procExecState_t execState)
{
    int i;

    if (reExec == NULL || ruleExecIdStr == NULL ||
      execState == RE_PROC_IDLE) return 0;

    for (i = 0; i < reExec->maxRunCnt; i++) {
        if (reExec->reExecProc[i].procExecState == execState &&
	  strcmp (reExec->reExecProc[i].ruleExecSubmitInp.ruleExecId,
	  ruleExecIdStr) == 0) {
	    return 1;
	}
    }
    return 0;
}

int
getRescClass (rescInfo_t *rescInfo)
{
    int classType;

    if (rescInfo == NULL) return USER__NULL_INPUT_ERR;

    classType = RescClass[rescInfo->rescClassInx].classType;

    return classType;
}

int
getRescGrpClass (rescGrpInfo_t *rescGrpInfo, rescInfo_t **outRescInfo)
{
    rescInfo_t *tmpRescInfo;
    rescGrpInfo_t *tmpRescGrpInfo = rescGrpInfo;

    while (tmpRescGrpInfo != NULL) {
        tmpRescInfo = tmpRescGrpInfo->rescInfo;
        if (getRescClass (tmpRescInfo) == COMPOUND_CL) {
            *outRescInfo = tmpRescInfo;
            return COMPOUND_CL;
        }
        tmpRescGrpInfo = tmpRescGrpInfo->next;
    }
    *outRescInfo = NULL;
    /* just use the top */
    return (getRescClass(rescGrpInfo->rescInfo));
}


int
compareRescAddr (rescInfo_t *srcRescInfo, rescInfo_t *destRescInfo)
{
    rodsHostAddr_t srcAddr;
    rodsHostAddr_t destAddr;
    rodsServerHost_t *srcServerHost = NULL;
    rodsServerHost_t *destServerHost = NULL;

    bzero (&srcAddr, sizeof (srcAddr));
    bzero (&destAddr, sizeof (destAddr));

    rstrcpy (srcAddr.hostAddr, srcRescInfo->rescLoc, NAME_LEN);
    rstrcpy (destAddr.hostAddr, destRescInfo->rescLoc, NAME_LEN);

    resolveHost (&srcAddr, &srcServerHost);
    resolveHost (&destAddr, &destServerHost);

    if (srcServerHost == destServerHost)
	return 1;
    else
	return 0;
}

/* getCacheRescInGrp - get the cache resc in the resource group specified
 * by rescGroupName. If rescGroupName does not exist, find the rescGroupName
 * that include memberRescInfo. Either rescGroupName or memberRescInfo must
 * exist. If rescGroupName is zero len, the rescGroupName will be updated.
 */
int
getCacheRescInGrp (rsComm_t *rsComm, char *rescGroupName, 
rescInfo_t *memberRescInfo, rescInfo_t **outCacheResc)
{
    int status; 
    rescGrpInfo_t *myRescGrpInfo = NULL;
    rescGrpInfo_t *tmpRescGrpInfo;

    *outCacheResc = NULL;

    if (rescGroupName == NULL || strlen (rescGroupName) == 0) {

	if (memberRescInfo == NULL) {
            rodsLog (LOG_ERROR,
              "getCacheRescInGrp: no rescGroupName input");
            return SYS_NO_CACHE_RESC_IN_GRP;
        }

        /* no input rescGrp. Try to find one that matches rescInfo. */
        status = getRescGrpOfResc (rsComm, memberRescInfo, &myRescGrpInfo);
        if (status < 0) {
            rodsLog (LOG_NOTICE,
              "getCacheRescInGrp:getRescGrpOfResc err for %s. stat=%d",
              memberRescInfo->rescName, status);
	    return status;
        } else if (rescGroupName != NULL) {
	    rstrcpy (rescGroupName, myRescGrpInfo->rescGroupName, NAME_LEN);
	}
    } else {
        status = resolveRescGrp (rsComm, rescGroupName, &myRescGrpInfo);
        if (status < 0) return status;
    }
    tmpRescGrpInfo = myRescGrpInfo;
    while (tmpRescGrpInfo != NULL) {
	rescInfo_t *tmpRescInfo;
        tmpRescInfo = tmpRescGrpInfo->rescInfo;
        if (RescClass[tmpRescInfo->rescClassInx].classType == CACHE_CL) {
	    *outCacheResc = tmpRescInfo;
	    freeAllRescGrpInfo (myRescGrpInfo);
	    return 0;
	}
	tmpRescGrpInfo = tmpRescGrpInfo->next;
    }
    freeAllRescGrpInfo (myRescGrpInfo);
    return SYS_NO_CACHE_RESC_IN_GRP;
}

int
getRescInGrp (rsComm_t *rsComm, char *rescName, char *rescGroupName,
rescInfo_t **outRescInfo)
{
    int status;
    rescGrpInfo_t *myRescGrpInfo = NULL;
    rescGrpInfo_t *tmpRescGrpInfo;

    if (rescGroupName == NULL || strlen (rescGroupName) == 0) 
	return USER__NULL_INPUT_ERR;


    status = resolveRescGrp (rsComm, rescGroupName, &myRescGrpInfo);
 
    if (status < 0) return status;

    tmpRescGrpInfo = myRescGrpInfo;
    while (tmpRescGrpInfo != NULL) {
        rescInfo_t *tmpRescInfo;
        tmpRescInfo = tmpRescGrpInfo->rescInfo;
        if (strcmp (tmpRescInfo->rescName, rescName) == 0) { 
	    if (outRescInfo != NULL)
                *outRescInfo = tmpRescInfo;
	    freeAllRescGrpInfo (myRescGrpInfo);
            return 0;
        }
        tmpRescGrpInfo = tmpRescGrpInfo->next;
    }
    freeAllRescGrpInfo (myRescGrpInfo);
    return SYS_UNMATCHED_RESC_IN_RESC_GRP;
}

int
getCacheDataInfoForRepl (rsComm_t *rsComm, dataObjInfo_t *srcDataObjInfoHead,
dataObjInfo_t *destDataObjInfoHead, dataObjInfo_t *compDataObjInfo, 
dataObjInfo_t **outDataObjInfo)
{
    char *rescGroupName;
    int status;

    rescGroupName = compDataObjInfo->rescGroupName;

    if (strlen (rescGroupName) == 0) {
	rescGrpInfo_t *rescGrpInfo = NULL;
	rescGrpInfo_t *tmpRescGrpInfo;
 
        /* no input rescGrp. Try to find one that matches rescInfo. */
	status = getRescGrpOfResc (rsComm, compDataObjInfo->rescInfo,
	  &rescGrpInfo);
	if (status < 0) {
	    rodsLog (LOG_NOTICE,
              "getCacheDataInfoForRepl:getRescGrpOfResc err for %s. stat=%d",
              compDataObjInfo->rescInfo->rescName, status);
	    return status;
	}
        tmpRescGrpInfo = rescGrpInfo;

        while (tmpRescGrpInfo != NULL) {
	    status = getCacheDataInfoInRescGrp (srcDataObjInfoHead,
	      destDataObjInfoHead, tmpRescGrpInfo->rescGroupName,
	      compDataObjInfo, outDataObjInfo);
	    if (status >= 0) {
		/* update the rescGroupName */
		rstrcpy (compDataObjInfo->rescGroupName, 
		  tmpRescGrpInfo->rescGroupName, NAME_LEN);
		rstrcpy ((*outDataObjInfo)->rescGroupName, 
		  tmpRescGrpInfo->rescGroupName, NAME_LEN);
		freeAllRescGrpInfo (rescGrpInfo);
		return 0;
	    } else {
	        status = getNonGrpCacheDataInfoInRescGrp (srcDataObjInfoHead,
                  destDataObjInfoHead, tmpRescGrpInfo,
                  compDataObjInfo, outDataObjInfo);
                if (status >= 0) {
                    /* update the rescGroupName */
                    rstrcpy (compDataObjInfo->rescGroupName,
                      tmpRescGrpInfo->rescGroupName, NAME_LEN);
		    rstrcpy ((*outDataObjInfo)->rescGroupName, 
		      tmpRescGrpInfo->rescGroupName, NAME_LEN);
                    freeAllRescGrpInfo (rescGrpInfo);
                    return 0;
		}
	    }
            tmpRescGrpInfo = tmpRescGrpInfo->cacheNext;
	}
	freeAllRescGrpInfo (rescGrpInfo);
	status = SYS_NO_CACHE_RESC_IN_GRP;
    } else {
        status = getCacheDataInfoInRescGrp (srcDataObjInfoHead,
          destDataObjInfoHead, rescGroupName, compDataObjInfo, outDataObjInfo);
	if (status < 0) {
	    rescGrpInfo_t *tmpRescGrpInfo;
	    /* maybe the cache copy does not have a group associated with it */
	    status = resolveRescGrp (rsComm, rescGroupName, &tmpRescGrpInfo);
	    if (status >= 0) {
                status = getNonGrpCacheDataInfoInRescGrp (srcDataObjInfoHead,
                  destDataObjInfoHead, tmpRescGrpInfo,
                  compDataObjInfo, outDataObjInfo);
                if (status >= 0) {
                    /* update the rescGroupName */
                    rstrcpy (compDataObjInfo->rescGroupName,
                      tmpRescGrpInfo->rescGroupName, NAME_LEN);
                    rstrcpy ((*outDataObjInfo)->rescGroupName,
                      tmpRescGrpInfo->rescGroupName, NAME_LEN);
                }
                freeAllRescGrpInfo (tmpRescGrpInfo);
	    }
	}
    }
    return status;
}

/* getNonGrpCacheDataInfoInRescGrp - get the cache dataObjInfo even
 * if it does not belong to the same group */

int
getNonGrpCacheDataInfoInRescGrp (dataObjInfo_t *srcDataObjInfoHead,
dataObjInfo_t *destDataObjInfoHead, rescGrpInfo_t *rescGrpInfo,
dataObjInfo_t *compDataObjInfo, dataObjInfo_t **outDataObjInfo)
{
    dataObjInfo_t *srcDataObjInfo;
    rescGrpInfo_t *tmpRescGrpInfo;

    srcDataObjInfo = srcDataObjInfoHead;
    while (srcDataObjInfo != NULL) {
	if (getRescClass (srcDataObjInfo->rescInfo) == CACHE_CL) {
	    tmpRescGrpInfo = rescGrpInfo;
	    while (tmpRescGrpInfo != NULL) {
		if (strcmp (srcDataObjInfo->rescInfo->rescName,
		  tmpRescGrpInfo->rescInfo->rescName) == 0) {
		    *outDataObjInfo = srcDataObjInfo;
		    return 0;
		}
		tmpRescGrpInfo = tmpRescGrpInfo->next;
	    }
	}
	srcDataObjInfo = srcDataObjInfo->next;
    }

    /* try destDataObjInfoHead but up to compDataObjInfo because
     * they have been updated and are good copies */

    srcDataObjInfo = destDataObjInfoHead;
    while (srcDataObjInfo != NULL) {
	if (srcDataObjInfo == compDataObjInfo) break;
        if (getRescClass (srcDataObjInfo->rescInfo) == CACHE_CL) {
            tmpRescGrpInfo = rescGrpInfo;
            while (tmpRescGrpInfo != NULL) {
                if (strcmp (srcDataObjInfo->rescInfo->rescName,
                  tmpRescGrpInfo->rescInfo->rescName) == 0) {
                    *outDataObjInfo = srcDataObjInfo;
                    return 0;
                }
                tmpRescGrpInfo = tmpRescGrpInfo->next;
            }
        }
        srcDataObjInfo = srcDataObjInfo->next;
    }
    return SYS_NO_CACHE_RESC_IN_GRP;
}

int
getCacheDataInfoInRescGrp (dataObjInfo_t *srcDataObjInfoHead,
dataObjInfo_t *destDataObjInfoHead, char *rescGroupName,
dataObjInfo_t *compDataObjInfo, dataObjInfo_t **outDataObjInfo)
{
    dataObjInfo_t *srcDataObjInfo;

    srcDataObjInfo = srcDataObjInfoHead;
    while (srcDataObjInfo != NULL) {
	if (strcmp (srcDataObjInfo->rescGroupName, rescGroupName) == 0) {
	    /* same group */
	    if (getRescClass (srcDataObjInfo->rescInfo) == CACHE_CL) {
		*outDataObjInfo = srcDataObjInfo;
		return 0;
	    }
	}
	srcDataObjInfo = srcDataObjInfo->next;
    }

    /* try destDataObjInfoHead but up to compDataObjInfo because
     * they have been updated and are good copies */

    srcDataObjInfo = destDataObjInfoHead;
    while (srcDataObjInfo != NULL) {
	if (srcDataObjInfo == compDataObjInfo) break;
        if (strcmp (srcDataObjInfo->rescGroupName, rescGroupName) == 0) {
            /* same group */
            if (getRescClass (srcDataObjInfo->rescInfo) == CACHE_CL) {
                *outDataObjInfo = srcDataObjInfo;
                return 0;
            }
        }
        srcDataObjInfo = srcDataObjInfo->next;
    }
    return SYS_NO_CACHE_RESC_IN_GRP;
}

/* getRescGrpOfResc - given the rescInfo, find a rescGrpInfo that this
 * resource belongs. The output is a link list of rescGrpInfo_t pointed to by
 * cacheNext.
 */

int
getRescGrpOfResc (rsComm_t *rsComm, rescInfo_t * rescInfo, 
rescGrpInfo_t **rescGrpInfo)
{
    rescGrpInfo_t *tmpRescGrpInfo, *myRescGrpInfo;
    rescGrpInfo_t *outRescGrpInfo = NULL;
    rescInfo_t *myRescInfo;
    int status;

    *rescGrpInfo = NULL;

    if ((status = initRescGrp (rsComm)) < 0) return status;

    /* in cache ? */
    tmpRescGrpInfo = CachedRescGrpInfo;
    while (tmpRescGrpInfo != NULL) {
	myRescGrpInfo = tmpRescGrpInfo;
	while (myRescGrpInfo != NULL) {
            myRescInfo = myRescGrpInfo->rescInfo;
            if (strcmp (rescInfo->rescName, myRescInfo->rescName) == 0) {
		replRescGrpInfo (tmpRescGrpInfo, &outRescGrpInfo);
		outRescGrpInfo->cacheNext = *rescGrpInfo;
		*rescGrpInfo = outRescGrpInfo;
		break;
            }
            myRescGrpInfo = myRescGrpInfo->next;
	}

        tmpRescGrpInfo = tmpRescGrpInfo->cacheNext;
    }

    if (*rescGrpInfo == NULL)
        return SYS_NO_CACHE_RESC_IN_GRP;
    else
	return 0;
}

/* isRescsInSameGrp - check if 2 rescources are in the same rescGroup
 * returns 1 if match, 0 for no match.
 */

int
isRescsInSameGrp (rsComm_t *rsComm, char *rescName1, char *rescName2,
rescGrpInfo_t **outRescGrpInfo)
{
    rescGrpInfo_t *tmpRescGrpInfo, *myRescGrpInfo;
    rescInfo_t *myRescInfo;
    int status;
    int match1, match2;

    if (outRescGrpInfo != NULL)
        *outRescGrpInfo = NULL;

    if ((status = initRescGrp (rsComm)) < 0) return status;

    /* in cache ? */
    tmpRescGrpInfo = CachedRescGrpInfo;
    while (tmpRescGrpInfo != NULL) {
        myRescGrpInfo = tmpRescGrpInfo;
	match1 = match2 = 0;
        while (myRescGrpInfo != NULL) {
            myRescInfo = myRescGrpInfo->rescInfo;
	    if (match1 == 0 && 
	      strcmp (rescName1, myRescInfo->rescName) == 0) {
		match1 = 1;
	    } else if (match2 == 0 && 
	      strcmp (rescName2, myRescInfo->rescName) == 0) {
		match2 = 1;
	    }
	    if (match1 == 1 && match2 == 1) {
                if (outRescGrpInfo != NULL) *outRescGrpInfo = tmpRescGrpInfo;
		return 1;
	    }
            myRescGrpInfo = myRescGrpInfo->next;
        }

        tmpRescGrpInfo = tmpRescGrpInfo->cacheNext;
    }
    return 0;
}


/* initRescGrp - Initialize the CachedRescGrpInfo queue
 */

int
initRescGrp (rsComm_t *rsComm)
{
    genQueryInp_t genQueryInp;
    rescGrpInfo_t *tmpRescGrpInfo;
    genQueryOut_t *genQueryOut = NULL;
    sqlResult_t *rescName, *rescGrpName;
    char *rescNameStr, *rescGrpNameStr;
    char *curRescGrpNameStr = NULL;
    int i;
    int status = 0;
    int savedRescGrpStatus = 0;

    if (RescGrpInit > 0) return 0;

    RescGrpInit = 1;

    /* query all resource groups */
    memset (&genQueryInp, 0, sizeof (genQueryInp_t));

    addInxIval (&genQueryInp.selectInp, COL_R_RESC_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_RESC_GROUP_NAME, ORDER_BY);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

    clearGenQueryInp (&genQueryInp);

    if (status < 0) {
	if (status == CAT_NO_ROWS_FOUND)
	    return 0;
	else
	    return status;
    }

    if ((rescName = getSqlResultByInx (genQueryOut, COL_R_RESC_NAME)) ==
      NULL) {
        rodsLog (LOG_NOTICE,
          "initRescGrp: getSqlResultByInx for COL_R_RESC_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((rescGrpName = getSqlResultByInx (genQueryOut, COL_RESC_GROUP_NAME)) ==
      NULL) {
        rodsLog (LOG_NOTICE,
          "initRescGrp: getSqlResultByInx for COL_RESC_GROUP_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    tmpRescGrpInfo = NULL;
    for (i = 0;i < genQueryOut->rowCnt; i++) {
        rescNameStr = &rescName->value[rescName->len * i];
        rescGrpNameStr = &rescGrpName->value[rescGrpName->len * i];
	if (tmpRescGrpInfo != NULL && curRescGrpNameStr != NULL) {
	    if (strcmp (rescGrpNameStr, curRescGrpNameStr) != 0) {
		/* a new rescGrp. queue the current one */
        	tmpRescGrpInfo->cacheNext = CachedRescGrpInfo;
		tmpRescGrpInfo->status = savedRescGrpStatus;
		savedRescGrpStatus = 0;
        	CachedRescGrpInfo = tmpRescGrpInfo;
		tmpRescGrpInfo = NULL;
	    }
	}
	curRescGrpNameStr = rescGrpNameStr;
        status = resolveAndQueResc (rescNameStr, rescGrpNameStr,
          &tmpRescGrpInfo);
        if (status < 0) {
	    if (status == SYS_RESC_IS_DOWN) {
		savedRescGrpStatus = SYS_RESC_IS_DOWN;
	    } else {
                rodsLog (LOG_NOTICE,
                  "initRescGrp: resolveAndQueResc error for %s. status = %d",
                  rescNameStr, status);
                freeGenQueryOut (&genQueryOut);
                return (status);
	    }
        }
    }

    /* query the remaining in cache */
    if (tmpRescGrpInfo != NULL) {
	tmpRescGrpInfo->status = savedRescGrpStatus;
        tmpRescGrpInfo->cacheNext = CachedRescGrpInfo;
        CachedRescGrpInfo = tmpRescGrpInfo;
    }

    return 0;
}

int
setDefaultResc (rsComm_t *rsComm, char *defaultRescList, char *optionStr,
keyValPair_t *condInput, rescGrpInfo_t **outRescGrpInfo)
{
    rescGrpInfo_t *myRescGrpInfo = NULL;
    rescGrpInfo_t *tmpRescGrpInfo, *prevRescGrpInfo;
    char *value = NULL;
    strArray_t strArray;
    int i, status;
    char *defaultResc;
    int startInx;

    if (defaultRescList != NULL && strcmp (defaultRescList, "null") != 0 &&
      optionStr != NULL &&  strcmp (optionStr, "force") == 0 &&
      rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
        condInput = NULL;
    }

    memset (&strArray, 0, sizeof (strArray));

    status = parseMultiStr (defaultRescList, &strArray);

#if 0	/* this will produce no output */
    if (status <= 0)
        return (0);
#endif

    value = strArray.value;
    if (strArray.len <= 1) {
        startInx = 0;
        defaultResc = value;
    } else {
        /* select one on the list randomly */
        startInx = random() % strArray.len;
        defaultResc = &value[startInx * strArray.size];
    }


    if (optionStr == NULL) {
        status = getRescInfo (rsComm, defaultResc, condInput,
          &myRescGrpInfo);
    } else if (strcmp (optionStr, "preferred") == 0) {
        status = getRescInfo (rsComm, NULL, condInput,
          &myRescGrpInfo);
        if (status >= 0) {
            if (strlen (myRescGrpInfo->rescGroupName) > 0) {
                for (i = 0; i < strArray.len; i++) {
                    int j;
                    j = startInx + i;
                    if (j >= strArray.len) {
                        /* wrap around */
                        j = strArray.len - j;
                    }
                    tmpRescGrpInfo = myRescGrpInfo;
                    prevRescGrpInfo = NULL;
                    while (tmpRescGrpInfo != NULL) {
                        if (strcmp (&value[j * strArray.size],
                          tmpRescGrpInfo->rescInfo->rescName) == 0) {
                            /* put it on top */
                            if (prevRescGrpInfo != NULL) {
                                prevRescGrpInfo->next = tmpRescGrpInfo->next;
                                tmpRescGrpInfo->next = myRescGrpInfo;
                                myRescGrpInfo = tmpRescGrpInfo;
                            }
                            break;
                        }
                        prevRescGrpInfo = tmpRescGrpInfo;
                        tmpRescGrpInfo = tmpRescGrpInfo->next;
                    }
                }
            }
        } else {
            /* error may mean there is no input resource. try to use the
             * default resource by dropping down */
            status = getRescInfo (rsComm, defaultResc, condInput,
              &myRescGrpInfo);
        }
    } else if (strcmp (optionStr, "forced") == 0) {
        status = getRescInfo (rsComm, defaultResc, NULL,
          &myRescGrpInfo);
    } else {
        status = getRescInfo (rsComm, defaultResc, condInput,
          &myRescGrpInfo);
    }

    if (status == CAT_NO_ROWS_FOUND)
      status = SYS_RESC_DOES_NOT_EXIST;

    if (value != NULL)
        free (value);

    if (status >= 0) {
        *outRescGrpInfo = myRescGrpInfo;
    } else {
        *outRescGrpInfo = NULL;
    }

    return (status);
}

