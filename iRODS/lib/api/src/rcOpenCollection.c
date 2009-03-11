/* This is script-generated code.  */ 
/* See openCollection.h for a description of this API call.*/

#include "openCollection.h"

int
rcOpenCollection (rcComm_t *conn, collInp_t *openCollInp)
{
    int status;
    status = procApiRequest (conn, OPEN_COLLECTION_AN, openCollInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}
