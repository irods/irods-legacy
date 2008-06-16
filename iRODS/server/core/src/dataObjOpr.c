/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* dataObjOpr.c - L1 type operation. Will call low level l1desc drivers
 */

#include "rodsDef.h"
#include "dataObjOpr.h"
#include "rodsDef.h"
#include "rsGlobalExtern.h"
#include "fileChksum.h"
#include "modDataObjMeta.h"
#include "objMetaOpr.h"
#include "dataObjClose.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"
#include "reDefines.h"
#include "reSysDataObjOpr.h"
#include "genQuery.h"
#include "rodsClient.h"

int
initL1desc ()
{
    memset (L1desc, 0, sizeof (l1desc_t) * NUM_L1_DESC);
    return (0);
}

int
allocL1desc ()
{
    int i;

    for (i = 3; i < NUM_FILE_DESC; i++) {
        if (L1desc[i].inuseFlag <= FD_FREE) {
            L1desc[i].inuseFlag = FD_INUSE;
            return (i);
        };
    }

    rodsLog (LOG_NOTICE,
     "allocL1desc: out of L1desc");

    return (SYS_OUT_OF_FILE_DESC);
}

int
initSpecCollDesc ()
{
    memset (SpecCollDesc, 0, sizeof (specCollDesc_t) * NUM_SPEC_COLL_DESC);
    return (0);
}

int
allocSpecCollDesc ()
{
    int i;

    for (i = 1; i < NUM_FILE_DESC; i++) {
        if (SpecCollDesc[i].inuseFlag <= FD_FREE) {
            SpecCollDesc[i].inuseFlag = FD_INUSE;
            return (i);
        };
    }

    rodsLog (LOG_NOTICE,
     "allocSpecCollDesc: out of SpecCollDesc");

    return (SYS_OUT_OF_FILE_DESC);
}

