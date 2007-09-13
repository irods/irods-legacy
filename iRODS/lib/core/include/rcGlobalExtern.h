/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rcGlobalExtern.h - Extern global declaration for client API */

#ifndef RC_GLOBAL_EXTERN_H
#define RC_GLOBAL_EXTERN_H

#include "rods.h"
#include "packStruct.h"
#include "apiHandler.h"
#include "objInfo.h"

extern packConstantArray_t PackConstantTable[];
extern packInstructArray_t RodsPackTable[];
extern int ProcessType;
extern packInstructArray_t ApiPackTable[];
extern packType_t packTypeTable[];
extern int NumOfPackTypes;
extern apidef_t RcApiTable[];
extern int NumOfApi;
extern char *dataObjCond[];
extern char *compareOperator[];
extern char *rescCond[];
extern char *userCond[];
extern char *collCond[];
extern rescTypeDef_t RescTypeDef[];
extern int NumRescTypeDef;
extern rescClass_t RescClass[];
extern int NumRescClass;
extern bundleTypeDef_t BundleTypeDef[];
extern int NumBundleType;

#endif	/* RC_GLOBAL_EXTERN_H */
