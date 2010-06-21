/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* See databaseObjectAdmin.h for a description of this API call.*/

#include "databaseObjectAdmin.h"
#include "objMetaOpr.h"
#include "miscServerFunct.h"
#include "icatHighLevelRoutines.h"

int
rsDatabaseObjectAdmin (rsComm_t *rsComm, databaseObjectAdminInp_t *databaseObjectAdminInp,
		      databaseObjectAdminOut_t **databaseObjectAdminOut)
{
    rodsServerHost_t *rodsServerHost;
    int status;

    status = getAndConnRcatHost(rsComm, MASTER_RCAT, NULL, &rodsServerHost);
    if (status < 0) {
       return(status);
    }

    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
       status = _rsDatabaseObjectAdmin (rsComm, databaseObjectAdminInp,
				   databaseObjectAdminOut);
#else
       status = SYS_NO_RCAT_SERVER_ERR;
#endif
    } 
    else {
       status = rcDatabaseObjectAdmin(rodsServerHost->conn, 
				      databaseObjectAdminInp,
				      databaseObjectAdminOut);
    }
    if (status < 0 ) { 
        rodsLog (LOG_NOTICE,
		 "rsDatabaseObjectAdmin: rcDatabaseObjectAdmin failed, status = %d",
		 status);
    }
    return (status);
}

#ifdef RODS_CAT
int
_rsDatabaseObjectAdmin (rsComm_t *rsComm, 
			databaseObjectAdminInp_t *databaseObjectAdminInp,
			databaseObjectAdminOut_t **databaseObjectAdminOut) {
    int status;
    int t1, t2;
    char *blank2;
    char *blank3;
    char *blank4;
    char *blank5;
    char *blank6;

    blank2 = malloc(2);
    *blank2='\0';
    blank3 = malloc(2);
    *blank3='\0';
    blank4 = malloc(2);
    *blank4='\0';
    blank5 = malloc(2);
    *blank5='\0';
    blank6 = malloc(2);
    *blank6='\0';

    databaseObjectAdminOut_t *myObjectAdminOut;

    myObjectAdminOut = (databaseObjectAdminOut_t *)malloc(sizeof(databaseObjectAdminOut_t));

    t1 = sizeof(databaseObjectAdminOut_t);
    printf("t1=%d\n",t1);

    t2 = sizeof(myObjectAdminOut);
    printf("t1=%d\n",t2);

    *databaseObjectAdminOut = myObjectAdminOut;
    myObjectAdminOut->dbrName=blank2;
    myObjectAdminOut->dboName=blank3;
    myObjectAdminOut->sqlId=0;
    myObjectAdminOut->sql=blank4;
    myObjectAdminOut->defaultParameters=blank5;
    myObjectAdminOut->description=blank6;

    status = chlDatabaseObjectAdmin(rsComm, 
			     databaseObjectAdminInp,
			     myObjectAdminOut);

    if (status < 0 ) { 
       rodsLog (LOG_NOTICE, 
		"_rsDatabaseObjectAdmin: databaseObjectAdmin status = %d",
		status);
       return (status);
    }
    return (status);
}
#endif
