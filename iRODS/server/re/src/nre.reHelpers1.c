/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "reHelpers1.h"
#include "index.h"

char *breakPoints[100];
int breakPointsInx = 0;
char myHostName[MAX_NAME_LEN];
char waitHdr[HEADER_TYPE_LEN];
char waitMsg[MAX_NAME_LEN];
int myPID;



int
convertArgWithVariableBinding(char *inS, char **outS, msParamArray_t *inMsParamArray, ruleExecInfo_t *rei)
{
  char *tS;
  int i;
 


  tS =  (char*)malloc(strlen(inS)  + 4 * MAX_COND_LEN);
  strcpy(tS,inS);

  i = replaceVariablesAndMsParams("", tS, inMsParamArray, rei);
  if (i < 0 || !strcmp(tS, inS)) {
    free(tS);
    *outS = NULL;
  }
  else
    *outS = tS;
  return(i);
}

/*void removeEscape(char *expr) {
    int i=0; // index for the original string
    int j=0; // index for the new string
    while(expr[i]!='\0') {
        switch(expr[i]) {
            case '\\':
                switch(expr[i+1]) {
                    case '\'':
                    case '\"':
                    case '\\':
                        i++; // skip backslash
                        continue;
                }
        }
        // copy from the original string to the new string
        expr[j++] = expr[i++];
    }
    expr[j]='\0';

}*/




int
replaceVariablesAndMsParams(char *action, char *inStr, msParamArray_t *inMsParamArray, ruleExecInfo_t *rei )
{
  int j;
  char *t1, *t2;

  t1 = inStr;
  j = 0;

  while ((t2 = strstr(t1,"$")) != NULL) {
    j = replaceSessionVar(action,t2,(int) (MAX_COND_LEN - (t2 - inStr) - 1),rei);
    if (j < 0) {
      rodsLog (LOG_NOTICE,"replaceSessionVar Failed at %s: %i\n", (char *)t2,j);
      return(j);
    }
    t1 = t2 + 1;
  }
  j  = replaceMsParams(inStr,inMsParamArray);
  return(j);
}

int
replaceMsParams(char *inStr, msParamArray_t *inMsParamArray)
{
  int j;
  char *t1, *t2;

  t1 = inStr;

  while ((t2 = strstr(t1,"*")) != NULL) {
    if (*(t2 + 1) == ' ') {
      t1 = t2 + 1;
      continue;
    }
    j = replaceStarVar(inStr,t2,(int) (MAX_COND_LEN - (t2 - inStr) - 1), inMsParamArray);
    if (j < 0) {
      rodsLog (LOG_NOTICE,"replaceMsParams Failed at %s: %i\n", (char *)t2,j);
      return(j);
    }
    t1 = t2 + 1;
  }
  return(0);
}






int
reREMatch(char *pat, char *str)
{
  int i;

  i = match(pat,str);
  if (i == MATCH_VALID)
    return(TRUE);
  else
    return(FALSE);
  /*
    printf("1:%sPPPP%sNNNN\n%i::%i\n",pat,str,strlen(pat),strlen(str));
  i = matche(pat,str);
  if (i == MATCH_PATTERN) {
    is_valid_pattern(pat,&j);
    if(j != MATCH_ABORT && j != MATCH_END) {
      rodsLog (LOG_NOTICE,"reREMatch:Pattern Problems:%i\n",j);
      rodsLog (LOG_NOTICE,"Pat:(%s)Str:(%s)::PatLen=%i::StrLen=%i\n",
	       pat,str,strlen(pat),strlen(str));
    }
    return(0);
  }
  else if (i == MATCH_VALID)
    return(TRUE);
  else {
    if(i != MATCH_ABORT && i != MATCH_END) {
      rodsLog (LOG_NOTICE,"reREMatch:Match Problem:%i\n",i);
      rodsLog (LOG_NOTICE,"Pat:(%s)Str:(%s)::PatLen=%i::StrLen=%i\n",
	       pat,str,strlen(pat),strlen(str));
    }
    return(FALSE);
  }
  */

}


