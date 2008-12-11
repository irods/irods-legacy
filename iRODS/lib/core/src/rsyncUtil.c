/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#ifndef windows_platform
#include <sys/time.h>
#endif
#include "rodsPath.h"
#include "rodsErrorTable.h"
#include "rodsLog.h"
#include "rsyncUtil.h"
#include "miscUtil.h"

int
rsyncUtil (rcComm_t *conn, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs,
rodsPathInp_t *rodsPathInp)
{
    int i;
    int status; 
    int savedStatus = 0;
    rodsPath_t *srcPath, *targPath;
    dataObjInp_t dataObjOprInp;
    dataObjCopyInp_t dataObjCopyInp;


    if (rodsPathInp == NULL) {
	return (USER__NULL_INPUT_ERR);
    }


    status = resolveRodsTarget (conn, myRodsEnv, rodsPathInp, RSYNC_OPR);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rsyncUtil: resolveRodsTarget");
        return (status);
    }

    if (rodsPathInp->srcPath[0].objType <= COLL_OBJ_T &&
      rodsPathInp->targPath[0].objType <= COLL_OBJ_T) {
        initCondForIrodsToIrodsRsync (myRodsEnv, myRodsArgs, &dataObjCopyInp);
    } else {
        initCondForRsync (myRodsEnv, myRodsArgs, &dataObjOprInp);
    }

    for (i = 0; i < rodsPathInp->numSrc; i++) {
	int srcType, targType;

        targPath = &rodsPathInp->targPath[i];
        srcPath = &rodsPathInp->srcPath[i];
	srcType = srcPath->objType;
	targType = targPath->objType;

	if (srcPath->objState != EXIST_ST) {
            rodsLog (LOG_ERROR,
              "rsyncUtil: Source path %s does not exist");
	    return USER_INPUT_PATH_ERR;
	}
	    
        if (srcPath->objType <= COLL_OBJ_T && srcPath->rodsObjStat != NULL &&
          rodsPathInp->srcPath[i].rodsObjStat->specColl != NULL) {
            dataObjOprInp.specColl = dataObjCopyInp.srcDataObjInp.specColl =
              srcPath->rodsObjStat->specColl;
        } else {
            dataObjOprInp.specColl = 
	      dataObjCopyInp.srcDataObjInp.specColl = NULL;
        }

	if (srcType == DATA_OBJ_T && targType == LOCAL_FILE_T) { 
	    status = rsyncDataToFileUtil (conn, srcPath, targPath,
	     myRodsEnv, myRodsArgs, &dataObjOprInp);
	} else if (srcType == LOCAL_FILE_T && targType == DATA_OBJ_T) {
	    dataObjOprInp.createMode = rodsPathInp->srcPath[i].objMode;
            status = rsyncFileToDataUtil (conn, srcPath, targPath,
             myRodsEnv, myRodsArgs, &dataObjOprInp);
        } else if (srcType == DATA_OBJ_T && targType == DATA_OBJ_T) {
            status = rsyncDataToDataUtil (conn, srcPath, targPath,
             myRodsEnv, myRodsArgs, &dataObjCopyInp);
        } else if (srcType == COLL_OBJ_T && targType == LOCAL_DIR_T) {
            status = rsyncCollToDirUtil (conn, srcPath, targPath,
             myRodsEnv, myRodsArgs, &dataObjOprInp);
            if (status >= 0 && dataObjOprInp.specColl != NULL &&
              dataObjOprInp.specColl->collClass == STRUCT_FILE_COLL) {
                dataObjOprInp.specColl = NULL;
		status = rsyncCollToDirUtil (conn, srcPath, targPath,
                  myRodsEnv, myRodsArgs, &dataObjOprInp);
	    }
        } else if (srcType == LOCAL_DIR_T && targType == COLL_OBJ_T) {
            status = rsyncDirToCollUtil (conn, srcPath, targPath,
             myRodsEnv, myRodsArgs, &dataObjOprInp);
        } else if (srcType == COLL_OBJ_T && targType == COLL_OBJ_T) {
            status = rsyncCollToCollUtil (conn, srcPath, targPath,
             myRodsEnv, myRodsArgs, &dataObjCopyInp);
            if (status >= 0 && dataObjOprInp.specColl != NULL &&
              dataObjCopyInp.srcDataObjInp.specColl->collClass == STRUCT_FILE_COLL) {
		dataObjCopyInp.srcDataObjInp.specColl = NULL;
                status = rsyncCollToCollUtil (conn, srcPath, targPath,
                 myRodsEnv, myRodsArgs, &dataObjCopyInp);
	    }
	} else {
	    /* should not be here */
	    rodsLog (LOG_ERROR,
	     "rsyncUtil: invalid srcType %d and targType %d combination for %s",
	      srcType, targType, targPath->outPath);
	    return (USER_INPUT_PATH_ERR);
	}
	/* XXXX may need to return a global status */
	if (status < 0) {
            rodsLogError (LOG_ERROR, status,
             "rsyncUtil: rsync error for %s", 
	      targPath->outPath);
	    savedStatus = status;
	} 
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

int
rsyncDataToFileUtil (rcComm_t *conn, rodsPath_t *srcPath, 
rodsPath_t *targPath, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs, 
dataObjInp_t *dataObjOprInp)
{
    int status;
    struct timeval startTime, endTime;
    int getFlag = 0;
    int syncFlag = 0;
    char *chksum;
 
    if (srcPath == NULL || targPath == NULL) {
       rodsLog (LOG_ERROR,
          "rsyncDataToFileUtil: NULL srcPath or targPath input");
        return (USER__NULL_INPUT_ERR);
    }

    if (myRodsArgs->verbose == True) {
        (void) gettimeofday(&startTime, (struct timezone *)0);
	bzero (&conn->transStat, sizeof (transStat_t));
    }

    if (targPath->objState == NOT_EXIST_ST) {
	getFlag = 1;
    } else if (myRodsArgs->sizeFlag == True) {
	/* sync by size */
	if (targPath->size != srcPath->size) {
	    getFlag = 1;
	}
    } else if (strlen (srcPath->chksum) > 0) {
	/* src has a checksum value */
        status = rcChksumLocFile (targPath->outPath, RSYNC_CHKSUM_KW,
          &dataObjOprInp->condInput);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "rsyncDataToFileUtil: rcChksumLocFile error for %s, status = %d",
              targPath->outPath, status);
            return (status);
        } else {
	    chksum = getValByKey (&dataObjOprInp->condInput, RSYNC_CHKSUM_KW);
	    if (strcmp (chksum, srcPath->chksum) != 0) {
		getFlag = 1;
	    }
	}
    } else { 
	/* exist but no chksum */
        status = rcChksumLocFile (targPath->outPath, RSYNC_CHKSUM_KW,
          &dataObjOprInp->condInput);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "rsyncDataToFileUtil: rcChksumLocFile error for %s, status = %d",
              targPath->outPath, status);
            return (status);
        } else {
	    syncFlag = 1;
	}
    }

    if (getFlag + syncFlag > 0) {
        rstrcpy (dataObjOprInp->objPath, srcPath->outPath, MAX_NAME_LEN);
        dataObjOprInp->dataSize = srcPath->size;
        dataObjOprInp->openFlags = O_RDONLY;
    }

    if (getFlag == 1) {
        status = rcDataObjGet (conn, dataObjOprInp, targPath->outPath);
        rmKeyVal (&dataObjOprInp->condInput, RSYNC_CHKSUM_KW);
	if (status >= 0) myChmod (targPath->outPath, srcPath->objMode);
    } else if (syncFlag == 1) {
	addKeyVal (&dataObjOprInp->condInput, RSYNC_DEST_PATH_KW, 
	  targPath->outPath);
	addKeyVal (&dataObjOprInp->condInput, RSYNC_MODE_KW, LOCAL_TO_IRODS);
        status = rcDataObjRsync (conn, dataObjOprInp);
        rmKeyVal (&dataObjOprInp->condInput, RSYNC_DEST_PATH_KW);
        rmKeyVal (&dataObjOprInp->condInput, RSYNC_MODE_KW);
    } else {
	status = 0;
    }

    if (status >= 0 && myRodsArgs->verbose == True) {
	if (getFlag + syncFlag > 0) {
            (void) gettimeofday(&endTime, (struct timezone *)0);
            printTiming (conn, srcPath->outPath, srcPath->size, 
	      targPath->outPath, &startTime, &endTime);
	} else {
	    printNoSync (srcPath->outPath, srcPath->size);
	}
    }

    return (status);
}

