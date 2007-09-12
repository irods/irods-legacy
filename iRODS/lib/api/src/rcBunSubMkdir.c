/* This is script-generated code.  */ 
/* See bunSubMkdir.h for a description of this API call.*/

#include "bunSubMkdir.h"

int
rcBunSubMkdir (rcComm_t *conn, subFile_t *subFile)
{
    int status;
    status = procApiRequest (conn, BUN_SUB_MKDIR_AN, subFile, NULL, 
        (void **) NULL, NULL);

    return (status);
}
