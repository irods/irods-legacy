/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See phyPathReg.h for a description of this API call.*/

#include "phyPathReg.h"
#include "rodsLog.h"
#include "icatDefines.h"
#include "fileUnlink.h"
#include "unregDataObj.h"
#include "objMetaOpr.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"
#include "miscServerFunct.h"
#include "regDataObj.h"
#include "fileOpendir.h"
#include "fileReaddir.h"
#include "fileClosedir.h"
#include "fileStat.h"
#include "collCreate.h"
#include "dataObjClose.h"
#include "fileMkdir.h"
#include "modColl.h"
#include "regColl.h"
#include "dataObjCreate.h"

int
rsPhyPathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp)
{
    int status;
    rescGrpInfo_t *rescGrpInfo = NULL;
    rodsServerHost_t *rodsServerHost = NULL;
    int remoteFlag;
    int rescCnt;
    rodsHostAddr_t addr;
    char *tmpStr = NULL;

    if ((tmpStr = getValByKey (&phyPathRegInp->condInput,
      COLLECTION_TYPE_KW)) != NULL && strcmp (tmpStr, UNMOUNT_STR) == 0) {
        status = unmountFileDir (rsComm, phyPathRegInp);
        return (status);
    } else if (tmpStr != NULL && strcmp (tmpStr, HAAW_BUNDLE_STR) == 0) {
	status = bundleReg (rsComm, phyPathRegInp);
        return (status);
    }

    status = getRescInfo (rsComm, NULL, &phyPathRegInp->condInput,
      &rescGrpInfo);

    if (status < 0) {
	rodsLog (LOG_ERROR,
	  "rsPhyPathReg: getRescInfo error for %s, status = %d",
	  phyPathRegInp->objPath, status);
	return (status);
    }

    rescCnt = getRescCnt (rescGrpInfo);

    if (rescCnt != 1) {
        rodsLog (LOG_ERROR,
          "rsPhyPathReg: The input resource is not unique for %s",
          phyPathRegInp->objPath);
        return (SYS_INVALID_RESC_TYPE);
    }

    memset (&addr, 0, sizeof (addr));
    
    rstrcpy (addr.hostAddr, rescGrpInfo->rescInfo->rescLoc, LONG_NAME_LEN);
    remoteFlag = resolveHost (&addr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsPhyPathReg (rsComm, phyPathRegInp, rescGrpInfo, 
	  rodsServerHost);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remotePhyPathReg (rsComm, phyPathRegInp, rodsServerHost);
    } else {
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else {
            rodsLog (LOG_ERROR,
              "rsPhyPathReg: resolveHost returned unrecognized value %d",
               remoteFlag);
            return (SYS_UNRECOGNIZED_REMOTE_FLAG);
        }
    }

    return (status);
}

