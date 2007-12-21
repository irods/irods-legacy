/* This is script-generated code.  */ 
/* See bunSubClosedir.h for a description of this API call.*/

#include "subStructFileClosedir.h"

int
rcBunSubClosedir (rcComm_t *conn, bunSubFdOprInp_t *bunSubClosedirInp)
{
    int status;
    status = procApiRequest (conn, SUB_STRUCT_FILE_CLOSEDIR_AN, bunSubClosedirInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}
