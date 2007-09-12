/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjWrite.h for a description of this API call.*/

#include "dataObjWrite.h"

int
rcDataObjWrite (rcComm_t *conn, dataObjWriteInp_t *dataObjWriteInp,
bytesBuf_t *dataObjWriteInpBBuf)
{
    int status;
    status = procApiRequest (conn, DATA_OBJ_WRITE_AN,  dataObjWriteInp, 
      dataObjWriteInpBBuf, (void **) NULL, NULL);

    return (status);
}