int
rsyncFileToDataUtil (rcComm_t *conn, rodsPath_t *srcPath, 
rodsPath_t *targPath, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs, 
dataObjInp_t *dataObjOprInp)
{
    int status;
    struct timeval startTime, endTime;
    int putFlag = 0;
    int syncFlag = 0;
    char *chksum;
 
    if (srcPath == NULL || targPath == NULL) {
       rodsLog (LOG_ERROR,
          "rsyncFileToDataUtil: NULL srcPath or targPath input");
        return (USER__NULL_INPUT_ERR);
    }

    if (myRodsArgs->verbose == True) {
        (void) gettimeofday(&startTime, (struct timezone *)0);
        bzero (&conn->transStat, sizeof (transStat_t));
    }

    if (targPath->objState == NOT_EXIST_ST) {
	putFlag = 1;
    } else if (myRodsArgs->sizeFlag == True) {
	/* sync by size */
	if (targPath->size != srcPath->size) {
	    putFlag = 1;
	}
    } else if (strlen (targPath->chksum) > 0) {
	/* src has a checksum value */
        status = rcChksumLocFile (srcPath->outPath, RSYNC_CHKSUM_KW,
          &dataObjOprInp->condInput);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "rsyncFileToDataUtil: rcChksumLocFile error for %s, status = %d",
              srcPath->outPath, status);
            return (status);
        } else {
	    chksum = getValByKey (&dataObjOprInp->condInput, RSYNC_CHKSUM_KW);
	    if (strcmp (chksum, targPath->chksum) != 0) {
		putFlag = 1;
	    }
	}
    } else { 
	/* exist but no chksum */
        status = rcChksumLocFile (srcPath->outPath, RSYNC_CHKSUM_KW,
          &dataObjOprInp->condInput);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "rsyncFileToDataUtil: rcChksumLocFile error for %s, status = %d",
              srcPath->outPath, status);
            return (status);
        } else {
	    syncFlag = 1;
	}
    }

    if (putFlag + syncFlag > 0) {
        rstrcpy (dataObjOprInp->objPath, targPath->outPath, MAX_NAME_LEN);
        dataObjOprInp->dataSize = srcPath->size;
        dataObjOprInp->openFlags = O_WRONLY;
    }

    if (putFlag == 1) {
        status = rcDataObjPut (conn, dataObjOprInp, srcPath->outPath);
        rmKeyVal (&dataObjOprInp->condInput, RSYNC_CHKSUM_KW);
    } else if (syncFlag == 1) {
	addKeyVal (&dataObjOprInp->condInput, RSYNC_DEST_PATH_KW, 
	  srcPath->outPath);
	addKeyVal (&dataObjOprInp->condInput, RSYNC_MODE_KW, IRODS_TO_LOCAL);
        status = rcDataObjRsync (conn, dataObjOprInp);
        rmKeyVal (&dataObjOprInp->condInput, RSYNC_DEST_PATH_KW);
        rmKeyVal (&dataObjOprInp->condInput, RSYNC_MODE_KW);
    } else {
        status = 0;
    }

    if (status >= 0 && myRodsArgs->verbose == True) {
	if (putFlag + syncFlag > 0) {
            (void) gettimeofday(&endTime, (struct timezone *)0);
            printTiming (conn, srcPath->outPath, srcPath->size,
              targPath->outPath, &startTime, &endTime);
        } else {
            printNoSync (srcPath->outPath, srcPath->size);
        }
    }

    return (status);
}

