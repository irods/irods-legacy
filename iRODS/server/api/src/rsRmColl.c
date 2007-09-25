/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* This is script-generated code (for the most part).  */
/* See collCreate.h for a description of this API call.*/

#include "rmColl.h"
#include "collCreate.h"
#include "icatHighLevelRoutines.h"
#include "rodsLog.h"
#include "icatDefines.h"
#include "dataObjRename.h"
#include "dataObjOpr.h"
#include "objMetaOpr.h"
#include "fileRmdir.h"
#include "bunSubRmdir.h"
#include "genQuery.h"
#include "dataObjUnlink.h"
#include "rmColl.h"

int
rsRmColl (rsComm_t *rsComm, collInp_t *rmCollInp)
{
    int status;
    rodsServerHost_t *rodsServerHost = NULL;


    status = getAndConnRcatHost (rsComm, MASTER_RCAT, rmCollInp->collName,
                                &rodsServerHost);
    if (status < 0) {
       return(status);
    }

    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
        status = _rsRmColl (rsComm, rmCollInp);
#else
        status = SYS_NO_RCAT_SERVER_ERR;
#endif
    } else {
	status = rcRmColl (rodsServerHost->conn, rmCollInp);
    }

    return (status);
}

int
_rsRmColl (rsComm_t *rsComm, collInp_t *rmCollInp)
{
    int status;
    dataObjInp_t dataObjInp;
    dataObjInfo_t *dataObjInfo = NULL;

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, rmCollInp->collName, MAX_NAME_LEN);
    status = resolveSpecColl (rsComm, &dataObjInp, &dataObjInfo, 1);

    if (status == COLL_OBJ_T && dataObjInfo->specColl != NULL) {
	status = svrRmSpecColl (rsComm, rmCollInp, dataObjInfo);
    } else { 
	status = svrRmColl (rsComm, rmCollInp);
    }

    return (0);
}

int
svrRmColl (rsComm_t *rsComm, collInp_t *rmCollInp)
{
    int status;
    collInfo_t collInfo;

    if (getValByKey (&rmCollInp->condInput, RECURSIVE_OPR__KW) != NULL) {
	status = _rsRmCollRecur (rsComm, rmCollInp);
	return (status);
    }

    memset (&collInfo, 0, sizeof (collInfo));

    rstrcpy (collInfo.collName, rmCollInp->collName, MAX_NAME_LEN);
#ifdef RODS_CAT
    status = chlDelColl (rsComm, &collInfo);
    return (status);
#else
    return (SYS_NO_RCAT_SERVER_ERR);
#endif
}

int
_rsRmCollRecur (rsComm_t *rsComm, collInp_t *rmCollInp)
{
    int status;
    ruleExecInfo_t rei;
    int trashPolicy;

    if (getValByKey (&rmCollInp->condInput, FORCE_FLAG_KW) != NULL) { 
        status = rsPhyRmCollRecur (rsComm, rmCollInp);
    } else {
        initReiWithDataObjInp (&rei, rsComm, NULL);
        status = applyRule ("acTrashPolicy", NULL, &rei, NO_SAVE_REI);
        trashPolicy = rei.status;

        if (trashPolicy != NO_TRASH_CAN) { 
            status = rsMvCollToTrash (rsComm, rmCollInp);
	    return status;
        } else {
	    status = rsPhyRmCollRecur (rsComm, rmCollInp);
	}
    }

    return (status);
}

