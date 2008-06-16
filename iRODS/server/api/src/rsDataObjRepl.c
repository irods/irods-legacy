/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See dataObjRepl.h for a description of this API call.*/

#include "dataObjRepl.h"
#include "dataObjOpr.h"
#include "dataObjCreate.h"
#include "dataObjOpen.h"
#include "dataObjPut.h"
#include "dataObjGet.h"
#include "rodsLog.h"
#include "objMetaOpr.h"
#include "reGlobalsExtern.h"
#include "reDefines.h"
#include "reSysDataObjOpr.h"

/* rsDataObjRepl - The Api handler of the rcDataObjRepl call - Replicate
 * a data object.
 * Input -
 *    rsComm_t *rsComm 
 *    dataObjInp_t *dataObjInp - The replication input
 *    transStat_t **transStat - transfer stat output
 */

int
rsDataObjRepl (rsComm_t *rsComm, dataObjInp_t *dataObjInp, 
transStat_t **transStat)
{
    int status;

    *transStat = malloc (sizeof (transStat_t));
    memset (*transStat, 0, sizeof (transStat_t));

    status = rsDataObjReplWithOutDataObj (rsComm, dataObjInp, 
     *transStat, NULL); 
    return (status);
}
    
int
rsDataObjReplWithOutDataObj (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
transStat_t *transStat, dataObjInfo_t *outDataObjInfo)
{
    int status;
    dataObjInfo_t *dataObjInfoHead = NULL;
    dataObjInfo_t *oldDataObjInfoHead = NULL;
    dataObjInfo_t *destDataObjInfo = NULL;
    rescGrpInfo_t *myRescGrpInfo = NULL;
    ruleExecInfo_t rei;
    int multiCopyFlag;
    char *accessPerm;
    int backupFlag;

    if (getValByKey (&dataObjInp->condInput, IRODS_ADMIN_KW) != NULL) {
        if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
            return (CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
        }
        accessPerm = NULL;
    } else {
        accessPerm = ACCESS_READ_OBJECT;
    }
    /* query rcat for resource info and sort it */
    initReiWithDataObjInp (&rei, rsComm, dataObjInp);

    status = applyRule ("acSetRescSchemeForCreate", NULL, &rei, NO_SAVE_REI);

    if (status < 0) {
        if (rei.status < 0)
            status = rei.status;
        rodsLog (LOG_NOTICE,
          "rsDataObjRepl: acSetRescSchemeForCreate error for %s, status = %d",
          dataObjInp->objPath, status);
        return (status);
    } else {
        myRescGrpInfo = rei.rgi;
        if (myRescGrpInfo == NULL) {
            return (SYS_INVALID_RESC_INPUT);
        }
    }

    status = applyRule ("acSetMultiReplPerResc", NULL, &rei, NO_SAVE_REI);
    if (strcmp (rei.statusStr, MULTI_COPIES_PER_RESC) == 0) {
        multiCopyFlag = 1;
    } else {
        multiCopyFlag = 0;
    }

    /* query rcat for dataObjInfo and sort it */

    if (multiCopyFlag) {
        status = getDataObjInfo (rsComm, dataObjInp, &dataObjInfoHead,
          accessPerm, 0);
    } else {
        status = getDataObjInfo (rsComm, dataObjInp, &dataObjInfoHead,
          accessPerm, 1);
    }

    if (status < 0) {
      rodsLog (LOG_NOTICE,
          "rsDataObjRepl: getDataObjInfo for %s", dataObjInp->objPath);
        return (status);
    }

    sortObjInfoForRepl (&dataObjInfoHead, &oldDataObjInfoHead, multiCopyFlag);

    if (getValByKey (&dataObjInp->condInput, UPDATE_REPL_KW) != NULL) {
	/* update old repl to new repl */
        status = _rsDataObjRepl (rsComm, dataObjInp, dataObjInfoHead,
          NULL, transStat, oldDataObjInfoHead);
        freeAllDataObjInfo (dataObjInfoHead);
        freeAllDataObjInfo (oldDataObjInfoHead);
        freeAllRescGrpInfo (myRescGrpInfo);
	return status;
    }

    if (getValByKey (&dataObjInp->condInput, BACKUP_RESC_NAME_KW) != NULL) {
	backupFlag = 1;
    } else {
	backupFlag = 0;
    }
    if (multiCopyFlag == 0 || backupFlag == 1) {
        status = resolveSingleReplCopy (&dataObjInfoHead, &oldDataObjInfoHead,
          &myRescGrpInfo, &destDataObjInfo, &dataObjInp->condInput);
        if (status == HAVE_GOOD_COPY || status == CAT_NO_ROWS_FOUND) {
            freeAllDataObjInfo (dataObjInfoHead);
            freeAllDataObjInfo (oldDataObjInfoHead);
            freeAllRescGrpInfo (myRescGrpInfo);
	    if (status == HAVE_GOOD_COPY && backupFlag == 0) {
		return (SYS_COPY_ALREADY_IN_RESC);
	    } else {
                return (0);
	    }
        }
    }

    rei.doi = dataObjInfoHead;
    status = applyRule ("acPreprocForDataObjOpen", NULL, &rei, NO_SAVE_REI);

    if (status < 0) {
        if (rei.status < 0)
            status = rei.status;
        rodsLog (LOG_NOTICE,
         "rsDataObjRepl: acPreprocForDataObjOpen error for %s, status=%d",
          dataObjInp->objPath, status);
        return (status);
    } else {
        dataObjInfoHead = rei.doi;
    }

    if (destDataObjInfo != NULL) {
        status = _rsDataObjRepl (rsComm, dataObjInp, dataObjInfoHead, 
          myRescGrpInfo, transStat, destDataObjInfo);
	if (status >= 0 && outDataObjInfo != NULL) {
	    *outDataObjInfo = *destDataObjInfo;
	}
    } else {
        status = _rsDataObjRepl (rsComm, dataObjInp, dataObjInfoHead, 
          myRescGrpInfo, transStat, outDataObjInfo);
    }

    freeAllDataObjInfo (dataObjInfoHead);
    freeAllDataObjInfo (oldDataObjInfoHead);
    freeAllRescGrpInfo (myRescGrpInfo);

    return (status);
} 

