/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#ifndef windows_platform
#include <sys/time.h>
#endif
#include "rodsPath.h"
#include "rodsErrorTable.h"
#include "rodsLog.h"
#include "putUtil.h"
#include "miscUtil.h"

int
putUtil (rcComm_t *conn, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs,
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

    initCondForPut (myRodsEnv, myRodsArgs, &dataObjOprInp, &rodsRestart);

    status = resolveRodsTarget (conn, myRodsEnv, rodsPathInp, 1);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "putUtil: resolveRodsTarget error, status = %d", status);
        return (status);
    }

    for (i = 0; i < rodsPathInp->numSrc; i++) {
	targPath = &rodsPathInp->targPath[i];

	if (targPath->objType == DATA_OBJ_T) {
	    dataObjOprInp.createMode = rodsPathInp->srcPath[i].objMode;
	    status = putFileUtil (conn, rodsPathInp->srcPath[i].outPath, 
	      targPath->outPath, rodsPathInp->srcPath[i].size, myRodsEnv, 
	       myRodsArgs, &dataObjOprInp);
	} else if (targPath->objType == COLL_OBJ_T) {
	    setStateForRestart (conn, &rodsRestart, targPath, myRodsArgs);
	    status = putDirUtil (conn, rodsPathInp->srcPath[i].outPath,
              targPath->outPath, myRodsEnv, myRodsArgs, &dataObjOprInp,
	      &rodsRestart);
            if (rodsRestart.fd > 0 && status < 0) {
                close (rodsRestart.fd);
                return (status);
            }
	} else {
	    /* should not be here */
	    rodsLog (LOG_ERROR,
	     "putUtil: invalid put dest objType %d for %s", 
	      targPath->objType, targPath->outPath);
	    return (USER_INPUT_PATH_ERR);
	}
	/* XXXX may need to return a global status */
	if (status < 0) {
	    rodsLogError (LOG_ERROR, status,
             "putUtil: put error for %s, status = %d", 
	      targPath->outPath, status);
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
putFileUtil (rcComm_t *conn, char *srcPath, char *targPath, rodsLong_t srcSize,
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, dataObjInp_t *dataObjOprInp)
{
    int status;
    struct timeval startTime, endTime;
 
    if (srcPath == NULL || targPath == NULL) {
       rodsLog (LOG_ERROR,
          "putFileUtil: NULL srcPath or targPath input");
        return (USER__NULL_INPUT_ERR);
    }

    if (rodsArgs->verbose == True) {
	(void) gettimeofday(&startTime, (struct timezone *)0);
    }

    /* have to take care of checksum here since it needs to be recalcuated */ 
    if (rodsArgs->checksum == True) {
        status = rcChksumLocFile (srcPath, REG_CHKSUM_KW,
          &dataObjOprInp->condInput);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "putFileUtil: rcChksumLocFile error for %s, status = %d", 
              srcPath, status);
            return (status);
        }
    } else if (rodsArgs->verifyChecksum == True) {
        status = rcChksumLocFile (srcPath, VERIFY_CHKSUM_KW,
          &dataObjOprInp->condInput);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "putFileUtil: rcChksumLocFile error for %s, status = %d",
              srcPath, status);
            return (status);
        }
    }
    if (strlen(targPath)>=MAX_PATH_ALLOWED-1) return(USER_PATH_EXCEEDS_MAX);
    rstrcpy (dataObjOprInp->objPath, targPath, MAX_NAME_LEN);
    dataObjOprInp->dataSize = srcSize;

    status = rcDataObjPut (conn, dataObjOprInp, srcPath);

    if (status >= 0 && rodsArgs->verbose == True) {
        (void) gettimeofday(&endTime, (struct timezone *)0);
	printTiming (conn, dataObjOprInp->objPath, srcSize, srcPath,
	 &startTime, &endTime);
    }

    return (status);
}

