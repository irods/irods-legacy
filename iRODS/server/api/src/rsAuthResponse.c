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

   status = getAndConnRcatHostNoLogin (rsComm, MASTER_RCAT, NULL,
                                &rodsServerHost);
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

   /* XXXXX will need to change logic a bit for remote zone users */
   if (strcmp (rsComm->proxyUser.userName, rsComm->clientUser.userName) != 0) {
      if (authCheckOut->privLevel < LOCAL_PRIV_USER_AUTH) {
         rodsLog (LOG_ERROR, 
          "rsAuthResponse: proxyuser %s with %d no priv to auth clientUser %s",
             rsComm->proxyUser.userName, 
             authCheckOut->privLevel, 
             rsComm->clientUser.userName); 
         free (authCheckOut);
         return (SYS_PROXYUSER_NO_PRIV);
      } else {
         rsComm->proxyUser.authInfo.authFlag = authCheckOut->privLevel;
         rsComm->clientUser.authInfo.authFlag = authCheckOut->clientPrivLevel;
         rodsLog(LOG_NOTICE,
          "rsAuthResponse set proxy authFlag to %d, client authFlag to %d, user:%s proxy:%s client:%s",
              authCheckOut->privLevel,
              authCheckOut->clientPrivLevel,
              authCheckInp.username,
              rsComm->proxyUser.userName,
              rsComm->clientUser.userName);
      }
   } else {	/* proxyUser and clientUser are the same */
      rsComm->proxyUser.authInfo.authFlag = 
      rsComm->clientUser.authInfo.authFlag = authCheckOut->privLevel;
      rodsLog(LOG_NOTICE,
       "rsAuthResponse set proxy and client authFlag to %d, user:%s proxy:%s client:%s",
           authCheckOut->privLevel,
           authCheckInp.username,
           rsComm->proxyUser.userName,
           rsComm->clientUser.userName);
   }

   free (authCheckOut);

   return (status);
} 

