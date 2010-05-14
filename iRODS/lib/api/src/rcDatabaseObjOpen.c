/* This is script-generated code.  */ 
/* See databaseObjOpen.h for a description of this API call.*/

#include "databaseObjOpen.h"

int
rcDatabaseObjOpen (rcComm_t *conn, 
		      databaseObjOpenInp_t *databaseObjOpenInp)
{
    int status;
    status = procApiRequest (conn, DATABASE_OBJ_OPEN_AN,  databaseObjOpenInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}
