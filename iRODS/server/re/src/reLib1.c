/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobals.h"
#include "initServer.h"
#include "reHelpers1.h"

#ifdef MYMALLOC
# Within reLib1.c here, change back the redefines of malloc back to normal
#define malloc(x) malloc(x)
#define free(x) free(x)
#endif


#if 0
int 
applyActionCall(char *actionCall,  ruleExecInfo_t *rei, int reiSaveFlag)
{
  char *args[MAX_NUM_OF_ARGS_IN_ACTION];
  char action[MAX_ACTION_SIZE];  
  int i, argc;

  i = parseAction(actionCall,action,args, &argc);
  if (i != 0)
    return(i);

  i  = applyRuleArg(action, args, argc, rei, reiSaveFlag);

  return(i);
}


int
applyRule(char *action, char *args[MAX_NUM_OF_ARGS_IN_ACTION], int argc,
	  ruleExecInfo_t *rei, int reiSaveFlag)
{
  int ruleInx, i, status;
  char *nextRule;
  ruleInx = -1; /* new rule */
  char ruleCondition[MAX_RULE_LENGTH * 3];
  char ruleAction[MAX_RULE_LENGTH * 3];
  char ruleRecovery[MAX_RULE_LENGTH * 3];
  char ruleHead[MAX_RULE_LENGTH * 3]; 
  char ruleBase[MAX_RULE_LENGTH * 3]; 
  int  first = 0;
  ruleExecInfo_t  *saveRei;
  int reTryWithoutRecovery = 0;
  funcPtr myFunc = NULL;
  int actionInx;
  int numOfStrArgs;
  int ii;

  mapExternalFuncToInternalProc(action);

  i = findNextRule (action,  &ruleInx);
  if (i != 0) {
    /* probably a microservice */
    i = executeMicroService (action,args,argc,rei);
    if (i < 0) {
      rodsLog (LOG_NOTICE,"applyRule Failed for action: %s with status %i",action,i);
    }
    return(i);
  }

  while (i == 0) {
    getRule(ruleInx, ruleBase,ruleHead, ruleCondition,ruleAction, ruleRecovery, MAX_RULE_LENGTH * 3);

    i  = initializeMsParam(ruleHead,args,argc, rei);
    if (i != 0) {
      rodsLog (LOG_NOTICE,"applyRule Failed in  initializeMsParam for action: %s with status %i",action,i);
      return(i);
    }
    /*****
    i = checkRuleHead(ruleHead,args,argc);
    if (i == 0) {
    ******/
    if (reTestFlag > 0) {
	  if (reTestFlag == COMMAND_TEST_1) 
	    fprintf(stdout,"+Testing Rule Number:%i for Action:%s\n",ruleInx,action);
	  else if (reTestFlag == HTML_TEST_1)
	    fprintf(stdout,"+Testing Rule Number:<FONT COLOR=#FF0000>%i</FONT> for Action:<FONT COLOR=#0000FF>%s</FONT><BR>\n",ruleInx,action);
	  else if (rei != NULL && rei->rsComm != NULL && rei->rsComm->rError != NULL)
	    rodsLog(LOG_NOTICE,&(rei->rsComm->rError),-1,"+Testing Rule Number:%i for Action:%s\n",ruleInx,action);
	}

      i = checkRuleCondition(action,  ruleCondition, args, argc, rei, reiSaveFlag);
      if (i == TRUE) {
	if (reiSaveFlag == SAVE_REI) {
	  if (first == 0 ) {
	    saveRei = (ruleExecInfo_t  *) mallocAndZero(sizeof(ruleExecInfo_t));
	    i = copyRuleExecInfo(rei,saveRei);
	    first = 1;
	  }
	  else if (reTryWithoutRecovery == 0) {
	    i = copyRuleExecInfo(saveRei,rei);
	  }
	}
	if (reTestFlag > 0) {
	  if (reTestFlag == COMMAND_TEST_1) 
	    fprintf(stdout,"+Executing Rule Number:%i for Action:%s\n",ruleInx,action);
	  else if (reTestFlag == HTML_TEST_1)
	    fprintf(stdout,"+Executing Rule Number:<FONT COLOR=#FF0000>%i</FONT> for Action:<FONT COLOR=#0000FF>%s</FONT><BR>\n",ruleInx,action);
	  else if (rei != NULL && rei->rsComm != NULL && rei->rsComm->rError != NULL)
	    rodsLog (LOG_NOTICE,"+Executing Rule Number:%i for Action:%s\n",ruleInx,action);
	}
	status = 
	   executeRuleBody(action, ruleAction, ruleRecovery, args, argc, rei, reiSaveFlag);
	if ( status == 0) {
	  if (reiSaveFlag == SAVE_REI)
	    freeRuleExecInfoStruct(saveRei, 0);
	  finalizeMsParam(ruleHead,args,argc, rei,status);
	  return(status);
	}
	else if ( status == CUT_ACTION_PROCESSED_ERR) {
	  if (reiSaveFlag == SAVE_REI)
	    freeRuleExecInfoStruct(saveRei, 0);
	  finalizeMsParam(ruleHead,args,argc, rei,status);
	  rodsLog (LOG_NOTICE,"applyRule Failed for action : %s with status %i",action, status);
	  return(status);
	}
	if ( status == RETRY_WITHOUT_RECOVERY_ERR)
	  reTryWithoutRecovery = 1;
	  finalizeMsParam(ruleHead,args,argc, rei,0);
      }
      /*****
    }
      *****/
    i = findNextRule (action,  &ruleInx);
  }
  finalizeMsParam(ruleHead,args,argc, rei,status);
  if (first == 1) {
    if (reiSaveFlag == SAVE_REI)
      freeRuleExecInfoStruct(saveRei, 0);
  }
  if (i == NO_MORE_RULES_ERR)
    return(i);
  return(status);

}
#endif

int 
applyRuleArg(char *action, char *args[MAX_NUM_OF_ARGS_IN_ACTION], int argc,
	       ruleExecInfo_t *rei, int reiSaveFlag)
{
  msParamArray_t *inMsParamArray = NULL;
  int i;

  i = applyRuleArgPA(action ,args,  argc, inMsParamArray, rei, reiSaveFlag);
  return(i);
}


