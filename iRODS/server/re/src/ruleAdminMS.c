/**
 * @file ruleAdminMS.c
 *
 */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "reGlobalsExtern.h"
#include "rsGlobalExtern.h"
#include "dataObjCreate.h"
#include "objMetaOpr.h"
#include "regDataObj.h"
/* #include "reAction.h" */
#include "miscServerFunct.h"



/**
 * \fn msiAdmChangeCoreIRB (msParam_t *newFileNameParam, ruleExecInfo_t *rei)
 *
 * \brief  This microservice copies the specified file in the configuration 
 * directory 'server/config/reConfigs' onto the core.irb file in the same directory.
 *
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author   Arcot Rajasekar
 * \date     2007-04
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-13
 * \remark Jewel Ward - reviewed msi documentation, 2009-06-21
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-23
 * 
 * \note  This microservice expects the alternate file to be of the form *.irb and to be
 *   located in the configuration directory.
 *
 * \note This microservice requires iRODS administration privilege.
 *  
 * \note This microservice changes the core.irb file currently in the configuration
 * directory. It can be invoked through an irule. When the server is re-started, the 
 * new core file will be used by the rule engine.
 *
 * \usage
 *
 * As seen in clients/icommands/test/chgCoreToOrig.ir
 *
 * testrule||msiAdmChangeCoreIRB(*A)|nop
 *
 * \param[in] newFileNameParam - is a msParam of type STR_MS_T, which is a new core file name without the .irb extension.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect The core.irb file is replaced by the alternate core file.
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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

/**
 * \fn msiAdmAppendToTopOfCoreIRB (msParam_t *newFileNameParam, ruleExecInfo_t *rei)
 *
 * \brief  This is a microservice that changes the core.irb file currently in
 * the configuration directory 'server/config/reConfigs' by prepending the given
 * rules file to it. When the server is started next time, 
 * then the new core file will be used by the rule engine.
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author   Arcot Rajasekar
 * \date     2007-04
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-13
 * \remark Jewel Ward - reviewed msi documentation, 2009-06-21
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-23
 * 
 * \note This microservice requires iRODS administration privileges.
 *  
 * \note  This microservice expects the prepended file to be of the form *.irb and to be
 * located in the configuration directory.
 *
 * \usage
 *
 * As seen in clients/icommands/test/chgCoreToCore1.ir
 * 
 * testrule||msiAdmAppendToTopOfCoreIRB(*A)|nop
 * *A=testnewcore
 * ruleExecOut
 *
 * \param[in] newFileNameParam - is a msParam of type STR_MS_T, which is a prepended core file name without the .irb extension.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect core.irb file is prepended by the alternate new core file.
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa msiAdmShowIRB, msiAdmChangeCoreIRB
 * \bug  no known bugs
**/
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

/**
 * \fn msiAdmShowDVM (msParam_t *bufParam, ruleExecInfo_t *rei)
 *
 * \brief  This is a microservice that reads the data-value-mapping data structure
 * in the Rule Engine and pretty-prints that structure to the stdout buffer.
 *
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author   Arcot Rajasekar  
 * \date     2007-08
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-14
 * \remark Jewel Ward - reviewed msi documentation, 2009-06-21
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-23
 * 
 * \note This microservice uses a dummy parameter.
 *  
 * \note   Lists the currently loaded dollar variable mappings from the rule 
 *  engine memory. The list is written to stdout in ruleExecOut.
 *
 * \usage
 *
 * As seen in clients/icommands/test/ruleTest18.ir
 *
 * myTest||msiAdmShowDVM(*A)##msiAdmAddAppRuleStruct(*B,*B,*B)##msiAdmShowDVM(*C)|nop
 *
 * \param[in] bufParam - is a msParam (not used for anything, a dummy parameter)
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified - rei->MsParamArray->MsParam->ruleExecOut->stdout is modified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa msiAdmShowIRB, msiAdmShowFNM
 * \bug  no known bugs
**/
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

/**
 * \fn msiAdmShowFNM (msParam_t *bufParam, ruleExecInfo_t *rei)
 *
 * \brief  This is a microservice that reads the function-name-mapping data structure
 * in the Rule Engine and pretty-prints that structure to the stdout buffer.
 *
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author   Arcot Rajasekar  
 * \date     2007-08
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-14
 * \remark Jewel Ward - reviewed msi documentation, 2009-06-21
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-23
 * 
 * \note This microservice has a dummy parameter.
 *  
 * \note   This microservice lists the currently loaded microServices and action
 * name mappings from the rule engine memory. The list is written to stdout in ruleExecOut.
 *
 * \usage
 *
 * As seen in clients/icommands/test/ruleTest19.ir
 *
 * testrule||msiAdmShowFNM(*A)##msiAdmAddAppRuleStruct(*B,*B,*B)##msiAdmShowFNM(*C)|nop
 *
 * \param[in] bufParam - is a msParam (not used for anything, a dummy parameter)
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified - rei->MsParamArray->MsParam->ruleExecOut->stdout is modified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa msiAdmShowIRB, msiAdmShowDVM
 * \bug  no known bugs
**/
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

