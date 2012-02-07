/* This is script-generated code.  */ 
/* See nccfGetVara.h for a description of this API call.*/

#include "nccfGetVara.h"

int
rcNccfGetVara (rcComm_t *conn,   nccfGetVarInp_t *nccfGetVarInp, 
 nccfGetVarOut_t ** nccfGetVarOut)
{
    int status;
    status = procApiRequest (conn, NCCF_GET_VARA_AN, nccfGetVarInp, NULL, 
        (void **) nccfGetVarOut, NULL);

    return (status);
}
