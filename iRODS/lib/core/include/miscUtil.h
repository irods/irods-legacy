/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* miscUtil.h - Header file for miscUtil.c */

#ifndef MISC_UTIL_H
#define MISC_UTIL_H

#include "rodsClient.h"
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

/* definition for flag in rcOpenCollection and collHandle_t */
#define LONG_METADATA       0x1     /* get verbose metadata */

typedef struct CollHandle {
    collState_t state;
    int flag;
    int rowInx;
    genQueryInp_t genQueryInp;
    dataObjInp_t dataObjInp;
    dataObjSqlResult_t dataObjSqlResult;
    collSqlResult_t collSqlResult;
} collHandle_t;
    
/* the output of rcReadCollection */
typedef struct CollEnt {
    objType_t objType;
    char collName[MAX_NAME_LEN];
    char dataName[MAX_NAME_LEN];
    rodsLong_t dataSize;
    char createTime[TIME_LEN];
    char modifyTime[TIME_LEN];
    char chksum[NAME_LEN];
    int replStatus;
    char dataId[NAME_LEN];
    char collOwner[NAME_LEN];    /* valid only for collection */
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
queryCollInCollReCur (rcComm_t *conn, char *collection,
rodsArguments_t *rodsArgs, genQueryInp_t *genQueryInp,
genQueryOut_t **genQueryOut);
int
queryCollInColl (rcComm_t *conn, char *collection,
rodsArguments_t *rodsArgs, genQueryInp_t *genQueryInp,
genQueryOut_t **genQueryOut);
int
queryCollInColl (rcComm_t *conn, char *collection,
rodsArguments_t *rodsArgs, genQueryInp_t *genQueryInp,
genQueryOut_t **genQueryOut);
int
queryDataObjInCollReCur (rcComm_t *conn, char *collection,
rodsArguments_t *rodsArgs, genQueryInp_t *genQueryInp,
genQueryOut_t **genQueryOut);
int
queryDataObjInColl (rcComm_t *conn, char *collection,
rodsArguments_t *rodsArgs, genQueryInp_t *genQueryInp,
genQueryOut_t **genQueryOut);
int
setQueryInpForLong (rodsArguments_t *rodsArgs,
genQueryInp_t *genQueryInp);

int
printTiming (rcComm_t *conn, char *objPath, rodsLong_t fileSize,
char *localFile, struct timeval *startTime, struct timeval *endTime);
int
queryDataObjAcl (rcComm_t *conn, char *dataId, genQueryOut_t **genQueryOut);
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
getNextCollMetaInfo (rcComm_t *conn, dataObjInp_t *dataObjInp,
genQueryInp_t *genQueryInp, collSqlResult_t *collSqlResult,
int *rowInx, collMetaInfo_t *outCollMetaInfo);
int
clearCollSqlResult (collSqlResult_t *collSqlResult);
int
clearDataObjSqlResult (dataObjSqlResult_t *dataObjSqlResult);
int
genQueryOutToDataObjRes (genQueryOut_t **genQueryOut,
dataObjSqlResult_t *dataObjSqlResult);
int
getNextDataObjMetaInfo (rcComm_t *conn, dataObjInp_t *dataObjInp,
genQueryInp_t *genQueryInp, dataObjSqlResult_t *dataObjSqlResult,
int *rowInx, dataObjMetaInfo_t *outDataObjMetaInfo);
int
rcOpenCollection (rcComm_t *conn, char *collection, 
int flag, collHandle_t *collHandle);
int
clearCollHandle (collHandle_t *collHandle);
int
rcCloseCollection (collHandle_t *collHandle);

#ifdef  __cplusplus
}
#endif

#endif	/* MISC_UTIL_H */