int 
applyRuleArgPA(char *action, char *args[MAX_NUM_OF_ARGS_IN_ACTION], int argc,
		      msParamArray_t *inMsParamArray, ruleExecInfo_t *rei, int reiSaveFlag)
{
  int i;
  int pFlag = 0;
  msParam_t *mP;
  char tmpStr[MAX_ACTION_SIZE];

  if (inMsParamArray == NULL) {
    inMsParamArray = mallocAndZero(sizeof(msParamArray_t));
    pFlag = 1;
  }
  for (i = 0; i < argc ; i++) {
    if (args[i][0] == '*') { 
      if ((mP = getMsParamByLabel (inMsParamArray, args[i])) == NULL) {
	addMsParam(inMsParamArray, args[i], NULL, NULL,NULL);
      }
    }
    else {
      addMsParam(inMsParamArray, args[i], STR_MS_T, strdup (args[i]),NULL);
    }
  }
  makeAction(tmpStr,action, args,argc, MAX_ACTION_SIZE);
  i = applyRule(tmpStr, inMsParamArray, rei, reiSaveFlag);
#if 0
  /* RAJA ADDED Jul 14, 2008 to get back the changed args */
  if (i == 0) {
    for (i = 0; i < argc ; i++) {
      if ((mP = getMsParamByLabel (inMsParamArray, args[i])) != NULL) 
	strcpy(args[i], (char *) mP->inOutStruct);
      /**** DANGER, DANGER: Potential overflow..... ****/
    }
    i = 0;
  }
  /* RAJA ADDED Jul 14, 2008 to get back the changed args */
#endif
  if (pFlag == 1)
    free(inMsParamArray);
  return(i);
  
}

int
initializeMsParamNew(char *ruleHead, char *args[MAX_NUM_OF_ARGS_IN_ACTION], int argc,
		     msParamArray_t *inMsParamArray,  ruleExecInfo_t *rei)
{
  int i;
  msParam_t *mP;
  char tmpStr[NAME_LEN];
  char *args2[MAX_NUM_OF_ARGS_IN_ACTION];
  int argc2 = 0;
  msParamArray_t *outMsParamArray;
  char *tmparg;

  /* save the old msParamArray in rei */
#ifdef ADDR_64BITS
  snprintf(tmpStr,NAME_LEN, "%lld", (rodsLong_t) rei->msParamArray);  /* pointer stored as long integer */
#else
  snprintf(tmpStr,NAME_LEN, "%d", (uint) rei->msParamArray);  /* pointer stored as long integer */
#endif
  pushStack(&msParamStack,tmpStr);            /* pointer->integer->string stored in stack */

  /* make a new msParamArray in rei */
  rei->msParamArray = malloc(sizeof(msParamArray_t));
  rei->msParamArray->len = 0;
  rei->msParamArray->msParam = NULL;
  outMsParamArray = rei->msParamArray;


  parseAction(ruleHead, tmpStr,args2, &argc2);
  
  /* stick things into msParamArray in rei */
  /**** changed by RAJA Jul 11, 2007 so that conversion of values in the internal strings also happen ****
  for (i = 0; i < argc ; i++) {
    if ((mP = getMsParamByLabel (inMsParamArray, args[i])) != NULL) {
      addMsParam(outMsParamArray,args2[i],mP->type, mP->inOutStruct, mP->inpOutBuf);
    }
    else {
      addMsParam(outMsParamArray, args2[i], NULL, NULL,NULL);
    }
  }
  **** changed by RAJA Jul 11, 2007 so that conversion of values in the internal strings also happen ****/
  for (i = 0; i < argc ; i++) {
    if ((mP = getMsParamByLabel (inMsParamArray, args[i])) != NULL) {
      tmparg = NULL;
      if (mP->inOutStruct == NULL || (!strcmp(mP->type, STR_MS_T) 
				      && !strcmp(mP->inOutStruct,mP->label) )) {
	convertArgWithVariableBinding(args[i],&tmparg,inMsParamArray,rei);
	if (tmparg != NULL)
	  addMsParam(outMsParamArray,args2[i],mP->type, tmparg, mP->inpOutBuf);
	else
	  addMsParam(outMsParamArray,args2[i],mP->type, mP->inOutStruct, mP->inpOutBuf);
      }
      else if (!strcmp(mP->type, STR_MS_T) ) {
	convertArgWithVariableBinding(mP->inOutStruct,&tmparg,inMsParamArray,rei);
	if (tmparg != NULL) 
	  addMsParam(outMsParamArray,args2[i],mP->type, tmparg, mP->inpOutBuf);
	else
	  addMsParam(outMsParamArray,args2[i],mP->type, mP->inOutStruct, mP->inpOutBuf);
      }
      else
	addMsParam(outMsParamArray,args2[i],mP->type, mP->inOutStruct, mP->inpOutBuf);
      if (tmparg != NULL)
	free(tmparg);
    }
    else {
	addMsParam(outMsParamArray, args2[i], NULL, NULL,NULL);
    }
  }
  /* RAJA added July 11 2007 to make sure that ruleExecOut is apassed along */
  if ((mP = getMsParamByLabel (inMsParamArray, "ruleExecOut")) != NULL) 
    /*    if (getMsParamByLabel (outMsParamArray,"ruleExecOut") != NULL)  RAJA CHANGED Sep 25 2007 ***/
    if (getMsParamByLabel (outMsParamArray,"ruleExecOut") == NULL)
      addMsParam(outMsParamArray,"ruleExecOut",mP->type, mP->inOutStruct, mP->inpOutBuf);
  /* RAJA added July 11 2007 to make sure that ruleExecOut is apassed along */

  freeRuleArgs (args2, argc2);
  return (0);
}


