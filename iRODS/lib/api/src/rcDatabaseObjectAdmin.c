/* This is script-generated code.  */ 
/* See databaseObjectAdmin.h for a description of this API call.*/

#include "databaseObjectAdmin.h"

int
rcDatabaseObjectAdmin (rcComm_t *conn, 
		      databaseObjectAdminInp_t *databaseObjectAdminInp, 
		      databaseObjectAdminOut_t **databaseObjectAdminOut)
{
    int status;
    status = procApiRequest (conn, DATABASE_OBJECT_ADMIN_AN,  databaseObjectAdminInp, NULL, 
        (void **) databaseObjectAdminOut, NULL);

    return (status);
}
