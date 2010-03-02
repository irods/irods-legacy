/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjPut.h for a description of this API call.*/

#include "dataObjPut.h"
#include "rcPortalOpr.h"
#include "oprComplete.h"

int
rcDataObjPut (rcComm_t *conn, dataObjInp_t *dataObjInp, char *locFilePath)
{
    int status;
    portalOprOut_t *portalOprOut = NULL;
    bytesBuf_t dataObjInpBBuf;

    if (dataObjInp->dataSize <= 0) {
	dataObjInp->dataSize = getFileSize (locFilePath);
	if (dataObjInp->dataSize < 0) {
	    return (USER_FILE_DOES_NOT_EXIST);
	}
    }

    memset (&conn->transStat, 0, sizeof (transStat_t));
    memset (&dataObjInpBBuf, 0, sizeof (dataObjInpBBuf));

    if (getValByKey (&dataObjInp->condInput, DATA_INCLUDED_KW) != NULL) {
	if (dataObjInp->dataSize > MAX_SZ_FOR_SINGLE_BUF) {
	    rmKeyVal (&dataObjInp->condInput, DATA_INCLUDED_KW);
	} else {
	    status = fillBBufWithFile (conn, &dataObjInpBBuf, locFilePath, 
	      dataObjInp->dataSize);
	    if (status < 0) {
	        rodsLog (LOG_NOTICE,
	          "rcDataObjPut: fileBBufWithFile error for %s", locFilePath);
	        return (status);
	    }
	}
    } else if (dataObjInp->dataSize < MAX_SZ_FOR_SINGLE_BUF) {
	addKeyVal (&dataObjInp->condInput, DATA_INCLUDED_KW, "");
        status = fillBBufWithFile (conn, &dataObjInpBBuf, locFilePath,
	  dataObjInp->dataSize);
        if (status < 0) {
            rodsLog (LOG_NOTICE,
              "rcDataObjPut: fileBBufWithFile error for %s", locFilePath);
            return (status);
	}
    }
    
    dataObjInp->oprType = PUT_OPR;

#ifndef PARA_OPR
    addKeyVal (&dataObjInp->condInput, NO_PARA_OP_KW, "");
#endif

    status = _rcDataObjPut (conn, dataObjInp, &dataObjInpBBuf, &portalOprOut);

    clearBBuf (&dataObjInpBBuf);
 
    if (status < 0 || 
      getValByKey (&dataObjInp->condInput, DATA_INCLUDED_KW) != NULL) {
	if (portalOprOut != NULL)
	    free (portalOprOut);
	return (status);
    }

    if (portalOprOut->numThreads <= 0) { 
	status = putFile (conn, portalOprOut->l1descInx, 
	  locFilePath, dataObjInp->dataSize);
#ifdef RBUDP_TRANSFER
    } else if (getUdpPortFromPortList (&portalOprOut->portList) != 0) {
	int veryVerbose;
	/* rbudp transfer */
        /* some sanity check */
        if (portalOprOut->numThreads != 1) {
            rcOprComplete (conn, SYS_INVALID_PORTAL_OPR);
            free (portalOprOut);
            return (SYS_INVALID_PORTAL_OPR);
        }
        conn->transStat.numThreads = portalOprOut->numThreads;
	if (getValByKey (&dataObjInp->condInput, VERY_VERBOSE_KW) != NULL) {
	    printf ("From server: NumThreads=%d, addr:%s, port:%d, cookie=%d\n",
	      portalOprOut->numThreads, portalOprOut->portList.hostAddr,
	      portalOprOut->portList.portNum, portalOprOut->portList.cookie);
	    veryVerbose = 2;
	} else {
	    veryVerbose = 0;
	}
        status = putFileToPortalRbudp (portalOprOut, locFilePath, -1,
          dataObjInp->dataSize, veryVerbose, 0, 0);
#endif  /* RBUDP_TRANSFER */
    } else {
        if (getValByKey (&dataObjInp->condInput, VERY_VERBOSE_KW) != NULL) {
            printf ("From server: NumThreads=%d, addr:%s, port:%d, cookie=%d\n",
              portalOprOut->numThreads, portalOprOut->portList.hostAddr,
              portalOprOut->portList.portNum, portalOprOut->portList.cookie);
	}
	/* some sanity check */
	if (portalOprOut->numThreads >= 20 * DEF_NUM_TRAN_THR) {
    	    rcOprComplete (conn, SYS_INVALID_PORTAL_OPR);
    	    free (portalOprOut);
	    return (SYS_INVALID_PORTAL_OPR);
	}
	conn->transStat.numThreads = portalOprOut->numThreads;
        status = putFileToPortal (conn, portalOprOut, locFilePath, 
	  dataObjInp->dataSize);
    }

    /* just send a complete msg */
    if (status < 0) {
	rcOprComplete (conn, status);
    } else {
        status = rcOprComplete (conn, portalOprOut->l1descInx);
    }
    free (portalOprOut);

    return (status);
}

int
_rcDataObjPut (rcComm_t *conn, dataObjInp_t *dataObjInp,
bytesBuf_t *dataObjInpBBuf, portalOprOut_t **portalOprOut)
{
    int status;

    status = procApiRequest (conn, DATA_OBJ_PUT_AN,  dataObjInp,
        dataObjInpBBuf, (void **) portalOprOut, NULL);

    if (*portalOprOut != NULL && (*portalOprOut)->l1descInx < 0) {
        status = (*portalOprOut)->l1descInx;
    }

    return status;
}

