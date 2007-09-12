/* This is script-generated code.  */ 
/* See bunSubTruncate.h for a description of this API call.*/

#include "bunSubTruncate.h"

int
rcBunSubTruncate (rcComm_t *conn, subFile_t *bunSubTruncateInp)
{
    int status;
    status = procApiRequest (conn, BUN_SUB_TRUNCATE_AN, bunSubTruncateInp, 
      NULL, (void **) NULL, NULL);

    return (status);
}
