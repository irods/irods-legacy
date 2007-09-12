/* This is script-generated code.  */ 
/* See modDataObjMeta.h for a description of this API call.*/

#include "modDataObjMeta.h"

int
rcModDataObjMeta (rcComm_t *conn, modDataObjMeta_t *modDataObjMetaInp)
{
    int status;
    status = procApiRequest (conn, MOD_DATA_OBJ_META_AN, modDataObjMetaInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}
