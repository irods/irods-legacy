/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* getUtil.h - Header for for getUtil.c */

#ifndef LS_UTIL_H
#define LS_UTIL_H

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"

#ifdef  __cplusplus
extern "C" {
#endif

int
lsUtil (rcComm_t *conn, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs,
rodsPathInp_t *rodsPathInp);
int
lsDataObjUtil (rcComm_t *conn, rodsPath_t *srcPath,
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs,
genQueryInp_t *genQueryInp);
int
printLsStrShort (char *srcPath);
int
lsDataObjUtilLong (rcComm_t *conn, char *srcPath, rodsEnv *myRodsEnv,
rodsArguments_t *rodsArgs, genQueryInp_t *genQueryInp);
int
printLsLong (rcComm_t *conn, rodsArguments_t *rodsArgs, 
genQueryOut_t *genQueryOut);
int
printLsColl (rcComm_t *conn, rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
genQueryOut_t *genQueryOut);
int
printLsShort (rcComm_t *conn, rodsArguments_t *rodsArgs, 
genQueryOut_t *genQueryOut);
int
initCondForLs (rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs,
genQueryInp_t *genQueryInp);
int
lsCollUtil (rcComm_t *conn, rodsPath_t *srcPath, rodsEnv *myRodsEnv,
rodsArguments_t *rodsArgs);
int
lsSpecCollUtil (rcComm_t *conn, rodsPath_t *srcPath, rodsEnv *myRodsEnv,
rodsArguments_t *rodsArgs);
int
printDataAcl (rcComm_t *conn, char *dataId);
int
printCollAcl (rcComm_t *conn, char *collId);
int
lsSpecDataObjUtilLong (rcComm_t *conn, rodsPath_t *srcPath, 
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs);
int
printSpecLs (rcComm_t *conn, rodsArguments_t *rodsArgs, rodsPath_t *srcPath,
genQueryOut_t *genQueryOut);
int
printSpecLsLong (char *objPath, char *ownerName, char *objSize,
char *modifyTime, specColl_t *specColl, rodsArguments_t *rodsArgs);
void 
printCollOrDir (char *myName, objType_t myType, rodsArguments_t *rodsArgs,
specColl_t *specColl);
#ifdef  __cplusplus
}
#endif

#endif	/* LS_UTIL_H */