int
rsPhyRmCollRecur (rsComm_t *rsComm, collInp_t *rmCollInp)
{
    int status, i;
    int savedStatus = 0;
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    dataObjInp_t dataObjInp;
    int collLen;
    int continueInx;
    collInfo_t collInfo;
    collInp_t tmpCollInp;
    int rmtrashFlag;

    if (getValByKey (&rmCollInp->condInput, IRODS_ADMIN_RMTRASH_KW) != NULL) {
        if (rsComm->clientUser.authInfo.authFlag != LOCAL_PRIV_USER_AUTH) {
           return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
        }
	rmtrashFlag = 2;
    } else if (getValByKey (&rmCollInp->condInput, IRODS_RMTRASH_KW) != NULL) {
        rmtrashFlag = 1;
    }

    collLen = strlen (rmCollInp->collName);

    /* Now get all the files */

    memset (&genQueryInp, 0, sizeof (genQueryInp));
    status = rsQueryDataObjInCollReCur (rsComm, rmCollInp->collName, 
      &genQueryInp, &genQueryOut, NULL, 1);

    if (status < 0 && status != CAT_NO_ROWS_FOUND) {
	rodsLog (LOG_ERROR,
	  "rsPhyRmCollRecur: rsQueryDataObjInCollReCur error for %s, stat=%d",
	  rmCollInp->collName, status);
	return (status);
    }

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    while (status >= 0) {
        sqlResult_t *subColl, *dataObj;

        if ((subColl = getSqlResultByInx (genQueryOut, COL_COLL_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "rsPhyRmCollRecur: getSqlResultByInx for COL_COLL_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        if ((dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "rsPhyRmCollRecur: getSqlResultByInx for COL_DATA_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

	addKeyVal (&rmCollInp->condInput, FORCE_FLAG_KW, "");
	addKeyVal (&dataObjInp.condInput, FORCE_FLAG_KW, "");
	if (rmtrashFlag == 2) {
	    addKeyVal (&dataObjInp.condInput, IRODS_ADMIN_RMTRASH_KW, "");
	}

        for (i = 0; i < genQueryOut->rowCnt; i++) {
            char *tmpSubColl, *tmpDataName;

            tmpSubColl = &subColl->value[subColl->len * i];
            tmpDataName = &dataObj->value[dataObj->len * i];

            snprintf (dataObjInp.objPath, MAX_NAME_LEN, "%s/%s",
              tmpSubColl, tmpDataName);
            status = rsDataObjUnlink (rsComm, &dataObjInp);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "rsPhyRmCollRecur:rsDataObjUnlink failed for %s. stat = %d", 
		  dataObjInp.objPath, status);
                /* need to set global error here */
                savedStatus = status;
            }
        }

        continueInx = genQueryOut->continueInx;

        freeGenQueryOut (&genQueryOut);

        if (continueInx > 0) {
            /* More to come */
            genQueryInp.continueInx = continueInx;
            status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
        } else {
            break;
        }
    }

    clearKeyVal (&dataObjInp.condInput);

    /* query all sub collections in rmCollInp->collName and the mk the required
     * subdirectories */

    status = rsQueryCollInColl (rsComm, rmCollInp->collName, &genQueryInp,
      &genQueryOut);

    memset (&tmpCollInp, 0, sizeof (tmpCollInp));

    while (status >= 0) {
        sqlResult_t *subColl;

        if ((subColl = getSqlResultByInx (genQueryOut, COL_COLL_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "rsPhyRmCollRecur: getSqlResultByInx for COL_COLL_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        if (rmtrashFlag == 1) {
            addKeyVal (&tmpCollInp.condInput, IRODS_RMTRASH_KW, "");
        } else if (rmtrashFlag == 2) {
            addKeyVal (&tmpCollInp.condInput, IRODS_ADMIN_RMTRASH_KW, "");
        }

        for (i = 0; i < genQueryOut->rowCnt; i++) {
            char *tmpSubColl;

            tmpSubColl = &subColl->value[subColl->len * i];
            if (strlen (tmpSubColl) < collLen)
                continue;
            /* recursively rm the collection */
	    rstrcpy (tmpCollInp.collName, tmpSubColl, MAX_NAME_LEN);
            status = rsPhyRmCollRecur (rsComm, &tmpCollInp);
            if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                  "rsPhyRmCollRecur: rsPhyRmCollRecur of %s failed, status = %d",
                  tmpSubColl, status);
                savedStatus = status;
            }
        }
        continueInx = genQueryOut->continueInx;

        freeGenQueryOut (&genQueryOut);

        if (continueInx > 0) {
            /* More to come */
            genQueryInp.continueInx = continueInx;
            status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
        } else {
            break;
        }
    }
    clearKeyVal (&tmpCollInp.condInput);


    memset (&collInfo, 0, sizeof (collInfo));

    rstrcpy (collInfo.collName, rmCollInp->collName, MAX_NAME_LEN);
#ifdef RODS_CAT
    if (rmtrashFlag > 0 && isTrashHome (rmCollInp->collName) > 0) {
	/* don't rm user's home trash coll */
	status = 0;
    } else {
        memset (&collInfo, 0, sizeof (collInfo));
        rstrcpy (collInfo.collName, rmCollInp->collName, MAX_NAME_LEN);
	if (rmtrashFlag == 2) {
	    status = chlDelCollByAdmin (rsComm, &collInfo);
	    if (status >= 0) {
	        chlCommit(rsComm);
	    }
	} else {
            status = chlDelColl (rsComm, &collInfo);
	}
    }

    if (status < 0) {
       rodsLog (LOG_ERROR,
        "rsPhyRmCollRecur: chlDelColl of %s failed, status = %d",
        rmCollInp->collName, status);
    }

    clearGenQueryInp (&genQueryInp);

    if (savedStatus < 0) {
        return (savedStatus);
    } else if (status == CAT_NO_ROWS_FOUND) {
        return (0);
    } else {
        return (status);
    }
#else
    return (SYS_NO_RCAT_SERVER_ERR);
#endif
}

int
rsMvCollToTrash (rsComm_t *rsComm, collInp_t *rmCollInp)
{
    int status;
    char trashPath[MAX_NAME_LEN];
    dataObjCopyInp_t dataObjRenameInp;
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    int continueInx;
    dataObjInfo_t dataObjInfo;

    /* check permission of files */

    memset (&genQueryInp, 0, sizeof (genQueryInp));
    status = rsQueryDataObjInCollReCur (rsComm, rmCollInp->collName,
      &genQueryInp, &genQueryOut, ACCESS_DELETE_OBJECT, 0);

    memset (&dataObjInfo, 0, sizeof (dataObjInfo));
    while (status >= 0) {
	sqlResult_t *subColl, *dataObj, *rescName;
	ruleExecInfo_t rei;

	/* check if allow to delete */

        if ((subColl = getSqlResultByInx (genQueryOut, COL_COLL_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "rsMvCollToTrash: getSqlResultByInx for COL_COLL_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        if ((dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "rsMvCollToTrash: getSqlResultByInx for COL_DATA_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        if ((rescName = getSqlResultByInx (genQueryOut, COL_D_RESC_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "rsMvCollToTrash: getSqlResultByInx for COL_D_RESC_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

	snprintf (dataObjInfo.objPath, MAX_NAME_LEN, "%s/%s", 
	  subColl->value, dataObj->value);
	rstrcpy (dataObjInfo.rescName, rescName->value, NAME_LEN);

        initReiWithDataObjInp (&rei, rsComm, NULL);
        rei.doi = &dataObjInfo;

        status = applyRule ("acDataDeletePolicy", NULL, &rei, NO_SAVE_REI);

        if (status < 0 && status != NO_MORE_RULES_ERR &&
          status != SYS_DELETE_DISALLOWED) {
            rodsLog (LOG_NOTICE,
              "rsMvCollToTrash: acDataDeletePolicy error for %s. status = %d",
              dataObjInfo.objPath, status);
            return (status);
        }

        if (rei.status == SYS_DELETE_DISALLOWED) {
            rodsLog (LOG_NOTICE,
            "rsMvCollToTrash:disallowed for %s via DataDeletePolicy,status=%d",
              dataObjInfo.objPath, rei.status);
            return (rei.status);
        }

        continueInx = genQueryOut->continueInx;

        freeGenQueryOut (&genQueryOut);

        if (continueInx > 0) {
            /* More to come */
            genQueryInp.continueInx = continueInx;
            status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
        } else {
            break;
        }
    }

    if (status < 0 && status != CAT_NO_ROWS_FOUND) {
        rodsLog (LOG_ERROR,
          "rsMvCollToTrash: rsQueryDataObjInCollReCur error for %s, stat=%d",
          rmCollInp->collName, status);
        return (status);
    }

    status = rsMkTrashPath (rsComm, rmCollInp->collName, trashPath);

    if (status < 0) {
        appendRandomToPath (trashPath);
        status = rsMkTrashPath (rsComm, rmCollInp->collName, trashPath);
        if (status < 0) {
            return (status);
        }
    }

    memset (&dataObjRenameInp, 0, sizeof (dataObjRenameInp));

    dataObjRenameInp.srcDataObjInp.oprType =
      dataObjRenameInp.destDataObjInp.oprType = RENAME_COLL;

    rstrcpy (dataObjRenameInp.destDataObjInp.objPath, trashPath, MAX_NAME_LEN);
    rstrcpy (dataObjRenameInp.srcDataObjInp.objPath, rmCollInp->collName,
      MAX_NAME_LEN);

    status = rsDataObjRename (rsComm, &dataObjRenameInp);

    if (status < 0) {
        appendRandomToPath (dataObjRenameInp.destDataObjInp.objPath);
        status = rsDataObjRename (rsComm, &dataObjRenameInp);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "mvCollToTrash: rcDataObjRename error for %s",
              dataObjRenameInp.destDataObjInp.objPath);
            return (status);
	}
    }

    return (status);
}

int
rsMkTrashPath (rsComm_t *rsComm, char *objPath, char *trashPath)
{
    int status;
    char *tmpStr, *savedTmpStr;
    char startTrashPath[MAX_NAME_LEN];
    char destTrashColl[MAX_NAME_LEN], myFile[MAX_NAME_LEN];
    char *trashPathPtr;

    trashPathPtr = trashPath;
    *trashPathPtr = '/';
    trashPathPtr++;
    tmpStr = objPath + 1;
    /* copy the zone */
    while (*tmpStr != '\0') {
	*trashPathPtr = *tmpStr;
	trashPathPtr ++;
	if (*tmpStr == '/') {
	    tmpStr ++;
	    break;
	}
	tmpStr ++;
    }

    if (*tmpStr == '\0') {
        rodsLog (LOG_ERROR,
          "rsMkTrashPath: input path %s too short", objPath);
	return (USER_INPUT_PATH_ERR);
    }
     
    /* skip "home/userName/ */

    if (strncmp (tmpStr, "home/", 5) == 0) {
        int nameLen; 
        tmpStr += 5;
        nameLen = strlen (rsComm->clientUser.userName);
        if (strncmp (tmpStr, rsComm->clientUser.userName, nameLen) == 0 &&
	  *(tmpStr + nameLen) == '/') { 
            tmpStr += (nameLen + 1);
        }
    }


    /* don't want to go back beyond /myZone/trash/home */
    *trashPathPtr = '\0';
    snprintf (startTrashPath, MAX_NAME_LEN, "%strash/home", trashPath);

    /* add home/userName/ */

    snprintf (trashPathPtr, MAX_NAME_LEN, "trash/home/%s/%s",  
      rsComm->clientUser.userName, tmpStr); 

    if ((status = splitPathByKey (trashPath, destTrashColl, myFile, '/')) < 0) {
        rodsLog (LOG_ERROR,
          "rsMkTrashPath: splitPathByKey error for %s ", trashPath);
        return (USER_INPUT_PATH_ERR);
    }

    status = rsMkCollR (rsComm, startTrashPath, destTrashColl);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsMkTrashPath: rsMkCollR error for startPath %s, destPath %s ",
          startTrashPath, destTrashColl);
    }

    return (status);
}

int
svrRmSpecColl (rsComm_t *rsComm, collInp_t *rmCollInp, 
dataObjInfo_t *dataObjInfo)
{
    int status;

    if (getValByKey (&rmCollInp->condInput, RECURSIVE_OPR__KW) != NULL) {
	/* XXXX need to take care of bundle */
        status = svrRmSpecCollRecur (rsComm, dataObjInfo);
    } else {
	/* XXXX need to take care of bundle */
	status = l3Rmdir (rsComm, dataObjInfo);
    }
#if 0
    rstrcpy (collInfo.collName, rmCollInp->collName, MAX_NAME_LEN);
#endif
    return (status);
}

int
l3Rmdir (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo)
{
    int rescTypeInx;
    fileRmdirInp_t fileRmdirInp;
    int status;

    if (getBunType (dataObjInfo->specColl) >= 0) {
        subFile_t subFile;
        memset (&subFile, 0, sizeof (subFile));
        rstrcpy (subFile.subFilePath, dataObjInfo->subPath,
          MAX_NAME_LEN);
        rstrcpy (subFile.addr.hostAddr, dataObjInfo->rescInfo->rescLoc,
          NAME_LEN);
        subFile.specColl = dataObjInfo->specColl;
        status = rsBunSubRmdir (rsComm, &subFile);
    } else {
        rescTypeInx = dataObjInfo->rescInfo->rescTypeInx;

        switch (RescTypeDef[rescTypeInx].rescCat) {
          case FILE_CAT:
            memset (&fileRmdirInp, 0, sizeof (fileRmdirInp));
            fileRmdirInp.fileType = RescTypeDef[rescTypeInx].driverType;
            rstrcpy (fileRmdirInp.dirName, dataObjInfo->filePath,
              MAX_NAME_LEN);
            rstrcpy (fileRmdirInp.addr.hostAddr,
              dataObjInfo->rescInfo->rescLoc, NAME_LEN);
            status = rsFileRmdir (rsComm, &fileRmdirInp);
            break;

          default:
            rodsLog (LOG_NOTICE,
              "l3Rmdir: rescCat type %d is not recognized",
              RescTypeDef[rescTypeInx].rescCat);
            status = SYS_INVALID_RESC_TYPE;
            break;
        }
    }
    return (status);
}

int
svrRmSpecCollRecur (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo)
{
    int status, i;
    dataObjInfo_t myDataObjInfo;
    genQueryOut_t *genQueryOut = NULL;
    dataObjInp_t dataObjInp;
    int continueInx;
    int savedStatus = 0;

    myDataObjInfo = *dataObjInfo;
    memset (&dataObjInp, 0, sizeof (dataObjInp));
#if 0
    addKeyVal (&dataObjInp.condInput, SEL_OBJ_TYPE_KW, "dataObj");
    addKeyVal (&dataObjInp.condInput, RECURSIVE_OPR__KW, "");
#endif
    rstrcpy (dataObjInp.objPath, dataObjInfo->objPath, MAX_NAME_LEN);
    status = rsQuerySpecColl (rsComm, &dataObjInp, &genQueryOut);

    while (status >= 0) {
        sqlResult_t *subColl, *dataObj;

        if ((subColl = getSqlResultByInx (genQueryOut, COL_COLL_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "svrRmSpecCollRecur:getSqlResultByInx COL_COLL_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        if ((dataObj = getSqlResultByInx (genQueryOut, COL_DATA_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "svrRmSpecCollRecur: getSqlResultByInx COL_DATA_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        for (i = 0; i < genQueryOut->rowCnt; i++) {
            char *tmpSubColl, *tmpDataName;

            tmpSubColl = &subColl->value[subColl->len * i];
            tmpDataName = &dataObj->value[dataObj->len * i];

            snprintf (myDataObjInfo.objPath, MAX_NAME_LEN, "%s/%s",
              tmpSubColl, tmpDataName);

            status = getMountedSubPhyPath (dataObjInfo->objPath,
              dataObjInfo->filePath, myDataObjInfo.objPath,
              myDataObjInfo.filePath);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "svrRmSpecCollRecur:getMountedSubPhyPat err for %s,stat=%d",
                  myDataObjInfo.filePath, status);
		savedStatus = status;
            }

	    if (strlen (tmpDataName) > 0) {
	        status = l3Unlink (rsComm, &myDataObjInfo);
	    } else {
#if 0
		status = l3Rmdir (rsComm, &myDataObjInfo);
#endif
		status = svrRmSpecCollRecur (rsComm, &myDataObjInfo);
	    }

            if (status < 0) {
                rodsLog (LOG_NOTICE,
                  "svrRmSpecCollRecur: l3Unlink error for %s. status = %d",
                  myDataObjInfo.objPath, status);
		savedStatus = status;
	    }
        }
        continueInx = genQueryOut->continueInx;

        freeGenQueryOut (&genQueryOut);

        if (continueInx > 0) {
            /* More to come */
            dataObjInp.openFlags = continueInx;
            status = rsQuerySpecColl (rsComm, &dataObjInp, &genQueryOut);
        } else {
            break;
        }
    }

    status = l3Rmdir (rsComm, dataObjInfo);
    if (status < 0) savedStatus = status;

    return (savedStatus);
}