int
finalizeMsParamNew(char *inAction,char *ruleHead, 
		   msParamArray_t *inMsParamArray, msParamArray_t *outMsParamArray,
		   ruleExecInfo_t *rei, int status)
{

  msParamArray_t *oldMsParamArray;
  char tmpStr[NAME_LEN];
  msParam_t *mP;
  int i;
  char *args[MAX_NUM_OF_ARGS_IN_ACTION];
  int argc = 0;
  char *args2[MAX_NUM_OF_ARGS_IN_ACTION];
  int argc2 = 0;

  parseAction(ruleHead, tmpStr,args, &argc);
  parseAction(inAction, tmpStr,args2, &argc2);

  /* get the old msParamArray  */
  popStack(&msParamStack,tmpStr);   
#ifdef ADDR_64BITS
  oldMsParamArray = (msParamArray_t *) strtoll (tmpStr, 0, 0);
#else
  oldMsParamArray = (msParamArray_t *) atoi(tmpStr);
#endif

  for (i = 0; i < argc2; i++) {
    if ((mP = getMsParamByLabel (outMsParamArray, args[i])) != NULL) {
      rmMsParamByLabel (inMsParamArray, args2[i],0);
      addMsParam(inMsParamArray, args2[i], mP->type, mP->inOutStruct, mP->inpOutBuf);
    }
  }

  /* RAJA added July 11 2007 to make sure that ruleExecOut is apassed along */
  if ((mP = getMsParamByLabel (outMsParamArray, "ruleExecOut")) != NULL) {
    rmMsParamByLabel (inMsParamArray,  "ruleExecOut",0);
    addMsParam(inMsParamArray,"ruleExecOut",mP->type, mP->inOutStruct, mP->inpOutBuf);
  }
  /* RAJA added July 11 2007 to make sure that ruleExecOut is apassed along */

  /* XXXX should use clearMsparamInRei (rei); ? */
  freeRuleArgs (args, argc);
  freeRuleArgs (args2, argc2);
  /* XXXX fix memleak. MW */
  /* free(rei->msParamArray);  no need to free outMsParamArray. It was 
   * set to rei->msParamArray */
  clearMsparamInRei (rei);
  rei->msParamArray = oldMsParamArray;
  return(0);
}


/** this was applyRulePA and got changed to applyRule ***/
int
applyRule(char *inAction, msParamArray_t *inMsParamArray,
	  ruleExecInfo_t *rei, int reiSaveFlag)
{
  int ruleInx, argc,i, status;
  /*char *nextRule; */
  char ruleCondition[MAX_RULE_LENGTH * 3];
  char ruleAction[MAX_RULE_LENGTH * 3];
  char ruleRecovery[MAX_RULE_LENGTH * 3];
  char ruleHead[MAX_RULE_LENGTH * 3]; 
  char ruleBase[MAX_RULE_LENGTH * 3]; 
  int  first = 0;
  ruleExecInfo_t  *saveRei;
  int reTryWithoutRecovery = 0;
  /*funcPtr myFunc = NULL;*/
  /*int actionInx;*/
  /*int numOfStrArgs;*/
  int ii;
  char *args[MAX_NUM_OF_ARGS_IN_ACTION];
  char action[MAX_ACTION_SIZE];  
  msParamArray_t *outMsParamArray;


  if (GlobalAllRuleExecFlag != 0) {
    ii = GlobalAllRuleExecFlag;
    i = applyAllRules(inAction, inMsParamArray, rei, reiSaveFlag, GlobalAllRuleExecFlag);
    GlobalAllRuleExecFlag = ii;
    return(i);
  }

  ruleInx = -1; /* new rule */


  if (strstr(inAction,"##") != NULL) { /* seems to be multiple actions */
    i = execMyRuleWithSaveFlag(inAction,inMsParamArray,rei,reiSaveFlag);
    return(i);
  }

  i = parseAction(inAction,action,args, &argc);
  if (i != 0)
    return(i);

  mapExternalFuncToInternalProc(action);

  i = findNextRule (action,  &ruleInx);
  if (i != 0) {
    /* probably a microservice */
#if 0
    i = executeMicroServiceNew(action,inMsParamArray,rei);
#endif
    i = executeMicroServiceNew(inAction,inMsParamArray,rei);
    return(i);
  }

  while (i == 0) {
    getRule(ruleInx, ruleBase,ruleHead, ruleCondition,ruleAction, ruleRecovery, MAX_RULE_LENGTH * 3);


    i  = initializeMsParamNew(ruleHead,args,argc, inMsParamArray, rei);
    if (i != 0)
      return(i);
    outMsParamArray = rei->msParamArray;

    /*****
    i = checkRuleHead(ruleHead,args,argc);
    freeRuleArgs (args, argc);
    if (i == 0) {
    ******/
    if (reTestFlag > 0) {
	  if (reTestFlag == COMMAND_TEST_1) 
	    fprintf(stdout,"+Testing Rule Number:%i for Action:%s\n",ruleInx,action);
	  else if (reTestFlag == HTML_TEST_1)
	    fprintf(stdout,"+Testing Rule Number:<FONT COLOR=#FF0000>%i</FONT> for Action:<FONT COLOR=#0000FF>%s</FONT><BR>\n",ruleInx,action);
	  else if (rei != 0 && rei->rsComm != 0 && &(rei->rsComm->rError) != 0)
	    rodsLog (LOG_NOTICE,"+Testing Rule Number:%i for Action:%s\n",ruleInx,action);
	}

      i = checkRuleConditionNew(action,  ruleCondition, outMsParamArray, rei, reiSaveFlag);
      if (i == TRUE) {
	if (reiSaveFlag == SAVE_REI) {
	  if (first == 0 ) {
	    saveRei = (ruleExecInfo_t  *) mallocAndZero(sizeof(ruleExecInfo_t));
	    i = copyRuleExecInfo(rei,saveRei);
	    first = 1;
	  }
	  else if (reTryWithoutRecovery == 0) {
	    i = copyRuleExecInfo(saveRei,rei);
	  }
	}
	if (reTestFlag > 0) {
	  if (reTestFlag == COMMAND_TEST_1) 
	    fprintf(stdout,"+Executing Rule Number:%i for Action:%s\n",ruleInx,action);
	  else if (reTestFlag == HTML_TEST_1)
	    fprintf(stdout,"+Executing Rule Number:<FONT COLOR=#FF0000>%i</FONT> for Action:<FONT COLOR=#0000FF>%s</FONT><BR>\n",ruleInx,action);
	  else
	    rodsLog (LOG_NOTICE,"+Executing Rule Number:%i for Action:%s\n",ruleInx,action);
	}
	status = 
	   executeRuleBodyNew(action, ruleAction, ruleRecovery, outMsParamArray, rei, reiSaveFlag);
	if ( status == 0  || status == CUT_ACTION_ON_SUCCESS_PROCESSED_ERR) {
	  if (reiSaveFlag == SAVE_REI)
	    freeRuleExecInfoStruct(saveRei, 0);
	  finalizeMsParamNew(inAction,ruleHead,inMsParamArray, outMsParamArray, rei,status);
	  return(0);
	}
	else if ( status == CUT_ACTION_PROCESSED_ERR || 
	  status == MSI_OPERATION_NOT_ALLOWED) {
	  if (reiSaveFlag == SAVE_REI)
	    freeRuleExecInfoStruct(saveRei, 0);
	  finalizeMsParamNew(inAction,ruleHead,inMsParamArray,  outMsParamArray, rei,status);
	  return(status);
	}
	if ( status == RETRY_WITHOUT_RECOVERY_ERR)
	  reTryWithoutRecovery = 1;
	finalizeMsParamNew(inAction,ruleHead,inMsParamArray,  outMsParamArray, rei,0);
        outMsParamArray = NULL;		/* set this since finalizeMsParamNew
					 * freed it.
					 */
      }
      else {/*** ADDED RAJA JUN 20, 2007 ***/
	finalizeMsParamNew(inAction,ruleHead,inMsParamArray,  outMsParamArray, rei,0);
      }
      /*****
    }
      *****/
    i = findNextRule (action,  &ruleInx);
  }

  if (first == 1) {
    if (reiSaveFlag == SAVE_REI)
      freeRuleExecInfoStruct(saveRei, 0);
  }
  if (i == NO_MORE_RULES_ERR) {
    rodsLog (LOG_NOTICE,"applyRule Failed for action 1: %s with status %i",action, i);
    return(i);
  }

  finalizeMsParamNew(inAction,ruleHead,inMsParamArray, outMsParamArray, rei,status);


  if (status < 0) {
      rodsLog (LOG_NOTICE,"applyRule Failed for action 2: %s with status %i",action, status);
  }
  return(status);
}




