/* This is script-generated code.  */ 
/* See bunSubRead.h for a description of this API call.*/

#include "bunSubRead.h"

int
rcBunSubRead (rcComm_t *conn, bunSubFdOprInp_t *bunSubReadInp,
bytesBuf_t *bunSubReadOutBBuf)
{
    int status;
    status = procApiRequest (conn, BUN_SUB_READ_AN, bunSubReadInp, NULL, 
        (void **) NULL, bunSubReadOutBBuf);

    return (status);
}
