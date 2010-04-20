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
putUtil (rcComm_t **myConn, rodsEnv *myRodsEnv, 
rodsArguments_t *myRodsArgs, rodsPathInp_t *rodsPathInp)
{
    int i;
    int status; 
    int savedStatus = 0;
    rodsPath_t *targPath;
    dataObjInp_t dataObjOprInp;
    bulkOprInp_t bulkOprInp;
    rodsRestart_t rodsRestart;
    rcComm_t *conn = *myConn;

    if (rodsPathInp == NULL) {
	return (USER__NULL_INPUT_ERR);
    }

    initCondForPut (myRodsEnv, myRodsArgs, &dataObjOprInp, &bulkOprInp,
      &rodsRestart);

    status = resolveRodsTarget (conn, myRodsEnv, rodsPathInp, 1);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "putUtil: resolveRodsTarget error, status = %d", status);
        return (status);
    }

    /* initialize the progress struct */
    if (gGuiProgressCB != NULL) {
        bzero (&conn->operProgress, sizeof (conn->operProgress));
        for (i = 0; i < rodsPathInp->numSrc; i++) {
            targPath = &rodsPathInp->targPath[i];
            if (targPath->objType == DATA_OBJ_T) {
                conn->operProgress.totalNumFiles++;
                if (rodsPathInp->srcPath[i].size > 0) {
                    conn->operProgress.totalFileSize +=
                      rodsPathInp->srcPath[i].size;
                }
            } else {
                getDirSizeForProgStat (rodsPathInp->srcPath[i].outPath,
                  &conn->operProgress);
            }
        }
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
	    if (myRodsArgs->bulk == True) {
		status = bulkPutDirUtil (myConn, 
		  rodsPathInp->srcPath[i].outPath, targPath->outPath, 
		  myRodsEnv, myRodsArgs, &dataObjOprInp, &bulkOprInp,
		  &rodsRestart);
	    } else {
	        status = putDirUtil (myConn, rodsPathInp->srcPath[i].outPath,
                  targPath->outPath, myRodsEnv, myRodsArgs, &dataObjOprInp,
	          &bulkOprInp, &rodsRestart, NULL);
	    }
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

    if (gGuiProgressCB != NULL) {
        rstrcpy (conn->operProgress.curFileName, srcPath, MAX_NAME_LEN);
        conn->operProgress.curFileSize = srcSize;
        conn->operProgress.curFileSizeDone = 0;
        conn->operProgress.flag = 0;
        gGuiProgressCB (&conn->operProgress);
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

    if (status >= 0) {
        if (rodsArgs->verbose == True) {
            (void) gettimeofday(&endTime, (struct timezone *)0);
	    printTiming (conn, dataObjOprInp->objPath, srcSize, srcPath,
	     &startTime, &endTime);
	}
        if (gGuiProgressCB != NULL) {
            conn->operProgress.totalNumFilesDone++;
            conn->operProgress.totalFileSizeDone += srcSize;
        }
    }

    return (status);
}

int
initCondForPut (rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjInp_t *dataObjOprInp, bulkOprInp_t *bulkOprInp, 
rodsRestart_t *rodsRestart)
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

    if (rodsArgs->bulk == True) {
        if (bulkOprInp == NULL) {
           rodsLog (LOG_ERROR,
              "initCondForPut: NULL bulkOprInp input");
            return (USER__NULL_INPUT_ERR);
	}
	bzero (bulkOprInp, sizeof (bulkOprInp_t));
    }

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
	if (rodsArgs->bulk == True) {
	    addKeyVal (&bulkOprInp->condInput, FORCE_FLAG_KW, "");
	}
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
            if (rodsArgs->bulk == True) {
                addKeyVal (&bulkOprInp->condInput, DEST_RESC_NAME_KW,
                  rodsArgs->resourceString);
            }

        }
    } else if (myRodsEnv != NULL && strlen (myRodsEnv->rodsDefResource) > 0) {
	/* use rodsDefResource but set the DEF_RESC_NAME_KW instead. 
	 * Used by dataObjCreate. Will only make a new replica only if
	 * DEST_RESC_NAME_KW is set */ 
        addKeyVal (&dataObjOprInp->condInput, DEF_RESC_NAME_KW,
          myRodsEnv->rodsDefResource);
        if (rodsArgs->bulk == True) {
            addKeyVal (&bulkOprInp->condInput, DEF_RESC_NAME_KW,
              myRodsEnv->rodsDefResource);
        }
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
putDirUtil (rcComm_t **myConn, char *srcDir, char *targColl, 
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, dataObjInp_t *dataObjOprInp,
bulkOprInp_t *bulkOprInp, rodsRestart_t *rodsRestart, 
bulkOprInfo_t *bulkOprInfo)
{
    int status = 0;
    int savedStatus = 0;
    DIR *dirPtr;
    struct dirent *myDirent;
#ifndef windows_platform
    struct stat statbuf;
#else
	struct irodsntstat statbuf;
#endif
    char srcChildPath[MAX_NAME_LEN], targChildPath[MAX_NAME_LEN];
    objType_t childObjType;
    rcComm_t *conn;

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

    if (rodsArgs->redirectConn == True && rodsArgs->force != True) {
        int reconnFlag;
        if (rodsArgs->reconnect == True) {
            reconnFlag = RECONN_TIMEOUT;
        } else {
            reconnFlag = NO_RECONN;
        }
	/* reconnect to the resource server */
	redirectConnToRescSvr (myConn, dataObjOprInp, myRodsEnv, reconnFlag);
    }

    conn = *myConn;
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

#ifndef windows_platform
        status = stat (srcChildPath, &statbuf);
#else
		status = iRODSNt_stat(srcChildPath, &statbuf);
#endif

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
	    if (bulkOprInfo == NULL) {
		/* normal put */
                status = putFileUtil (conn, srcChildPath, targChildPath,
                  statbuf.st_size, myRodsEnv, rodsArgs, dataObjOprInp);
	    } else if (bulkOprInfo->flags == BULK_OPR_SMALL_FILES) {
		if (statbuf.st_size <= MAX_BULK_OPR_FILE_SIZE) {
                    status = bulkPutFileUtil (conn, srcChildPath, targChildPath,
                      statbuf.st_size, myRodsEnv, rodsArgs, bulkOprInp,
		      bulkOprInfo);
		} else {
		    continue;
		}
	    } else if (bulkOprInfo->flags == BULK_OPR_LARGE_FILES) {
		if (statbuf.st_size > MAX_BULK_OPR_FILE_SIZE) {
                    status = putFileUtil (conn, srcChildPath, targChildPath,
                      statbuf.st_size, myRodsEnv, rodsArgs, dataObjOprInp);
		} else {
		    continue;
		}
	    }

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

	    rodsArgs->redirectConn = 0;    /* only do it once */

            status = putDirUtil (myConn, srcChildPath, targChildPath, 
              myRodsEnv, rodsArgs, dataObjOprInp, bulkOprInp,
	      rodsRestart, bulkOprInfo);

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

int
bulkPutDirUtil (rcComm_t **myConn, char *srcDir, char *targColl,
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, dataObjInp_t *dataObjOprInp,
bulkOprInp_t *bulkOprInp, rodsRestart_t *rodsRestart)
{
    int status;
    bulkOprInfo_t bulkOprInfo;

    /* do large files first */
    bzero (&bulkOprInfo, sizeof (bulkOprInfo));
    bulkOprInfo.flags = BULK_OPR_LARGE_FILES;

    status = putDirUtil (myConn, srcDir, targColl, myRodsEnv, rodsArgs,
      dataObjOprInp, bulkOprInp, rodsRestart, &bulkOprInfo);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "bulkPutDirUtil: Large files bulkPut error for %s", srcDir);
        return status;
    }

    /* now bulk put the small files */
    bzero (&bulkOprInfo, sizeof (bulkOprInfo));
    bulkOprInfo.flags = BULK_OPR_SMALL_FILES;
    rstrcpy (dataObjOprInp->objPath, targColl, MAX_NAME_LEN);
    rstrcpy (bulkOprInp->objPath, targColl, MAX_NAME_LEN);
    /* set the pwd */
    if (*srcDir == '/') {
	/* absolute path */
	bulkOprInfo.cwd[0] = '\0';
    } else {
	if (getcwd (bulkOprInfo.cwd, MAX_NAME_LEN) == NULL) {
	    status = SYS_INVALID_FILE_PATH - errno;
            rodsLogError (LOG_ERROR, status,
              "bulkPutDirUtil: getcwd error for %s", srcDir);
            return status;
	}
    }
    
    /* need to make phyBunDir */
    getPhyBunDir (DEF_PHY_BUN_ROOT_DIR, (*myConn)->clientUser.userName,
      bulkOprInfo.phyBunDir);

    if ((status = mkdirR ("/", bulkOprInfo.phyBunDir, 0750)) < 0) {
        rodsLogError (LOG_ERROR, status,
          "bulkPutDirUtil: mkdirR error for %s", srcDir);
        return status;
    }

    status = putDirUtil (myConn, srcDir, targColl, myRodsEnv, rodsArgs, 
      dataObjOprInp, bulkOprInp, rodsRestart, &bulkOprInfo);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "bulkPutDirUtil: Small files bulkPut error for %s", srcDir);
	return status;
    }

    if (bulkOprInfo.count > 0) {
        status = tarAndBulkPut (*myConn, bulkOprInp, &bulkOprInfo);
        if (status >= 0) {
            /* return the count */
            status = bulkOprInfo.count;
        } else {
            rodsLogError (LOG_ERROR, status,
              "bulkPutDirUtil: tarAndBulkPut error for %s", 
	      bulkOprInfo.cachedSrcPath);
        }
        clearBulkOprInfo (&bulkOprInfo);
    }
    rmdir (bulkOprInfo.phyBunDir);
    return status;
}

