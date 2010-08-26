/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsBulkDataObjPut.c. See bulkDataObjReg.h for a description of
 * this API call.*/

#include "apiHeaderAll.h"
#include "objMetaOpr.h"
#include "resource.h"
#include "specColl.h"
#include "dataObjOpr.h"
#include "physPath.h"
#include "miscServerFunct.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"

int
rsBulkDataObjPut (rsComm_t *rsComm, bulkOprInp_t *bulkOprInp,
bytesBuf_t *bulkOprInpBBuf)
{
    int status;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    specCollCache_t *specCollCache = NULL;
    dataObjInp_t dataObjInp;

    resolveLinkedPath (rsComm, bulkOprInp->objPath, &specCollCache,
      &bulkOprInp->condInput);

    /* need to setup dataObjInp */
    initDataObjInpFromBulkOpr (&dataObjInp, bulkOprInp);

    remoteFlag = getAndConnRemoteZone (rsComm, &dataObjInp, &rodsServerHost,
      REMOTE_CREATE);

    if (remoteFlag < 0) {
        return (remoteFlag);
    } else if (remoteFlag == LOCAL_HOST) {
        status = _rsBulkDataObjPut (rsComm, bulkOprInp, bulkOprInpBBuf);
    } else {
        status = rcBulkDataObjPut (rodsServerHost->conn, bulkOprInp,
          bulkOprInpBBuf);
    }
    return status;
}

int
_rsBulkDataObjPut (rsComm_t *rsComm, bulkOprInp_t *bulkOprInp,
bytesBuf_t *bulkOprInpBBuf)
{
    int status;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    rescInfo_t *rescInfo;
    char *inpRescGrpName;
    char phyBunDir[MAX_NAME_LEN];
    rescGrpInfo_t *myRescGrpInfo = NULL;
    int flags = BULK_OPR_FLAG;
    dataObjInp_t dataObjInp;
    fileDriverType_t fileType;
    rodsObjStat_t *myRodsObjStat = NULL;

    inpRescGrpName = getValByKey (&bulkOprInp->condInput, RESC_GROUP_NAME_KW);

    status = chkCollForExtAndReg (rsComm, bulkOprInp->objPath, &myRodsObjStat);
    if (status < 0) return status;

    /* query rcat for resource info and sort it */

    /* need to setup dataObjInp */
    initDataObjInpFromBulkOpr (&dataObjInp, bulkOprInp);

    if (myRodsObjStat->specColl != NULL) {
        status = resolveResc (myRodsObjStat->specColl->resource, &rescInfo);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "_rsBulkDataObjPut: resolveResc error for %s, status = %d",
             myRodsObjStat->specColl->resource, status);
	    freeRodsObjStat (myRodsObjStat);
            return (status);
	}
    } else {
        status = getRescGrpForCreate (rsComm, &dataObjInp, &myRescGrpInfo);
        if (status < 0) {
	    freeRodsObjStat (myRodsObjStat);
	    return status;
	}

        /* just take the top one */
        rescInfo = myRescGrpInfo->rescInfo;
    }
    fileType = getRescType (rescInfo);
    if (fileType != UNIX_FILE_TYPE && fileType != NT_FILE_TYPE) {
	freeRodsObjStat (myRodsObjStat);
	return SYS_INVALID_RESC_FOR_BULK_OPR;
    }

    remoteFlag = resolveHostByRescInfo (rescInfo, &rodsServerHost);

    if (remoteFlag == REMOTE_HOST) {
        addKeyVal (&bulkOprInp->condInput, DEST_RESC_NAME_KW,
          rescInfo->rescName);
	if (myRodsObjStat->specColl == NULL && inpRescGrpName == NULL && 
	  strlen (myRescGrpInfo->rescGroupName) > 0) {
            addKeyVal (&bulkOprInp->condInput, RESC_GROUP_NAME_KW,
              myRescGrpInfo->rescGroupName);
	}
	freeRodsObjStat (myRodsObjStat);
        if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
            return status;
        }
        status = rcBulkDataObjPut (rodsServerHost->conn, bulkOprInp, 
          bulkOprInpBBuf);
        freeAllRescGrpInfo (myRescGrpInfo);
	return status;
    }

    status = createBunDirForBulkPut (rsComm, &dataObjInp, rescInfo, 
      myRodsObjStat->specColl, phyBunDir);

    if (status < 0) {
	freeRodsObjStat (myRodsObjStat);
	return status;
    }

