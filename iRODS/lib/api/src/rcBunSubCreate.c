/* This is script-generated code.  */ 
/* See bunSubCreate.h for a description of this API call.*/

#include "bunSubCreate.h"

int
rcBunSubCreate (rcComm_t *conn, subFile_t *subFile)
{
    int status;
    status = procApiRequest (conn, BUN_SUB_CREATE_AN, subFile, NULL, 
        (void **) NULL, NULL);

    return (status);
}