int initializeReDebug(rsComm_t *svrComm, int flag) 
{
  char condRead[NAME_LEN];
  int i, s,m, status;
  char *readhdr = NULL;
  char *readmsg = NULL;
  char *user = NULL;
  char *addr = NULL;

  if (svrComm == NULL)
    return(0);
  if ( GlobalREDebugFlag != 4 ) 
    return(0);

  s=0;
  m=0;
  myPID = (int) getpid();
  myHostName[0] = '\0';
  gethostname (myHostName, MAX_NAME_LEN);
  sprintf(condRead, "(*XUSER  == \"%s@%s\") && (*XHDR == \"STARTDEBUG\")",
	  svrComm->clientUser.userName, svrComm->clientUser.rodsZone);
  status = _readXMsg(GlobalREDebugFlag, condRead, &m, &s, &readhdr, &readmsg, &user, &addr);
  if (status >= 0) {
    if ( (readmsg != NULL)  && strlen(readmsg) > 0) {
      GlobalREDebugFlag = atoi(readmsg);
    }
    if (readhdr != NULL)
      free(readhdr);
    if (readmsg != NULL)
      free(readmsg);
    if (user != NULL)
      free(user);
    if (addr != NULL)
      free(addr);
    /* initialize reDebug stack space*/
    for (i = 0; i < REDEBUG_STACK_SIZE_FULL; i++) 
      reDebugStackFull[i] = NULL;
    for (i = 0; i < REDEBUG_STACK_SIZE_CURR; i++)
      reDebugStackCurr[i] = NULL;
    reDebugStackFullPtr = 0;
    reDebugStackCurrPtr = 0;
    snprintf(waitHdr,HEADER_TYPE_LEN - 1,   "idbug:");

    rodsLog (LOG_NOTICE,"reDebugInitialization: Got Debug StreamId:%i\n",GlobalREDebugFlag);
    snprintf(waitMsg, MAX_NAME_LEN, "PROCESS BEGIN at %s:%i. Client connected from %s at port %i\n", 
	     myHostName, myPID, svrComm->clientAddr,ntohs(svrComm->localAddr.sin_port));
    _writeXMsg(GlobalREDebugFlag, "idbug", waitMsg);
    snprintf(waitMsg, MAX_NAME_LEN, "%s:%i is waiting\n", myHostName, myPID);  }
  return(0);
}


