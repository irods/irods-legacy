/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsStructFileExtAndReg.c. See structFileExtAndReg.h for a description of 
 * this API call.*/

#include "structFileExtAndReg.h"
#include "apiHeaderAll.h"
#include "objMetaOpr.h"
#include "dataObjOpr.h"
#include "miscServerFunct.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"

int
rsStructFileExtAndReg (rsComm_t *rsComm,
structFileExtAndRegInp_t *structFileExtAndRegInp)
{
    int status;
    dataObjInp_t dataObjInp;
    openedDataObjInp_t dataObjCloseInp;
    dataObjInfo_t *dataObjInfo;
    int l1descInx;
    rescInfo_t *rescInfo;
    char *rescGroupName;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    char *bunFilePath;
    char phyBunDir[MAX_NAME_LEN];
    int flags = 0;
#if 0
    dataObjInp_t dirRegInp;
    structFileOprInp_t structFileOprInp;
#endif
    specCollCache_t *specCollCache = NULL;

    resolveLinkedPath (rsComm, structFileExtAndRegInp->objPath, &specCollCache,
      &structFileExtAndRegInp->condInput);

    resolveLinkedPath (rsComm, structFileExtAndRegInp->collection, 
      &specCollCache, NULL);

    if (!isSameZone (structFileExtAndRegInp->objPath, 
      structFileExtAndRegInp->collection))
        return SYS_CROSS_ZONE_MV_NOT_SUPPORTED;

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, structFileExtAndRegInp->objPath,
      MAX_NAME_LEN);

    /* replicate the condInput. may have resource input */
    replKeyVal (&structFileExtAndRegInp->condInput, &dataObjInp.condInput);
    dataObjInp.openFlags = O_RDONLY;

    remoteFlag = getAndConnRemoteZone (rsComm, &dataObjInp, &rodsServerHost,
      REMOTE_OPEN);

    if (remoteFlag < 0) {
        return (remoteFlag);
    } else if (remoteFlag == REMOTE_HOST) {
        status = rcStructFileExtAndReg (rodsServerHost->conn, 
          structFileExtAndRegInp);
        return status;
    }

    /* open the structured file */
    addKeyVal (&dataObjInp.condInput, NO_OPEN_FLAG_KW, "");
    l1descInx = _rsDataObjOpen (rsComm, &dataObjInp);

    if (l1descInx < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileExtAndReg: _rsDataObjOpen of %s error. status = %d",
          dataObjInp.objPath, l1descInx);
        return (l1descInx);
    }

    rescInfo = L1desc[l1descInx].dataObjInfo->rescInfo;
    rescGroupName = L1desc[l1descInx].dataObjInfo->rescGroupName;
    remoteFlag = resolveHostByRescInfo (rescInfo, &rodsServerHost);
    bzero (&dataObjCloseInp, sizeof (dataObjCloseInp));
    dataObjCloseInp.l1descInx = l1descInx;

    if (remoteFlag == REMOTE_HOST) {
        addKeyVal (&structFileExtAndRegInp->condInput, RESC_NAME_KW,
          rescInfo->rescName);

        if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
            return status;
        }
        status = rcStructFileExtAndReg (rodsServerHost->conn,
          structFileExtAndRegInp);

        rsDataObjClose (rsComm, &dataObjCloseInp);


        return status;
    }

    status = chkCollForExtAndReg (rsComm, structFileExtAndRegInp->collection);
    if (status < 0) return status;


    dataObjInfo = L1desc[l1descInx].dataObjInfo;

    createPhyBundleDir (rsComm, dataObjInfo->filePath, phyBunDir);

    status = unbunPhyBunFile (rsComm, &dataObjInp, rescInfo, 
      dataObjInfo->filePath, phyBunDir);

    if (status == SYS_DIR_IN_VAULT_NOT_EMPTY) {
	/* rename the phyBunDir */
	snprintf (phyBunDir, MAX_NAME_LEN, "%s.%-d", 
	 phyBunDir, (int) random ());
        status = unbunPhyBunFile (rsComm, &dataObjInp, rescInfo,
          dataObjInfo->filePath, phyBunDir);
    }
    if (status < 0) {
        rodsLog (LOG_ERROR,
        "rsStructFileExtAndReg:unbunPhyBunFile err for %s to dir %s.stat=%d",
          bunFilePath, phyBunDir, status);
        rsDataObjClose (rsComm, &dataObjCloseInp);
        return status;
    }

    if (getValByKey (&structFileExtAndRegInp->condInput, FORCE_FLAG_KW) 
      != NULL) {
	flags = flags | FORCE_FLAG_FLAG;
    }
    if (getValByKey (&structFileExtAndRegInp->condInput, BULK_OPR_KW)
      != NULL) {
        flags = flags | BULK_OPR_FLAG;
    }

    status = regUnbunSubfiles (rsComm, rescInfo, rescGroupName,
      structFileExtAndRegInp->collection, phyBunDir, flags, NULL);

    if (status == CAT_NO_ROWS_FOUND) {
        /* some subfiles have been deleted. harmless */
        status = 0;
    } else if (status < 0) {
        rodsLog (LOG_ERROR,
          "_rsUnbunAndRegPhyBunfile: regUnbunPhySubfiles for dir %s. stat = %d",
          phyBunDir, status);
    }
    rsDataObjClose (rsComm, &dataObjCloseInp);

    return status;
}

