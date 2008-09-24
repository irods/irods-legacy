/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* See authResponse.h for a description of this API call.*/

#include "authRequest.h"
#include "authResponse.h"
#include "authCheck.h"

int
rsAuthResponse (rsComm_t *rsComm, authResponseInp_t *authResponseInp)
{
   int status;
   char *bufp;
   authCheckInp_t authCheckInp;
   authCheckOut_t *authCheckOut = NULL;
   rodsServerHost_t *rodsServerHost;

   bufp = _rsAuthRequestGetChallenge();

   /* need to do NoLogin because it could get into inf loop for cross 
    * zone auth */

   status = getAndConnRcatHostNoLogin (rsComm, SLAVE_RCAT, 
    rsComm->clientUser.rodsZone, &rodsServerHost);
   if (status < 0) {
      return(status);
   }

   memset (&authCheckInp, 0, sizeof (authCheckInp)); 
   authCheckInp.challenge = bufp;
   authCheckInp.response = authResponseInp->response;
   authCheckInp.username = authResponseInp->username;

   if (rodsServerHost->localFlag == LOCAL_HOST) {
      status = rsAuthCheck (rsComm, &authCheckInp, &authCheckOut);
   } else {
      status = rcAuthCheck (rodsServerHost->conn, &authCheckInp, &authCheckOut);
   }
   if (status < 0) {
      rodsLog (LOG_NOTICE,
            "rsAuthResponse: rxAuthCheck failed, status = %d", status);
      return (status);
   }

   status = chkProxyUserPriv (rsComm, authCheckOut->privLevel);

   if (status < 0) {
      free (authCheckOut);
      return status;
   } 

   rodsLog(LOG_NOTICE,
    "rsAuthResponse set proxy authFlag to %d, client authFlag to %d, user:%s proxy:%s client:%s",
          authCheckOut->privLevel,
          authCheckOut->clientPrivLevel,
          authCheckInp.username,
          rsComm->proxyUser.userName,
          rsComm->clientUser.userName);

   if (strcmp (rsComm->proxyUser.userName, rsComm->clientUser.userName) != 0) {
      rsComm->proxyUser.authInfo.authFlag = authCheckOut->privLevel;
      rsComm->clientUser.authInfo.authFlag = authCheckOut->clientPrivLevel;
   } else {	/* proxyUser and clientUser are the same */
      rsComm->proxyUser.authInfo.authFlag =
      rsComm->clientUser.authInfo.authFlag = authCheckOut->privLevel;
   } 

   free (authCheckOut);

   return (status);
} 

int
chkProxyUserPriv (rsComm_t *rsComm, int proxyUserPriv)
{
    if (strcmp (rsComm->proxyUser.userName, rsComm->clientUser.userName) 
      == 0) return 0;

    /* remote privileged user can only do things on behalf of users from
     * the same zone */
    if (proxyUserPriv >= LOCAL_PRIV_USER_AUTH ||
      (proxyUserPriv >= REMOTE_PRIV_USER_AUTH &&
      strcmp (rsComm->proxyUser.rodsZone,rsComm->clientUser.rodsZone) == 0)) {
	return 0;
    } else {
        rodsLog (LOG_ERROR,
         "rsAuthResponse: proxyuser %s with %d no priv to auth clientUser %s",
             rsComm->proxyUser.userName,
             proxyUserPriv,
             rsComm->clientUser.userName);
         return (SYS_PROXYUSER_NO_PRIV);
    }
}

