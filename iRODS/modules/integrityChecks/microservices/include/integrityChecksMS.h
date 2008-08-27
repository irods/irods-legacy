/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/**
 * @file	integrityChecks.h
 *
 * @brief	Declarations for the msiIntegrityChecks* microservices.
 */



#ifndef INTEGRITYCHECKS_H
#define INTEGRITYCHECKS_H

#include "rods.h"
#include "rodsClient.h"
#include "rcMisc.h"


int msiCheckFilesizeRange (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPin3, msParam_t *mPout1, ruleExecInfo_t *rei);
int msiCheckFileDatatypes (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPout1, ruleExecInfo_t *rei);
int msiVerifyOwner (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPout1, msParam_t *mPout2, ruleExecInfo_t *rei);
int msiVerifyACL (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPin3, msParam_t mPin4, msParam_t *mPout1, msParam_t *mPout2, ruleExecInfo_t *rei);
int msiVerifyAVU (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPin3, msParam_t *mPin4, msParam_t *mPout1, 
	msParam_t *mPout2, ruleExecInfo_t *rei);
int msiVerifyExpiry (msParam_t *mPin1, msParam_t *mPin2, msParam_t* mPin3, msParam_t *mPout1, msParam_t *mPout2, ruleExecInfo_t *rei);


#endif	/* INTEGRITYCHECKS_H */
