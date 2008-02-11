/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* objMetaOpr.h - header file for objMetaOpr.c
 */



#ifndef OBJ_META_OPR_H
#define OBJ_META_OPR_H

#include "rods.h"
#include "initServer.h"
#include "objInfo.h"
#include "dataObjInpOut.h"
#include "ruleExecSubmit.h"
#include "rcGlobalExtern.h"
#include "rsGlobalExtern.h"

/* definition for return value of resolveSingleReplCopy */
#define NO_GOOD_COPY	0
#define HAVE_GOOD_COPY	1

/* definition for trimjFlag in matchAndTrimRescGrp */
#define TRIM_MATCHED_RESC_INFO		0x1
#define REQUE_MATCHED_RESC_INFO		0x2
#define TRIM_MATCHED_OBJ_INFO		0x4
#define TRIM_UNMATCHED_OBJ_INFO		0x8
int
getRescInfo (rsComm_t *rsComm, char *defaultResc, keyValPair_t *condInput, 
rescGrpInfo_t **rescGrpInfo);
int
_getRescInfo (rsComm_t *rsComm, char *rescName, rescGrpInfo_t **rescGrpInfo);
int
resolveAndQueResc (char *rescName, char *rescGrpName,  
rescGrpInfo_t **rescGrpInfo);
int
resolveRescGrp (rsComm_t *rsComm, char *rescGroupName,
rescGrpInfo_t **rescGrpInfo);
int
resolveResc (char *rescName, rescInfo_t **rescInfo);
int
getNumResc (rescGrpInfo_t *rescGrpInfo);
int
sortResc (rescGrpInfo_t **rescGrpInfo, keyValPair_t *condInput, 
char *sortScheme);
int
getDataObjInfo (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t **dataObjInfoHead, char *accessPerm, int ignoreCondInput);
int
checkCollAccessPerm (rsComm_t *rsComm, char *collection, char *accessPerm);
int
updateDataObjReplStatus (rsComm_t *rsComm, int l1descInx, int replStatus);
int
dataObjExist (rsComm_t *rsComm, dataObjInp_t *dataObjInp);
int
sortObjInfoForRepl (dataObjInfo_t **dataObjInfoHead, 
dataObjInfo_t **oldDataObjInfoHead, int deleteOldFlag);
int
sortObjInfoForOpen ( dataObjInfo_t **dataObjInfoHead, keyValPair_t *condInput,
int writeFlag);
int
sortDataObjInfoRandom (dataObjInfo_t **dataObjInfoHead);
int
requeDataObjInfoByResc (dataObjInfo_t **dataObjInfoHead, char *preferedResc,
int writeFlag, int topFlag);
int
requeDataObjInfoByReplNum (dataObjInfo_t **dataObjInfoHead, int replNum);
dataObjInfo_t *
chkCopyInResc (dataObjInfo_t *dataObjInfoHead, rescGrpInfo_t *myRescGrpInfo);
int
chkAndTrimCopyInRescGrp (dataObjInfo_t **dataObjInfoHead, 
rescGrpInfo_t **rescGrpInfoHead, int trimDataObjFlag);
int
initDataObjInfoQuery (dataObjInp_t *dataObjInp, genQueryInp_t *genQueryInp,
int ignoreCondInput);
int
sortObjInfo (dataObjInfo_t **dataObjInfoHead,
dataObjInfo_t **dirtyArchInfo, dataObjInfo_t **dirtyCacheInfo,
dataObjInfo_t **oldArchInfo, dataObjInfo_t **oldCacheInfo);
int
chkOrphanFile (rsComm_t *rsComm, char *filePath, char *rescName,
dataObjInfo_t *dataObjInfo);
int
getNumDataObjInfo (dataObjInfo_t *dataObjInfoHead);
int
replRescGrpInfo (rescGrpInfo_t *srcRescGrpInfo, 
rescGrpInfo_t **destRescGrpInfo);
int
getReInfo (rsComm_t *rsComm, genQueryOut_t **genQueryOut);
int
getReInfoById (rsComm_t *rsComm, char *ruleExecId, genQueryOut_t **genQueryOut);
int
getNextQueuedRuleExec (rsComm_t *rsComm, genQueryOut_t **inGenQueryOut,
int startInx, ruleExecSubmitInp_t *queuedRuleExec, int statusFlag);
int
regExeStatus (rsComm_t *rsComm, char *ruleExecId, char *exeStatus);
int
runQueuedRuleExec (rsComm_t *rsComm, genQueryOut_t **genQueryOut,
time_t endTime, int statusFlag);
int
svrCloseQueryOut (rsComm_t *rsComm, genQueryOut_t *genQueryOut);
int
queryRescInRescGrp (rsComm_t *rsComm, char *rescGroupName,
genQueryOut_t **genQueryOut);
int
resolveSingleReplCopy (dataObjInfo_t **dataObjInfoHead,
dataObjInfo_t **oldDataObjInfoHead, rescGrpInfo_t **destRescGrpInfo,
dataObjInfo_t **destDataObjInfo, keyValPair_t *condInput);
int
isColl(rsComm_t *rsComm, char *objName, rodsLong_t *collId);
int
isData(rsComm_t *rsComm, char *objName, rodsLong_t *dataId);
int
isUser(rsComm_t *rsComm, char *objName);
int
isResc(rsComm_t *rsComm, char *objName);
int
isMeta(rsComm_t *rsComm, char *objName);
int
isToken(rsComm_t *rsComm, char *objName);
int
getObjType(rsComm_t *rsComm, char *objName, char * objType);
int
addAVUMetadataFromKVPairs (rsComm_t *rsComm, char *objName, char *inObjType,
                           keyValPair_t *kVP);
int
rsQueryDataObjInCollReCur (rsComm_t *rsComm, char *collection,
genQueryInp_t *genQueryInp, genQueryOut_t **genQueryOut, char *accessPerm,
int singleFlag);
int
rsQueryCollInColl (rsComm_t *rsComm, char *collection,
genQueryInp_t *genQueryInp, genQueryOut_t **genQueryOut);
int
matchDataObjInfoByCondInput (dataObjInfo_t **dataObjInfoHead,
dataObjInfo_t **oldDataObjInfoHead, keyValPair_t *condInput,
dataObjInfo_t **matchedDataObjInfo, dataObjInfo_t **matchedOldDataObjInfo);
int
resolveInfoForPhymv (dataObjInfo_t **dataObjInfoHead,
dataObjInfo_t **oldDataObjInfoHead, rescGrpInfo_t **destRescGrpInfo,
keyValPair_t *condInput, int multiCopyFlag);
int
matchAndTrimRescGrp (dataObjInfo_t **dataObjInfoHead,
rescGrpInfo_t **rescGrpInfoHead, int trimjFlag);
int
resolveInfoForTrim (dataObjInfo_t **dataObjInfoHead,
keyValPair_t *condInput);
int
requeDataObjInfoByDestResc (dataObjInfo_t **dataObjInfoHead,
keyValPair_t *condInput, int writeFlag, int topFlag);
int 
resolveSpecColl (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t **dataObjInfo, int writeFlag);
int
getStructFileType (specColl_t *specColl);
int
getDataObjInfoIncSpecColl (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t **dataObjInfo);
int
modCollInfo2 (rsComm_t *rsComm, specColl_t *specColl, int clearFlag);
int
regNewObjSize (rsComm_t *rsComm, char *objPath, int replNum,
rodsLong_t newSize);
#endif	/* OBJ_META_OPR_H */
