/* This is script-generated code.  */ 
/* See bulkDataObjPut.h for a description of this API call.*/

#include "bulkDataObjPut.h"

int
rcBulkDataObjPut (rcComm_t *conn, bulkOprInp_t *bulkOprInp,
bytesBuf_t *bulkOprInpBBuf)
{
    int status;
    status = procApiRequest (conn, BULK_DATA_OBJ_PUT_AN, bulkOprInp, 
      bulkOprInpBBuf, (void **) NULL, NULL);

    return (status);
}
