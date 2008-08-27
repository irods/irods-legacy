/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjGet.h for a description of this API call.*/

#include "dataObjGet.h"
#include "rcPortalOpr.h"
#include "apiHeaderAll.h"


int
rcDataObjGet (rcComm_t *conn, dataObjInp_t *dataObjInp, char *locFilePath)
{
    int status;
    portalOprOut_t *portalOprOut = NULL;
    bytesBuf_t dataObjOutBBuf;
    struct stat statbuf;

    if (strcmp (locFilePath, STDOUT_FILE_NAME) == 0) {
	/* no parallel I/O if pipe to stdout */
	dataObjInp->numThreads = NO_THREADING;
    } else if (stat (locFilePath, &statbuf) >= 0) {
	/* local file exists */
	if (getValByKey (&dataObjInp->condInput, FORCE_FLAG_KW) == NULL) {
	    return (OVERWITE_WITHOUT_FORCE_FLAG);
	}
    }

    memset (&conn->transStat, 0, sizeof (transStat_t));

    memset (&dataObjOutBBuf, 0, sizeof (bytesBuf_t));

    dataObjInp->oprType = GET_OPR;

#ifndef PARA_OPR
    addKeyVal (&dataObjInp->condInput, NO_PARA_OP_KW, "");
#endif

    status = procApiRequest (conn, DATA_OBJ_GET_AN,  dataObjInp, NULL, 
        (void **) &portalOprOut, &dataObjOutBBuf);

    if (status < 0) {
        if (portalOprOut != NULL)
            free (portalOprOut);
        return (status);
    } else if (portalOprOut != NULL && portalOprOut->l1descInx < 0) {
        status = portalOprOut->l1descInx;
	if (portalOprOut != NULL)
            free (portalOprOut);
        return (status);
    }

    if (status == 0 || dataObjOutBBuf.len > 0) {
	/* data included */
	if (dataObjInp->dataSize > 0 && 
	  dataObjInp->dataSize != dataObjOutBBuf.len) { 
             rodsLog (LOG_NOTICE,
              "putFile: totalWritten %lld dataSize %lld mismatch",
              dataObjOutBBuf.len, dataObjInp->dataSize);
            return (SYS_COPY_LEN_ERR);
	}
	status = getIncludeFile (conn, &dataObjOutBBuf, locFilePath);
	free (dataObjOutBBuf.buf);
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
            veryVerbose = 2;
        } else {
            veryVerbose = 0;
        }
        status = getFileToPortalRbudp (portalOprOut, locFilePath, 0,
          dataObjInp->dataSize, veryVerbose, 0);
        /* just send a complete msg */
        if (status < 0) {
            rcOprComplete (conn, status);
        } else {
            status = rcOprComplete (conn, portalOprOut->l1descInx);
        }
#endif  /* RBUDP_TRANSFER */
    } else {

        if (portalOprOut->numThreads <= 0) {
            status = getFile (conn, portalOprOut->l1descInx,
              locFilePath, dataObjInp->dataSize);
        } else {
            /* some sanity check */
            if (portalOprOut->numThreads >= 20 * MAX_NUM_TRAN_THR) {
                rcOprComplete (conn, SYS_INVALID_PORTAL_OPR);
                free (portalOprOut);
                return (SYS_INVALID_PORTAL_OPR);
            }

            conn->transStat.numThreads = portalOprOut->numThreads;
            status = getFileFromPortal (conn, portalOprOut, locFilePath,
              dataObjInp->dataSize);
	}
        /* just send a complete msg */
        if (status < 0) {
            rcOprComplete (conn, status);
        } else {
            status = rcOprComplete (conn, portalOprOut->l1descInx);
        }
    }

    if (getValByKey (&dataObjInp->condInput, VERIFY_CHKSUM_KW) != NULL) {
	if (portalOprOut == NULL || strlen (portalOprOut->chksum) == 0) {
	    rodsLog (LOG_ERROR, 
	      "rcDataObjGet: VERIFY_CHKSUM_KW set but no chksum from server");
	} else {
	    char chksumStr[NAME_LEN];

            status = chksumLocFile (locFilePath, chksumStr);

            if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                  "rcDataObjGet: chksumLocFile error for %s, status = %d",
		  locFilePath, status);
		if (portalOprOut != NULL)
		    free (portalOprOut);
                return (status);
            }
 
	    if (strcmp (portalOprOut->chksum, chksumStr) != 0) {
	        status = USER_CHKSUM_MISMATCH;
                rodsLogError (LOG_ERROR, status,
                  "rcDataObjGet: chksum mismatch error for %s, status = %d",
                  locFilePath, status);
		if (portalOprOut != NULL)
		    free (portalOprOut);
                return (status);
            }
	}
    }
    if (portalOprOut != NULL) {
        free (portalOprOut);
    }

    return (status);
}