int
initCondForPut (rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjInp_t *dataObjOprInp, rodsRestart_t *rodsRestart)
{
#ifdef RBUDP_TRANSFER
    char *tmpStr;
#endif  /* RBUDP_TRANSFER */

    if (dataObjOprInp == NULL) {
       rodsLog (LOG_ERROR,
          "initCondForPut: NULL dataObjOprInp input");
        return (USER__NULL_INPUT_ERR);
    }

    memset (dataObjOprInp, 0, sizeof (dataObjInp_t));

    dataObjOprInp->oprType = PUT_OPR;

    if (rodsArgs == NULL) {
	return (0);
    }

    if (rodsArgs->all == True) {
	addKeyVal (&dataObjOprInp->condInput, ALL_KW, "");
    }

    if (rodsArgs->dataType == True) {
        if (rodsArgs->dataTypeString == NULL) {
	    addKeyVal (&dataObjOprInp->condInput, DATA_TYPE_KW, "generic");
        } else {
            if (strcmp (rodsArgs->dataTypeString, "t") == 0 ||
              strcmp (rodsArgs->dataTypeString, "tar") == 0) {
                addKeyVal (&dataObjOprInp->condInput, DATA_TYPE_KW,
                  "tar file");
            } else {
                addKeyVal (&dataObjOprInp->condInput, DATA_TYPE_KW,
                  rodsArgs->dataTypeString);
            }
        }
    } else {
	addKeyVal (&dataObjOprInp->condInput, DATA_TYPE_KW, "generic");
    }

    if (rodsArgs->force == True) { 
        addKeyVal (&dataObjOprInp->condInput, FORCE_FLAG_KW, "");
    }

#ifdef windows_platform
    dataObjOprInp->numThreads = NO_THREADING;
#else
    if (rodsArgs->number == True) {
	if (rodsArgs->numberValue == 0) {
	    dataObjOprInp->numThreads = NO_THREADING;
	} else {
	    dataObjOprInp->numThreads = rodsArgs->numberValue;
	}
    }
#endif

    if (rodsArgs->physicalPath == True) {
	if (rodsArgs->physicalPathString == NULL) {
	    rodsLog (LOG_ERROR, 
	      "initCondForPut: NULL physicalPathString error");
	    return (USER__NULL_INPUT_ERR);
	} else {
            addKeyVal (&dataObjOprInp->condInput, FILE_PATH_KW, 
	      rodsArgs->physicalPathString);
	}
    }

    if (rodsArgs->resource == True) {
        if (rodsArgs->resourceString == NULL) {
            rodsLog (LOG_ERROR,
              "initCondForPut: NULL resourceString error");
            return (USER__NULL_INPUT_ERR);
        } else {
            addKeyVal (&dataObjOprInp->condInput, DEST_RESC_NAME_KW,
              rodsArgs->resourceString);
        }
    } else if (myRodsEnv != NULL && strlen (myRodsEnv->rodsDefResource) > 0) {
        addKeyVal (&dataObjOprInp->condInput, DEST_RESC_NAME_KW,
          myRodsEnv->rodsDefResource);
    } 

    if (rodsArgs->replNum == True) {
        addKeyVal (&dataObjOprInp->condInput, REPL_NUM_KW,
          rodsArgs->replNumValue);
    }

#ifdef RBUDP_TRANSFER
    if (rodsArgs->rbudp == True) {
	/* use -Q for rbudp transfer */
        addKeyVal (&dataObjOprInp->condInput, RBUDP_TRANSFER_KW, "");
    }

    if (rodsArgs->veryVerbose == True) {
        addKeyVal (&dataObjOprInp->condInput, VERY_VERBOSE_KW, "");
    }

    if ((tmpStr = getenv (RBUDP_SEND_RATE_KW)) != NULL) {
        addKeyVal (&dataObjOprInp->condInput, RBUDP_SEND_RATE_KW, tmpStr);
    }

    if ((tmpStr = getenv (RBUDP_PACK_SIZE_KW)) != NULL) {
        addKeyVal (&dataObjOprInp->condInput, RBUDP_PACK_SIZE_KW, tmpStr);
    }
#else	/* RBUDP_TRANSFER */
    if (rodsArgs->rbudp == True) {
        rodsLog (LOG_NOTICE,
          "initCondForPut: RBUDP_TRANSFER (-d) not supported");
    }
#endif  /* RBUDP_TRANSFER */

    memset (rodsRestart, 0, sizeof (rodsRestart_t));
    if (rodsArgs->restart == True) {
	int status;
	status = openRestartFile (rodsArgs->restartFileString, rodsRestart,
	  rodsArgs);
	if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "initCondForPut: openRestartFile of %s error",
	    rodsArgs->restartFileString);
	    return (status);
	}
    }

    /* Not needed - dataObjOprInp->createMode = 0700; */
    /* mmap in rbudp needs O_RDWR */
    dataObjOprInp->openFlags = O_RDWR;

    return (0);
}