int 
chkCollForExtAndReg (rsComm_t *rsComm, char *collection)
{
    dataObjInp_t dataObjInp;
    int status;
    rodsObjStat_t *rodsObjStatOut = NULL;

    bzero (&dataObjInp, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, collection, MAX_NAME_LEN);
    status = collStat (rsComm, &dataObjInp, &rodsObjStatOut);
    if (status == CAT_NO_ROWS_FOUND) {
	status = rsMkCollR (rsComm, "/", collection);
	if (status < 0) {
            rodsLog (LOG_ERROR,
              "chkCollForExtAndReg: rsMkCollR of %s error. status = %d",
              collection, status);
            return (status);
	} else {
	    status = collStat (rsComm, &dataObjInp, &rodsObjStatOut);
	}
    }

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "chkCollForExtAndReg: collStat of %s error. status = %d",
          dataObjInp.objPath, status);
        return (status);
    } else if (rodsObjStatOut->specColl != NULL) {
        freeRodsObjStat (rodsObjStatOut);
        rodsLog (LOG_ERROR,
          "chkCollForExtAndReg: %s is a mounted collection",
          dataObjInp.objPath);
        return (SYS_STRUCT_FILE_INMOUNTED_COLL);
    }
    freeRodsObjStat (rodsObjStatOut);

    status = checkCollAccessPerm (rsComm, collection, ACCESS_DELETE_OBJECT);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "chkCollForExtAndReg: no permission to write %s, status = %d",
          collection, status);
    }
    return (status);
}


/* regUnbunSubfiles - The top level for registering all files in phyBunDir 
 * to the collection Valid values for flags are: 
 *     BULK_OPR_FLAG and FORCE_FLAG_FLAG.
 */

