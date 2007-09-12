/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"

int
print_hello(ruleExecInfo_t *rei)
{
  /***
  rodsLog(LOG_NOTICE, "TTTTT Hello\n");
  rodsLogAndErrorMsg(LOG_NOTICE, &(rei->rsComm->rError),-1, "VVVVV Hello\n");
  ***/
  RE_TEST_MACRO ("Test for print_hello\n");
  fprintf(stdout, "Hello\n");
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
recover_print_hello(ruleExecInfo_t *rei)
{
  RE_TEST_MACRO ("\b\b\b\b\b     \b\b\b\b\b");
    fprintf(stdout,"\b\b\b\b\b     \b\b\b\b\b");
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
print_doi(dataObjInfo_t *doi)
{
  if (reTestFlag == COMMAND_TEST_1) {
    fprintf(stdout,"     objPath = %s\n",doi->objPath);
    fprintf(stdout,"     rescName= %s\n",doi->rescName);
    fprintf(stdout,"     dataType= %s\n",doi->dataType);
    fprintf(stdout,"     dataSize= %lld\n",doi->dataSize);
  }
  else if(reTestFlag == HTML_TEST_1) {
    fprintf(stdout," <UL>\n");
    fprintf(stdout,"  <LI>     objPath = %s\n",doi->objPath);
    fprintf(stdout,"  <LI>     rescName= %s\n",doi->rescName);
    fprintf(stdout,"  <LI>     dataType= %s\n",doi->dataType);
    fprintf(stdout,"  <LI>     dataSize= %lld\n",doi->dataSize);
    fprintf(stdout," </UL>\n");
  }
  else {
    rodsLog (LOG_NOTICE,"     objPath = %s\n",doi->objPath);
    rodsLog (LOG_NOTICE,"     rescName= %s\n",doi->rescName);
    rodsLog (LOG_NOTICE,"     dataType= %s\n",doi->dataType);
    rodsLog (LOG_NOTICE,"     dataSize= %lld\n",doi->dataSize);
  }
}


int
print_uoi(userInfo_t *uoi)
{
  if (reTestFlag == COMMAND_TEST_1) {
    fprintf(stdout,"     userName = %s\n",uoi->userName);
    fprintf(stdout,"     rodsZone= %s\n",uoi->rodsZone);
    fprintf(stdout,"     userType= %s\n",uoi->userType);
  }
  else if(reTestFlag == HTML_TEST_1) {
    fprintf(stdout," <UL>\n");
    fprintf(stdout,"  <LI>     userName= %s\n",uoi->userName);
    fprintf(stdout,"  <LI>     rodsZone= %s\n",uoi->rodsZone);
    fprintf(stdout,"  <LI>     userType= %s\n",uoi->userType);

    fprintf(stdout," </UL>\n");
  }
  else {
    rodsLog (LOG_NOTICE,"     userName= %s\n",uoi->userName);
    rodsLog (LOG_NOTICE,"     rodsZone= %s\n",uoi->rodsZone);
    rodsLog (LOG_NOTICE,"     userType= %s\n",uoi->userType);

  }
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

