/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* See pamAuthRequest.h for a description of this API call.*/

#include "pamAuthRequest.h"
#include "genQuery.h"
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"



int
rsPamAuthRequest (rsComm_t *rsComm, pamAuthRequestInp_t *pamAuthRequestInp,
		  pamAuthRequestOut_t **pamAuthRequestOut)
{
    rodsServerHost_t *rodsServerHost;
    int status;

    status = getAndConnRcatHost(rsComm, MASTER_RCAT, NULL, &rodsServerHost);
    if (status < 0) {
       return(status);
    }
    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
       status = _rsPamAuthRequest(rsComm,  pamAuthRequestInp,
				  pamAuthRequestOut);

#else
       status = SYS_NO_RCAT_SERVER_ERR;
#endif
    }
    else {
       status = rcPamAuthRequest(rodsServerHost->conn, pamAuthRequestInp,
				 pamAuthRequestOut);
    }
    return (status);
}

int
_rsPamAuthRequest (rsComm_t *rsComm, pamAuthRequestInp_t *pamAuthRequestInp,
		   pamAuthRequestOut_t **pamAuthRequestOut) {
    int status;
    pamAuthRequestOut_t *result;

    *pamAuthRequestOut = (pamAuthRequestOut_t *)
       malloc(sizeof(pamAuthRequestOut_t));
    memset((char *)*pamAuthRequestOut, 0, sizeof(pamAuthRequestOut_t));

    result = *pamAuthRequestOut;

#if defined(PAM_AUTH)

/* //  Need to actually run the PAM auth code here and only call
   chlUpdateIrodsPamPassword if the password/user is valid */

/* For testing, comment out the line below but this will allow
   all PAM auth attempts to succeed */
    status = PAM_AUTH_NOT_BUILT_INTO_SERVER;
    if (status) return(status);


    result->irodsPamPassword = (char*)malloc(100);
    if (result->irodsPamPassword == 0) return (SYS_MALLOC_ERR);
    status = chlUpdateIrodsPamPassword(rsComm, 
				       pamAuthRequestInp->pamUser, 
				       &result->irodsPamPassword);

    return(status);
#else
    status = PAM_AUTH_NOT_BUILT_INTO_SERVER;
    return (status);
#endif
} 
