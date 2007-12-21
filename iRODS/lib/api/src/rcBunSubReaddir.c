/* This is script-generated code.  */ 
/* See bunSubReaddir.h for a description of this API call.*/

#include "subStructFileReaddir.h"

int
rcBunSubReaddir (rcComm_t *conn, bunSubFdOprInp_t *bunSubReaddirInp,
rodsDirent_t **rodsDirent)
{
    int status;
    status = procApiRequest (conn, SUB_STRUCT_FILE_READDIR_AN, bunSubReaddirInp, NULL, 
        (void **) rodsDirent, NULL);

    return (status);
}
