/* This is script-generated code.  */ 
/* See dataObjTruncate.h for a description of this API call.*/

#include "dataObjTruncate.h"

int
rcDataObjTruncate (rcComm_t *conn, dataObjInp_t *dataObjInp)
{
    int status;
    status = procApiRequest (conn, DATA_OBJ_TRUNCATE_AN, dataObjInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}
