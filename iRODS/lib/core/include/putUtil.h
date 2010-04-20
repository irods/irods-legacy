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

/* definition for flags in bulkOprInfo_t */
#define BULK_OPR_LARGE_FILES	0x0	/* bulk opr for large files */
#define BULK_OPR_SMALL_FILES	0x1	/* bulk opr for small files */

#define DEF_PHY_BUN_ROOT_DIR	"/tmp"

typedef struct {
    int flags;
    int count;
    int size;
    char cwd[MAX_NAME_LEN];
    char phyBunDir[MAX_NAME_LEN];
    char cachedSrcPath[MAX_NAME_LEN];
    char cachedSubPhyBunDir[MAX_NAME_LEN];
    char phyBunPath[MAX_NUM_BULK_OPR_FILES][MAX_NAME_LEN];
    bytesBuf_t bytesBuf;
} bulkOprInfo_t;

int
putUtil (rcComm_t **myConn, rodsEnv *myEnv, rodsArguments_t *myRodsArgs, 
rodsPathInp_t *rodsPathInp);
int
putFileUtil (rcComm_t *conn, char *srcPath, char *targPath, 
rodsLong_t srcSize, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs, 
dataObjInp_t *dataObjOprInp);
int
initCondForPut (rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs,
dataObjInp_t *dataObjOprInp, bulkOprInp_t *bulkOprInp,
rodsRestart_t *rodsRestart);
int
putDirUtil (rcComm_t **myConn, char *srcDir, char *targColl,
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, dataObjInp_t *dataObjOprInp,
bulkOprInp_t *bulkOprInp, rodsRestart_t *rodsRestart, bulkOprInfo_t *bulkOprInfo);
int
bulkPutDirUtil (rcComm_t **myConn, char *srcDir, char *targColl,
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, dataObjInp_t *dataObjOprInp,
bulkOprInp_t *bulkOprInp, rodsRestart_t *rodsRestart);
int
getPhyBunDir (char *phyBunRootDir, char *userName, char *outPhyBunDir);
int
bulkPutFileUtil (rcComm_t *conn, char *srcPath, char *targPath,
rodsLong_t srcSize, int createMode, rodsEnv *myRodsEnv, 
rodsArguments_t *myRodsArgs, bulkOprInp_t *bulkOprInp, 
bulkOprInfo_t *bulkOprInfo);
int
getPhyBunPath (char *collection, char *objPath, char *phyBunDir,
char *outPhyBunPath);
int
tarAndBulkPut (rcComm_t *conn, bulkOprInp_t *bulkOprInp,
bulkOprInfo_t *bulkOprInfo);
int
clearBulkOprInfo (bulkOprInfo_t *bulkOprInfo);
#ifdef  __cplusplus
}
#endif

#endif	/* PUT_UTIL_H */