int
getPhyBunDir (char *phyBunRootDir, char *userName, char *outPhyBunDir) 
{
    struct stat statbuf;

    while (1)
    {
        snprintf (outPhyBunDir, MAX_NAME_LEN, "%s/%s.phybun.%d", phyBunRootDir,
          userName, (int) random ());
        if (stat (outPhyBunDir, &statbuf) < 0) break;
    }
    return 0;
}

int
bulkPutFileUtil (rcComm_t *conn, char *srcPath, char *targPath,
rodsLong_t srcSize, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs,
bulkOprInp_t *bulkOprInp, bulkOprInfo_t *bulkOprInfo)
{
    char tmpSrcPath[MAX_NAME_LEN];
    char subPhyBunDir[MAX_NAME_LEN];
    char *mySrcPath;
    int status;
    char *phyBunPath;

    phyBunPath =  &bulkOprInfo->phyBunPath[bulkOprInfo->count][0];
    status = getPhyBunPath (bulkOprInp->objPath, targPath, 
      bulkOprInfo->phyBunDir, phyBunPath);
    if (status < 0) return status;

    /* need to create the subPhyBunDir */
    if ((status = splitPathByKey (phyBunPath, subPhyBunDir, tmpSrcPath, 
      '/')) < 0) {
        rodsLogError (LOG_ERROR, status,
          "bulkPutFileUtil: splitPathByKey for %s error",
          phyBunPath);
        return (status);
    }

    if (strcmp (subPhyBunDir, bulkOprInfo->cachedSubPhyBunDir) != 0 &&
      strcmp (subPhyBunDir, bulkOprInfo->phyBunDir) != 0) {
	mkdirR (bulkOprInfo->phyBunDir, subPhyBunDir, 0750);
	rstrcpy (bulkOprInfo->cachedSubPhyBunDir, subPhyBunDir, MAX_NAME_LEN);
    }

    rstrcpy (bulkOprInfo->cachedSrcPath, srcPath, MAX_NAME_LEN);
    /* need to get absolute path for symlink */
    if (strlen (bulkOprInfo->cwd) == 0) {
	mySrcPath = srcPath;
    } else {
	snprintf (tmpSrcPath, MAX_NAME_LEN, "%s/%s",
	  bulkOprInfo->cwd, srcPath);
	mySrcPath = tmpSrcPath;
    }

    if (symlink (tmpSrcPath, phyBunPath) < 0) { 
        status = USER_INPUT_PATH_ERR - errno;
        rodsLogError (LOG_ERROR, status,
        "bulkPutFileUtil: symlink error for %s to %s", tmpSrcPath, phyBunPath);
        return status;
    }
    bulkOprInfo->count++;
    bulkOprInfo->size += srcSize;

    if (bulkOprInfo->count >= MAX_NUM_BULK_OPR_FILES ||
      bulkOprInfo->size >= BULK_OPR_BUF_SIZE - MAX_BULK_OPR_FILE_SIZE) {
	/* tar send it */
	status = tarAndBulkPut (conn, bulkOprInp, bulkOprInfo);
	if (status >= 0) {
	    /* return the count */
	    status = bulkOprInfo->count;
	} else {
            rodsLogError (LOG_ERROR, status,
              "bulkPutFileUtil: tarAndBulkPut error for %s", srcPath);
	}
	clearBulkOprInfo (bulkOprInfo);
    }
    return status;
}