int
freeSpecCollDesc (int specCollInx)
{
    if (specCollInx < 1 || specCollInx >= NUM_SPEC_COLL_DESC) {
        rodsLog (LOG_NOTICE,
         "freeSpecCollDesc: specCollInx %d out of range", specCollInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }

    if (SpecCollDesc[specCollInx].dataObjInfo != NULL) {
        freeDataObjInfo (SpecCollDesc[specCollInx].dataObjInfo);
    }

    memset (&SpecCollDesc[specCollInx], 0, sizeof (specCollDesc_t));

    return (0);
}

int
closeAllL1desc (rsComm_t *rsComm)
{
    int i;

    if (rsComm == NULL) {
	return 0;
    }
    for (i = 3; i < NUM_FILE_DESC; i++) {
        if (L1desc[i].inuseFlag == FD_INUSE && 
	  L1desc[i].l3descInx > 2) {
	    l3Close (rsComm, i);
	}
    }
    return (0);
}

int
freeL1desc (int l1descInx)
{
    if (l1descInx < 3 || l1descInx >= NUM_FILE_DESC) {
        rodsLog (LOG_NOTICE,
         "freeL1desc: l1descInx %d out of range", l1descInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }

    if (L1desc[l1descInx].dataObjInfo != NULL) {
        freeDataObjInfo (L1desc[l1descInx].dataObjInfo);
    }

    if (L1desc[l1descInx].otherDataObjInfo != NULL) {
        freeAllDataObjInfo (L1desc[l1descInx].otherDataObjInfo);
    }

    memset (&L1desc[l1descInx], 0, sizeof (l1desc_t));

    return (0);
}

int
fillL1desc (int l1descInx, dataObjInp_t *dataObjInp,
dataObjInfo_t *dataObjInfo, int replStatus, rodsLong_t dataSize)
{
    keyValPair_t *condInput;
    char *tmpPtr;

    condInput = &dataObjInp->condInput;

    L1desc[l1descInx].dataObjInp = dataObjInp;
    L1desc[l1descInx].dataObjInfo = dataObjInfo;
    if (dataObjInp != NULL) {
	L1desc[l1descInx].oprType = dataObjInp->oprType;
    }
    L1desc[l1descInx].replStatus = replStatus;
    L1desc[l1descInx].dataSize = dataSize;
    if (condInput != NULL && condInput->len > 0) {
	if ((tmpPtr = getValByKey (condInput, REG_CHKSUM_KW)) != NULL) {
	    L1desc[l1descInx].chksumFlag = REG_CHKSUM;
	    rstrcpy (L1desc[l1descInx].chksum, tmpPtr, NAME_LEN);
	} else if ((tmpPtr = getValByKey (condInput, VERIFY_CHKSUM_KW)) != 
	  NULL) {
	    L1desc[l1descInx].chksumFlag = VERIFY_CHKSUM;
	    rstrcpy (L1desc[l1descInx].chksum, tmpPtr, NAME_LEN);
	}
    }
    return (0);
}

int 
queResc (rescInfo_t *myRescInfo, char *rescGroupName,
rescGrpInfo_t **rescGrpInfoHead, int topFlag)
{
    rescGrpInfo_t *myRescGrpInfo;
    int status;

    if (myRescInfo == NULL)
	return (0);
 
    myRescGrpInfo = (rescGrpInfo_t *) malloc (sizeof (rescGrpInfo_t));
    memset (myRescGrpInfo, 0, sizeof (rescGrpInfo_t));

    myRescGrpInfo->rescInfo = myRescInfo;

    if (rescGroupName != NULL) {
	rstrcpy (myRescGrpInfo->rescGroupName, rescGroupName, NAME_LEN);
    }

    status = queRescGrp (rescGrpInfoHead, myRescGrpInfo, topFlag);

    return (status);

}

int 
queRescGrp (rescGrpInfo_t **rescGrpInfoHead, rescGrpInfo_t *myRescGrpInfo, 
int topFlag)
{
    rescGrpInfo_t *tmpRescGrpInfo, *lastRescGrpInfo = NULL;

    tmpRescGrpInfo = *rescGrpInfoHead;
    if (topFlag > 0) {
        *rescGrpInfoHead = myRescGrpInfo;
        myRescGrpInfo->next = tmpRescGrpInfo;
    } else {
        while (tmpRescGrpInfo != NULL) {
            lastRescGrpInfo = tmpRescGrpInfo;
            tmpRescGrpInfo = tmpRescGrpInfo->next;
        }

        if (lastRescGrpInfo == NULL) {
            *rescGrpInfoHead = myRescGrpInfo;
        } else {
            lastRescGrpInfo->next = myRescGrpInfo;
        }
    }

    return (0);
}

int
initDataObjInfoWithInp (dataObjInfo_t *dataObjInfo, dataObjInp_t *dataObjInp)
{
    char *rescName, *dataType, *filePath;
    keyValPair_t *condInput;

    condInput = &dataObjInp->condInput;

    memset (dataObjInfo, 0, sizeof (dataObjInfo_t));
    rstrcpy (dataObjInfo->objPath, dataObjInp->objPath, MAX_NAME_LEN);
    rescName = getValByKey (condInput, RESC_NAME_KW);
    if (rescName != NULL) {
        rstrcpy (dataObjInfo->rescName, rescName, LONG_NAME_LEN);
    }

    dataType = getValByKey (condInput, DATA_TYPE_KW);
    if (dataType != NULL) {
        rstrcpy (dataObjInfo->dataType, dataType, NAME_LEN);
    } else {
	rstrcpy (dataObjInfo->dataType, "generic", NAME_LEN);
    }

    filePath = getValByKey (condInput, FILE_PATH_KW);
    if (filePath != NULL) {
        rstrcpy (dataObjInfo->filePath, filePath, MAX_NAME_LEN);
    }

    return (0);
}

int
getFileMode (int l1descInx)
{
    int createMode;

    dataObjInp_t *dataObjInp = L1desc[l1descInx].dataObjInp;

    if (dataObjInp != NULL && 
      (dataObjInp->createMode & 0110) != 0) {
	if ((DEFAULT_FILE_MODE & 0070) != 0) {
	    createMode = DEFAULT_FILE_MODE | 0110;
	} else {
	    createMode = DEFAULT_FILE_MODE | 0100;
	}
    } else {
	createMode = DEFAULT_FILE_MODE;
    }

    return (createMode);
}

int
getFileFlags (int l1descInx)
{
    int flags;

    dataObjInp_t *dataObjInp = L1desc[l1descInx].dataObjInp;

    if (dataObjInp != NULL) { 
	flags = dataObjInp->openFlags;
    } else {
        flags = O_RDONLY;
    }

    return (flags);
}

int
getFilePathName (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo,
dataObjInp_t *dataObjInp)
{
    char *filePath;
    vaultPathPolicy_t vaultPathPolicy;
    int status;

    if (dataObjInp != NULL && 
      (filePath = getValByKey (&dataObjInp->condInput, FILE_PATH_KW)) != NULL 
      && strlen (filePath) > 0) {
        rstrcpy (dataObjInfo->filePath, filePath, MAX_NAME_LEN);
	return (0);
    }

    /* Make up a physical path */ 
    if (dataObjInfo->rescInfo == NULL) {
        rodsLog (LOG_ERROR,
          "getFilePathName: rescInfo for %s not resolved", 
	  dataObjInp->objPath);
        return (SYS_INVALID_RESC_INPUT);
    }

    status = getVaultPathPolicy (rsComm, dataObjInfo, &vaultPathPolicy);
    if (status < 0) {
	return (status);
    }

    if (vaultPathPolicy.scheme == GRAFT_PATH_S) {
	status = setPathForGraftPathScheme (dataObjInp->objPath, 
	 dataObjInfo->rescInfo->rescVaultPath, vaultPathPolicy.addUserName,
	 rsComm->clientUser.userName, vaultPathPolicy.trimDirCnt, 
	  dataObjInfo->filePath);
    } else {
        status = setPathForRandomScheme (dataObjInp->objPath,
          dataObjInfo->rescInfo->rescVaultPath, rsComm->clientUser.userName,
	  dataObjInfo->filePath);
    }
    if (status < 0) {
	return (status);
    }

    return (status);
}

int
getVaultPathPolicy (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo,
vaultPathPolicy_t *outVaultPathPolicy)
{
    ruleExecInfo_t rei;
    msParam_t *msParam;
    int status;

    if (outVaultPathPolicy == NULL || dataObjInfo == NULL || rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "getVaultPathPolicy: NULL input");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    } 
    initReiWithDataObjInp (&rei, rsComm, NULL);
   
    rei.doi = dataObjInfo;
    status = applyRule ("acSetVaultPathPolicy", NULL, &rei, NO_SAVE_REI);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "getVaultPathPolicy: rule acSetVaultPathPolicy error, status = %d",
          status);
        return (status);
    }

    if ((msParam = getMsParamByLabel (&rei.inOutMsParamArray,
      VAULT_PATH_POLICY)) == NULL) {
        /* use the default */
        outVaultPathPolicy->scheme = DEF_VAULT_PATH_SCHEME;
        outVaultPathPolicy->addUserName = DEF_ADD_USER_FLAG;
        outVaultPathPolicy->trimDirCnt = DEF_TRIM_DIR_CNT;
    } else {
        *outVaultPathPolicy = *((vaultPathPolicy_t *) msParam->inOutStruct);
        clearMsParamArray (&rei.inOutMsParamArray, 1);
    }
    return (0);
}

int 
setPathForRandomScheme (char *objPath, char *vaultPath, char *userName,
char *outPath)
{
    int myRandom;
    int dir1, dir2;
    char logicalCollName[MAX_NAME_LEN];
    char logicalFileName[MAX_NAME_LEN];
    int status;

    myRandom = random (); 
    dir1 = myRandom & 0xf;
    dir2 = (myRandom >> 4) & 0xf;

    status = splitPathByKey(objPath,
                           logicalCollName, logicalFileName, '/');

    if (status < 0) {
        rodsLog (LOG_ERROR,
	  "setPathForRandomScheme: splitPathByKey error for %s, status = %d",
	  outPath, status);
        return (status);
    }

    snprintf (outPath, MAX_NAME_LEN,
      "%s/%s/%d/%d/%s.%d", vaultPath, userName, dir1, dir2, 
      logicalFileName, (uint) time (NULL));
    return (0);
}