int
regUnbunSubfiles (rsComm_t *rsComm, rescInfo_t *rescInfo, char *rescGroupName,
char *collection, char *phyBunDir, int flags, genQueryOut_t *attriArray)
{
    genQueryOut_t bulkDataObjRegInp; 
    renamedPhyFiles_t renamedPhyFiles;
    int status = 0;

    if ((flags & BULK_OPR_FLAG) != 0) {
	bzero (&renamedPhyFiles, sizeof (renamedPhyFiles));
	initBulkDataObjRegInp (&bulkDataObjRegInp);
	/* the continueInx is used for the matching of objPath */
	if (attriArray != NULL) attriArray->continueInx = 0;
    }
    status = _regUnbunSubfiles (rsComm, rescInfo, rescGroupName, collection, 
      phyBunDir, flags, &bulkDataObjRegInp, &renamedPhyFiles, attriArray);
 
    if ((flags & BULK_OPR_FLAG) != 0) {
        if (bulkDataObjRegInp.rowCnt > 0) {
	    int status1;
            status1 = rsBulkDataObjReg (rsComm, &bulkDataObjRegInp);
            if (status1 < 0) {
		status = status1;
                rodsLog (LOG_ERROR,
                  "regUnbunSubfiles: rsBulkDataObjReg error for %s. stat = %d",
                  collection, status1);
                cleanupBulkRegFiles (rsComm, &bulkDataObjRegInp);
            }
            postProcRenamedPhyFiles (&renamedPhyFiles, status);
	}
	clearGenQueryOut (&bulkDataObjRegInp);
    }
    return status;
}

/* _regUnbunSubfiles - non bulk version of registering all files in phyBunDir 
 * to the collection. Valid values for flags are: 
 *	BULK_OPR_FLAG and FORCE_FLAG_FLAG.
 */

int
_regUnbunSubfiles (rsComm_t *rsComm, rescInfo_t *rescInfo, char *rescGroupName,
char *collection, char *phyBunDir, int flags, genQueryOut_t *bulkDataObjRegInp,
renamedPhyFiles_t *renamedPhyFiles, genQueryOut_t *attriArray)
{
    DIR *dirPtr;
    struct dirent *myDirent;
    struct stat statbuf;
    char subfilePath[MAX_NAME_LEN];
    char subObjPath[MAX_NAME_LEN];
    dataObjInp_t dataObjInp;
    int status;
    int savedStatus = 0;

    dirPtr = opendir (phyBunDir);
    if (dirPtr == NULL) {
        rodsLog (LOG_ERROR,
        "regUnbunphySubfiles: opendir error for %s, errno = %d",
         phyBunDir, errno);
        return (UNIX_FILE_OPENDIR_ERR - errno);
    }
    bzero (&dataObjInp, sizeof (dataObjInp));
    while ((myDirent = readdir (dirPtr)) != NULL) {
        if (strcmp (myDirent->d_name, ".") == 0 ||
          strcmp (myDirent->d_name, "..") == 0) {
            continue;
        }
        snprintf (subfilePath, MAX_NAME_LEN, "%s/%s",
          phyBunDir, myDirent->d_name);

        status = stat (subfilePath, &statbuf);

        if (status != 0) {
            rodsLog (LOG_ERROR,
              "regUnbunphySubfiles: stat error for %s, errno = %d",
              subfilePath, errno);
            savedStatus = UNIX_FILE_STAT_ERR - errno;
	    unlink (subfilePath);
	    continue;
        }

        snprintf (subObjPath, MAX_NAME_LEN, "%s/%s",
          collection, myDirent->d_name);

        if ((statbuf.st_mode & S_IFDIR) != 0) {
            status = rsMkCollR (rsComm, "/", subObjPath);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "regUnbunSubfiles: rsMkCollR of %s error. status = %d",
                  subObjPath, status);
                savedStatus = status;
		continue;
	    }
	    status = _regUnbunSubfiles (rsComm, rescInfo, rescGroupName,
	      subObjPath, subfilePath, flags, bulkDataObjRegInp, 
	      renamedPhyFiles, attriArray);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "regUnbunSubfiles: regUnbunSubfiles of %s error. status=%d",
                  subObjPath, status);
                savedStatus = status;
                continue;
            }
        } else if ((statbuf.st_mode & S_IFREG) != 0) {
	    if ((flags & BULK_OPR_FLAG) == 0) {
	        status = regSubfile (rsComm, rescInfo, rescGroupName,
		    subObjPath, subfilePath, statbuf.st_size, flags);
	        unlink (subfilePath);
                if (status < 0) {
                    rodsLog (LOG_ERROR,
                      "regUnbunSubfiles: regSubfile of %s error. status=%d",
                      subObjPath, status);
                    savedStatus = status;
                    continue;
		}
	    } else {
                status = bulkProcAndRegSubfile (rsComm, rescInfo, rescGroupName,
		  subObjPath, subfilePath, statbuf.st_size, 
		  statbuf.st_mode & 0777, flags, bulkDataObjRegInp, 
		  renamedPhyFiles, attriArray);
	        unlink (subfilePath);
                if (status < 0) {
                    rodsLog (LOG_ERROR,
                     "regUnbunSubfiles:bulkProcAndRegSubfile of %s err.stat=%d",
                        subObjPath, status);
                    savedStatus = status;
                    continue;
                }
	    }
	}
    }
    closedir (dirPtr);
    rmdir (phyBunDir);
    return savedStatus;
}

