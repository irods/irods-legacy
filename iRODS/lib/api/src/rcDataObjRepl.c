/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjRepl.h for a description of this API call.*/

#include "dataObjRepl.h"

int
rcDataObjRepl (rcComm_t *conn, dataObjInp_t *dataObjInp)
{
    int status;
    transferStat_t *transferStat = NULL;

    memset (&conn->transStat, 0, sizeof (transferStat_t));

    dataObjInp->oprType = REPLICATE_OPR;

    status = _rcDataObjRepl (conn, dataObjInp, &transferStat);

    if (status >= 0 && transferStat != NULL) {
	conn->transStat = *(transferStat);
    } else if (status == SYS_UNMATCHED_API_NUM) {
	 /* try older version */
	transStat_t *transStat = NULL;
        status = _rcDataObjRepl250 (conn, dataObjInp, &transStat);
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
_rcDataObjRepl (rcComm_t *conn, dataObjInp_t *dataObjInp, 
transferStat_t **transferStat)
{
    int status;

    status = procApiRequest (conn, DATA_OBJ_REPL_AN,  dataObjInp, NULL, 
        (void **) transferStat, NULL);

    return status;
}

int
_rcDataObjRepl250 (rcComm_t *conn, dataObjInp_t *dataObjInp,
transStat_t **transStat)
{
    int status;

    status = procApiRequest (conn, DATA_OBJ_REPL250_AN,  dataObjInp, NULL,
        (void **) transStat, NULL);

    return status;
}