int 
setPathForGraftPathScheme (char *objPath, char *vaultPath, int addUserName,
char *userName, int trimDirCnt, char *outPath)
{
    int i;
    char *objPathPtr, *tmpPtr;
    int len;

    objPathPtr = objPath + 1;

    for (i = 0; i < trimDirCnt; i++) {
	tmpPtr = strchr (objPathPtr, '/');
	if (tmpPtr == NULL) {
            rodsLog (LOG_ERROR,
              "setPathForGraftPathScheme: objPath %s too short", objPath);
	    break;	/* just use the shorten one */
	} else {
	    /* skip over '/' */
	    objPathPtr = tmpPtr + 1;
	    /* don't skip over the trash path */
	    if (i == 0 && strncmp (objPathPtr, "trash/", 6) == 0) break; 
	}
    }

    if (addUserName > 0 && userName != NULL) {
        len = snprintf (outPath, MAX_NAME_LEN,
          "%s/%s/%s", vaultPath, userName, objPathPtr);
    } else {
        len = snprintf (outPath, MAX_NAME_LEN,
          "%s/%s", vaultPath, objPathPtr);
    }

    if (len >= MAX_NAME_LEN) {
	rodsLog (LOG_ERROR,
	  "setPathForGraftPathScheme: filePath %s too long", objPath);
	return (USER_STRLEN_TOOLONG);
    } else {
        return (0);
    }
}

/* resolveDupFilePath - try to resolve deplicate file path in the same
 * resource.
 */

int
resolveDupFilePath (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo,
dataObjInp_t *dataObjInp)
{
    char tmpStr[NAME_LEN];
    char *filePath;

    if (getSizeInVault (rsComm, dataObjInfo) == SYS_PATH_IS_NOT_A_FILE) {
	/* a dir */
	return (SYS_PATH_IS_NOT_A_FILE);
    }
    if (chkAndHandleOrphanFile (rsComm, dataObjInfo->filePath, 
     dataObjInfo->rescInfo, dataObjInfo->replStatus) >= 0) {
        /* this is an orphan file or has been renamed */
        return 0;
    }

    if (dataObjInp != NULL) {
        filePath = getValByKey (&dataObjInp->condInput, FILE_PATH_KW);
        if (filePath != NULL && strlen (filePath) > 0) {
            return -1;
	}
    }

    if (strlen (dataObjInfo->filePath) >= MAX_NAME_LEN - 3) {
        return -1;
    }

    snprintf (tmpStr, NAME_LEN, ".%d", dataObjInfo->replNum);
    strcat (dataObjInfo->filePath, tmpStr);

    return (0);
}

int
getchkPathPerm (rsComm_t *rsComm, dataObjInp_t *dataObjInp, 
dataObjInfo_t *dataObjInfo)
{
    int chkPathPerm;
    char *filePath;
    rescInfo_t *rescInfo;
    ruleExecInfo_t rei;

    if (rsComm->clientUser.authInfo.authFlag == LOCAL_PRIV_USER_AUTH) {
        return (NO_CHK_PATH_PERM);
    }

    if (dataObjInp == NULL || dataObjInfo == NULL) {
        return (NO_CHK_PATH_PERM);
    }

    rescInfo = dataObjInfo->rescInfo;

    if ((filePath = getValByKey (&dataObjInp->condInput, FILE_PATH_KW)) != NULL 
      && strlen (filePath) > 0) {
        /* the user input a path */
        if (rescInfo == NULL) {
            chkPathPerm = NO_CHK_PATH_PERM;
        } else {
    	    initReiWithDataObjInp (&rei, rsComm, dataObjInp);
	    rei.doi = dataObjInfo;
	    rei.status = CHK_PERM_FLAG;		/* default */
	    applyRule ("acNoChkFilePathPerm", NULL, &rei, NO_SAVE_REI);
	    if (rei.status == CHK_PERM_FLAG) {
                chkPathPerm = RescTypeDef[rescInfo->rescTypeInx].chkPathPerm;
	    } else {
		chkPathPerm = NO_CHK_PATH_PERM;
	    }
        }
    } else {
            chkPathPerm = NO_CHK_PATH_PERM;
    }
    return (chkPathPerm);
}

int
getErrno (int errCode)
{
    int myErrno;

    myErrno = errCode % 1000;

    return (myErrno * (-1));
}

int 
getCopiesFromCond (keyValPair_t *condInput)
{
    char *myValue;

    myValue = getValByKey (condInput, COPIES_KW);

    if (myValue == NULL) {
	return (1);
    } else if (strcmp (myValue, "all") == 0) {
	return (ALL_COPIES);
    } else {
	return (atoi (myValue));
    }
}

int
getWriteFlag (int openFlag)
{
    if (openFlag & O_WRONLY || openFlag & O_RDWR) {
	return (1);
    } else {
	return (0);
    }
}

/* getCondQuery - check whether the datObj query will be based on condition 
 * XXXXX - this routine is deplicated because the replStatus is adjusted in
 * ModDataObjMeta 
 */

int
getCondQuery (keyValPair_t *condInput)
{
    int i;

    if (condInput == NULL) {
	return (0);
    }

    for (i = 0; i < condInput->len; i++) {
        if (strcmp (condInput->keyWord[i], "regChksum") != 0 &&
	  strcmp (condInput->keyWord[i], "verifyChksum") != 0 &&
	  strcmp (condInput->keyWord[i], "copies") != 0) {
	    return (1);
	}
    }

    return (0);
}

int
getRescCnt (rescGrpInfo_t *myRescGrpInfo)
{
    rescGrpInfo_t *tmpRescGrpInfo;
    int rescCnt = 0;

    tmpRescGrpInfo = myRescGrpInfo;

    while (tmpRescGrpInfo != NULL) {
	rescCnt ++;
	tmpRescGrpInfo = tmpRescGrpInfo->next;
    }

    return (rescCnt);
}

rodsLong_t 
getSizeInVault (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo)
{
    rodsStat_t *myStat = NULL;
    int status;
    rodsLong_t mysize;

    status = l3Stat (rsComm, dataObjInfo, &myStat);

    if (status < 0) {
	rodsLog (LOG_DEBUG,
	  "getSizeInVault: l3Stat error for %s. status = %d",
	  dataObjInfo->filePath, status);
	return (status);
    } else {
        if (myStat->st_mode & S_IFDIR) {
            return ((rodsLong_t) SYS_PATH_IS_NOT_A_FILE);
        }
	mysize = myStat->st_size;
	free (myStat);
	return (mysize);
    }
}

