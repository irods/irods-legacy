/* This is script-generated code.  */ 
/* See bunSubRmdir.h for a description of this API call.*/

#include "bunSubRmdir.h"

int
rcBunSubRmdir (rcComm_t *conn, subFile_t *subFile)
{
    int status;
    status = procApiRequest (conn, BUN_SUB_RMDIR_AN, subFile, NULL, 
        (void **) NULL, NULL);

    return (status);
}
