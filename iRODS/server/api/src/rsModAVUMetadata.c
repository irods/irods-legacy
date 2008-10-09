/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See modAVUMetadata.h for a description of this API call.*/

#include "modAVUMetadata.h"
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"

int
rsModAVUMetadata (rsComm_t *rsComm, modAVUMetadataInp_t *modAVUMetadataInp )
{
    rodsServerHost_t *rodsServerHost;
    int status;
    char *myHint;

    if (strcmp(modAVUMetadataInp->arg0,"add")==0) {
	myHint = modAVUMetadataInp->arg2;
    } else if (strcmp(modAVUMetadataInp->arg0,"adda")==0) {
        myHint = modAVUMetadataInp->arg2;
    } else if (strcmp(modAVUMetadataInp->arg0,"rmw")==0) {
        myHint = modAVUMetadataInp->arg2;
    } else if (strcmp(modAVUMetadataInp->arg0,"rmi")==0) {
        myHint = modAVUMetadataInp->arg2;
    } else if (strcmp(modAVUMetadataInp->arg0,"rm")==0) {
        myHint = modAVUMetadataInp->arg2;
    } else if (strcmp(modAVUMetadataInp->arg0,"cp")==0) {
        myHint = modAVUMetadataInp->arg3;
    } else {
	/* assume local */
	myHint = NULL;
    }
 
    status = getAndConnRcatHost(rsComm, MASTER_RCAT, myHint, &rodsServerHost);
    if (status < 0) {
       return(status);
    }

    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
       status = _rsModAVUMetadata (rsComm, modAVUMetadataInp);
#else
       status = SYS_NO_RCAT_SERVER_ERR;
#endif
    }
    else {
       status = rcModAVUMetadata(rodsServerHost->conn,
			       modAVUMetadataInp);
    }

    if (status < 0) { 
       rodsLog (LOG_NOTICE,
		"rsModAVUMetadata: rcModAVUMetadata failed");
    }
    return (status);
}

#ifdef RODS_CAT
int
_rsModAVUMetadata (rsComm_t *rsComm, modAVUMetadataInp_t *modAVUMetadataInp )
{
    int status;

    if (strcmp(modAVUMetadataInp->arg0,"add")==0) {
       status = chlAddAVUMetadata(rsComm, 0,
				  modAVUMetadataInp->arg1,
				  modAVUMetadataInp->arg2,
				  modAVUMetadataInp->arg3,
				  modAVUMetadataInp->arg4,
				  modAVUMetadataInp->arg5);
       return(status);
    }
    if (strcmp(modAVUMetadataInp->arg0,"adda")==0) {
       status = chlAddAVUMetadata(rsComm, 1,
				  modAVUMetadataInp->arg1,
				  modAVUMetadataInp->arg2,
				  modAVUMetadataInp->arg3,
				  modAVUMetadataInp->arg4,
				  modAVUMetadataInp->arg5);
       return(status);
    }
    if (strcmp(modAVUMetadataInp->arg0,"rmw")==0) {
       status = chlDeleteAVUMetadata(rsComm, 1,
				  modAVUMetadataInp->arg1,
				  modAVUMetadataInp->arg2,
				  modAVUMetadataInp->arg3,
				  modAVUMetadataInp->arg4,
				  modAVUMetadataInp->arg5);
       return(status);
    }
    if (strcmp(modAVUMetadataInp->arg0,"rmi")==0) {
       status = chlDeleteAVUMetadata(rsComm, 2,
				  modAVUMetadataInp->arg1,
				  modAVUMetadataInp->arg2,
				  modAVUMetadataInp->arg3,
				  modAVUMetadataInp->arg4,
				  modAVUMetadataInp->arg5);
       return(status);
    }
    if (strcmp(modAVUMetadataInp->arg0,"rm")==0) {
       status = chlDeleteAVUMetadata(rsComm, 0,
				  modAVUMetadataInp->arg1,
				  modAVUMetadataInp->arg2,
				  modAVUMetadataInp->arg3,
				  modAVUMetadataInp->arg4,
				  modAVUMetadataInp->arg5);
       return(status);
    }
    if (strcmp(modAVUMetadataInp->arg0,"cp")==0) {
       status = chlCopyAVUMetadata(rsComm, 
				  modAVUMetadataInp->arg1,
				  modAVUMetadataInp->arg2,
				  modAVUMetadataInp->arg3,
				  modAVUMetadataInp->arg4);
       return(status);
    }

    return(CAT_INVALID_ARGUMENT);

} 
#endif