int
rsyncDataToDataUtil (rcComm_t *conn, rodsPath_t *srcPath, 
rodsPath_t *targPath, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs, 
dataObjCopyInp_t *dataObjCopyInp)
{
    int status;
    struct timeval startTime, endTime;
    int syncFlag = 0;
    int cpFlag = 0;
 
    if (srcPath == NULL || targPath == NULL) {
       rodsLog (LOG_ERROR,
          "rsyncDataToDataUtil: NULL srcPath or targPath input");
        return (USER__NULL_INPUT_ERR);
    }

    if (myRodsArgs->verbose == True) {
        (void) gettimeofday(&startTime, (struct timezone *)0);
        bzero (&conn->transStat, sizeof (transStat_t));
    }

    if (targPath->objState == NOT_EXIST_ST) {
	cpFlag = 1;
    } else if (myRodsArgs->sizeFlag == True) {
	/* sync by size */
	if (targPath->size != srcPath->size) {
	    cpFlag = 1;
	}
    } else if (strlen (srcPath->chksum) > 0 && strlen (targPath->chksum) > 0) {
	/* src and trg has a checksum value */
	if (strcmp (targPath->chksum, srcPath->chksum) != 0) {
	    cpFlag = 1;
	}
    } else { 
	syncFlag = 1;
    }

    if (cpFlag == 1) {
        rstrcpy (dataObjCopyInp->srcDataObjInp.objPath, srcPath->outPath, 
          MAX_NAME_LEN);
        dataObjCopyInp->srcDataObjInp.dataSize = srcPath->size;
        rstrcpy (dataObjCopyInp->destDataObjInp.objPath, targPath->outPath, 
          MAX_NAME_LEN);
	status = rcDataObjCopy (conn, dataObjCopyInp);
    } else if (syncFlag == 1) {
	dataObjInp_t *dataObjOprInp = &dataObjCopyInp->destDataObjInp;
        rstrcpy (dataObjOprInp->objPath, srcPath->outPath, 
          MAX_NAME_LEN);
        dataObjOprInp->dataSize = srcPath->size;
        addKeyVal (&dataObjOprInp->condInput, RSYNC_DEST_PATH_KW,
          targPath->outPath);
	addKeyVal (&dataObjOprInp->condInput, RSYNC_MODE_KW, IRODS_TO_IRODS);
	status = rcDataObjRsync (conn, dataObjOprInp);
        rmKeyVal (&dataObjOprInp->condInput, RSYNC_MODE_KW);
        rmKeyVal (&dataObjOprInp->condInput, RSYNC_DEST_PATH_KW);
    } else {
	status = 0;
    }

    if (status >= 0 && myRodsArgs->verbose == True) {
	if (cpFlag + syncFlag > 0) {
            (void) gettimeofday(&endTime, (struct timezone *)0);
            printTiming (conn, srcPath->outPath, srcPath->size, 
	      targPath->outPath, &startTime, &endTime);
	} else {
            printNoSync (srcPath->outPath, srcPath->size);
        }
    }

    return (status);
}

