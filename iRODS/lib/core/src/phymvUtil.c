/*** Copyright (c), The Regents of the University of California            ***
 *** For more inforeplation please refer to files in the COPYRIGHT directory ***/
#include "rodsPath.h"
#include "rodsErrorTable.h"
#include "rodsLog.h"
#include "miscUtil.h"
#include "phymvUtil.h"

int
phymvUtil (rcComm_t *conn, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs,
rodsPathInp_t *rodsPathInp)
{
    int i;
    int status; 
    int savedStatus = 0;
    dataObjInp_t dataObjInp;


    if (rodsPathInp == NULL) {
	return (USER__NULL_INPUT_ERR);
    }

    initCondForPhymv (myRodsEnv, myRodsArgs, &dataObjInp);

    for (i = 0; i < rodsPathInp->numSrc; i++) {
	if (rodsPathInp->srcPath[i].objType == UNKNOWN_OBJ_T) {
	    getRodsObjType (conn, &rodsPathInp->srcPath[i]);
	    if (rodsPathInp->srcPath[i].objState == NOT_EXIST_ST) {
                rodsLog (LOG_ERROR,
                  "phymvUtil: srcPath %s does not exist",
                  rodsPathInp->srcPath[i].outPath);
		savedStatus = USER_INPUT_PATH_ERR;
		continue;
	    }
	}

	if (rodsPathInp->srcPath[i].objType == DATA_OBJ_T) {
	    status = phymvDataObjUtil (conn, rodsPathInp->srcPath[i].outPath, 
	     myRodsEnv, myRodsArgs, &dataObjInp);
	} else if (rodsPathInp->srcPath[i].objType ==  COLL_OBJ_T) {
	    status = phymvCollUtil (conn, rodsPathInp->srcPath[i].outPath,
              myRodsEnv, myRodsArgs, &dataObjInp);
	} else {
	    /* should not be here */
	    rodsLog (LOG_ERROR,
	     "phymvUtil: invalid phymv objType %d for %s", 
	     rodsPathInp->srcPath[i].objType, rodsPathInp->srcPath[i].outPath);
	    return (USER_INPUT_PATH_ERR);
	}
	/* XXXX may need to return a global status */
	if (status < 0) {
	    rodsLogError (LOG_ERROR, status,
             "phymvUtil: phymv error for %s, status = %d", 
	      rodsPathInp->srcPath[i].outPath, status);
	    savedStatus = status;
	} 
    }
    if (savedStatus < 0) {
        return (savedStatus);
    } else if (status == CAT_NO_ROWS_FOUND) {
        return (0);
    } else {
        return (status);
    }
}

int
phymvDataObjUtil (rcComm_t *conn, char *srcPath, 
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjInp_t *dataObjInp)
{
    int status;
    struct timeval startTime, endTime;
 
    if (srcPath == NULL) {
       rodsLog (LOG_ERROR,
          "phymvDataObjUtil: NULL srcPath input");
        return (USER__NULL_INPUT_ERR);
    }

    if (rodsArgs->verbose == True) {
        (void) gettimeofday(&startTime, (struct timezone *)0);
    }

    rstrcpy (dataObjInp->objPath, srcPath, MAX_NAME_LEN);

    status = rcDataObjPhymv (conn, dataObjInp);

    if (status >= 0 && rodsArgs->verbose == True) {
        (void) gettimeofday(&endTime, (struct timezone *)0);
        printTiming (conn, dataObjInp->objPath, conn->transStat.bytesWritten, 
	  NULL, &startTime, &endTime);
    }

    return (status);
}

int
initCondForPhymv (rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjInp_t *dataObjInp)
{
    char *myResc = NULL;

    if (dataObjInp == NULL) {
       rodsLog (LOG_ERROR,
          "initCondForPhymv: NULL dataObjInp input");
        return (USER__NULL_INPUT_ERR);
    }

    memset (dataObjInp, 0, sizeof (dataObjInp_t));

    if (rodsArgs == NULL) {
	return (0);
    }

    if (rodsArgs->admin == True) {
        addKeyVal (&dataObjInp->condInput, IRODS_ADMIN_KW, "");
    }

    if (rodsArgs->replNum == True) {
        addKeyVal (&dataObjInp->condInput, REPL_NUM_KW, 
	  rodsArgs->replNumValue);
    }

    if (rodsArgs->srcResc == True) {
        addKeyVal (&dataObjInp->condInput, RESC_NAME_KW,
          rodsArgs->srcRescString);
    }

    if (rodsArgs->resource == True) {
        if (rodsArgs->resourceString == NULL) {
            rodsLog (LOG_ERROR,
              "initCondForPhymv: NULL resourceString error");
            return (USER__NULL_INPUT_ERR);
        } else {
	    myResc = rodsArgs->resourceString;
            addKeyVal (&dataObjInp->condInput, DEST_RESC_NAME_KW,
              rodsArgs->resourceString);
        }
    } else if (myRodsEnv != NULL && strlen (myRodsEnv->rodsDefResource) > 0) {
	myResc = myRodsEnv->rodsDefResource;
        addKeyVal (&dataObjInp->condInput, DEST_RESC_NAME_KW,
          myRodsEnv->rodsDefResource);
    }

    return (0);
}

