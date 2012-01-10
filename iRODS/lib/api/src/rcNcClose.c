/* This is script-generated code.  */ 
/* See ncClose.h for a description of this API call.*/

#include "ncClose.h"

int
rcNcClose (rcComm_t *conn, int ncid)
{
    int status;
    status = procApiRequest (conn, NC_CLOSE_AN, &ncid, NULL, 
        (void **) NULL, NULL);

    return (status);
}
