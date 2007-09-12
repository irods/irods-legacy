/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See fileRmdir.h for a description of this API call.*/

#include "fileRmdir.h"
#include "miscServerFunct.h"

int
rsFileRmdir (rsComm_t *rsComm, fileRmdirInp_t *fileRmdirInp)
{
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    int status;

    remoteFlag = resolveHost (&fileRmdirInp->addr, &rodsServerHost);
    if (remoteFlag == LOCAL_HOST) {
        status = _rsFileRmdir (rsComm, fileRmdirInp);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remoteFileRmdir (rsComm, fileRmdirInp, rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_NOTICE,
              "rsFileRmdir: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    /* Manually insert call-specific code here */

    return (status);
}

int
remoteFileRmdir (rsComm_t *rsComm, fileRmdirInp_t *fileRmdirInp,
rodsServerHost_t *rodsServerHost)
{    
    int status;

        if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteFileRmdir: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }


    status = rcFileRmdir (rodsServerHost->conn, fileRmdirInp);

    if (status < 0) { 
        rodsLog (LOG_NOTICE,
         "remoteFileOpen: rcFileRmdir failed for %s",
          fileRmdirInp->dirName);
    }

    return status;
}

int
_rsFileRmdir (rsComm_t *rsComm, fileRmdirInp_t *fileRmdirInp)
{
    int status;

    status = fileRmdir (fileRmdirInp->fileType, rsComm, fileRmdirInp->dirName);

    if (status < 0) {
        rodsLog (LOG_NOTICE, 
          "_rsFileRmdir: fileRmdir for %s, status = %d",
          fileRmdirInp->dirName, status);
        return (status);
    }

    return (status);
} 
