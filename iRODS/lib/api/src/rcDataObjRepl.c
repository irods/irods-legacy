/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjRepl.h for a description of this API call.*/

#include "dataObjRepl.h"

int
rcDataObjRepl (rcComm_t *conn, dataObjInp_t *dataObjInp)
{
    int status;
    transStat_t *transStat = NULL;

    memset (&conn->transStat, 0, sizeof (transStat_t));

    dataObjInp->oprType = REPLICATE_OPR;

    status = _rcDataObjRepl (conn, dataObjInp, &transStat);

    if (status >= 0 && transStat != NULL) {
	conn->transStat = *(transStat);
    }

    if (transStat != NULL) {
	free (transStat);
    }

    return (status);
}

int
_rcDataObjRepl (rcComm_t *conn, dataObjInp_t *dataObjInp, 
transStat_t **transStat)
{
    int status;

    status = procApiRequest (conn, DATA_OBJ_REPL_AN,  dataObjInp, NULL, 
        (void **) transStat, NULL);

    return status;
}

