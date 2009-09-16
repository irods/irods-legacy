/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsStructFileExtAndReg.c. See structFileExtAndReg.h for a description of 
 * this API call.*/

#include "structFileExtAndReg.h"
#include "apiHeaderAll.h"
#include "objMetaOpr.h"
#include "dataObjOpr.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"

int
rsStructFileExtAndReg (rsComm_t *rsComm,
structFileExtAndRegInp_t *structFileExtAndRegInp)
{
    int status;
    dataObjInp_t dataObjInp;
    dataObjInp_t dirRegInp;
    openedDataObjInp_t dataObjCloseInp;
    dataObjInfo_t *dataObjInfo;
    int l1descInx;
    structFileOprInp_t structFileOprInp;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;

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

    status = chkCollForExtAndReg (rsComm, structFileExtAndRegInp->collection);
    if (status < 0) return status;

    /* open the structured file */
    addKeyVal (&dataObjInp.condInput, NO_OPEN_FLAG_KW, "");
    l1descInx = _rsDataObjOpen (rsComm, &dataObjInp);

    if (l1descInx < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileExtAndReg: _rsDataObjOpen of %s error. status = %d",
          dataObjInp.objPath, l1descInx);
        return (l1descInx);
    }

    dataObjInfo = L1desc[l1descInx].dataObjInfo;
    status = initStructFileOprInp (rsComm, &structFileOprInp,
      structFileExtAndRegInp, dataObjInfo);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileExtAndReg: initStructFileOprInp of %s error. stat = %d",
          dataObjInp.objPath, status);
	bzero (&dataObjCloseInp, sizeof (dataObjCloseInp));
        dataObjCloseInp.l1descInx = l1descInx;
        rsDataObjClose (rsComm, &dataObjCloseInp);
        return (status);
    }

    status = rsStructFileExtract (rsComm, &structFileOprInp);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileExtAndReg: rsStructFileExtract of %s error. stat = %d",
          dataObjInp.objPath, status);
        dataObjCloseInp.l1descInx = l1descInx;
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
    addKeyVal (&dirRegInp.condInput, NO_CHK_FILE_PERM_KW, "");
    addKeyVal (&dirRegInp.condInput, DEST_RESC_NAME_KW, 
      dataObjInfo->rescName);

    status = rsPhyPathReg (rsComm, &dirRegInp);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileExtAndReg: rsPhyPathReg of %s to %s error. stat = %d",
          structFileOprInp.specColl->cacheDir, dirRegInp.objPath, status);
        rodsLog (LOG_ERROR,
          "rsStructFileExtAndReg: Orphan files may be left in %s",
          structFileOprInp.specColl->cacheDir);
        dataObjCloseInp.l1descInx = l1descInx;
        rsDataObjClose (rsComm, &dataObjCloseInp);
        return (status);
    }

    dataObjCloseInp.l1descInx = l1descInx;
    rsDataObjClose (rsComm, &dataObjCloseInp);

    return (status);
}

int 
chkCollForExtAndReg (rsComm_t *rsComm, char *collection)
{
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
    return (0);
}

