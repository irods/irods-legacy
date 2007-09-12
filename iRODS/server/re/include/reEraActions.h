/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* reEraActions.h - header file for reEraActions.c
 */

#ifndef REERAACTIONS_H
#define REERAACTIONS_H

int msiCopyAVUMetadata(msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, ruleExecInfo_t *rei);
int msiGetDataObjAVUs(msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei);
int msiGetDataObjPSmeta(msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei);
int msiGetCollectionPSmeta(msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei);
int msiLoadMetadataFromFile(msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei);
int msiGetDataObjAIP(msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei);
int msiExportRecursiveCollMeta(msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei);
int msiSaveBufferToLocalFile(msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, ruleExecInfo_t *rei);

#endif	/* REERAACTIONS_H */

