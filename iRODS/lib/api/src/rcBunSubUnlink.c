/* This is script-generated code.  */ 
/* See bunSubUnlink.h for a description of this API call.*/

#include "bunSubUnlink.h"

int
rcBunSubUnlink (rcComm_t *conn, subFile_t *subFile)
{
    int status;
    status = procApiRequest (conn, BUN_SUB_UNLINK_AN, subFile, NULL, 
        (void **) NULL, NULL);

    return (status);
}
