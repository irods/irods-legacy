/* This is script-generated code.  */ 
/* See dataObjOpenAndStat.h for a description of this API call.*/

#include "dataObjOpenAndStat.h"

int
rcDataObjOpenAndStat (rcComm_t *conn, dataObjInp_t *dataObjInp,
openStat_t **openStat)
{
    int status;
    status = procApiRequest (conn, DATA_OBJ_OPEN_AND_STAT_AN, dataObjInp, NULL,
        (void **) openStat, NULL);

    return (status);
}


