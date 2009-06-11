/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsStructFileBundle.c. See structFileBundle.h for a description of 
 * this API call.*/

#include "structFileBundle.h"
#include "apiHeaderAll.h"
#include "objMetaOpr.h"
#include "dataObjOpr.h"
#include "miscServerFunct.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"

int
rsStructFileBundle (rsComm_t *rsComm,
structFileExtAndRegInp_t *structFileBundleInp)
{
    char *destRescName;
    int status;
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    rodsHostAddr_t rescAddr;
    rescGrpInfo_t *rescGrpInfo = NULL;

    if ((destRescName = 
     getValByKey (&structFileBundleInp->condInput, DEST_RESC_NAME_KW)) == NULL 
     && (destRescName = 
     getValByKey (&structFileBundleInp->condInput, DEF_RESC_NAME_KW)) == NULL) {
        return USER_NO_RESC_INPUT_ERR;
    }

    if (isLocalZone (structFileBundleInp->collection) == 0) {
        /* can only do local zone */
        return SYS_INVALID_ZONE_NAME;
    }
    status = _getRescInfo (rsComm, destRescName, &rescGrpInfo);
    if (status < 0) {
         rodsLog (LOG_ERROR,
          "rsStructFileBundle: _getRescInfo of %s error for %s. stat = %d",
          destRescName, structFileBundleInp->collection, status);
        return status;
    }

    bzero (&rescAddr, sizeof (rescAddr));
    rstrcpy (rescAddr.hostAddr, rescGrpInfo->rescInfo->rescLoc, NAME_LEN);
    remoteFlag = resolveHost (&rescAddr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsStructFileBundle (rsComm, structFileBundleInp);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteStructFileBundle (rsComm, structFileBundleInp, 
	  rodsServerHost);
    } else if (remoteFlag < 0) {
            status = remoteFlag;
    }

    return status;
}

int
_rsStructFileBundle (rsComm_t *rsComm,
structFileExtAndRegInp_t *structFileBundleInp)
{
    int status;
    dataObjInp_t dataObjInp;
    openedDataObjInp_t dataObjCloseInp;
#if 0
    structFileOprInp_t structFileOprInp;
#else
    collInp_t collInp;
    collEnt_t *collEnt;
    int handleInx;
    int collLen;
    char phyBunDir[MAX_NAME_LEN];
    char tmpPath[MAX_NAME_LEN];
#endif
    chkObjPermAndStat_t chkObjPermAndStatInp;
    int l1descInx;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    int savedStatus = 0;

    /* open the structured file */
    memset (&dataObjInp, 0, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, structFileBundleInp->objPath, 
      MAX_NAME_LEN);
 
    /* replicate the condInput. may have resource input */
    replKeyVal (&structFileBundleInp->condInput, &dataObjInp.condInput);

    dataObjInp.openFlags = O_WRONLY;  
    remoteFlag = getAndConnRemoteZone (rsComm, &dataObjInp, &rodsServerHost,
      REMOTE_CREATE);

    if (remoteFlag < 0) {
        return (remoteFlag);
    } else if (remoteFlag == REMOTE_HOST) {
        status = rcStructFileBundle (rodsServerHost->conn, 
	  structFileBundleInp);
        return status;
    }

    l1descInx = rsDataObjCreate (rsComm, &dataObjInp);
    if (l1descInx < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileBundle: rsDataObjCreate of %s error. status = %d",
          dataObjInp.objPath, l1descInx);
        return (l1descInx);
    }
    l3Close (rsComm, l1descInx);
    L1desc[l1descInx].l3descInx = 0;

#if 0
    status = initStructFileOprInp (rsComm, &structFileOprInp, 
      structFileBundleInp, L1desc[l1descInx].dataObjInfo);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileBundle: initStructFileOprInp of %s error. stat = %d",
          dataObjInp.objPath, status);
	bzero (&dataObjCloseInp, sizeof (dataObjCloseInp));
        dataObjCloseInp.l1descInx = l1descInx;
        rsDataObjClose (rsComm, &dataObjCloseInp);
        return (status);
    }