int
regSubfile (rsComm_t *rsComm, rescInfo_t *rescInfo, char *rescGroupName,
char *subObjPath, char *subfilePath, rodsLong_t dataSize, int flags)
{
    dataObjInfo_t dataObjInfo;
    dataObjInp_t dataObjInp;
    struct stat statbuf;
    int status;
    int modFlag = 0;

    bzero (&dataObjInp, sizeof (dataObjInp));
    bzero (&dataObjInfo, sizeof (dataObjInfo));
    rstrcpy (dataObjInp.objPath, subObjPath, MAX_NAME_LEN);
    rstrcpy (dataObjInfo.objPath, subObjPath, MAX_NAME_LEN);
    rstrcpy (dataObjInfo.rescName, rescInfo->rescName, NAME_LEN);
    rstrcpy (dataObjInfo.dataType, "generic", NAME_LEN);
    dataObjInfo.rescInfo = rescInfo;
    rstrcpy (dataObjInfo.rescGroupName, rescGroupName, NAME_LEN);
    dataObjInfo.dataSize = dataSize;

    status = getFilePathName (rsComm, &dataObjInfo, &dataObjInp);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "regSubFile: getFilePathName err for %s. status = %d",
          dataObjInp.objPath, status);
        return (status);
    }

    status = stat (dataObjInfo.filePath, &statbuf);
    if (status == 0 || errno != ENOENT) {
        if ((statbuf.st_mode & S_IFDIR) != 0) {
	    return SYS_PATH_IS_NOT_A_FILE;
	}

        if (chkOrphanFile (rsComm, dataObjInfo.filePath, rescInfo->rescName, 
	  &dataObjInfo) > 0) {
	    /* an orphan file. just rename it */
	    fileRenameInp_t fileRenameInp;
	    bzero (&fileRenameInp, sizeof (fileRenameInp));
            rstrcpy (fileRenameInp.oldFileName, dataObjInfo.filePath, 
	      MAX_NAME_LEN);
            status = renameFilePathToNewDir (rsComm, ORPHAN_DIR, 
	      &fileRenameInp, rescInfo, 1);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "regSubFile: renameFilePathToNewDir err for %s. status = %d",
                  fileRenameInp.oldFileName, status);
                return (status);
	    }
	} else {
	    /* not an orphan file */
	    if ((flags & FORCE_FLAG_FLAG) != 0 && dataObjInfo.dataId > 0 && 
	      strcmp (dataObjInfo.objPath, subObjPath) == 0) {
		/* overwrite the current file */
		modFlag = 1;
		unlink (dataObjInfo.filePath);
	    } else {
		status = SYS_COPY_ALREADY_IN_RESC;
                rodsLog (LOG_ERROR,
                  "regSubFile: phypath %s is already in use. status = %d",
                  dataObjInfo.filePath, status);
                return (status);
	    }
        }
    }
    /* make the necessary dir */
    mkDirForFilePath (UNIX_FILE_TYPE, rsComm, "/", dataObjInfo.filePath,
      getDefDirMode ());
    /* add a link */
    status = link (subfilePath, dataObjInfo.filePath);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "regSubFile: link error %s to %s. errno = %d",
          subfilePath, dataObjInfo.filePath, errno);
        return (UNIX_FILE_LINK_ERR - errno);
    }

    if (modFlag == 0) {
        status = svrRegDataObj (rsComm, &dataObjInfo);
    } else {
        char tmpStr[MAX_NAME_LEN];
        modDataObjMeta_t modDataObjMetaInp;
	keyValPair_t regParam;

	bzero (&modDataObjMetaInp, sizeof (modDataObjMetaInp));
	bzero (&regParam, sizeof (regParam));
        snprintf (tmpStr, MAX_NAME_LEN, "%lld", dataSize);
        addKeyVal (&regParam, DATA_SIZE_KW, tmpStr);
        addKeyVal (&regParam, ALL_REPL_STATUS_KW, tmpStr);
        snprintf (tmpStr, MAX_NAME_LEN, "%d", (int) time (NULL));
        addKeyVal (&regParam, DATA_MODIFY_KW, tmpStr);

        modDataObjMetaInp.dataObjInfo = &dataObjInfo;
        modDataObjMetaInp.regParam = &regParam;

        status = rsModDataObjMeta (rsComm, &modDataObjMetaInp);

        clearKeyVal (&regParam);
    }

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "regSubFile: svrRegDataObj of %s. errno = %d",
          dataObjInfo.objPath, errno);
	unlink (dataObjInfo.filePath);
    }
    return status;
}

