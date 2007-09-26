/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"
#include "execMyRule.h"

int
fillSubmitConditions (char *action, char *inDelayCondition, bytesBuf_t *packedReiAndArgBBuf, 
        ruleExecSubmitInp_t *ruleSubmitInfo,  ruleExecInfo_t *rei);

int assign(msParam_t* var, msParam_t* value, ruleExecInfo_t *rei)
{
  char *varName;
  char *varMap;
  char *varValue;
  char aVal[MAX_COND_LEN * 2];
  int i,j;
  char eaVal[MAX_COND_LEN * 2];
  varName = (char *)var->label;
  varValue = (char *) var->inOutStruct;
  if ( varName[0] == '$') {
    i = getVarMap("", var->inOutStruct, &varMap, 0);
    if (i < 0) return(i);
    rstrcpy(aVal,value->inOutStruct,MAX_COND_LEN * 2);
    i = evaluateExpression(aVal, eaVal,rei);
    if (i < 0 ) 
      return(i);
    /*    if (strcmp(eaVal, aVal))*/
    rodsLog (LOG_NOTICE,"BEFORE:uioc=%s,uiop=%s:cp=%d,pp=%d\n",rei->uoic->userName,rei->uoip->userName, rei->uoic,rei->uoip);
    i = setVarValue(varMap,rei, eaVal);
    rodsLog (LOG_NOTICE,"AFTER:uioc=%s,uiop=%s:cp=%d,pp=%d\n",rei->uoic->userName,rei->uoip->userName, rei->uoic,rei->uoip);
  }
  else { /* it is a *-variable so just copy */
    if ((value->type == NULL && value->label != NULL) ||
	strcmp (value->type, STR_MS_T) == 0 ||
	strcmp (value->type, STR_PI) == 0 ) {
      if (value->inOutStruct != NULL)
	rstrcpy(aVal,value->inOutStruct,MAX_COND_LEN * 2);
      else
	rstrcpy(aVal,value->label,MAX_COND_LEN * 2);
      i = evaluateExpression(aVal, eaVal,rei);
      if (i < 0)
	fillMsParam (var, NULL, value->type, value->inOutStruct, value->inpOutBuf);
      else if (!strcmp(eaVal,aVal)) {
	var->inOutStruct = strdup(aVal);
	var->type = strdup(STR_MS_T);
      }
      else {
	/*	fillIntInMsParam(var,j); */
	var->inOutStruct = strdup(eaVal);
	var->type = strdup(STR_MS_T);
      }
    }
    else
      fillMsParam (var, NULL, value->type, value->inOutStruct, value->inpOutBuf);
  }
  return(0);
}


int whileExec(msParam_t* condition, msParam_t* whileBody,
	      msParam_t* recoverWhileBody, ruleExecInfo_t *rei)
{
  int i,j, done,res;
  char *cond, *ruleBody;
  msParamArray_t *inMsParamArray = NULL;
  char eaVal[MAX_COND_LEN * 2];
  done = 0;
  inMsParamArray = rei->msParamArray;
  cond = (char *) malloc(strlen(condition->label) + 10 + MAX_COND_LEN * 2);
  ruleBody = (char *) malloc ( strlen(whileBody->inOutStruct) +
			       strlen(recoverWhileBody->inOutStruct) + 10 + MAX_COND_LEN * 2);
  sprintf(ruleBody,"%s|%s",(char *) whileBody->inOutStruct,
	    (char *) recoverWhileBody->inOutStruct + MAX_COND_LEN * 8);
    
  while (done == 0) {
    strcpy(cond , condition->label);
    i = evaluateExpression(cond, eaVal, rei);
    if (i < 0)
      return(i);
    if (i == 0) { /* FALSE */
      done = 1;
      break;
    }    
    i = execMyRuleWithSaveFlag(ruleBody, inMsParamArray, rei, 0);
    if (i != 0) 
      return(i);
    /*  RECOVERY OF OTHER EXECUTED BODY IS NOT DONE. HOW CAN WE DO THAT */
  }
  return(0);
}


