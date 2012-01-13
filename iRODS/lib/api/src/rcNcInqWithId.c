/* This is script-generated code.  */ 
/* See ncInqWithId.h for a description of this API call.*/

#include "ncInqWithId.h"

int
rcNcInqWithId (rcComm_t *conn, ncInqIdInp_t *ncInqWithIdInp,
ncInqWithIdOut_t **ncInqWithIdOut)
{
    int status;

    status = procApiRequest (conn, NC_INQ_WITH_ID_AN, ncInqWithIdInp, NULL, 
        (void **) ncInqWithIdOut, NULL);

    return (status);
}
