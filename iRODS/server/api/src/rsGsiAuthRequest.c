/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* See gsiAuthRequest.h for a description of this API call.*/

#include "gsiAuthRequest.h"
#include "authResponse.h"
#include "genQuery.h"
#include "reGlobals.h"

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
   int noNameMode;

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

   noNameMode=0;
   if (strlen(rsComm->clientUser.userName)>0) {  
      /* regular mode */

      snprintf (condition1, MAX_NAME_LEN, "='%s'", clientName);
      addInxVal (&genQueryInp.sqlCondInp, COL_USER_DN, condition1);

      snprintf (condition2, MAX_NAME_LEN, "='%s'", 
		rsComm->clientUser.userName);
      addInxVal (&genQueryInp.sqlCondInp, COL_USER_NAME, condition2);

      addInxIval (&genQueryInp.selectInp, COL_USER_ID, 1);
      addInxIval (&genQueryInp.selectInp, COL_USER_TYPE, 1);

      genQueryInp.maxRows = 2;

      status = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
   }
   else {
      /* 
	 The client isn't providing the rodsUserName so query on just
         the DN.  If it returns just one row, set the clientUser to 
         the returned irods user name.
      */
      noNameMode=1;
      memset (&genQueryInp, 0, sizeof (genQueryInp_t));
      
      snprintf (condition1, MAX_NAME_LEN, "='%s'", clientName);
      addInxVal (&genQueryInp.sqlCondInp, COL_USER_DN, condition1);

      addInxIval (&genQueryInp.selectInp, COL_USER_ID, 1);
      addInxIval (&genQueryInp.selectInp, COL_USER_TYPE, 1);
      addInxIval (&genQueryInp.selectInp, COL_USER_NAME, 1);

      genQueryInp.maxRows = 2;

      status = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

      if (status == 0) {
      	 strncpy(rsComm->clientUser.userName, genQueryOut->sqlResult[2].value,
      		 NAME_LEN);
       	 strncpy(rsComm->proxyUser.userName, genQueryOut->sqlResult[2].value,
		 NAME_LEN);
      }

      if (status == CAT_NO_ROWS_FOUND) { /* not found */

	 /* execute the rule acGetUserByDN.  By default this
            is a no-op but at some sites can be configured to
            run a process to determine a user by DN (for VO support)
            or possibly create the user.
	    The stdout of the process is the irodsUserName to use.

	    The corresponding rule would be something like this:
	    acGetUserByDN(*arg,*OUT)||msiExecCmd(t,"*arg",null,null,null,*OUT)|nop
	 */

	 ruleExecInfo_t rei;
	 char *args[1];
	 msParamArray_t *myMsParamArray;
	 msParamArray_t myInOutParamArray;
	 msParam_t *mP;
	 execCmdOut_t *execCmdOut;
	 int userFoundViaRule=0;

	 memset((char*)&rei,0,sizeof(rei));
	 rei.rsComm = rsComm;
	 rei.uoic = &rsComm->clientUser;
	 rei.uoip = &rsComm->proxyUser;
	 args[0]=clientName;
	 char out[200]="*cmdOutput";
	 args[1]=out;

	 rei.inOutMsParamArray = myInOutParamArray;

	 myMsParamArray = malloc (sizeof (msParamArray_t));
	 memset (myMsParamArray, 0, sizeof (msParamArray_t));

	 status = applyRuleArgPA("acGetUserByDN", args, 2, 
				 myMsParamArray, &rei, NO_SAVE_REI);	

#ifdef GSI_DEBUG
	 printf("acGetUserByDN status=%d\n",status);

	 int i;
	 for (i=0;i<myMsParamArray->len;i++)
         {
	    char *r;
	    msParam_t *myP;
            myP = myMsParamArray->msParam[i];
	    r = myP->label;
	    printf("l1=%s\n", r);
	 }
#endif
	 /* if it ran OK, set the username to the returned value (stdout) */
	 if (status==0) {
	    int len;
	    if ((mP = getMsParamByLabel(myMsParamArray,"*cmdOutput"))!= NULL) {
	       execCmdOut = (execCmdOut_t *) mP->inOutStruct;
	       if (execCmdOut != NULL && execCmdOut->stdoutBuf.buf != NULL) {
		  len = strlen(execCmdOut->stdoutBuf.buf);
		  if (len > 1) {
		     len--; /* skip trailing \n */
		     if (len > NAME_LEN) len=NAME_LEN;
		     strncpy(rsComm->clientUser.userName, 
			     execCmdOut->stdoutBuf.buf, len);
		     strncpy(rsComm->proxyUser.userName, 
			     execCmdOut->stdoutBuf.buf, len);
#ifdef GSI_DEBUG
		     fprintf(stdout,"set to '%s'\n",
			     rsComm->clientUser.userName);
#endif

		     userFoundViaRule=1;
		  }
	       }
#ifdef GSI_DEBUG
	       if (execCmdOut->stderrBuf.buf != NULL) {
		  fprintf(stderr,"%s", (char *) execCmdOut->stderrBuf.buf);
	       }
#endif
	    }
	 }

	 /* If the rule didn't work, try the query again as the rule
            may have added the user. */
	 if (!userFoundViaRule) {
	    memset (&genQueryInp, 0, sizeof (genQueryInp_t));
      
	    snprintf (condition1, MAX_NAME_LEN, "='%s'", clientName);
	    addInxVal (&genQueryInp.sqlCondInp, COL_USER_DN, condition1);

	    addInxIval (&genQueryInp.selectInp, COL_USER_ID, 1);
	    addInxIval (&genQueryInp.selectInp, COL_USER_TYPE, 1);
	    addInxIval (&genQueryInp.selectInp, COL_USER_NAME, 1);

	    genQueryInp.maxRows = 2;

	    status = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

	    if (status == 0) {
	       strncpy(rsComm->clientUser.userName, genQueryOut->sqlResult[2].value,
		       NAME_LEN);
	       strncpy(rsComm->proxyUser.userName, genQueryOut->sqlResult[2].value,
		       NAME_LEN);
	    }
	 }
      }
   }
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

   if (noNameMode==0) {
      if (genQueryOut->rowCnt !=1 || genQueryOut->attriCnt != 2) {
	 gsiAuthReqError = GSI_QUERY_INTERNAL_ERROR;
	 return(GSI_QUERY_INTERNAL_ERROR);
      }
   }
   else {
      if (genQueryOut->rowCnt !=1 || genQueryOut->attriCnt != 3) {
	 gsiAuthReqError = GSI_QUERY_INTERNAL_ERROR;
	 return(GSI_QUERY_INTERNAL_ERROR);
      }
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

   status = chkProxyUserPriv (rsComm, privLevel);

   if (status < 0) return status;

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