int forExec(msParam_t* initial, msParam_t* condition, msParam_t* step, 
	    msParam_t* forBody, msParam_t* recoverForBody, ruleExecInfo_t *rei)
{

  int i,j, done,res;
  char *cond, *ruleBody, *stepStr;
  char eaVal[MAX_COND_LEN * 2];
  done = 0;
  i = execMyRuleWithSaveFlag(initial->label,rei->msParamArray, rei),0;
  if (i != 0)
    return(i);
  cond = (char *) malloc(strlen(condition->label) + 10 + MAX_COND_LEN * 2);
  stepStr = (char *) malloc(strlen(step->label) + 10 + MAX_COND_LEN * 2);
  ruleBody = (char *) malloc ( strlen(forBody->inOutStruct) +
                                 strlen(recoverForBody->inOutStruct) + 10 + MAX_COND_LEN * 8);
  
  while (done == 0) {
    strcpy(cond , condition->label);
    i = evaluateExpression(cond, eaVal, rei);
    if (i < 0)
      return(i);
    if (i == 0) { /* FALSE */
      done = 1;
      break;
    }
    sprintf(ruleBody,"%s|%s",(char *) forBody->inOutStruct,
            (char *) recoverForBody->inOutStruct);
    i = execMyRuleWithSaveFlag(ruleBody, rei->msParamArray, rei, 0);
    if (i != 0)
      return(i);
    strcpy(stepStr,step->label);
    i = execMyRuleWithSaveFlag(stepStr,rei->msParamArray, rei,0);
    if (i != 0)
      return(i);
  }
  return(0);
}

int ifExec(msParam_t* condition, msParam_t* thenC, msParam_t* recoverThen, 
	   msParam_t* elseC, msParam_t* recoverElse, ruleExecInfo_t *rei)
{
  int i,j;  
  char *cond, *thenStr, *elseStr;
  char eaVal[MAX_COND_LEN * 2];

  if (condition->inOutStruct == NULL) {
    cond = (char *) malloc(strlen(condition->label) + 10 + MAX_COND_LEN * 2);
    strcpy(cond , condition->label);
  }
  else {
    cond = (char *) malloc(strlen(condition->inOutStruct) + 10 + MAX_COND_LEN * 2);
    strcpy(cond , condition->inOutStruct);
  }

  i = evaluateExpression(cond, eaVal, rei);
  free(cond);
  if (i < 0)
    return(i);
  if (i == 1) { /* TRUE */
    thenStr = (char *) malloc(strlen(thenC->inOutStruct) + strlen(recoverThen->inOutStruct) + 10 + MAX_COND_LEN * 8);
    sprintf(thenStr,"%s|%s",(char *) thenC->inOutStruct,
            (char *) recoverThen->inOutStruct);
    i = execMyRuleWithSaveFlag(thenStr, rei->msParamArray, rei, 0);
    free(thenStr);
  }
  else {
    elseStr = (char *) malloc(strlen(elseC->inOutStruct) + strlen(recoverElse->inOutStruct) + 10 + MAX_COND_LEN * 8);
    sprintf(elseStr,"%s|%s",(char *) elseC->inOutStruct,
            (char *) recoverElse->inOutStruct);
    i = execMyRuleWithSaveFlag(elseStr, rei->msParamArray, rei, 0);
    free(elseStr);
  }
  return(i);

}

int forEachExec(msParam_t* inlist, msParam_t* body, msParam_t* recoverBody,
	      ruleExecInfo_t *rei)
{

  char *label ;
  int i;
  msParamArray_t *inMsParamArray;
  void *value, *restPtr, *inPtr, *buf, *inStructPtr, *msParamStruct ;
  char *typ;
  char *bodyStr;
  msParam_t *msParam, *list;
  bytesBuf_t *inBufPtr, *msParamBuf;
  int first = 1;
  int inx;
  char *outtyp;

  list = (msParam_t *) malloc (sizeof (msParam_t));
  memset (list, 0, sizeof (msParam_t));
  replMsParam (inlist,list);
  typ = strdup(list->type);
  label = list->label;
  inPtr = list->inOutStruct;
  inStructPtr = inPtr;
  inBufPtr = list->inpOutBuf;
  outtyp = typ;
  buf = inBufPtr;
  value = NULL;

  inMsParamArray = rei->msParamArray;
  msParam = getMsParamByLabel(inMsParamArray, label);
  if (msParam != NULL) {
    msParamStruct = msParam->inOutStruct;
    msParamBuf = msParam->inpOutBuf;

  }
  else 
    msParamStruct = NULL;

  i = 0;

  bodyStr = (char *) malloc ( strlen(body->inOutStruct) +
                               strlen(recoverBody->inOutStruct) + 10 + MAX_COND_LEN * 8);
  sprintf(bodyStr,"%s|%s",(char *) body->inOutStruct,
	    (char *) recoverBody->inOutStruct);
    
  restPtr = NULL;
  inx = 0;
  while (inPtr != NULL) {
    i = getNextValueAndBufFromListOrStruct(typ, inPtr, &value, &buf, &restPtr, &inx, &outtyp);
    if ( i != 0)
      break;

    if (first == 1 && msParam == NULL) {
      addMsParam(inMsParamArray, label, outtyp ,value, buf);
      msParam = getMsParamByLabel(inMsParamArray, label);
      first = 0;
    }
    else {
      msParam->inOutStruct = value;
      msParam->inpOutBuf = buf;
      msParam->type = outtyp;
    }


    i = execMyRuleWithSaveFlag(bodyStr, inMsParamArray, rei, 0);
    if ( i != 0)
      break;
    /*  RECOVERY OF OTHER EXECUTED BODY IS NOT DONE. HOW CAN WE DO THAT */
    /***    freeNextValueStruct(&value,outtyp,"internal"); ***/
    inPtr = restPtr;    
  }
  /***  freeNextValueStruct(&value,outtyp,"all");***/
  if (msParamStruct != NULL) {
    /* value in rei->params get replaced back... */
    msParam->inOutStruct = msParamStruct;
    msParam->inpOutBuf = msParamBuf;
    msParam->type = typ;
  }
  /*** 
       clearMsParamArray (list,1);
       free(list);
  ***/
  if (i == NO_VALUES_FOUND)
    return(0);
  return(i);
}

