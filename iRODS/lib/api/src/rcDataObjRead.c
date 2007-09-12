/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjRead.h for a description of this API call.*/

#include "dataObjRead.h"

int
rcDataObjRead (rcComm_t *conn, dataObjReadInp_t *fileReadInp,
bytesBuf_t *dataObjReadOutBBuf)
{
    int status;
    status = procApiRequest (conn, DATA_OBJ_READ_AN,  fileReadInp, NULL, 
        (void **) NULL, dataObjReadOutBBuf);

    return (status);
}
