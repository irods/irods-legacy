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
    dataObjCloseInp_t dataObjCloseInp;
    dataObjInfo_t *dataObjInfo;
    int l1descInx;
    structFileOprInp_t structFileOprInp;
    vaultPathPolicy_t vaultPathPolicy;
    int addUserNameFlag;

    status = chkCollForExtAndReg (rsComm, structFileExtAndRegInp->collection);

    /* open the structured file */
    memset (&dataObjInp, 0, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, structFileExtAndRegInp->objPath, 
      MAX_NAME_LEN);
 
    /* replicate the condInput. may have resource input */
    replKeyVal (&structFileExtAndRegInp->condInput, &dataObjInp.condInput);
    dataObjInp.openFlags = O_RDONLY;  
    l1descInx = _rsDataObjOpen (rsComm, &dataObjInp, DO_NOT_PHYOPEN);

    if (l1descInx < 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileExtAndReg: _rsDataObjOpen of %s error. status = %d",
          dataObjInp.objPath, l1descInx);
        return (l1descInx);
    }

    memset (&structFileOprInp, 0, sizeof (structFileOprInp));
    structFileOprInp.specColl = malloc (sizeof (specColl_t));
    memset (structFileOprInp.specColl, 0, sizeof (specColl_t));

    dataObjInfo = L1desc[l1descInx].dataObjInfo;
    if (strcmp (dataObjInfo->dataType, TAR_DT_STR) == 0) {
        structFileOprInp.specColl->type = TAR_STRUCT_FILE_T;
    } else if (strcmp (dataObjInfo->dataType, HAAW_DT_STR) == 0) {
        structFileOprInp.specColl->type = HAAW_STRUCT_FILE_T;
    } else {
        rodsLog (LOG_ERROR,
          "rsStructFileExtAndReg: objType %s of %s is not a struct file",
          dataObjInfo->dataType, dataObjInp.objPath);
	return SYS_OBJ_TYPE_NOT_STRUCT_FILE;
    }


    if (getStructFileType (dataObjInfo->specColl) >= 0) {
        rodsLog (LOG_ERROR,
          "rsStructFileExtAndReg: struct file %s is in a mounted strct file",
          dataObjInp.objPath);
	dataObjCloseInp.l1descInx = l1descInx;
        rsDataObjClose (rsComm, &dataObjCloseInp);
	return (SYS_STRUCT_FILE_INMOUNTED_COLL);
    }

    rstrcpy (structFileOprInp.specColl->collection,
      structFileExtAndRegInp->collection, MAX_NAME_LEN);
    structFileOprInp.specColl->collClass = STRUCT_FILE_COLL;
    rstrcpy (structFileOprInp.specColl->resource, dataObjInfo->rescName, 
      NAME_LEN);
    rstrcpy (structFileOprInp.specColl->phyPath,
      dataObjInfo->filePath, MAX_NAME_LEN);
    rstrcpy (structFileOprInp.addr.hostAddr, dataObjInfo->rescInfo->rescLoc,
      NAME_LEN);
    /* set the cacheDir */
    status = getVaultPathPolicy (rsComm, dataObjInfo, &vaultPathPolicy);
    if (status < 0) {
        dataObjCloseInp.l1descInx = l1descInx;
        rsDataObjClose (rsComm, &dataObjCloseInp);
        return (status);
    }
    /* don't do other type of Policy except GRAFT_PATH_S */
    if (vaultPathPolicy.scheme == GRAFT_PATH_S) {
	addUserNameFlag = vaultPathPolicy.addUserName;
    } else {
	addUserNameFlag = 1;
    }
    status = setPathForGraftPathScheme (structFileExtAndRegInp->collection,
      dataObjInfo->rescInfo->rescVaultPath, addUserNameFlag,
      rsComm->clientUser.userName, vaultPathPolicy.trimDirCnt,
      structFileOprInp.specColl->cacheDir);

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
    openCollInp_t openCollInp;
    collEnt_t *collEnt;
    int handleInx;
    collInp_t modCollInp;

    memset (&openCollInp, 0, sizeof (openCollInp));
    rstrcpy (openCollInp.collName, collection, MAX_NAME_LEN);

    handleInx = rsOpenCollection (rsComm, &openCollInp);
    if (handleInx < 0) {
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

