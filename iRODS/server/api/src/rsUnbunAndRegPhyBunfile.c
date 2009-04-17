/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsStructFileExtAndReg.c. See unbunAndRegPhyBunfile.h for a description of 
 * this API call.*/

#include "unbunAndRegPhyBunfile.h"
#include "apiHeaderAll.h"
#include "miscServerFunct.h"
#include "objMetaOpr.h"
#include "dataObjOpr.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"

int
rsUnbunAndRegPhyBunfile (rsComm_t *rsComm, dataObjInp_t *dataObjInp)
{
    int status;
    char *rescName;
    rescGrpInfo_t *rescGrpInfo = NULL;

    if ((rescName = getValByKey (&dataObjInp->condInput, DEST_RESC_NAME_KW)) 
      == NULL) {
        return USER_NO_RESC_INPUT_ERR;
    }

    status = resolveAndQueResc (rescName, NULL, &rescGrpInfo);
    if (status < 0) {
        rodsLog (LOG_NOTICE,
          "rsUnbunAndRegPhyBunfile: resolveAndQueRescerror for %s, status = %d",
          rescName, status);
        return (status);
    }

    status = _rsUnbunAndRegPhyBunfile (rsComm, dataObjInp, 
      rescGrpInfo->rescInfo);

    return (status);
}

int
_rsUnbunAndRegPhyBunfile (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
rescInfo_t *rescInfo)
{
    int status;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    char *bunFilePath;
    char phyBunDir[MAX_NAME_LEN];

    remoteFlag = resolveHostByRescInfo (rescInfo, &rodsServerHost);

    if (remoteFlag == REMOTE_HOST) {
        addKeyVal (&dataObjInp->condInput, DEST_RESC_NAME_KW, 
	  rescInfo->rescName);
	status = remoteUnbunAndRegPhyBunfile (rsComm, dataObjInp, 
	  rodsServerHost);
    }
    /* process this locally */
    if ((bunFilePath = getValByKey (&dataObjInp->condInput, FILE_PATH_KW))
      == NULL) {
        rodsLog (LOG_ERROR,
          "_rsUnbunAndRegPhyBunfile: No filePath input for %s",
          dataObjInp->objPath);
        return (SYS_INVALID_FILE_PATH);
    }

    createPhyBundleDir (rsComm, bunFilePath, phyBunDir);

    status = unbunPhyBunFile (rsComm, dataObjInp, rescInfo, bunFilePath,
      phyBunDir);

    if (status < 0) return status;

    return status;
}

int 
regUnbunPhySubfiles (rsComm_t *rsComm, rescInfo_t *rescInfo, char *phyBunDir)
{
    DIR *dirPtr;
    struct dirent *myDirent;
    struct stat statbuf;
    char subfilePath[MAX_NAME_LEN];
    dataObjInp_t dataObjInp;
    int status;
    dataObjInfo_t *dataObjInfoHead = NULL; 
    dataObjInfo_t *bunDataObjInfo= NULL; 	/* the copy in BUNDLE_RESC */

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
            closedir (dirPtr);
            return (UNIX_FILE_STAT_ERR - errno);
        }

        if ((statbuf.st_mode & S_IFREG) == 0) continue;

	/* do the registration */
	addKeyVal (&dataObjInp.condInput, QUERY_BY_DATA_ID_KW, 
	  myDirent->d_name);
	status = getDataObjInfo (rsComm, &dataObjInp, &dataObjInfoHead,
	  NULL, 1);
       if (status < 0) {
            rodsLog (LOG_DEBUG,
              "regUnbunphySubfiles: getDataObjInfo error for %s, status = %d",
              subfilePath, status);
	    /* don't terminate beause the data Obj may be deleted */
	    unlink (subfilePath);
	    continue;
        }
	requeDataObjInfoByResc (&dataObjInfoHead, BUNDLE_RESC, 1, 1);
	bunDataObjInfo = NULL;
	if (strcmp (dataObjInfoHead->rescName, BUNDLE_RESC) != 0) {
	    /* no match */
	    rodsLog (LOG_DEBUG,
              "regUnbunphySubfiles: No copy in BUNDLE_RESC for %s",
              dataObjInfoHead->objPath);
            /* don't terminate beause the copy may be deleted */
	    unlink (subfilePath);
            continue;
        } else {
	    bunDataObjInfo = dataObjInfoHead;
	}
	requeDataObjInfoByResc (&dataObjInfoHead, rescInfo->rescName, 1, 1);
	/* The copy in DEST_RESC_NAME_KW should be on top */
	if (strcmp (dataObjInfoHead->rescName, rescInfo->rescName) != 0) {
	    /* no copy. stage it */
	    status = regPhySubFile (rsComm, subfilePath, bunDataObjInfo, 
	      rescInfo);
	} else {
	    /* have a copy. don't do anything for now */
            unlink (subfilePath);
            continue;
	}
    }
    clearKeyVal (&dataObjInp.condInput);
    closedir (dirPtr);
    return status;
}

