/*** Copyright (c), The Regents of the University of California            ***
 *** For more inforeplation please refer to files in the COPYRIGHT directory ***/
#include "rodsPath.h"
#include "rodsErrorTable.h"
#include "rodsLog.h"
#include "miscUtil.h"
#include "replUtil.h"

int
replUtil (rcComm_t *conn, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs,
rodsPathInp_t *rodsPathInp)
{
    int i;
    int status; 
    int savedStatus = 0;
    dataObjInp_t dataObjInp;
    rodsRestart_t rodsRestart;


    if (rodsPathInp == NULL) {
	return (USER__NULL_INPUT_ERR);
    }

    initCondForRepl (myRodsEnv, myRodsArgs, &dataObjInp, &rodsRestart);

    for (i = 0; i < rodsPathInp->numSrc; i++) {
	if (rodsPathInp->srcPath[i].objType == UNKNOWN_OBJ_T) {
	    getRodsObjType (conn, &rodsPathInp->srcPath[i]);
	    if (rodsPathInp->srcPath[i].objState == NOT_EXIST_ST) {
                rodsLog (LOG_ERROR,
                  "replUtil: srcPath %s does not exist",
                  rodsPathInp->srcPath[i].outPath);
		savedStatus = USER_INPUT_PATH_ERR;
		continue;
	    }
	}

	if (rodsPathInp->srcPath[i].objType == DATA_OBJ_T) {
	    status = replDataObjUtil (conn, rodsPathInp->srcPath[i].outPath, 
	     myRodsEnv, myRodsArgs, &dataObjInp);
	} else if (rodsPathInp->srcPath[i].objType ==  COLL_OBJ_T) {
            setStateForRestart (conn, &rodsRestart, &rodsPathInp->srcPath[i], 
	      myRodsArgs);
	    status = replCollUtil (conn, rodsPathInp->srcPath[i].outPath,
              myRodsEnv, myRodsArgs, &dataObjInp, &rodsRestart);
            if (rodsRestart.fd > 0 && status < 0) {
                close (rodsRestart.fd);
                return (status);
            }
	} else {
	    /* should not be here */
	    rodsLog (LOG_ERROR,
	     "replUtil: invalid repl objType %d for %s", 
	     rodsPathInp->srcPath[i].objType, rodsPathInp->srcPath[i].outPath);
	    return (USER_INPUT_PATH_ERR);
	}
	/* XXXX may need to return a global status */
	if (status < 0) {
	    rodsLogError (LOG_ERROR, status,
             "replUtil: repl error for %s, status = %d", 
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
replDataObjUtil (rcComm_t *conn, char *srcPath, 
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjInp_t *dataObjInp)
{
    int status;
    struct timeval startTime, endTime;
 
    if (srcPath == NULL) {
       rodsLog (LOG_ERROR,
          "replDataObjUtil: NULL srcPath input");
        return (USER__NULL_INPUT_ERR);
    }

    if (rodsArgs->verbose == True) {
        (void) gettimeofday(&startTime, (struct timezone *)0);
    }

    rstrcpy (dataObjInp->objPath, srcPath, MAX_NAME_LEN);

    status = rcDataObjRepl (conn, dataObjInp);

    if (status >= 0 && rodsArgs->verbose == True) {
        (void) gettimeofday(&endTime, (struct timezone *)0);
        printTiming (conn, dataObjInp->objPath, conn->transStat.bytesWritten, 
	  NULL, &startTime, &endTime);
    }

    return (status);
}

int
initCondForRepl (rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjInp_t *dataObjInp, rodsRestart_t *rodsRestart)
{
    char *myResc = NULL;

    if (dataObjInp == NULL) {
       rodsLog (LOG_ERROR,
          "initCondForRepl: NULL dataObjInp input");
        return (USER__NULL_INPUT_ERR);
    }

    memset (dataObjInp, 0, sizeof (dataObjInp_t));

    if (rodsArgs == NULL) {
	return (0);
    }

    if (rodsArgs->replNum == True) {
        addKeyVal (&dataObjInp->condInput, REPL_NUM_KW, 
	  rodsArgs->replNumValue);
    }

    if (rodsArgs->all == True) {
        addKeyVal (&dataObjInp->condInput, ALL_KW, "");
    }

    if (rodsArgs->admin == True) {
        addKeyVal (&dataObjInp->condInput, IRODS_ADMIN_KW, "");
    }

    if (rodsArgs->srcResc == True) {
        addKeyVal (&dataObjInp->condInput, RESC_NAME_KW,
          rodsArgs->srcRescString);
    }

    if (rodsArgs->resource == True) {
        if (rodsArgs->resourceString == NULL) {
            rodsLog (LOG_ERROR,
              "initCondForRepl: NULL resourceString error");
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

    if (rodsArgs->backupMode == True) {
        addKeyVal (&dataObjInp->condInput, BACKUP_RESC_NAME_KW,
          myResc);
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

    return (0);
}

int
replCollUtil (rcComm_t *conn, char *srcColl, rodsEnv *myRodsEnv, 
rodsArguments_t *rodsArgs, dataObjInp_t *dataObjInp, 
rodsRestart_t *rodsRestart)
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
          "replCollUtil: NULL srcColl input");
        return (USER__NULL_INPUT_ERR);
    }

    if (rodsArgs->recursive != True) {
        rodsLog (LOG_ERROR,
        "replCollUtil: -r option must be used for getting %s collection",
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

        status = chkStateForResume (conn, rodsRestart, srcChildPath,
          rodsArgs, DATA_OBJ_T, &dataObjInp->condInput, 0);

        if (status < 0) {
            /* restart failed */
            break;
        } else if (status == 0) {
            continue;
        }

        status = replDataObjUtil (conn, srcChildPath,
         myRodsEnv, rodsArgs, dataObjInp);

	
	if (status == SYS_COPY_ALREADY_IN_RESC) {
           if (rodsArgs->verbose == True) {
              printf ("copy of %s already exists. Probably OK\n",
                srcChildPath);
            }
            status = 0;
	}

        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "replCollUtil: getDataObjUtil failed for %s. status = %d",
              srcChildPath, status);
            if (rodsRestart->fd > 0) {
                break;
            } else {
                savedStatus = status;
            }
        } else {
            status = procAndWrriteRestartFile (rodsRestart, srcChildPath);
        }
    }

#if 0
    while (status >= 0) {
	sqlResult_t *subColl, *dataObj;

        if ((subColl = getSqlResultByInx (genQueryOut, COL_COLL_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "replCollUtil: getSqlResultByInx for COL_COLL_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        if ((dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "replCollUtil: getSqlResultByInx for COL_DATA_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

	for (i = 0; i < genQueryOut->rowCnt; i++) {
	    char *tmpSubColl, *tmpDataName;

	    tmpSubColl = &subColl->value[subColl->len * i];
	    tmpDataName = &dataObj->value[dataObj->len * i];

	    snprintf (srcChildPath, MAX_NAME_LEN, "%s/%s",
	      tmpSubColl, tmpDataName);

	     
            status = chkStateForResume (conn, rodsRestart, srcChildPath,
              rodsArgs, DATA_OBJ_T, &dataObjInp->condInput, 0);

            if (status < 0) {
                /* restart failed */
                freeGenQueryOut (&genQueryOut);
                clearGenQueryInp (&genQueryInp);
                return (status);
            } else if (status == 0) {
                continue;
            }

	    status = replDataObjUtil (conn, srcChildPath,
             myRodsEnv, rodsArgs, dataObjInp);
            if (rodsRestart->fd > 0) {
                if (status >= 0) {
                    /* write the restart file */
                    rodsRestart->curCnt ++;
                    status = writeRestartFile (rodsRestart, srcChildPath);
                } else {
		    if (status == SYS_COPY_ALREADY_IN_RESC) {
			/* this could happen because the opr may not have been
			 * recorded */
			if (rodsArgs->verbose == True) {
			  printf ("copy of %s already exists. Probably OK\n",
			    srcChildPath);
			}
			status = 0;
		    } else {
                        /* don't continue with restart */
                        rodsLogError (LOG_ERROR, status,
                          "getCollUtil:replDataObjUtil failed for %s.stat=%d",
                          srcChildPath, status);
                        freeGenQueryOut (&genQueryOut);
                        clearGenQueryInp (&genQueryInp);
                        return (status);
		    }
                }
            } else if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                  "replCollUtil: replDataObjUtil failed for %s. status = %d",
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

    clearGenQueryInp (&genQueryInp);


    if (savedStatus < 0) {
	return (savedStatus);
    } else if (status == CAT_NO_ROWS_FOUND) {
	return (0);
    } else {
        return (status);
    }
}

