/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See userAdmin.h for a description of this API call.*/

#include "userAdmin.h"
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"

int
rsUserAdmin (rsComm_t *rsComm, userAdminInp_t *userAdminInp )
{
    rodsServerHost_t *rodsServerHost;
    int status;

    rodsLog(LOG_DEBUG, "userAdmin");

    status = getAndConnRcatHost(rsComm, MASTER_RCAT, NULL, &rodsServerHost);
    if (status < 0) {
       return(status);
    }

    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
       status = _rsUserAdmin (rsComm, userAdminInp);
#else
       status = SYS_NO_RCAT_SERVER_ERR;
#endif
    }
    else {
       status = rcUserAdmin(rodsServerHost->conn,
			       userAdminInp);
    }

    if (status < 0) { 
       rodsLog (LOG_NOTICE,
		"rsUserAdmin: rcUserAdmin failed");
    }
    return (status);
}

#ifdef RODS_CAT
int
_rsUserAdmin(rsComm_t *rsComm, userAdminInp_t *userAdminInp )
{
    int status;

    rodsLog (LOG_DEBUG,
	     "_rsUserAdmin arg0=%s", 
	     userAdminInp->arg0);

    if (strcmp(userAdminInp->arg0,"userpw")==0) {
       status = chlModUser(rsComm, 
			   userAdminInp->arg1,
			   userAdminInp->arg2,
			   userAdminInp->arg3);
       if (status != 0) chlRollback(rsComm);
       return(status);
    }

    return(CAT_INVALID_ARGUMENT);
} 
#endif
