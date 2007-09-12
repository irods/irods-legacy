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
