/* This is script-generated code.  */ 
/* See regDataObj.h for a description of this API call.*/

#include "regDataObj.h"

int
rcRegDataObj (rcComm_t *conn, dataObjInfo_t *dataObjInfo, 
dataObjInfo_t **outDataObjInfo)
{
    int status;
    status = procApiRequest (conn, REG_DATA_OBJ_AN, dataObjInfo, NULL, 
        (void **) outDataObjInfo, NULL);

    return (status);
}