int
dataObjChksum (rsComm_t *rsComm, int l1descInx, keyValPair_t *regParam)
{
    int status;

    char *chksumStr = NULL;	/* computed chksum string */
    dataObjInfo_t *dataObjInfo = L1desc[l1descInx].dataObjInfo;
    int oprType = L1desc[l1descInx].oprType;
    int srcL1descInx;
    dataObjInfo_t *srcDataObjInfo;

    if (L1desc[l1descInx].chksumFlag == VERIFY_CHKSUM) {
	status = _dataObjChksum (rsComm, dataObjInfo, &chksumStr);

	if (status < 0) {
	    return (status);
	}

	if (strlen (L1desc[l1descInx].chksum) > 0) {
	    /* from a put type operation */
	    /* verify against the input value. */
	    if (strcmp (L1desc[l1descInx].chksum, chksumStr) != 0) {
		rodsLog (LOG_NOTICE,
		 "dataObjChksum: mismach chksum for %s. input = %s, compute %s",
		  dataObjInfo->objPath,
		  L1desc[l1descInx].chksum, chksumStr);
		free (chksumStr);
		return (USER_CHKSUM_MISMATCH);
	    }
            if (strcmp (dataObjInfo->chksum, chksumStr) != 0) {
                /* not the same as in rcat */
                addKeyVal (regParam, CHKSUM_KW, chksumStr);
            }
            free (chksumStr);
            return (0);
	} else if (oprType == REPLICATE_DEST) { 
	    if (strlen (dataObjInfo->chksum) > 0) {
	        /* for replication, the chksum in dataObjInfo was duplicated */
                if (strcmp (dataObjInfo->chksum, chksumStr) != 0) {
                    rodsLog (LOG_NOTICE,
                     "dataObjChksum: mismach chksum for %s.Rcat=%s,computed %s",
                     dataObjInfo->objPath, dataObjInfo->chksum, chksumStr);
		    status = USER_CHKSUM_MISMATCH;
		} else {
		    /* not need to register because reg repl will do it */
		    status = 0;
		}
	    } else {
		/* just register it */
		addKeyVal (regParam, CHKSUM_KW, chksumStr);
		status = 0;
	    }
	    free (chksumStr);
	    return (status);
        } else if (oprType == COPY_DEST) { 
	    /* created through copy */
	    srcL1descInx = L1desc[l1descInx].srcL1descInx;
	    if (srcL1descInx <= 2) {
		/* not a valid srcL1descInx */
	        rodsLog (LOG_NOTICE,
	          "dataObjChksum: invalid srcL1descInx %d fopy copy",
		  srcL1descInx);
		/* just register it for now */
                addKeyVal (regParam, CHKSUM_KW, chksumStr);
	        free (chksumStr);
		return (0);
	    } 
	    srcDataObjInfo = L1desc[srcL1descInx].dataObjInfo;
	    
            if (strlen (srcDataObjInfo->chksum) > 0) {
                if (strcmp (srcDataObjInfo->chksum, chksumStr) != 0) {
                    rodsLog (LOG_NOTICE,
                     "dataObjChksum: mismach chksum for %s.Rcat=%s,computed %s",
                     dataObjInfo->objPath, srcDataObjInfo->chksum, chksumStr);
                     status = USER_CHKSUM_MISMATCH;
                } else {
		    addKeyVal (regParam, CHKSUM_KW, chksumStr);
                    status = 0;
                }
            } else {
                /* just register it */
                addKeyVal (regParam, CHKSUM_KW, chksumStr);
                status = 0;
            }
            free (chksumStr);
            return (status);
	} else {
	    addKeyVal (regParam, CHKSUM_KW, chksumStr);
	    free (chksumStr);
	    return (0); 
	}
    }

    /* assume REG_CHKSUM */

    if (strlen (L1desc[l1descInx].chksum) > 0) { 
        /* from a put type operation */

        if (strcmp (dataObjInfo->chksum, L1desc[l1descInx].chksum) != 0) {
            /* not the same as in rcat */
            addKeyVal (regParam, CHKSUM_KW, L1desc[l1descInx].chksum);
	}
	return (0);
    } else if (oprType == COPY_DEST) {
        /* created through copy */
        srcL1descInx = L1desc[l1descInx].srcL1descInx;
        if (srcL1descInx <= 2) {
            /* not a valid srcL1descInx */
            rodsLog (LOG_NOTICE,
              "dataObjChksum: invalid srcL1descInx %d fopy copy",
              srcL1descInx);
	    /* do nothing */
            return (0);
        }
        srcDataObjInfo = L1desc[srcL1descInx].dataObjInfo;
        if (strlen (srcDataObjInfo->chksum) > 0) {
            addKeyVal (regParam, CHKSUM_KW, srcDataObjInfo->chksum);
        }
	return (0);
    }
    return (0);
}

int 
_dataObjChksum (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo, char **chksumStr)
{
    fileChksumInp_t fileChksumInp;
    int rescTypeInx;
    int status;
    rescInfo_t *rescInfo = dataObjInfo->rescInfo;

    rescTypeInx = rescInfo->rescTypeInx;

    switch (RescTypeDef[rescTypeInx].rescCat) {
      case FILE_CAT:
        memset (&fileChksumInp, 0, sizeof (fileChksumInp));
        fileChksumInp.fileType = RescTypeDef[rescTypeInx].driverType;
        rstrcpy (fileChksumInp.addr.hostAddr, rescInfo->rescLoc,
          NAME_LEN);
        rstrcpy (fileChksumInp.fileName, dataObjInfo->filePath, MAX_NAME_LEN);
	status = rsFileChksum (rsComm, &fileChksumInp, chksumStr);
        break;
      default:
        rodsLog (LOG_NOTICE,
          "_dataObjChksum: rescCat type %d is not recognized",
          RescTypeDef[rescTypeInx].rescCat);
        status = SYS_INVALID_RESC_TYPE;
        break;
    }
    return (status);
}