int
rsyncCollToDirUtil (rcComm_t *conn, rodsPath_t *srcPath, 
rodsPath_t *targPath, rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjInp_t *dataObjOprInp)
{
    int status = 0;
    int savedStatus = 0;
    char *srcColl, *targDir;
    char srcChildPath[MAX_NAME_LEN], targChildPath[MAX_NAME_LEN];
    int collLen;
    rodsPath_t mySrcPath, myTargPath;
    collHandle_t collHandle;
    collEnt_t collEnt;

    if (srcPath == NULL || targPath == NULL) {
       rodsLog (LOG_ERROR,
          "rsyncCollToDirUtil: NULL srcPath or targPath input");
        return (USER__NULL_INPUT_ERR);
    }

    srcColl = srcPath->outPath;
    targDir = targPath->outPath;

    if (rodsArgs->recursive != True) {
        rodsLog (LOG_ERROR,
        "rsyncCollToDirUtil: -r option must be used for collection %s",
         targDir);
        return (USER_INPUT_OPTION_ERR);
    }

    collLen = strlen (srcColl);

    status = rclOpenCollection (conn, srcColl, 
      RECUR_QUERY_FG | VERY_LONG_METADATA_FG, &collHandle);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "getCollUtil: rclOpenCollection of %s error. status = %d",
          srcColl, status);
        return status;
    }

    memset (&mySrcPath, 0, sizeof (mySrcPath));
    memset (&myTargPath, 0, sizeof (myTargPath));
    myTargPath.objType = LOCAL_FILE_T;
    mySrcPath.objType = DATA_OBJ_T;

    while ((status = rclReadCollection (conn, &collHandle, &collEnt)) >= 0) {
        if (collEnt.objType == DATA_OBJ_T) {
            snprintf (myTargPath.outPath, MAX_NAME_LEN, "%s%s/%s",
              targDir, collEnt.collName + collLen,
              collEnt.dataName);
            snprintf (mySrcPath.outPath, MAX_NAME_LEN, "%s/%s",
              collEnt.collName, collEnt.dataName);
            /* fill in some info for mySrcPath */
            if (strlen (mySrcPath.dataId) == 0)
                rstrcpy (mySrcPath.dataId, collEnt.dataId, NAME_LEN);
            mySrcPath.size = collEnt.dataSize;
            mySrcPath.objMode = collEnt.dataMode;
            rstrcpy (mySrcPath.chksum, collEnt.chksum, NAME_LEN);
            mySrcPath.objState = EXIST_ST;

            getFileType (&myTargPath);

            status = rsyncDataToFileUtil (conn, &mySrcPath, &myTargPath,
              myRodsEnv, rodsArgs, dataObjOprInp);
            if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                  "rsyncCollUtil: rsyncDataObjUtil failed for %s. status = %d",
                  srcChildPath, status);
                /* need to set global error here */
                savedStatus = status;
                status = 0;
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
                rstrcpy (myTargPath.outPath, targChildPath, MAX_NAME_LEN);
                rstrcpy (mySrcPath.outPath, collEnt.collName, MAX_NAME_LEN);

                status = rsyncCollToDirUtil (conn, &mySrcPath,
                  &myTargPath, myRodsEnv, rodsArgs, &childDataObjInp);

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

int
rsyncDirToCollUtil (rcComm_t *conn, rodsPath_t *srcPath, 
rodsPath_t *targPath, rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjInp_t *dataObjOprInp)
{
    int status = 0;
    int savedStatus = 0;
    DIR *dirPtr;
    struct dirent *myDirent;
    struct stat statbuf;
    char *srcDir, *targColl;
    rodsPath_t mySrcPath, myTargPath;

    if (srcPath == NULL || targPath == NULL) {
       rodsLog (LOG_ERROR,
          "rsyncDirToCollUtil: NULL srcPath or targPath input");
        return (USER__NULL_INPUT_ERR);
    }

    srcDir = srcPath->outPath;
    targColl = targPath->outPath;

    if (rodsArgs->recursive != True) {
        rodsLog (LOG_ERROR,
        "rsyncDirToCollUtil: -r option must be used for putting %s directory",
         srcDir);
        return (USER_INPUT_OPTION_ERR);
    }

    dirPtr = opendir (srcDir);
    if (dirPtr == NULL) {
        rodsLog (LOG_ERROR,
        "rsyncDirToCollUtil: opendir local dir error for %s, errno = %d\n",
         srcDir, errno);
        return (USER_INPUT_PATH_ERR);
    }

    if (rodsArgs->verbose == True) {
        fprintf (stdout, "C- %s:\n", targColl);
    }

    memset (&mySrcPath, 0, sizeof (mySrcPath));
    memset (&myTargPath, 0, sizeof (myTargPath));
    myTargPath.objType = DATA_OBJ_T;
    mySrcPath.objType = LOCAL_FILE_T;

    while ((myDirent = readdir (dirPtr)) != NULL) {
        if (strcmp (myDirent->d_name, ".") == 0 ||
          strcmp (myDirent->d_name, "..") == 0) {
            continue;
        }
        snprintf (mySrcPath.outPath, MAX_NAME_LEN, "%s/%s",
          srcDir, myDirent->d_name);

        status = stat (mySrcPath.outPath, &statbuf);

        if (status != 0) {
            rodsLog (LOG_ERROR,
              "rsyncDirToCollUtil: stat error for %s, errno = %d\n",
              mySrcPath.outPath, errno);
            closedir (dirPtr);
            return (USER_INPUT_PATH_ERR);
        }

	dataObjOprInp->createMode = statbuf.st_mode;
        snprintf (myTargPath.outPath, MAX_NAME_LEN, "%s/%s",
          targColl, myDirent->d_name);

        if ((statbuf.st_mode & S_IFREG) != 0) {     /* a file */
            myTargPath.objType = DATA_OBJ_T;
            mySrcPath.objType = LOCAL_FILE_T;
	    mySrcPath.objState = EXIST_ST;
	    mySrcPath.size = statbuf.st_size;
	    getRodsObjType (conn, &myTargPath);
            status = rsyncFileToDataUtil (conn, &mySrcPath, &myTargPath,
              myRodsEnv, rodsArgs, dataObjOprInp);
	    /* fix a big mem leak */
	    freeRodsObjStat (myTargPath.rodsObjStat);
        } else if ((statbuf.st_mode & S_IFDIR) != 0) {      /* a directory */
            status = mkCollR (conn, targColl, myTargPath.outPath);
            if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                  "rsyncDirToCollUtil: mkColl error for %s", 
		  myTargPath.outPath);
            } else {
                myTargPath.objType = COLL_OBJ_T;
                mySrcPath.objType = LOCAL_DIR_T;
                mySrcPath.objState = myTargPath.objState = EXIST_ST;
                getRodsObjType (conn, &myTargPath);
                status = rsyncDirToCollUtil (conn, &mySrcPath, &myTargPath,
                  myRodsEnv, rodsArgs, dataObjOprInp);
	        /* fix a big mem leak */
		freeRodsObjStat (myTargPath.rodsObjStat);
            }
        } else {
            rodsLog (LOG_ERROR,
              "rsyncDirToCollUtil: unknown local path type %d for %s",
              statbuf.st_mode, mySrcPath.outPath);
            status = USER_INPUT_PATH_ERR;
        }

        if (status < 0) {
            savedStatus = status;
            rodsLogError (LOG_ERROR, status,
             "rsyncDirToCollUtil: put %s failed. status = %d",
              mySrcPath.outPath, status);
        }
    }

    closedir (dirPtr);

    if (savedStatus < 0) {
        return (savedStatus);
    } else if (status == CAT_NO_ROWS_FOUND || 
      status == SYS_SPEC_COLL_OBJ_NOT_EXIST) {
        return (0);
    } else {
        return (status);
    }

}

