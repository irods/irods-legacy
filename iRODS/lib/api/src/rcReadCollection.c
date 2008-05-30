/* This is script-generated code.  */ 
/* See readCollection.h for a description of this API call.*/

#include "readCollection.h"


int
rcReadCollection (rcComm_t *conn, int handleInxInp,
collEnt_t **collEnt)
{
    int status;
    status = procApiRequest (conn, READ_COLLECTION_AN, &handleInxInp, NULL, 
        (void **) collEnt, NULL);

    return (status);
}