int
applyAllRules(char *inAction, msParamArray_t *inMsParamArray,
	  ruleExecInfo_t *rei, int reiSaveFlag, int allRuleExecFlag)
{
  int ruleInx, argc,i, status;
  /*char *nextRule;*/
  char ruleCondition[MAX_RULE_LENGTH * 3];
  char ruleAction[MAX_RULE_LENGTH * 3];
  char ruleRecovery[MAX_RULE_LENGTH * 3];
  char ruleHead[MAX_RULE_LENGTH * 3]; 
  char ruleBase[MAX_RULE_LENGTH * 3]; 
  int  first = 0;
  int  success = 0;
  ruleExecInfo_t  *saveRei;
  int reTryWithoutRecovery = 0;
  /*funcPtr myFunc = NULL;*/
  /*int actionInx;*/
  /*int numOfStrArgs;*/
  /*int ii;*/
  char *args[MAX_NUM_OF_ARGS_IN_ACTION];
  char action[MAX_ACTION_SIZE];  
  msParamArray_t *outMsParamArray;

  ruleInx = -1; /* new rule */
  outMsParamArray =  NULL;

  GlobalAllRuleExecFlag = allRuleExecFlag;

  if (strstr(inAction,"##") != NULL) { /* seems to be multiple actions */
    i = execMyRuleWithSaveFlag(inAction,inMsParamArray,rei,reiSaveFlag);
    if( GlobalAllRuleExecFlag != 9) GlobalAllRuleExecFlag = 0;
    return(i);
  }

  i = parseAction(inAction,action,args, &argc);
  if (i != 0) {
    if( GlobalAllRuleExecFlag != 9) GlobalAllRuleExecFlag = 0;
    return(i);
  }

  mapExternalFuncToInternalProc(action);

  i = findNextRule (action,  &ruleInx);
  if (i != 0) {
    /* probably a microservice */
#if 0
    i = executeMicroServiceNew(action,inMsParamArray,rei);
#endif
    i = executeMicroServiceNew(inAction,inMsParamArray,rei);
    if( GlobalAllRuleExecFlag != 9) GlobalAllRuleExecFlag = 0;
    return(i);
  }

  while (i == 0) {
    getRule(ruleInx, ruleBase,ruleHead, ruleCondition,ruleAction, ruleRecovery, MAX_RULE_LENGTH * 3);

    if (outMsParamArray == NULL) {
      i  = initializeMsParamNew(ruleHead,args,argc, inMsParamArray, rei);
      if (i != 0) {
	if( GlobalAllRuleExecFlag != 9) GlobalAllRuleExecFlag = 0;
	return(i);
      }
      outMsParamArray = rei->msParamArray;
      
    }

    /*****
    i = checkRuleHead(ruleHead,args,argc);
    freeRuleArgs (args, argc);
    if (i == 0) {
    ******/
    if (reTestFlag > 0) {
	  if (reTestFlag == COMMAND_TEST_1) 
	    fprintf(stdout,"+Testing Rule Number:%i for Action:%s\n",ruleInx,action);
	  else if (reTestFlag == HTML_TEST_1)
	    fprintf(stdout,"+Testing Rule Number:<FONT COLOR=#FF0000>%i</FONT> for Action:<FONT COLOR=#0000FF>%s</FONT><BR>\n",ruleInx,action);
	  else if (rei != 0 && rei->rsComm != 0 && &(rei->rsComm->rError) != 0)
	    rodsLog (LOG_NOTICE,"+Testing Rule Number:%i for Action:%s\n",ruleInx,action);
	}

      i = checkRuleConditionNew(action,  ruleCondition, outMsParamArray, rei, reiSaveFlag);
      if (i == TRUE) {
	if (reiSaveFlag == SAVE_REI) {
	  if (first == 0 ) {
	    saveRei = (ruleExecInfo_t  *) mallocAndZero(sizeof(ruleExecInfo_t));
	    i = copyRuleExecInfo(rei,saveRei);
	    first = 1;
	  }
	  else if (reTryWithoutRecovery == 0) {
	    i = copyRuleExecInfo(saveRei,rei);
	  }
	}
	if (reTestFlag > 0) {
	  if (reTestFlag == COMMAND_TEST_1) 
	    fprintf(stdout,"+Executing Rule Number:%i for Action:%s\n",ruleInx,action);
	  else if (reTestFlag == HTML_TEST_1)
	    fprintf(stdout,"+Executing Rule Number:<FONT COLOR=#FF0000>%i</FONT> for Action:<FONT COLOR=#0000FF>%s</FONT><BR>\n",ruleInx,action);
	  else
	    rodsLog (LOG_NOTICE,"+Executing Rule Number:%i for Action:%s\n",ruleInx,action);
	}
	status = 
	   executeRuleBodyNew(action, ruleAction, ruleRecovery, outMsParamArray, rei, reiSaveFlag);
	if ( status == 0) {
	  if (reiSaveFlag == SAVE_REI)
	    freeRuleExecInfoStruct(saveRei, 0);
	  /**** finalizeMsParamNew(inAction,ruleHead,inMsParamArray, outMsParamArray, rei,status);
		if( GlobalAllRuleExecFlag != 9) GlobalAllRuleExecFlag = 0;
		return(status); ***/
	  success = 1;
	}
	else if ( status == CUT_ACTION_PROCESSED_ERR) {
	  if (reiSaveFlag == SAVE_REI)
	    freeRuleExecInfoStruct(saveRei, 0);
	  finalizeMsParamNew(inAction,ruleHead,inMsParamArray,  outMsParamArray, rei,status);
	  if( GlobalAllRuleExecFlag != 9) GlobalAllRuleExecFlag = 0;
	  return(status);
	}
        else if ( status == CUT_ACTION_ON_SUCCESS_PROCESSED_ERR) {
          if (reiSaveFlag == SAVE_REI)
            freeRuleExecInfoStruct(saveRei, 0);
          finalizeMsParamNew(inAction,ruleHead,inMsParamArray,  outMsParamArray, rei,status);
          if( GlobalAllRuleExecFlag != 9) GlobalAllRuleExecFlag = 0;
          return(0);
        }
	else if ( status == RETRY_WITHOUT_RECOVERY_ERR) {
	  reTryWithoutRecovery = 1;
	  /*** finalizeMsParamNew(inAction,ruleHead,inMsParamArray,  outMsParamArray, rei,0);***/
	}
	/***  outMsParamArray = NULL; 	***/ /* set this since finalizeMsParamNew
					 * freed it.
					 */
      }
      else {/*** ADDED RAJA JUN 20, 2007 ***/
	/*** finalizeMsParamNew(inAction,ruleHead,inMsParamArray,  outMsParamArray, rei,0); ***/
      }
      /*****
    }
      *****/
    i = findNextRule (action,  &ruleInx);
  }

  if (first == 1) {
    if (reiSaveFlag == SAVE_REI)
      freeRuleExecInfoStruct(saveRei, 0);
  }
  if (i == NO_MORE_RULES_ERR) {
    if( GlobalAllRuleExecFlag != 9) GlobalAllRuleExecFlag = 0;
    if (success == 1) {
      finalizeMsParamNew(inAction,ruleHead,inMsParamArray, outMsParamArray, rei,status);
      return(0);
    }
    else {
      rodsLog (LOG_NOTICE,"applyRule Failed for action 3: %s with status %i",action, i);
      return(i);
    }
  }

  finalizeMsParamNew(inAction,ruleHead,inMsParamArray, outMsParamArray, rei,status);

  if (status < 0) {
      rodsLog (LOG_NOTICE,"applyRule Failed for action 4: %s with status %i",action, status);
  }
  if( GlobalAllRuleExecFlag != 9) GlobalAllRuleExecFlag = 0;
  if (success == 1)
    return(0);
  else
    return(status);
}


