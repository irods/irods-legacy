/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjLseek.h for a description of this API call.*/

#include "dataObjLseek.h"

int
rcDataObjLseek (rcComm_t *conn, fileLseekInp_t *dataObjLseekInp,
fileLseekOut_t **dataObjLseekOut)
{
    int status;
    status = procApiRequest (conn, DATA_OBJ_LSEEK_AN,  dataObjLseekInp, NULL, 
        (void **) dataObjLseekOut, NULL);

    return (status);
}
