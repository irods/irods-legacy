/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See getHostForPut.h for a description of this API call.*/

#include "getHostForPut.h"
#include "rodsLog.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "getRemoteZoneResc.h"
#include "dataObjCreate.h"
#include "objMetaOpr.h"

int
rsGetHostForPut (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
char **outHost)
{
    int status;
    rescGrpInfo_t *myRescGrpInfo;
    rescInfo_t *myRescInfo;

    *outHost = NULL;

    if (isLocalZone (dataObjInp->objPath) == 0) {
	/* it is a remote zone. better connect to this host */
	*outHost = strdup (THIS_ADDRESS);
	return 0;
    }

    if (getValByKey (&dataObjInp->condInput, ALL_KW) != NULL ||
      getValByKey (&dataObjInp->condInput, FORCE_FLAG_KW) != NULL) {
	/* going to ALL copies or overwriting files. not sure which is the 
         * best */ 
        *outHost = strdup (THIS_ADDRESS);
        return 0;
    }
    status = getRescGrpForCreate (rsComm, dataObjInp, &myRescGrpInfo);
    if (status < 0) return status;

    myRescInfo = myRescGrpInfo->rescInfo;
    /* status == 1 means random sorting scheme */
    if ((status == 1 && getRescCnt (myRescGrpInfo) > 1) || 
      getRescClass (myRescInfo) == COMPOUND_CL) {
	freeAllRescGrpInfo (myRescGrpInfo);
        *outHost = strdup (THIS_ADDRESS);
    } else {
	rodsServerHost_t *rodsServerHost;
        rodsHostAddr_t addr;
        bzero (&addr, sizeof (addr));
        rstrcpy (addr.hostAddr, myRescInfo->rescLoc, NAME_LEN);
	status = resolveHost (&addr, &rodsServerHost);
	if (status < 0) {
	    freeAllRescGrpInfo (myRescGrpInfo);
	    return status;
	}
	*outHost = strdup (rodsServerHost->hostName->name);
	freeAllRescGrpInfo (myRescGrpInfo);
    }
    return 0;
}

