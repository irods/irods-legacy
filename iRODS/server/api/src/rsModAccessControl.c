/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See modAccessControl.h for a description of this API call.*/

#include "modAccessControl.h"
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"

int
rsModAccessControl (rsComm_t *rsComm, modAccessControlInp_t *modAccessControlInp )
{
    rodsServerHost_t *rodsServerHost;
    int status;

    status = getAndConnRcatHost(rsComm, MASTER_RCAT, 
      modAccessControlInp->path, &rodsServerHost);
    if (status < 0) {
       return(status);
    }

    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
       status = _rsModAccessControl (rsComm, modAccessControlInp);
#else
       status = SYS_NO_RCAT_SERVER_ERR;
#endif
    }
    else {
       status = rcModAccessControl(rodsServerHost->conn,
			       modAccessControlInp);
    }

    if (status < 0) { 
       rodsLog (LOG_NOTICE,
		"rsModAccessControl: rcModAccessControl failed");
    }
    return (status);
}

#ifdef RODS_CAT
int
_rsModAccessControl (rsComm_t *rsComm, 
		     modAccessControlInp_t *modAccessControlInp )
{
    int status;

    status = chlModAccessControl(rsComm, 
				 modAccessControlInp->recursiveFlag,
				 modAccessControlInp->accessLevel,
				 modAccessControlInp->userName,
				 modAccessControlInp->zone,
				 modAccessControlInp->path );
    return(status);
} 
#endif