#endif

    memset (&chkObjPermAndStatInp, 0, sizeof (chkObjPermAndStatInp));
    rstrcpy (chkObjPermAndStatInp.objPath, 
      structFileBundleInp->collection, MAX_NAME_LEN); 
    chkObjPermAndStatInp.flags = CHK_COLL_FOR_BUNDLE_OPR;
    addKeyVal (&chkObjPermAndStatInp.condInput, RESC_NAME_KW,
      L1desc[l1descInx].dataObjInfo->rescName);

   status = rsChkObjPermAndStat (rsComm, &chkObjPermAndStatInp);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileBundle: rsChkObjPermAndStat of %s error. stat = %d",
          chkObjPermAndStatInp.objPath, status);
        dataObjCloseInp.l1descInx = l1descInx;
        rsDataObjClose (rsComm, &dataObjCloseInp);
        return (status);
    }

#if 0
    structFileOprInp.oprType = NO_REG_COLL_INFO | LOGICAL_BUNDLE;
    structFileOprInp.specColl->cacheDirty = 1;

    status = rsStructFileSync (rsComm, &structFileOprInp);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileBundle: rsStructFileSync of %s error. stat = %d",
          dataObjInp.objPath, status);
    } else {
	/* mark it was written so the size would be adjusted */
	L1desc[l1descInx].bytesWritten = 1;
    }
#endif

    createPhyBundleDir (rsComm, L1desc[l1descInx].dataObjInfo->filePath,
      phyBunDir);

    bzero (&collInp, sizeof (collInp));
    rstrcpy (collInp.collName, structFileBundleInp->collection, MAX_NAME_LEN);
    collInp.flags = RECUR_QUERY_FG | VERY_LONG_METADATA_FG |
      NO_TRIM_REPL_FG;
    handleInx = rsOpenCollection (rsComm, &collInp);
    if (handleInx < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileBundle: rsOpenCollection of %s error. status = %d",
          collInp.collName, handleInx);
        return (handleInx);
    }

    collLen = strlen (collInp.collName) + 1;

    while ((status = rsReadCollection (rsComm, &handleInx, &collEnt)) >= 0) {
        if (collEnt->objType == DATA_OBJ_T) {
            if (collEnt->collName[collLen] == '\0') {
                snprintf (tmpPath, MAX_NAME_LEN, "%s/%s",
                  phyBunDir, collEnt->dataName);
            } else {
                snprintf (tmpPath, MAX_NAME_LEN, "%s/%s/%s",
                  phyBunDir, collEnt->collName + collLen, collEnt->dataName);
	        mkDirForFilePath (UNIX_FILE_TYPE, rsComm, phyBunDir, 
	          tmpPath, DEFAULT_DIR_MODE);
            }
            /* add a link */
            status = link (collEnt->phyPath, tmpPath);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "rsStructFileBundle: link error %s to %s. errno = %d",
                  collEnt->phyPath, tmpPath, errno);
                return (UNIX_FILE_LINK_ERR - errno);
            }
        } else {        /* a collection */
            snprintf (tmpPath, MAX_NAME_LEN, "%s/%s",
              phyBunDir, collEnt->collName + collLen);
            mkdirR (phyBunDir, tmpPath, DEFAULT_DIR_MODE);
	}
    }
    status = phyBundle (rsComm, L1desc[l1descInx].dataObjInfo, phyBunDir,
      collInp.collName);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileBundle: phyBundle of %s error. stat = %d",
          L1desc[l1descInx].dataObjInfo->objPath, status);
	L1desc[l1descInx].bytesWritten = 0;
	savedStatus = status;
    } else {
        /* mark it was written so the size would be adjusted */
        L1desc[l1descInx].bytesWritten = 1;
    }

    /* XXXXX need to recursive rm. But this could be dangerous */
    rmLinkedFilesInUnixDir (phyBunDir);
    rmdir (phyBunDir);

    dataObjCloseInp.l1descInx = l1descInx;
    status = rsDataObjClose (rsComm, &dataObjCloseInp);

    if (status >= 0)
	return savedStatus;
    else
        return (status);
}

int
remoteStructFileBundle (rsComm_t *rsComm,
structFileExtAndRegInp_t *structFileBundleInp, rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteStructFileBundle: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcStructFileBundle (rodsServerHost->conn, structFileBundleInp);
    return status;
}