int
execMyRule(char * ruleDef, msParamArray_t *inMsParamArray,
	  ruleExecInfo_t *rei)
{

  return(execMyRuleWithSaveFlag(ruleDef,inMsParamArray,rei,0));
}

int
execMyRuleWithSaveFlag(char * ruleDef, msParamArray_t *inMsParamArray,
	  ruleExecInfo_t *rei,int reiSaveFlag)

{
  int i;
  char *inAction;
  char *ruleBase;
  char *ruleHead;
  char *ruleCondition;
  char *ruleAction;
  char *ruleRecovery;
  char buf[MAX_RULE_LENGTH];
  char l0[MAX_RULE_LENGTH];
  char l1[MAX_RULE_LENGTH];
  char l2[MAX_RULE_LENGTH];
  char l3[MAX_RULE_LENGTH];
  int status;
  char action[MAX_ACTION_SIZE];  
  char *args[MAX_NUM_OF_ARGS_IN_ACTION];
  int argc;


  rstrcpy(buf, ruleDef, MAX_RULE_LENGTH);
  
  if (strstr(buf,"|") == NULL) {
    status = applyRule(ruleDef, inMsParamArray, rei,1);
    return(status);
  }


  rSplitStr(buf, l1, MAX_RULE_LENGTH, l0, MAX_RULE_LENGTH, '|');
  inAction = strdup(l1);  /** action **/
  ruleBase = strdup("MyRule");
  ruleHead = strdup(l1);  /** ruleHead **/
  rSplitStr(l0, l1, MAX_RULE_LENGTH, l3, MAX_RULE_LENGTH,'|');
  ruleCondition = strdup(l1); /** condition **/
  rSplitStr(l3, l1, MAX_RULE_LENGTH, l2, MAX_RULE_LENGTH, '|');
  ruleAction = strdup(l1);  /** function calls **/
  ruleRecovery = strdup(l2);  /** rollback calls **/
  if (strlen(ruleAction) == 0 && strlen(ruleRecovery) == 0) {
    ruleRecovery = ruleCondition;
    ruleAction = inAction;
    inAction = strdup("");
    ruleCondition = strdup("");
  }
  else if (strlen(ruleRecovery) == 0) {
    ruleRecovery = ruleAction;
    ruleAction = ruleCondition;
    ruleCondition = inAction;
    inAction = strdup("");
  }

  /*
  rodsLog(LOG_NOTICE,"PPP:%s::%s::%s::%s\n",inAction,ruleCondition,ruleAction,ruleRecovery);
  */
  i = parseAction(inAction,action,args, &argc);
  if (i != 0)
    return(i);
  
  freeRuleArgs (args, argc);

  if (reTestFlag > 0) {
    if (reTestFlag == COMMAND_TEST_1) 
      fprintf(stdout,"+Testing MyRule Rule for Action:%s\n",inAction);
    else if (reTestFlag == HTML_TEST_1)
      fprintf(stdout,"+Testing MyRule for Action:<FONT COLOR=#0000FF>%s</FONT><BR>\n",inAction);
    else if (rei != NULL && rei->rsComm != NULL && &(rei->rsComm->rError) != NULL)
      rodsLog (LOG_NOTICE,"+Testing MyRule for Action:%s\n",inAction);
  }

  i = checkRuleConditionNew(action,  ruleCondition, inMsParamArray, rei, reiSaveFlag);
  if (i == TRUE) {
    if (reTestFlag > 0) {
      if (reTestFlag == COMMAND_TEST_1) 
	fprintf(stdout,"+Executing MyRule  for Action:%s\n",action);
	  else if (reTestFlag == HTML_TEST_1)
	    fprintf(stdout,"+Executing MyRule for Action:<FONT COLOR=#0000FF>%s</FONT><BR>\n",action);
	  else if (rei != NULL && rei->rsComm != NULL && &(rei->rsComm->rError) != NULL)
	    rodsLog (LOG_NOTICE,"+Executing MyRule for Action:%s\n",action);
    }
    status = 
	   executeMyRuleBody(action, ruleAction, ruleRecovery, inMsParamArray, rei, reiSaveFlag);
    if (status < 0) {
      rodsLog (LOG_NOTICE,"execMyRule %s Failed with status %i",ruleDef, status);
    }
    return(status);
  }
  else {
    rodsLog (LOG_NOTICE,"execMyRule %s Failed  with status %i",ruleDef, i);
    return (RULE_FAILED_ERR);
  }
}