int
remotePhyPathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, 
rodsServerHost_t *rodsServerHost)
{
   int status;

   if (rodsServerHost == NULL) {
        rodsLog (LOG_ERROR,
          "remotePhyPathReg: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcPhyPathReg (rodsServerHost->conn, phyPathRegInp);

    if (status < 0) {
        rodsLog (LOG_ERROR,
         "remotePhyPathReg: rcPhyPathReg failed for %s",
          phyPathRegInp->objPath);
    }

    return (status);
}

int
_rsPhyPathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, 
rescGrpInfo_t *rescGrpInfo, rodsServerHost_t *rodsServerHost)
{
    int status;
    fileOpenInp_t chkNVPathPermInp;
    int rescTypeInx;
    char *tmpFilePath;
    char filePath[MAX_NAME_LEN];
    dataObjInfo_t dataObjInfo;
    char *tmpStr = NULL;

    if ((tmpFilePath = getValByKey (&phyPathRegInp->condInput, FILE_PATH_KW))
      == NULL) {
        rodsLog (LOG_ERROR,
	  "_rsPhyPathReg: No filePath input for %s",
	  phyPathRegInp->objPath);
	return (SYS_INVALID_FILE_PATH);
    } else {
	/* have to do this since it will be over written later */
	rstrcpy (filePath, tmpFilePath, MAX_NAME_LEN);
    }

    /* check if we need to chk permission */

    memset (&dataObjInfo, 0, sizeof (dataObjInfo));
    rstrcpy (dataObjInfo.objPath, phyPathRegInp->objPath, MAX_NAME_LEN);
    rstrcpy (dataObjInfo.filePath, filePath, MAX_NAME_LEN);
    dataObjInfo.rescInfo = rescGrpInfo->rescInfo;
    rstrcpy (dataObjInfo.rescName, rescGrpInfo->rescInfo->rescName, 
      LONG_NAME_LEN);

 
    if (getchkPathPerm (rsComm, phyPathRegInp, &dataObjInfo)) { 
        memset (&chkNVPathPermInp, 0, sizeof (chkNVPathPermInp));

        rescTypeInx = rescGrpInfo->rescInfo->rescTypeInx;
        rstrcpy (chkNVPathPermInp.fileName, filePath, MAX_NAME_LEN);
        chkNVPathPermInp.fileType = RescTypeDef[rescTypeInx].driverType;
        rstrcpy (chkNVPathPermInp.addr.hostAddr,  
	  rescGrpInfo->rescInfo->rescLoc, NAME_LEN);

        status = chkFilePathPerm (rsComm, &chkNVPathPermInp, rodsServerHost);
    
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "_rsPhyPathReg: chkFilePathPerm error for %s",
              phyPathRegInp->objPath);
            return (SYS_INVALID_FILE_PATH);
        }
    } else {
	status = 0;
    }

    if (getValByKey (&phyPathRegInp->condInput, COLLECTION_KW) != NULL) {
	status = dirPathReg (rsComm, phyPathRegInp, filePath, 
	  rescGrpInfo->rescInfo); 
    } else if ((tmpStr = getValByKey (&phyPathRegInp->condInput, 
      COLLECTION_TYPE_KW)) != NULL && strcmp (tmpStr, MOUNT_POINT_STR) == 0) {
        status = mountFileDir (rsComm, phyPathRegInp, filePath,
          rescGrpInfo->rescInfo);
    } else {
	status = filePathReg (rsComm, phyPathRegInp, filePath,
	  rescGrpInfo->rescInfo); 
    }

    return (status);
}

int
filePathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, char *filePath, 
rescInfo_t *rescInfo)
{
    dataObjInfo_t dataObjInfo;
    int status;

    initDataObjInfoWithInp (&dataObjInfo, phyPathRegInp);
    dataObjInfo.replStatus = NEWLY_CREATED_COPY;
    dataObjInfo.rescInfo = rescInfo;
    rstrcpy (dataObjInfo.rescName, rescInfo->rescName, NAME_LEN);

    if (dataObjInfo.dataSize <= 0 && 
      (dataObjInfo.dataSize = getSizeInVault (rsComm, &dataObjInfo)) < 0) {
	status = (int) dataObjInfo.dataSize;
        rodsLog (LOG_ERROR,
         "filePathReg: getSizeInVault for %s failed, status = %d",
          dataObjInfo.objPath, status);
	return (status);
    }

    status = svrRegDataObj (rsComm, &dataObjInfo);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "filePathReg: rsRegDataObj for %s failed, status = %d",
          dataObjInfo.objPath, status);
    }
    return (status);
}

