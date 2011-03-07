/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "reHelpers1.h"




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
