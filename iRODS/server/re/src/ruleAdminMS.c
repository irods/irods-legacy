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
msiAdmChangeCoreIRB(msParam_t *newFileNameParam, ruleExecInfo_t *rei)
{
  /*  newFileNameParam contains the file name of the new core.
      The file  should be in reConfigs  directory */

  char sysString[1000];
  int i;

  RE_TEST_MACRO ("Loopback on admChangeCoreIRB");

  if (strcmp (newFileNameParam->type,STR_MS_T) != 0)
    return(USER_PARAM_TYPE_ERR);

  if ((i = isUserPrivileged(rei->rsComm)) != 0)
    return (i);

  snprintf(sysString, 999, "cp %s/reConfigs/%s %s/reConfigs/core.irb",
	        (char *) getConfigDir (), (char *) newFileNameParam->inOutStruct, 
	   (char *) getConfigDir ());
  system(sysString);

   
  return(0);
}

int msiAdmAppendToTopOfCoreIRB(msParam_t *newFileNameParam, ruleExecInfo_t *rei)
{
  /*  newFileNameParam contains the file name to be added to top of  core.irb
      The file  should be in reConfigs  directory */

  char sysString[1000];
  int i;
  char *conDir;

  RE_TEST_MACRO ("Loopback on admAppendToTopOfCoreIRB");

  if (strcmp (newFileNameParam->type,STR_MS_T) != 0)
    return(USER_PARAM_TYPE_ERR);

  if ((i = isUserPrivileged(rei->rsComm)) != 0)
    return (i);
  conDir = getConfigDir ();
  snprintf(sysString, 999, "cat %s/reConfigs/%s %s/reConfigs/core.irb > %s/reConfigs/admtmpcore.irb",
	   conDir, (char *) newFileNameParam->inOutStruct, 
	   conDir, conDir);
  system(sysString);
  snprintf(sysString, 999, "mv %s/reConfigs/admtmpcore.irb %s/reConfigs/core.irb",
	        conDir,conDir); 
  system(sysString);
  return(0);
 

}

int msiAdmShowDVM(msParam_t *bufParam, ruleExecInfo_t *rei)
{
  int i;

    _writeString("stdout","----------------------------- DVM -----------------------------\n",rei);

    i = _admShowDVM(bufParam, rei, &appRuleVarDef,0);
    if (i != 0)
      return(i);
    i = _admShowDVM(bufParam, rei, &coreRuleVarDef,1000);
    _writeString("stdout","----------------------------- DVM -----------------------------\n",rei);
    return(i);
}

int _admShowDVM(msParam_t *bufParam, ruleExecInfo_t *rei, rulevardef_t *inRuleVarDef, int inx)
{
  int j;
  char outStr[MAX_RULE_LENGTH];

  _writeString("stdout","---------------------------------------------------------------\n",rei);
  for( j = 0 ; j  < inRuleVarDef->MaxNumOfDVars ; j++) {
    sprintf(outStr," %-5i %-15.15s %s ===> %s\n",j+inx,inRuleVarDef->action[j], 
	    inRuleVarDef->varName[j], inRuleVarDef->var2CMap[j]);
    _writeString("stdout",outStr,rei);
  }
  _writeString("stdout","---------------------------------------------------------------\n",rei);
  return(0);
}

int msiAdmShowFNM(msParam_t *bufParam, ruleExecInfo_t *rei)
{
  int i;

  _writeString("stdout","----------------------------- FNM -----------------------------\n",rei);
    i = _admShowFNM(bufParam, rei, &appRuleFuncMapDef,0);
    if (i != 0)
      return(i);
    i = _admShowFNM(bufParam, rei, &coreRuleFuncMapDef,1000);
  _writeString("stdout","----------------------------- FNM -----------------------------\n",rei);
    return(i);
}

int _admShowFNM(msParam_t *bufParam, ruleExecInfo_t *rei, rulefmapdef_t *inRuleFuncMapDef, int inx)
{

  int j;
  char outStr[MAX_RULE_LENGTH];

  _writeString("stdout","---------------------------------------------------------------\n",rei);
  for( j = 0 ; j  < inRuleFuncMapDef->MaxNumOfFMaps ; j++) {
    sprintf(outStr," %-5i %s ===> %s\n",j+inx, inRuleFuncMapDef->funcName[j], inRuleFuncMapDef->func2CMap[j]);
    _writeString("stdout",outStr,rei);
  }
  _writeString("stdout","---------------------------------------------------------------\n",rei);
  return(0);

}
int msiAdmShowIRB(msParam_t *bufParam, ruleExecInfo_t *rei)
{
  int i;


    i = _admShowIRB(bufParam, rei, &appRuleStrct, 0);
    if (i != 0)
      return(i);
    i = _admShowIRB(bufParam, rei, &coreRuleStrct, 1000);
    return(i);
}

