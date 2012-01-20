/* This is script-generated code.  */ 
/* See ncGetVarsByType.h for a description of this API call.*/

#include "ncGetVarsByType.h"

int
rcNcGetVarsByType (rcComm_t *conn,  ncGetVarInp_t *ncGetVarInp,
ncGetVarOut_t **ncGetVarOut)
{
    int status;
    status = procApiRequest (conn, NC_GET_VARS_BY_TYPE_AN, ncGetVarInp, NULL, 
        (void **) ncGetVarOut, NULL);

    return (status);
}
