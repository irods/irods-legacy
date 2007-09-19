/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "reGlobalsExtern.h"
#include "rsGlobalExtern.h"
#include "dataObjCreate.h"
#include "objMetaOpr.h"
#include "regDataObj.h"
/* #include "reAction.h" */
#include "miscServerFunct.h"


int
msiRegisterData(ruleExecInfo_t *rei)
{  
  int status;
  dataObjInfo_t *myDataObjInfo;


  /**** This is Just a Test Stub  ****/
  if (reTestFlag > 0 ) {
    if (reTestFlag == COMMAND_TEST_1 || reTestFlag == HTML_TEST_1) {
      print_doi(rei->doi);
    }
    else {
      rodsLog (LOG_NOTICE,"   Calling chlRegDataObj\n");
      print_doi(rei->doi);
    }
    if (reLoopBackFlag > 0)
      return(0);
  }
  /**** This is Just a Test Stub  ****/

  myDataObjInfo = L1desc[rei->l1descInx].dataObjInfo;
  status = svrRegDataObj (rei->rsComm, myDataObjInfo);
  if (status < 0) {
    rodsLog (LOG_NOTICE,
	     "msiRegisterData: rsRegDataObj for %s failed, status = %d",
	     myDataObjInfo->objPath, status);
    return (status);
  } else {
    myDataObjInfo->replNum = status;
    return (0);
  }
}

int
recover_msiRegisterData(ruleExecInfo_t *rei)
{

  int status;
  /**** This is Just a Test Stub  ****/
  if (reTestFlag > 0 ) {
    if (reTestFlag == LOG_TEST_1)
      rodsLog (LOG_NOTICE,"   ROLLBACK:Calling recover_chlRegDataObj\n");
    if (reLoopBackFlag > 0)
      return(0);
  }
  /**** This is Just a Test Stub  ****/

  msiRollback(rei); /** rolling back **/
  return(0);

}
int
print_bye(ruleExecInfo_t *rei)
{
  RE_TEST_MACRO ("Bye\n");
  fprintf(stdout, "Bye\n");
  return(0);
}

int
print_eol(ruleExecInfo_t *rei)
{
  RE_TEST_MACRO ("\n");
  fprintf(stdout, "\n");
  return(0);
}
int
print_hello_arg(msParam_t* xs, ruleExecInfo_t *rei )
{
  char *s;

  s = (char *) xs->inOutStruct;
  RE_TEST_MACRO (s);
    fprintf(stdout,"%s\n",s);
  return(0);
}

int
recover_print_bye(ruleExecInfo_t *rei)
{
   RE_TEST_MACRO ("\b\b\b   \b\b\b");
    fprintf(stdout,"\b\b\b   \b\b\b");
  return(0);
}

int
recover_print_eol(ruleExecInfo_t *rei)
{
   RE_TEST_MACRO ("*\b");
    fprintf(stdout,"*\b");
  return(0);
}

int
recover_print_hello_arg(msParam_t* xs, ruleExecInfo_t *rei)
{
  int i;
  char *s;

  s = (char *) xs->inOutStruct;
  for (i = 0; i < strlen(s);i++)
    fprintf(stdout,"\b \b");
  return(0);
}



int 
msitest1 (msParam_t *A, msParam_t *B, msParam_t* C, ruleExecInfo_t *rei)
{ /** A is IN, B is IN, C is OUT **/
  msParam_t *mPA, *mPB, *mPC;

  char tmpStr[200];
  mPA = (msParam_t *) A;
  mPB = (msParam_t *) B;
  mPC = (msParam_t *) C;

  mPC->inOutStruct = (void *) strdup("msitest1 ValC");
  mPC->type = (char *) strdup("STR_PI");
  snprintf(tmpStr,199, "msitest1: In A=%s, In B=%s, Out C=%s", 
	   (char *) mPA->inOutStruct,(char *) mPB->inOutStruct,(char *) mPC->inOutStruct);
  printf("%s\n",tmpStr);
  RE_TEST_MACRO (tmpStr);

  return(0);
}
int 
msitest2 (msParam_t *A, msParam_t *B, msParam_t* C, ruleExecInfo_t *rei)
{ /** A is IN, B is OUT, C is OUT **/
  msParam_t *mPA, *mPB, *mPC;

  char tmpStr[200];
  mPA = (msParam_t *) A;
  mPB = (msParam_t *) B;
  mPC = (msParam_t *) C;

  mPB->inOutStruct = (void *) strdup("msitest2 ValB");
  mPC->inOutStruct = (void *) strdup("msitest2 ValC");
  mPB->type = (char *) strdup("STR_PI");
  mPC->type = (char *) strdup("STR_PI");
  snprintf(tmpStr,199, "msitest2: In A=%s, Out B=%s, Out C=%s", 
	   (char *) mPA->inOutStruct,(char *) mPB->inOutStruct,(char *) mPC->inOutStruct);
  printf("%s\n",tmpStr);
  RE_TEST_MACRO (tmpStr);

  return(0);
}

int 
msitest3 (msParam_t *A, msParam_t *B, msParam_t* C, ruleExecInfo_t *rei)
{ /** A is IN, B is IN C is IN **/
  msParam_t *mPA, *mPB, *mPC;

  char tmpStr[200];
  mPA = (msParam_t *) A;
  mPB = (msParam_t *) B;
  mPC = (msParam_t *) C;

  snprintf(tmpStr,199, "msitest3: In A=%s, In B=%s, In C=%s", 
	   (char *) mPA->inOutStruct,(char *) mPB->inOutStruct,(char *) mPC->inOutStruct);
  printf("%s\n",tmpStr);
  RE_TEST_MACRO (tmpStr);

  return(0);
}

