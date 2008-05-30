/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* miscUtil.h - Header file for miscUtil.c */

#ifndef MISC_UTIL_H
#define MISC_UTIL_H

#include "rodsPath.h"
#include "parseCommandLine.h"

typedef struct CollSqlResult {
    int rowCnt;
    int attriCnt;
    int continueInx;
    int totalRowCount;
    sqlResult_t collName; 
    sqlResult_t collType; 
    sqlResult_t collInfo1; 
    sqlResult_t collInfo2;
    sqlResult_t collOwner;
} collSqlResult_t;

typedef struct CollMetaInfo {
    char *collName;
    char *collOwner;
    specColl_t specColl;
} collMetaInfo_t;

typedef struct DataObjSqlResult {
    int rowCnt;
    int attriCnt;
    int continueInx;
    int totalRowCount;
    sqlResult_t collName;
    sqlResult_t dataName;
    sqlResult_t dataSize;
    sqlResult_t createTime;
    sqlResult_t modifyTime;
    sqlResult_t chksum;		/* chksum, replStatus and dataId are used only
				 * for rsync */
    sqlResult_t replStatus;
    sqlResult_t dataId;
    sqlResult_t resource;
    sqlResult_t phyPath;
    sqlResult_t ownerName;
    sqlResult_t replNum;
} dataObjSqlResult_t;

typedef struct DataObjMetaInfo {
    char *collName;
    char *dataName;
    char *dataSize;
    char *createTime;
    char *modifyTime;
    char *chksum;
    char *replStatus;
    char *dataId;
} dataObjMetaInfo_t;

/* definition for state in collHandle_t */
typedef enum {
    COLL_CLOSED,
    COLL_OPENED,
    COLL_DATA_OBJ_QUERIED,
    COLL_COLL_OBJ_QUERIED,
} collState_t;

typedef enum {
    RC_COMM,
    RS_COMM,
} connType_t;

/* struct for query by both client and server */
typedef struct QueryHandle {
    void *conn;         /* either rsComm or rcComm */
    connType_t connType;
    funcPtr querySpecColl;      /* rcQuerySpecColl or rsQuerySpecColl */
    funcPtr genQuery;           /* rcGenQuery or rsGenQuery */
} queryHandle_t;

/* definition for flag in rclOpenCollection and collHandle_t */
#define LONG_METADATA_FG     0x1     /* get verbose metadata */
#define VERY_LONG_METADATA_FG     0x2   /* get verbose metadata */
#define RECUR_QUERY_FG       0x4     /* get recursive query */

typedef struct CollHandle {
    collState_t state;
    int inuseFlag;
    int flags;
    int rowInx;
    queryHandle_t queryHandle;
    genQueryInp_t genQueryInp;
    dataObjInp_t dataObjInp;
    dataObjSqlResult_t dataObjSqlResult;
    collSqlResult_t collSqlResult;
} collHandle_t;
    
/* the output of rclReadCollection */
typedef struct CollEnt {
    objType_t objType;
    int replNum;
    int replStatus;
    int dummy;
    rodsLong_t dataSize;
    char *collName;		/* valid for dataObj and collection */
    char *dataName;
    char *dataId;
    char *createTime;
    char *modifyTime;
    char *chksum;
    char *resource;
    char *phyPath;
    char *ownerName;    	 /* valid for dataObj and collection */
    specColl_t specColl;	 /* valid only for collection */ 
} collEnt_t;

#ifdef  __cplusplus
extern "C" {
#endif
int
mkdirR (char *startDir, char *destDir, int mode);
int
mkColl (rcComm_t *conn, char *collection);
int
mkCollR (rcComm_t *conn, char *startColl, char *destColl);
int
getRodsObjType (rcComm_t *conn, rodsPath_t *rodsPath);
int
genAllInCollQCond (char *collection, char *collQCond);
int
queryCollInColl (queryHandle_t *queryHandle, char *collection,
int flags, genQueryInp_t *genQueryInp,
genQueryOut_t **genQueryOut);
int
queryDataObjInColl (queryHandle_t *queryHandle, char *collection,
int flags, genQueryInp_t *genQueryInp,
genQueryOut_t **genQueryOut);
int
setQueryInpForData (int flags, genQueryInp_t *genQueryInp);

int
printTiming (rcComm_t *conn, char *objPath, rodsLong_t fileSize,
char *localFile, struct timeval *startTime, struct timeval *endTime);
int
queryDataObjAcl (rcComm_t *conn, char *dataId, genQueryOut_t **genQueryOut);
int
queryCollAcl (rcComm_t *conn, char *collName, genQueryOut_t **genQueryOut);
char *
useridToName(rcComm_t *conn, char *username);
int
extractRodsObjType (rodsPath_t *rodsPath, sqlResult_t *dataId, 
sqlResult_t *replStatus, sqlResult_t *chksum, sqlResult_t *dataSize, 
int inx, int rowCnt);
int
genQueryOutToCollRes (genQueryOut_t **genQueryOut,
collSqlResult_t *collSqlResult);
int
setSqlResultValue (sqlResult_t *sqlResult, int attriInx, char *valueStr,
int rowCnt);
int
clearCollSqlResult (collSqlResult_t *collSqlResult);
int
clearDataObjSqlResult (dataObjSqlResult_t *dataObjSqlResult);
int
genQueryOutToDataObjRes (genQueryOut_t **genQueryOut,
dataObjSqlResult_t *dataObjSqlResult);
int
rclOpenCollection (rcComm_t *conn, char *collection, 
int flag, collHandle_t *collHandle);
int
rclReadCollection (rcComm_t *conn, collHandle_t *collHandle,
collEnt_t *collEnt);
int
readCollection (queryHandle_t *queryHandle, collHandle_t *collHandle,
collEnt_t *collEnt);
int
clearCollHandle (collHandle_t *collHandle, int freeSpecColl);
int
rclCloseCollection (collHandle_t *collHandle);
int
getNextCollMetaInfo (queryHandle_t *queryHandle, dataObjInp_t *dataObjInp,
genQueryInp_t *genQueryInp, collSqlResult_t *collSqlResult,
int *rowInx, collEnt_t *outCollEnt);
int
getNextDataObjMetaInfo (queryHandle_t *queryHandle, dataObjInp_t *dataObjInp,
genQueryInp_t *genQueryInp, dataObjSqlResult_t *dataObjSqlResult,
int *rowInx, collEnt_t *outCollEnt);
int
genCollResInColl (queryHandle_t *queryHandle, collHandle_t *collHandle);
int
genDataResInColl (queryHandle_t *queryHandle, collHandle_t *collHandle);
int
setQueryFlag (rodsArguments_t *rodsArgs);
int
rclInitQueryHandle (queryHandle_t *queryHandle, rcComm_t *conn);
#ifdef  __cplusplus
}
#endif

#endif	/* MISC_UTIL_H */
