/*** Copyright (c), The Unregents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* unregDataObj.c
 */

#include "regReplica.h"
#include "objMetaOpr.h"
#include "icatHighLevelRoutines.h"

int
rsRegReplica (rsComm_t *rsComm, regReplica_t *regReplicaInp)
{
    int status;
    rodsServerHost_t *rodsServerHost = NULL;
    dataObjInfo_t *srcDataObjInfo;
    dataObjInfo_t *destDataObjInfo;

    srcDataObjInfo = regReplicaInp->srcDataObjInfo;
    destDataObjInfo = regReplicaInp->destDataObjInfo;

    status = getAndConnRcatHost (rsComm, MASTER_RCAT, srcDataObjInfo->objPath,
      &rodsServerHost);
    if (status < 0) {
       return(status);
    }
    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
        status = _rsRegReplica (rsComm, regReplicaInp);
#else
        status = SYS_NO_RCAT_SERVER_ERR;
#endif
    } else {
        status = rcRegReplica (rodsServerHost->conn, regReplicaInp);
	if (status >= 0) regReplicaInp->destDataObjInfo->replNum = status;

    }

    return (status);
}

int
_rsRegReplica (rsComm_t *rsComm, regReplica_t *regReplicaInp)
{
#ifdef RODS_CAT
    int status;
    dataObjInfo_t *srcDataObjInfo;
    dataObjInfo_t *destDataObjInfo;
    int savedClientAuthFlag;

    srcDataObjInfo = regReplicaInp->srcDataObjInfo;
    destDataObjInfo = regReplicaInp->destDataObjInfo;

#if 0
    status = checkDupReplica (rsComm, srcDataObjInfo->dataId, 
      destDataObjInfo->rescName, destDataObjInfo->filePath);
    if (status >= 0) {
	destDataObjInfo->replNum = status;
	return status;
    }
#endif
    if (getValByKey (&regReplicaInp->condInput, SU_CLIENT_USER_KW) != NULL) {
	savedClientAuthFlag = rsComm->clientUser.authInfo.authFlag;
	rsComm->clientUser.authInfo.authFlag = LOCAL_PRIV_USER_AUTH;
        status = chlRegReplica (rsComm, srcDataObjInfo, destDataObjInfo,
          &regReplicaInp->condInput);
	/* restore it */
	rsComm->clientUser.authInfo.authFlag = savedClientAuthFlag;
    } else {
        status = chlRegReplica (rsComm, srcDataObjInfo, destDataObjInfo,
          &regReplicaInp->condInput);
	if (status >= 0) status = destDataObjInfo->replNum;
    }
    if (status == CAT_SUCCESS_BUT_WITH_NO_INFO || 
      status == CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME) {
	int status2;
	/* register a repl with a copy with the same resource and phyPaht.
         * could be caused by 2 staging at the same time */
        status2 = checkDupReplica (rsComm, srcDataObjInfo->dataId,
          destDataObjInfo->rescName, destDataObjInfo->filePath);
        if (status2 >= 0) {
            destDataObjInfo->replNum = status2;
	    destDataObjInfo->dataId = srcDataObjInfo->dataId;
            return status2;
        }
    }
    return (status);
#else
    return (SYS_NO_RCAT_SERVER_ERR);
#endif
}

