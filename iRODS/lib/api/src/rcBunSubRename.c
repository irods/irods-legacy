/* This is script-generated code.  */ 
/* See bunSubRename.h for a description of this API call.*/

#include "subStructFileRename.h"

int
rcBunSubRename (rcComm_t *conn, bunSubRenameInp_t *bunSubRenameInp)
{
    int status;
    status = procApiRequest (conn, SUB_STRUCT_FILE_RENAME_AN, bunSubRenameInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}
