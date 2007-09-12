/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"


int
msiGetIcatTime(msParam_t* timeOutParam,  msParam_t* typeInParam, ruleExecInfo_t *rei)
{
  char *type;
  char tStr0[TIME_LEN],tStr[TIME_LEN];
  int i;

  type = typeInParam->inOutStruct;

  if (!strcmp(type,"icat") || !strcmp(type,"unix")) {
    getNowStr(tStr);
  }
  else { /* !strcmp(type,"human") */
    getNowStr(tStr0);
    getLocalTimeFromRodsTime(tStr0,tStr);
  }
  i = fillStrInMsParam (timeOutParam,tStr);
  return(i);
}


int
msiVacuum(ruleExecInfo_t *rei)
{
   int i;
   rodsLog(LOG_NOTICE, "msiVacuum called\n");

   i = doForkExec("/usr/bin/perl", "./vacuumdb.pl");

   if (i) {
      rodsLog(LOG_ERROR, "msiVacuum doForkExec failure\n");
   }

   rodsLog(LOG_NOTICE, "msiVacuum done\n");

   return(0);
}





int  msiSetResource(msParam_t* xrescName, ruleExecInfo_t *rei)
{
  char *rescName;

  rescName = (char *) xrescName->inOutStruct;
  if (reTestFlag > 0 ) {
    if (reTestFlag == LOG_TEST_1) 
      rodsLog (LOG_NOTICE,"   Calling msiSetResource\n");
  }

  strcpy(rei->doi->rescName,rescName);
  return(0);
}


int msiCheckOwner(ruleExecInfo_t *rei)
{
    if (reTestFlag > 0 ) {
    if (reTestFlag == LOG_TEST_1) 
      rodsLog (LOG_NOTICE,"   Calling msiCheckOwner\n");
  }

  if (!strcmp(rei->doi->dataOwnerName,rei->uoic->userName) &&
      !strcmp(rei->doi->dataOwnerZone,rei->uoic->rodsZone))
    return(0);
  else
    return(ACTION_FAILED_ERR);

}
int msiCheckPermission(msParam_t* xperm, ruleExecInfo_t *rei)
{
  char *perm;

  perm = (char *) xperm->inOutStruct;
    if (reTestFlag > 0 ) {
    if (reTestFlag == LOG_TEST_1) 
      rodsLog (LOG_NOTICE,"   Calling msiCheckPermission\n");
  }
  if (strstr(rei->doi->dataAccess,perm) != NULL)
    return(0);
  else
    return(ACTION_FAILED_ERR);

}


int
msiCommit(ruleExecInfo_t *rei) {
   int status;

  /**** This is Just a Test Stub  ****/
  if (reTestFlag > 0 ) {
    if (reTestFlag == LOG_TEST_1) {
      rodsLog (LOG_NOTICE,"   Calling msiCommit\n");
    }
    if (reLoopBackFlag > 0)
      return(0);
  }
  /**** This is Just a Test Stub  ****/
#ifdef RODS_CAT
   status = chlCommit(rei->rsComm);
#else
   status =  SYS_NO_RCAT_SERVER_ERR;
#endif
   return(status);
}

int
msiRollback(ruleExecInfo_t *rei) {
   int status;
  /**** This is Just a Test Stub  ****/
  if (reTestFlag > 0 ) {
    if (reTestFlag == LOG_TEST_1) {
      rodsLog (LOG_NOTICE,"   Calling msiRollback\n");
    }
    if (reLoopBackFlag > 0)
      return(0);
  }
  /**** This is Just a Test Stub  ****/


#ifdef RODS_CAT
   status = chlRollback(rei->rsComm);
#else
   status =  SYS_NO_RCAT_SERVER_ERR;
#endif
   return(status);
}