/**
 * \fn msiAdmShowIRB (msParam_t *bufParam, ruleExecInfo_t *rei)
 *
 * \brief  This is a microservice that reads the data structure in the Rule Engine, which holds the 
 * current set of Rules, and pretty-prints that structure to the stdout buffer.
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author   Arcot Rajasekar
 * \date     2007-06
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-14
 * \remark Jewel Ward - reviewed msi documentation, 2009-06-21
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-23
 * 
 * \note This microservice has a dummy parameter.
 *  
 * \usage
 *
 * As seen in clients/icommands/test/showCore.ir
 *
 * myTest||msiAdmShowIRB(*A)|nop
 *
 * \param[in] bufParam - is a msParam (not used for anything, a dummy parameter)
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified - rei->MsParamArray->MsParam->ruleExecOut->stdout is modified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa msiAdmShowDVM, msiAdmShowFNM
 * \bug  no known bugs
**/
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
      /*
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
      */
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

/**
 * \fn msiAdmClearAppRuleStruct (ruleExecInfo_t *rei)
 *
 * \brief  This is a microservice that clears the application level IRB Rules and DVM 
 * and FNM mappings that were loaded into the Rule engine's working memory.
 *
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author   Arcot Rajasekar
 * \date     2007-09
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-14
 * \remark Jewel Ward - reviewed msi documentation, 2009-06-21
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-23
 * 
 * \note This microservice needs iRODS administration privileges to perform 
 * this function.
 *  
 * \note   Clears the application structures in the working memory of the rule engine
 * holding the rules, $-variable mappings and microService name mappings.
 *
 * \usage
 *
 * testrule||msiAdmClearAppRuleStruct(*A)|nop
 *
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect The rule engine's application-level ruleset and mappings get cleared.
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa msiAdmAddAppRuleStruct, msiAdmShowIRB, msiAdmShowDVM, msiAdmShowFNM
 * \bug  no known bugs
**/
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

/**
 * \fn msiAdmAddAppRuleStruct(msParam_t *irbFilesParam, msParam_t *dvmFilesParam, 
 *  msParam_t *fnmFilesParam, ruleExecInfo_t *rei)
 *
 * \brief  This is a microservice that reads the given file in the configuration directory
 * 'server/config/reConfigs' and adds them to the Rule list being used by the Rule 
 * Engine. These Rules are loaded at the beginning of the core.irb file, and hence can
 * be used to override the core Rules from the core.irb file (i.e., it adds application level 
 * IRB Rules and DVM and FNM mappings to the Rule engine).
 *
 * \module core 
 *
 * \since pre-2.1
 *
 * \author  Arcot Rajasekar
 * \date    2007-09
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-13
 * \remark Jewel Ward - reviewed msi documentation, 2009-06-21
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-24
 * 
 * \note This microservice requires iRODS administration privileges.
 *  
 * \note Adds the given rules (irb) file and $-variable mapping (dvm) and microService
 * logical microService logical name mapping (fnm) files to the working memory
 * of the rule engine. Any subsequent rule or microServices will also use the newly 
 * prepended rules and mappings
 *
 * \usage
 *
 * As seen in clients/icommands/test/ruleTest18.ir
 *
 * myTest||msiAdmShowDVM(*A)##msiAdmAddAppRuleStruct(*B,*B,*B)##msiAdmShowDVM(*C)|nop
 *
 * \param[in] irbFilesParam - a msParam of type STR_MS_T, which is an application Rules file name without the .irb extension.
 * \param[in] dvmFilesParam - a msParam of type STR_MS_T, which is a variable file name mapping without the .dvm extension.
 * \param[in] fnmFilesParam - a msParam of type STR_MS_T, which is an application microService mapping file name without the .fnm extension.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence
 * \DolVarModified
 * \iCatAttrDependence
 * \iCatAttrModified
 * \sideeffect The rule engine's application ruleset and mappings get modified.
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa msiAdmClearAppRuleStruct, msiAdmShowIRB, msiAdmShowDVM, msiAdmShowFNM
 * \bug  no known bugs
**/
int msiAdmAddAppRuleStruct(msParam_t *irbFilesParam, msParam_t *dvmFilesParam, 
  msParam_t *fnmFilesParam, ruleExecInfo_t *rei)
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
