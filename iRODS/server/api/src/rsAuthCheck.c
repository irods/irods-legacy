/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* See authCheck.h for a description of this API call.*/

#include "authRequest.h"
#include "authCheck.h"
#include "icatHighLevelRoutines.h"

int
rsAuthCheck (rsComm_t *rsComm, authCheckInp_t *authCheckInp, 
		authCheckOut_t **authCheckOut)
{
#ifdef RODS_CAT
   int status;
   int privLevel;
   int clientPrivLevel;
   authCheckOut_t *result;

   *authCheckOut = malloc(sizeof(authCheckOut_t));
   memset((char *)*authCheckOut, 0, sizeof(authCheckOut_t));

   rodsLog(LOG_NOTICE, "rsAuthCheck user %s zone %s", authCheckInp->username,
	   authCheckInp->userZone);
   status = chlCheckAuth(rsComm, authCheckInp->challenge, 
			 authCheckInp->response, 
			 authCheckInp->username, authCheckInp->userZone, 
			 &privLevel, &clientPrivLevel);
   if (status < 0) {
      rodsLog (LOG_NOTICE, 
	       "_rsAuthCheck: chlCheckAuth status = %d", status);
   }
   if (status == 0) {
      result = *authCheckOut;
      result->privLevel = privLevel;
      result->clientPrivLevel = clientPrivLevel;
   }

   return (status);
#else
   /* this call should never be made to an intermediate server (at
      least I think that's true) */
   return ( SYS_NO_ICAT_SERVER_ERR);
#endif
} 

