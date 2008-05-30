/* This is script-generated code.  */ 
/* See closeCollection.h for a description of this API call.*/

#include "closeCollection.h"


int
rcCloseCollection (rcComm_t *conn, int handleInxInp)
{
    int status;
    status = procApiRequest (conn, CLOSE_COLLECTION_AN, &handleInxInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}
