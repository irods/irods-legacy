/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/**
 * @file	icutils.h
 *
 * @brief	Declarations for the msiIntegrityChecks* microservices.
 */



#ifndef ICUTILS
#define ICUTILS

#include "rods.h"
#include "rodsGenQuery.h"

int msiListFields (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPout1, msParam_t *mPout2, ruleExecInfo_t *rei);
int msiListCollACL (msParam_t* mPin1, msParam_t* mPin2, msParam_t* mPin3, msParam_t* mPin4, msParam_t* mPout1, msParam_t* mPout2, ruleExecInfo_t *rei);

/* junk functions */
int msiHiThere (msParam_t* mPout1, ruleExecInfo_t *rei);
int msiTestWritePosInt (msParam_t* mPout1, ruleExecInfo_t *rei);

#endif	/* ICUTILS */
