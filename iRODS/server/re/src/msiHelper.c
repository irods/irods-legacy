/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* reHelper.c */

#include "msiHelper.h"
int
msiGetStdoutInExecCmdOut (msParam_t *inpExecCmdOut, msParam_t *outStr,
ruleExecInfo_t *rei)
{
    char *strPtr;

    rei->status = getStdoutInExecCmdOut (inpExecCmdOut, &strPtr);

    if (rei->status < 0) return rei->status;

    fillStrInMsParam (outStr, strPtr);

    return rei->status;
}

int
msiGetStderrInExecCmdOut (msParam_t *inpExecCmdOut, msParam_t *outStr,
ruleExecInfo_t *rei)
{
    char *strPtr;

    rei->status = getStderrInExecCmdOut (inpExecCmdOut, &strPtr);

    if (rei->status < 0) return rei->status;

    fillStrInMsParam (outStr, strPtr);

    return rei->status;
}

