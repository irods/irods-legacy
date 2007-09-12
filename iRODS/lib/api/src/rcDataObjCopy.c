/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjCopy.h for a description of this API call.*/

#include "dataObjCopy.h"

int
rcDataObjCopy (rcComm_t *conn, dataObjCopyInp_t *dataObjCopyInp)
{
    int status;
    transStat_t *transStat = NULL;

    memset (&conn->transStat, 0, sizeof (transStat_t));

    dataObjCopyInp->srcDataObjInp.oprType = COPY_SRC;
    dataObjCopyInp->destDataObjInp.oprType = COPY_DEST;

    status = procApiRequest (conn, DATA_OBJ_COPY_AN,  dataObjCopyInp, NULL, 
        (void **) &transStat, NULL);

    if (status >= 0 && transStat != NULL) {
        conn->transStat = *(transStat);
    }

    if (transStat != NULL) {
        free (transStat);
    }

    return (status);
}
