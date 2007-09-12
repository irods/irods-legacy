/* This is script-generated code.  */ 
/* See bunSubLseek.h for a description of this API call.*/

#include "bunSubLseek.h"

int
rcBunSubLseek (rcComm_t *conn, bunSubLseekInp_t *bunSubLseekInp,
fileLseekOut_t **bunSubLseekOut)
{
    int status;
    status = procApiRequest (conn, BUN_SUB_LSEEK_AN, bunSubLseekInp, NULL, 
        (void **) bunSubLseekOut, NULL);

    return (status);
}
