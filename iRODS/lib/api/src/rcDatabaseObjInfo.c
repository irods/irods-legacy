/* This is script-generated code.  */ 
/* See databaseObjInfo.h for a description of this API call.*/

#include "databaseObjInfo.h"

int
rcDatabaseObjInfo (rcComm_t *conn, 
		      databaseObjInfoInp_t *databaseObjInfoInp, 
		      databaseObjInfoOut_t **databaseObjInfoOut)
{
    int status;
    status = procApiRequest (conn, DATABASE_OBJ_INFO_AN,  databaseObjInfoInp, NULL, 
        (void **) databaseObjInfoOut, NULL);

    return (status);
}