/* _rsDataObjRepl - An internal version of rsDataObjRepl with the 
 * Additinal input - 
 *   dataObjInfo_t *srcDataObjInfoHead _ a link list of the src to be 
 *     replicated. Only one will be picked. 
 *   rescGrpInfo_t *destRescGrpInfo - The dest resource info
 *   dataObjInfo_t *destDataObjInfo - This can be both input and output.
 *      If destDataObjInfo == NULL, dest is new and no output is required.
 *      If destDataObjInfo != NULL:
 *	    If destDataObjInfo->dataId <= 0, no input but put output in
 *	    destDataObjInfo. This is needed by msiSysReplDataObj and
 *	    msiStageDataObj which need a copy of destDataObjInfo.
 *	    If destDataObjInfo->dataId > 0, the dest repl exists. Need to
 *          overwrite it. 
 */

int
_rsDataObjRepl (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t *srcDataObjInfoHead, rescGrpInfo_t *destRescGrpInfo,
transStat_t *transStat, dataObjInfo_t *inpDestDataObjInfo) 
{
    dataObjInfo_t *destDataObjInfo;
    dataObjInfo_t *srcDataObjInfo;
    rescGrpInfo_t *tmpRescGrpInfo;
    rescInfo_t *tmpRescInfo;
    int status;
    int allFlag;
    int savedStatus = 0;

    if (getValByKey (&dataObjInp->condInput, ALL_KW) != NULL) {
	allFlag = 1;
    } else {
	allFlag = 0;
    }

    transStat->bytesWritten = srcDataObjInfoHead->dataSize;
    destDataObjInfo = inpDestDataObjInfo;
    while (destDataObjInfo != NULL) {
        if (destDataObjInfo->dataId > 0) {
	    srcDataObjInfo = srcDataObjInfoHead;
	    while (srcDataObjInfo != NULL) {
                /* overwrite a specific destDataObjInfo */
                status = _rsDataObjReplS (rsComm, dataObjInp, srcDataObjInfo,
                  NULL, "", destDataObjInfo);
		if (status >= 0) {
		    break;
		}
		srcDataObjInfo = srcDataObjInfo->next;
	    }
            if (status >= 0) {
                transStat->numThreads = dataObjInp->numThreads;
		if (allFlag == 0) {
                    return 0;
		}
            } else {
                savedStatus = status;
            }
	}
	destDataObjInfo = destDataObjInfo->next;
    }
	    
    tmpRescGrpInfo = destRescGrpInfo;
    while (tmpRescGrpInfo != NULL) {
        tmpRescInfo = tmpRescGrpInfo->rescInfo;
        srcDataObjInfo = srcDataObjInfoHead;
        while (srcDataObjInfo != NULL) {
            status = _rsDataObjReplS (rsComm, dataObjInp, srcDataObjInfo,
              tmpRescInfo, tmpRescGrpInfo->rescGroupName, destDataObjInfo);

             if (status >= 0) {
                  break;
              }
              srcDataObjInfo = srcDataObjInfo->next;
	}
        if (status >= 0) {
            transStat->numThreads = dataObjInp->numThreads;
            if (allFlag == 0) {
                return 0;
            }
        } else {
            savedStatus = status;
        }
        tmpRescGrpInfo = tmpRescGrpInfo->next;
    }

    return (savedStatus);
}

