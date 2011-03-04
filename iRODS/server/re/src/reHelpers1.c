/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "reHelpers1.h"


int
checkRuleCondition(char *action, char *incond, char *args[MAX_NUM_OF_ARGS_IN_ACTION], 
		   int argc, ruleExecInfo_t *rei, int reiSaveFlag )
{
  int i,k;
  /*  char *t1, *t2, *t3;*/
  char cond[MAX_COND_LEN];
  char evaStr[MAX_COND_LEN * 2];
  /*
  dataObjInfo_t *doi;
  rescInfo_t *roi;
  userInfo_t *uoi;
  collInfo_t *coi;

  doi = rei->doi;
  roi = rei->roi;
  uoi = rei->uoi;
  coi = rei->coi;
  i = 0;
  */

  
  rstrcpy (cond,incond,MAX_COND_LEN);
  i  = replaceVariables(action, cond,args,argc,rei);
  if (i < 0)
    return(0); /* FALSE */

  /****
  t1 = cond;
  /  * replace variables with values * /
  while ((t2 = strstr(t1,"$")) != NULL) {
    j =  replaceDollarParam(t2,(int) (MAX_COND_LEN - (t2 - cond) - 1),
			    args, argc, rei);
    if (j < 0) {
      rodsLog (LOG_NOTICE,"replaceDollarParam Failed at %s: %i\n", (char *)t2,j);
      return(0);
    }
    t1 = t2 + 1;
  }
  ****/
  /***
  while(strcmp(dataObjCond[i],"ENDOFLIST")) {
    if ((t2  = strstr(t1,dataObjCond[i])) == NULL) {
      t1 = cond;
      i++;
    }
    else if (*(t2 - 1) != '$') {
      t1 = t2 + 1;
    }
    else {
      t2--;
      j = replaceDataVar(t2,(int) (MAX_COND_LEN - (t2 - cond) - 1), i, doi);
      if (j < 0) {
	rodsLog (LOG_NOTICE,"replaceVar Failed at %s: %i\n", (char *)(t2-1),j);
	return(0);
      }
      t1 = t2;
    }
  }
  ***/
  /* check condition and return */
  k =  computeExpression(cond, rei, reiSaveFlag,evaStr);
  if (k < 0) {
    rodsLog (LOG_NOTICE,"computeExpression Failed: %s: %i\n", cond,k);
  }
  return(k);
}




int
checkRuleConditionNew(char *action, char *incond,  msParamArray_t *inMsParamArray,
		   ruleExecInfo_t *rei, int reiSaveFlag )
{
  int i,k;
  /* char *t1, *t2, *t3;*/
  char cond[MAX_COND_LEN];
  char evaStr[MAX_COND_LEN * 2];
  /*
  dataObjInfo_t *doi;
  rescInfo_t *roi;
  userInfo_t *uoi;
  collInfo_t *coi;

  doi = rei->doi;
  roi = rei->roi;
  uoi = rei->uoi;
  coi = rei->coi;
  i = 0;
  */

  
  rstrcpy (cond,incond,MAX_COND_LEN);
  i  = replaceVariablesAndMsParams(action, cond, inMsParamArray, rei);
  if (i < 0)
    return(0); /* FALSE */

  
  /* check condition and return */
  k =  computeExpression(cond, rei, reiSaveFlag, evaStr);
  if (k < 0) {
    rodsLog (LOG_NOTICE,"computeExpression Failed: %s: %i\n", cond, k);
  }
  return(k);
}


