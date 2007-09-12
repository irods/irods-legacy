/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjChksum.h for a description of this API call.*/

#include "dataObjChksum.h"

int
rcDataObjChksum (rcComm_t *conn, dataObjInp_t *dataObjChksumInp, 
char **outChksum)
{
    int status;
    status = procApiRequest (conn, DATA_OBJ_CHKSUM_AN,  dataObjChksumInp, NULL, 
        (void **) outChksum, NULL);

    return (status);
}