/* getNumThreads - get the number of threads.
 * inpNumThr - 0 - server decide
 *	       < 0 - NO_THREADING 	
 *	       > 0 - num of threads wanted
 */

int
getNumThreads (rsComm_t *rsComm, rodsLong_t dataSize, int inpNumThr, 
keyValPair_t *condInput)
{
    ruleExecInfo_t rei;
    dataObjInp_t doinp;
    int numThr, status;

    if (inpNumThr == NO_THREADING)
        return 0;

    if (dataSize < 0)
        return 1;

    if (dataSize <= MAX_SZ_FOR_SINGLE_BUF) {
        if (inpNumThr > 0) {
            inpNumThr = 1;
        } else {
            return 0;
        }
    }

    if (getValByKey (condInput, NO_PARA_OP_KW) != NULL) {
        /* client specify no para opr */
        return (1);
    }

#ifndef PARA_OPR
    return (1);
#endif

    memset (&doinp, 0, sizeof (doinp));
    doinp.numThreads = inpNumThr;

    doinp.dataSize = dataSize;

    initReiWithDataObjInp (&rei, rsComm, &doinp);

    status = applyRule ("acSetNumThreads", NULL, &rei, NO_SAVE_REI);

    if (status < 0) {
        rodsLog (LOG_ERROR,
	  "getNumThreads: acGetNumThreads error, status = %d",
	  status);
    } else {
	numThr = rei.status;
    }

    return (numThr);
}

int
initDataOprInp (dataOprInp_t *dataOprInp, int l1descInx, int oprType)
{
    dataObjInfo_t *dataObjInfo;
    dataObjInp_t  *dataObjInp;

    dataObjInfo = L1desc[l1descInx].dataObjInfo;
    dataObjInp = L1desc[l1descInx].dataObjInp;

    memset (dataOprInp, 0, sizeof (dataOprInp_t));

    dataOprInp->oprType = oprType;
    dataOprInp->numThreads = dataObjInp->numThreads;
    dataOprInp->offset = dataObjInp->offset;
    if (oprType == PUT_OPR && dataObjInp->dataSize > 0) {
	dataOprInp->dataSize = dataObjInp->dataSize;
    } else if (dataObjInfo->dataSize > 0) { 
	dataOprInp->dataSize = dataObjInfo->dataSize;
    } else {
        dataOprInp->dataSize = dataObjInp->dataSize;
    }
    if (oprType == PUT_OPR || oprType == COPY_TO_LOCAL_OPR ||
      oprType == SAME_HOST_COPY_OPR) {
        dataOprInp->destL3descInx = L1desc[l1descInx].l3descInx;
        dataOprInp->destRescTypeInx = dataObjInfo->rescInfo->rescTypeInx;
    } else if (oprType == GET_OPR || COPY_TO_REM_OPR) {
        dataOprInp->srcL3descInx = L1desc[l1descInx].l3descInx;
        dataOprInp->srcRescTypeInx = dataObjInfo->rescInfo->rescTypeInx;
    }
    if (getValByKey (&dataObjInp->condInput, STREAMING_KW) != NULL) {
        addKeyVal (&dataOprInp->condInput, STREAMING_KW, "");
    }

    if (getValByKey (&dataObjInp->condInput, NO_PARA_OP_KW) != NULL) {
        addKeyVal (&dataOprInp->condInput, NO_PARA_OP_KW, "");
    }

    return (0);
}

int
initDataObjInfoForRepl (dataObjInfo_t *destDataObjInfo, 
dataObjInfo_t *srcDataObjInfo, rescInfo_t *destRescInfo)
{
    memset (destDataObjInfo, 0, sizeof (dataObjInfo_t));
    *destDataObjInfo = *srcDataObjInfo;
    destDataObjInfo->filePath[0] = '\0';
    rstrcpy (destDataObjInfo->rescName, destRescInfo->rescName, NAME_LEN);
    destDataObjInfo->replNum = destDataObjInfo->dataId = 0;
    destDataObjInfo->rescInfo = destRescInfo;

    return (0);
}

int 
convL3descInx (int l3descInx)
{
    if (l3descInx <= 2 || FileDesc[l3descInx].inuseFlag == 0 ||
     FileDesc[l3descInx].rodsServerHost == NULL) {
        return l3descInx;
    }

    if (FileDesc[l3descInx].rodsServerHost->localFlag == LOCAL_HOST) {
        return (l3descInx);
    } else {
        return (FileDesc[l3descInx].fd);
    }
}   

int
getRescTypeInx (char *rescType)
{
    int i;

    if (rescType == NULL) {
	return SYS_INTERNAL_NULL_INPUT_ERR;
    }

    for (i = 0; i < NumRescTypeDef; i++) {
	if (strstr (rescType, RescTypeDef[i].typeName) != NULL) {
	    return (i);
	}
    }
    rodsLog (LOG_NOTICE,
      "getRescTypeInx: No match for input rescType %s", rescType);
    
    return (UNMATCHED_KEY_OR_INDEX);
}

int
getRescClassInx (char *rescClass) 
{
    int i;

    if (rescClass == NULL) {
        return SYS_INTERNAL_NULL_INPUT_ERR;
    }

    for (i = 0; i < NumRescClass; i++) {
        if (strstr (rescClass, RescClass[i].className) != NULL) {
	    if (strstr (rescClass, "primary") != NULL) {
		return (i | PRIMARY_FLAG);
	    } else {
                return (i);
	    }
        }
    }
    rodsLog (LOG_NOTICE,
      "getRescClassInx: No match for input rescClass %s", rescClass);

    return (UNMATCHED_KEY_OR_INDEX);
}

int
dataObjChksumAndReg (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo, 
char **chksumStr) 
{
    keyValPair_t regParam;
    modDataObjMeta_t modDataObjMetaInp;
    int status;

    status = _dataObjChksum (rsComm, dataObjInfo, chksumStr);
    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "dataObjChksumAndReg: _dataObjChksum error for %s, status = %d",
          dataObjInfo->objPath, status);
        return (status);
    }

    /* register it */
    memset (&regParam, 0, sizeof (regParam));
    addKeyVal (&regParam, CHKSUM_KW, *chksumStr);

    modDataObjMetaInp.dataObjInfo = dataObjInfo;
    modDataObjMetaInp.regParam = &regParam;

    status = rsModDataObjMeta (rsComm, &modDataObjMetaInp);

    clearKeyVal (&regParam);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "rsDataObjGet: rsModDataObjMeta error for %s, status = %d",
         dataObjInfo->objPath, status);
	/* don't return error because it is not fatal */
    }

    return (0);
}

