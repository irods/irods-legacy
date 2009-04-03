/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsPhyBundleColl.c. See phyBundleColl.h for a description of
 * this API call.*/

#include "phyBundleColl.h"
#include "objMetaOpr.h"
#include "miscServerFunct.h"
#include "openCollection.h"
#include "readCollection.h"
#include "closeCollection.h"
#include "dataObjRepl.h"
#include "dataObjCreate.h"

int
rsPhyBundleColl (rsComm_t *rsComm, structFileExtAndRegInp_t *phyBundleCollInp)
{
    char *destRescName;
    int status; 
    rodsServerHost_t *rodsServerHost;
    int remoteFlag;
    rodsHostAddr_t rescAddr;
    rescGrpInfo_t *rescGrpInfo = NULL;

    if ((destRescName = getValByKey (&phyBundleCollInp->condInput, 
      DEST_RESC_NAME_KW)) == NULL) {
	return USER_NO_RESC_INPUT_ERR;
    }

    if (isLocalZone (phyBundleCollInp->collection) == 0) { 
	/* can only do local zone */
	return SYS_INVALID_ZONE_NAME;
    }
    status = _getRescInfo (rsComm, destRescName, &rescGrpInfo);
    if (status < 0) {
	 rodsLog (LOG_ERROR,
          "rsPhyBundleColl: _getRescInfo of %s error for %s. stat = %d",
          destRescName, phyBundleCollInp->collection, status);
	return status;
    }

    bzero (&rescAddr, sizeof (rescAddr));
    rstrcpy (rescAddr.hostAddr, rescGrpInfo->rescInfo->rescLoc, NAME_LEN);
    remoteFlag = resolveHost (&rescAddr, &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
        status = _rsPhyBundleColl (rsComm, phyBundleCollInp, rescGrpInfo);
    } else if (remoteFlag == REMOTE_HOST) {
        status = remotePhyBundleColl (rsComm, phyBundleCollInp, rodsServerHost);
    } else if (remoteFlag < 0) {
	    status = remoteFlag;
    }

    return status;
}

int
_rsPhyBundleColl (rsComm_t *rsComm, structFileExtAndRegInp_t *phyBundleCollInp,
rescGrpInfo_t *rescGrpInfo)
{
    rescInfo_t *myRescInfo;
    char *myRescName;
    collInp_t collInp;
    collEnt_t *collEnt;
    char objPath[MAX_NAME_LEN];
    char collName[MAX_NAME_LEN];
    char dataName[MAX_NAME_LEN];
    char phyBunDir[MAX_NAME_LEN];
    char phyBunSubPath[MAX_NAME_LEN];
    int handleInx;
    int status, l1descInx;
    dataObjInp_t dataObjInp;
    dataObjInfo_t dataObjInfo;

    myRescInfo = rescGrpInfo->rescInfo;
    myRescName = myRescInfo->rescName;
    bzero (&collInp, sizeof (collInp));
    rstrcpy (collInp.collName, phyBundleCollInp->collection, MAX_NAME_LEN);
    collInp.flags = RECUR_QUERY_FG | VERY_LONG_METADATA_FG;
    handleInx = rsOpenCollection (rsComm, &collInp);
    if (handleInx < 0) {
        rodsLog (LOG_ERROR,
          "_rsPhyBundleColl: rsOpenCollection of %s error. status = %d",
          collInp.collName, handleInx);
        return (handleInx);
    }

    if (CollHandle[handleInx].rodsObjStat->specColl != NULL) {
        rodsLog (LOG_ERROR,
          "_rsPhyBundleColl: unable to bundle mounted collection %s",
          collInp.collName);
        rsCloseCollection (rsComm, &handleInx);
        return (0);
    }

    /* greate the bundle file */ 
    l1descInx = createPhyBundleDataObj (rsComm, phyBundleCollInp->collection,
      rescGrpInfo, &dataObjInp);

    if (l1descInx < 0) return l1descInx;

    createPhyBundleDir (rsComm, L1desc[l1descInx].dataObjInfo, phyBunDir);

    objPath[0] = '\0';
    collName[0] = '\0';
    dataName[0] = '\0';
    phyBunSubPath[0] = '\0';
    while ((status = rsReadCollection (rsComm, &handleInx, &collEnt)) >= 0) {
	if (collEnt->objType == DATA_OBJ_T) {
	    if (collName[0] == '\0') {
		/* a new dataObj.  */
		rstrcpy (collName, collEnt->collName, MAX_NAME_LEN);
		rstrcpy (dataName, collEnt->dataName, MAX_NAME_LEN);
	    } else if (strcmp (collName, collEnt->collName) != 0 ||
	      strcmp (collName, collEnt->collName) != 0) {
		/* next dataObj. See if we need to replicate */
		if (phyBunSubPath[0] == '\0') {
		    /* don't have a good copy yet */
		    status = replDataObjForBundle (rsComm, collName, dataName,
		      myRescName, &dataObjInfo);
		}
	    }
 
#if 0
	    if (isDataObjBundled (&collEnt)) {
		/* already bundled, skip */
		objPath[0] = '\0';
	
	    if (objPath[0] == '\0') {
		snprintf (objPath, MAX_NAME_LEN, "%s/%s",
		  collEnt.collName, collEnt.dataName); 
#endif
	}
    }

    return status;
}

int
replDataObjForBundle (rsComm_t *rsComm, char *collName, char *dataName,
char *rescName, dataObjInfo_t *outCacheObjInfo)
{
    transStat_t transStat;
    dataObjInp_t dataObjInp;
    int status;

    if (outCacheObjInfo != NULL)
        memset (outCacheObjInfo, 0, sizeof (dataObjInfo_t));
    memset (&dataObjInp, 0, sizeof (dataObjInp_t));
    memset (&transStat, 0, sizeof (transStat));

    snprintf (dataObjInp.objPath, MAX_NAME_LEN, "%s/%s", collName, dataName);
    addKeyVal (&dataObjInp.condInput, BACKUP_RESC_NAME_KW, rescName);
    addKeyVal (&dataObjInp.condInput, IRODS_ADMIN_KW, "");

    status = rsDataObjReplWithOutDataObj (rsComm, &dataObjInp, &transStat,
      outCacheObjInfo);
    clearKeyVal (&dataObjInp.condInput);
    return status;
}

int
createPhyBundleDir (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo, 
char *outPhyBundleDir)
{
    /* the dir where we put the files to bundle is in phyPath.dir */
    snprintf (outPhyBundleDir, MAX_NAME_LEN, "%s.dir",  dataObjInfo->filePath);
    mkdirR ("/", outPhyBundleDir, 0750);
    return (0);
}

int
createPhyBundleDataObj (rsComm_t *rsComm, char *collection, 
rescGrpInfo_t *rescGrpInfo, dataObjInp_t *dataObjInp)
{
    int myRanNum;
    int l1descInx;
    int status;
    int rescTypeInx = rescGrpInfo->rescInfo->rescTypeInx;

    /* XXXXXX We do bundle only with UNIX_FILE_TYPE for now */
    if (RescTypeDef[rescTypeInx].driverType != UNIX_FILE_TYPE) {
        rodsLog (LOG_ERROR,
          "createPhyBundleFile: resource %s is not UNIX_FILE_TYPE",
          rescGrpInfo->rescInfo->rescName);
        return SYS_INVALID_RESC_TYPE;
    }
	
    bzero (dataObjInp, sizeof (dataObjInp_t));
    myRanNum = random ();
    status = rsMkBundlePath (rsComm, collection, dataObjInp->objPath, myRanNum);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "createPhyBundleFile: getPhyBundlePath error for %s. status = %d",
          collection, status);
        return status;
    }

    addKeyVal (&dataObjInp->condInput, NO_OPEN_FLAG_KW, "");
    addKeyVal (&dataObjInp->condInput, DATA_TYPE_KW, "tar bundle");

    l1descInx = _rsDataObjCreateWithRescInfo (rsComm, dataObjInp,
      rescGrpInfo->rescInfo, rescGrpInfo->rescGroupName);

    return l1descInx;
}

