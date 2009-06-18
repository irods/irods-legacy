/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* msiHelper.h - header file for msiHelper.c
 */



#ifndef MSI_HELPER_H
#define MSI_HELPER_H

#include "rods.h"
#include "objMetaOpr.h"
#include "dataObjRepl.h"
#include "reGlobalsExtern.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"


int
msiGetStdoutInExecCmdOut (msParam_t *inpExecCmdOut, msParam_t *outStr,
ruleExecInfo_t *rei);
int
msiGetStderrInExecCmdOut (msParam_t *inpExecCmdOut, msParam_t *outStr,
ruleExecInfo_t *rei);
int
msiAddKeyValToMspStr (msParam_t *keyStr, msParam_t *valStr,
msParam_t *msKeyValStr, ruleExecInfo_t *rei);
int
msiWriteRodsLog (msParam_t *inpParam1,  msParam_t *outParam,
ruleExecInfo_t *rei);
int
msiSplitPath (msParam_t *inpPath,  msParam_t *outParentColl, 
msParam_t *outChildName, ruleExecInfo_t *rei);
#endif  /* MSI_HELPER_H */