int
convertArgWithVariableBinding(char *inS, char **outS, msParamArray_t *inMsParamArray, ruleExecInfo_t *rei)
{
  char *tS;
  int i;
 


  tS =  malloc(strlen(inS)  + 4 * MAX_COND_LEN);
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

int
evaluateExpression(char *expr, char *res, ruleExecInfo_t *rei)
{
  int i;

  i  = replaceVariablesAndMsParams("", expr, rei->msParamArray, rei);
  if (i < 0) return(i);
  i =  computeExpression(expr, rei, 0, res);
  /***
  if (i == 1)
    strcpy(res,expr);
  ***/
  return(i);
}

int
computeExpression( char *expr, ruleExecInfo_t *rei, int reiSaveFlag , char *res)
{

   int i;
   char expr1[MAX_COND_LEN], expr2[MAX_COND_LEN], expr3[MAX_COND_LEN];
   char oper1[MAX_OPER_LEN];
   static int k;
   if (strlen(expr) == 0) {
     strcpy(res,expr);
     return (TRUE);
   }
   strcpy(res,"");
   if ((i = splitExpression(expr,expr1,expr2,oper1)) < 0)
     return(i);
   if (strlen(oper1) == 0) {
     if (strlen(expr2) == 0) {
       /*
       if (expr1[0] == '(' && expr1[strlen(expr1)-1] == ')') {
	 expr1[0] = ' ';
	 expr1[strlen(expr1)-1] ='\0';
	 if (goodExpr(expr1) != 0) {
           strcpy(res,expr);
	   return(TRUE);	   
	 }
	 trimWS(expr1);
         strcpy(expr3,expr1);
	 if ((i = splitExpression(expr3,expr1,expr2,oper1)) < 0)
	   return(i);
	 if (strlen(oper1) == 0) {
	   strcpy(res,expr);
           return(TRUE);
         }
       }
       else {
	 strcpy(res,expr);
	 return(TRUE);
       }
       */
       trimWS(expr1);
       strcpy(expr3,expr1);
       if ((i = splitExpression(expr3,expr1,expr2,oper1)) < 0)
	 return(i);
       if (strlen(oper1) == 0) {
	 strcpy(res,expr);
	 return(TRUE);
       }

     }
     else {
       strcpy(res,expr);
       return(TRUE);
     }
   }
   k++;

   i  = _computeExpression(expr1,expr2,oper1,rei,reiSaveFlag, res );

   k--;
   if (i < 0)
     strcpy(res,expr);
   return(i);
}

int
_computeExpression(char *expr1, char *expr2, char *oper1, ruleExecInfo_t *rei, int reiSaveFlag , char *res )
{
   int i,j,k,iii,jjj,kkk;
   double x,y;
   /*   char aaa[MAX_COND_LEN], bbb[MAX_COND_LEN];*/
   char inres1[MAX_COND_LEN * 2];
   char inres2[MAX_COND_LEN * 2];

   if (isLogical(oper1)) {
     if ((i = computeExpression(expr1,rei, reiSaveFlag,inres1)) < 0) 
       return(i);
     if ((j = computeExpression(expr2,rei, reiSaveFlag,inres2)) < 0)
       return(j);
     if (!strcmp(oper1, "&&")) {
       /*
       sprintf(res,"%i", atoi(inres1) && atoi(inres2));
       return( atoi(res));
       */
       sprintf(res,"%i", i && j);
       return( atoi(res));
     }
     else if (!strcmp(oper1, "%%")) {
       /*
       sprintf(res,"%i", atoi(inres1) || atoi(inres2));
       return( atoi(res));
       */
       sprintf(res,"%i", i || j);
       return( atoi(res));
     }
     else
       return(-1);
   }
   else if (isNumber(expr1) && isNumber(expr2)) {
     x = atof(expr1);
     y = atof(expr2);
     if (!strcmp(oper1, "<"))
       sprintf(res,"%d", x < y);
     else if (!strcmp(oper1, ">"))
       sprintf(res,"%d", x > y);
     else if (!strcmp(oper1, "=="))
       sprintf(res,"%d", x == y);
     else if (!strcmp(oper1, "<="))
      sprintf(res,"%d", x <= y);
     else if (!strcmp(oper1, ">="))
       sprintf(res,"%d", x >= y);
     else if (!strcmp(oper1, "!="))
       sprintf(res,"%d", x != y);
     else if (!strcmp(oper1, "++"))
       sprintf(res,"%d",  (int) (x + y));
     else if ( (strchr(expr1,'.') != NULL) || 
	       (strchr(expr2,'.') != NULL) ) {
       if (!strcmp(oper1, "+"))
	 sprintf(res,"%f",  x + y);
       else if (!strcmp(oper1, "-"))
	 sprintf(res,"%f",  x - y);
       else if (!strcmp(oper1, "*"))
	 sprintf(res,"%f",  x * y);
       else if (!strcmp(oper1, "/"))
	 sprintf(res,"%f",  x / y);
       else
	 return(-2);
     }
     else { /* both are integers */
       if (!strcmp(oper1, "+"))
         sprintf(res,"%d", (int) (x + y));
       else if (!strcmp(oper1, "-"))
         sprintf(res,"%d", (int) (x - y));
       else if (!strcmp(oper1, "*"))
         sprintf(res,"%d", (int) (x * y));
       else if (!strcmp(oper1, "/"))
         sprintf(res,"%d", (int) (x / y));
       else
         return(-2);
     }
     return(atoi(res));
   }
   /********RAJA remove May 29 2009 being done later anyway
   else if (isNumber(expr1)) {
     iii = computeExpression(expr2, rei, reiSaveFlag,inres1);
     if (iii < 0)
       return(iii);
     i = _computeExpression(expr1,inres1,oper1, rei, reiSaveFlag,res);
     return(i);
   }
   else if (isNumber(expr2)) {
     iii = computeExpression(expr1, rei, reiSaveFlag,inres1);
     if (iii < 0)
       return(iii);
     i = _computeExpression(inres1,expr2, oper1, rei, reiSaveFlag, res);
     return(i);
   }
   ************/
   else if (isAFunction(expr1) || isAFunction(expr2)) {
     if (isAFunction(expr1)) {
       if (executeRuleAction(expr1, rei, reiSaveFlag) == 0)
	 iii = 1;
       else
	 iii = 0;
     }
     else {
       if (strlen(expr1) == 0)
	 iii = -1;
       else 
	 iii = atoi(expr1);
     }
     if (isAFunction(expr2)) {
       if (executeRuleAction(expr2, rei, reiSaveFlag) == 0)
	 jjj = 1;
       else
	 jjj = 0;
     }
     else {
       if (strlen(expr2) == 0)
	 jjj = -1;
       else 
	 jjj = atoi(expr2);
     }
     if (iii == -1) {
       sprintf(res,"%d",jjj);
       return(atoi(res));
     }
     if (jjj == -1){
       sprintf(res,"%d",iii);
       return(atoi(res));
     }
     if (!strcmp(oper1, "<"))
       sprintf(res,"%i",  iii < jjj);
     else if (!strcmp(oper1, ">"))
       sprintf(res,"%i",  iii > jjj);
     else if (!strcmp(oper1, "=="))
       sprintf(res,"%i",  iii == jjj);
     else if (!strcmp(oper1, "<="))
       sprintf(res,"%i",  iii <= jjj);
     else if (!strcmp(oper1, ">="))
       sprintf(res,"%i",  iii >= jjj);
     else if (!strcmp(oper1, "!="))
       sprintf(res,"%i",  iii != jjj);
     else
       return(-3);
     return(atoi(res));
   }
   else { /* expr1 and expr2 can be string/numeric expressions */
     iii = computeExpression(expr1, rei, reiSaveFlag,inres1);
     if (iii <  0)
       return(iii);
     jjj = computeExpression(expr2, rei, reiSaveFlag,inres2);
     if (jjj < 0)
       return(jjj);
     if (strcmp(expr1,inres1) || strcmp(expr2,inres2)) {
       kkk = _computeExpression(inres1,inres2,oper1, rei, reiSaveFlag,res);
       return(kkk);
     }
     /*
     if (strcmp(expr1,inres1) && strcmp(expr2,inres2)) {
       kkk = _computeExpression(inres1,inres2,oper1, rei, reiSaveFlag,res);
       return(kkk);
     }
     else if (strcmp(expr1,inres1)) {
       kkk = _computeExpression(inres1,expr2,oper1, rei, reiSaveFlag,res);
       return(kkk);
     }
     else if (strcmp(expr2,inres2)) {
       sprintf(bbb,"%d",jjj);
       kkk = _computeExpression(expr1,inres2,oper1, rei, reiSaveFlag,res);
       return(kkk);
     }
     */
     if (!strcmp(oper1, "<")) {
       if (strcmp(expr1,expr2) < 0)
	 iii = 1;
       else
	 iii = 0;
     }
     else if (!strcmp(oper1, ">")){
       if (strcmp(expr1,expr2) > 0)
	 iii = 1;
       else
	 iii = 0;
     }
     else if (!strcmp(oper1, "==")){
       if (strcmp(expr1,expr2) == 0)
	 iii = 1;
       else
	 iii = 0;
     }
     else if (!strcmp(oper1, "<=")){
       if (strcmp(expr1,expr2) <= 0)
	 iii = 1;
       else
	 iii = 0;
     }
     else if (!strcmp(oper1, ">=")){
       if (strcmp(expr1,expr2) >= 0)
	 iii = 1;
       else
	 iii = 0;
     }
     else if (!strcmp(oper1, "!=")){
       if (strcmp(expr1,expr2) != 0)
	 iii = 1;
       else
	 iii = 0;
     }
     else if (!strcmp(oper1, "not like")) {
       k = reREMatch(expr2,expr1);
       if (k == 0) 
	 iii = 1;
       else
	 iii = 0;
     }
     else if (!strcmp(oper1, "like")) {
       iii = reREMatch(expr2,expr1);
     }
     else
       return(-4);
     sprintf(res,"%i",  iii);
     return(iii);
   }
   /*   return(-5); removed RAJA June 13 2008 as it is not reached */
}

int
replaceVariables(char *action, char *inStr, char *args[MAX_NUM_OF_ARGS_IN_ACTION], int argc,
		   ruleExecInfo_t *rei )
{
  int j;
  char *t1, *t2;

  t1 = inStr;

  while ((t2 = strstr(t1,"$")) != NULL) {
    j =  replaceDollarParam(action, t2,(int) (MAX_COND_LEN - (t2 - inStr) - 1),
			    args, argc, rei);
    if (j < 0) {
      rodsLog (LOG_NOTICE,"replaceDollarParam Failed at %s: %i\n", (char *)t2,j);
      return(j);
    }
    t1 = t2 + 1;
  }
  return(0);
}

int
replaceVariablesNew(char *action, char *inStr, msParamArray_t *inMsParamArray, ruleExecInfo_t *rei )
{
  int j;
  char *t1, *t2;

  if (strncmp(inStr,"assign(",7) == 0) 
    return(0);

  j = 0;
  t1 = inStr;


  
  while ((t2 = strstr(t1,"$")) != NULL) {
    j = replaceSessionVar(action,t2,(int) (MAX_COND_LEN - (t2 - inStr) - 1),rei);
    if (j < 0) {
      rodsLog (LOG_NOTICE,"replaceSessionVar Failed at %s: %i\n", (char *)t2,j);
      return(j);
    }
    t1 = t2 + 1;
  }
  return(j);
}

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
replaceDollarParam(char *action, char *dPtr, int len, 
		   char *args[MAX_NUM_OF_ARGS_IN_ACTION], int argc,
		   ruleExecInfo_t *rei)
{
  /*char *t1,*t2,*t3;*/
  int i,j;

  i = replaceArgVar(dPtr,len, args, argc);
  if ( i != UNKNOWN_PARAM_IN_RULE_ERR)
    return(i);
  j = replaceSessionVar(action, dPtr,len,rei);
  return(j);


  /*
  if      ((j = replaceArgVar(dPtr,len, args, argc)) == 0)
    return(0);
  else if ((j = replaceDataVar(dPtr,len,rei->doi)) == 0)
    return(0);
  else if ((j = replaceCollVar(dPtr,len,rei->coi)) == 0)
    return(0);
  else if ((j = replaceUserVar(dPtr,len,rei->uoic, rei->uoip)) == 0)
    return(0);
  else if ((j = replaceRescVar(dPtr,len,rei->roi)) == 0)
    return(0);
  else
    return(UNKNOWN_PARAM_IN_RULE_ERR);
  */
  /*
  t1 = dPtr + 1;

  i = 0;
  while(strcmp(dataObjCond[i],"ENDOFLIST")) {
    if (strstr(t1,dataObjCond[i]) == t1) {
      j =  replaceDataVar(dPtr, len,i,rei->doi);
      return (j);
    }
    else
      i++;
  }

  i = 0;
  while(strcmp(rescCond[i],"ENDOFLIST")) {
    if (strstr(t1,rescCond[i]) == t1) {
      j =  replaceRescVar(dPtr, len,i,rei->roi);
      return(j);
    }
    else
      i++;
  }
  i = 0;
  while(strcmp(userCond[i],"ENDOFLIST")) {
    if (strstr(t1,userCond[i]) == t1) {
      j =  replaceUserVar(dPtr, len,i,rei->uoi);
      return(j);
    }
    else
      i++;
  }
  i = 0;
  while(strcmp(collCond[i],"ENDOFLIST")) {
    if (strstr(t1,collCond[i]) == t1) {
      j =  replaceCollVar(dPtr, len,i,rei->coi);
      return(j);
    }
    else
      i++;
  }

  */
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

int processXMsg(int streamId, int *msgNum, int *seqNum, 
		char * readhdr, char *readmsg, 
		char *callLabel, int flag, char *hdr, char *actionStr, 
		msParamArray_t *inMsParamArray, ruleExecInfo_t *rei)
{

  char cmd;

  cmd = readmsg[0];

  switch(cmd) {
  case 'n': /* next */
  case 's': /* step */
    break;
  case 'c': /* continue */
    break;
  case 'e': /* examine */
    break;
  case 'l': /* list rule */
    break;
  case 'b': /* break point */
    break;
  case 'w': /* where are you now */
    _writeXMsg(streamId, hdr, actionStr);
    break;
  default:
    break;

  }
  return(0);
}



int
reDebug(char *callLabel, int flag, char *actionStr, msParamArray_t *inMsParamArray, ruleExecInfo_t *rei)
{
  int i, m, s, status, sleepT, j;
  char hdr[HEADER_TYPE_LEN];
  char myHostName[MAX_NAME_LEN];
  char *readhdr = NULL;
  char *readmsg = NULL;
  char *user = NULL;
  char *addr = NULL;
  static int mNum = 0;
  static int sNum = 0;
  char condRead[NAME_LEN];
  char myActionStr[10][MAX_NAME_LEN + 10];
  int aNum = 0;
  char seActionStr[10 * MAX_NAME_LEN + 100];
  myHostName[0] = '\0';

  gethostname (myHostName, MAX_NAME_LEN);
  sleepT = 1;
  condRead[0] = '\0'; 
  snprintf(hdr, HEADER_TYPE_LEN - 1,   "idbug:%s",callLabel);

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
    strcpy(seActionStr, actionStr);
    for (j = 0; j < aNum; j++) {
      strncat(seActionStr, myActionStr[j], 10 * MAX_NAME_LEN + 100);
    }
    i = _writeXMsg(GlobalREDebugFlag, hdr, seActionStr);
  }
  else {
    i = _writeXMsg(GlobalREDebugFlag, hdr, actionStr);
  }
  

  while ( GlobalREDebugFlag > 3 ) {
    s = sNum;
    m = mNum;
    status = _readXMsg(GlobalREDebugFlag, condRead, &m, &s, &readhdr, &readmsg, &user, &addr);
    if (status  >= 0) {
      
      processXMsg(GlobalREDebugFlag, &m, &s, readhdr, readmsg,
		  callLabel, flag, hdr,  actionStr, inMsParamArray, rei);
      mNum = m;
      sNum = s + 1;
      break;
    }
    else {
      sleep(sleepT);
      sleepT = 2 * sleepT;
      if (sleepT > 10) sleepT = 10;
    }
  }


  return(0);
}