int processXMsg(int streamId, int *msgNum, int *seqNum, 
		char * readhdr, char *readmsg, 
		char *callLabel, int flag, char *hdr, char *actionStr, char *seActionStr,
		Env *env, int curStat, ruleExecInfo_t *rei)
{

  char cmd;
  char myhdr[HEADER_TYPE_LEN];
  char mymsg[MAX_NAME_LEN];
  char *outStr = NULL;
  char *ptr;
/*  Res *mP; */
  int i,n;
  int iLevel, wCnt;
  int  ruleInx = 0;
  Region *r;
  Res *res;
  rError_t errmsg;
  int found;
/*  char ruleAction[MAX_RULE_LENGTH];
  char ruleRecovery[MAX_RULE_LENGTH];
  char ruleHead[MAX_ACTION_SIZE];
  char ruleBase[MAX_ACTION_SIZE];
  char *actionArray[MAX_ACTION_IN_RULE];
  char *recoveryArray[MAX_ACTION_IN_RULE];*/

  snprintf(myhdr, HEADER_TYPE_LEN - 1,   "idbug:%s",callLabel);
  cmd = readmsg[0];

  switch(cmd) {
  case 'n': /* next */
    return(REDEBUG_NEXT);
    break;
  case 'd': /* discontinue. stop now */
    return(REDEBUG_WAIT);
    break;
  case 'c': /* continue */
    return(REDEBUG_CONTINUE);
    break;
  case 'C': /* continue */
    return(REDEBUG_CONTINUE_VERBOSE);
    break;
  case 'b': /* break point */
    if (breakPointsInx == 99) {
      _writeXMsg(streamId, myhdr, "Maximum Breakpoint Count reached. Breakpoint not set\n");
      return(REDEBUG_WAIT);
    }

    breakPoints[breakPointsInx] = strdup((char *)(readmsg + 1));
    trimWS(breakPoints[breakPointsInx]);
    snprintf(mymsg, MAX_NAME_LEN, "Breakpoint %i  set at %s\n", breakPointsInx,
	   breakPoints[breakPointsInx]);
    _writeXMsg(streamId, myhdr, mymsg);
    breakPointsInx++;
    return(REDEBUG_WAIT);
    break;
  case 'e': /* examine * and $ variables */
  case 'p': /* print an expression */
	    ptr = (char *)(readmsg + 1);
	    trimWS(ptr);
	    snprintf(myhdr, HEADER_TYPE_LEN - 1,   "idbug: Printing %s\n",ptr);
	    r = make_region(0, NULL);
	    errmsg.len = 0;
	    errmsg.errMsg = NULL;
	    res = parseAndComputeExpression(ptr, env, rei, 0, &errmsg, r);
	    outStr = convertResToString(res);
    	snprintf(mymsg, MAX_NAME_LEN, "%s\n", outStr);
    	free(outStr);
	    if(getNodeType(res) == N_ERROR) {
	    	errMsgToString(&errmsg, mymsg + strlen(mymsg), MAX_NAME_LEN - strlen(mymsg));
	    }
    	freeRErrorContent(&errmsg);
	    _writeXMsg(streamId, myhdr, mymsg);

      return(REDEBUG_WAIT);
  case 'l': /* list rule and * and $ variables */
    ptr = (char *)(readmsg + 1);
    trimWS(ptr);
    snprintf(myhdr, HEADER_TYPE_LEN - 1,   "idbug: Listing %s",ptr);
    if (*ptr == '*') {
      mymsg[0] = '\n';
      mymsg[1] = '\0';
	  Env *cenv = env;
	  found = 0;
	  while(cenv!=NULL) {
		  int n = cenv->current->len;
		  int i;
		  for(i = 0;i<n;i++) {
			  Bucket *b = cenv->current->buckets[i];
			  while(b!=NULL) {
				  found = 1;
				  char typeString[128];
				  typeToString(((Res *)b->value)->exprType, NULL, typeString, 128);
				  snprintf(mymsg + strlen(mymsg), MAX_NAME_LEN - strlen(mymsg), "%s of type %s\n", b->key, typeString);
				  b = b->next;
			  }
		  }
		  cenv = cenv->previous;
      }
      if (!found) {
    	  snprintf(mymsg + strlen(mymsg), MAX_NAME_LEN - strlen(mymsg), "<empty>\n");
      }
      _writeXMsg(streamId, myhdr, mymsg);
      return(REDEBUG_WAIT);
    }
    else  if (*ptr == '$') {
      mymsg[0] = '\n';
      mymsg[1] = '\0';
      for (i= 0; i < coreRuleVarDef .MaxNumOfDVars; i++) {
	snprintf(&mymsg[strlen(mymsg)], MAX_NAME_LEN - strlen(mymsg), "$%s\n", coreRuleVarDef.varName[i]);
      }
      _writeXMsg(streamId, myhdr, mymsg);
      return(REDEBUG_WAIT);
    }
    else if (*ptr == 'r') {
      ptr ++;
      trimWS(ptr);
      mymsg[0] = '\0';
      snprintf(myhdr, HEADER_TYPE_LEN - 1,   "idbug: Listing %s",ptr);
      RuleIndexListNode *node;
      found = 0;
      while (findNextRule2 (ptr, ruleInx, &node) != NO_MORE_RULES_ERR) {
    	  found = 1;
    	  if(node->secondaryIndex) {
    		  n = node->condIndex->valIndex->len;
    		  int i;
    		  for(i=0;i<n;i++) {
    			  Bucket *b = node->condIndex->valIndex->buckets[i];
    			  while(b!=NULL) {
    	    		  RuleDesc *rd = getRuleDesc(*(int *)b->value);
    	    		  char buf[MAX_RULE_LEN];
    	    		  ruleToString(buf, MAX_RULE_LEN, rd);
    	    		  snprintf(mymsg + strlen(mymsg), MAX_NAME_LEN - strlen(mymsg), "%i: %s\n%s\n", node->ruleIndex, rd->node->base[0] == 's'? "<source>":rd->node->base + 1, buf);
    	    		  b = b->next;
    			  }
    		  }
    	  } else {
    		  RuleDesc *rd = getRuleDesc(node->ruleIndex);
    		  char buf[MAX_RULE_LEN];
    		  ruleToString(buf, MAX_RULE_LEN, rd);
    		  snprintf(mymsg + strlen(mymsg), MAX_NAME_LEN - strlen(mymsg), "\n  %i: %s\n%s\n", node->ruleIndex, rd->node->base[0] == 's'? "<source>":rd->node->base + 1, buf);
    	  }
    	  ruleInx ++;
      }
      if (!found) {
    	  snprintf(mymsg, MAX_NAME_LEN,"Rule %s not found\n", ptr);
      }
      _writeXMsg(streamId, myhdr, mymsg);
      return(REDEBUG_WAIT);
    }
    else {
      snprintf(mymsg, MAX_NAME_LEN, "Unknown Parameter Type");
      _writeXMsg(streamId, myhdr, mymsg);
      return(REDEBUG_WAIT);
    }
    break;
  case 'w': /* where are you now in CURRENT stack*/
    wCnt = 20;
    iLevel = 0;
    ptr = (char *)(readmsg + 1);
    trimWS(ptr);
    if (strlen(ptr) > 0)
      wCnt = atoi(ptr);

    i = reDebugStackCurrPtr - 1;
    while (wCnt > 0 && reDebugStackCurr[i] != NULL) {
      snprintf(myhdr, HEADER_TYPE_LEN - 1,   "idbug: Level %3i",iLevel);
      _writeXMsg(streamId,  myhdr, reDebugStackCurr[i] );
      if (strstr(reDebugStackCurr[i] , "ApplyRule") != NULL)
	iLevel++;
      wCnt--;
      i--;
      if (i < 0) 
	i = REDEBUG_STACK_SIZE_CURR - 1;
    }
    return(REDEBUG_WAIT);
    break;
  case 'W': /* where are you now in FULL stack*/
    wCnt = 20;
    iLevel = 0;
    ptr = (char *)(readmsg + 1);
    trimWS(ptr);
    if (strlen(ptr) > 0)
      wCnt = atoi(ptr);

    i = reDebugStackFullPtr - 1;
    while (wCnt > 0 && reDebugStackFull[i] != NULL) {
      snprintf(myhdr, HEADER_TYPE_LEN - 1,   "idbug: Step %4i",iLevel);
      _writeXMsg(streamId,  myhdr, reDebugStackFull[i] );
      iLevel++;
      wCnt--;
      i--;
      if (i < 0) 
	i = REDEBUG_STACK_SIZE_CURR - 1;
    }
    return(REDEBUG_WAIT);
    break;
  default:
    snprintf(mymsg, MAX_NAME_LEN, "Unknown Action: %s.", readmsg );
    _writeXMsg(streamId, myhdr, mymsg);
    return(REDEBUG_WAIT);
    break;

  }
  return(curStat);
}