int _admShowIRB(msParam_t *bufParam, ruleExecInfo_t *rei, ruleStruct_t *inRuleStrct, int inx)
{
    int n, i,j;
    char outStr[MAX_RULE_LENGTH];
    char ruleCondition[MAX_RULE_LENGTH];
    char ruleAction[MAX_RULE_LENGTH];
    char ruleRecovery[MAX_RULE_LENGTH];
    char ruleHead[MAX_ACTION_SIZE]; 
    char ruleBase[MAX_ACTION_SIZE]; 
    char *actionArray[MAX_ACTION_IN_RULE];
    char *recoveryArray[MAX_ACTION_IN_RULE];
    /*char configDirEV[200];*/
    char ruleSet[RULE_SET_DEF_LENGTH];
    char oldRuleBase[MAX_ACTION_SIZE];

    strcpy(ruleSet,"");
    strcpy(oldRuleBase,"");

  for( j = inx ; (j-inx) < inRuleStrct->MaxNumOfRules ; j++) {
    getRule(j, ruleBase, ruleHead, ruleCondition,ruleAction,ruleRecovery, MAX_RULE_LENGTH);
    if (strcmp(oldRuleBase,ruleBase)) {
      if (strlen(oldRuleBase) > 0)
	_writeString("stdout","---------------------------------------------------------------\n",rei);
      strcpy(oldRuleBase,ruleBase);
    }
    n = getActionRecoveryList(ruleAction,ruleRecovery,actionArray,recoveryArray);
    sprintf(outStr," %-5i%s.%s\n",j,ruleBase, ruleHead);
    _writeString("stdout",outStr,rei);
    if (strlen(ruleCondition) != 0) 
      sprintf(outStr,"      IF (%s) {\n",ruleCondition);
    else 
      sprintf(outStr,"      {\n");
    _writeString("stdout",outStr,rei);
    for (i = 0; i < n; i++) {
      /**
      if (strlen(actionArray[i]) < 20) {
	if (i == 0) 
	  sprintf(outStr,"      DO   %-20.20s[%s]\n",actionArray[i],recoveryArray[i]);
	else
	  sprintf(outStr,"      AND  %-20.20s[%s]\n",actionArray[i],recoveryArray[i]);
      }
      else {
	if (i == 0) 
	  sprintf(outStr,"      DO   %s       [%s]\n",actionArray[i],recoveryArray[i]);
	else
	  sprintf(outStr,"      AND  %s       [%s]\n",actionArray[i],recoveryArray[i]);
      }
      **/
      if (strlen(actionArray[i]) < 20) {
	if (recoveryArray[i] == NULL || 
	    strlen(recoveryArray[i]) == 0 || 
	    !strcmp(recoveryArray[i],"nop") || 
	    !strcmp(recoveryArray[i],"null")) 
	  sprintf(outStr,"        %-20.20s\n",actionArray[i]);
	else
	  sprintf(outStr,"        %-20.20s[%s]\n",actionArray[i],recoveryArray[i]);
      }
      else {
	if (recoveryArray[i] == NULL || 
	    strlen(recoveryArray[i]) == 0 || 
	    !strcmp(recoveryArray[i],"nop") || 
	    !strcmp(recoveryArray[i],"null")) 
	  sprintf(outStr,"        %s\n",actionArray[i]);
	else
	  sprintf(outStr,"        %s       [%s]\n",actionArray[i],recoveryArray[i]);
      }
      _writeString("stdout",outStr,rei);
    }
    _writeString("stdout","      }\n",rei);
  }
  _writeString("stdout","---------------------------------------------------------------\n",rei);
  return(0);
}

int msiAdmClearAppRuleStruct(ruleExecInfo_t *rei)
{

  int i;

  if ((i = isUserPrivileged(rei->rsComm)) != 0)
    return (i);
  i = clearRuleStruct(&appRuleStrct);
  if (i < 0)
    return(i);
  i = clearDVarStruct(&appRuleVarDef);
  if (i < 0)
    return(i);
  i = clearFuncMapStruct(&appRuleFuncMapDef);
  return(i);

}
int msiAdmAddAppRuleStruct(msParam_t *irbFilesParam, msParam_t *dvmFilesParam, 
			msParam_t *fnmFilesParam,  ruleExecInfo_t *rei)
{

  int i;

  if ((i = isUserPrivileged(rei->rsComm)) != 0)
    return (i);

  RE_TEST_MACRO ("Loopback on admAddAppRuleStruct");

  if (strlen((char *) irbFilesParam->inOutStruct) > 0) {
    i = readRuleStructFromFile((char *) irbFilesParam->inOutStruct, &appRuleStrct);
    if (i < 0)
      return(i);
  }
  if (strlen((char *) dvmFilesParam->inOutStruct) > 0) {
    i = readDVarStructFromFile((char *) dvmFilesParam->inOutStruct, &appRuleVarDef);
    if (i < 0)
      return(i);
  }
  if (strlen((char *) fnmFilesParam->inOutStruct) > 0) {
    i = readFuncMapStructFromFile((char *) fnmFilesParam->inOutStruct, &appRuleFuncMapDef);
    if (i < 0)
      return(i);
  }
  return(0);
  
}
