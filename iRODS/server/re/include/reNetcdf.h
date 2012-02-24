/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* reSysDataObjOpr.h - header file for reSysDataObjOpr.c
 */



#ifndef RE_NETCDF_H
#define RE_NETCDF_H

#include "apiHeaderAll.h"
#include "reGlobalsExtern.h"
#include "rsGlobalExtern.h"

int
msiNcOpen (msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam,
ruleExecInfo_t *rei);
int
msiNcCreate (msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam,
ruleExecInfo_t *rei);
int
msiNcClose (msParam_t *inpParam1, ruleExecInfo_t *rei);
int
msiNcInqId (msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *inpParam3,
msParam_t *outParam, ruleExecInfo_t *rei);
int
msiNcInqWithId (msParam_t *inpParam1, msParam_t *inpParam2,
msParam_t *inpParam3, msParam_t *outParam, ruleExecInfo_t *rei);
int
msiNcGetVarsByType (msParam_t *dataTypeParam, msParam_t *ncidParam,
msParam_t *varidParam, msParam_t *ndimParam, msParam_t *startParam,
msParam_t *countParam, msParam_t *strideParam,
msParam_t *outParam, ruleExecInfo_t *rei);
#ifdef LIB_CF
int
msiNccfGetVara (msParam_t *ncidParam, msParam_t *varidParam,
msParam_t *lvlIndexParam, msParam_t *timestepParam,
msParam_t *latRange0Param, msParam_t *latRange1Param,
msParam_t *lonRange0Param, msParam_t *lonRange1Param,
msParam_t *maxOutArrayLenParam, msParam_t *outParam, ruleExecInfo_t *rei);
#endif
int
msiNcGetArrayLen (msParam_t *inpParam, msParam_t *outParam,
ruleExecInfo_t *rei);
int
msiNcGetNumDim (msParam_t *inpParam, msParam_t *outParam,
ruleExecInfo_t *rei);
int
msiNcGetDataType (msParam_t *inpParam, msParam_t *outParam,
ruleExecInfo_t *rei);
int
msiNcGetElementInArray (msParam_t *arrayStructParam, msParam_t *indexParam,
msParam_t *outParam, ruleExecInfo_t *rei);
int
msiFloatToString (msParam_t *floatParam, msParam_t *stringParam,
ruleExecInfo_t *rei);
#endif	/* RE_NETCDF_H */
