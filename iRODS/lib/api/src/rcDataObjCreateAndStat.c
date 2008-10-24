/* This is script-generated code.  */ 
/* See dataObjCreateAndStat.h for a description of this API call.*/

#include "dataObjCreateAndStat.h"

int
rcDataObjCreateAndStat (rcComm_t *conn, dataObjInp_t *dataObjInp,
openStat_t **openStat)
{
    int status;
    status = procApiRequest (conn, DATA_OBJ_CREATE_AND_STAT_AN, dataObjInp, NULL,
        (void **) openStat, NULL);

    return (status);
}


