/* This is script-generated code.  */ 
/* See bunSubClose.h for a description of this API call.*/

#include "subStructFileClose.h"

int
rcBunSubClose (rcComm_t *conn, bunSubFdOprInp_t *bunSubCloseInp)
{
    int status;
    status = procApiRequest (conn, SUB_STRUCT_FILE_CLOSE_AN, bunSubCloseInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}
