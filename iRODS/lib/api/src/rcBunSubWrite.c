/* This is script-generated code.  */ 
/* See bunSubWrite.h for a description of this API call.*/

#include "subStructFileWrite.h"

int
rcBunSubWrite (rcComm_t *conn, bunSubFdOprInp_t *bunSubWriteInp,
bytesBuf_t *bunSubWriteOutBBuf)
{
    int status;
    status = procApiRequest (conn, SUB_STRUCT_FILE_WRITE_AN, bunSubWriteInp, 
     bunSubWriteOutBBuf, (void **) NULL, NULL);

    return (status);
}