int
initRuleStruct(char *irbSet, char *dvmSet, char *fnmSet)
{
  int i;
  char r1[NAME_LEN], r2[RULE_SET_DEF_LENGTH], r3[RULE_SET_DEF_LENGTH];
  
  strcpy(r2,irbSet);
  coreRuleStrct.MaxNumOfRules = 0;
  appRuleStrct.MaxNumOfRules = 0;
  GlobalAllRuleExecFlag = 0;

  while (strlen(r2) > 0) {
    i = rSplitStr(r2,r1,NAME_LEN,r3,RULE_SET_DEF_LENGTH,',');
    if (i == 0)
      i = readRuleStructFromFile(r1, &coreRuleStrct);
    if (i != 0)
      return(i);
    strcpy(r2,r3);
  }
  strcpy(r2,dvmSet);
  coreRuleVarDef.MaxNumOfDVars = 0;
  appRuleVarDef.MaxNumOfDVars = 0;
  
  while (strlen(r2) > 0) {
    i = rSplitStr(r2,r1,NAME_LEN,r3,RULE_SET_DEF_LENGTH,',');
    if (i == 0)
      i = readDVarStructFromFile(r1, &coreRuleVarDef);
    if (i != 0)
      return(i);
    strcpy(r2,r3);
  }
  strcpy(r2,fnmSet);
  coreRuleFuncMapDef.MaxNumOfFMaps = 0;
  appRuleFuncMapDef.MaxNumOfFMaps = 0;
  
  while (strlen(r2) > 0) {
    i = rSplitStr(r2,r1,NAME_LEN,r3,RULE_SET_DEF_LENGTH,',');
    if (i == 0)
      i = readFuncMapStructFromFile(r1, &coreRuleFuncMapDef);
    if (i != 0)
      return(i);
    strcpy(r2,r3);
  }
  if (getenv(RETESTFLAG) != NULL) {
    reTestFlag = atoi(getenv(RETESTFLAG));
    if (getenv(RELOOPBACKFLAG) != NULL)
      reLoopBackFlag = atoi(getenv(RELOOPBACKFLAG));
    else
      reLoopBackFlag = 0;
  }
  else {
    reTestFlag = 0;
    reLoopBackFlag = 0;
  }
  if (getenv("GLOBALALLRULEEXECFLAG") != NULL)
    GlobalAllRuleExecFlag = 9;


  delayStack.size = NAME_LEN;
  delayStack.len = 0;
  delayStack.value = NULL;

  msParamStack.size = NAME_LEN;
  msParamStack.len = 0;
  msParamStack.value = NULL;


  return(0);
}

int
readRuleStructFromFile(char *ruleBaseName, ruleStruct_t *inRuleStrct)
{
  int i = 0;
  char l0[MAX_RULE_LENGTH];
  char l1[MAX_RULE_LENGTH];
  char l2[MAX_RULE_LENGTH];
  char l3[MAX_RULE_LENGTH];
   char rulesFileName[MAX_NAME_LEN];
   FILE *file;
   char buf[MAX_RULE_LENGTH];
   char *configDir;
   char *t;
   i = inRuleStrct->MaxNumOfRules;

   configDir = getConfigDir ();
   snprintf (rulesFileName,MAX_NAME_LEN, "%s/reConfigs/%s.irb", configDir,ruleBaseName);

   file = fopen(rulesFileName, "r");
   if (file == NULL) {
     rodsLog(LOG_NOTICE,
	     "readRuleStructFromFile() could not open rules file %s\n",
	     rulesFileName);
     return(RULES_FILE_READ_ERROR);
   }
   buf[MAX_RULE_LENGTH-1]='\0';
   while (fgets (buf, MAX_RULE_LENGTH-1, file) != NULL) {
     if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';
     if (buf[0] == '#' || strlen(buf) < 4) 
       continue;
     rSplitStr(buf, l1, MAX_RULE_LENGTH, l0, MAX_RULE_LENGTH, '|');
     inRuleStrct->action[i] = strdup(l1);  /** action **/
     inRuleStrct->ruleHead[i] = strdup(l1);
     if ((t = strstr(inRuleStrct->action[i],"(")) != NULL) {
       *t = '\0';
     }
     /*****     rSplitStr(l0, l1, MAX_RULE_LENGTH, l2, MAX_RULE_LENGTH, '|'); 
		inRuleStrct->ruleHead[i] = strdup(l1);
		rSplitStr(l2, l1, MAX_RULE_LENGTH, l3, MAX_RULE_LENGTH,'|');
      *****/
     inRuleStrct->ruleBase[i] = strdup(ruleBaseName);
     rSplitStr(l0, l1, MAX_RULE_LENGTH, l3, MAX_RULE_LENGTH,'|');
     inRuleStrct->ruleCondition[i] = strdup(l1); /** condition **/
     rSplitStr(l3, l1, MAX_RULE_LENGTH, l2, MAX_RULE_LENGTH, '|');
     inRuleStrct->ruleAction[i] = strdup(l1);  /** function calls **/
     inRuleStrct->ruleRecovery[i] = strdup(l2);  /** rollback calls **/
     i++;
   }
   fclose (file);
   inRuleStrct->MaxNumOfRules = i;
   return(0);
}

