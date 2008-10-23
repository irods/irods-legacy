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
    rsComm->proxyUser.rodsZone, &rodsServerHost);
   if (status < 0) {
      return(status);
   }

   memset (&authCheckInp, 0, sizeof (authCheckInp)); 
   authCheckInp.challenge = bufp;
   authCheckInp.response = authResponseInp->response;
   authCheckInp.username = authResponseInp->username;
   authCheckInp.userZone = authResponseInp->userZone;

   if (rodsServerHost->localFlag == LOCAL_HOST) {
      status = rsAuthCheck (rsComm, &authCheckInp, &authCheckOut);
   } else {
      status = rcAuthCheck (rodsServerHost->conn, &authCheckInp, &authCheckOut);
      /* not likely we need this connection again */
      rcDisconnect(rodsServerHost->conn);
      rodsServerHost->conn = NULL;
   }
   if (status < 0) {
      rodsLog (LOG_NOTICE,
            "rsAuthResponse: rxAuthCheck failed, status = %d", status);
      return (status);
   }

   /* have to modify privLevel if the icat is a foreign icat because
    * a local user in a foreign zone is not a local user in this zone
    * and vice vera for a remote user
    */

    if (rodsServerHost->rcatEnabled == REMOTE_ICAT) {
	/* proxy is easy because rodsServerHost is based on proxy user */
        if (authCheckOut->privLevel == LOCAL_PRIV_USER_AUTH)
            authCheckOut->privLevel = REMOTE_PRIV_USER_AUTH;
        else if (authCheckOut->privLevel == LOCAL_PRIV_USER_AUTH)
            authCheckOut->privLevel = REMOTE_PRIV_USER_AUTH;

	/* adjust client user */
	if (strcmp (rsComm->proxyUser.userName, rsComm->clientUser.userName) 
	  == 0) {
            authCheckOut->clientPrivLevel = authCheckOut->privLevel;
	} else {
	    zoneInfo_t *tmpZoneInfo;
	    status = getLocalZoneInfo (&tmpZoneInfo);
            if (status < 0) {
                free (authCheckOut);
                return status;
            }

	    if (strcmp (tmpZoneInfo->zoneName, rsComm->clientUser.rodsZone)
	      == 0) {
		/* client is from local zone */
        	if (authCheckOut->clientPrivLevel == REMOTE_PRIV_USER_AUTH) {
                    authCheckOut->clientPrivLevel = LOCAL_PRIV_USER_AUTH;
		} else if (authCheckOut->clientPrivLevel == REMOTE_USER_AUTH) {
                    authCheckOut->clientPrivLevel = LOCAL_USER_AUTH;
		}
	    } else {
		/* client is from remote zone */
                if (authCheckOut->clientPrivLevel == LOCAL_PRIV_USER_AUTH) {
                    authCheckOut->clientPrivLevel = REMOTE_USER_AUTH;
                } else if (authCheckOut->clientPrivLevel == LOCAL_USER_AUTH) {
                    authCheckOut->clientPrivLevel = REMOTE_USER_AUTH;
                }
	    }
	}
   } else if (strcmp (rsComm->proxyUser.userName, rsComm->clientUser.userName)
    == 0) {
        authCheckOut->clientPrivLevel = authCheckOut->privLevel;
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

