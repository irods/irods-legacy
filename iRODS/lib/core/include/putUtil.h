/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* putUtil.h - Header for for putUtil.c */

#ifndef PUT_UTIL_H
#define PUT_UTIL_H

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"

#ifdef  __cplusplus
extern "C" {
#endif

int
putUtil (rcComm_t *conn, rodsEnv *myEnv, rodsArguments_t *myRodsArgs, 
rodsPathInp_t *rodsPathInp);
int
putFileUtil (rcComm_t *conn, char *srcPath, char *targPath, 
rodsLong_t srcSize, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs, 
dataObjInp_t *dataObjOprInp);
int
initCondForPut (rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs,
dataObjInp_t *dataObjOprInp, rodsRestart_t *rodsRestart);
int
putDirUtil (rcComm_t *conn, char *srcDir, char *targColl,
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, dataObjInp_t *dataObjOprInp,
rodsRestart_t *rodsRestart);

#ifdef  __cplusplus
}
#endif

#endif	/* PUT_UTIL_H */
