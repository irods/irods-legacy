/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See databaseObjOpen.h for a description of this API call.*/

#include "databaseObjOpen.h"
#include "dboHighLevelRoutines.h"

int
rsDatabaseObjOpen (rsComm_t *rsComm, databaseObjOpenInp_t *databaseObjOpenInp)
{
    rodsServerHost_t *rodsServerHost;
    int status;

/* Will need to change this to connect to the database-resource host, 
   once those are defined.
*/
    status = getAndConnRcatHost(rsComm, MASTER_RCAT, NULL, &rodsServerHost);
    if (status < 0) {
       return(status);
    }

    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef DBO
       status = _rsDatabaseObjOpen (rsComm, databaseObjOpenInp);
#else
       status = SYS_NO_RCAT_SERVER_ERR;
#endif
    } 
    else {
       status = rcDatabaseObjOpen(rodsServerHost->conn,
			   databaseObjOpenInp);
    }

    if (status < 0 && status != CAT_NO_ROWS_FOUND) { 
        rodsLog (LOG_NOTICE,
		 "rsDatabaseObjOpen: rcDatabaseObjOpen failed, status = %d", status);
    }
    return (status);
}

#ifdef DBO
int
_rsDatabaseObjOpen (rsComm_t *rsComm, databaseObjOpenInp_t *databaseObjOpenInp)
{
    int status;

    status = dboOpen(databaseObjOpenInp->dboName);
    if (status < 0 ) { 
       rodsLog (LOG_NOTICE, 
		"_rsDatabaseObjOpen: databaseObjOpen for %s, status = %d",
		databaseObjOpenInp->dboName, status);
    }
    return (status);
} 
#endif
