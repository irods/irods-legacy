/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsBulkDataObjReg.c. See bulkDataObjReg.h for a description of
 * this API call.*/

#include "apiHeaderAll.h"
#include "icatHighLevelRoutines.h"

int
rsBulkDataObjReg (rsComm_t *rsComm, genQueryOut_t *bulkDataObjRegInp)
{
    dataObjInfo_t dataObjInfo;
    modDataObjMeta_t modDataObjMetaInp;
    keyValPair_t regParam;
    sqlResult_t *objPath, *dataType, *dataSize, *rescName, *filePath, 
      *dataMode, *oprType;
    char *tmpObjPath, *tmpDataType, *tmpDataSize, *tmpRescName, *tmpFilePath,
      *tmpDataMode, *tmpOprType;
    int status, i;

    if ((objPath =
      getSqlResultByInx (bulkDataObjRegInp, COL_DATA_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "rsBulkDataObjReg: getSqlResultByInx for COL_DATA_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((dataType =
      getSqlResultByInx (bulkDataObjRegInp, COL_DATA_TYPE_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "rsBulkDataObjReg: getSqlResultByInx for COL_DATA_TYPE_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }
    if ((dataSize =
      getSqlResultByInx (bulkDataObjRegInp, COL_DATA_SIZE)) == NULL) {
        rodsLog (LOG_NOTICE,
          "rsBulkDataObjReg: getSqlResultByInx for COL_DATA_SIZE failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((rescName =
      getSqlResultByInx (bulkDataObjRegInp, COL_D_RESC_NAME)) == NULL) {
        rodsLog (LOG_NOTICE,
          "rsBulkDataObjReg: getSqlResultByInx for COL_D_RESC_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((filePath =
      getSqlResultByInx (bulkDataObjRegInp, COL_D_DATA_PATH)) == NULL) {
        rodsLog (LOG_NOTICE,
          "rsBulkDataObjReg: getSqlResultByInx for COL_D_DATA_PATH failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataMode =
      getSqlResultByInx (bulkDataObjRegInp, COL_DATA_MODE)) == NULL) {
        rodsLog (LOG_NOTICE,
          "rsBulkDataObjReg: getSqlResultByInx for COL_DATA_MODE failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((oprType =
      getSqlResultByInx (bulkDataObjRegInp, OPR_TYPE_INX)) == NULL) {
        rodsLog (LOG_ERROR,
          "rsBulkDataObjReg: getSqlResultByInx for OPR_TYPE_INX failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

   for (i = 0;i < bulkDataObjRegInp->rowCnt; i++) {
        tmpObjPath = &objPath->value[objPath->len * i];
        tmpDataType = &dataType->value[dataType->len * i];
        tmpDataSize = &dataSize->value[dataSize->len * i];
        tmpRescName = &rescName->value[rescName->len * i];
        tmpFilePath = &filePath->value[filePath->len * i];
        tmpDataMode = &dataMode->value[dataMode->len * i];
        tmpOprType = &oprType->value[oprType->len * i];
        bzero (&dataObjInfo, sizeof (dataObjInfo_t));
	dataObjInfo.flags = NO_COMMIT_FLAG;
        rstrcpy (dataObjInfo.objPath, tmpObjPath, MAX_NAME_LEN);
        rstrcpy (dataObjInfo.dataType, tmpDataType, NAME_LEN);
	dataObjInfo.dataSize = strtoll (tmpDataSize, 0, 0);
        rstrcpy (dataObjInfo.rescName, tmpRescName, NAME_LEN);
        rstrcpy (dataObjInfo.filePath, tmpFilePath, MAX_NAME_LEN);
        rstrcpy (dataObjInfo.dataMode, tmpDataMode, NAME_LEN);
	if (strcmp (tmpOprType, REGISTER_OPR) == 0) {
	    status = svrRegDataObj (rsComm, &dataObjInfo);
	} else {
            bzero (&modDataObjMetaInp, sizeof (modDataObjMetaInp));
            bzero (&regParam, sizeof (regParam));
            addKeyVal (&regParam, DATA_SIZE_KW, tmpDataSize);
            addKeyVal (&regParam, ALL_REPL_STATUS_KW, "");
            snprintf (tmpStr, MAX_NAME_LEN, "%d", (int) time (NULL));
            addKeyVal (&regParam, DATA_MODIFY_KW, tmpStr);

            modDataObjMetaInp.dataObjInfo = &dataObjInfo;
            modDataObjMetaInp.regParam = &regParam;

            status = rsModDataObjMeta (rsComm, &modDataObjMetaInp);

            clearKeyVal (&regParam);
        }
	if (status < 0) {
	    rodsLog (LOG_ERROR,
	     "rsBulkDataObjReg: RegDataObj or ModDataObj failed for %s,stat=%d",
              tmpObjPath, status);
	    chlRollback (rsComm);
            return status;
	}
    }
    status = chlCommit(rsComm);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "rsBulkDataObjReg: chlCommit failed, status = %d", status);
    }
    return status;
}
