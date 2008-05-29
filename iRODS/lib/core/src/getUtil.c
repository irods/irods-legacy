/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#ifndef windows_platform
#include <sys/time.h>
#endif
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
    char srcChildPath[MAX_NAME_LEN], targChildPath[MAX_NAME_LEN];
    int collLen;
    collHandle_t collHandle;
    collEnt_t collEnt;

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
    status = rclOpenCollection (conn, srcColl, RECUR_QUERY_FG, 
      &collHandle);

    if (status < 0) {
	rodsLog (LOG_ERROR,
          "getCollUtil: rclOpenCollection of %s error. status = %d",
          srcColl, status);
        return status;
    }
    while ((status = rclReadCollection (conn, &collHandle, &collEnt)) >= 0) {
        if (collEnt.objType == DATA_OBJ_T) {
            rodsLong_t mySize;

            mySize = collEnt.dataSize;    /* have to save it. May be freed */  

            snprintf (targChildPath, MAX_NAME_LEN, "%s%s/%s",
              targDir, collEnt.collName + collLen,
              collEnt.dataName);
            snprintf (srcChildPath, MAX_NAME_LEN, "%s/%s",
              collEnt.collName, collEnt.dataName);

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
	} else if (collEnt.objType == COLL_OBJ_T) {
            snprintf (targChildPath, MAX_NAME_LEN, "%s/%s",
              targDir, collEnt.collName + collLen);

            mkdirR (targDir, targChildPath, 0750);

            if (collEnt.specColl.collClass != NO_SPEC_COLL) {
                /* the child is a spec coll. need to drill down */
                dataObjInp_t childDataObjInp;
                childDataObjInp = *dataObjOprInp;
                childDataObjInp.specColl = &collEnt.specColl;
                status = getCollUtil (conn, collEnt.collName, targChildPath,
                  myRodsEnv, rodsArgs, &childDataObjInp, rodsRestart);
                if (status < 0 && status != CAT_NO_ROWS_FOUND) {
                    return (status);
                }
            }
        }
    }
    rclCloseCollection (&collHandle);

    if (savedStatus < 0) {
	return (savedStatus);
    } else if (status == CAT_NO_ROWS_FOUND || 
      status == SYS_SPEC_COLL_OBJ_NOT_EXIST) {
	return (0);
    } else {
        return (status);
    }
}