int
phymvCollUtil (rcComm_t *conn, char *srcColl, rodsEnv *myRodsEnv, 
rodsArguments_t *rodsArgs, dataObjInp_t *dataObjInp)
{
    int status;
    int savedStatus = 0;
    genQueryInp_t genQueryInp;
    char srcChildPath[MAX_NAME_LEN];
    genQueryOut_t *genQueryOut = NULL;
    int collLen;
    int rowInx;
    dataObjSqlResult_t dataObjSqlResult;
    dataObjMetaInfo_t dataObjMetaInfo;

    if (srcColl == NULL) {
       rodsLog (LOG_ERROR,
          "phymvCollUtil: NULL srcColl input");
        return (USER__NULL_INPUT_ERR);
    }

    if (rodsArgs->recursive != True) {
        rodsLog (LOG_ERROR,
        "phymvCollUtil: -r option must be used for getting %s collection",
         srcColl);
        return (USER_INPUT_OPTION_ERR);
    }

    if (rodsArgs->verbose == True) {
        fprintf (stdout, "C- %s:\n", srcColl);
    }

    collLen = strlen (srcColl);

    /* Now get all the files */

    memset (&genQueryInp, 0, sizeof (genQueryInp));
    status = queryDataObjInCollReCur (conn, srcColl, rodsArgs, &genQueryInp,
      &genQueryOut);

    if (status >= 0) {
        status = genQueryOutToDataObjRes (&genQueryOut, &dataObjSqlResult);
    }

    rowInx = 0;
    while (status >= 0 &&
      (status = getNextDataObjMetaInfo (conn, dataObjInp, &genQueryInp,
      &dataObjSqlResult, &rowInx, &dataObjMetaInfo)) >= 0) {

        snprintf (srcChildPath, MAX_NAME_LEN, "%s/%s",
          dataObjMetaInfo.collName, dataObjMetaInfo.dataName);

        status = phymvDataObjUtil (conn, srcChildPath,
         myRodsEnv, rodsArgs, dataObjInp);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "phymvCollUtil: phymvDataObjUtil failed for %s. status = %d",
              srcChildPath, status);
            /* need to set global error here */
            savedStatus = status;
	    status = 0;
        }
    }

#if 0
    while (status >= 0) {
	sqlResult_t *subColl, *dataObj;

        if ((subColl = getSqlResultByInx (genQueryOut, COL_COLL_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "phymvCollUtil: getSqlResultByInx for COL_COLL_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        if ((dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "phymvCollUtil: getSqlResultByInx for COL_DATA_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

	for (i = 0; i < genQueryOut->rowCnt; i++) {
	    char *tmpSubColl, *tmpDataName;

	    tmpSubColl = &subColl->value[subColl->len * i];
	    tmpDataName = &dataObj->value[dataObj->len * i];

	    snprintf (srcChildPath, MAX_NAME_LEN, "%s/%s",
	      tmpSubColl, tmpDataName);
	    status = phymvDataObjUtil (conn, srcChildPath,
             myRodsEnv, rodsArgs, dataObjInp);
	    if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                  "phymvCollUtil: phymvDataObjUtil failed for %s. status = %d",
	          srcChildPath, status);
		/* need to set global error here */
		savedStatus = status;
	    }
	}

	continueInx = genQueryOut->continueInx;

	freeGenQueryOut (&genQueryOut);

	if (continueInx > 0) {
	    /* More to come */
	    genQueryInp.continueInx = continueInx;
            status =  rcGenQuery (conn, &genQueryInp, &genQueryOut);
	} else {
	    break;
	}
    }
#endif

    if (savedStatus < 0) {
	return (savedStatus);
    } else if (status == CAT_NO_ROWS_FOUND) {
	return (0);
    } else {
        return (status);
    }
}