/* rsMkBundlePath - set the BundlePath to 
 * /zone/bundle/home/.../collection.myRanNum. Make all the necessary 
 * parent collections. The output is put in bundlePath.
 */

int
rsMkBundlePath (rsComm_t *rsComm, char *collection, char *bundlePath, 
int myRanNum)
{
    int status;
    char *tmpStr;
    char startBundlePath[MAX_NAME_LEN];
    char destBundleColl[MAX_NAME_LEN], myFile[MAX_NAME_LEN];
    char *bundlePathPtr;

    bundlePathPtr = bundlePath;
    *bundlePathPtr = '/';
    bundlePathPtr++;
    tmpStr = collection + 1;
    /* copy the zone */
    while (*tmpStr != '\0') {
        *bundlePathPtr = *tmpStr;
        bundlePathPtr ++;
        if (*tmpStr == '/') {
            tmpStr ++;
            break;
        }
        tmpStr ++;
    }

    if (*tmpStr == '\0') {
        rodsLog (LOG_ERROR,
          "rsMkBundlePath: input path %s too short", collection);
        return (USER_INPUT_PATH_ERR);
    }

    /* cannot bundle trash and bundle */
    if (strncmp (tmpStr, "trash/", 6) == 0 || 
      strncmp (tmpStr, "bundle/", 7) == 0) {
        rodsLog (LOG_ERROR,
          "rsMkBundlePath: cannot bundle trash or bundle path %s", collection);
        return (USER_INPUT_PATH_ERR);
    }


    /* don't want to go back beyond /myZone/bundle/home */
    *bundlePathPtr = '\0';
    rstrcpy (startBundlePath, bundlePath, MAX_NAME_LEN);

    snprintf (bundlePathPtr, MAX_NAME_LEN, "bundle/%s.%d", tmpStr, myRanNum);

    if ((status = splitPathByKey (bundlePath, destBundleColl, myFile, '/')) 
      < 0) {
        rodsLog (LOG_ERROR,
          "rsMkBundlePath: splitPathByKey error for %s ", bundlePath);
        return (USER_INPUT_PATH_ERR);
    }

    status = rsMkCollR (rsComm, startBundlePath, destBundleColl);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "rsMkBundlePath: rsMkCollR error for startPath %s, destPath %s ",
          startBundlePath, destBundleColl);
    }

    return (status);
}

int
remotePhyBundleColl (rsComm_t *rsComm, 
structFileExtAndRegInp_t *phyBundleCollInp, rodsServerHost_t *rodsServerHost)
{
    int status;

    if (rodsServerHost == NULL) {
        rodsLog (LOG_NOTICE,
          "remotePhyBundleColl: Invalid rodsServerHost");
        return SYS_INVALID_SERVER_HOST;
    }

    if ((status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
        return status;
    }

    status = rcPhyBundleColl (rodsServerHost->conn, phyBundleCollInp);
    return status;
}

