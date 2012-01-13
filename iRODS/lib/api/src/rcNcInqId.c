/* This is script-generated code.  */ 
/* See ncInqId.h for a description of this API call.*/

#include "ncInqId.h"

int
rcNcInqId (rcComm_t *conn, ncInqIdInp_t *ncInqIdInp, int **outId)
{
    int status;

    status = procApiRequest (conn, NC_INQ_ID_AN, ncInqIdInp, NULL, 
        (void **) outId, NULL);

    return (status);
}
