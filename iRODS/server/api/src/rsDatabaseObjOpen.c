/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See databaseObjOpen.h for a description of this API call.*/

#include "databaseObjOpen.h"
#include "objMetaOpr.h"
#include "miscServerFunct.h"
#include "dboHighLevelRoutines.h"
int
remoteDatabaseObjOpen(rsComm_t *rsComm,
		      databaseObjOpenInp_t *databaseObjOpenInp,
		      rodsServerHost_t *rodsServerHost) {
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remoteDatabaseOpen: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcDatabaseObjOpen(rodsServerHost->conn,
			       databaseObjOpenInp);
    return status;
}

int
rsDatabaseObjOpen (rsComm_t *rsComm, databaseObjOpenInp_t *databaseObjOpenInp)
{
    rodsServerHost_t *rodsServerHost;
    int status;
    int remoteFlag;
    rodsHostAddr_t rescAddr;
    rescGrpInfo_t *rescGrpInfo = NULL;

    status = _getRescInfo (rsComm, databaseObjOpenInp->dbrName, &rescGrpInfo);
    if (status < 0) {
	 rodsLog (LOG_ERROR,
		  "rsDatabaseObjOpen: _getRescInfo of %s error, stat = %d",
		  databaseObjOpenInp->dbrName, status);
	return status;
    }

    bzero (&rescAddr, sizeof (rescAddr));
    rstrcpy (rescAddr.hostAddr, rescGrpInfo->rescInfo->rescLoc, NAME_LEN);
    remoteFlag = resolveHost (&rescAddr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
       status = _rsDatabaseObjOpen(rsComm, databaseObjOpenInp);
    } else if (remoteFlag == REMOTE_HOST) {
       status = remoteDatabaseObjOpen(rsComm,
				      databaseObjOpenInp, 
				      rodsServerHost );
    } else if (remoteFlag < 0) {
       status = remoteFlag;
    }

    if (status < 0 ) { 
        rodsLog (LOG_NOTICE,
		 "rsDatabaseObjOpen: rcDatabaseObjOpen failed, status = %d",
		 status);
    }
    return (status);
}

int
_rsDatabaseObjOpen (rsComm_t *rsComm, databaseObjOpenInp_t *databaseObjOpenInp)
{

#ifdef DBO
    int status;
    status = dboOpen(databaseObjOpenInp->dboName);
    if (status < 0 ) { 
       rodsLog (LOG_NOTICE, 
		"_rsDatabaseObjOpen: databaseObjOpen for %s, status = %d",
		databaseObjOpenInp->dboName, status);
    }
    return (status);
#else
    return(DBO_NOT_COMPILED_IN);
#endif
} 