/* _rsDataObjReplS - replicate a single obj 
 *   dataObjInfo_t *srcDataObjInfo - the src to be replicated. 
 *   rescInfo_t *destRescInfo - The dest resource info
 *   dataObjInfo_t *destDataObjInfo - This can be both input and output.
 *      If destDataObjInfo == NULL, dest is new and no output is required.
 *      If destDataObjInfo != NULL:
 *          If destDataObjInfo->dataId <= 0, no input but put output in
 *          destDataObjInfo. This is needed by msiSysReplDataObj and
 *          msiStageDataObj which need a copy of destDataObjInfo.
 *          If destDataObjInfo->dataId > 0, the dest repl exists. Need to
 *          overwrite it.
 */
int
_rsDataObjReplS (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t *srcDataObjInfo, rescInfo_t *destRescInfo, 
char *rescGroupName, dataObjInfo_t *destDataObjInfo)
{
    int status, status1;
    int l1descInx;
    dataObjCloseInp_t dataObjCloseInp;

    l1descInx = dataObjOpenForRepl (rsComm, dataObjInp, srcDataObjInfo,
      destRescInfo, rescGroupName, destDataObjInfo);

    if (l1descInx < 0) {
        return (l1descInx);
    }

    if (dataObjInp->numThreads == 0) {
        status = l3DataCopySingleBuf (rsComm, l1descInx);
    } else {
        status = dataObjCopy (rsComm, l1descInx);
    }

    memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));

    dataObjCloseInp.l1descInx = l1descInx;
    if (status >= 0) {
	dataObjCloseInp.bytesWritten = L1desc[l1descInx].dataObjInfo->dataSize;
    }

    status1 = rsDataObjClose (rsComm, &dataObjCloseInp);

    if (status < 0) {
	return status;
    } else if (status1 < 0) {
	return status1;
    } else {
        return (status);
    }
}

/* dataObjOpenForRepl - Create/open the dest and open the src
 */

