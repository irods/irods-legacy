/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjUnlink.h for a description of this API call.*/

#include "dataObjUnlink.h"

int
rcDataObjUnlink (rcComm_t *conn, dataObjInp_t *dataObjUnlinkInp)
{
    int status;
    status = procApiRequest (conn, DATA_OBJ_UNLINK_AN,  dataObjUnlinkInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}