/* chkAndHandleOrphanFile - Check whether the file is an orphan file.
 * If it is, rename it.  
 * If it belongs to an old copy, move the old path and register it.
 *
 * return 0 - the filePath is NOT an orphan file.
 *        1 - the filePath is an orphan file and has been renamed.
 *        < 0 - error
 */

int
chkAndHandleOrphanFile (rsComm_t *rsComm, char *filePath, rescInfo_t *rescInfo,
int replStatus)
{
    fileRenameInp_t fileRenameInp;
    int status;
    dataObjInfo_t myDataObjInfo;
    int rescTypeInx = rescInfo->rescTypeInx;

    if (RescTypeDef[rescTypeInx].rescCat != FILE_CAT) {
	/* can't do anything with non file type */
	return (-1);
    }

    if (strlen (filePath) + 17 >= MAX_NAME_LEN) {
	/* the new path name will be too long to add "/orphan + random" */
	return (-1);
    }
 
    /* check if the input filePath is assocated with a dataObj */

    memset (&myDataObjInfo, 0, sizeof (myDataObjInfo));
    memset (&fileRenameInp, 0, sizeof (fileRenameInp));
    if ((status = chkOrphanFile (
      rsComm, filePath, rescInfo->rescName, &myDataObjInfo)) == 0) {
        rstrcpy (fileRenameInp.oldFileName, filePath, MAX_NAME_LEN);
	/* not an orphan file */
	if (replStatus > OLD_COPY) {
            modDataObjMeta_t modDataObjMetaInp;
            keyValPair_t regParam;

	    /* a new copy. rename and reg the path of the old one */
            status = renameFilePathToNewDir (rsComm, REPL_DIR, &fileRenameInp, 
	      rescInfo, 1);
            if (status < 0) {
                return (status);
	    }
	    /* register the change */
	    memset (&regParam, 0, sizeof (regParam));
            addKeyVal (&regParam, FILE_PATH_KW, fileRenameInp.newFileName);
	    modDataObjMetaInp.dataObjInfo = &myDataObjInfo;
	    modDataObjMetaInp.regParam = &regParam;
	    status = rsModDataObjMeta (rsComm, &modDataObjMetaInp);
            clearKeyVal (&regParam);
	    if (status < 0) {
                rodsLog (LOG_ERROR,
                 "chkAndHandleOrphan: rsModDataObjMeta of %s error. stat = %d",
                 fileRenameInp.newFileName, status);
		/* need to rollback the change in path */
                rstrcpy (fileRenameInp.oldFileName, fileRenameInp.newFileName, 
		  MAX_NAME_LEN);
                rstrcpy (fileRenameInp.newFileName, filePath, MAX_NAME_LEN);
    	        status = rsFileRename (rsComm, &fileRenameInp);

    	        if (status < 0) {
        	    rodsLog (LOG_ERROR,
                     "chkAndHandleOrphan: rsFileRename %s failed, status = %d",
          	     fileRenameInp.oldFileName, status);
		    return (status);
		}
		/* this thing still failed */
		return (-1);
	    } else {
		return (0);
	    }
	} else {
            /* this is an old copy. change the path but don't
	     * actually rename it */
            rstrcpy (fileRenameInp.oldFileName, filePath, MAX_NAME_LEN);
            status = renameFilePathToNewDir (rsComm, REPL_DIR, &fileRenameInp,
              rescInfo, 0);
            if (status >= 0) {
                rstrcpy (filePath, fileRenameInp.newFileName, MAX_NAME_LEN);
                return (0);
            } else {
                return (status);
            }
	}

    } else if (status > 0) {
        /* this is an orphan file. need to rename it */
        rstrcpy (fileRenameInp.oldFileName, filePath, MAX_NAME_LEN);
	status = renameFilePathToNewDir (rsComm, ORPHAN_DIR, &fileRenameInp, 
	  rescInfo, 1);
	if (status >= 0) {
	    return (1);
	} else {
            return (status);
	}
    } else {
	/* error */
	return (status);
    }
}

int
renameFilePathToNewDir (rsComm_t *rsComm, char *newDir,
fileRenameInp_t *fileRenameInp, rescInfo_t *rescInfo, int renameFlag)
{
    int len, status;
    char *oldPtr, *newPtr;
    int rescTypeInx = rescInfo->rescTypeInx;
    char *filePath = fileRenameInp->oldFileName;

    fileRenameInp->fileType = RescTypeDef[rescTypeInx].driverType;

    rstrcpy (fileRenameInp->addr.hostAddr, rescInfo->rescLoc, NAME_LEN);

    len = strlen (rescInfo->rescVaultPath);

    if (len <= 0) {
	return (-1);
    }
    
    if (strncmp (filePath, rescInfo->rescVaultPath, len) != 0) {
	/* not in rescVaultPath */
	return -1;
    }

    rstrcpy (fileRenameInp->newFileName, rescInfo->rescVaultPath, MAX_NAME_LEN);
    oldPtr = filePath + len;
    newPtr = fileRenameInp->newFileName + len;

    snprintf (newPtr, MAX_NAME_LEN - len, "/%s%s.%-d", newDir, oldPtr, 
     (uint) random());
    
    if (renameFlag > 0) {
        status = rsFileRename (rsComm, fileRenameInp);
        if (status < 0) {
            rodsLog (LOG_NOTICE,
             "renameFilePathToNewDir:rsFileRename from %s to %s failed,stat=%d",
              filePath, fileRenameInp->newFileName, status);
	    return -1;
        } else {
            return (0); 
        }
    } else {
	return (0);
    }
}

