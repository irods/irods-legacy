/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See dataObjPut.h for a description of this API call.*/

#include "dataObjPut.h"
#include "rodsLog.h"
#include "dataPut.h"
#include "filePut.h"
#include "objMetaOpr.h"
#include "dataObjOpen.h"
#include "dataObjCreate.h"
#include "regDataObj.h"
#include "dataObjUnlink.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "rsApiHandler.h"
#include "subStructFilePut.h"
#include "dataObjRepl.h"


int
rsDataObjPut (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
bytesBuf_t *dataObjInpBBuf, portalOprOut_t **portalOprOut)
{
    int status;

    status = _rsDataObjPut (rsComm, dataObjInp, dataObjInpBBuf,
      portalOprOut, BRANCH_MSG);

    return (status);
}

/* l2DataObjPut - process put request 
 * The reply to this API can go off the main part of the API's
 * reuest/reply protocol and uses the sendAndRecvOffMainMsg call
 * to handle a sequence of reuest/reply until a return value of
 * SYS_HANDLER_NO_ERROR.
 * handlerFlag - INTERNAL_SVR_CALL - called internally by the server.
 *                 affect how return values are handled
 */

int
_rsDataObjPut (rsComm_t *rsComm, dataObjInp_t *dataObjInp, 
bytesBuf_t *dataObjInpBBuf, portalOprOut_t **portalOprOut, int handlerFlag)
{
    int status;
    int l1descInx;
    int retval;
    dataObjCloseInp_t dataObjCloseInp;
    int allFlag;
    transStat_t *transStat = NULL;
    dataObjInp_t replDataObjInp;

    if (getValByKey (&dataObjInp->condInput, ALL_KW) != NULL) {
        allFlag = 1;
    } else {
	allFlag = 0;
    }

    if (getValByKey (&dataObjInp->condInput, DATA_INCLUDED_KW) != NULL) {
        status = l3DataPutSingleBuf (rsComm, dataObjInp, dataObjInpBBuf);
        if (status >= 0 && allFlag == 1) {
	    /* update the rest of copies */
	    addKeyVal (&dataObjInp->condInput, UPDATE_REPL_KW, "");
	    status = rsDataObjRepl (rsComm, dataObjInp, &transStat);
	    if (transStat!= NULL) free (transStat);
	}
	if (status > 0) status = 0;
        return (status);
    }

    l1descInx = rsDataObjCreate (rsComm, dataObjInp);
 
    if (l1descInx < 0) 
	return l1descInx;

    L1desc[l1descInx].oprType = PUT_OPR;
    L1desc[l1descInx].dataSize = dataObjInp->dataSize;

    status = l2DataObjPut (rsComm, l1descInx, portalOprOut);

    if (status < 0) {
        memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
        dataObjCloseInp.l1descInx = l1descInx;
        rsDataObjClose (rsComm, &dataObjCloseInp);
	return (status);
    } 

    if (allFlag == 1) {
	/* need to save dataObjInp. get freed in sendAndRecvBranchMsg */
	memset (&replDataObjInp, 0, sizeof (replDataObjInp));
	rstrcpy (replDataObjInp.objPath, dataObjInp->objPath, MAX_NAME_LEN);
	addKeyVal (&replDataObjInp.condInput, UPDATE_REPL_KW, "");
    }

    retval = sendAndRecvBranchMsg (rsComm, rsComm->apiInx, status,
      (void *) *portalOprOut, NULL);

    if (retval < 0) {
        memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
        dataObjCloseInp.l1descInx = l1descInx;
        rsDataObjClose (rsComm, &dataObjCloseInp);
	if (allFlag == 1) clearKeyVal (&replDataObjInp.condInput);
    } else if (allFlag == 1) {
        status = rsDataObjRepl (rsComm, &replDataObjInp, &transStat);
        if (transStat!= NULL) free (transStat);
	clearKeyVal (&replDataObjInp.condInput);
    }

    if (handlerFlag & INTERNAL_SVR_CALL) {
        /* internal call. want to know the real status */
	return (retval);
    } else {
        /* already send the client the status */
        return (SYS_NO_HANDLER_REPLY_MSG);
    }
}

/* l2DataObjPut - process put request other than DATA_INCLUDED type
 * Data transfer is not done here
 */

int
l2DataObjPut (rsComm_t *rsComm, int l1descInx, 
portalOprOut_t **portalOprOut)
{
    int l3descInx;
    int status;
    dataOprInp_t dataOprInp;

    l3descInx = L1desc[l1descInx].l3descInx;

    initDataOprInp (&dataOprInp, l1descInx, PUT_OPR);
    status =  rsDataPut (rsComm, &dataOprInp, portalOprOut);

    if (status >= 0) {
        (*portalOprOut)->l1descInx = l1descInx;
	L1desc[l1descInx].bytesWritten = dataOprInp.dataSize;
    }
    return (status);
}

