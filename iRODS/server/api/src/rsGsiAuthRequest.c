/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* See gsiAuthRequest.h for a description of this API call.*/

#include "gsiAuthRequest.h"
#include "genQuery.h"

static int gsiAuthReqStatus=0;
static int gsiAuthReqError=0;
static char gsiAuthReqErrorMsg[1000];

int
rsGsiAuthRequest (rsComm_t *rsComm, gsiAuthRequestOut_t **gsiAuthRequestOut)
{
    gsiAuthRequestOut_t *result;
    int status;

    if (gsiAuthReqStatus==1) {
       gsiAuthReqStatus=0;
       if (gsiAuthReqError != 0) {
	  rodsLogAndErrorMsg( LOG_NOTICE, &rsComm->rError, gsiAuthReqError,
			      gsiAuthReqErrorMsg);
       }
       return gsiAuthReqError;
    }

    *gsiAuthRequestOut = malloc(sizeof(gsiAuthRequestOut_t));
    memset((char *)*gsiAuthRequestOut, 0, sizeof(gsiAuthRequestOut_t));

    result = *gsiAuthRequestOut;

#if defined(GSI_AUTH)
    status = igsiSetupCreds(NULL, rsComm, NULL, &result->serverDN);
    if (status==0) {
        rsComm->gsiRequest=1;
    }
    return(status);
#else
    status = GSI_NOT_BUILT_INTO_SERVER;
    rodsLog (LOG_ERROR,
	    "rsGsiAuthRequest failed GSI_NOT_BUILT_INTO_SERVER, status = %d",
	     status);
    return (status);
#endif

} 

int igsiServersideAuth(rsComm_t *rsComm) {
   int status;
#if defined(GSI_AUTH)
   char clientName[500];
   genQueryInp_t genQueryInp;
   genQueryOut_t *genQueryOut;
   char condition1[MAX_NAME_LEN];
   char condition2[MAX_NAME_LEN];
   char *tResult;
   int privLevel;
   int clientPrivLevel;

#ifdef GSI_DEBUG
   char *getVar;
   getVar = getenv("X509_CERT_DIR");
   if (getVar != NULL) {
      printf("X509_CERT_DIR:%s\n",getVar);
   }
#endif

   gsiAuthReqStatus=1;

   status = igsiEstablishContextServerside(rsComm, clientName, 
					    500);
#ifdef GSI_DEBUG
   if (status==0) printf("clientName:%s\n",clientName);
#endif

   memset (&genQueryInp, 0, sizeof (genQueryInp_t));

   snprintf (condition1, MAX_NAME_LEN, "='%s'", clientName);
   addInxVal (&genQueryInp.sqlCondInp, COL_USER_DN, condition1);

   snprintf (condition2, MAX_NAME_LEN, "='%s'", rsComm->clientUser.userName);
   addInxVal (&genQueryInp.sqlCondInp, COL_USER_NAME, condition2);

   addInxIval (&genQueryInp.selectInp, COL_USER_ID, 1);
   addInxIval (&genQueryInp.selectInp, COL_USER_TYPE, 1);

   genQueryInp.maxRows = 2;

   status = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

   if (status == CAT_NO_ROWS_FOUND || genQueryOut==NULL) {
      status = GSI_DN_DOES_NOT_MATCH_USER;
      rodsLog (LOG_NOTICE,
	       "igsiServersideAuth: DN mismatch, user=%s, Certificate DN=%s, status=%d",
	       rsComm->clientUser.userName,
	       clientName,
	       status);
      snprintf(gsiAuthReqErrorMsg, 1000, 
	       "igsiServersideAuth: DN mismatch, user=%s, Certificate DN=%s, status=%d",
	       rsComm->clientUser.userName,
	       clientName,
	       status);
      gsiAuthReqError = status;
      return(status);
   }

   if (status < 0) {
      rodsLog (LOG_NOTICE,
	       "igsiServersideAuth: rsGenQuery failed, status = %d", status);
      snprintf(gsiAuthReqErrorMsg, 1000, 
	       "igsiServersideAuth: rsGenQuery failed, status = %d", status);
      gsiAuthReqError = status;
      return (status);
   }

   if (genQueryOut->rowCnt !=1 || genQueryOut->attriCnt != 2) {
      gsiAuthReqError = GSI_QUERY_INTERNAL_ERROR;
      return(GSI_QUERY_INTERNAL_ERROR);
   }

#ifdef GSI_DEBUG
   printf("Results=%d\n",genQueryOut->rowCnt);
#endif

   tResult = genQueryOut->sqlResult[0].value;
#ifdef GSI_DEBUG
   printf("0:%s\n",tResult);
#endif
   tResult = genQueryOut->sqlResult[1].value;
#ifdef GSI_DEBUG
   printf("1:%s\n",tResult);
#endif
   privLevel = LOCAL_USER_AUTH;
   clientPrivLevel = LOCAL_USER_AUTH;

   if (strcmp(tResult, "rodsadmin") == 0) {
      privLevel = LOCAL_PRIV_USER_AUTH;
      clientPrivLevel = LOCAL_PRIV_USER_AUTH;
   }

   /* XXXXX will need to change logic a bit for, perhaps for server-server
      and for remote zone users */
   rsComm->proxyUser.authInfo.authFlag = privLevel;
   rsComm->clientUser.authInfo.authFlag = clientPrivLevel;

   return status;
#else
    status = GSI_NOT_BUILT_INTO_SERVER;
    rodsLog (LOG_ERROR,
	    "igsiServersideAuth failed GSI_NOT_BUILT_INTO_SERVER, status = %d",
	     status);
    return (status);
#endif
}