int delayExec(msParam_t *mPA, msParam_t *mPB, msParam_t *mPC, ruleExecInfo_t *rei)
{
  int i;
  char actionCall[MAX_ACTION_SIZE];  
  char recoveryActionCall[MAX_ACTION_SIZE];  
  char delayCondition[MAX_ACTION_SIZE]; 

  rstrcpy(delayCondition, (char *) mPA->inOutStruct,MAX_ACTION_SIZE);
  rstrcpy(actionCall, (char *) mPB->inOutStruct,MAX_ACTION_SIZE);
  rstrcpy(recoveryActionCall, (char *) mPC->inOutStruct,MAX_ACTION_SIZE);
  i = _delayExec(actionCall, recoveryActionCall, delayCondition, rei);
  return(i);
}



int _delayExec(char *inActionCall, char *recoveryActionCall, 
	       char *delayCondition,  ruleExecInfo_t *rei)
{

  char *args[MAX_NUM_OF_ARGS_IN_ACTION];
  int i, argc;
  ruleExecSubmitInp_t *ruleSubmitInfo;
  char action[MAX_ACTION_SIZE];  
  char tmpStr[NAME_LEN];  
  bytesBuf_t *packedReiAndArgBBuf = NULL;
  char *ruleExecId;
  char *actionCall;


  RE_TEST_MACRO ("    Calling _delayExec");

  actionCall = inActionCall;
  /* Get Arguments */
  if (strstr(actionCall,"##") == NULL && !strcmp(recoveryActionCall,"nop")) {
    /* single call */
    i = parseAction(actionCall,action,args, &argc);
    if (i != 0)
      return(i);
    mapExternalFuncToInternalProc(action);
    argc = 0;
  }
  else {
    actionCall = malloc(strlen(inActionCall) + strlen(recoveryActionCall) + 3);
    sprintf(actionCall,"%s|%s",inActionCall,recoveryActionCall);
    args[0] = NULL;
    args[1] = NULL;
    argc = 0;
  }
  /* Pack Rei and Args */
  i = packReiAndArg (rei->rsComm, rei, args, argc, &packedReiAndArgBBuf);
  if (i < 0) {
    if (actionCall != inActionCall) 
      free (actionCall);
    return(i);
  }
  /* fill Conditions into Submit Struct */
  ruleSubmitInfo = mallocAndZero(sizeof(ruleExecSubmitInp_t));
  i  = fillSubmitConditions (actionCall, delayCondition, packedReiAndArgBBuf, ruleSubmitInfo, rei);
  if (actionCall != inActionCall) 
    free (actionCall);
  if (i < 0) {
    free(ruleSubmitInfo);
    return(i);
  }
  
  /* Store ReiArgs Struct in a File */
  i = rsRuleExecSubmit(rei->rsComm, ruleSubmitInfo, &ruleExecId);
  if (packedReiAndArgBBuf != NULL) {
    clearBBuf (packedReiAndArgBBuf);
    free (packedReiAndArgBBuf);
  }
  
  free(ruleSubmitInfo);
  if (i < 0) 
    return(i);
  free (ruleExecId);
  snprintf(tmpStr,NAME_LEN, "%d",i);
  i = pushStack(&delayStack,tmpStr);
  return(i);
}