int
dirPathReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, char *filePath,
rescInfo_t *rescInfo)
{
    collInp_t collCreateInp;
    fileOpendirInp_t fileOpendirInp;
    fileClosedirInp_t fileClosedirInp;
    int rescTypeInx;
    int status;
    int dirFd;
    dataObjInp_t subPhyPathRegInp;
    fileReaddirInp_t fileReaddirInp;
    rodsDirent_t *rodsDirent = NULL;

    memset (&collCreateInp, 0, sizeof (collCreateInp));
    rstrcpy (collCreateInp.collName, phyPathRegInp->objPath, MAX_NAME_LEN);
    /* create the coll just in case it does not exist */
    rsCollCreate (rsComm, &collCreateInp);

    memset (&fileOpendirInp, 0, sizeof (fileOpendirInp));

    rescTypeInx = rescInfo->rescTypeInx;
    rstrcpy (fileOpendirInp.dirName, filePath, MAX_NAME_LEN);
    fileOpendirInp.fileType = RescTypeDef[rescTypeInx].driverType;
    rstrcpy (fileOpendirInp.addr.hostAddr,  rescInfo->rescLoc, NAME_LEN);

    dirFd = rsFileOpendir (rsComm, &fileOpendirInp);
    if (dirFd < 0) {
       rodsLog (LOG_ERROR,
          "dirPathReg: rsFileOpendir for %s error, status = %d",
          filePath, dirFd);
        return (dirFd);
    }

    fileReaddirInp.fileInx = dirFd;

    while ((status = rsFileReaddir (rsComm, &fileReaddirInp, &rodsDirent))
      >= 0) {
        fileStatInp_t fileStatInp;
	rodsStat_t *myStat = NULL;

        if (strcmp (rodsDirent->d_name, ".") == 0 ||
          strcmp (rodsDirent->d_name, "..") == 0) {
            continue;
        }

	memset (&fileStatInp, 0, sizeof (fileStatInp));

        snprintf (fileStatInp.fileName, MAX_NAME_LEN, "%s/%s",
          filePath, rodsDirent->d_name);

        fileStatInp.fileType = fileOpendirInp.fileType; 
	fileStatInp.addr = fileOpendirInp.addr;
        status = rsFileStat (rsComm, &fileStatInp, &myStat);

	if (status != 0) {
            rodsLog (LOG_ERROR,
	      "dirPathReg: rsFileStat failed for %s, status = %d",
	      fileStatInp.fileName, status);
	    return (status);
	}

	subPhyPathRegInp = *phyPathRegInp;
	snprintf (subPhyPathRegInp.objPath, MAX_NAME_LEN, "%s/%s",
	  phyPathRegInp->objPath, rodsDirent->d_name);

	if ((myStat->st_mode & S_IFREG) != 0) {     /* a file */
            addKeyVal (&subPhyPathRegInp.condInput, FILE_PATH_KW, 
	      fileStatInp.fileName);
	    subPhyPathRegInp.dataSize = myStat->st_size;
	    status = filePathReg (rsComm, &subPhyPathRegInp, 
	      fileStatInp.fileName, rescInfo);
        } else if ((myStat->st_mode & S_IFDIR) != 0) {      /* a directory */
            status = dirPathReg (rsComm, &subPhyPathRegInp,
              fileStatInp.fileName, rescInfo);
	}
	free (myStat);
    }
    if (status == -1) {         /* just EOF */
        status = 0;
    }

    fileClosedirInp.fileInx = dirFd;
    rsFileClosedir (rsComm, &fileClosedirInp);

    return (status);
}

int
mountFileDir (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp, char *filePath,
rescInfo_t *rescInfo)
{
    collInp_t collCreateInp;
    int rescTypeInx;
    int status;
    fileStatInp_t fileStatInp;
    rodsStat_t *myStat = NULL;


    memset (&fileStatInp, 0, sizeof (fileStatInp));

    rstrcpy (fileStatInp.fileName, filePath, MAX_NAME_LEN);

    rescTypeInx = rescInfo->rescTypeInx;
    fileStatInp.fileType = RescTypeDef[rescTypeInx].driverType;
    rstrcpy (fileStatInp.addr.hostAddr,  rescInfo->rescLoc, NAME_LEN);
    status = rsFileStat (rsComm, &fileStatInp, &myStat);

    if (status < 0) {
	fileMkdirInp_t fileMkdirInp;

        rodsLog (LOG_NOTICE,
          "mountFileDir: rsFileStat failed for %s, status = %d, create it",
          fileStatInp.fileName, status);
	memset (&fileMkdirInp, 0, sizeof (fileMkdirInp));
	rstrcpy (fileMkdirInp.dirName, filePath, MAX_NAME_LEN);
        fileMkdirInp.fileType = RescTypeDef[rescTypeInx].driverType;
	fileMkdirInp.mode = DEFAULT_DIR_MODE;
        rstrcpy (fileMkdirInp.addr.hostAddr,  rescInfo->rescLoc, NAME_LEN);
	status = rsFileMkdir (rsComm, &fileMkdirInp);
	if (status < 0) {
            return (status);
	}
    } else if ((myStat->st_mode & S_IFDIR) == 0) {
        rodsLog (LOG_ERROR,
          "mountFileDir: phyPath %s is not a directory",
          fileStatInp.fileName);
	free (myStat);
        return (USER_FILE_DOES_NOT_EXIST);
    }

    free (myStat);
    /* mk the collection */

    memset (&collCreateInp, 0, sizeof (collCreateInp));
    rstrcpy (collCreateInp.collName, phyPathRegInp->objPath, MAX_NAME_LEN);
    addKeyVal (&collCreateInp.condInput, COLLECTION_TYPE_KW, MOUNT_POINT_STR);

    addKeyVal (&collCreateInp.condInput, COLLECTION_INFO1_KW, filePath);
    addKeyVal (&collCreateInp.condInput, COLLECTION_INFO2_KW, 
      rescInfo->rescName);

    /* try to mod the coll first */
    status = rsModColl (rsComm, &collCreateInp);

    if (status < 0) {	/* try to create it */
       status = rsRegColl (rsComm, &collCreateInp);
    }

    return (status);
}

