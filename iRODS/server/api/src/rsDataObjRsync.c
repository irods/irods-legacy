#include "reGlobalsExtern.h"
#include "dataObjRsync.h"
#include "objMetaOpr.h"
#include "dataObjOpr.h"
#include "rsApiHandler.h"
#include "modDataObjMeta.h"
#include "getRemoteZoneResc.h"

int
rsDataObjRsync (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
msParamArray_t **outParamArray)
{
    int status;
    char *rsyncMode;
    char *remoteZoneOpr;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;

    *outParamArray = NULL;
    if (dataObjInp == NULL) { 
       rodsLog(LOG_ERROR, "rsDataObjRsync error. NULL input");
       return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsyncMode = getValByKey (&dataObjInp->condInput, RSYNC_MODE_KW);
    if (rsyncMode == NULL) {
	rodsLog (LOG_ERROR,
	  "rsDataObjRsync: RSYNC_MODE_KW input is missing");
	return (USER_RSYNC_NO_MODE_INPUT_ERR);
    }

    if (strcmp (rsyncMode, LOCAL_TO_IRODS) == 0) {
	remoteZoneOpr = REMOTE_CREATE;
    } else {
	remoteZoneOpr = REMOTE_OPEN;
    }

    remoteFlag = getAndConnRemoteZone (rsComm, dataObjInp, &rodsServerHost,
      remoteZoneOpr);

    if (remoteFlag < 0) {
        return (remoteFlag);
    } else if (remoteFlag == REMOTE_HOST) {
	int l1descInx;

        status = _rcDataObjRsync (rodsServerHost->conn, dataObjInp,
          outParamArray);
	if (status < 0) {
            return (status);
        }
	if (status == SYS_SVR_TO_CLI_MSI_REQUEST) {
	    /* server request to client */
            l1descInx = allocAndSetL1descForZoneOpr (0, rodsServerHost);
            if (l1descInx < 0) return l1descInx;
	    if (*outParamArray == NULL) {
	        *outParamArray = malloc (sizeof (msParamArray_t));
	        bzero (*outParamArray, sizeof (msParamArray_t));
	    } 
	    addIntParamToArray (*outParamArray, CL_ZONE_OPR_INX, l1descInx);
	}
        return status;
    }

    if (strcmp (rsyncMode, IRODS_TO_LOCAL) == 0) {
	status = rsRsyncFileToData (rsComm, dataObjInp);
    } else if (strcmp (rsyncMode, LOCAL_TO_IRODS) == 0) { 
	status = rsRsyncDataToFile (rsComm, dataObjInp);
    } else if (strcmp (rsyncMode, IRODS_TO_IRODS) == 0) {
	status = rsRsyncDataToData (rsComm, dataObjInp);
    } else {
        rodsLog (LOG_ERROR, 
          "rsDataObjRsync: rsyncMode %s  not supported");
        return (USER_RSYNC_NO_MODE_INPUT_ERR);
    }
    
    return (status);
}

int
rsRsyncDataToFile (rsComm_t *rsComm, dataObjInp_t *dataObjInp)
{
    int status;
    char *fileChksumStr = NULL;
     char *dataObjChksumStr = NULL;
    dataObjInfo_t *dataObjInfoHead = NULL;
    msParamArray_t *myMsParamArray;
    dataObjInp_t *myDataObjInp;

    fileChksumStr = getValByKey (&dataObjInp->condInput, RSYNC_CHKSUM_KW);

    if (fileChksumStr == NULL) {
        rodsLog (LOG_ERROR,
          "rsRsyncDataToFile: RSYNC_CHKSUM_KW input is missing");
        return (CHKSUM_EMPTY_IN_STRUCT_ERR);
    }

    status = _rsDataObjChksum (rsComm, dataObjInp, &dataObjChksumStr,
      &dataObjInfoHead);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsRsyncDataToFile: RSYNC_CHKSUM_KW input is missing");
        return (CHKSUM_EMPTY_IN_STRUCT_ERR);
    }

    freeAllDataObjInfo (dataObjInfoHead);

    if (strcmp (dataObjChksumStr, fileChksumStr) == 0) {
	free (dataObjChksumStr);
	return (0);
    }

    myMsParamArray = malloc (sizeof (msParamArray_t));
    memset (myMsParamArray, 0, sizeof (msParamArray_t));
    /* have to get its own dataObjInp_t */
    myDataObjInp = malloc (sizeof (dataObjInp_t));
    replDataObjInp (dataObjInp, myDataObjInp);

    status = addMsParam (myMsParamArray, CL_GET_ACTION, DataObjInp_MS_T,
      (void *) myDataObjInp, NULL);

    if (status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, status,
          "rsRsyncDataToFile: addMsParam error. status = %d", status);
        return (status);
    }

    /* tell the client to do the put */
    status = sendAndRecvBranchMsg (rsComm, rsComm->apiInx,
     SYS_SVR_TO_CLI_MSI_REQUEST, (void *) myMsParamArray, NULL);

    return (status);
}

