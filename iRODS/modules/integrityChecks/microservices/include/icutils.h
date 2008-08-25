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

void initStructs (bytesBuf_t* mybuf, genQueryOut_t** mygqout, genQueryInp_t* mygqin);
int msiListFields (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPout1, msParam_t *mPout2, ruleExecInfo_t *rei);
int msiListCollACL (msParam_t* mPin1, msParam_t* mPout1, msParam_t* mPout2, ruleExecInfo_t *rei);

/* junk functions */
int msiHiThere (ruleExecInfo_t *rei);
int hithere ();

#endif	/* ICUTILS */
