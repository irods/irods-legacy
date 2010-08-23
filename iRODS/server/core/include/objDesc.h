/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* objDesc.h - header file for objDesc.c
 */



#ifndef OBJ_DESC_H
#define OBJ_DESC_H

#include "rods.h"
#include "initServer.h"
#include "objInfo.h"
#include "dataObjInpOut.h"
#include "fileRename.h"
#include "miscUtil.h"
#include "structFileSync.h"
#include "structFileExtAndReg.h"
#include "dataObjOpenAndStat.h"

#define NUM_L1_DESC	1026 	/* number of L1Desc */

#define ORPHAN_DIR	"orphan"
#define REPL_DIR	"replica"
#define CHK_ORPHAN_CNT_LIMIT  20  /* number of failed check before stopping */
/* definition for getNumThreads */

#define REG_CHKSUM	1
#define VERIFY_CHKSUM	2

/* values in l1desc_t is the desired value. values in dataObjInfo are
 * the values in rcat */
typedef struct l1desc {
    int l3descInx;
    int inuseFlag;
    int oprType;
    int openType;
    int oprStatus;
    int dataObjInpReplFlag;
    dataObjInp_t *dataObjInp;
    dataObjInfo_t *dataObjInfo;
    dataObjInfo_t *otherDataObjInfo;
    int copiesNeeded;
    rescGrpInfo_t *moreRescGrpInfo;
    rodsLong_t bytesWritten;    /* mark whether it has been written */
    rodsLong_t dataSize;	/* this is the target size. The size in 
				 * dataObjInfo is the registered size */
    int replStatus;     /* the replica status */
    int chksumFlag;     /* parsed from condition */
    int srcL1descInx;
    char chksum[NAME_LEN]; /* the input chksum */
#ifdef LOG_TRANSFERS
    struct timeval openStartTime;
#endif
    int remoteL1descInx;
    int stageFlag;
    rescInfo_t *replRescInfo;	/* if non NULL, repl to this resc on close */
    dataObjInfo_t *replDataObjInfo; /* if non NULL, repl to this dataObjInfo
				     * on close */
    rodsServerHost_t *remoteZoneHost;
} l1desc_t;

int
initL1Desc ();

int
allocL1Desc ();

int
freeL1Desc (int fileInx);

int
fillL1desc (int l1descInx, dataObjInp_t *dataObjInp,
dataObjInfo_t *dataObjInfo, int replStatus, rodsLong_t dataSize);

int
getFileMode (dataObjInp_t *dataObjInp);

int
getFileFlags (int l1descInx);

int
getFilePathName (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo,
dataObjInp_t *dataObjInp);
int
getVaultPathPolicy (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo,
vaultPathPolicy_t *outVaultPathPolicy);
int
setPathForGraftPathScheme (char *objPath, char *vaultPath, int addUserName,
char *userName, int trimDirCnt, char *outPath);
int 
setPathForRandomScheme (char *objPath, char *vaultPath, char *userName,
char *outPath);
int
resolveDupFilePath (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo,
dataObjInp_t *dataObjInp);
int
getchkPathPerm (rsComm_t *rsComm, dataObjInp_t *dataObjInp, dataObjInfo_t *dataObjInfo);
int
getErrno (int errCode);
int 
getCopiesFromCond (keyValPair_t *condInput);

int
getWriteFlag (int openFlag);
int
getCondQuery (keyValPair_t *condInput);
int
setCondQuery (int headL1descInx, int condQueryFlag,
dataObjInfo_t *nextDataObjInfo, dataObjInfo_t **otherDataObjInfo);
int
dataObjChksum (rsComm_t *rsComm, int l1descInx, keyValPair_t *regParam);
int
_dataObjChksum (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo, char **chksumStr);
int
getNumThreads (rsComm_t *rsComm, rodsLong_t dataSize, int inpNumThr, 
keyValPair_t *condInput, char *destRescName, char *srcRescName);
rodsLong_t 
getSizeInVault (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo);
int
initDataOprInp (dataOprInp_t *dataOprInp, int l1descInx, int oprType);
int
initDataObjInfoForRepl (rsComm_t *rsComm, dataObjInfo_t *destDataObjInfo,
dataObjInfo_t *srcDataObjInfo, rescInfo_t *destRescInfo, char *rescGroupName);
int
convL3descInx (int l3descInx);
int
dataObjChksumAndReg (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo,
char **chksumStr);
int
chkAndHandleOrphanFile (rsComm_t *rsComm, char *filePath, 
rescInfo_t *rescInfo, int replStatus);
int
initDataObjInfoWithInp (dataObjInfo_t *dataObjInfo, dataObjInp_t *dataObjInp);
int
allocL1desc ();
int
freeL1desc (int l1descInx);
int
closeAllL1desc (rsComm_t *rsComm);
int
initSpecCollDesc ();
int
allocSpecCollDesc ();
int
freeSpecCollDesc (int specCollInx);
int
renameFilePathToNewDir (rsComm_t *rsComm, char *newDir,
fileRenameInp_t *fileRenameInp, rescInfo_t *rescInfo, int renameFlag);
int
syncDataObjPhyPath (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t *dataObjInfoHead);
int
syncDataObjPhyPathS (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t *dataObjInfo);
int
syncCollPhyPath (rsComm_t *rsComm, char *collection);
int
isInVault (dataObjInfo_t *dataObjInfo);
int
initL1desc ();
int
initCollHandle ();
int
allocCollHandle ();
int
freeCollHandle (int handleInx);
int
rsInitQueryHandle (queryHandle_t *queryHandle, rsComm_t *rsComm);
int
initStructFileOprInp (rsComm_t *rsComm, structFileOprInp_t *structFileOprInp,
structFileExtAndRegInp_t *structFileExtAndRegInp,
dataObjInfo_t *dataObjInfo);
int
allocAndSetL1descForZoneOpr (int l3descInx, dataObjInp_t *dataObjInp,
rodsServerHost_t *remoteZoneHost, openStat_t *openStat);
int
getDefFileMode ();
int
getDefDirMode ();
int
isL1descInuse ();
int
getLogPathFromPhyPath (char *phyPath, rescInfo_t *rescInfo, char *outLogPath);
#endif	/* OBJ_DESC_H */
