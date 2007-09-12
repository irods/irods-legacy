#include "reGlobalsExtern.h"
#include "execMyRule.h"

int
rsExecMyRule (rsComm_t *rsComm, execMyRuleInp_t *execMyRuleInp,
msParamArray_t **outParamArray)
{
    int status;
    ruleExecInfo_t rei;
    char *iFlag;
    int         oldReTestFlag, oldReLoopBackFlag;

    if (execMyRuleInp == NULL) { 
       rodsLog(LOG_NOTICE,
        "rsExecMyRule error. NULL input");
       return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    initReiWithDataObjInp (&rei, rsComm, NULL);
    rei.condInputData = &execMyRuleInp->condInput;
    /* need to have a non zero inpParamArray for execMyRule to work */
    if (execMyRuleInp->inpParamArray == NULL) {
	execMyRuleInp->inpParamArray = 
	  (msParamArray_t *) malloc (sizeof (msParamArray_t));
	memset (execMyRuleInp->inpParamArray, 0, sizeof (msParamArray_t));
    }
    rei.msParamArray = execMyRuleInp->inpParamArray;

    if ((iFlag = getValByKey (rei.condInputData,"looptest")) != NULL && !strcmp(iFlag,"true")) {
      oldReTestFlag = reTestFlag;
      oldReLoopBackFlag = reLoopBackFlag;
      reTestFlag = LOG_TEST_1;
      reLoopBackFlag = LOOP_BACK_1;
    }


    status = execMyRule (execMyRuleInp->myRule, execMyRuleInp->inpParamArray,
      &rei);
    
    if (iFlag != NULL) {
      reTestFlag = oldReTestFlag;
      reLoopBackFlag = oldReLoopBackFlag;
    }
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsExecMyRule : execMyRule error for %s, status = %d",     
          execMyRuleInp->myRule, status);
        return (status);
    }

    trimMsParamArray (rei.msParamArray, execMyRuleInp->outParamDesc);
    
    *outParamArray = rei.msParamArray;
    rei.msParamArray = NULL;

    return (status);
}