int recover_delayExec(msParam_t *actionCall, msParam_t *delayCondition,  ruleExecInfo_t *rei)
{

  int i;
  ruleExecDelInp_t ruleExecDelInp;

  RE_TEST_MACRO ("    Calling recover_delayExec");

  i  = popStack(&delayStack,ruleExecDelInp.ruleExecId);
  if (i < 0)
    return(i);

  i = rsRuleExecDel(rei->rsComm, &ruleExecDelInp);
  return(i);

}


int remoteExec(msParam_t *mPD, msParam_t *mPA, msParam_t *mPB, msParam_t *mPC, ruleExecInfo_t *rei)
{
  int i;
  execMyRuleInp_t execMyRuleInp;
  msParamArray_t *tmpParamArray, *aParamArray;
  msParamArray_t *outParamArray = NULL;
  char tmpStr[LONG_NAME_LEN];
  /*
  char actionCall[MAX_ACTION_SIZE];  
  char recoveryActionCall[MAX_ACTION_SIZE];  
  char delayCondition[MAX_ACTION_SIZE]; 
  char hostName[MAX_ACTION_SIZE]; 
  rstrcpy(hostName, (char *) mPD->inOutStruct,MAX_ACTION_SIZE);
  rstrcpy(delayCondition, (char *) mPA->inOutStruct,MAX_ACTION_SIZE);
  rstrcpy(actionCall, (char *) mPB->inOutStruct,MAX_ACTION_SIZE);
  rstrcpy(recoveryActionCall, (char *) mPC->inOutStruct,MAX_ACTION_SIZE);
  i = _remoteExec(actionCall, recoveryActionCall, delayCondition, hostName, rei);
  */
  memset (&execMyRuleInp, 0, sizeof (execMyRuleInp));
  execMyRuleInp.condInput.len=0;
  rstrcpy (execMyRuleInp.outParamDesc, ALL_MS_PARAM_KW, LONG_NAME_LEN);
  /*  rstrcpy (execMyRuleInp.addr.hostAddr, mPD->inOutStruct, LONG_NAME_LEN);*/
  rstrcpy (tmpStr, (char *) mPD->inOutStruct, LONG_NAME_LEN);
  i = evaluateExpression(tmpStr, execMyRuleInp.addr.hostAddr, rei);
  if (i < 0)
    return(i);
  snprintf(execMyRuleInp.myRule, META_STR_LEN, "remExec||%s|%s",  mPB->inOutStruct,mPC->inOutStruct);
  addKeyVal(&execMyRuleInp.condInput,"execCondition",mPA->inOutStruct);
  
  tmpParamArray =  (msParamArray_t *) malloc (sizeof (msParamArray_t));
  memset (tmpParamArray, 0, sizeof (msParamArray_t));
  i = replMsParamArray (rei->msParamArray,tmpParamArray);
  if (i < 0) {
    free(tmpParamArray);
    return(i);
  }
  aParamArray = rei->msParamArray;
  rei->msParamArray = tmpParamArray;
  execMyRuleInp.inpParamArray = rei->msParamArray;
  i = rsExecMyRule (rei->rsComm, &execMyRuleInp,  &outParamArray);
  carryOverMsParam(outParamArray, aParamArray);
  rei->msParamArray = aParamArray;
  clearMsParamArray(tmpParamArray,0);
  free(tmpParamArray);
  return(i);
}


