/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* See dataObjPhymv.h for a description of this API call.*/

#include "dataObjPhymv.h"
#include "dataObjRepl.h"
#include "dataObjOpr.h"
#include "rodsLog.h"
#include "objMetaOpr.h"
#include "reGlobalsExtern.h"
#include "reDefines.h"
#include "reSysDataObjOpr.h"
#include "dataObjCreate.h"
#include "getRemoteZoneResc.h"

/* rsDataObjPhymv - The Api handler of the rcDataObjPhymv call - phymove
 * a data object from one resource to another.
 * Input -
 *    rsComm_t *rsComm
 *    dataObjInp_t *dataObjInp - The replication input
 *    transStat_t **transStat - transfer stat output
 */

int
rsDataObjPhymv (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
transStat_t **transStat)
{
    int status;
    dataObjInfo_t *dataObjInfoHead = NULL;
    dataObjInfo_t *oldDataObjInfoHead = NULL;
    rescGrpInfo_t *myRescGrpInfo = NULL;
    ruleExecInfo_t rei;
    int multiCopyFlag;
    char *accessPerm;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;

    remoteFlag = getAndConnRemoteZone (rsComm, dataObjInp, &rodsServerHost,
      REMOTE_OPEN);

    if (remoteFlag < 0) {
        return (remoteFlag);
    } else if (remoteFlag == REMOTE_HOST) {
        status = _rcDataObjPhymv (rodsServerHost->conn, dataObjInp,
          transStat);
        return status;
    }

    *transStat = malloc (sizeof (transStat_t));
    memset (*transStat, 0, sizeof (transStat_t));

    if (getValByKey (&dataObjInp->condInput, IRODS_ADMIN_KW) != NULL) {
	if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
	    return (CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
	}
	accessPerm = NULL;
    } else {
	accessPerm = ACCESS_DELETE_OBJECT;
    }

    /* query rcat for resource info and sort it */
#if 0
    initReiWithDataObjInp (&rei, rsComm, dataObjInp);

    status = applyRule ("acSetRescSchemeForCreate", NULL, &rei, NO_SAVE_REI);

    if (status < 0) {
        if (rei.status < 0)
            status = rei.status;
        rodsLog (LOG_NOTICE,
          "rsDataObjPhymv: acSetRescSchemeForCreate error for %s, status = %d",
          dataObjInp->objPath, status);
        return (status);
    } else {
        myRescGrpInfo = rei.rgi;
        if (myRescGrpInfo == NULL) {
            return (SYS_INVALID_RESC_INPUT);
        }
    }
#endif
    status = getRescGrpForCreate (rsComm, dataObjInp, &myRescGrpInfo);
    if (status < 0) return status;

    initReiWithDataObjInp (&rei, rsComm, dataObjInp);
    status = applyRule ("acSetMultiReplPerResc", NULL, &rei, NO_SAVE_REI);
    if (strcmp (rei.statusStr, MULTI_COPIES_PER_RESC) == 0) {
        multiCopyFlag = 1;
    } else {
        multiCopyFlag = 0;
    }

    /* query rcat for dataObjInfo and sort it */
    status = getDataObjInfo (rsComm, dataObjInp, &dataObjInfoHead,
      accessPerm, 1);

    if (status < 0) {
      rodsLog (LOG_NOTICE,
          "rsDataObjPhymv: getDataObjInfo for %s", dataObjInp->objPath);
        return (status);
    }

	
    status = resolveInfoForPhymv (&dataObjInfoHead, &oldDataObjInfoHead,
      &myRescGrpInfo, &dataObjInp->condInput, multiCopyFlag);
    if (status < 0) {
        freeAllDataObjInfo (dataObjInfoHead);
        freeAllDataObjInfo (oldDataObjInfoHead);
        freeAllRescGrpInfo (myRescGrpInfo);
        if (status == CAT_NO_ROWS_FOUND) {
	    return (0);
	} else {
            return (status);
	}
    }

    status = _rsDataObjPhymv (rsComm, dataObjInp, dataObjInfoHead, 
      myRescGrpInfo, *transStat, multiCopyFlag);

    freeAllDataObjInfo (dataObjInfoHead);
    freeAllDataObjInfo (oldDataObjInfoHead);
    freeAllRescGrpInfo (myRescGrpInfo);

    return (status);
}

int
_rsDataObjPhymv (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t *srcDataObjInfoHead, rescGrpInfo_t *destRescGrpInfo,
transStat_t *transStat, int multiCopyFlag)
{
    dataObjInfo_t *srcDataObjInfo;
    rescGrpInfo_t *tmpRescGrpInfo;
    rescInfo_t *tmpRescInfo;
    int status;
    int savedStatus = 0;

    tmpRescGrpInfo = destRescGrpInfo;
    srcDataObjInfo = srcDataObjInfoHead;
    while (tmpRescGrpInfo != NULL) {
        tmpRescInfo = tmpRescGrpInfo->rescInfo;
        while (srcDataObjInfo != NULL) {
	    /* use _rsDataObjReplS for the phymv */ 
	    dataObjInp->oprType = PHYMV_OPR;	/* should be set already */
            status = _rsDataObjReplS (rsComm, dataObjInp, srcDataObjInfo,
              tmpRescInfo, tmpRescGrpInfo->rescGroupName, NULL);

	    if (multiCopyFlag == 0) {
		if (status >= 0) {
		    srcDataObjInfo = srcDataObjInfo->next;
		} else {
		    savedStatus = status;
		}
		/* use another resc */
		break;
	    } else {
		if (status < 0) {
		    savedStatus = status;
		    /* use another resc */
		    break;
		}
	    }
            srcDataObjInfo = srcDataObjInfo->next;
        }
	if (status >= 0)
            transStat->numThreads = dataObjInp->numThreads;
        tmpRescGrpInfo = tmpRescGrpInfo->next;
    }

    if (srcDataObjInfo != NULL) {
	/* not everything got moved */
	if (savedStatus == 0) {
	    status = SYS_COPY_ALREADY_IN_RESC;
	}
    } else {
	savedStatus = 0;
    }
	    
    return (savedStatus);
}