int
syncDataObjPhyPath (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t *dataObjInfoHead)
{
    dataObjInfo_t *tmpDataObjInfo;
    int status;
    int savedStatus = 0;

    tmpDataObjInfo = dataObjInfoHead;
    while (tmpDataObjInfo != NULL) {
	status = syncDataObjPhyPathS (rsComm, dataObjInp, tmpDataObjInfo);
	if (status < 0) {
	    savedStatus = status;
	}
	tmpDataObjInfo = tmpDataObjInfo->next;
    }

    return (savedStatus);
} 

int
syncDataObjPhyPathS (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t *dataObjInfo)
{
    int status, status1;
    fileRenameInp_t fileRenameInp;
    rescInfo_t *rescInfo;
    int rescTypeInx;
    modDataObjMeta_t modDataObjMetaInp;
    keyValPair_t regParam;
    vaultPathPolicy_t vaultPathPolicy;

    status = getVaultPathPolicy (rsComm, dataObjInfo, &vaultPathPolicy);
    if (status < 0) {
	rodsLog (LOG_NOTICE,
          "syncDataObjPhyPathS: getVaultPathPolicy error for %s, status = %d",
	  dataObjInfo->objPath, status);
    } else {
	if (vaultPathPolicy.scheme != GRAFT_PATH_S) {
	    /* no need to sync */
	    return (0);
	}
    }

    if (isInVault (dataObjInfo) == 0) {
	/* not in vault. */
	return (0);
    }

    /* Save the current objPath */
    memset (&fileRenameInp, 0, sizeof (fileRenameInp));
    rstrcpy (fileRenameInp.oldFileName, dataObjInfo->filePath, 
      MAX_NAME_LEN);
    if (dataObjInp == NULL) {
	dataObjInp_t myDdataObjInp;
	memset (&myDdataObjInp, 0, sizeof (myDdataObjInp));
	rstrcpy (myDdataObjInp.objPath, dataObjInfo->objPath, MAX_NAME_LEN);
        status = getFilePathName (rsComm, dataObjInfo, &myDdataObjInp);
    } else {
        status = getFilePathName (rsComm, dataObjInfo, dataObjInp);
    }

    if (status < 0) {
	return status;
    }
    if (strcmp (fileRenameInp.oldFileName, dataObjInfo->filePath) == 0) {
	return (0);
    }

    rescInfo = dataObjInfo->rescInfo;
    /* see if the new file exist */
    if (getSizeInVault (rsComm, dataObjInfo) >= 0) {
        if (chkAndHandleOrphanFile (rsComm, dataObjInfo->filePath,
          rescInfo, OLD_COPY) <= 0) {
            rodsLog (LOG_ERROR,
             "syncDataObjPhyPath:newFileName %s to %s already exists",
              dataObjInfo->filePath);
	    return (SYS_INVALID_FILE_PATH);
	}
    }

    /* rename it */
    rescTypeInx = rescInfo->rescTypeInx;
    fileRenameInp.fileType = RescTypeDef[rescTypeInx].driverType;
    rstrcpy (fileRenameInp.addr.hostAddr, rescInfo->rescLoc, NAME_LEN);
    rstrcpy (fileRenameInp.newFileName, dataObjInfo->filePath,
      MAX_NAME_LEN);
 
    status = rsFileRename (rsComm, &fileRenameInp);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "syncDataObjPhyPath:rsFileRename from %s to %s failed,stat=%d",
          fileRenameInp.oldFileName, fileRenameInp.newFileName);
	return (status);
    }

    /* register the change */
    memset (&regParam, 0, sizeof (regParam));
    addKeyVal (&regParam, FILE_PATH_KW, fileRenameInp.newFileName);
    modDataObjMetaInp.dataObjInfo = dataObjInfo;
    modDataObjMetaInp.regParam = &regParam;
    status = rsModDataObjMeta (rsComm, &modDataObjMetaInp);
    clearKeyVal (&regParam);
    if (status < 0) {
        char tmpPath[MAX_NAME_LEN]; 
        rodsLog (LOG_ERROR,
         "syncDataObjPhyPath: rsModDataObjMeta of %s error. stat = %d",
         fileRenameInp.newFileName, status);
        /* need to rollback the change in path */
	rstrcpy (tmpPath, fileRenameInp.oldFileName, MAX_NAME_LEN);
        rstrcpy (fileRenameInp.oldFileName, fileRenameInp.newFileName,
          MAX_NAME_LEN);
        rstrcpy (fileRenameInp.newFileName, tmpPath, MAX_NAME_LEN);
        status1 = rsFileRename (rsComm, &fileRenameInp);

        if (status1 < 0) {
            rodsLog (LOG_ERROR,
             "syncDataObjPhyPath: rollback rename %s failed, status = %d",
             fileRenameInp.oldFileName, status1);
        }
	return (status);
    }
    return (0);
}

