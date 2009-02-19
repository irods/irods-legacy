/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsFileSyncToArch.c - server routine that handles the fileSyncToArch
 * API
 */

/* script generated code */
#include "fileSyncToArch.h"
#include "fileOpr.h"
#include "miscServerFunct.h"

int
rsFileSyncToArch (rsComm_t *rsComm, fileStageSyncInp_t *fileSyncToArchInp)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    remoteFlag = resolveHost (&fileSyncToArchInp->addr, &rodsServerHost);

    if (remoteFlag < 0) {
	return (remoteFlag);
    } else {
	status = rsFileSyncToArchByHost (rsComm, fileSyncToArchInp, 
	  rodsServerHost);
	return (status);
    }
}

int 
rsFileSyncToArchByHost (rsComm_t *rsComm, 
fileStageSyncInp_t *fileSyncToArchInp, rodsServerHost_t *rodsServerHost)
{
    int status;
    int remoteFlag;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
         "rsFileSyncToArchByHost: Input NULL rodsServerHost");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
	    
    remoteFlag = rodsServerHost->localFlag;
    
    if (remoteFlag == LOCAL_HOST) {
	status = _rsFileSyncToArch (rsComm, fileSyncToArchInp);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteFileSyncToArch (rsComm, fileSyncToArchInp, 
	  rodsServerHost);
    } else {
	if (remoteFlag < 0) {
	    return (remoteFlag);
	} else {
	    rodsLog (LOG_NOTICE,
	      "rsFileSyncToArchByHost: resolveHost returned value %d",
	       remoteFlag);
	    return (SYS_UNRECOGNIZED_REMOTE_FLAG);
	}
    }

    return (status);
}

int
remoteFileSyncToArch (rsComm_t *rsComm, 
fileStageSyncInp_t *fileSyncToArchInp, rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
	  "remoteFileSyncToArch: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcFileSyncToArch (rodsServerHost->conn, fileSyncToArchInp);

    if (status < 0) { 
        rodsLog (LOG_NOTICE,
	 "remoteFileSyncToArch: rcFileSyncToArch failed for %s",
	  fileSyncToArchInp->filename);
    }

    return status;
}

/* _rsFileSyncToArch - this the local version of rsFileSyncToArch.
 */

int
_rsFileSyncToArch (rsComm_t *rsComm, fileStageSyncInp_t *fileSyncToArchInp)
{
    int status;

    /* XXXX need to check resource permission and vault permission
     * when RCAT is available 
     */

    status = fileSyncToArch (fileSyncToArchInp->fileType, rsComm, 
      fileSyncToArchInp->filename, fileSyncToArchInp->cacheFilename, 
      fileSyncToArchInp->optionalInfo);

    if (status < 0) {
	rodsLog (LOG_NOTICE, 
	  "_rsFileSyncToArch: fileSyncToArch for %s, status = %d",
	  fileSyncToArchInp->filename, status);
        return (status);
    }

    return (status);
} 
 