int
clearRuleStruct(ruleStruct_t *inRuleStrct)
{
  int i;
  for (i = 0 ; i < inRuleStrct->MaxNumOfRules ; i++) {
    if (inRuleStrct->ruleBase[i]  != NULL)
      free(inRuleStrct->ruleBase[i]);
    if (inRuleStrct->ruleHead[i]  != NULL)
      free(inRuleStrct->ruleHead[i]);
    if (inRuleStrct->ruleCondition[i]  != NULL)
      free(inRuleStrct->ruleCondition[i]);
    if (inRuleStrct->ruleAction[i]  != NULL)
      free(inRuleStrct->ruleAction[i]);
    if (inRuleStrct->ruleRecovery[i]  != NULL)
      free(inRuleStrct->ruleRecovery[i]);
    
  }
  inRuleStrct->MaxNumOfRules  = 0;
  return(0);
}

int clearDVarStruct(rulevardef_t *inRuleVarDef) 
{
  int i;
  for (i = 0 ; i < inRuleVarDef->MaxNumOfDVars; i++) {
    if (inRuleVarDef->varName[i] != NULL)
      free(inRuleVarDef->varName[i]);
    if (inRuleVarDef->action[i] != NULL)
      free(inRuleVarDef->action[i]);
    if (inRuleVarDef->var2CMap[i] != NULL)
      free(inRuleVarDef->var2CMap[i]);
  }
  inRuleVarDef->MaxNumOfDVars =  0;
  return(0);
}

int clearFuncMapStruct( rulefmapdef_t* inRuleFuncMapDef)
{
  int i;
  for (i = 0 ; i < inRuleFuncMapDef->MaxNumOfFMaps; i++) {
    if (inRuleFuncMapDef->funcName[i] != NULL)
      free(inRuleFuncMapDef->funcName[i]);
    if (inRuleFuncMapDef->func2CMap[i] != NULL)
      free(inRuleFuncMapDef->func2CMap[i]);
  }
  inRuleFuncMapDef->MaxNumOfFMaps = 0;
  return(0);
}


int
readDVarStructFromFile(char *dvarBaseName,rulevardef_t *inRuleVarDef)
{
  int i = 0;
  char l0[MAX_DVAR_LENGTH];
  char l1[MAX_DVAR_LENGTH];
  /*  char l2[MAX_DVAR_LENGTH];*/
  char l3[MAX_DVAR_LENGTH];
   char dvarsFileName[MAX_NAME_LEN];
   FILE *file;
   char buf[MAX_DVAR_LENGTH];
   char *configDir;

   i = inRuleVarDef->MaxNumOfDVars;

   configDir = getConfigDir ();
   snprintf (dvarsFileName,MAX_NAME_LEN, "%s/reConfigs/%s.dvm", configDir,dvarBaseName);

   file = fopen(dvarsFileName, "r");
   if (file == NULL) {
     rodsLog(LOG_NOTICE,
	     "readDvarStructFromFile() could not open dvm file %s\n",
	     dvarsFileName);
     return(DVARMAP_FILE_READ_ERROR);
   }
   buf[MAX_DVAR_LENGTH-1]='\0';
   while (fgets (buf, MAX_DVAR_LENGTH-1, file) != NULL) {
     if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';
     if (buf[0] == '#' || strlen(buf) < 4) 
       continue;
     rSplitStr(buf, l1, MAX_DVAR_LENGTH, l0, MAX_DVAR_LENGTH, '|');
     inRuleVarDef->varName[i] = strdup(l1);  /** varName **/
     rSplitStr(l0, l1, MAX_DVAR_LENGTH, l3, MAX_DVAR_LENGTH,'|');
     inRuleVarDef->action[i] = strdup(l1); /** action **/
     inRuleVarDef->var2CMap[i] = strdup(l3);  /** var2CMap **/
     i++;
   }
   fclose (file);
   inRuleVarDef->MaxNumOfDVars = i;
   return(0);
}

int
readFuncMapStructFromFile(char *fmapBaseName, rulefmapdef_t* inRuleFuncMapDef)
{
  int i = 0;
  char l0[MAX_FMAP_LENGTH];
  char l1[MAX_FMAP_LENGTH];
  /*char l2[MAX_FMAP_LENGTH];
    char l3[MAX_FMAP_LENGTH];*/
   char fmapsFileName[MAX_NAME_LEN];
   FILE *file;
   char buf[MAX_FMAP_LENGTH];
   char *configDir;

   i = inRuleFuncMapDef->MaxNumOfFMaps;

   configDir = getConfigDir ();
   snprintf (fmapsFileName,MAX_NAME_LEN, "%s/reConfigs/%s.fnm", configDir,fmapBaseName);

   file = fopen(fmapsFileName, "r");
   if (file == NULL) {
     rodsLog(LOG_NOTICE,
	     "readFmapStructFromFile() could not open fnm file %s\n",
	     fmapsFileName);
     return(FMAP_FILE_READ_ERROR);
   }
   buf[MAX_FMAP_LENGTH-1]='\0';
   while (fgets (buf, MAX_FMAP_LENGTH-1, file) != NULL) {
     if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';
     if (buf[0] == '#' || strlen(buf) < 4) 
       continue;
     rSplitStr(buf, l1, MAX_FMAP_LENGTH, l0, MAX_FMAP_LENGTH, '|');
     inRuleFuncMapDef->funcName[i] = strdup(l1);  /** funcName **/
     inRuleFuncMapDef->func2CMap[i] = strdup(l0);  /** func2CMap **/
     i++;
   }
   fclose (file);
   inRuleFuncMapDef->MaxNumOfFMaps = i;
   return(0);
}

