/* This is script-generated code.  */ 
/* See bunSubWrite.h for a description of this API call.*/

#include "bunSubWrite.h"

int
rcBunSubWrite (rcComm_t *conn, bunSubFdOprInp_t *bunSubWriteInp,
bytesBuf_t *bunSubWriteOutBBuf)
{
    int status;
    status = procApiRequest (conn, BUN_SUB_WRITE_AN, bunSubWriteInp, 
     bunSubWriteOutBBuf, (void **) NULL, NULL);

    return (status);
}