int
l3DataPutSingleBuf (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
bytesBuf_t *dataObjInpBBuf)
{
    int status = 0;
    int bytesWritten;
    dataObjCloseInp_t dataObjCloseInp;
    int l1descInx;
    dataObjInfo_t *myDataObjInfo;

    /* don't actually physically open the file */
    addKeyVal (&dataObjInp->condInput, NO_OPEN_FLAG_KW, "");
    l1descInx = rsDataObjCreate (rsComm, dataObjInp);

    if (l1descInx <= 2) {
	if (l1descInx >= 0) {
            rodsLog (LOG_ERROR,
             "l3DataPutSingleBuf: rsDataObjCreate of %s error, status = %d",
              dataObjInp->objPath, l1descInx);
	    return SYS_FILE_DESC_OUT_OF_RANGE;
	} else {
	    return l1descInx;
	}
    }
 
    myDataObjInfo = L1desc[l1descInx].dataObjInfo;
    
    bytesWritten = l3FilePutSingleBuf (rsComm, l1descInx, dataObjInpBBuf);

    if (bytesWritten >= 0) {
	if (L1desc[l1descInx].replStatus == NEWLY_CREATED_COPY && 
	  myDataObjInfo->specColl == NULL) {
            status = svrRegDataObj (rsComm, myDataObjInfo);
            if (status < 0) {
                rodsLog (LOG_NOTICE,
                  "l3DataPutSingleBuf: rsRegDataObj for %s failed, status = %d",
                  myDataObjInfo->objPath, status);
		l3Unlink (rsComm, myDataObjInfo);
                return (status);
	    } else {
                myDataObjInfo->replNum = status;
	    }
        }
        /* myDataObjInfo->dataSize = bytesWritten; update size problem */
        L1desc[l1descInx].bytesWritten = bytesWritten;
    }
 
    L1desc[l1descInx].dataSize = dataObjInp->dataSize;
    memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
    dataObjCloseInp.l1descInx = l1descInx;
    status = rsDataObjClose (rsComm, &dataObjCloseInp);
    if (status < 0) {
	rodsLog (LOG_NOTICE,
	  "l3DataPutSingleBuf: rsDataObjClose of %d error, status = %d",
	    l1descInx, status);
    }

    if (bytesWritten < 0)
        return (bytesWritten);
    else
	return status;

}

int
l3FilePutSingleBuf (rsComm_t *rsComm, int l1descInx, bytesBuf_t *dataObjInpBBuf)
{
    dataObjInfo_t *dataObjInfo;
    int rescTypeInx;
    fileOpenInp_t filePutInp;
    int bytesWritten;
    dataObjInp_t *dataObjInp;
    int retryCnt = 0;

    dataObjInfo = L1desc[l1descInx].dataObjInfo;

    dataObjInp = L1desc[l1descInx].dataObjInp;

    if (getStructFileType (dataObjInfo->specColl) >= 0) {
        subFile_t subFile;
        memset (&subFile, 0, sizeof (subFile));
        rstrcpy (subFile.subFilePath, dataObjInfo->subPath,
          MAX_NAME_LEN);
        rstrcpy (subFile.addr.hostAddr, dataObjInfo->rescInfo->rescLoc,
          NAME_LEN);
        subFile.specColl = dataObjInfo->specColl;
        subFile.mode = getFileMode (l1descInx);
        subFile.flags = O_WRONLY | dataObjInp->openFlags;
        if (getValByKey (&dataObjInp->condInput, FORCE_FLAG_KW) != NULL) {
            subFile.flags |= FORCE_FLAG;
        }
        bytesWritten = rsSubStructFilePut (rsComm, &subFile, dataObjInpBBuf);
        return (bytesWritten);
    }

    rescTypeInx = dataObjInfo->rescInfo->rescTypeInx;

    switch (RescTypeDef[rescTypeInx].rescCat) {
      case FILE_CAT:
        memset (&filePutInp, 0, sizeof (filePutInp));
	if (getValByKey (&dataObjInp->condInput, FORCE_FLAG_KW) != NULL) {
	    filePutInp.otherFlags |= FORCE_FLAG;
	}
        filePutInp.fileType = RescTypeDef[rescTypeInx].driverType;
        rstrcpy (filePutInp.addr.hostAddr,  dataObjInfo->rescInfo->rescLoc,
          NAME_LEN);
        rstrcpy (filePutInp.fileName, dataObjInfo->filePath, MAX_NAME_LEN);
        filePutInp.mode = getFileMode (l1descInx);
        filePutInp.flags = O_WRONLY | dataObjInp->openFlags;
        if (getchkPathPerm (rsComm, L1desc[l1descInx].dataObjInp, 
          L1desc[l1descInx].dataObjInfo)) {
            filePutInp.otherFlags |= CHK_PERM_FLAG;
        }
        bytesWritten = rsFilePut (rsComm, &filePutInp, dataObjInpBBuf);
        /* file already exists ? */
        while (bytesWritten < 0 && retryCnt < 10 &&
          (filePutInp.otherFlags & FORCE_FLAG) == 0 &&
	  getErrno (bytesWritten) == EEXIST) {
            if (resolveDupFilePath (rsComm, dataObjInfo, dataObjInp) < 0) {
                break;
            }
            rstrcpy (filePutInp.fileName, dataObjInfo->filePath,
              MAX_NAME_LEN);
	    bytesWritten = rsFilePut (rsComm, &filePutInp, dataObjInpBBuf);
            retryCnt ++;
        }

        break;
      default:
        rodsLog (LOG_NOTICE,
          "l3Open: rescCat type %d is not recognized",
          RescTypeDef[rescTypeInx].rescCat);
        bytesWritten = SYS_INVALID_RESC_TYPE;
        break;
    }
    return (bytesWritten);
}

