/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include <sys/time.h>
#include "rodsPath.h"
#include "rodsErrorTable.h"
#include "rodsLog.h"
#include "lsUtil.h"
#include "getUtil.h"
#include "miscUtil.h"

int
getUtil (rcComm_t *conn, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs,
rodsPathInp_t *rodsPathInp)
{
    int i;
    int status; 
    int savedStatus = 0;
    rodsPath_t *targPath;
    dataObjInp_t dataObjOprInp;
    rodsRestart_t rodsRestart;

    if (rodsPathInp == NULL) {
	return (USER__NULL_INPUT_ERR);
    }

    initCondForGet (myRodsEnv, myRodsArgs, &dataObjOprInp, &rodsRestart);

    status = resolveRodsTarget (conn, myRodsEnv, rodsPathInp, 1);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "getUtil: resolveRodsTarget");
        return (status);
    }

    for (i = 0; i < rodsPathInp->numSrc; i++) {
        targPath = &rodsPathInp->targPath[i];

	if (rodsPathInp->srcPath[i].rodsObjStat != NULL &&
	  rodsPathInp->srcPath[i].rodsObjStat->specColl != NULL) {
	    dataObjOprInp.specColl = 
	      rodsPathInp->srcPath[i].rodsObjStat->specColl;
	} else {
	    dataObjOprInp.specColl = NULL;
	}
	if (targPath->objType == LOCAL_FILE_T) {
	    status = getDataObjUtil (conn, rodsPathInp->srcPath[i].outPath, 
	     targPath->outPath, rodsPathInp->srcPath[i].size, myRodsEnv, 
	      myRodsArgs, &dataObjOprInp);
	} else if (targPath->objType ==  LOCAL_DIR_T) {
            setStateForRestart (conn, &rodsRestart, targPath, myRodsArgs);
	    status = getCollUtil (conn, rodsPathInp->srcPath[i].outPath,
              targPath->outPath, myRodsEnv, myRodsArgs, &dataObjOprInp,
	      &rodsRestart);
            if (rodsRestart.fd > 0 && status < 0) {
                close (rodsRestart.fd);
                return (status);
            }
            if (dataObjOprInp.specColl != NULL &&
	     dataObjOprInp.specColl->class == BUNDLE_COLL) {
		dataObjOprInp.specColl = NULL;
                status = getCollUtil (conn, rodsPathInp->srcPath[i].outPath,
                  targPath->outPath, myRodsEnv, myRodsArgs, &dataObjOprInp,
                  &rodsRestart);
                if (rodsRestart.fd > 0 && status < 0) {
                    close (rodsRestart.fd);
                    return (status);
                }
	    }
	} else {
	    /* should not be here */
	    rodsLog (LOG_ERROR,
	     "getUtil: invalid get dest objType %d for %s", 
	      targPath->objType, targPath->outPath);
	    return (USER_INPUT_PATH_ERR);
	}
	/* XXXX may need to return a global status */
	if (status < 0) {
            rodsLogError (LOG_ERROR, status,
             "getUtil: get error for %s", 
	      targPath->outPath);
	    savedStatus = status;
	} 
    }

    if (rodsRestart.fd > 0) {
        close (rodsRestart.fd);
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
getDataObjUtil (rcComm_t *conn, char *srcPath, char *targPath, 
rodsLong_t srcSize, rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjInp_t *dataObjOprInp)
{
    int status;
    struct timeval startTime, endTime;
 
    if (srcPath == NULL || targPath == NULL) {
       rodsLog (LOG_ERROR,
          "getDataObjUtil: NULL srcPath or targPath input");
        return (USER__NULL_INPUT_ERR);
    }

    if (rodsArgs->verbose == True) {
        (void) gettimeofday(&startTime, (struct timezone *)0);
    }


    rstrcpy (dataObjOprInp->objPath, srcPath, MAX_NAME_LEN);
    /* XXXXX need to modify rcDataObjGet to verify dataSize if given */
    dataObjOprInp->dataSize = srcSize;

    status = rcDataObjGet (conn, dataObjOprInp, targPath);

    if (status >= 0 && rodsArgs->verbose == True) {
        (void) gettimeofday(&endTime, (struct timezone *)0);
        printTiming (conn, dataObjOprInp->objPath, srcSize, targPath,
         &startTime, &endTime);
    }

    return (status);
}

int
initCondForGet (rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjInp_t *dataObjOprInp, rodsRestart_t *rodsRestart)
{
    if (dataObjOprInp == NULL) {
       rodsLog (LOG_ERROR,
          "initCondForGet: NULL dataObjOprInp input");
        return (USER__NULL_INPUT_ERR);
    }

    memset (dataObjOprInp, 0, sizeof (dataObjInp_t));

    if (rodsArgs == NULL) {
	return (0);
    }

    if (rodsArgs->force == True) { 
        addKeyVal (&dataObjOprInp->condInput, FORCE_FLAG_KW, "");
    }

    if (rodsArgs->verifyChecksum == True) {
        addKeyVal (&dataObjOprInp->condInput, VERIFY_CHKSUM_KW, "");
    }

    if (rodsArgs->number == True) {
	if (rodsArgs->numberValue == 0) {
	    dataObjOprInp->numThreads = NO_THREADING;
	} else {
	    dataObjOprInp->numThreads = rodsArgs->numberValue;
	}
    }

    if (rodsArgs->replNum == True) {
        addKeyVal (&dataObjOprInp->condInput, REPL_NUM_KW, 
	  rodsArgs->replNumValue);
    }

    memset (rodsRestart, 0, sizeof (rodsRestart_t));
    if (rodsArgs->restart == True) {
        int status;
        status = openRestartFile (rodsArgs->restartFileString, rodsRestart,
          rodsArgs);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "initCondForPut: openRestartFile of %s errno",
            rodsArgs->restartFileString);
            return (status);
        }
    }

    dataObjOprInp->openFlags = O_RDONLY;

    return (0);
}