int
rsyncCollToCollUtil (rcComm_t *conn, rodsPath_t *srcPath, 
rodsPath_t *targPath, rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjCopyInp_t *dataObjCopyInp)
{
    int status = 0;
    int savedStatus = 0;
    char *srcColl, *targColl;
    char targChildPath[MAX_NAME_LEN];
    int collLen;
    rodsPath_t mySrcPath, myTargPath;
    collHandle_t collHandle;
    collEnt_t collEnt;
    dataObjInp_t *dataObjOprInp = &collHandle.dataObjInp;

    if (srcPath == NULL || targPath == NULL) {
       rodsLog (LOG_ERROR,
          "rsyncCollToCollUtil: NULL srcPath or targPath input");
        return (USER__NULL_INPUT_ERR);
    }

    srcColl = srcPath->outPath;
    targColl = targPath->outPath;

    if (rodsArgs->recursive != True) {
        rodsLog (LOG_ERROR,
        "rsyncCollToCollUtil: -r option must be used for collection %s",
         targColl);
        return (USER_INPUT_OPTION_ERR);
    }

    collLen = strlen (srcColl);

    status = rclOpenCollection (conn, srcColl,
      RECUR_QUERY_FG | VERY_LONG_METADATA_FG, &collHandle);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "getCollUtil: rclOpenCollection of %s error. status = %d",
          srcColl, status);
        return status;
    }

    if (dataObjOprInp->specColl != NULL) {
        if (rodsArgs->verbose == True) {
            if (rodsArgs->verbose == True) {
                char objType[NAME_LEN];
                status = getSpecCollTypeStr (dataObjOprInp->specColl, objType);
                if (status < 0) return (status);
                fprintf (stdout, "C- %s    %-5.5s :\n", targColl, objType);
            }
        }
    }

    memset (&mySrcPath, 0, sizeof (mySrcPath));
    memset (&myTargPath, 0, sizeof (myTargPath));
    myTargPath.objType = DATA_OBJ_T;
    mySrcPath.objType = DATA_OBJ_T;

    while ((status = rclReadCollection (conn, &collHandle, &collEnt)) >= 0) {
        if (collEnt.objType == DATA_OBJ_T) {
            snprintf (myTargPath.outPath, MAX_NAME_LEN, "%s%s/%s",
              targColl, collEnt.collName + collLen,
              collEnt.dataName);
            snprintf (mySrcPath.outPath, MAX_NAME_LEN, "%s/%s",
              collEnt.collName, collEnt.dataName);
            /* fill in some info for mySrcPath */
            if (strlen (mySrcPath.dataId) == 0)
                rstrcpy (mySrcPath.dataId, collEnt.dataId, NAME_LEN);
            mySrcPath.size = collEnt.dataSize;
            rstrcpy (mySrcPath.chksum, collEnt.chksum, NAME_LEN);
            mySrcPath.objState = EXIST_ST;

            getFileType (&myTargPath);

            status = rsyncDataToDataUtil (conn, &mySrcPath,
             &myTargPath, myRodsEnv, rodsArgs, dataObjCopyInp);
            if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                  "rsyncCollToCollUtil: rsyncDataToDataUtil failed for %s.stat=%d",
                  myTargPath.outPath, status);
                /* need to set global error here */
                savedStatus = status;
                status = 0;
            }
        } else if (collEnt.objType == COLL_OBJ_T) {
            if (strlen (collEnt.collName) <= collLen)
                continue;

            snprintf (targChildPath, MAX_NAME_LEN, "%s%s",
              targColl, collEnt.collName + collLen);

            mkColl (conn, targChildPath);

            if (collEnt.specColl.collClass != NO_SPEC_COLL) {
                /* the child is a spec coll. need to drill down */
                dataObjCopyInp_t childDataObjCopyInp;
                childDataObjCopyInp = *dataObjCopyInp;
                childDataObjCopyInp.srcDataObjInp.specColl = 
		  &collEnt.specColl;
                rstrcpy (myTargPath.outPath, targChildPath, MAX_NAME_LEN);
                rstrcpy (mySrcPath.outPath, collEnt.collName, MAX_NAME_LEN);
                status = rsyncCollToCollUtil (conn, &mySrcPath,
                  &myTargPath, myRodsEnv, rodsArgs, &childDataObjCopyInp);


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

int
initCondForRsync (rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjInp_t *dataObjInp)
{
    char *myResc = NULL;

    if (dataObjInp == NULL) {
       rodsLog (LOG_ERROR,
          "initCondForRsync: NULL dataObjOprInp input");
        return (USER__NULL_INPUT_ERR);
    }

    memset (dataObjInp, 0, sizeof (dataObjInp_t));

    if (rodsArgs == NULL) {
	return (0);
    }

    /* always turn on the force flag */
    addKeyVal (&dataObjInp->condInput, FORCE_FLAG_KW, "");

    if (rodsArgs->sizeFlag == True) {
        addKeyVal (&dataObjInp->condInput, VERIFY_BY_SIZE_KW, "");
    }

    if (rodsArgs->all == True) {
        addKeyVal (&dataObjInp->condInput, ALL_KW, "");
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


    return (0);
}

int
initCondForIrodsToIrodsRsync (rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjCopyInp_t *dataObjCopyInp)
{
    char *myResc = NULL;

    if (dataObjCopyInp == NULL) {
       rodsLog (LOG_ERROR,
          "initCondForCp: NULL dataObjCopyInp incp");
        return (USER__NULL_INPUT_ERR);
    }

    memset (dataObjCopyInp, 0, sizeof (dataObjCopyInp_t));

    if (rodsArgs == NULL) {
	return (0);
    }

    /* always turn on the force flag */
    addKeyVal (&dataObjCopyInp->destDataObjInp.condInput, FORCE_FLAG_KW, "");

    if (rodsArgs->sizeFlag == True) {
        addKeyVal (&dataObjCopyInp->destDataObjInp.condInput, 
	  VERIFY_BY_SIZE_KW, "");
    }

    if (rodsArgs->all == True) {
        addKeyVal (&dataObjCopyInp->destDataObjInp.condInput, ALL_KW, "");
    }

    if (rodsArgs->resource == True) {
        if (rodsArgs->resourceString == NULL) {
            rodsLog (LOG_ERROR,
              "initCondForRepl: NULL resourceString error");
            return (USER__NULL_INPUT_ERR);
        } else {
            myResc = rodsArgs->resourceString;
            addKeyVal (&dataObjCopyInp->destDataObjInp.condInput, 
	      DEST_RESC_NAME_KW, rodsArgs->resourceString);
        }
    } else if (myRodsEnv != NULL && strlen (myRodsEnv->rodsDefResource) > 0) {
        myResc = myRodsEnv->rodsDefResource;
        addKeyVal (&dataObjCopyInp->destDataObjInp.condInput, 
	  DEST_RESC_NAME_KW, myRodsEnv->rodsDefResource);
    }

    return (0);
}

