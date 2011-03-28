/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjPhymv.h for a description of this API call.*/

#include "dataObjPhymv.h"

int
rcDataObjPhymv (rcComm_t *conn, dataObjInp_t *dataObjInp)
{
    int status;
    transferStat_t *transferStat = NULL;

    memset (&conn->transStat, 0, sizeof (transferStat_t));

    dataObjInp->oprType = PHYMV_OPR;

    status = _rcDataObjPhymv (conn, dataObjInp, &transferStat);

    if (status >= 0 && transferStat != NULL) {
	conn->transStat = *(transferStat);
    } else if (status == SYS_UNMATCHED_API_NUM) {
         /* try older version */
        transStat_t *transStat = NULL;
        status = _rcDataObjPhymv250 (conn, dataObjInp, &transStat);
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
_rcDataObjPhymv (rcComm_t *conn, dataObjInp_t *dataObjInp, 
transferStat_t **transferStat)
{
    int status;

    status = procApiRequest (conn, DATA_OBJ_PHYMV_AN,  dataObjInp, NULL,
        (void **) transferStat, NULL);
    return (status);
}

int
_rcDataObjPhymv250 (rcComm_t *conn, dataObjInp_t *dataObjInp,
transStat_t **transStat)
{
    int status;

    status = procApiRequest (conn, DATA_OBJ_PHYMV250_AN,  dataObjInp, NULL,
        (void **) transStat, NULL);
    return (status);
}