int
getCollUtil (rcComm_t *conn, char *srcColl, char *targDir, 
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, dataObjInp_t *dataObjOprInp,
rodsRestart_t *rodsRestart)
{
    int status = 0; 
    int savedStatus = 0;
    genQueryInp_t genQueryInp;
    char srcChildPath[MAX_NAME_LEN], targChildPath[MAX_NAME_LEN];
    genQueryOut_t *genQueryOut = NULL;
    int collLen;
    int rowInx;
    collSqlResult_t collSqlResult;
    collMetaInfo_t collMetaInfo;
    dataObjSqlResult_t dataObjSqlResult;
    dataObjMetaInfo_t dataObjMetaInfo;

    if (srcColl == NULL || targDir == NULL) {
       rodsLog (LOG_ERROR,
          "getCollUtil: NULL srcColl or targDir input");
        return (USER__NULL_INPUT_ERR);
    }

    if (rodsArgs->recursive != True) {
        rodsLog (LOG_ERROR,
        "getCollUtil: -r option must be used for getting %s collection",
         targDir);
        return (USER_INPUT_OPTION_ERR);
    }

    collLen = strlen (srcColl);

    printCollOrDir (targDir, LOCAL_DIR_T, rodsArgs, dataObjOprInp->specColl);

    if (dataObjOprInp->specColl != NULL) {
	/* do the collection */
        addKeyVal (&dataObjOprInp->condInput, SEL_OBJ_TYPE_KW, "collection");
        rstrcpy (dataObjOprInp->objPath, srcColl, MAX_NAME_LEN);
        status = rcQuerySpecColl (conn, dataObjOprInp, &genQueryOut);
    } else {
        /* query all sub collections in srcColl and the mk the required
         * subdirectories */ 

        status = queryCollInCollReCur (conn, srcColl, rodsArgs, &genQueryInp,
          &genQueryOut);
    }

    if (status >= 0) {
	status = genQueryOutToCollRes (&genQueryOut, &collSqlResult);
    }

    rowInx = 0;
    while (status >= 0 && 
      (status = getNextCollMetaInfo (conn, dataObjOprInp, &genQueryInp,
      &collSqlResult, &rowInx, &collMetaInfo)) >= 0) {

        snprintf (targChildPath, MAX_NAME_LEN, "%s/%s",
          targDir, collMetaInfo.collName + collLen);

#ifdef _WIN32
        mkdirR (targDir, targChildPath);
#else
        mkdirR (targDir, targChildPath, 0750);
#endif

	if (collMetaInfo.specColl.class != NO_SPEC_COLL) {
	    /* the child is a spec coll. need to drill down */
            dataObjInp_t childDataObjInp;
            childDataObjInp = *dataObjOprInp;
            childDataObjInp.specColl = &collMetaInfo.specColl;
            status = getCollUtil (conn, collMetaInfo.collName, targChildPath,
              myRodsEnv, rodsArgs, &childDataObjInp, rodsRestart);
            if (status < 0 && status != CAT_NO_ROWS_FOUND) {
                return (status);
            }
	}
    }

#if 0	/* Refactored */
    while (status >= 0) {
	sqlResult_t *subColl, *collType, *collInfo1, *collInfo2;

        if ((subColl = getSqlResultByInx (genQueryOut, COL_COLL_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "getCollUtil: getSqlResultByInx for COL_COLL_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        if (dataObjOprInp->specColl == NULL) {
	    /* some collections may be special */
            if ((collType = getSqlResultByInx (genQueryOut,
              COL_COLL_TYPE)) == NULL) {
                rodsLog (LOG_ERROR,
                 "getCollUtil:getSqlResultByInx for COL_COLL_TYPE failed");
                return (UNMATCHED_KEY_OR_INDEX);
            } else if ((collInfo1 = getSqlResultByInx (genQueryOut,
              COL_COLL_INFO1)) == NULL) {
                rodsLog (LOG_ERROR,
                 "getCollUtil:getSqlResultByInx for COL_COLL_INFO1 failed");
                return (UNMATCHED_KEY_OR_INDEX);
            } else if ((collInfo2 = getSqlResultByInx (genQueryOut,
              COL_COLL_INFO2)) == NULL) {
                rodsLog (LOG_ERROR,
                 "getCollUtil:getSqlResultByInx for COL_COLL_INFO2 failed");
                return (UNMATCHED_KEY_OR_INDEX);
	    }
	}

	for (i = 0; i < genQueryOut->rowCnt; i++) {
	    char *tmpSubColl, *tmpCollType, *tmpCollInfo1, *tmpCollInfo2;

	    tmpSubColl = &subColl->value[subColl->len * i];
            snprintf (targChildPath, MAX_NAME_LEN, "%s/%s",
              targDir, tmpSubColl + collLen);

	    if (strlen (tmpSubColl) > collLen)
#ifdef _WIN32
		mkdirR (targDir, targChildPath);
#else
	        mkdirR (targDir, targChildPath, 0750);
#endif
            if (dataObjOprInp->specColl == NULL) {
                tmpCollType = &collType->value[collType->len * i];
                tmpCollInfo1 = &collInfo1->value[collInfo1->len * i];
                tmpCollInfo2 = &collInfo2->value[collInfo2->len * i];
                if (tmpCollType[0] != '\0') {
                    /* spec Coll */
                    dataObjInp_t childDataObjInp;
                    specColl_t specColl;

                    childDataObjInp = *dataObjOprInp;
                    childDataObjInp.specColl = &specColl;
		    status = resolveSpecCollType (tmpCollType, tmpSubColl,
		      tmpCollInfo1, tmpCollInfo2, &specColl);
		    if (status < 0) return status;
                    status = getCollUtil (conn, tmpSubColl, targChildPath,
                      myRodsEnv, rodsArgs, &childDataObjInp, rodsRestart);
                    if (status < 0 && status != CAT_NO_ROWS_FOUND) {
                        return (status);
                    }
                }
	    }
	}

	continueInx = genQueryOut->continueInx;

	freeGenQueryOut (&genQueryOut);

	if (continueInx > 0) {
	    /* More to come */

            if (dataObjOprInp->specColl != NULL) {
		dataObjOprInp->openFlags = continueInx;
	        status = rcQuerySpecColl (conn, dataObjOprInp, &genQueryOut);
	    } else {
	        genQueryInp.continueInx = continueInx;
                status =  rcGenQuery (conn, &genQueryInp, &genQueryOut);
	    }
	} else {
	    break;
	}
    }
#endif

    if (dataObjOprInp->specColl == NULL) {
        clearGenQueryInp (&genQueryInp);
    }

    if (status < 0 && status != CAT_NO_ROWS_FOUND && 
      status != SYS_SPEC_COLL_OBJ_NOT_EXIST) {
        rodsLogError (LOG_ERROR, status,
          "getCollUtil: queryCollInCollReCur error for %s", srcColl);
	return (status);
    }
    
    /* Now get all the files */

    if (dataObjOprInp->specColl != NULL) {
        /* do the collection */
        addKeyVal (&dataObjOprInp->condInput, SEL_OBJ_TYPE_KW, "dataObj");
        status = rcQuerySpecColl (conn, dataObjOprInp, &genQueryOut);
    } else {
        memset (&genQueryInp, 0, sizeof (genQueryInp));
        status = queryDataObjInCollReCur (conn, srcColl, rodsArgs, &genQueryInp,
          &genQueryOut);
    }

    if (status >= 0) {
        status = genQueryOutToDataObjRes (&genQueryOut, &dataObjSqlResult);
    }

    rowInx = 0;

    while (status >= 0 &&
      (status = getNextDataObjMetaInfo (conn, dataObjOprInp, &genQueryInp,
      &dataObjSqlResult, &rowInx, &dataObjMetaInfo)) >= 0) {
	rodsLong_t mySize;

	mySize = strtoll (dataObjMetaInfo.dataSize, 0, 0);

        snprintf (targChildPath, MAX_NAME_LEN, "%s%s/%s",
          targDir, dataObjMetaInfo.collName + collLen, 
	  dataObjMetaInfo.dataName);
        snprintf (srcChildPath, MAX_NAME_LEN, "%s/%s",
          dataObjMetaInfo.collName, dataObjMetaInfo.dataName);

        status = chkStateForResume (conn, rodsRestart, targChildPath,
          rodsArgs, LOCAL_FILE_T, &dataObjOprInp->condInput, 1);

        if (status < 0) {
            /* restart failed */
	    break;
        } else if (status == 0) {
            continue;
        }

        status = getDataObjUtil (conn, srcChildPath,
         targChildPath, mySize, myRodsEnv, rodsArgs, dataObjOprInp);
	if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "getCollUtil: getDataObjUtil failed for %s. status = %d",
              srcChildPath, status);
	    if (rodsRestart->fd > 0) {
		break;
	    } else {
		savedStatus = status;
	    }
	} else {
	    status = procAndWrriteRestartFile (rodsRestart, targChildPath);
	} 
    }
#if 0
        if (rodsRestart->fd > 0) {
            if (status >= 0) {
                /* write the restart file */
                rodsRestart->curCnt ++;
                status = writeRestartFile (rodsRestart, targChildPath);
            } else {
                /* don't continue with restart */
                rodsLogError (LOG_ERROR, status,
                  "getCollUtil: getDataObjUtil failed for %s. status = %d",
                  srcChildPath, status);
		break;
            }
        } else if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "getCollUtil: getDataObjUtil failed for %s. status = %d",
              srcChildPath, status);
            /* need to set global error here */
            savedStatus = status;
        }
    }