int
tarAndBulkPut (rcComm_t *conn, bulkOprInp_t *bulkOprInp, 
bulkOprInfo_t *bulkOprInfo)
{
    int status;

    if (bulkOprInfo == NULL || bulkOprInfo->count <= 0) return 0;

    bulkOprInfo->bytesBuf.len = BULK_OPR_BUF_SIZE;
    bulkOprInfo->bytesBuf.buf = NULL;

    status = tarToBuf (bulkOprInfo->phyBunDir, &bulkOprInfo->bytesBuf);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "tarAndBulkPut: tarToBuf error for %s", bulkOprInfo->phyBunDir);
        return status;
    }

    /* send it */
    if (bulkOprInfo->bytesBuf.buf != NULL) {
        status = rcBulkDataObjPut (conn, bulkOprInp, &bulkOprInfo->bytesBuf);
    }
    return (status);
}

int
clearBulkOprInfo (bulkOprInfo_t *bulkOprInfo)
{
    int i;

    if (bulkOprInfo == NULL || bulkOprInfo->count <= 0) return 0;
    if (bulkOprInfo->bytesBuf.buf != NULL) {
        free (bulkOprInfo->bytesBuf.buf);
	bulkOprInfo->bytesBuf.buf = NULL;
    }
    bulkOprInfo->bytesBuf.len = 0;

    for (i = 0; i < bulkOprInfo->count; i++) {
	unlink (&bulkOprInfo->phyBunPath[i][0]);
    }
    rmSubDir (bulkOprInfo->phyBunDir);
    bulkOprInfo->count = bulkOprInfo->size = 0;
    *bulkOprInfo->cachedSrcPath = *bulkOprInfo->cachedSubPhyBunDir = '\0';
    /* rmdir all subPhyBunDir */

    return 0;
}


int
getPhyBunPath (char *collection, char *objPath, char *phyBunDir,
char *outPhyBunPath)
{
    char *subPath;
    int collLen = strlen (collection);

    subPath = objPath + collLen;
    if (*subPath != '/') {
        rodsLogError (LOG_ERROR, USER_INPUT_PATH_ERR,
          "getPhyBunPath: inconsistent collection %s and objPath %s", 
	  collection, objPath);
        return USER_INPUT_PATH_ERR;
    }
    snprintf (outPhyBunPath, MAX_NAME_LEN, "%s%s", phyBunDir, subPath);
    return 0;
}