#ifdef BULK_OPR_WITH_TAR
    status = untarBuf (phyBunDir, bulkOprInpBBuf);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "_rsBulkDataObjPut: untarBuf for dir %s. stat = %d",
          phyBunDir, status);
        return status;
    }
#else
    status = unbunBulkBuf (phyBunDir, bulkOprInp, bulkOprInpBBuf);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "_rsBulkDataObjPut: unbunBulkBuf for dir %s. stat = %d",
          phyBunDir, status);
        return status;
    }
#endif
    if (myRodsObjStat->specColl != NULL) {
	freeRodsObjStat (myRodsObjStat);
	return status;
    } else {
        freeRodsObjStat (myRodsObjStat);
    }

    if (strlen (myRescGrpInfo->rescGroupName) > 0) 
        inpRescGrpName = myRescGrpInfo->rescGroupName;

    if (getValByKey (&bulkOprInp->condInput, FORCE_FLAG_KW) != NULL) {
        flags = flags | FORCE_FLAG_FLAG;
    }

    if (getValByKey (&bulkOprInp->condInput, VERIFY_CHKSUM_KW) != NULL) {
        flags = flags | VERIFY_CHKSUM_FLAG;
    }

    status = regUnbunSubfiles (rsComm, rescInfo, inpRescGrpName,
      bulkOprInp->objPath, phyBunDir, flags, &bulkOprInp->attriArray);

    if (status == CAT_NO_ROWS_FOUND) {
        /* some subfiles have been deleted. harmless */
        status = 0;
    } else if (status < 0) {
        rodsLog (LOG_ERROR,
          "_rsBulkDataObjPut: regUnbunSubfiles for dir %s. stat = %d",
          phyBunDir, status);
    }
    freeAllRescGrpInfo (myRescGrpInfo);
    return status;
}

int
createBunDirForBulkPut (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
rescInfo_t *rescInfo, specColl_t *specColl, char *phyBunDir)
{
    dataObjInfo_t dataObjInfo;
    struct stat statbuf;
    int status;

    if (dataObjInp == NULL || rescInfo == NULL || phyBunDir == NULL) 
	return USER__NULL_INPUT_ERR;

    if (specColl != NULL) {
        status = getMountedSubPhyPath (specColl->collection,
          specColl->phyPath, dataObjInp->objPath, phyBunDir);
        if (status >= 0) {
            mkdirR ("/", phyBunDir, getDefDirMode ());
        }
	return status;
    }
    bzero (&dataObjInfo, sizeof (dataObjInfo));
    rstrcpy (dataObjInfo.objPath, dataObjInp->objPath, MAX_NAME_LEN);
    rstrcpy (dataObjInfo.rescName, rescInfo->rescName, NAME_LEN);
    dataObjInfo.rescInfo = rescInfo;

    status = getFilePathName (rsComm, &dataObjInfo, dataObjInp);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "createBunDirForBulkPut: getFilePathName err for %s. status = %d",
          dataObjInp->objPath, status);
        return (status);
    }
    do {
	snprintf (phyBunDir, MAX_NAME_LEN, "%s/%s.%d", dataObjInfo.filePath,
	  TMP_PHY_BUN_DIR, (int) random ());
	status =  stat (phyBunDir, &statbuf);
    } while (status == 0);

    mkdirR ("/", phyBunDir, getDefDirMode ());

    return 0;
}

int
initDataObjInpFromBulkOpr (dataObjInp_t *dataObjInp, bulkOprInp_t *bulkOprInp)
{
    if (dataObjInp == NULL || bulkOprInp == NULL)
	return USER__NULL_INPUT_ERR;

    bzero (dataObjInp, sizeof (dataObjInp_t));
    rstrcpy (dataObjInp->objPath, bulkOprInp->objPath, MAX_NAME_LEN);
    dataObjInp->condInput = bulkOprInp->condInput;

    return (0);
}