/*****
int _remoteExec(char *inActionCall, char *recoveryActionCall, 
	       char *delayCondition,  char *hostName, ruleExecInfo_t *rei)
{

  char *args[MAX_NUM_OF_ARGS_IN_ACTION];
  int i, argc;
  ruleExecSubmitInp_t *ruleSubmitInfo;
  char action[MAX_ACTION_SIZE];  
  char tmpStr[NAME_LEN];  
  bytesBuf_t *packedReiAndArgBBuf = NULL;
  char *ruleExecId;
  char *actionCall;


  RE_TEST_MACRO ("    Calling _delayExec");

  actionCall = inActionCall;
  if (strstr(actionCall,"##") == NULL && !strcmp(recoveryActionCall,"nop")) {
    i = parseAction(actionCall,action,args, &argc);
    if (i != 0)
      return(i);
    mapExternalFuncToInternalProc(action);
    argc = 0;
  }
  else {
    actionCall = malloc(strlen(inActionCall) + strlen(recoveryActionCall) + 3);
    sprintf(actionCall,"%s|%s",inActionCall,recoveryActionCall);
    args[0] = NULL;
    args[1] = NULL;
    argc = 0;
  }

  i = packReiAndArg (rei->rsComm, rei, args, argc, &packedReiAndArgBBuf);
  if (i < 0) {
    if (actionCall != inActionCall) 
      free (actionCall);
    return(i);
  }

  ruleSubmitInfo = mallocAndZero(sizeof(ruleExecSubmitInp_t));
  i  = fillSubmitConditions (actionCall, delayCondition, packedReiAndArgBBuf, ruleSubmitInfo, rei);
  strncpy(ruleSubmitInfo->exeAddress,hostName,NAME_LEN);
  if (actionCall != inActionCall) 
    free (actionCall);
  if (i < 0) {
    free(ruleSubmitInfo);
    return(i);
  }
  
  i = rsRemoteRuleExecSubmit(rei->rsComm, ruleSubmitInfo, &ruleExecId);
  if (packedReiAndArgBBuf != NULL) {
    clearBBuf (packedReiAndArgBBuf);
    free (packedReiAndArgBBuf);
  }
  
  free(ruleSubmitInfo);
  if (i < 0) 
    return(i);
  free (ruleExecId);
  snprintf(tmpStr,NAME_LEN, "%d",i);
  i = pushStack(&delayStack,tmpStr);
  return(i);
}
******/
int recover_remoteExec(msParam_t *actionCall, msParam_t *delayCondition, char *hostName, ruleExecInfo_t *rei)
{

  int i;
  ruleExecDelInp_t ruleExecDelInp;

  RE_TEST_MACRO ("    Calling recover_delayExec");

  i  = popStack(&delayStack,ruleExecDelInp.ruleExecId);
  if (i < 0)
    return(i);
  /***
  i = rsRemoteRuleExecDel(rei->rsComm, &ruleExecDelInp);
  ***/
  return(i);

}


int
doForkExec(prog,arg1)
     char *prog;
     char *arg1;
{
   int pid, i;
     
   i = checkFilePerms(prog);
   if (i) return(i);

   i = checkFilePerms(arg1);
   if (i) return(i);

   pid = fork();
   if (pid == -1) return -1;

   if (pid) {
      /*  This is still the parent.  */
   }
   else {
      /* child */
      for (i=0;i<100;i++) {
	 close(i);
      }
      i = execl(prog, prog, arg1, 0);
      printf("execl failed %d\n",i);
      exit(0);
   }
}


int
msiGoodFailure(ruleExecInfo_t *rei)
{
  
  /**** This is Just a Test Stub  ****/
  if (reTestFlag > 0) {
    if (reTestFlag == LOG_TEST_1)
      rodsLog (LOG_NOTICE,"   Calling msiGoodFailure So that It will Retry Other Rules Without Recovery\n");
    return(RETRY_WITHOUT_RECOVERY_ERR);
  }
  /**** This is Just a Test Stub  ****/
  return(RETRY_WITHOUT_RECOVERY_ERR);
}



/* check that a file exists and is not writable by group or other */ 
int
checkFilePerms(char *fileName) {
   struct stat buf;
   if (stat (fileName, &buf) == -1) {
      printf ("Stat failed for file %s\n", 
              fileName);
      return(-1);
   }

   if (buf.st_mode & 022) {
      printf ("File is writable by group or other: %s.\n",
              fileName);
      return(-2);
   }
   return(0);
}

int 
msiFreeBuffer(msParam_t* memoryParam, ruleExecInfo_t *rei)
{


  RE_TEST_MACRO ("Loopback on msiAssociateKeyValuePairsToObj");


  if (memoryParam->inpOutBuf != NULL)
    free(memoryParam->inpOutBuf);
  memoryParam->inpOutBuf = NULL;
  return(0);

}

int
msiSleep(msParam_t* secPtr, msParam_t* microsecPtr,  ruleExecInfo_t *rei)
{

  int sec, microsec;

  sec = atoi(secPtr->inOutStruct);
  microsec = atoi(microsecPtr->inOutStruct);

  rodsSleep (sec, microsec);
  return(0);
}

int
msiApplyAllRules(msParam_t *actionParam, msParam_t* reiSaveFlagParam, 
		 msParam_t* allRuleExecFlagParam, ruleExecInfo_t *rei)
{
  int i;
  char *action;
  int reiSaveFlag;
  int allRuleExecFlag;

  action = actionParam->inOutStruct;
  reiSaveFlag = atoi(reiSaveFlagParam->inOutStruct);
  allRuleExecFlag = atoi(allRuleExecFlagParam->inOutStruct);
  i = applyAllRules(action, rei->msParamArray, rei, reiSaveFlag,allRuleExecFlag);
  return(i);

}