int
unmountFileDir (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp)
{
    int status;
#if 0
    collInfo_t collInfo;

    memset (&collInfo, 0, sizeof (collInfo));

    rstrcpy (collInfo.collName, phyPathRegInp->objPath, MAX_NAME_LEN);

    status = chlModColl (rsComm, &collInfo);
#endif
    collInp_t modCollInp;

    memset (&modCollInp, 0, sizeof (modCollInp));
    rstrcpy (modCollInp.collName, phyPathRegInp->objPath, MAX_NAME_LEN);
    addKeyVal (&modCollInp.condInput, COLLECTION_TYPE_KW, 
      "NULL_SPECIAL_VALUE");
    addKeyVal (&modCollInp.condInput, COLLECTION_INFO1_KW, "");
    addKeyVal (&modCollInp.condInput, COLLECTION_INFO2_KW, "");

    status = rsModColl (rsComm, &modCollInp);

    return (status);
}

int
bundleReg (rsComm_t *rsComm, dataObjInp_t *phyPathRegInp)
{
    collInp_t collCreateInp;
    int status;
    dataObjInfo_t *dataObjInfo = NULL;
    char *bundlePath = NULL;
    dataObjInp_t dataObjInp;

    if ((bundlePath = getValByKey (&phyPathRegInp->condInput, FILE_PATH_KW))
      == NULL) {
        rodsLog (LOG_ERROR,
          "bundleReg: No bundlePath input for %s",
          phyPathRegInp->objPath);
        return (SYS_INVALID_FILE_PATH);
    }

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, bundlePath, sizeof (dataObjInp));
    /* user need to have write permission */
    dataObjInp.openFlags = O_WRONLY;
    status = getDataObjInfoIncSpecColl (rsComm, &dataObjInp, &dataObjInfo);
    if (status < 0) {
	int myStatus;
	/* try to make one */
	dataObjInp.condInput = phyPathRegInp->condInput;
	/* have to remove FILE_PATH_KW because getFullPathName will use it */
	rmKeyVal (&dataObjInp.condInput, FILE_PATH_KW);
	myStatus = rsDataObjCreate (rsComm, &dataObjInp);
	if (myStatus < 0) {
            rodsLog (LOG_ERROR,
              "bundleReg: Problem with open/create bundlePath %s, status = %d",
              dataObjInp.objPath, status);
            return (status);
	} else {
	    dataObjCloseInp_t dataObjCloseInp;
	    dataObjCloseInp.l1descInx = myStatus;
	    rsDataObjClose (rsComm, &dataObjCloseInp);
	}
    }

    /* mk the collection */

    memset (&collCreateInp, 0, sizeof (collCreateInp));
    rstrcpy (collCreateInp.collName, phyPathRegInp->objPath, MAX_NAME_LEN);
    addKeyVal (&collCreateInp.condInput, COLLECTION_TYPE_KW, HAAW_BUNDLE_STR);

    /* have to use dataObjInp.objPath because bundle path was removed */ 
    addKeyVal (&collCreateInp.condInput, COLLECTION_INFO1_KW, 
     dataObjInp.objPath);

    /* try to mod the coll first */
    status = rsModColl (rsComm, &collCreateInp);

    if (status < 0) {	/* try to create it */
       status = rsRegColl (rsComm, &collCreateInp);
    }

    return (status);
}

