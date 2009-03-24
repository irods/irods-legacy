/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"


int msiCreateUser(ruleExecInfo_t *rei)
{
  int i;
  /**** This is Just a Test Stub  ****/
  if (reTestFlag > 0 ) {
    if (reTestFlag == COMMAND_TEST_1 || reTestFlag == HTML_TEST_1) {
      print_uoi(rei->uoio);
    }
    else {
      rodsLog (LOG_NOTICE,"   Calling msiCreateUser For \n");
      print_uoi(rei->uoio);
    }
    if (reLoopBackFlag > 0)
      return(0);
  }
  /**** This is Just a Test Stub  ****/
#ifdef RODS_CAT
  i =  chlRegUserRE(rei->rsComm, rei->uoio);
#else
  i =  SYS_NO_ICAT_SERVER_ERR;
#endif
  return(i);
}

int msiCreateCollByAdmin(msParam_t* xparColl, msParam_t* xchildName, ruleExecInfo_t *rei)
{
    int i;
    collInfo_t collInfo;
  char *parColl;
  char *childName;

  parColl = (char *) xparColl->inOutStruct;
  childName = (char *) xchildName->inOutStruct;
  /**** This is Just a Test Stub  ****/
  if (reTestFlag > 0 ) {
    if (reTestFlag == COMMAND_TEST_1 || reTestFlag == HTML_TEST_1) {
      fprintf(stdout,"  NewCollection =%s/%s\n",
	       parColl,childName);
    }
    else {
      rodsLog (LOG_NOTICE,"   Calling msiCreateCollByAdmin Coll: %s/%s\n",
	       parColl,childName);
    }
    if (reLoopBackFlag > 0)
      return(0);
  }
  /**** This is Just a Test Stub  ****/

  snprintf(collInfo.collName,MAX_NAME_LEN, "%s/%s",parColl,childName);
  snprintf(collInfo.collOwnerName,MAX_NAME_LEN, "%s",rei->uoio->userName);
  snprintf(collInfo.collOwnerZone,MAX_NAME_LEN, "%s",rei->uoio->rodsZone);
#ifdef RODS_CAT
  i =  chlRegCollByAdmin(rei->rsComm, &collInfo );
#else
  i =  SYS_NO_RCAT_SERVER_ERR;
#endif
  return(i);
}

int msiDeleteCollByAdmin(msParam_t* xparColl, msParam_t* xchildName, ruleExecInfo_t *rei)
{
   int i;
   collInfo_t collInfo;
  char *parColl;
  char *childName;

  parColl = (char *) xparColl->inOutStruct;
  childName = (char *) xchildName->inOutStruct;
   /**** This is Just a Test Stub  ****/
   if (reTestFlag > 0 ) {
      if (reTestFlag == COMMAND_TEST_1 || reTestFlag == HTML_TEST_1) {
	 fprintf(stdout,"  NewCollection =%s/%s\n",
		 parColl,childName);
      }
      else {
	 rodsLog (LOG_NOTICE,"   Calling msiDeleteCallByAdmin Coll: %s/%s\n",
		  parColl,childName);
      }
      return(0);
   }
   /**** End of Test Stub  ****/


   snprintf(collInfo.collName,MAX_NAME_LEN, "%s/%s",parColl,childName);
   snprintf(collInfo.collOwnerName,MAX_NAME_LEN, "%s",rei->uoio->userName);
   snprintf(collInfo.collOwnerZone,MAX_NAME_LEN, "%s",rei->uoio->rodsZone);
#ifdef RODS_CAT
   i = chlDelCollByAdmin(rei->rsComm, &collInfo );
#else
   i = SYS_NO_RCAT_SERVER_ERR;
#endif
   if (i == CAT_UNKNOWN_COLLECTION) {
      /* Not sure where this kind of logic belongs, chl, rules,
         or here; but for now it's here.  */
      /* If the error is that it does not exist, return OK. */
      freeRErrorContent(&rei->rsComm->rError); /* remove suberrors if any */
      return(0); 
   }
   return(i);
}