int
findNextRule (char *action,  int *ruleInx)
{
  int i;
   i = *ruleInx;
   i++;

   if (i < 0)
     i = 0;
   if (i < 1000) {
     for( ; i < appRuleStrct.MaxNumOfRules; i++) {
       if (!strcmp( appRuleStrct.action[i],action)) {
	 *ruleInx = i;
	 return(0);
       }
     }
     i = 1000;
   }
   i  = i - 1000;
   for( ; i < coreRuleStrct.MaxNumOfRules; i++) {
     if (!strcmp( coreRuleStrct.action[i],action)) {
       *ruleInx = i + 1000;
       return(0);
     }
   }
   return(NO_MORE_RULES_ERR);
}



int
getRule(int ri, char *ruleBase, char *ruleHead, char *ruleCondition, 
	char *ruleAction, char *ruleRecovery, int rSize)
{

  if (ri < 1000) {
    rstrcpy( ruleBase , appRuleStrct.ruleBase[ri], rSize);
    rstrcpy( ruleHead , appRuleStrct.ruleHead[ri], rSize);
    rstrcpy( ruleCondition , appRuleStrct.ruleCondition[ri], rSize);
    rstrcpy( ruleAction , appRuleStrct.ruleAction[ri], rSize);
    rstrcpy( ruleRecovery , appRuleStrct.ruleRecovery[ri], rSize);
  }
  else {
    ri = ri - 1000;
    rstrcpy( ruleBase , coreRuleStrct.ruleBase[ri], rSize);
    rstrcpy( ruleHead , coreRuleStrct.ruleHead[ri], rSize);
    rstrcpy( ruleCondition , coreRuleStrct.ruleCondition[ri], rSize);
    rstrcpy( ruleAction , coreRuleStrct.ruleAction[ri], rSize);
    rstrcpy( ruleRecovery , coreRuleStrct.ruleRecovery[ri], rSize);
  }
  return(0);
}








int
initializeMsParam(char *ruleHead, char *args[MAX_NUM_OF_ARGS_IN_ACTION], int argc,
	  ruleExecInfo_t *rei)
{

  msParam_t *mP;
  char tmpStr[NAME_LEN];
  char *args2[MAX_NUM_OF_ARGS_IN_ACTION];
  int argc2 = 0;
  int i;

  /* save the old msParamArray in rei */
#ifdef ADDR_64BITS
  snprintf(tmpStr,NAME_LEN, "%lld", (rodsLong_t) rei->msParamArray);  /* pointer stored as long integer */
#else
  snprintf(tmpStr,NAME_LEN, "%d", (uint) rei->msParamArray);  /* pointer stored as long integer */
#endif
  pushStack(&msParamStack,tmpStr);            /* pointer->integer->string stored in stack */
  
  /* make a new msParamArray in rei */
  rei->msParamArray = malloc(sizeof(msParamArray_t));
  rei->msParamArray->len = 0;
  rei->msParamArray->msParam = NULL;

  parseAction(ruleHead, tmpStr,args2, &argc2);
  
  if (argc < argc2)
    return(INSUFFICIENT_INPUT_ARG_ERR);

  /* stick things into msParamArray in rei */
  for (i = 0; i < argc2 ; i++) {
    if (args[i][0] == '*') {
      /* this is an output and hence an empty struct is added here */
      addMsParam(rei->msParamArray, args2[i], "", NULL,NULL);
    }
    else {
      mP = (msParam_t *) args[i];
      addMsParam(rei->msParamArray, args2[i], "int *msParam", mP, NULL);
    }
  }
  freeRuleArgs (args2, argc2);

  return(0);
  
}

int
finalizeMsParam(char *ruleHead, char *args[MAX_NUM_OF_ARGS_IN_ACTION], int argc,
	  ruleExecInfo_t *rei, int status)
{

  msParamArray_t *oldMsParamArray;
  char tmpStr[NAME_LEN];
  msParam_t *mP;
  int i;
  char *args2[MAX_NUM_OF_ARGS_IN_ACTION];
  int argc2 = 0;

  parseAction(ruleHead, tmpStr,args2, &argc2);

  /* get the old msParamArray  */
  popStack(&msParamStack,tmpStr);   
#ifdef ADDR_64BITS
  oldMsParamArray = (msParamArray_t *) strtoll (tmpStr, 0, 0);
#else
  oldMsParamArray = (msParamArray_t *) atoi(tmpStr);
#endif

  for (i = 0; i < argc; i++) {
    if (args[i][0] == '*') { /* it has not been bound  and hence an output */
      mP = getMsParamByLabel (rei->msParamArray, args2[i]);
      rmMsParamByLabel (oldMsParamArray, args[i], 0);
      addMsParam(oldMsParamArray, args[i], mP->type, mP->inOutStruct, mP->inpOutBuf);
    }
    else {
      /* currently nothing, should I also copy fromnew to old
	 because something changed. for the present it is considered a constant */
    }
  }
  freeRuleArgs (args2, argc2);

  free(rei->msParamArray);
  rei->msParamArray = oldMsParamArray;
  return(0);
}



void *
mymalloc(char *file,int line, int x)
{
    void *p;
        p = malloc(x);
        rodsLog(LOG_NOTICE, "MYMALLOC: %s:%i:%i=%x",file,line,x,p);
       return(p);
}

void
myfree(char *file,int line, void* p)
{
      rodsLog(LOG_NOTICE, "MYFREE: %s:%i=%x",file,line, p);
        free(p);
}
void *
mystrdup(char *file,int line, char *x)
{
    void *p;
        p = strdup(x);
        rodsLog(LOG_NOTICE, "MYSTRDUP: %s:%i=%d",file,line,p);
       return(p);
}