int
bulkProcAndRegSubfile (rsComm_t *rsComm, rescInfo_t *rescInfo, 
char *rescGroupName, char *subObjPath, char *subfilePath, rodsLong_t dataSize, 
int dataMode, int flags, genQueryOut_t *bulkDataObjRegInp, 
renamedPhyFiles_t *renamedPhyFiles, genQueryOut_t *attriArray)
{
    dataObjInfo_t dataObjInfo;
    dataObjInp_t dataObjInp;
    struct stat statbuf;
    int status;
    int modFlag = 0;
    char *myChksum = NULL;
    int myDataMode = dataMode;

    bzero (&dataObjInp, sizeof (dataObjInp));
    bzero (&dataObjInfo, sizeof (dataObjInfo));
    rstrcpy (dataObjInp.objPath, subObjPath, MAX_NAME_LEN);
    rstrcpy (dataObjInfo.objPath, subObjPath, MAX_NAME_LEN);
    rstrcpy (dataObjInfo.rescName, rescInfo->rescName, NAME_LEN);
    rstrcpy (dataObjInfo.dataType, "generic", NAME_LEN);
    dataObjInfo.rescInfo = rescInfo;
    dataObjInfo.dataSize = dataSize;

    status = getFilePathName (rsComm, &dataObjInfo, &dataObjInp);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "regSubFile: getFilePathName err for %s. status = %d",
          dataObjInp.objPath, status);
        return (status);
    }

    status = stat (dataObjInfo.filePath, &statbuf);
    if (status == 0 || errno != ENOENT) {
        if ((statbuf.st_mode & S_IFDIR) != 0) {
	    return SYS_PATH_IS_NOT_A_FILE;
	}
        if (chkOrphanFile (rsComm, dataObjInfo.filePath, rescInfo->rescName, 
	  &dataObjInfo) <= 0) {
            /* not an orphan file */
            if ((flags & FORCE_FLAG_FLAG) != 0 && dataObjInfo.dataId > 0 &&
              strcmp (dataObjInfo.objPath, subObjPath) == 0) {
                /* overwrite the current file */
                modFlag = 1;
            } else {
	        status = SYS_COPY_ALREADY_IN_RESC;
                rodsLog (LOG_ERROR,
                  "bulkProcAndRegSubfile: phypath %s is already in use. status = %d",
                  dataObjInfo.filePath, status);
                return (status);
	    }
        }
        /* rename it to the orphan dir */
        fileRenameInp_t fileRenameInp;
        bzero (&fileRenameInp, sizeof (fileRenameInp));
        rstrcpy (fileRenameInp.oldFileName, dataObjInfo.filePath,
          MAX_NAME_LEN);
        status = renameFilePathToNewDir (rsComm, ORPHAN_DIR,
          &fileRenameInp, rescInfo, 1);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "bulkProcAndRegSubfile: renameFilePathToNewDir err for %s. status = %d",
              fileRenameInp.oldFileName, status);
            return (status);
        }
	if (modFlag > 0) {
	    status = addRenamedPhyFile (subObjPath, fileRenameInp.oldFileName,
	      fileRenameInp.newFileName, renamedPhyFiles);
	    if (status < 0) return status;
	}
    } else {
        /* make the necessary dir */
        mkDirForFilePath (UNIX_FILE_TYPE, rsComm, "/", dataObjInfo.filePath,
          getDefDirMode ());
    }
    /* add a link */
    status = link (subfilePath, dataObjInfo.filePath);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "bulkProcAndRegSubfile: link error %s to %s. errno = %d",
          subfilePath, dataObjInfo.filePath, errno);
        return (UNIX_FILE_LINK_ERR - errno);
    }

    if (attriArray != NULL) {
	/* dataMode in attriArray overwrites passed in value */
	status =getAttriInAttriArray (subObjPath, attriArray, &myDataMode,
	  &myChksum);
        if (status < 0) {
            rodsLog (LOG_NOTICE,
              "bulkProcAndRegSubfile: matchObjPath error for %s, stat = %d",
              subObjPath, status);
	} else {
	    if ((flags & VERIFY_CHKSUM_FLAG) != 0 && myChksum != NULL) {
		char chksumStr[NAME_LEN];
		/* verify the chksum */
		status = chksumLocFile (dataObjInfo.filePath, chksumStr);
        	if (status < 0) {
		    rodsLog (LOG_ERROR,
                     "bulkProcAndRegSubfile: chksumLocFile error for %s ", 
		      dataObjInfo.filePath);
                    return (status);
		}
		if (strcmp (myChksum, chksumStr) != 0) {
        	    rodsLog (LOG_ERROR,
                      "bulkProcAndRegSubfile: chksum of %s %s != input %s",
		        dataObjInfo.filePath, chksumStr, myChksum);
                    return (USER_CHKSUM_MISMATCH);
		}
	    }
        }
    }

    status = bulkRegSubfile (rsComm, rescInfo->rescName, rescGroupName,
      subObjPath, dataObjInfo.filePath, dataSize, myDataMode, modFlag, 
      dataObjInfo.replNum, myChksum, bulkDataObjRegInp, renamedPhyFiles);

    return status;
}