int
regPhySubFile (rsComm_t *rsComm, char *subfilePath, 
dataObjInfo_t *bunDataObjInfo, rescInfo_t *rescInfo)
{
    dataObjInfo_t stageDataObjInfo;
    dataObjInp_t dataObjInp;
    struct stat statbuf;
    int status;
    regReplica_t regReplicaInp;

    bzero (&dataObjInp, sizeof (dataObjInp));
    bzero (&stageDataObjInfo, sizeof (stageDataObjInfo));
    rstrcpy (dataObjInp.objPath, bunDataObjInfo->objPath, MAX_NAME_LEN);
    rstrcpy (stageDataObjInfo.objPath, bunDataObjInfo->objPath, MAX_NAME_LEN);
    rstrcpy (stageDataObjInfo.rescName, rescInfo->rescName, NAME_LEN);
    stageDataObjInfo.rescInfo = rescInfo;

    status = getFilePathName (rsComm, &stageDataObjInfo, &dataObjInp);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "regPhySubFile: getFilePathName err for %s. status = %d",
          dataObjInp.objPath, status);
        return (status);
    }

    status = stat (stageDataObjInfo.filePath, &statbuf);
    if (status == 0 || errno != ENOENT) {
	status = resolveDupFilePath (rsComm, &stageDataObjInfo, &dataObjInp);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "regPhySubFile: resolveDupFilePath err for %s. status = %d",
              stageDataObjInfo.filePath, status);
            return (status);
	}
    }
    /* make the necessary dir */
    mkDirForFilePath (UNIX_FILE_TYPE, rsComm, "/", stageDataObjInfo.filePath,
      DEFAULT_DIR_MODE);
    /* add a link */
    status = link (subfilePath, stageDataObjInfo.filePath);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "regPhySubFile: link error %s to %s. errno = %d",
          subfilePath, stageDataObjInfo.filePath, errno);
        return (UNIX_FILE_LINK_ERR - errno);
    }

    bzero (&regReplicaInp, sizeof (regReplicaInp));
    regReplicaInp.srcDataObjInfo = bunDataObjInfo;
    regReplicaInp.destDataObjInfo = &stageDataObjInfo;
    addKeyVal (&regReplicaInp.condInput, SU_CLIENT_USER_KW, "");
    addKeyVal (&regReplicaInp.condInput, IRODS_ADMIN_KW, "");

    status = rsRegReplica (rsComm, &regReplicaInp);

    clearKeyVal (&regReplicaInp.condInput);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "regPhySubFile: rsRegReplica error for %s. status = %d",
          bunDataObjInfo->objPath, status);
        return status;
    }

    return status;
}

int
unbunPhyBunFile (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
rescInfo_t *rescInfo, char *bunFilePath, char *phyBunDir)
{
    int status;
    structFileOprInp_t structFileOprInp;

    /* untar the bunfile */
    memset (&structFileOprInp, 0, sizeof (structFileOprInp_t));
    structFileOprInp.specColl = malloc (sizeof (specColl_t));
    memset (structFileOprInp.specColl, 0, sizeof (specColl_t));
    structFileOprInp.specColl->type = TAR_STRUCT_FILE_T;
    snprintf (structFileOprInp.specColl->collection, MAX_NAME_LEN,
      "%s.dir", dataObjInp->objPath);
    rstrcpy (structFileOprInp.specColl->objPath,
      dataObjInp->objPath, MAX_NAME_LEN);
    structFileOprInp.specColl->collClass = STRUCT_FILE_COLL;
    rstrcpy (structFileOprInp.specColl->resource, rescInfo->rescName,
      NAME_LEN);
    rstrcpy (structFileOprInp.specColl->phyPath, bunFilePath, MAX_NAME_LEN);
    rstrcpy (structFileOprInp.addr.hostAddr, rescInfo->rescLoc,
      NAME_LEN);
    /* set the cacheDir */
    rstrcpy (structFileOprInp.specColl->cacheDir, phyBunDir, MAX_NAME_LEN);

    status = rsStructFileSync (rsComm, &structFileOprInp);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "unbunPhyBunFile: rsStructFileSync err for %s. status = %d",
          dataObjInp->objPath, status);
    }
    free (structFileOprInp.specColl);

    return (status);
}

int
remoteUnbunAndRegPhyBunfile (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteUnbunAndRegPhyBunfile: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcUnbunAndRegPhyBunfile (rodsServerHost->conn, dataObjInp);

    return status;
}