#endif

#if 0	/* refactored */
    while (status >= 0) {
	sqlResult_t *subColl, *dataObj, *dataSize;;

        if ((subColl = getSqlResultByInx (genQueryOut, COL_COLL_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "getCollUtil: getSqlResultByInx for COL_COLL_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        if ((dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "getCollUtil: getSqlResultByInx for COL_DATA_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        if (dataObjOprInp->specColl != NULL) {
            if ((dataSize = getSqlResultByInx (genQueryOut, COL_DATA_SIZE))
              == NULL) {
                rodsLog (LOG_ERROR,
                  "getCollUtil: getSqlResultByInx for COL_DATA_SIZE failed");
                return (UNMATCHED_KEY_OR_INDEX);
	    }
        }


	for (i = 0; i < genQueryOut->rowCnt; i++) {
	    char *tmpSubColl, *tmpDataName, *tmpDataSize;
	    rodsLong_t mySize;

	    tmpSubColl = &subColl->value[subColl->len * i];
	    tmpDataName = &dataObj->value[dataObj->len * i];

            if (dataObjOprInp->specColl != NULL) {
		tmpDataSize = &dataSize->value[dataSize->len * i];
		mySize = strtoll (tmpDataSize, 0, 0);
	    } else {
		mySize = -1;
	    } 

	    snprintf (targChildPath, MAX_NAME_LEN, "%s%s/%s", 
	      targDir, tmpSubColl + collLen, tmpDataName);
	    snprintf (srcChildPath, MAX_NAME_LEN, "%s/%s",
	      tmpSubColl, tmpDataName);

            status = chkStateForResume (conn, rodsRestart, targChildPath,
              rodsArgs, LOCAL_FILE_T, &dataObjOprInp->condInput, 1);

            if (status < 0) {
                /* restart failed */
		freeGenQueryOut (&genQueryOut);
    		clearGenQueryInp (&genQueryInp);
                return (status);
            } else if (status == 0) {
                continue;
            }

	    status = getDataObjUtil (conn, srcChildPath,
             targChildPath, mySize, myRodsEnv, rodsArgs, dataObjOprInp);
            if (rodsRestart->fd > 0) {
                if (status >= 0) {
                    /* write the restart file */
                    rodsRestart->curCnt ++;
                    status = writeRestartFile (rodsRestart, targChildPath);
                } else {
                    /* don't continue with restart */
                    freeGenQueryOut (&genQueryOut);
                    clearGenQueryInp (&genQueryInp);
                    rodsLogError (LOG_ERROR, status,
                      "getCollUtil: getDataObjUtil failed for %s. status = %d",
                      srcChildPath, status);

                    return (status);
                }
            } else if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                  "getCollUtil: getDataObjUtil failed for %s. status = %d",
	          srcChildPath, status);
		/* need to set global error here */
		savedStatus = status;
            }
	}

	continueInx = genQueryOut->continueInx;

	freeGenQueryOut (&genQueryOut);

	if (continueInx > 0) {
	    /* More to come */
            if (dataObjOprInp->specColl != NULL) {
                dataObjOprInp->openFlags = continueInx;
                status = rcQuerySpecColl (conn, dataObjOprInp, &genQueryOut);
            } else {
	        genQueryInp.continueInx = continueInx;
                status =  rcGenQuery (conn, &genQueryInp, &genQueryOut);
	    }
	} else {
	    break;
	}
    }
#endif

    if (dataObjOprInp->specColl == NULL) {
        clearGenQueryInp (&genQueryInp);
    }

    if (savedStatus < 0) {
	return (savedStatus);
    } else if (status == CAT_NO_ROWS_FOUND || 
      status == SYS_SPEC_COLL_OBJ_NOT_EXIST) {
	return (0);
    } else {
        return (status);
    }
}

