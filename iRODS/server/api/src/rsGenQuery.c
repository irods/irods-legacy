/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* See genQuery.h for a description of this API call.*/

#include "genQuery.h"
#include "icatHighLevelRoutines.h"

/* can be used for debug: */
/* extern int printGenQI( genQueryInp_t *genQueryInp); */

int
rsGenQuery (rsComm_t *rsComm, genQueryInp_t *genQueryInp, 
genQueryOut_t **genQueryOut)
{
    rodsServerHost_t *rodsServerHost;
    int status;

    status = getAndConnRcatHost(rsComm, MASTER_RCAT, NULL,
				&rodsServerHost);
    if (status < 0) {
       return(status);
    }

    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
       status = _rsGenQuery (rsComm, genQueryInp, genQueryOut);
#else
       rodsLog(LOG_NOTICE, 
	       "rsGenQuery error. RCAT is not configured on this host");
       return (SYS_NO_RCAT_SERVER_ERR);
#endif 
    } else {
        status = rcGenQuery(rodsServerHost->conn,
			   genQueryInp, genQueryOut);
    }
    if (status < 0  && status != CAT_NO_ROWS_FOUND) {
        rodsLog (LOG_NOTICE,
		 "rsGenQuery: rcGenQuery failed, status = %d", status);
    }
    return (status);
}

#ifdef RODS_CAT
int
_rsGenQuery (rsComm_t *rsComm, genQueryInp_t *genQueryInp,
	     genQueryOut_t **genQueryOut)
{
    int status;

    /*  printGenQI(genQueryInp);  for debug */

    *genQueryOut = malloc(sizeof(genQueryOut_t));
    memset((char *)*genQueryOut, 0, sizeof(genQueryOut_t));

#ifdef GEN_QUERY_AC
    chlGenQueryAccessControlSetup(rsComm->clientUser.userName, 
			      rsComm->clientUser.rodsZone,
			      rsComm->proxyUser.authInfo.authFlag);
#endif

    status = chlGenQuery(*genQueryInp, *genQueryOut);

    if (status < 0) {
       free (*genQueryOut);
       *genQueryOut = NULL;
       if (status != CAT_NO_ROWS_FOUND) {
	  rodsLog (LOG_NOTICE, 
		   "_rsGenQuery: genQuery status = %d", status);
       }
       return (status);
    }
    return (status);
} 
#endif
