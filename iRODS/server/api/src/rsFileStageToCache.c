/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsFileStageToCache.c - server routine that handles the fileStageToCache
 * API
 */

/* script generated code */
#include "fileStageToCache.h"
#include "fileOpr.h"
#include "miscServerFunct.h"

int
rsFileStageToCache (rsComm_t *rsComm, fileStageSyncInp_t *fileStageToCacheInp)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    remoteFlag = resolveHost (&fileStageToCacheInp->addr, &rodsServerHost);

    if (remoteFlag < 0) {
	return (remoteFlag);
    } else {
	status = rsFileStageToCacheByHost (rsComm, fileStageToCacheInp, 
	  rodsServerHost);
	return (status);
    }
}

int 
rsFileStageToCacheByHost (rsComm_t *rsComm, 
fileStageSyncInp_t *fileStageToCacheInp, rodsServerHost_t *rodsServerHost)
{
    int status;
    int remoteFlag;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
         "rsFileStageToCacheByHost: Input NULL rodsServerHost");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
	    
    remoteFlag = rodsServerHost->localFlag;
    
    if (remoteFlag == LOCAL_HOST) {
	status = _rsFileStageToCache (rsComm, fileStageToCacheInp);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteFileStageToCache (rsComm, fileStageToCacheInp, 
	  rodsServerHost);
    } else {
	if (remoteFlag < 0) {
	    return (remoteFlag);
	} else {
	    rodsLog (LOG_NOTICE,
	      "rsFileStageToCacheByHost: resolveHost returned value %d",
	       remoteFlag);
	    return (SYS_UNRECOGNIZED_REMOTE_FLAG);
	}
    }

    return (status);
}

int
remoteFileStageToCache (rsComm_t *rsComm, 
fileStageSyncInp_t *fileStageToCacheInp, rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
	  "remoteFileStageToCache: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcFileStageToCache (rodsServerHost->conn, fileStageToCacheInp);

    if (status < 0) { 
        rodsLog (LOG_NOTICE,
	 "remoteFileStageToCache: rcFileStageToCache failed for %s",
	  fileStageToCacheInp->filename);
    }

    return status;
}

/* _rsFileStageToCache - this the local version of rsFileStageToCache.
 */

int
_rsFileStageToCache (rsComm_t *rsComm, fileStageSyncInp_t *fileStageToCacheInp)
{
    int status;

    /* XXXX need to check resource permission and vault permission
     * when RCAT is available 
     */

    status = fileStageToCache (fileStageToCacheInp->fileType, rsComm, 
      fileStageToCacheInp->filename, fileStageToCacheInp->cacheFilename, 
      fileStageToCacheInp->optionalInfo);

    if (status < 0) {
	rodsLog (LOG_NOTICE, 
	  "_rsFileStageToCache: fileStageToCache for %s, status = %d",
	  fileStageToCacheInp->filename, status);
        return (status);
    }

    return (status);
} 
 
