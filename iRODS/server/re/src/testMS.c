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
  _writeString("stdout", "Hello\n", rei);
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
  return(0);
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
  return(0);
}

