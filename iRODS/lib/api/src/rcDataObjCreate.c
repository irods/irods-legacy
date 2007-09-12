/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjCreate.h for a description of this API call.*/

#include "dataObjCreate.h"

int
rcDataObjCreate (rcComm_t *conn, dataObjInp_t *dataObjInp)
{
    int status;
    status = procApiRequest (conn, DATA_OBJ_CREATE_AN,  dataObjInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}