int
rsRsyncFileToData (rsComm_t *rsComm, dataObjInp_t *dataObjInp)
{
    int status;
    char *fileChksumStr = NULL;
     char *dataObjChksumStr = NULL;
    dataObjInfo_t *dataObjInfoHead = NULL;
    msParamArray_t *myMsParamArray;
    dataObjInp_t *myDataObjInp;

    fileChksumStr = getValByKey (&dataObjInp->condInput, RSYNC_CHKSUM_KW);

    if (fileChksumStr == NULL) {
        rodsLog (LOG_ERROR,
          "rsRsyncFileToData: RSYNC_CHKSUM_KW input is missing");
        return (CHKSUM_EMPTY_IN_STRUCT_ERR);
    }

    status = _rsDataObjChksum (rsComm, dataObjInp, &dataObjChksumStr,
      &dataObjInfoHead);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsRsyncDataToFile: RSYNC_CHKSUM_KW input is missing");
        return (CHKSUM_EMPTY_IN_STRUCT_ERR);
    }

    freeAllDataObjInfo (dataObjInfoHead);

    if (strcmp (dataObjChksumStr, fileChksumStr) == 0) {
	free (dataObjChksumStr);
	return (0);
    }

    myMsParamArray = malloc (sizeof (msParamArray_t));
    memset (myMsParamArray, 0, sizeof (msParamArray_t));
    /* have to get its own dataObjInp_t */
    myDataObjInp = malloc (sizeof (dataObjInp_t));
    replDataObjInp (dataObjInp, myDataObjInp);
    addKeyVal (&myDataObjInp->condInput, REG_CHKSUM_KW, fileChksumStr);

    status = addMsParam (myMsParamArray, CL_PUT_ACTION, DataObjInp_MS_T,
      (void *) myDataObjInp, NULL);

    if (status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, status,
          "rsRsyncDataToFile: addMsParam error. status = %d", status);
        return (status);
    }

    /* tell the client to do the put */
    status = sendAndRecvBranchMsg (rsComm, rsComm->apiInx,
     SYS_SVR_TO_CLI_MSI_REQUEST, (void *) myMsParamArray, NULL);

    return (status);
}

int
rsRsyncDataToData (rsComm_t *rsComm, dataObjInp_t *dataObjInp)
{
    int status;
    char *srcChksumStr = NULL;
     char *destChksumStr = NULL;
    dataObjInfo_t *srcDataObjInfoHead = NULL;
    dataObjInfo_t *destDataObjInfoHead = NULL;
    dataObjCopyInp_t dataObjCopyInp;
    char *destObjPath;
    transStat_t *transStat = NULL;

    destObjPath = getValByKey (&dataObjInp->condInput, RSYNC_DEST_PATH_KW);
    if (destObjPath == NULL) {
        rodsLog (LOG_ERROR,
          "rsRsyncDataToData: RSYNC_DEST_PATH_KW input is missing for %s",
	  dataObjInp->objPath);
        return (USER_RSYNC_NO_MODE_INPUT_ERR);
    }

    memset (&dataObjCopyInp, 0, sizeof (dataObjCopyInp));
    rstrcpy (dataObjCopyInp.srcDataObjInp.objPath, dataObjInp->objPath,
      MAX_NAME_LEN);
    dataObjCopyInp.srcDataObjInp.dataSize = dataObjInp->dataSize;
    dataObjCopyInp.destDataObjInp = *dataObjInp;
    /* move the cond */
    memset (&dataObjInp->condInput, 0, sizeof (keyValPair_t));
    rstrcpy (dataObjCopyInp.destDataObjInp.objPath, destObjPath,
      MAX_NAME_LEN);

    status = _rsDataObjChksum (rsComm, &dataObjCopyInp.srcDataObjInp, 
      &srcChksumStr, &srcDataObjInfoHead);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsRsyncDataToData: _rsDataObjChksum error for %s, status = %d",
	  dataObjCopyInp.srcDataObjInp.objPath, status);
        return (status);
    }

    status = _rsDataObjChksum (rsComm, &dataObjCopyInp.destDataObjInp, 
      &destChksumStr, &destDataObjInfoHead);


    if (status >= 0 && strcmp (srcChksumStr, destChksumStr) == 0) {
	free (srcChksumStr);
	free (destChksumStr);
	return (0);
    }

    freeAllDataObjInfo (srcDataObjInfoHead);
    freeAllDataObjInfo (destDataObjInfoHead);

    status = rsDataObjCopy (rsComm, &dataObjCopyInp, &transStat);
    if (transStat != NULL) {
	free (transStat);
    }

    return (status);
}

