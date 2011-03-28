/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjCopy.h for a description of this API call.*/

#include "dataObjCopy.h"

int
rcDataObjCopy (rcComm_t *conn, dataObjCopyInp_t *dataObjCopyInp)
{
    int status;
    transferStat_t *transferStat = NULL;

    memset (&conn->transStat, 0, sizeof (transferStat_t));

    dataObjCopyInp->srcDataObjInp.oprType = COPY_SRC;
    dataObjCopyInp->destDataObjInp.oprType = COPY_DEST;

    status = _rcDataObjCopy (conn, dataObjCopyInp, &transferStat);

    if (status >= 0 && transferStat != NULL) {
        conn->transStat = *(transferStat);
    } else if (status == SYS_UNMATCHED_API_NUM) {
         /* try older version */
        transStat_t *transStat = NULL;
        status = _rcDataObjCopy250 (conn, dataObjCopyInp, &transStat);
        if (status >= 0 && transStat != NULL) {
            conn->transStat.numThreads = transStat->numThreads;
            conn->transStat.bytesWritten = transStat->bytesWritten;
            conn->transStat.flags = 0;
        }
        if (transStat != NULL) free (transStat);
        return status;
    }
    if (transferStat != NULL) {
        free (transferStat);
    }

    return (status);
}

int
_rcDataObjCopy (rcComm_t *conn, dataObjCopyInp_t *dataObjCopyInp,
transferStat_t **transferStat)
{
    int status;

    status = procApiRequest (conn, DATA_OBJ_COPY_AN,  dataObjCopyInp, NULL,
        (void **) transferStat, NULL);

    return status;
}

int
_rcDataObjCopy250 (rcComm_t *conn, dataObjCopyInp_t *dataObjCopyInp,
transStat_t **transStat)
{
    int status;

    status = procApiRequest (conn, DATA_OBJ_COPY250_AN,  dataObjCopyInp, NULL,
        (void **) transStat, NULL);

    return status;
}