int 
msiDeleteUser(ruleExecInfo_t *rei) {
  int i;
  /**** This is Just a Test Stub  ****/
  if (reTestFlag > 0 ) {
    if (reTestFlag == COMMAND_TEST_1 || reTestFlag == HTML_TEST_1) {
      print_uoi(rei->uoio);
    }
    else {
      rodsLog (LOG_NOTICE,"   Calling chlDeleteUser For \n");
      print_uoi(rei->uoio);
    }
    return(0);
  }
  /**** End of Test Stub  ****/
#ifdef RODS_CAT
  i =  chlDelUserRE(rei->rsComm, rei->uoio);
#else
  i = SYS_NO_RCAT_SERVER_ERR;
#endif
  return(i);
}

int 
msiAddUserToGroup(msParam_t *msParam, ruleExecInfo_t *rei) {
  int i;
#ifdef RODS_CAT
  char *groupName;
#endif
  if (reTestFlag > 0 ) {  /* Test stub mode */
    if (reTestFlag == COMMAND_TEST_1 || reTestFlag == HTML_TEST_1) {
      print_uoi(rei->uoio);
    }
    else {
      rodsLog (LOG_NOTICE,"   Calling chlModGroup For \n");
      print_uoi(rei->uoio);
    }
    return(0);
  }
#ifdef RODS_CAT
  if (strncmp(rei->uoio->userType,"rodsgroup",9)==0) {
     return(0);
  }
  groupName =  (char *) msParam->inOutStruct;
  i =  chlModGroup(rei->rsComm, groupName, "add", rei->uoio->userName,
		   rei->uoio->rodsZone);
#else
  i = SYS_NO_RCAT_SERVER_ERR;
#endif
  return(i);
}

int msiRenameLocalZone(msParam_t* oldName, msParam_t* newName,
		       ruleExecInfo_t *rei) {
   int status;
   char *oldNameStr;
   char *newNameStr;

   oldNameStr = (char *) oldName->inOutStruct;
   newNameStr = (char *) newName->inOutStruct;
#ifdef RODS_CAT
   status = chlRenameLocalZone(rei->rsComm, oldNameStr, newNameStr);
#else
   status = SYS_NO_RCAT_SERVER_ERR;
#endif
   return(status);
}

int msiRenameCollection(msParam_t* oldName, msParam_t* newName,
		       ruleExecInfo_t *rei) {
   int status;
   char *oldNameStr;
   char *newNameStr;

   oldNameStr = (char *) oldName->inOutStruct;
   newNameStr = (char *) newName->inOutStruct;
#ifdef RODS_CAT
   status = chlRenameColl(rei->rsComm, oldNameStr, newNameStr);
#else
   status = SYS_NO_RCAT_SERVER_ERR;
#endif
   return(status);
}

/* msiAclPolicy -
 *
 * When called (e.g. from acAclPolicy) and with "STRICT" as the
 * argument, this will set the ACL policy (for GeneralQuery) to be
 * extended (most strict).  See core.irb.
 *
 */
int
msiAclPolicy(msParam_t *msParam, ruleExecInfo_t *rei)
{
#if 0
   msParamArray_t *myMsParamArray;
   int flag=1;
#endif
   char *inputArg;

   inputArg =  (char *) msParam->inOutStruct;
   if (inputArg != NULL) {
      if (strncmp(inputArg,"STRICT",6)==0) {
#if 0
	 /* No longer need this as we're calling
	    chlGenQueryAccessControlSetup directly below (in case
	    msiAclPolicy will be called in a different manner than via
	    acAclPolicy sometime).
	    Leaving it in (ifdef'ed out tho) in case needed later.
	 */
	 myMsParamArray = mallocAndZero (sizeof (msParamArray_t));
	 addMsParamToArray (myMsParamArray, "STRICT", INT_MS_T, &flag,
			    NULL, 0);
	 rei->inOutMsParamArray=*myMsParamArray;
#endif
#ifdef RODS_CAT
	 chlGenQueryAccessControlSetup(NULL, NULL, 0, 2);
#endif
      }
   }
   else {
#ifdef RODS_CAT
      chlGenQueryAccessControlSetup(NULL, NULL, 0, 0);
#endif
   }
   return (0);
}