int
putDirUtil (rcComm_t *conn, char *srcDir, char *targColl, 
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, dataObjInp_t *dataObjOprInp,
rodsRestart_t *rodsRestart)
{
    int status = 0;
    int savedStatus = 0;
    DIR *dirPtr;
    struct dirent *myDirent;
    struct stat statbuf;
    char srcChildPath[MAX_NAME_LEN], targChildPath[MAX_NAME_LEN];
    objType_t childObjType;

    if (srcDir == NULL || targColl == NULL) {
       rodsLog (LOG_ERROR,
          "putDirUtil: NULL srcDir or targColl input");
        return (USER__NULL_INPUT_ERR);
    }

    if (rodsArgs->recursive != True) {
        rodsLog (LOG_ERROR,
        "putDirUtil: -r option must be used for putting %s directory",
         srcDir);
        return (USER_INPUT_OPTION_ERR);
    }

    dirPtr = opendir (srcDir);
    if (dirPtr == NULL) {
	rodsLog (LOG_ERROR,
        "putDirUtil: opendir local dir error for %s, errno = %d\n",
         srcDir, errno);
        return (USER_INPUT_PATH_ERR);
    }

    if (rodsArgs->verbose == True) {
	fprintf (stdout, "C- %s:\n", targColl);
    }

    while ((myDirent = readdir (dirPtr)) != NULL) {
        if (strcmp (myDirent->d_name, ".") == 0 || 
	  strcmp (myDirent->d_name, "..") == 0) {
            continue;
	}
        snprintf (srcChildPath, MAX_NAME_LEN, "%s/%s", 
	  srcDir, myDirent->d_name);

        status = stat (srcChildPath, &statbuf);

        if (status != 0) {
            rodsLog (LOG_ERROR,
	      "putDirUtil: stat error for %s, errno = %d\n", 
	      srcChildPath, errno);
            closedir (dirPtr);
            return (USER_INPUT_PATH_ERR);
        }

	dataObjOprInp->createMode = statbuf.st_mode;
        snprintf (targChildPath, MAX_NAME_LEN, "%s/%s",
	  targColl, myDirent->d_name);

	if (statbuf.st_mode & S_IFREG) {
	    childObjType = DATA_OBJ_T;
	} else if (statbuf.st_mode & S_IFDIR) {
	    childObjType = COLL_OBJ_T;
        } else {
            rodsLog (LOG_ERROR,
              "putDirUtil: unknown local path type %d for %s",
              statbuf.st_mode, srcChildPath);
            savedStatus = USER_INPUT_PATH_ERR;
	    continue;
        }

	status = chkStateForResume (conn, rodsRestart, targChildPath,
	  rodsArgs, childObjType, &dataObjOprInp->condInput, 1);

	if (status < 0) {
	    /* restart failed */
	    closedir (dirPtr);
	    return (status);
	} else if (status == 0) {
	    continue;
	}

        if (childObjType == DATA_OBJ_T) {     /* a file */
	    status = putFileUtil (conn, srcChildPath, targChildPath, 
	      statbuf.st_size, myRodsEnv, rodsArgs, dataObjOprInp);
	    if (rodsRestart->fd > 0) {
		if (status >= 0) {
		    /* write the restart file */
		    rodsRestart->curCnt ++;
		    status = writeRestartFile (rodsRestart, targChildPath);
	        } else {
		    /* don't continue with restart */
		    closedir (dirPtr);
                    rodsLogError (LOG_ERROR, status,
                     "putDirUtil: put %s failed. status = %d",
                      srcChildPath, status);
		    return (status);
		}
	    }
        } else {      /* a directory */
	    status = mkColl (conn, targChildPath);
	    if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                  "putDirUtil: mkColl error for %s", targChildPath);
	    }

            status = putDirUtil (conn, srcChildPath, targChildPath, 
              myRodsEnv, rodsArgs, dataObjOprInp, rodsRestart);
	    if (rodsRestart->fd > 0 && status < 0) {
		closedir (dirPtr);
		return (status);
	    }
        }

        if (status < 0) {
            savedStatus = status;
	    rodsLogError (LOG_ERROR, status,
	     "putDirUtil: put %s failed. status = %d",
	      srcChildPath, status); 
        }
    }

    closedir (dirPtr);

    if (savedStatus < 0) {
        return (savedStatus);
    } else if (status == CAT_NO_ROWS_FOUND) {
        return (0);
    } else {
        return (status);
    }
}

