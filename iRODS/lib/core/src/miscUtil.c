/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "rodsClient.h"
#include "rodsLog.h"
#include "miscUtil.h"

int
mkColl (rcComm_t *conn, char *collection)
{
    int status;
    collInp_t collCreateInp;

    memset (&collCreateInp, 0, sizeof (collCreateInp));

    rstrcpy (collCreateInp.collName, collection, MAX_NAME_LEN);
   
    status = rcCollCreate (conn, &collCreateInp);

    if (status == CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME)
	status = 0;

    return (status);
}

/* mk the directory resursively */

int
mkCollR (rcComm_t *conn, char *startColl, char *destColl)
{
    int status;
    int startLen;
    int pathLen, tmpLen;
    char tmpPath[MAX_NAME_LEN];
    rodsPath_t rodsPath;

    startLen = strlen (startColl);
    pathLen = strlen (destColl);

    rstrcpy (tmpPath, destColl, MAX_NAME_LEN);

    tmpLen = pathLen;

    memset (&rodsPath, 0, sizeof (rodsPath));
    while (tmpLen > startLen) {
        rodsPath.objType = COLL_OBJ_T;
	rodsPath.objState = UNKNOWN_ST;
	rstrcpy (rodsPath.outPath, tmpPath, MAX_NAME_LEN);
	status =getRodsObjType (conn, &rodsPath);
        if (status >= 0 && rodsPath.objState == EXIST_ST) {
	    clearRodsPath (&rodsPath);
	    break;
        } else {
	    clearRodsPath (&rodsPath);
	}

        /* Go backward */

        while (tmpLen && tmpPath[tmpLen] != '/')
            tmpLen --;
        tmpPath[tmpLen] = '\0';
    }

    /* Now we go forward and make the required coll */
    while (tmpLen < pathLen) {
        /* Put back the '/' */
        tmpPath[tmpLen] = '/';
       status = mkColl (conn, tmpPath);
        if (status < 0) {
            rodsLog (LOG_NOTICE,
             "mkCollR: mkdir failed for %s, status =%d",
              tmpPath, status);
            return status;
        }
        while (tmpLen && tmpPath[tmpLen] != '\0')
            tmpLen ++;
    }
    return 0;
}

int
mkdirR (char *startDir, char *destDir, int mode)
{
    int status;
    int startLen;
    int pathLen, tmpLen;
    char tmpPath[MAX_NAME_LEN];
    struct stat statbuf;

    startLen = strlen (startDir);
    pathLen = strlen (destDir);

    rstrcpy (tmpPath, destDir, MAX_NAME_LEN);

    tmpLen = pathLen;

    while (tmpLen > startLen) {
	if ((status = stat (tmpPath, &statbuf)) >= 0) break;
        if (status >= 0) {
	    break;
        }

        /* Go backward */

        while (tmpLen && tmpPath[tmpLen] != '/')
            tmpLen --;
        tmpPath[tmpLen] = '\0';
    }

    /* Now we go forward and make the required coll */
    while (tmpLen < pathLen) {
        /* Put back the '/' */
        tmpPath[tmpLen] = '/';
#ifdef _WIN32
        status = mkdir (tmpPath);
#else
        status = mkdir (tmpPath, mode);
#endif
        if (status < 0) {
            rodsLog (LOG_NOTICE,
             "mkdirR: mkdir failed for %s, status =%d",
              tmpPath, status);
            return status;
        }
        while (tmpLen && tmpPath[tmpLen] != '\0')
            tmpLen ++;
    }
    return 0;
}

int 
getRodsObjType (rcComm_t *conn, rodsPath_t *rodsPath)
{
    int status;
    dataObjInp_t dataObjInp;
    rodsObjStat_t *rodsObjStatOut = NULL;

    if (rodsPath == NULL) {
	return (USER__NULL_INPUT_ERR);
    }

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, rodsPath->outPath, MAX_NAME_LEN);
    status = rcObjStat (conn, &dataObjInp, &rodsObjStatOut);

    if (status < 0) {
        rodsPath->objState = NOT_EXIST_ST;
        return (NOT_EXIST_ST);
    } else if (rodsPath->objType == COLL_OBJ_T && 
      rodsObjStatOut->objType != COLL_OBJ_T) {
        rodsPath->objState = NOT_EXIST_ST;
    } else if (rodsPath->objType == DATA_OBJ_T && 
      rodsObjStatOut->objType != DATA_OBJ_T) {
        rodsPath->objState = NOT_EXIST_ST;
    } else {
	if (rodsObjStatOut->objType == UNKNOWN_OBJ_T) {
	    rodsPath->objState = NOT_EXIST_ST;
	} else {
	    rodsPath->objState = EXIST_ST;
	}
	rodsPath->objType = rodsObjStatOut->objType;
	if (rodsPath->objType == DATA_OBJ_T) {
            rstrcpy (rodsPath->dataId, rodsObjStatOut->dataId, NAME_LEN);
            rodsPath->size = rodsObjStatOut->objSize;
            rstrcpy (rodsPath->chksum, rodsObjStatOut->chksum, NAME_LEN);
	}
    }
    rodsPath->rodsObjStat = rodsObjStatOut;

    return (rodsPath->objState);
}

int
extractRodsObjType (rodsPath_t *rodsPath, sqlResult_t *dataId, 
sqlResult_t *replStatus, sqlResult_t *chksum, sqlResult_t *dataSize, 
int inx, int rowCnt)
{
    int i;
    char *prevdataId = NULL;
    int gotCopy = 0;

    if (dataId == NULL) {
	/* special coll */
        rodsPath->size =
          strtoll (&dataSize->value[dataSize->len * inx], 0, 0);
        rodsPath->objState = EXIST_ST;
	return (inx);
    }

    i = inx;

    for (i = inx;i < rowCnt; i++) {
	if (prevdataId != NULL) { 
	    if (strcmp (prevdataId, &dataId->value[dataId->len * i]) != 0) {
	        break;
	    }
	} else {
	    prevdataId = &dataId->value[dataId->len * i];
	}

        if (gotCopy == 0 && 
	  atoi (&replStatus->value[replStatus->len * i]) > 0) {
            rstrcpy (rodsPath->dataId, &dataId->value[dataId->len * i],
              NAME_LEN);
            rodsPath->size =
              strtoll (&dataSize->value[dataSize->len * i], 0, 0);
            rstrcpy (rodsPath->chksum,
              &chksum->value[chksum->len * i], NAME_LEN);
	    gotCopy = 1;
        }
    }

    if (strlen (rodsPath->dataId) == 0) {
        /* just use the first one */
        rstrcpy (rodsPath->dataId, &dataId->value[dataId->len * inx], NAME_LEN);
        rodsPath->size = strtoll (&dataSize->value[dataSize->len * inx], 0, 0);
            rstrcpy (rodsPath->chksum, 
	      &chksum->value[chksum->len * inx], NAME_LEN);
    }

    rodsPath->objState = EXIST_ST;
    return (i - 1);
}