int
processBreakPoint(int streamId, int *msgNum, int *seqNum,
		  char *callLabel, int flag, char *hdr, char *actionStr, char *seActionStr,
		  Env *env, int curStat, ruleExecInfo_t *rei)
{

  int i;
  char myhdr[HEADER_TYPE_LEN];
  char mymsg[MAX_NAME_LEN];

  snprintf(myhdr, HEADER_TYPE_LEN - 1,   "idbug:%s",callLabel);


  if (breakPointsInx > 0) {
    for (i = 0; i < breakPointsInx; i++) {
      if (strstr(actionStr, breakPoints[i]) ==  actionStr) {
	snprintf(mymsg,MAX_NAME_LEN, "Breaking at BreakPoint %i:%s\n", i , breakPoints[i]);
	_writeXMsg(streamId, myhdr, mymsg);
	snprintf(mymsg,MAX_NAME_LEN, "with values: %s\n", seActionStr);
	_writeXMsg(streamId, myhdr, mymsg);
	curStat = REDEBUG_WAIT;
	return(curStat);
      }
    }
  }
  return(curStat);
}


int
storeInStack(char *hdr, char* step)
{
  char *stackStr;
  char *stackStr2;
  char *s;
  int i;


  stackStr = (char *) malloc(strlen(hdr) + strlen(step) + 5);
  sprintf(stackStr,"%s: %s\n", hdr, step);


  i = reDebugStackFullPtr;
  if (reDebugStackFull[i] != NULL) {
    free(reDebugStackFull[i]);
  }
  reDebugStackFull[i] = stackStr;
  reDebugStackFullPtr = (i + 1 ) % REDEBUG_STACK_SIZE_FULL;
  i = reDebugStackFullPtr;
  if (reDebugStackFull[i] != NULL) {
    free(reDebugStackFull[i]);
    reDebugStackFull[i] = NULL;
  }

  if (strstr(hdr,"Done") == hdr) { /* Pop the stack */
    s = (char *) hdr + 4;
    i = reDebugStackCurrPtr - 1;
    if (i < 0)
      i = REDEBUG_STACK_SIZE_CURR - 1;
    while (reDebugStackCurr[i] != NULL && strstr(reDebugStackCurr[i] , s) != reDebugStackCurr[i]) {
      free(reDebugStackCurr[i]);
      reDebugStackCurr[i] = NULL;
      i = i - 1;
      if (i < 0)
	i = REDEBUG_STACK_SIZE_CURR - 1;
    }
    if (reDebugStackCurr[i] != NULL) {
      free(reDebugStackCurr[i]);
      reDebugStackCurr[i] = NULL;
    }
    reDebugStackCurrPtr = i;
    return(0);
  }

  stackStr2 = strdup(stackStr);
  i = reDebugStackCurrPtr;
  if (reDebugStackCurr[i] != NULL) {
    free(reDebugStackCurr[i]);
  }
  reDebugStackCurr[i] = stackStr2;
  reDebugStackCurrPtr = (i + 1 ) % REDEBUG_STACK_SIZE_CURR;
  i = reDebugStackCurrPtr;
  if (reDebugStackCurr[i] != NULL) {
    free(reDebugStackCurr[i]);
    reDebugStackCurr[i] = NULL;
  }
  return(0);
}