int
bulkRegSubfile (rsComm_t *rsComm, char *rescName, char *rescGroupName,
char *subObjPath, char *subfilePath, rodsLong_t dataSize, int dataMode, 
int modFlag, int replNum, char *chksum, genQueryOut_t *bulkDataObjRegInp, 
renamedPhyFiles_t *renamedPhyFiles)
{
    int status;

    /* XXXXXXXX use NULL for chksum for now */
    status = fillBulkDataObjRegInp (rescName, rescGroupName, subObjPath, 
      subfilePath, "generic", dataSize, dataMode, modFlag, 
      replNum, chksum, bulkDataObjRegInp);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "bulkRegSubfile: fillBulkDataObjRegInp error for %s. status = %d",
          subfilePath, status);
        return status;
    }

    if (bulkDataObjRegInp->rowCnt >= MAX_NUM_BULK_OPR_FILES) {
        status = rsBulkDataObjReg (rsComm, bulkDataObjRegInp);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "bulkRegSubfile: rsBulkDataObjReg error for %s. status = %d",
              subfilePath, status);
	    cleanupBulkRegFiles (rsComm, bulkDataObjRegInp);
	}
	postProcRenamedPhyFiles (renamedPhyFiles, status);
	bulkDataObjRegInp->rowCnt = 0;
    }
    return status;
}