int 
dataObjOpenForRepl (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t *inpSrcDataObjInfo, rescInfo_t *destRescInfo,
char *rescGroupName, dataObjInfo_t *inpDestDataObjInfo)
{
    dataObjInfo_t *myDestDataObjInfo, *srcDataObjInfo;
    int destL1descInx;
    int srcL1descInx;
    int status;
    int destExist;
    int replStatus;

    /* open the dest */

    dataObjInp->dataSize = inpSrcDataObjInfo->dataSize;
    destL1descInx = allocL1desc ();

    myDestDataObjInfo = calloc (1, sizeof (dataObjInfo_t));
    srcDataObjInfo = calloc (1, sizeof (dataObjInfo_t));
    *srcDataObjInfo = *inpSrcDataObjInfo;
    if (inpDestDataObjInfo != NULL && inpDestDataObjInfo->dataId > 0) {
	/* overwriting an existing replica */
	/* inherit the replStatus of the src */
	inpDestDataObjInfo->replStatus = srcDataObjInfo->replStatus;
	*myDestDataObjInfo = *inpDestDataObjInfo;
	destExist = 1;
	replStatus = srcDataObjInfo->replStatus | OPEN_EXISTING_COPY;
	addKeyVal (&dataObjInp->condInput, FORCE_FLAG_KW, "");
	dataObjInp->openFlags |= (O_TRUNC | O_WRONLY);
    } else {
        initDataObjInfoForRepl (myDestDataObjInfo, srcDataObjInfo, 
	 destRescInfo);
	rstrcpy (myDestDataObjInfo->rescGroupName, rescGroupName, NAME_LEN);
	destExist = 0;
	replStatus = srcDataObjInfo->replStatus;
    }

    fillL1desc (destL1descInx, dataObjInp, myDestDataObjInfo, 
      replStatus, srcDataObjInfo->dataSize);
    if (dataObjInp->oprType == PHYMV_OPR) {
	L1desc[destL1descInx].oprType = PHYMV_DEST;
	myDestDataObjInfo->replNum = srcDataObjInfo->replNum;
	myDestDataObjInfo->dataId = srcDataObjInfo->dataId;
    } else {
        L1desc[destL1descInx].oprType = REPLICATE_DEST;
    }

    dataObjInp->numThreads = getNumThreads (rsComm, dataObjInp->dataSize, 
      dataObjInp->numThreads, NULL);


    if (dataObjInp->numThreads > 0) {
	if (destExist > 0) {
            status = dataOpen (rsComm, destL1descInx);
	} else {
            status = getFilePathName (rsComm, myDestDataObjInfo,
             L1desc[destL1descInx].dataObjInp);
	    if (status >= 0) 
                status = dataCreate (rsComm, destL1descInx);
	}

        if (status < 0) {
	    freeL1desc (destL1descInx);
	    return (status);
        }
    } else {
	if (destExist == 0) {
	    status = getFilePathName (rsComm, myDestDataObjInfo, 
	     L1desc[destL1descInx].dataObjInp);
            if (status < 0) {
                freeL1desc (destL1descInx);
                return (status);
            }
	}
    }

    if (inpDestDataObjInfo != NULL && inpDestDataObjInfo->dataId == 0) {
        /* a new replica */
        *inpDestDataObjInfo = *myDestDataObjInfo;
    }

    /* open the src */

    srcL1descInx = allocL1desc ();
    fillL1desc (srcL1descInx, dataObjInp, srcDataObjInfo, 
     srcDataObjInfo->replStatus, srcDataObjInfo->dataSize);
    if (dataObjInp->oprType == PHYMV_OPR) {
        L1desc[srcL1descInx].oprType = PHYMV_SRC;
    } else {
        L1desc[srcL1descInx].oprType = REPLICATE_SRC;
    }

    if (dataObjInp->numThreads > 0) {
	dataObjCloseInp_t dataObjCloseInp;

	dataObjInp->openFlags = O_RDONLY;
        status = dataOpen (rsComm, srcL1descInx);
	if (status < 0) {
	    freeL1desc (srcL1descInx);
    	    memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
            dataObjCloseInp.l1descInx = destL1descInx;
	    rsDataObjClose (rsComm, &dataObjCloseInp);
	    return (status);
	}
    }
    L1desc[destL1descInx].srcL1descInx = srcL1descInx;

    return (destL1descInx);
}