int sendWaitXMsg (int streamId) {
  _writeXMsg(streamId, waitHdr, waitMsg);
  return(0);
}
int cleanUpDebug(int streamId) {
  int i;
  for (i = 0 ; i < REDEBUG_STACK_SIZE_CURR; i++) {
    if (reDebugStackCurr[i] != NULL) {
      free(reDebugStackCurr[i]);
      reDebugStackCurr[i] = NULL;
    }
  }
  for (i = 0 ; i < REDEBUG_STACK_SIZE_FULL; i++) {
    if (reDebugStackFull[i] != NULL) {
      free(reDebugStackFull[i]);
      reDebugStackFull[i] = NULL;
    }
  }
  reDebugStackCurrPtr = 0;
  reDebugStackFullPtr = 0;
  GlobalREDebugFlag = -1;
  return(0);
}

int
reDebug(char *callLabel, int flag, char *actionStr, Env *env, ruleExecInfo_t *rei)
{
  int i, m, s, status, sleepT, j;
  int processedBreakPoint = 0;
  char hdr[HEADER_TYPE_LEN];
  char *readhdr = NULL;
  char *readmsg = NULL;
  char *user = NULL;
  char *addr = NULL;
  static int mNum = 0;
  static int sNum = 0;
  static int curStat = 0; 
  char condRead[MAX_NAME_LEN];
  char myActionStr[10][MAX_NAME_LEN + 10];
  int aNum = 0;
  char seActionStr[10 * MAX_NAME_LEN + 100];
  rsComm_t *svrComm;
  int waitCnt = 0;
  sleepT = 1;
  svrComm = rei->rsComm;

  snprintf(hdr, HEADER_TYPE_LEN - 1,   "iaudit:%s",callLabel);
  condRead[0] = '\0'; 
  rodsLog (LOG_NOTICE,"PPP:%s\n",hdr);
  strcpy(seActionStr, actionStr);
  if (GlobalREAuditFlag > 0) {
    if (flag == -4) {
      if (rei->uoic != NULL && rei->uoic->userName != NULL && rei->uoic->rodsZone != NULL) {
	snprintf(myActionStr[aNum],MAX_NAME_LEN + 10 , "  USER:%s@%s", rei->uoic->userName, rei->uoic->rodsZone);
	aNum++;
      }
      if (rei->doi != NULL && rei->doi->objPath != NULL && strlen(rei->doi->objPath) > 0 ) {
	snprintf(myActionStr[aNum],MAX_NAME_LEN + 10 , "  DATA:%s", rei->doi->objPath);
	aNum++;
      }
      if (rei->doi != NULL && rei->doi->filePath != NULL && strlen(rei->doi->filePath) > 0) {
	snprintf(myActionStr[aNum],MAX_NAME_LEN + 10 , "  FILE:%s", rei->doi->filePath);
	aNum++;
      }
      if (rei->doinp != NULL && rei->doinp->objPath != NULL && strlen(rei->doinp->objPath) > 0) {
	snprintf(myActionStr[aNum],MAX_NAME_LEN + 10 , "  DATAIN:%s",rei->doinp->objPath);
	aNum++;
      }
      if (rei->doi != NULL && rei->doi->rescName != NULL && strlen(rei->doi->rescName) > 0) {
	snprintf(myActionStr[aNum],MAX_NAME_LEN + 10 , "  RESC:%s",rei->doi->rescName);
	aNum++;
      }
      if (rei->rgi != NULL && rei->rgi->rescInfo->rescName != NULL && strlen(rei->rgi->rescInfo->rescName) > 0) {
	snprintf(myActionStr[aNum],MAX_NAME_LEN + 10 , "  RESC:%s",rei->rgi->rescInfo->rescName);
	aNum++;
      }
      if (rei->doi != NULL && rei->doi->rescGroupName != NULL && strlen(rei->doi->rescGroupName) > 0) {
	snprintf(myActionStr[aNum],MAX_NAME_LEN + 10 , "  RESCGRP:%s",rei->doi->rescGroupName);
	aNum++;
      }
      if (rei->rgi != NULL && rei->rgi->rescGroupName != NULL && strlen(rei->rgi->rescGroupName) > 0) {
	snprintf(myActionStr[aNum],MAX_NAME_LEN + 10 , "  RESCGRP:%s",rei->rgi->rescGroupName);
	aNum++;
      }
      if (rei->coi != NULL && rei->coi->collName != NULL) {
	snprintf(myActionStr[aNum],MAX_NAME_LEN + 10 , "  COLL:%s", rei->coi->collName);
	aNum++;
      }
      for (j = 0; j < aNum; j++) {
	strncat(seActionStr, myActionStr[j], 10 * MAX_NAME_LEN + 100);
      }
    }
  }

  /* Write audit trail */
  if (GlobalREAuditFlag == 3) {
    i = _writeXMsg(GlobalREAuditFlag, hdr, actionStr);
  }
  

  /* Send current position for debugging */
  if ( GlobalREDebugFlag > 5 ) {
    if (curStat != REDEBUG_CONTINUE) {
      snprintf(hdr, HEADER_TYPE_LEN - 1,   "idbug:%s",callLabel);
      i = _writeXMsg(GlobalREDebugFlag, hdr, actionStr);
    }
  }

  /* store in stack */
  storeInStack(callLabel, actionStr);

  while ( GlobalREDebugFlag > 5 ) {
    s = sNum;
    m = mNum;
    /* what should be the condition */
    sprintf(condRead, "(*XSEQNUM >= %d) && (*XADDR != \"%s:%i\") && (*XUSER  == \"%s@%s\") && ((*XHDR == \"CMSG:ALL\") %%%% (*XHDR == \"CMSG:%s:%i\"))",
	    s, myHostName, myPID, svrComm->clientUser.userName, svrComm->clientUser.rodsZone, myHostName, myPID);

    /*
    sprintf(condRead, "(*XSEQNUM >= %d)  && ((*XHDR == CMSG:ALL) %%%% (*XHDR == CMSG:%s:%i))",
	    s,  myHostName, getpid());
    */
    status = _readXMsg(GlobalREDebugFlag, condRead, &m, &s, &readhdr, &readmsg, &user, &addr);
    if (status == SYS_UNMATCHED_XMSG_TICKET) {
      cleanUpDebug(GlobalREDebugFlag);
      return(0);
    }
    if (status  >= 0) {
      rodsLog (LOG_NOTICE,"Getting XMsg:%i:%s:%s\n", s,readhdr, readmsg);
      curStat = processXMsg(GlobalREDebugFlag, &m, &s, readhdr, readmsg,
			    callLabel, flag, hdr,  actionStr, seActionStr, 
			    env, curStat, rei);
      if (readhdr != NULL)
	free(readhdr);
      if (readmsg != NULL)
	free(readmsg);
      if (user != NULL)
	free(user);
      if (addr != NULL)
	free(addr);

      mNum = m;
      sNum = s + 1;
      if (curStat == REDEBUG_WAIT) {
	sendWaitXMsg(GlobalREDebugFlag);
      }
      if (curStat == REDEBUG_CONTINUE || curStat == REDEBUG_CONTINUE_VERBOSE) {
	if (processedBreakPoint == 1)
	  break;
	curStat = processBreakPoint(GlobalREDebugFlag, &m, &s, 
				   callLabel, flag, hdr,  actionStr, seActionStr,
				   env, curStat, rei);
	processedBreakPoint = 1;
	if (curStat == REDEBUG_WAIT) {
	  sendWaitXMsg(GlobalREDebugFlag);
	  continue;
	}
	else
	  break;
      }
      else if (curStat == REDEBUG_NEXT || curStat == REDEBUG_STEP )
	break;
    }
    else {
      if (curStat == REDEBUG_CONTINUE || curStat == REDEBUG_CONTINUE_VERBOSE) {
	
	rodsLog(LOG_NOTICE, "Calling 2: %s,%p",actionStr, &actionStr);
	curStat = processBreakPoint(GlobalREDebugFlag, &m, &s, 
				    callLabel, flag, hdr,  actionStr, seActionStr,
				    env, curStat, rei);
	processedBreakPoint = 1;
	if (curStat == REDEBUG_WAIT) {
	  sendWaitXMsg(GlobalREDebugFlag);
	  continue;
	}
	else
	  break;
      }
      sleep(sleepT);
      sleepT = 2 * sleepT;
      /* if (sleepT > 10) sleepT = 10;*/
      if (sleepT > 1){
	sleepT = 1;
	waitCnt++;
	if (waitCnt > 60) {
	  sendWaitXMsg(GlobalREDebugFlag);
	  waitCnt = 0;
	}
      }
    }
  }


  return(0);
}