int
addRenamedPhyFile (char *subObjPath, char *oldFileName, char *newFileName, 
renamedPhyFiles_t *renamedPhyFiles)
{
    if (subObjPath == NULL || oldFileName == NULL || newFileName == NULL ||
      renamedPhyFiles == NULL) return USER__NULL_INPUT_ERR;

    if (renamedPhyFiles->count >= MAX_NUM_BULK_OPR_FILES) {
        rodsLog (LOG_ERROR,
          "addRenamedPhyFile: count >= %d for %s", MAX_NUM_BULK_OPR_FILES,
          subObjPath);
        return (SYS_RENAME_STRUCT_COUNT_EXCEEDED);
    }
    rstrcpy (&renamedPhyFiles->objPath[renamedPhyFiles->count][0], 
      subObjPath, MAX_NAME_LEN);
    rstrcpy (&renamedPhyFiles->origFilePath[renamedPhyFiles->count][0], 
      oldFileName, MAX_NAME_LEN);
    rstrcpy (&renamedPhyFiles->newFilePath[renamedPhyFiles->count][0], 
      newFileName, MAX_NAME_LEN);
    renamedPhyFiles->count++;
    return 0;
}

int
postProcRenamedPhyFiles (renamedPhyFiles_t *renamedPhyFiles, int regStatus)
{
    int i;
    int status = 0;
    int savedStatus = 0;

    if (renamedPhyFiles == NULL) return USER__NULL_INPUT_ERR;

    if (regStatus >= 0) {
	for (i = 0; i < renamedPhyFiles->count; i++) {
	    unlink (&renamedPhyFiles->newFilePath[i][0]);
	}
    } else {
	/* restore the phy files */
	for (i = 0; i < renamedPhyFiles->count; i++) {
	    status = rename (&renamedPhyFiles->newFilePath[i][0], 
	      &renamedPhyFiles->origFilePath[i][0]);
	    savedStatus = UNIX_FILE_RENAME_ERR - errno;
            rodsLog (LOG_ERROR,
              "postProcRenamedPhyFiles: rename error from %s to %s, status=%d",
	      &renamedPhyFiles->newFilePath[i][0],
              &renamedPhyFiles->origFilePath[i][0], savedStatus);
	}
    }
    bzero (renamedPhyFiles, sizeof (renamedPhyFiles_t));

    return savedStatus;
}

int
cleanupBulkRegFiles (rsComm_t *rsComm, genQueryOut_t *bulkDataObjRegInp)
{
    sqlResult_t *filePath, *rescName;
    char *tmpFilePath, *tmpRescName;
    int i;

    if (bulkDataObjRegInp == NULL) return USER__NULL_INPUT_ERR;

    if ((filePath =
      getSqlResultByInx (bulkDataObjRegInp, COL_D_DATA_PATH)) == NULL) {
        rodsLog (LOG_NOTICE,
          "cleanupBulkRegFiles: getSqlResultByInx for COL_D_DATA_PATH failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((rescName =
      getSqlResultByInx (bulkDataObjRegInp, COL_D_RESC_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "rsBulkDataObjReg: getSqlResultByInx for COL_D_RESC_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    for (i = 0;i < bulkDataObjRegInp->rowCnt; i++) {
        tmpFilePath = &filePath->value[filePath->len * i];
        tmpRescName = &rescName->value[rescName->len * i];
	/* make sure it is an orphan file */
        if (chkOrphanFile (rsComm, tmpFilePath, tmpRescName, NULL) > 0) {
            unlink (tmpFilePath);
	}
    }

    return 0;
}

