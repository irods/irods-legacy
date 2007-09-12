/* This is script-generated code.  */ 
/* See bunSubOpen.h for a description of this API call.*/

#include "bunSubOpen.h"

int
rcBunSubOpen (rcComm_t *conn, subFile_t *subFile)
{
    int status;
    status = procApiRequest (conn, BUN_SUB_OPEN_AN, subFile, NULL, 
        (void **) NULL, NULL);

    return (status);
}