int
syncCollPhyPath (rsComm_t *rsComm, char *collection)
{
    int status, i;
    int savedStatus = 0;
    genQueryOut_t *genQueryOut = NULL;
    genQueryInp_t genQueryInp;
    int continueInx;

    status = rsQueryDataObjInCollReCur (rsComm, collection, 
      &genQueryInp, &genQueryOut, NULL, 0);

    if (status<0 && status != CAT_NO_ROWS_FOUND) {
	savedStatus=status; /* return the error code */
    }

    while (status >= 0) {
        sqlResult_t *dataIdRes, *subCollRes, *dataNameRes, *replNumRes, 
	  *rescNameRes, *filePathRes;
	char *tmpDataId, *tmpDataName, *tmpSubColl, *tmpReplNum, 
	  *tmpRescName, *tmpFilePath;
        dataObjInfo_t dataObjInfo;

	memset (&dataObjInfo, 0, sizeof (dataObjInfo));

        if ((dataIdRes = getSqlResultByInx (genQueryOut, COL_D_DATA_ID))
          == NULL) {
            rodsLog (LOG_ERROR,
              "syncCollPhyPath: getSqlResultByInx for COL_COLL_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }
        if ((subCollRes = getSqlResultByInx (genQueryOut, COL_COLL_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "syncCollPhyPath: getSqlResultByInx for COL_COLL_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }
        if ((dataNameRes = getSqlResultByInx (genQueryOut, COL_DATA_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "syncCollPhyPath: getSqlResultByInx for COL_DATA_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }
        if ((replNumRes = getSqlResultByInx (genQueryOut, COL_DATA_REPL_NUM))
          == NULL) {
            rodsLog (LOG_ERROR,
             "syncCollPhyPath:getSqlResultByIn for COL_DATA_REPL_NUM failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }
        if ((rescNameRes = getSqlResultByInx (genQueryOut, COL_D_RESC_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
             "syncCollPhyPath: getSqlResultByInx for COL_D_RESC_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }
        if ((filePathRes = getSqlResultByInx (genQueryOut, COL_D_DATA_PATH))
          == NULL) {
            rodsLog (LOG_ERROR,
             "syncCollPhyPath: getSqlResultByInx for COL_D_DATA_PATH failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }
        for (i = 0;i < genQueryOut->rowCnt; i++) {
            tmpDataId = &dataIdRes->value[dataIdRes->len * i];
            tmpDataName = &dataNameRes->value[dataNameRes->len * i];
            tmpSubColl = &subCollRes->value[subCollRes->len * i];
            tmpReplNum = &replNumRes->value[replNumRes->len * i];
            tmpRescName = &rescNameRes->value[rescNameRes->len * i];
            tmpFilePath = &filePathRes->value[filePathRes->len * i];

	    dataObjInfo.dataId = strtoll (tmpDataId, 0, 0);
	    snprintf (dataObjInfo.objPath, MAX_NAME_LEN, "%s/%s",
	      tmpSubColl, tmpDataName);
	    dataObjInfo.replNum = atoi (tmpReplNum);
            rstrcpy (dataObjInfo.rescName, tmpRescName, NAME_LEN);
            status = resolveResc (tmpRescName, &dataObjInfo.rescInfo);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "syncCollPhyPath: resolveResc error for %s, status = %d",
                  tmpRescName, status);
                return (status);
            }
            rstrcpy (dataObjInfo.filePath, tmpFilePath, MAX_NAME_LEN);

            status = syncDataObjPhyPathS (rsComm, NULL, &dataObjInfo);
	    if (status < 0) {
		rodsLog (LOG_ERROR,
                  "syncCollPhyPath: syncDataObjPhyPathS error for %s,stat=%d",
                  dataObjInfo.filePath, status);
		savedStatus = status;
            }

	}

        continueInx = genQueryOut->continueInx;

        freeGenQueryOut (&genQueryOut);

        if (continueInx > 0) {
            /* More to come */
            genQueryInp.continueInx = continueInx;
            status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
        } else {
            break;
        }
    }
    clearGenQueryInp (&genQueryInp);

    return (savedStatus);
}

int
getMultiCopyPerResc ()
{
    ruleExecInfo_t rei;

    memset (&rei, 0, sizeof (rei));
    applyRule ("acSetMultiReplPerResc", NULL, &rei, NO_SAVE_REI);
    if (strcmp (rei.statusStr, MULTI_COPIES_PER_RESC) == 0) {
        return 1;
    } else {
        return 0;
    }
}

/* mk the collection resursively */

int
rsMkCollR (rsComm_t *rsComm, char *startColl, char *destColl)
{
    int status;
    int startLen;
    int pathLen, tmpLen;
    char tmpPath[MAX_NAME_LEN];

    startLen = strlen (startColl);
    pathLen = strlen (destColl);

    rstrcpy (tmpPath, destColl, MAX_NAME_LEN);

    tmpLen = pathLen;

    while (tmpLen > startLen) {
	if (isColl (rsComm, tmpPath, NULL) >= 0) {
	    break;
	}

        /* Go backward */

        while (tmpLen && tmpPath[tmpLen] != '/')
            tmpLen --;
        tmpPath[tmpLen] = '\0';
    }

    /* Now we go forward and make the required coll */
    while (tmpLen < pathLen) {
	collInp_t collCreateInp;

        /* Put back the '/' */
        tmpPath[tmpLen] = '/';
	memset (&collCreateInp, 0, sizeof (collCreateInp));
	rstrcpy (collCreateInp.collName, tmpPath, MAX_NAME_LEN);
        status = rsCollCreate (rsComm, &collCreateInp);

        if (status < 0) {
            rodsLog (LOG_ERROR,
             "rsMkCollR: rsCollCreate failed for %s, status =%d",
              tmpPath, status);
            return status;
        }
        while (tmpLen && tmpPath[tmpLen] != '\0')
            tmpLen ++;
    }
    return 0;
}

int
isInVault (dataObjInfo_t *dataObjInfo)
{
    int len;

    if (dataObjInfo == NULL || dataObjInfo->rescInfo == NULL) {
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    len = strlen (dataObjInfo->rescInfo->rescVaultPath);

    if (strncmp (dataObjInfo->rescInfo->rescVaultPath, 
      dataObjInfo->filePath, len) == 0) {
        return (1);
    } else {
	return (0);
    }
}

int
initCollHandle ()
{
    memset (CollHandle, 0, sizeof (collHandle_t) * NUM_COLL_HANDLE);
    return (0);
}

int
allocCollHandle ()
{
    int i;

    for (i = 0; i < NUM_COLL_HANDLE; i++) {
        if (CollHandle[i].inuseFlag <= FD_FREE) {
            CollHandle[i].inuseFlag = FD_INUSE;
            return (i);
        };
    }

    rodsLog (LOG_NOTICE,
     "allocCollHandle: out of CollHandle");

    return (SYS_OUT_OF_FILE_DESC);
}

int
freeCollHandle (int handleInx)
{
    if (handleInx < 0 || handleInx >= NUM_COLL_HANDLE) {
        rodsLog (LOG_NOTICE,
         "freeCollHandle: handleInx %d out of range", handleInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }

    /* don't free specColl. It is in cache */
    clearCollHandle (&CollHandle[handleInx], 0);
    memset (&CollHandle[handleInx], 0, sizeof (collHandle_t));

    return (0);
}

int
rsInitQueryHandle (queryHandle_t *queryHandle, rsComm_t *rsComm)
{
    if (queryHandle == NULL || rsComm == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    queryHandle->conn = rsComm;
    queryHandle->connType = RS_COMM;
    queryHandle->querySpecColl = (funcPtr) rsQuerySpecColl;
    queryHandle->genQuery = (funcPtr) rsGenQuery;

    return (0);
}

