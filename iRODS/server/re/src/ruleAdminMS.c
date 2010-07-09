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
 * \DolVarDependence none
 * \DolVarModified   none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect The core.irb file is replaced by the alternate core file.
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa none
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
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect core.irb file is prepended by the alternate new core file.
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
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
 * \DolVarDependence none
 * \DolVarModified - rei->MsParamArray->MsParam->ruleExecOut->stdout is modified
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect none
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
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
 * \DolVarDependence none
 * \DolVarModified - rei->MsParamArray->MsParam->ruleExecOut->stdout is modified
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
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
 * \DolVarDependence none
 * \DolVarModified - rei->MsParamArray->MsParam->ruleExecOut->stdout is modified
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect none
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
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
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect The rule engine's application-level ruleset and mappings get cleared.
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
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
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect The rule engine's application ruleset and mappings get modified.
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
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

/**************** Micro-Services for  ruleDB ********************************/
/** Rules **/


/**
 * \fn msiAdmReadRulesFromFileIntoStruct(msParam_t *inIrbFileNameParam, msParam_t *outCoreRuleStruct, ruleExecInfo_t *rei)
 *
 * \brief  This is a microservice that reads the given file in the configuration directory
 * 'server/config/reConfigs' or any file in the server local file system and 
 * reads them into a rule structure. 
 *
 * \module core 
 *
 * \since 2.5
 *
 * \author  Arcot Rajasekar
 * \date    2010
 * 
 * \note This microservice requires iRODS administration privileges.
 *  
 * \note Adds the given rules from an  irb-file to a given rule structure
 *
 * \usage
 *
 * As seen in clients/icommands/test/ruleTest37.ir
 *
 * myTestRule||msiAdmReadRulesFromFileIntoStruct(*FileName,*CS)##msiAdmInsertRulesFromStructIntoDB(*BaseName,*CS)|nop
 *
 * \param[in] inIrbFileNameParam - of type STR_MS_T, which is Rules file  in  irb format
 *      either in 'server/config/reConfigs' directory and  without the .irb extension, 
 *      or a full file  path in other directories in the server.
 * \param[out] outCoreRuleStruct - of type RuleStruct_MS_T (can be NULL in  which case it is allocated)
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect none
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa msiAdmInsertRulesFromStructIntoDB, msiGetRulesFromDBIntoStruct, msiAdmWriteRulesFromStructIntoFile
 * \bug  no known bugs
**/
int msiAdmReadRulesFromFileIntoStruct(msParam_t *inIrbFileNameParam, msParam_t *outCoreRuleStruct, ruleExecInfo_t *rei)
{

  int i;
  ruleStruct_t *coreRuleStrct;

  if ((i = isUserPrivileged(rei->rsComm)) != 0)
    return (i);

  RE_TEST_MACRO ("Loopback on msiAdmReadRulesFromFileIntoStruct");

  
  if (inIrbFileNameParam == NULL ||
      strcmp (inIrbFileNameParam->type,STR_MS_T) != 0 ||
      inIrbFileNameParam->inOutStruct == NULL ||
      strlen((char *) inIrbFileNameParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);
  if (outCoreRuleStruct->type != NULL &&
      strcmp (outCoreRuleStruct->type,RuleStruct_MS_T) == 0 &&
      outCoreRuleStruct->inOutStruct != NULL) {
    coreRuleStrct = (ruleStruct_t *) outCoreRuleStruct->inOutStruct;
  }
  else {
    coreRuleStrct = (ruleStruct_t *) malloc (sizeof(ruleStruct_t));
    coreRuleStrct->MaxNumOfRules = 0;    
  }
  i = readRuleStructFromFile((char*) inIrbFileNameParam->inOutStruct, coreRuleStrct);
  if (i != 0) {
    if (strcmp (outCoreRuleStruct->type,RuleStruct_MS_T) != 0 )    
      free(coreRuleStrct);
    return(i);
  }

  outCoreRuleStruct->inOutStruct = (void *) coreRuleStrct;
  if (outCoreRuleStruct->type == NULL || 
      strcmp (outCoreRuleStruct->type,RuleStruct_MS_T) != 0)
    outCoreRuleStruct->type = (char *) strdup(RuleStruct_MS_T);
  return(0);
}


/**
 * \fn msiAdmInsertRulesFromStructIntoDB(msParam_t *inIrbBaseNameParam, msParam_t *inCoreRuleStruct, ruleExecInfo_t *rei)
 *
 * \brief  This is a microservice that reads the content of a rule srtruture and writes them as a 
 * a new rule-base set by  populating the core rule  tables of the  iCAT.
 * It also maintains  versioning of the rule base in the iCAT by giving an older version number to an existing base set of rules. 
 * \module core 
 *
 * \since 2.5
 *
 * \author  Arcot Rajasekar
 * \date    2010
 * 
 * \note This microservice requires iRODS administration privileges.
 *  
 * \note Adds rules to the iCAT rule-base
 *
 * \usage
 *
 * As seen in clients/icommands/test/ruleTest37.ir
 *
 * myTestRule||msiAdmReadRulesFromFileIntoStruct(*FileName,*CS)##msiAdmInsertRulesFromStructIntoDB(*BaseName,*CS)|nop
 *
 * \param[in] inIrbBaseNameParam - of type STR_MS_T, which is name of the base that is being added.
 * \param[in] inCoreRuleStruct - of type RuleStruct_MS_T containing the rules.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified icat rule-tables get modified
 * \sideeffect 
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa msiAdmReadRulesFromFileIntoStruct, msiGetRulesFromDBIntoStruct, msiAdmWriteRulesFromStructIntoFile
 * \bug  no known bugs
**/
int msiAdmInsertRulesFromStructIntoDB(msParam_t *inIrbBaseNameParam, msParam_t *inCoreRuleStruct, ruleExecInfo_t *rei)
{

  ruleStruct_t *coreRuleStruct;
  int i;

  if ((i = isUserPrivileged(rei->rsComm)) != 0)
    return (i);

  RE_TEST_MACRO ("Loopback on msiAdmInsertRulesFromStructIntoDB");

  if (inIrbBaseNameParam == NULL || inCoreRuleStruct == NULL ||
      strcmp (inIrbBaseNameParam->type,STR_MS_T) != 0 ||
      strcmp (inCoreRuleStruct->type,RuleStruct_MS_T) != 0 ||
      inIrbBaseNameParam->inOutStruct == NULL ||
      inCoreRuleStruct->inOutStruct == NULL ||
      strlen((char *) inIrbBaseNameParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);

  coreRuleStruct = (ruleStruct_t *) inCoreRuleStruct->inOutStruct;
  i = insertRulesIntoDB((char *) inIrbBaseNameParam->inOutStruct, coreRuleStruct, rei);
  return(i);
    
}



/**
 * \fn msiGetRulesFromDBIntoStruct(msParam_t *inIrbBaseNameParam, msParam_t *inVersionParam,
 *                  msParam_t *outCoreRuleStruct, ruleExecInfo_t *rei)
 *
 * \brief  This is a microservice that queries the iCAT for rules with a given base name and 
 *     version number and populates the in a rule structure
 *
 * \module core 
 *
 * \since 2.5
 *
 * \author  Arcot Rajasekar
 * \date    2010
 * 
 * \note This microservice requires iRODS administration privileges.
 *  
 * \note Queries rules from the iCAT rule-base
 *
 * \usage
 *
 * As seen in clients/icommands/test/ruleTest38.ir
 *
 * myTestRule||msiGetRulesFromDBIntoStruct(*BaseName,*VersionStr, *CS)##msiAdmWriteRulesFromStructIntoFile(*FileName,*CS)|nop
 *
 * \param[in] inIrbBaseNameParam - of type STR_MS_T, which is name of the base that is being queried.
 * \param[in] inVersiobParam - of type STR_MS_T, which is the version string of the base being queired (use 0 for the current)
 * \param[out] outCoreRuleStruct - of type RuleStruct_MS_T (can be NULL in  which case it is allocated)
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect 
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa msiAdmReadRulesFromFileIntoStruct, msiAdmInsertRulesFromStructIntoDB, msiAdmWriteRulesFromStructIntoFile
 * \bug  no known bugs
**/
int
msiGetRulesFromDBIntoStruct(msParam_t *inIrbBaseNameParam, msParam_t *inVersionParam, 
			    msParam_t *outCoreRuleStruct, ruleExecInfo_t *rei)
{
    
  int i;
  ruleStruct_t *coreRuleStrct;

  RE_TEST_MACRO ("Loopback on msiGetRulesFromDBIntoStruct");

  if (inIrbBaseNameParam == NULL ||
      strcmp (inIrbBaseNameParam->type,STR_MS_T) != 0 ||
      inIrbBaseNameParam->inOutStruct == NULL ||
      strlen((char *) inIrbBaseNameParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);
  if (inVersionParam == NULL ||
      strcmp (inVersionParam->type,STR_MS_T) != 0 ||
      inVersionParam->inOutStruct == NULL ||
      strlen((char *) inVersionParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);
  if (outCoreRuleStruct->type != NULL &&
      strcmp (outCoreRuleStruct->type,RuleStruct_MS_T) == 0 &&
      outCoreRuleStruct->inOutStruct != NULL) {
    coreRuleStrct = (ruleStruct_t *) outCoreRuleStruct->inOutStruct;
  }
  else {
    coreRuleStrct = (ruleStruct_t *) malloc (sizeof(ruleStruct_t));
    coreRuleStrct->MaxNumOfRules = 0;
  }
  i = readRuleStructFromDB((char*) inIrbBaseNameParam->inOutStruct, (char*) inVersionParam->inOutStruct,  coreRuleStrct, rei);
  if (i != 0) {
    if (strcmp (outCoreRuleStruct->type,RuleStruct_MS_T) != 0 )
      free(coreRuleStrct);
    return(i);
  }

  outCoreRuleStruct->inOutStruct = (void *) coreRuleStrct;
  if (outCoreRuleStruct->type == NULL ||
      strcmp (outCoreRuleStruct->type,RuleStruct_MS_T) != 0)
    outCoreRuleStruct->type = (char *) strdup(RuleStruct_MS_T);
  return(0);
}


/**
 * \fn msiAdmWriteRulesFromStructIntoFile(msParam_t *inIrbFileNameParam, msParam_t *inCoreRuleStruct, ruleExecInfo_t *rei)
 *
 * \brief  This is a microservice that writes into a  given file the contents of  a rule structure
 * The file can be in n'server/config/reConfigs' or any file-path in the server local file system and 
 *
 * \module core 
 *
 * \since 2.5
 *
 * \author  Arcot Rajasekar
 * \date    2010
 * 
 * \note This microservice requires iRODS administration privileges.
 *  
 * \note writes a file with rules from the rue structure (in irb format)
 *
 * \usage
 *
 * As seen in clients/icommands/test/ruleTest38.ir
 *
 * myTestRule||msiGetRulesFromDBIntoStruct(*BaseName,*VersionStr, *CS)##msiAdmWriteRulesFromStructIntoFile(*FileName,*CS)|nop
 *
 * \param[in] inIrbFileNameParam - of type STR_MS_T, which is either a base-name in which case the file will 
 *      be written in 'server/config/reConfigs' directory with a  .irb extension, 
 *      or a full file  path in other directories in the server.
 * \param[in] inCoreRuleStruct - of type RuleStruct_MS_T 
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect  a new rule file is created
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa msiAdmReadRulesFromFileIntoStruct, msiAdmInsertRulesFromStructIntoDB, msiGetRulesFromDBIntoStruct
 * \bug  no known bugs
**/

int
msiAdmWriteRulesFromStructIntoFile(msParam_t *inIrbFileNameParam, msParam_t *inCoreRuleStruct, ruleExecInfo_t *rei)
{
  int i;
  ruleStruct_t *myRuleStruct;

  if ((i = isUserPrivileged(rei->rsComm)) != 0)
    return (i);

  RE_TEST_MACRO ("Loopback on msiAdmWriteRulesFromStructIntoFile");

  if (inIrbFileNameParam == NULL || inCoreRuleStruct == NULL ||
      strcmp (inIrbFileNameParam->type,STR_MS_T) != 0 ||
      strcmp (inCoreRuleStruct->type,RuleStruct_MS_T) != 0 ||
      inIrbFileNameParam->inOutStruct == NULL ||
      strlen((char *) inIrbFileNameParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);

  myRuleStruct = (ruleStruct_t *) inCoreRuleStruct->inOutStruct;
  i = writeRulesIntoFile((char *) inIrbFileNameParam->inOutStruct, myRuleStruct, rei);
  return(i);

}


/** Data Variable Mappings **/
#if 0


int msiAdmReadDVMapsFromFileIntoStruct(msParam_t *inDvmFileNameParam, msParam_t *outCoreDVMapStruct, ruleExecInfo_t *rei)
{

  int i;
  dvmStruct_t *coreDVMapStrct;

  if ((i = isUserPrivileged(rei->rsComm)) != 0)
    return (i);

  RE_TEST_MACRO ("Loopback on msiAdmReadDVMapsFromFileIntoStruct");

  
  if (inDvmFileNameParam == NULL ||
      strcmp (inDvmFileNameParam->type,STR_MS_T) != 0 ||
      inDvmFileNameParam->inOutStruct == NULL ||
      strlen((char *) inDvmFileNameParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);
  if (outCoreDVMapStruct->type != NULL &&
      strcmp (outCoreDVMapStruct->type,DVMapStruct_MS_T) == 0 &&
      outCoreDVMapStruct->inOutStruct != NULL) {
    coreDVMapStrct = (dvmStruct_t *) outCoreDVMapStruct->inOutStruct;
  }
  else {
    coreDVMapStrct = (dvmStruct_t *) malloc (sizeof(dvmStruct_t));
    coreDVMapStrct->MaxNumOfDVMaps = 0;    
  }
  i = readDVMapStructFromFile((char*) inDvmFileNameParam->inOutStruct, coreDVMapStrct);
  if (i != 0) {
    if (strcmp (outCoreDVMapStruct->type,DVMapStruct_MS_T) != 0 )    
      free(coreDVMapStrct);
    return(i);
  }

  outCoreDVMapStruct->inOutStruct = (void *) coreDVMapStrct;
  if (outCoreDVMapStruct->type == NULL || 
      strcmp (outCoreDVMapStruct->type,DVMapStruct_MS_T) != 0)
    outCoreDVMapStruct->type = (char *) strdup(DVMapStruct_MS_T);
  return(0);
}


int msiAdmInsertDVMapsFromStructIntoDB(msParam_t *inDvmBaseNameParam, msParam_t *inCoreDVMapStruct, ruleExecInfo_t *rei)
{

  dvmStruct_t *coreDVMapStruct;
  int i;

  if ((i = isUserPrivileged(rei->rsComm)) != 0)
    return (i);

  RE_TEST_MACRO ("Loopback on msiAdmInsertDVMapsFromStructIntoDB");

  if (inDvmBaseNameParam == NULL || inCoreDVMapStruct == NULL ||
      strcmp (inDvmBaseNameParam->type,STR_MS_T) != 0 ||
      strcmp (inCoreDVMapStruct->type,DVMapStruct_MS_T) != 0 ||
      inDvmBaseNameParam->inOutStruct == NULL ||
      inCoreDVMapStruct->inOutStruct == NULL ||
      strlen((char *) inDvmBaseNameParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);

  coreDVMapStruct = (dvmStruct_t *) inCoreDVMapStruct->inOutStruct;
  i = insertDVMapsIntoDB((char *) inDvmBaseNameParam->inOutStruct, coreDVMapStruct, rei);
  return(i);
    
}


int
msiGetDVMapsFromDBIntoStruct(msParam_t *inDvmBaseNameParam, msParam_t *inVersionParam, 
			    msParam_t *outCoreDVMapStruct, ruleExecInfo_t *rei)
{
    
  int i;
  dvmStruct_t *coreDVMapStrct;

  RE_TEST_MACRO ("Loopback on msiGetDVMapsFromDBIntoStruct");

  if (inDvmBaseNameParam == NULL ||
      strcmp (inDvmBaseNameParam->type,STR_MS_T) != 0 ||
      inDvmBaseNameParam->inOutStruct == NULL ||
      strlen((char *) inDvmBaseNameParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);
  if (inVersionParam == NULL ||
      strcmp (inVersionParam->type,STR_MS_T) != 0 ||
      inVersionParam->inOutStruct == NULL ||
      strlen((char *) inVersionParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);
  if (outCoreDVMapStruct->type != NULL &&
      strcmp (outCoreDVMapStruct->type,DVMapStruct_MS_T) == 0 &&
      outCoreDVMapStruct->inOutStruct != NULL) {
    coreDVMapStrct = (dvmStruct_t *) outCoreDVMapStruct->inOutStruct;
  }
  else {
    coreDVMapStrct = (dvmStruct_t *) malloc (sizeof(dvmStruct_t));
    coreDVMapStrct->MaxNumOfDVMaps = 0;
  }
  i = readDVMapStructFromDB((char*) inDvmBaseNameParam->inOutStruct, (char*) inVersionParam->inOutStruct,  coreDVMapStrct, rei);
  if (i != 0) {
    if (strcmp (outCoreDVMapStruct->type,DVMapStruct_MS_T) != 0 )
      free(coreDVMapStrct);
    return(i);
  }

  outCoreDVMapStruct->inOutStruct = (void *) coreDVMapStrct;
  if (outCoreDVMapStruct->type == NULL ||
      strcmp (outCoreDVMapStruct->type,DVMapStruct_MS_T) != 0)
    outCoreDVMapStruct->type = (char *) strdup(DVMapStruct_MS_T);
  return(0);
}

int
msiAdmWriteDVMapsFromStructIntoFile(msParam_t *inDvmFileNameParam, msParam_t *inCoreDVMapStruct, ruleExecInfo_t *rei)
{
  int i;
  dvmStruct_t *myDVMapStruct;

  if ((i = isUserPrivileged(rei->rsComm)) != 0)
    return (i);

  RE_TEST_MACRO ("Loopback on msiAdmWriteDVMapsFromStructIntoFile");

  if (inDvmFileNameParam == NULL || inCoreDVMapStruct == NULL ||
      strcmp (inDvmFileNameParam->type,STR_MS_T) != 0 ||
      strcmp (inCoreDVMapStruct->type,DVMapStruct_MS_T) != 0 ||
      inDvmFileNameParam->inOutStruct == NULL ||
      strlen((char *) inDvmFileNameParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);

  myDVMapStruct = (dvmStruct_t *) inCoreDVMapStruct->inOutStruct;
  i = writeDVMapsIntoFile((char *) inDvmFileNameParam->inOutStruct, myDVMapStruct, rei);
  return(i);

}

/** Function Mappings **/

int msiAdmReadFNMapsFromFileIntoStruct(msParam_t *inFnmFileNameParam, msParam_t *outCoreFNMapStruct, ruleExecInfo_t *rei)
{

  int i;
  fnmapStruct_t *coreFNMapStrct;

  if ((i = isUserPrivileged(rei->rsComm)) != 0)
    return (i);

  RE_TEST_MACRO ("Loopback on msiAdmReadFNMapsFromFileIntoStruct");

  
  if (inFnmFileNameParam == NULL ||
      strcmp (inFnmFileNameParam->type,STR_MS_T) != 0 ||
      inFnmFileNameParam->inOutStruct == NULL ||
      strlen((char *) inFnmFileNameParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);
  if (outCoreFNMapStruct->type != NULL &&
      strcmp (outCoreFNMapStruct->type,FNMapStruct_MS_T) == 0 &&
      outCoreFNMapStruct->inOutStruct != NULL) {
    coreFNMapStrct = (fnmapStruct_t *) outCoreFNMapStruct->inOutStruct;
  }
  else {
    coreFNMapStrct = (fnmapStruct_t *) malloc (sizeof(fnmapStruct_t));
    coreFNMapStrct->MaxNumOfFNMaps = 0;    
  }
  i = readFNMapStructFromFile((char*) inFnmFileNameParam->inOutStruct, coreFNMapStrct);
  if (i != 0) {
    if (strcmp (outCoreFNMapStruct->type,FNMapStruct_MS_T) != 0 )    
      free(coreFNMapStrct);
    return(i);
  }

  outCoreFNMapStruct->inOutStruct = (void *) coreFNMapStrct;
  if (outCoreFNMapStruct->type == NULL || 
      strcmp (outCoreFNMapStruct->type,FNMapStruct_MS_T) != 0)
    outCoreFNMapStruct->type = (char *) strdup(FNMapStruct_MS_T);
  return(0);
}


int msiAdmInsertFNMapsFromStructIntoDB(msParam_t *inFnmBaseNameParam, msParam_t *inCoreFNMapStruct, ruleExecInfo_t *rei)
{

  fnmapStruct_t *coreFNMapStruct;
  int i;

  if ((i = isUserPrivileged(rei->rsComm)) != 0)
    return (i);

  RE_TEST_MACRO ("Loopback on msiAdmInsertFNMapsFromStructIntoDB");

  if (inFnmBaseNameParam == NULL || inCoreFNMapStruct == NULL ||
      strcmp (inFnmBaseNameParam->type,STR_MS_T) != 0 ||
      strcmp (inCoreFNMapStruct->type,FNMapStruct_MS_T) != 0 ||
      inFnmBaseNameParam->inOutStruct == NULL ||
      inCoreFNMapStruct->inOutStruct == NULL ||
      strlen((char *) inFnmBaseNameParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);

  coreFNMapStruct = (fnmapStruct_t *) inCoreFNMapStruct->inOutStruct;
  i = insertFNMapsIntoDB((char *) inFnmBaseNameParam->inOutStruct, coreFNMapStruct, rei);
  return(i);
    
}


int
msiGetFNMapsFromDBIntoStruct(msParam_t *inFnmBaseNameParam, msParam_t *inVersionParam, 
			    msParam_t *outCoreFNMapStruct, ruleExecInfo_t *rei)
{
    
  int i;
  fnmapStruct_t *coreFNMapStrct;

  RE_TEST_MACRO ("Loopback on msiGetFNMapsFromDBIntoStruct");

  if (inFnmBaseNameParam == NULL ||
      strcmp (inFnmBaseNameParam->type,STR_MS_T) != 0 ||
      inFnmBaseNameParam->inOutStruct == NULL ||
      strlen((char *) inFnmBaseNameParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);
  if (inVersionParam == NULL ||
      strcmp (inVersionParam->type,STR_MS_T) != 0 ||
      inVersionParam->inOutStruct == NULL ||
      strlen((char *) inVersionParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);
  if (outCoreFNMapStruct->type != NULL &&
      strcmp (outCoreFNMapStruct->type,FNMapStruct_MS_T) == 0 &&
      outCoreFNMapStruct->inOutStruct != NULL) {
    coreFNMapStrct = (fnmapStruct_t *) outCoreFNMapStruct->inOutStruct;
  }
  else {
    coreFNMapStrct = (fnmapStruct_t *) malloc (sizeof(fnmapStruct_t));
    coreFNMapStrct->MaxNumOfFNMaps = 0;
  }
  i = readFNMapStructFromDB((char*) inFnmBaseNameParam->inOutStruct, (char*) inVersionParam->inOutStruct,  coreFNMapStrct, rei);
  if (i != 0) {
    if (strcmp (outCoreFNMapStruct->type,FNMapStruct_MS_T) != 0 )
      free(coreFNMapStrct);
    return(i);
  }

  outCoreFNMapStruct->inOutStruct = (void *) coreFNMapStrct;
  if (outCoreFNMapStruct->type == NULL ||
      strcmp (outCoreFNMapStruct->type,FNMapStruct_MS_T) != 0)
    outCoreFNMapStruct->type = (char *) strdup(FNMapStruct_MS_T);
  return(0);
}

int
msiAdmWriteFNMapsFromStructIntoFile(msParam_t *inFnmFileNameParam, msParam_t *inCoreFNMapStruct, ruleExecInfo_t *rei)
{
  int i;
  fnmapStruct_t *myFNMapStruct;

  if ((i = isUserPrivileged(rei->rsComm)) != 0)
    return (i);

  RE_TEST_MACRO ("Loopback on msiAdmWriteFNMapsFromStructIntoFile");

  if (inFnmFileNameParam == NULL || inCoreFNMapStruct == NULL ||
      strcmp (inFnmFileNameParam->type,STR_MS_T) != 0 ||
      strcmp (inCoreFNMapStruct->type,FNMapStruct_MS_T) != 0 ||
      inFnmFileNameParam->inOutStruct == NULL ||
      strlen((char *) inFnmFileNameParam->inOutStruct) == 0 )
    return(PARAOPR_EMPTY_IN_STRUCT_ERR);

  myFNMapStruct = (fnmapStruct_t *) inCoreFNMapStruct->inOutStruct;
  i = writeFNMapsIntoFile((char *) inFnmFileNameParam->inOutStruct, myFNMapStruct, rei);
  return(i);

}
#endif
