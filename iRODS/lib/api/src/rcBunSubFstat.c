/* This is script-generated code.  */ 
/* See bunSubFstat.h for a description of this API call.*/

#include "subStructFileFstat.h"

int
rcBunSubFstat (rcComm_t *conn, bunSubFdOprInp_t *bunSubFstatInp,
rodsStat_t **bunSubStatOut)
{
    int status;
    status = procApiRequest (conn, SUB_STRUCT_FILE_FSTAT_AN, bunSubFstatInp, NULL, 
        (void **) bunSubStatOut, NULL);

    return (status);
}
