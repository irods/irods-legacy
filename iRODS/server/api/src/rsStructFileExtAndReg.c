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
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    char *bunFilePath;
    char phyBunDir[MAX_NAME_LEN];
#if 0
    dataObjInp_t dirRegInp;
    structFileOprInp_t structFileOprInp;
#endif

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

    status = regUnbunSubfiles (rsComm, rescInfo, 
      structFileExtAndRegInp->collection, phyBunDir);

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

#if 0
    status = initStructFileOprInp (rsComm, &structFileOprInp,
      structFileExtAndRegInp, dataObjInfo);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileExtAndReg: initStructFileOprInp of %s error. stat = %d",
          dataObjInp.objPath, status);
        rsDataObjClose (rsComm, &dataObjCloseInp);
        return (status);
    }

    status = rsStructFileExtract (rsComm, &structFileOprInp);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileExtAndReg: rsStructFileExtract of %s error. stat = %d",
          dataObjInp.objPath, status);
        rsDataObjClose (rsComm, &dataObjCloseInp);
        return (status);
    }

    /* register the files in cacheDir */
    memset (&dirRegInp, 0, sizeof (dirRegInp));
    addKeyVal (&dirRegInp.condInput, FILE_PATH_KW, 
      structFileOprInp.specColl->cacheDir);
    rstrcpy (dirRegInp.objPath, structFileExtAndRegInp->collection, 
      MAX_NAME_LEN);
    addKeyVal (&dirRegInp.condInput, COLLECTION_KW, "");
    /* collection permission was checked in chkCollForExtAndReg */
    addKeyVal (&dirRegInp.condInput, DEST_RESC_NAME_KW, 
      dataObjInfo->rescName);

    status = phyPathRegNoChkPerm (rsComm, &dirRegInp);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileExtAndReg: rsPhyPathReg of %s to %s error. stat = %d",
          structFileOprInp.specColl->cacheDir, dirRegInp.objPath, status);
        rodsLog (LOG_ERROR,
          "rsStructFileExtAndReg: Orphan files may be left in %s",
          structFileOprInp.specColl->cacheDir);
    }

    dataObjCloseInp.l1descInx = l1descInx;
    rsDataObjClose (rsComm, &dataObjCloseInp);

    return (status);
#endif
}

int 
chkCollForExtAndReg (rsComm_t *rsComm, char *collection)
{
#if 0
    int status;
    collInp_t openCollInp;
    collEnt_t *collEnt;
    int handleInx;
    collInp_t modCollInp;

    memset (&openCollInp, 0, sizeof (openCollInp));
    rstrcpy (openCollInp.collName, collection, MAX_NAME_LEN);

    handleInx = rsOpenCollection (rsComm, &openCollInp);
    if (handleInx < 0) {
	if (handleInx == USER_FILE_DOES_NOT_EXIST) return 0;
        rodsLog (LOG_ERROR,
          "chkCollForExtAndReg: rsOpenCollection of %s error. status = %d",
          openCollInp.collName, handleInx);
        return (handleInx);
    }

    if (CollHandle[handleInx].rodsObjStat->specColl != NULL) {
        rodsLog (LOG_ERROR,
          "chkCollForExtAndReg: %s is a mounted collection",
          openCollInp.collName);
        return (SYS_STRUCT_FILE_INMOUNTED_COLL);
    }

    status = rsReadCollection (rsComm, &handleInx, &collEnt);
    rsCloseCollection (rsComm, &handleInx);
    if (status >= 0) {
        rodsLog (LOG_ERROR,
          "chkCollForExtAndReg: %s is a not empty",
          openCollInp.collName);
	return (SYS_COLLECTION_NOT_EMPTY);
    }
    /* now check if we can register into the collection. should call
     * checkAndGetObjectId() but don't have an external API for it.
     * Hack it with rsModColl */

    memset (&modCollInp, 0, sizeof (modCollInp));
    rstrcpy (modCollInp.collName, collection, MAX_NAME_LEN);
    /* this is not a mounted collion. so it won't hurt */
    addKeyVal (&modCollInp.condInput, COLLECTION_TYPE_KW,
      "NULL_SPECIAL_VALUE");

    status = rsModColl (rsComm, &modCollInp);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "chkCollForExtAndReg: problem with permission of %s, status = %d",
          openCollInp.collName, status);
        return (status);
    }
#endif
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
        free (rodsObjStatOut);
        rodsLog (LOG_ERROR,
          "chkCollForExtAndReg: %s is a mounted collection",
          dataObjInp.objPath);
        return (SYS_STRUCT_FILE_INMOUNTED_COLL);
    }
    free (rodsObjStatOut);

    status = checkCollAccessPerm (rsComm, collection, ACCESS_DELETE_OBJECT);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "chkCollForExtAndReg: no permission to write %s, status = %d",
          collection, status);
    }
    return (status);
}

int
regUnbunSubfiles (rsComm_t *rsComm, rescInfo_t *rescInfo, char *collection,
char *phyBunDir)
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
	    status = regUnbunSubfiles (rsComm, rescInfo, subObjPath,
	      subfilePath);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "regUnbunSubfiles: regUnbunSubfiles of %s error. status=%d",
                  subObjPath, status);
                savedStatus = status;
                continue;
            }
        } else if ((statbuf.st_mode & S_IFREG) != 0) {
	    status = regSubfile (rsComm, rescInfo, subObjPath, subfilePath,
	      statbuf.st_size);
	    unlink (subfilePath);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "regUnbunSubfiles: regSubfile of %s error. status=%d",
                  subObjPath, status);
                savedStatus = status;
                continue;
            }
	}
    }
    closedir (dirPtr);
    rmdir (phyBunDir);
    return savedStatus;
}

int
regSubfile (rsComm_t *rsComm, rescInfo_t *rescInfo, char *subObjPath,
char *subfilePath, rodsLong_t dataSize)
{
    dataObjInfo_t dataObjInfo;
    dataObjInp_t dataObjInp;
    struct stat statbuf;
    int status;

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
        status = resolveDupFilePath (rsComm, &dataObjInfo, &dataObjInp);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "regSubFile: resolveDupFilePath err for %s. status = %d",
              dataObjInfo.filePath, status);
            return (status);
        }
    }
    /* make the necessary dir */
    mkDirForFilePath (UNIX_FILE_TYPE, rsComm, "/", dataObjInfo.filePath,
      DEFAULT_DIR_MODE);
    /* add a link */
    status = link (subfilePath, dataObjInfo.filePath);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "regSubFile: link error %s to %s. errno = %d",
          subfilePath, dataObjInfo.filePath, errno);
        return (UNIX_FILE_LINK_ERR - errno);
    }

    status = svrRegDataObj (rsComm, &dataObjInfo);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "regSubFile: svrRegDataObj of %s. errno = %d",
          dataObjInfo.objPath, errno);
	unlink (dataObjInfo.filePath);
    }
    return status;
}

