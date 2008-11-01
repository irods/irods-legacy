/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* collInfoMS.h - header file for collInfoMS.c
 */

#ifndef COLLINFOMS_H
#define COLLINFOMS_H

#include "apiHeaderAll.h"
#include "objMetaOpr.h"
#include "miscUtil.h"


int msiGetCollectionContentsReport(msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, ruleExecInfo_t *rei);
int msiGetCollectionSize(msParam_t *collPath, msParam_t *outKVPairs, msParam_t *status, ruleExecInfo_t *rei);


#endif	/* COLLINFOMS_H */

