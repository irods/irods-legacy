/* This is script-generated code.  */ 
/* See bulkDataObjPut.h for a description of this API call.*/

#include "bulkDataObjPut.h"

int
rcBulkDataObjPut (rcComm_t *conn, dataObjInp_t *dataObjInp,
bytesBuf_t *dataObjInpBBuf)
{
    int status;
    status = procApiRequest (conn, BULK_DATA_OBJ_PUT_AN, dataObjInp, 
      dataObjInpBBuf, (void **) NULL, NULL);

    return (status);
}
