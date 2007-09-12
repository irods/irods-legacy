/* This is script-generated code.  */ 
/* See bunSubOpendir.h for a description of this API call.*/

#include "bunSubOpendir.h"

int
rcBunSubOpendir (rcComm_t *conn, subFile_t *subFile)
{
    int status;
    status = procApiRequest (conn, BUN_SUB_OPENDIR_AN, subFile, NULL, 
        (void **) NULL, NULL);

    return (status);
}