int
dataObjCopy (rsComm_t *rsComm, int l1descInx)
{
    int srcL1descInx, destL1descInx;
    int srcL3descInx, destL3descInx;
    int status;
    portalOprOut_t *portalOprOut = NULL;
    dataCopyInp_t dataCopyInp;
    dataOprInp_t *dataOprInp;


    dataOprInp = &dataCopyInp.dataOprInp;
    srcL1descInx = L1desc[l1descInx].srcL1descInx;
    destL1descInx = l1descInx;

    srcL3descInx = L1desc[srcL1descInx].l3descInx;
    destL3descInx = L1desc[destL1descInx].l3descInx;

    if (FileDesc[srcL3descInx].rodsServerHost == 
      FileDesc[destL3descInx].rodsServerHost) {
	dataObjInfo_t *dataObjInfo;

        initDataOprInp (&dataCopyInp.dataOprInp, l1descInx, SAME_HOST_COPY_OPR);
	/* source and dest on the same host */

	dataObjInfo = L1desc[srcL1descInx].dataObjInfo;
	dataOprInp->srcL3descInx = srcL3descInx;
	dataOprInp->srcRescTypeInx = 
	  dataObjInfo->rescInfo->rescTypeInx;
	if (FileDesc[srcL3descInx].rodsServerHost->localFlag == LOCAL_HOST) {
	    addKeyVal (&dataOprInp->condInput, EXEC_LOCALLY_KW, "");
	}
    } else if (
      FileDesc[srcL3descInx].rodsServerHost->localFlag == LOCAL_HOST &&
      FileDesc[destL3descInx].rodsServerHost->localFlag == REMOTE_HOST) {

        initDataOprInp (&dataCopyInp.dataOprInp, srcL1descInx, COPY_TO_REM_OPR);
	status = l2DataObjPut (rsComm, destL1descInx, &portalOprOut);
       if (status < 0) {
            rodsLog (LOG_NOTICE,
              "dataObjCopy: l2DataObjPut error for %s",
              L1desc[srcL1descInx].dataObjInfo->objPath);
            return (status);
        }
        dataCopyInp.portalOprOut = *portalOprOut;
        addKeyVal (&dataOprInp->condInput, EXEC_LOCALLY_KW, "");
    } else if (
      FileDesc[srcL3descInx].rodsServerHost->localFlag == REMOTE_HOST &&
      FileDesc[destL3descInx].rodsServerHost->localFlag == LOCAL_HOST) {

        initDataOprInp (&dataCopyInp.dataOprInp, l1descInx, COPY_TO_LOCAL_OPR);
        status = l2DataObjGet (rsComm, srcL1descInx, &portalOprOut);
       if (status < 0) {
            rodsLog (LOG_NOTICE,
              "dataObjCopy: _rsDataObjGet error for %s",
              L1desc[srcL1descInx].dataObjInfo->objPath);
            return (status);
        }
        addKeyVal (&dataOprInp->condInput, EXEC_LOCALLY_KW, "");
        dataCopyInp.portalOprOut = *portalOprOut;
    } else {
        initDataOprInp (&dataCopyInp.dataOprInp, l1descInx, COPY_TO_LOCAL_OPR);
        status = l2DataObjGet (rsComm, srcL1descInx, &portalOprOut);

       if (status < 0) {
            rodsLog (LOG_NOTICE,
              "dataObjCopy: _rsDataObjGet error for %s", 
	      L1desc[srcL1descInx].dataObjInfo->objPath);
            return (status);
        }

        dataCopyInp.portalOprOut = *portalOprOut;
    }
    status =  rsDataCopy (rsComm, &dataCopyInp);

    return (status);
}

int
l3DataCopySingleBuf (rsComm_t *rsComm, int l1descInx)
{
    bytesBuf_t dataBBuf;
    int bytesRead, bytesWritten;
    int srcL1descInx;

    memset (&dataBBuf, 0, sizeof (bytesBuf_t));

    srcL1descInx = L1desc[l1descInx].srcL1descInx;
    if (L1desc[srcL1descInx].dataSize < 0) {
	rodsLog (LOG_ERROR,
	  "l3DataCopySingleBuf: dataSize %lld for %s is negative",
	  L1desc[srcL1descInx].dataSize, 
	  L1desc[srcL1descInx].dataObjInfo->objPath); 
	return (SYS_COPY_LEN_ERR);
    } else if (L1desc[srcL1descInx].dataSize == 0) {
	bytesRead = 0;
    } else {
        dataBBuf.buf = malloc (L1desc[srcL1descInx].dataSize);
        bytesRead = l3FileGetSingleBuf (rsComm, srcL1descInx, &dataBBuf);
    }

    if (bytesRead < 0) {
	return (bytesRead);
    }

    bytesWritten = l3FilePutSingleBuf (rsComm, l1descInx, &dataBBuf);
 
    if (dataBBuf.buf != NULL) {
	free (dataBBuf.buf);
	memset (&dataBBuf, 0, sizeof (bytesBuf_t));
    }

    if (bytesWritten != bytesRead) {
	if (bytesWritten >= 0) {
            rodsLog (LOG_NOTICE,
              "l3DataCopySingleBuf: l3FilePut error, towrite %d, written %d",
              bytesRead, bytesWritten);
            return (SYS_COPY_LEN_ERR);
        } else {
	    return (bytesWritten);
	}
    }
	
    L1desc[l1descInx].bytesWritten = bytesWritten;

    return (0); 
}