/* genAllInCollQCond - Generate a sqlCondInp for querying every thing under  
 * a collection. 
 * The server will handle special char such as "-" and "%".
 */

int
genAllInCollQCond (char *collection, char *collQCond)
{
    if (collection == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    if (strcmp (collection, "/") == 0) {
        snprintf (collQCond, MAX_NAME_LEN, " like '/%%' ");
    } else {

        snprintf (collQCond, MAX_NAME_LEN*2, " = '%s' || like '%s/%%' ",
          collection, collection);
    }
    return (0);
}

int
queryCollInCollReCur (rcComm_t *conn, char *collection, 
rodsArguments_t *rodsArgs, genQueryInp_t *genQueryInp,
genQueryOut_t **genQueryOut)
{
    char collQCond[MAX_NAME_LEN*2];
    int status;

    if (collection == NULL || genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    memset (genQueryInp, 0, sizeof (genQueryInp_t));

    genAllInCollQCond (collection, collQCond);

    addInxVal (&genQueryInp->sqlCondInp, COL_COLL_NAME, collQCond);

    addInxIval (&genQueryInp->selectInp, COL_COLL_NAME, 1);
    addInxIval (&genQueryInp->selectInp, COL_COLL_TYPE, 1);
    addInxIval (&genQueryInp->selectInp, COL_COLL_INFO1, 1);
    addInxIval (&genQueryInp->selectInp, COL_COLL_INFO2, 1);
    addInxIval (&genQueryInp->selectInp, COL_COLL_OWNER_NAME, 1);

    genQueryInp->maxRows = MAX_SQL_ROWS;

    status =  rcGenQuery (conn, genQueryInp, genQueryOut);

    return (status);

}

int
queryCollInColl (rcComm_t *conn, char *collection,
rodsArguments_t *rodsArgs, genQueryInp_t *genQueryInp,
genQueryOut_t **genQueryOut)
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
    addInxIval (&genQueryInp->selectInp, COL_COLL_OWNER_NAME, 1);
    addInxIval (&genQueryInp->selectInp, COL_COLL_TYPE, 1);
    addInxIval (&genQueryInp->selectInp, COL_COLL_INFO1, 1);
    addInxIval (&genQueryInp->selectInp, COL_COLL_INFO2, 1);

    genQueryInp->maxRows = MAX_SQL_ROWS;

    status =  rcGenQuery (conn, genQueryInp, genQueryOut);

    return (status);
}

/* queryDataObjInCollReCur - query all dataobj in a collection and
 * all subcollections in it.
 * The genQueryInp may already have some sqlCondInp and selectInp
 * in it.
 */ 
int
queryDataObjInCollReCur (rcComm_t *conn, char *collection,
rodsArguments_t *rodsArgs, genQueryInp_t *genQueryInp,
genQueryOut_t **genQueryOut)
{
    char collQCond[MAX_NAME_LEN*2];
    int status;

    if (collection == NULL || genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    genAllInCollQCond (collection, collQCond);

    addInxVal (&genQueryInp->sqlCondInp, COL_COLL_NAME, collQCond);

    if (rodsArgs->longOption == True) {
        setQueryInpForLong (rodsArgs, genQueryInp);
    } else {
        addInxIval (&genQueryInp->selectInp, COL_COLL_NAME, 1);
        addInxIval (&genQueryInp->selectInp, COL_DATA_NAME, 1);
        addInxIval (&genQueryInp->selectInp, COL_D_DATA_ID, 1);
    }

    genQueryInp->maxRows = MAX_SQL_ROWS;

    status =  rcGenQuery (conn, genQueryInp, genQueryOut);

    return (status);

}

int
queryDataObjInColl (rcComm_t *conn, char *collection, 
rodsArguments_t *rodsArgs, genQueryInp_t *genQueryInp,
genQueryOut_t **genQueryOut)
{
    char collQCond[MAX_NAME_LEN];
    int status;

    if (collection == NULL || genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    memset (genQueryInp, 0, sizeof (genQueryInp_t));

    snprintf (collQCond, MAX_NAME_LEN, " = '%s'", collection);

    addInxVal (&genQueryInp->sqlCondInp, COL_COLL_NAME, collQCond);

    if (rodsArgs->longOption == True) {
	setQueryInpForLong (rodsArgs, genQueryInp);
    } else {
        addInxIval (&genQueryInp->selectInp, COL_COLL_NAME, 1);
        addInxIval (&genQueryInp->selectInp, COL_DATA_NAME, 1);
        addInxIval (&genQueryInp->selectInp, COL_D_DATA_ID, 1);
    }

    genQueryInp->maxRows = MAX_SQL_ROWS;

    status =  rcGenQuery (conn, genQueryInp, genQueryOut);

    return (status);

}

int
setQueryInpForLong (rodsArguments_t *rodsArgs, 
genQueryInp_t *genQueryInp)
{

    if (genQueryInp == NULL) { 
        return (USER__NULL_INPUT_ERR);
    }

    addInxIval (&genQueryInp->selectInp, COL_D_DATA_ID, 1);
    addInxIval (&genQueryInp->selectInp, COL_DATA_NAME, 1);
    addInxIval (&genQueryInp->selectInp, COL_DATA_REPL_NUM, 1);
    addInxIval (&genQueryInp->selectInp, COL_DATA_SIZE, 1);
    addInxIval (&genQueryInp->selectInp, COL_D_RESC_NAME, 1);
    addInxIval (&genQueryInp->selectInp, COL_D_REPL_STATUS, 1);
    addInxIval (&genQueryInp->selectInp, COL_D_MODIFY_TIME, 1);
    addInxIval (&genQueryInp->selectInp, COL_D_OWNER_NAME, 1);

    if (rodsArgs->veryLongOption == True) {
        addInxIval (&genQueryInp->selectInp, COL_D_DATA_PATH, 1);
        addInxIval (&genQueryInp->selectInp, COL_D_DATA_CHECKSUM, 1);
        addInxIval (&genQueryInp->selectInp, COL_COLL_NAME, 1);
        addInxIval (&genQueryInp->selectInp, COL_D_CREATE_TIME, 1);
    }

    genQueryInp->maxRows = MAX_SQL_ROWS;

    return (0);
}

int
printTiming (rcComm_t *conn, char *objPath, rodsLong_t fileSize, 
char *localFile, struct timeval *startTime, struct timeval *endTime)
{
    struct timeval diffTime;
    char myDir[MAX_NAME_LEN], myFile[MAX_NAME_LEN];
    float transRate, sizeInMb, timeInSec;
    int status;
    

    if ((status = splitPathByKey (objPath, myDir, myFile, '/')) < 0) {
        rodsLogError (LOG_NOTICE, status,
          "printTiming: splitPathByKey for %s error, status = %d",
          objPath, status);
        return (status);
    }

    diffTime.tv_sec = endTime->tv_sec - startTime->tv_sec;
    diffTime.tv_usec = endTime->tv_usec - startTime->tv_usec;

    if (diffTime.tv_usec < 0) {
	diffTime.tv_sec --;
	diffTime.tv_usec += 1000000;
    }

    timeInSec = (float) diffTime.tv_sec + ((float) diffTime.tv_usec /
     1000000.0);

    if (fileSize < 0) {
	/* may be we can find it from the local file */

	if (localFile != NULL) {
	    fileSize = getFileSize (localFile);
	}
    }

    
    if (fileSize <= 0) {
	transRate = 0.0;
	sizeInMb = 0.0;
    } else {
	sizeInMb = (float) fileSize / 1048600.0;
        if (timeInSec == 0.0) {
	    transRate = 0.0;
        } else {
	    transRate = sizeInMb / timeInSec;
	}
    }

    if (fileSize < 0) {
	fprintf (stdout,
	  "   %-25.25s  %.3f sec\n",    
	    myFile, timeInSec);
    } else {
        fprintf (stdout,
          "   %-25.25s  %10.3f MB | %.3f sec | %d thr | %6.3f MB/s\n",
            myFile, sizeInMb, timeInSec, conn->transStat.numThreads, transRate);
    }

    return (0);
}

int
queryDataObjAcl (rcComm_t *conn, char *dataId, genQueryOut_t **genQueryOut)
{
    genQueryInp_t genQueryInp;
    int status;
    char tmpStr[MAX_NAME_LEN];

    if (dataId == NULL || genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    memset (&genQueryInp, 0, sizeof (genQueryInp_t));

    addInxIval (&genQueryInp.selectInp, COL_USER_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_ACCESS_NAME, 1);

    snprintf (tmpStr, MAX_NAME_LEN, " = '%s'", dataId);

    addInxVal (&genQueryInp.sqlCondInp, COL_DATA_ACCESS_DATA_ID, tmpStr);

    snprintf (tmpStr, MAX_NAME_LEN, "='%s'", "access_type");

    /* Currently necessary since other namespaces exist in the token table */
    addInxVal (&genQueryInp.sqlCondInp, COL_DATA_TOKEN_NAMESPACE, tmpStr);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status =  rcGenQuery (conn, &genQueryInp, genQueryOut);

    return (status);

}

int
genQueryOutToCollRes (genQueryOut_t **genQueryOut, 
collSqlResult_t *collSqlResult)
{
    genQueryOut_t *myGenQueryOut;
    sqlResult_t *collName, *collType, *collInfo1, *collInfo2, *collOwner;

    if (genQueryOut == NULL || (myGenQueryOut = *genQueryOut) == NULL ||
      collSqlResult == NULL) 
        return (USER__NULL_INPUT_ERR);

    collSqlResult->rowCnt = myGenQueryOut->rowCnt;
    collSqlResult->attriCnt = myGenQueryOut->attriCnt;
    collSqlResult->continueInx = myGenQueryOut->continueInx;
    collSqlResult->totalRowCount = myGenQueryOut->totalRowCount;
    
    if ((collName = getSqlResultByInx (myGenQueryOut, COL_COLL_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
          "genQueryOutToCollRes: getSqlResultByInx for COL_COLL_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else {
	collSqlResult->collName = *collName;
    }

    if ((collType = getSqlResultByInx (myGenQueryOut, COL_COLL_TYPE)) == NULL) {
	/* should inherit parent's specColl */
	setSqlResultValue (&collSqlResult->collType, COL_COLL_NAME, 
	  INHERIT_PAR_SPEC_COLL_STR, myGenQueryOut->rowCnt); 
        setSqlResultValue (&collSqlResult->collInfo1, COL_COLL_INFO1, "",
        myGenQueryOut->rowCnt);
        setSqlResultValue (&collSqlResult->collInfo2, COL_COLL_INFO2, "",
        myGenQueryOut->rowCnt);
        setSqlResultValue (&collSqlResult->collOwner, COL_COLL_OWNER_NAME, "",
        myGenQueryOut->rowCnt);
    } else {
	collSqlResult->collType = *collType;
        if ((collInfo1 = getSqlResultByInx (myGenQueryOut, COL_COLL_INFO1)) == 
	  NULL) {
            rodsLog (LOG_ERROR,
              "genQueryOutToCollRes: getSqlResultByInx COL_COLL_INFO1 failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else {
	    collSqlResult->collInfo1 = *collInfo1;
	}
        if ((collInfo2 = getSqlResultByInx (myGenQueryOut, COL_COLL_INFO2)) == 
          NULL) {
            rodsLog (LOG_ERROR,
              "genQueryOutToCollRes: getSqlResultByInx COL_COLL_INFO2 failed");
            free (collSqlResult);
            return (UNMATCHED_KEY_OR_INDEX);
        } else {
            collSqlResult->collInfo2 = *collInfo2;
        }
        if ((collOwner = getSqlResultByInx (myGenQueryOut, 
	  COL_COLL_OWNER_NAME)) == NULL) {
            rodsLog (LOG_ERROR,
              "genQueryOutToCollRes: getSqlResultByInx COL_COLL_OWNER_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        } else {
            collSqlResult->collOwner = *collOwner;
        }
    }

    free (*genQueryOut);
    *genQueryOut = NULL;
    return (0);
}
 
int
setSqlResultValue (sqlResult_t *sqlResult, int attriInx, char *valueStr,
int rowCnt)
{
    if (sqlResult == NULL || rowCnt <= 0) return (0);
	
    sqlResult->attriInx = attriInx;
    if (valueStr == NULL) {
	sqlResult->len = 1;
    } else {
        sqlResult->len = strlen (valueStr) + 1;
    }
    if (sqlResult->len == 1) {
	sqlResult->value = (char *)malloc (rowCnt);
	memset (sqlResult->value, 0, rowCnt);
    } else {
        int i;
	char *tmpPtr;
	tmpPtr = sqlResult->value = (char *)malloc (rowCnt * sqlResult->len);
	for (i = 0; i < rowCnt; i++) {
	    rstrcpy (tmpPtr, valueStr, sqlResult->len);
	    tmpPtr += sqlResult->len;
	}
    }
    return (0); 
}

int
getNextCollMetaInfo (rcComm_t *conn, dataObjInp_t *dataObjInp, 
genQueryInp_t *genQueryInp, collSqlResult_t *collSqlResult, 
int *rowInx, collMetaInfo_t *outCollMetaInfo)
{
    char *value;
    int len;
    char *collType, *collInfo1, *collInfo2;
    int status = 0;

    if (collSqlResult == NULL || outCollMetaInfo == NULL)
        return (USER__NULL_INPUT_ERR);

    if (*rowInx >= collSqlResult->rowCnt) {
	genQueryOut_t *genQueryOut = NULL;
	int continueInx = collSqlResult->continueInx;
	clearCollSqlResult (collSqlResult);
 
        if (continueInx > 0) {
            /* More to come */

            if (dataObjInp->specColl != NULL) {
                dataObjInp->openFlags = continueInx;
                status = rcQuerySpecColl (conn, dataObjInp, &genQueryOut);
            } else {
                genQueryInp->continueInx = continueInx;
                status =  rcGenQuery (conn, genQueryInp, &genQueryOut);
            }
	    if (status < 0) {
		return (status);
	    } else {
		status = genQueryOutToCollRes (&genQueryOut, collSqlResult);
		*rowInx = 0;
		free (genQueryOut);
	    }
	} else {
	    return (CAT_NO_ROWS_FOUND);
	}
    }
    value = collSqlResult->collName.value;
    len = collSqlResult->collName.len;
    outCollMetaInfo->collName = &value[len * (*rowInx)];

    value = collSqlResult->collOwner.value;
    len = collSqlResult->collOwner.len;
    outCollMetaInfo->collOwner = &value[len * (*rowInx)];

    value = collSqlResult->collType.value;
    len = collSqlResult->collType.len;
    collType = &value[len * (*rowInx)];

    if (*collType != '\0') {
        value = collSqlResult->collInfo1.value;
        len = collSqlResult->collInfo1.len;
        collInfo1 = &value[len * (*rowInx)];

        value = collSqlResult->collInfo2.value;
        len = collSqlResult->collInfo2.len;
        collInfo2 = &value[len * (*rowInx)];

	if (strcmp (collType, INHERIT_PAR_SPEC_COLL_STR) == 0) { 
	    if (dataObjInp->specColl == NULL) {
		rodsLog (LOG_ERROR,
		  "getNextCollMetaInfo: parent specColl is NULL for %s",
		  outCollMetaInfo->collName);
		outCollMetaInfo->specColl.collClass = NO_SPEC_COLL;
	    } else {
		outCollMetaInfo->specColl = *dataObjInp->specColl;
	    }
            status = 0;
	} else {
	    status = resolveSpecCollType (collType, outCollMetaInfo->collName,
              collInfo1, collInfo2, &outCollMetaInfo->specColl);
	}
    } else {
	outCollMetaInfo->specColl.collClass = NO_SPEC_COLL;
	status = 0;
    }
    (*rowInx) ++;
    return (status);
}

int
clearCollSqlResult (collSqlResult_t *collSqlResult)
{
    if (collSqlResult == NULL) return (USER__NULL_INPUT_ERR);

    if (collSqlResult->collName.value != NULL) 
      free (collSqlResult->collName.value);
    if (collSqlResult->collType.value != NULL) 
      free (collSqlResult->collType.value);
    if (collSqlResult->collInfo1.value != NULL) 
      free (collSqlResult->collInfo1.value);
    if (collSqlResult->collInfo2.value != NULL) 
      free (collSqlResult->collInfo2.value);
    if (collSqlResult->collOwner.value != NULL) 
      free (collSqlResult->collOwner.value);

    memset (collSqlResult, 0, sizeof (collSqlResult_t));

    return 0;
}

int
clearDataObjSqlResult (dataObjSqlResult_t *dataObjSqlResult)
{
    if (dataObjSqlResult == NULL) return (USER__NULL_INPUT_ERR);

    if (dataObjSqlResult->collName.value != NULL)
      free (dataObjSqlResult->collName.value);
    if (dataObjSqlResult->dataName.value != NULL)
      free (dataObjSqlResult->dataName.value);
    if (dataObjSqlResult->dataSize.value != NULL)
      free (dataObjSqlResult->dataSize.value);
    if (dataObjSqlResult->createTime.value != NULL)
      free (dataObjSqlResult->createTime.value);
    if (dataObjSqlResult->modifyTime.value != NULL)
      free (dataObjSqlResult->modifyTime.value);
    if (dataObjSqlResult->chksum.value != NULL)
      free (dataObjSqlResult->chksum.value);
    if (dataObjSqlResult->replStatus.value != NULL)
      free (dataObjSqlResult->replStatus.value);
    if (dataObjSqlResult->dataId.value != NULL)
      free (dataObjSqlResult->dataId.value);
    if (dataObjSqlResult->resource.value != NULL)
      free (dataObjSqlResult->resource.value);
    if (dataObjSqlResult->phyPath.value != NULL)
      free (dataObjSqlResult->phyPath.value);
    if (dataObjSqlResult->ownerName.value != NULL)
      free (dataObjSqlResult->ownerName.value);
    if (dataObjSqlResult->replNum.value != NULL)
      free (dataObjSqlResult->replNum.value);

    memset (dataObjSqlResult, 0, sizeof (dataObjSqlResult_t));

    return 0;
}

int
genQueryOutToDataObjRes (genQueryOut_t **genQueryOut,
dataObjSqlResult_t *dataObjSqlResult)
{
    genQueryOut_t *myGenQueryOut;
    sqlResult_t *collName, *dataName, *dataSize, *createTime, *modifyTime,
      *chksum, *replStatus, *dataId, *resource, *phyPath, *ownerName, *replNum;

    if (genQueryOut == NULL || (myGenQueryOut = *genQueryOut) == NULL ||
      dataObjSqlResult == NULL)
        return (USER__NULL_INPUT_ERR);

    dataObjSqlResult->rowCnt = myGenQueryOut->rowCnt;
    dataObjSqlResult->attriCnt = myGenQueryOut->attriCnt;
    dataObjSqlResult->continueInx = myGenQueryOut->continueInx;
    dataObjSqlResult->totalRowCount = myGenQueryOut->totalRowCount;

    if ((collName = getSqlResultByInx (myGenQueryOut, COL_COLL_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
         "genQueryOutToDataObjRes: getSqlResultByInx for COL_COLL_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else {
        dataObjSqlResult->collName = *collName;
    }

    if ((dataName = getSqlResultByInx (myGenQueryOut, COL_DATA_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
         "genQueryOutToDataObjRes: getSqlResultByInx for COL_DATA_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else {
        dataObjSqlResult->dataName = *dataName;
    }

    if ((dataSize = getSqlResultByInx (myGenQueryOut, COL_DATA_SIZE)) == NULL) {
	setSqlResultValue (&dataObjSqlResult->dataSize, COL_DATA_SIZE, "-1",
        myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->dataSize = *dataSize;
    }

    if ((createTime = getSqlResultByInx (myGenQueryOut, COL_D_CREATE_TIME)) 
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->createTime, COL_D_CREATE_TIME, 
	  "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->createTime = *createTime;
    }

    if ((modifyTime = getSqlResultByInx (myGenQueryOut, COL_D_MODIFY_TIME)) 
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->modifyTime, COL_D_MODIFY_TIME, 
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->modifyTime = *modifyTime;
    }

    if ((dataId = getSqlResultByInx (myGenQueryOut, COL_D_DATA_ID))
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->dataId, COL_D_DATA_ID,
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->dataId = *dataId;
    }

    if ((chksum = getSqlResultByInx (myGenQueryOut, COL_D_DATA_CHECKSUM)) 
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->chksum, COL_D_DATA_CHECKSUM,
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->chksum = *chksum;
    }

    if ((replStatus = getSqlResultByInx (myGenQueryOut, COL_D_REPL_STATUS)) 
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->replStatus, COL_D_REPL_STATUS, 
	  "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->replStatus = *replStatus;
    }

    if ((resource = getSqlResultByInx (myGenQueryOut, COL_D_RESC_NAME))
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->resource, COL_D_RESC_NAME,
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->resource = *resource;
    }

    if ((phyPath = getSqlResultByInx (myGenQueryOut, COL_D_DATA_PATH))
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->phyPath, COL_D_DATA_PATH,
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->phyPath = *phyPath;
    }

    if ((ownerName = getSqlResultByInx (myGenQueryOut, COL_D_OWNER_NAME))
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->ownerName, COL_D_OWNER_NAME,
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->ownerName = *ownerName;
    }

    if ((replNum = getSqlResultByInx (myGenQueryOut, COL_DATA_REPL_NUM))
      == NULL) {
        setSqlResultValue (&dataObjSqlResult->replNum, COL_DATA_REPL_NUM,
          "", myGenQueryOut->rowCnt);
    } else {
        dataObjSqlResult->replNum = *replNum;
    }

    free (*genQueryOut);
    *genQueryOut = NULL;
    
    return (0);
}

int
getNextDataObjMetaInfo (rcComm_t *conn, dataObjInp_t *dataObjInp,
genQueryInp_t *genQueryInp, dataObjSqlResult_t *dataObjSqlResult,
int *rowInx, dataObjMetaInfo_t *outDataObjMetaInfo)
{
    int status;
    char *value;
    int len;
    int nextInx;
    char *replStatus, *dataId;
    int dataIdLen, replStatusLen;

    if (dataObjSqlResult == NULL || outDataObjMetaInfo == NULL)
        return (USER__NULL_INPUT_ERR);

    if (*rowInx >= dataObjSqlResult->rowCnt) {
        genQueryOut_t *genQueryOut = NULL;
        int continueInx = dataObjSqlResult->continueInx;
        clearDataObjSqlResult (dataObjSqlResult);

        if (continueInx > 0) {
            /* More to come */

            if (dataObjInp->specColl != NULL) {
                dataObjInp->openFlags = continueInx;
                status = rcQuerySpecColl (conn, dataObjInp, &genQueryOut);
            } else {
                genQueryInp->continueInx = continueInx;
                status =  rcGenQuery (conn, genQueryInp, &genQueryOut);
            }
            if (status < 0) {
                return (status);
            } else {
                status = genQueryOutToDataObjRes (&genQueryOut, 
		  dataObjSqlResult);
                *rowInx = 0;
                free (genQueryOut);
            }
        } else {
            return (CAT_NO_ROWS_FOUND);
        }
    }

    dataId = dataObjSqlResult->dataId.value;
    dataIdLen = dataObjSqlResult->dataId.len;
    replStatus = dataObjSqlResult->replStatus.value;
    replStatusLen = dataObjSqlResult->replStatus.len;

    if (strlen (dataId) > 0) {
        int i;
        char *prevdataId = NULL;
        int gotCopy = 0;

	/* rsync type query ask for dataId. Others don't. Need to 
	 * screen out dup copies */

        for (i = *rowInx; i < dataObjSqlResult->rowCnt; i++) {
            if (prevdataId != NULL) {
                if (strcmp (prevdataId, &dataId[dataIdLen * i]) != 0) {
                    break;
                }
            } else {
                prevdataId = &dataId[dataIdLen * i];
		*rowInx = i;
            }

            if (gotCopy == 0 &&
              atoi (&replStatus[replStatusLen * i]) > 0) {
	        *rowInx = i;
                gotCopy = 1;
            }
        }
	nextInx = i;
    } else {
	nextInx = (*rowInx) + 1;
    }

    value = dataObjSqlResult->collName.value;
    len = dataObjSqlResult->collName.len;
    outDataObjMetaInfo->collName = &value[len * (*rowInx)];

    value = dataObjSqlResult->dataName.value;
    len = dataObjSqlResult->dataName.len;
    outDataObjMetaInfo->dataName = &value[len * (*rowInx)];

    value = dataObjSqlResult->dataSize.value;
    len = dataObjSqlResult->dataSize.len;
    outDataObjMetaInfo->dataSize = &value[len * (*rowInx)];

    value = dataObjSqlResult->createTime.value;
    len = dataObjSqlResult->createTime.len;
    outDataObjMetaInfo->createTime = &value[len * (*rowInx)];

    value = dataObjSqlResult->modifyTime.value;
    len = dataObjSqlResult->modifyTime.len;
    outDataObjMetaInfo->modifyTime = &value[len * (*rowInx)];

    outDataObjMetaInfo->dataId = &dataId[dataIdLen * (*rowInx)];

    outDataObjMetaInfo->replStatus = &replStatus[replStatusLen * (*rowInx)];

    value = dataObjSqlResult->chksum.value;
    len = dataObjSqlResult->chksum.len;
    outDataObjMetaInfo->chksum = &value[len * (*rowInx)];

    (*rowInx) = nextInx;
    return (0);
}

int
rclOpenCollection (rcComm_t *conn, char *collection, int flag,
collHandle_t *collHandle)
{
    rodsObjStat_t *rodsObjStatOut = NULL;
    int status;

    if (conn == NULL || collection == NULL || collHandle == NULL) {
	rodsLog (LOG_ERROR,
          "rclOpenCollection: NULL conn, collection or collHandle input");
	return USER__NULL_INPUT_ERR;
    }

    memset (collHandle, 0, sizeof (collHandle_t));

    rstrcpy (collHandle->dataObjInp.objPath, collection, MAX_NAME_LEN);
    status = rcObjStat (conn, &collHandle->dataObjInp, &rodsObjStatOut);

    
    if (status < 0) return status;
	
    if (rodsObjStatOut->objType != COLL_OBJ_T) {
	free (rodsObjStatOut);
	return CAT_UNKNOWN_COLLECTION;
    }

    collHandle->dataObjInp.specColl = rodsObjStatOut->specColl;

    free (rodsObjStatOut);

    collHandle->state = COLL_OPENED;
    collHandle->flag = flag;
    /* the collection exist. now query the data in it */
    return (0);
}

int
rclReadCollection (rcComm_t *conn, collHandle_t *collHandle,  
collEnt_t *collEnt)
{
    int status = 0;
    genQueryOut_t *genQueryOut = NULL;
    collMetaInfo_t collMetaInfo;
    dataObjMetaInfo_t dataObjMetaInfo;
    rodsArguments_t rodsArgs;

    if (conn == NULL || collHandle == NULL || collEnt == NULL) {
        rodsLog (LOG_ERROR,
          "rclReadCollection: NULL conn or collHandle input");
        return USER__NULL_INPUT_ERR;
    }

    memset (collEnt, 0, sizeof (collEnt_t));

    if ((collHandle->flag & VERY_LONG_METADATA_FG) != 0) {
        rodsArgs.longOption = True;
        rodsArgs.veryLongOption = True;
    } else if ((collHandle->flag & LONG_METADATA_FG) != 0) {
        rodsArgs.longOption = True;
    } else {
        rodsArgs.longOption = False;
    }

    if (collHandle->state == COLL_CLOSED) return (CAT_NO_ROWS_FOUND);

    if ((collHandle->flag & RECUR_QUERY_FG) != 0) {
	/* recursive - coll first, dataObj second */
        if (collHandle->state == COLL_OPENED) {
	    status = genCollResInColl (conn, collHandle);
	}

        if (collHandle->state == COLL_COLL_OBJ_QUERIED) {
            memset (&collMetaInfo, 0, sizeof (collMetaInfo));
            status = newGetNextCollMetaInfo (conn, &collHandle->dataObjInp,
              &collHandle->genQueryInp, &collHandle->collSqlResult,
              &collHandle->rowInx, collEnt);
            if (status >= 0) {
                return status;
            } else {
                if (status != CAT_NO_ROWS_FOUND) {
                    rodsLog (LOG_ERROR,
                      "rclReadCollection: getNextCollMetaInfo error for %s. status = %d",
                      collHandle->dataObjInp.objPath, status);
                }
                if (collHandle->dataObjInp.specColl == NULL) {
                    clearGenQueryInp (&collHandle->genQueryInp);
                }
            }
	    status = genDataResInColl (conn, collHandle);
        }
        if (collHandle->state == COLL_DATA_OBJ_QUERIED) {
            memset (&dataObjMetaInfo, 0, sizeof (dataObjMetaInfo));
            status = newGetNextDataObjMetaInfo (conn, &collHandle->dataObjInp,
              &collHandle->genQueryInp, &collHandle->dataObjSqlResult,
              &collHandle->rowInx, collEnt);

            if (status >= 0) {
                return status;
            } else {
                if (status != CAT_NO_ROWS_FOUND) {
                    rodsLog (LOG_ERROR,
                      "rclReadCollection: getNextDataObjMetaInfo error for %s. status = %d",
                      collHandle->dataObjInp.objPath, status);
                }
                /* cleanup */
                if (collHandle->dataObjInp.specColl == NULL) {
                    clearGenQueryInp (&collHandle->genQueryInp);
                }
                /* Nothing else to do. cleanup */
                clearCollHandle (collHandle);
            }
            return status;
        }
    } else {
        if (collHandle->state == COLL_OPENED) {
	    status = genDataResInColl (conn, collHandle);
        }

        if (collHandle->state == COLL_DATA_OBJ_QUERIED) {
            memset (&dataObjMetaInfo, 0, sizeof (dataObjMetaInfo));
            status = newGetNextDataObjMetaInfo (conn, &collHandle->dataObjInp,
              &collHandle->genQueryInp, &collHandle->dataObjSqlResult,
              &collHandle->rowInx, collEnt);

            if (status >= 0) {
                return status;
            } else {
                if (status != CAT_NO_ROWS_FOUND) {
                    rodsLog (LOG_ERROR,
                      "rclReadCollection: getNextDataObjMetaInfo error for %s. status = %d",
                      collHandle->dataObjInp.objPath, status);
                }
                /* cleanup */
                if (collHandle->dataObjInp.specColl == NULL) {
                    clearGenQueryInp (&collHandle->genQueryInp);
                }
            }

	    status = genCollResInColl (conn, collHandle);
        }

        if (collHandle->state == COLL_COLL_OBJ_QUERIED) {
            memset (&collMetaInfo, 0, sizeof (collMetaInfo));
            status = newGetNextCollMetaInfo (conn, &collHandle->dataObjInp,
              &collHandle->genQueryInp, &collHandle->collSqlResult,
              &collHandle->rowInx, collEnt);
	    if (status < 0) {
                if (status != CAT_NO_ROWS_FOUND) {
                    rodsLog (LOG_ERROR,
                      "rclReadCollection: getNextCollMetaInfo error for %s. status = %d",
                      collHandle->dataObjInp.objPath, status);
                }
                /* cleanup */
                if (collHandle->dataObjInp.specColl == NULL) {
                    clearGenQueryInp (&collHandle->genQueryInp);
                }
                /* Nothing else to do. cleanup */
	        clearCollHandle (collHandle);
            }
	    return status;
        }
    }
    return (CAT_NO_ROWS_FOUND);
}


int
genCollResInColl (rcComm_t *conn, collHandle_t *collHandle)
{
    genQueryOut_t *genQueryOut = NULL;
    int status = 0;
    rodsArguments_t rodsArgs;

    /* query for sub-collections */
    if (collHandle->dataObjInp.specColl != NULL) {
        /* query */
        addKeyVal (&collHandle->dataObjInp.condInput,
          SEL_OBJ_TYPE_KW, "collection");
        status = rcQuerySpecColl (conn, &collHandle->dataObjInp,
          &genQueryOut);
    } else {
        memset (&collHandle->genQueryInp, 0, sizeof (genQueryInp_t));
        if ((collHandle->flag & RECUR_QUERY_FG) != 0) {
            status = queryCollInCollReCur (conn,
              collHandle->dataObjInp.objPath, &rodsArgs,
              &collHandle->genQueryInp, &genQueryOut);
        } else {
            status = queryCollInColl (conn,
              collHandle->dataObjInp.objPath, &rodsArgs,
              &collHandle->genQueryInp, &genQueryOut);
        }
    }

    collHandle->rowInx = 0;
    collHandle->state = COLL_COLL_OBJ_QUERIED;
    if (status >= 0) {
        status = genQueryOutToCollRes (&genQueryOut,
          &collHandle->collSqlResult);
    } else if (status != CAT_NO_ROWS_FOUND) {
        rodsLog (LOG_ERROR,
          "genCollResInColl: query collection error for %s. status = %d",
          collHandle->dataObjInp.objPath, status);
    }
    return status;
}

int
genDataResInColl (rcComm_t *conn, collHandle_t *collHandle)
{
    genQueryOut_t *genQueryOut = NULL;
    int status = 0;
    rodsArguments_t rodsArgs;

    if (collHandle->dataObjInp.specColl != NULL) {
        /* query */
        addKeyVal (&collHandle->dataObjInp.condInput,
          SEL_OBJ_TYPE_KW, "dataObj");
        status = rcQuerySpecColl (conn, &collHandle->dataObjInp,
          &genQueryOut);
    } else {
        memset (&collHandle->genQueryInp, 0, sizeof (genQueryInp_t));
        if ((collHandle->flag & RECUR_QUERY_FG) != 0) {
            status = queryDataObjInCollReCur (conn,
              collHandle->dataObjInp.objPath, &rodsArgs,
              &collHandle->genQueryInp, &genQueryOut);
        } else {
            status = queryDataObjInColl (conn,
              collHandle->dataObjInp.objPath, &rodsArgs,
              &collHandle->genQueryInp, &genQueryOut);
        }
    }

    collHandle->rowInx = 0;
    collHandle->state = COLL_DATA_OBJ_QUERIED;
    if (status >= 0) {
        status = genQueryOutToDataObjRes (&genQueryOut,
          &collHandle->dataObjSqlResult);
    } else if (status != CAT_NO_ROWS_FOUND) {
        rodsLog (LOG_ERROR,
          "genDataResInColl: query dataObj error for %s. status = %d",
          collHandle->dataObjInp.objPath, status);
    }
    return status;
}

int
rclCloseCollection (collHandle_t *collHandle)
{
    return (clearCollHandle (collHandle));
}

int
clearCollHandle (collHandle_t *collHandle)
{
    if (collHandle == NULL) return 0;
    if (collHandle->dataObjInp.specColl == NULL) {
        clearGenQueryInp (&collHandle->genQueryInp);
    }
    if (collHandle->dataObjInp.specColl != NULL) {
        free (collHandle->dataObjInp.specColl);
    }
    clearKeyVal (&collHandle->dataObjInp.condInput);
    memset (&collHandle->dataObjInp, 0,  sizeof (dataObjInp_t));

    clearDataObjSqlResult (&collHandle->dataObjSqlResult);
    clearCollSqlResult (&collHandle->collSqlResult);

    collHandle->state = COLL_CLOSED;
    collHandle->rowInx = 0;

    return (0);
}

int
newGetNextCollMetaInfo (rcComm_t *conn, dataObjInp_t *dataObjInp,
genQueryInp_t *genQueryInp, collSqlResult_t *collSqlResult,
int *rowInx, collEnt_t *outCollEnt)
{
    char *value;
    int len;
    char *collType, *collInfo1, *collInfo2;
    int status = 0;

    if (collSqlResult == NULL || outCollEnt == NULL)
        return (USER__NULL_INPUT_ERR);

    memset (outCollEnt, 0, sizeof (collEnt_t));

    outCollEnt->objType = COLL_OBJ_T;
    if (*rowInx >= collSqlResult->rowCnt) {
        genQueryOut_t *genQueryOut = NULL;
        int continueInx = collSqlResult->continueInx;
        clearCollSqlResult (collSqlResult);

        if (continueInx > 0) {
            /* More to come */

            if (dataObjInp->specColl != NULL) {
                dataObjInp->openFlags = continueInx;
                status = rcQuerySpecColl (conn, dataObjInp, &genQueryOut);
            } else {
                genQueryInp->continueInx = continueInx;
                status =  rcGenQuery (conn, genQueryInp, &genQueryOut);
            }
            if (status < 0) {
                return (status);
            } else {
                status = genQueryOutToCollRes (&genQueryOut, collSqlResult);
                *rowInx = 0;
                free (genQueryOut);
            }
        } else {
            return (CAT_NO_ROWS_FOUND);
        }
    }
    value = collSqlResult->collName.value;
    len = collSqlResult->collName.len;
    outCollEnt->collName = &value[len * (*rowInx)];

    value = collSqlResult->collOwner.value;
    len = collSqlResult->collOwner.len;
    outCollEnt->ownerName = &value[len * (*rowInx)];

    value = collSqlResult->collType.value;
    len = collSqlResult->collType.len;
    collType = &value[len * (*rowInx)];

    if (*collType != '\0') {
        value = collSqlResult->collInfo1.value;
        len = collSqlResult->collInfo1.len;
        collInfo1 = &value[len * (*rowInx)];

        value = collSqlResult->collInfo2.value;
        len = collSqlResult->collInfo2.len;
        collInfo2 = &value[len * (*rowInx)];

        if (strcmp (collType, INHERIT_PAR_SPEC_COLL_STR) == 0) {
            if (dataObjInp->specColl == NULL) {
                rodsLog (LOG_ERROR,
                  "getNextCollMetaInfo: parent specColl is NULL for %s",
                  outCollEnt->collName);
                outCollEnt->specColl.collClass = NO_SPEC_COLL;
            } else {
                outCollEnt->specColl = *dataObjInp->specColl;
            }
            status = 0;
        } else {
            status = resolveSpecCollType (collType, outCollEnt->collName,
              collInfo1, collInfo2, &outCollEnt->specColl);
        }
    } else {
        outCollEnt->specColl.collClass = NO_SPEC_COLL;
        status = 0;
    }
    (*rowInx) ++;
    return (status);
}

int
newGetNextDataObjMetaInfo (rcComm_t *conn, dataObjInp_t *dataObjInp,
genQueryInp_t *genQueryInp, dataObjSqlResult_t *dataObjSqlResult,
int *rowInx, collEnt_t *outCollEnt)
{
    int status;
    char *value;
    int len;
    int nextInx;
    char *replStatus, *dataId;
    int dataIdLen, replStatusLen;

    if (dataObjSqlResult == NULL || outCollEnt == NULL)
        return (USER__NULL_INPUT_ERR);

    memset (outCollEnt, 0, sizeof (collEnt_t));
    outCollEnt->objType = DATA_OBJ_T;
    if (*rowInx >= dataObjSqlResult->rowCnt) {
        genQueryOut_t *genQueryOut = NULL;
        int continueInx = dataObjSqlResult->continueInx;
        clearDataObjSqlResult (dataObjSqlResult);

        if (continueInx > 0) {
            /* More to come */

            if (dataObjInp->specColl != NULL) {
                dataObjInp->openFlags = continueInx;
                status = rcQuerySpecColl (conn, dataObjInp, &genQueryOut);
            } else {
                genQueryInp->continueInx = continueInx;
                status =  rcGenQuery (conn, genQueryInp, &genQueryOut);
            }
            if (status < 0) {
                return (status);
            } else {
                status = genQueryOutToDataObjRes (&genQueryOut,
                  dataObjSqlResult);
                *rowInx = 0;
                free (genQueryOut);
            }
        } else {
            return (CAT_NO_ROWS_FOUND);
        }
    }

    dataId = dataObjSqlResult->dataId.value;
    dataIdLen = dataObjSqlResult->dataId.len;
    replStatus = dataObjSqlResult->replStatus.value;
    replStatusLen = dataObjSqlResult->replStatus.len;

    if (strlen (dataId) > 0) {
        int i;
        char *prevdataId = NULL;
        int gotCopy = 0;

        /* rsync type query ask for dataId. Others don't. Need to
         * screen out dup copies */

        for (i = *rowInx; i < dataObjSqlResult->rowCnt; i++) {
            if (prevdataId != NULL) {
                if (strcmp (prevdataId, &dataId[dataIdLen * i]) != 0) {
                    break;
                }
            } else {
                prevdataId = &dataId[dataIdLen * i];
                *rowInx = i;
            }

            if (gotCopy == 0 &&
              atoi (&replStatus[replStatusLen * i]) > 0) {
                *rowInx = i;
                gotCopy = 1;
            }
        }
        nextInx = i;
    } else {
        nextInx = (*rowInx) + 1;
    }

    value = dataObjSqlResult->collName.value;
    len = dataObjSqlResult->collName.len;
    outCollEnt->collName = &value[len * (*rowInx)];

    value = dataObjSqlResult->dataName.value;
    len = dataObjSqlResult->dataName.len;
    outCollEnt->dataName = &value[len * (*rowInx)];

    value = dataObjSqlResult->dataSize.value;
    len = dataObjSqlResult->dataSize.len;
    outCollEnt->dataSize = strtoll (&value[len * (*rowInx)], 0, 0);

    value = dataObjSqlResult->createTime.value;
    len = dataObjSqlResult->createTime.len;
    outCollEnt->createTime = &value[len * (*rowInx)];

    value = dataObjSqlResult->modifyTime.value;
    len = dataObjSqlResult->modifyTime.len;
    outCollEnt->modifyTime = &value[len * (*rowInx)];

    outCollEnt->dataId = &dataId[dataIdLen * (*rowInx)];

    outCollEnt->replStatus = atoi (&replStatus[replStatusLen * (*rowInx)]);

    value = dataObjSqlResult->replNum.value;
    len = dataObjSqlResult->replNum.len;
    outCollEnt->replNum = atoi (&value[len * (*rowInx)]);

    value = dataObjSqlResult->chksum.value;
    len = dataObjSqlResult->chksum.len;
    outCollEnt->chksum = &value[len * (*rowInx)];

    value = dataObjSqlResult->resource.value;
    len = dataObjSqlResult->resource.len;
    outCollEnt->resource = &value[len * (*rowInx)];

    value = dataObjSqlResult->ownerName.value;
    len = dataObjSqlResult->ownerName.len;
    outCollEnt->ownerName = &value[len * (*rowInx)];

    value = dataObjSqlResult->phyPath.value;
    len = dataObjSqlResult->phyPath.len;
    outCollEnt->phyPath = &value[len * (*rowInx)];

    (*rowInx) = nextInx;
    return (0);
}

