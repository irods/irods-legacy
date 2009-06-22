/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsDataObjRename.c - rename a data object.
 */
#include "dataObjRename.h"
#include "objMetaOpr.h"
#include "subStructFileRename.h"
#include "icatHighLevelRoutines.h"
#include "dataObjUnlink.h"
#include "phyBundleColl.h"

int
rsDataObjRename (rsComm_t *rsComm, dataObjCopyInp_t *dataObjRenameInp)
{
    int status;
    dataObjInp_t *srcDataObjInp, *destDataObjInp;
    rodsServerHost_t *rodsServerHost = NULL;
    dataObjInfo_t *srcDataObjInfo = NULL;
    dataObjInfo_t *destDataObjInfo = NULL;
    int srcType, destType;

    srcDataObjInp = &dataObjRenameInp->srcDataObjInp;
    destDataObjInp = &dataObjRenameInp->destDataObjInp;

    /* connect to rcat for cross zone */
    status = getAndConnRcatHost (rsComm, MASTER_RCAT, 
     srcDataObjInp->objPath, &rodsServerHost);

    if (status < 0) {
        return (status);
    } else if (rodsServerHost->rcatEnabled == REMOTE_ICAT) {
        status = rcDataObjRename (rodsServerHost->conn, dataObjRenameInp);
        return status;
    }

    if (strcmp (srcDataObjInp->objPath, destDataObjInp->objPath) == 0) {
        return (SAME_SRC_DEST_PATHS_ERR);
    }

    srcType = resolveSpecColl (rsComm, srcDataObjInp, 
      &srcDataObjInfo, 1);

    destType = resolveSpecColl (rsComm, destDataObjInp,
      &destDataObjInfo, 1);

    if (destType >= 0) {
        rodsLog (LOG_ERROR,
          "rsDataObjRename: dest objPath %s exists",
          destDataObjInp->objPath);
	freeDataObjInfo (srcDataObjInfo);
	freeDataObjInfo (destDataObjInfo);
        return (SYS_DEST_SPEC_COLL_SUB_EXIST);
    }

    if (srcType >= 0) {	/*specColl of some sort */
	if (destType != SYS_SPEC_COLL_OBJ_NOT_EXIST || destDataObjInfo == NULL 
          || destDataObjInfo->specColl == NULL) {
            rodsLog (LOG_ERROR,
              "rsDataObjRename: src %s is in spec coll but dest %s is not",
	      srcDataObjInp->objPath, destDataObjInp->objPath);
	    return (SYS_SRC_DEST_SPEC_COLL_CONFLICT);
	}
	status = specCollObjRename (rsComm, srcDataObjInfo, destDataObjInfo);
	freeDataObjInfo (srcDataObjInfo);
	freeDataObjInfo (destDataObjInfo);
	return (status);
    } else if (srcType == SYS_SPEC_COLL_OBJ_NOT_EXIST) {
        /* for STRUCT_FILE_COLL to make a directory in the structFile, the
         * STRUCT_FILE_OPR_KW must be set */
        if (getSpecCollOpr (&srcDataObjInp->condInput, 
	  srcDataObjInfo->specColl) != NORMAL_OPR_ON_STRUCT_FILE_COLL) {
	    return (SYS_SPEC_COLL_OBJ_NOT_EXIST);
	}
    } else if (destType == SYS_SPEC_COLL_OBJ_NOT_EXIST) {
        rodsLog (LOG_ERROR,
          "rsDataObjRename: src %s is not in spec coll but dest %s is",
          srcDataObjInp->objPath, destDataObjInp->objPath);
        return (SYS_SRC_DEST_SPEC_COLL_CONFLICT);
    }

    status = getAndConnRcatHost (rsComm, MASTER_RCAT, 
      dataObjRenameInp->srcDataObjInp.objPath,
      &rodsServerHost);
    if (status < 0) {
       return(status);
    }
    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
        status = _rsDataObjRename (rsComm, dataObjRenameInp);
#else
        status = SYS_NO_RCAT_SERVER_ERR;
#endif
    } else {
        status = rcDataObjRename (rodsServerHost->conn, dataObjRenameInp);
    }

    return (status);
}

int
_rsDataObjRename (rsComm_t *rsComm, dataObjCopyInp_t *dataObjRenameInp)
{
#ifdef RODS_CAT
    int status;
    char srcColl[MAX_NAME_LEN], srcObj[MAX_NAME_LEN];
    char destColl[MAX_NAME_LEN], destObj[MAX_NAME_LEN];
    dataObjInp_t *srcDataObjInp, *destDataObjInp;
    dataObjInfo_t *dataObjInfoHead = NULL;
    rodsLong_t srcId, destId;
    int multiCopyFlag;
    int renameFlag = 0;

    char *args[MAX_NUM_OF_ARGS_IN_ACTION];
    int i, argc;
    ruleExecInfo_t rei2;

    memset ((char*)&rei2, 0, sizeof (ruleExecInfo_t));
    rei2.rsComm = rsComm;
    if (rsComm != NULL) {
      rei2.uoic = &rsComm->clientUser;
      rei2.uoip = &rsComm->proxyUser;
    }

    srcDataObjInp = &dataObjRenameInp->srcDataObjInp;
    destDataObjInp = &dataObjRenameInp->destDataObjInp;

    if ((status = splitPathByKey (
      srcDataObjInp->objPath, srcColl, srcObj, '/')) < 0) {
        rodsLog (LOG_ERROR,
          "_rsDataObjRename: splitPathByKey for %s error, status = %d",
          srcDataObjInp->objPath, status);
        return (status);
    }

    if ((status = splitPathByKey (
      destDataObjInp->objPath, destColl, destObj, '/')) < 0) {
        rodsLog (LOG_ERROR,
          "_rsDataObjRename: splitPathByKey for %s error, status = %d",
          destDataObjInp->objPath, status);
        return (status);
    }

    multiCopyFlag = getMultiCopyPerResc ();

     if (srcDataObjInp->oprType == RENAME_DATA_OBJ) {
	status = getDataObjInfo (rsComm, srcDataObjInp, &dataObjInfoHead,
          ACCESS_DELETE_OBJECT, 0);
	if (status >= 0) {
	    srcId = dataObjInfoHead->dataId;
        } else {
            rodsLog (LOG_ERROR,
              "_rsDataObjRename: src data %s does not exist, status = %d",
              srcDataObjInp->objPath, status);
            return (status);
        }
    } else if (srcDataObjInp->oprType == RENAME_COLL) {
        status = isColl (rsComm, srcDataObjInp->objPath, &srcId);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "_rsDataObjRename: src coll %s does not exist, status = %d",
              srcDataObjInp->objPath, status);
            return (status);
        }
    } else {
	if ((status = isData (rsComm, srcDataObjInp->objPath, &srcId)) >= 0) {
	    if (isData (rsComm, destDataObjInp->objPath, &destId) >= 0 &&
	      getValByKey (&srcDataObjInp->condInput, FORCE_FLAG_KW) != NULL) {
		/* dest exist */
		rsDataObjUnlink (rsComm, destDataObjInp);
	    }	
	    srcDataObjInp->oprType = destDataObjInp->oprType = RENAME_DATA_OBJ;
            status = getDataObjInfo (rsComm, srcDataObjInp, &dataObjInfoHead,
              ACCESS_DELETE_OBJECT, 0);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "_rsDataObjRename: src data %s does not exist, status = %d",
                  srcDataObjInp->objPath, status);
                return (status);
            }
	} else if ((status = isColl (rsComm, srcDataObjInp->objPath, &srcId))
	 >= 0) {
	    srcDataObjInp->oprType = destDataObjInp->oprType = RENAME_COLL;
	} else {
            rodsLog (LOG_ERROR,
               "_rsDataObjRename: src obj %s does not exist, status = %d",
              srcDataObjInp->objPath, status);
            return (status);
	}
    }

    if (srcDataObjInp->oprType == RENAME_DATA_OBJ) {
	if (strcmp (dataObjInfoHead->dataType, TAR_BUNDLE_TYPE) == 0) {
            rodsLog (LOG_ERROR,
              "_rsDataObjRename: cannot rename tar bundle type obj %s",
              srcDataObjInp->objPath);
	    return CANT_RM_MV_BUNDLE_TYPE;
	}
    }

    if (strcmp (srcObj, destObj) != 0) {
	/* rename */
	if (srcId < 0) {
	    status = srcId;
	    return (status);
	}
	renameFlag = 1;

	/** RAJA ADDED June 1 2009 for pre-post processing rule hooks **/
        args[0] = srcDataObjInp->objPath; 
        args[1] = destDataObjInp->objPath;
        argc = 2;
	i =  applyRuleArg("acPreProcForObjRename",args,argc, &rei2, NO_SAVE_REI);
	if (i < 0) {
	  if (rei2.status < 0) {
	    i = rei2.status;
	  }
	  rodsLog (LOG_ERROR,
		   "rsDataObjRename: acPreProcForObjRename error for source %s and destination %s,stat=%d",
		   args[0], args[1], i);
	  return i;
	}
	/** RAJA ADDED June 1 2009 for pre-post processing rule hooks **/

	status = chlRenameObject (rsComm, srcId, destObj);

        /** RAJA ADDED June 1 2009 for pre-post processing rule hooks **/

        i =  applyRuleArg("acPostProcForObjRename",args,argc, &rei2, NO_SAVE_REI);
        if (i < 0) {
          if (rei2.status < 0) {
            i = rei2.status;
          }
          rodsLog (LOG_ERROR,
                   "rsDataObjRename: acPostProcForObjRename error for source %s and destination %s,stat=%d",
                   args[0], args[1], i);
          return i;
        }
        /** RAJA ADDED June 1 2009 for pre-post processing rule hooks **/

    }

    if (status < 0) {
	return (status);
    }

    if (strcmp (srcColl, destColl) != 0) {
	/* move. The destColl is the target  */
	status = isColl (rsComm, destColl, &destId);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "_rsDataObjRename: dest coll %s does not exist, status = %d",
              destColl, status);
            return (status);
        }

        /** RAJA ADDED June 1 2009 for pre-post processing rule hooks **/
        args[0] = srcDataObjInp->objPath;
        args[1] = destDataObjInp->objPath;
        argc = 2;
        i =  applyRuleArg("acPreProcForObjRename",args,argc, &rei2, NO_SAVE_REI);
        if (i < 0) {
          if (rei2.status < 0) {
            i = rei2.status;
          }
          rodsLog (LOG_ERROR,
                   "rsDataObjRename: acPreProcForObjRename error for source %s and destination %s,stat=%d",
                   args[0], args[1], i);
          return i;
        }
        /** RAJA ADDED June 1 2009 for pre-post processing rule hooks **/

	status = chlMoveObject (rsComm, srcId, destId);

        /** RAJA ADDED June 1 2009 for pre-post processing rule hooks **/

        i =  applyRuleArg("acPostProcForObjRename",args,argc, &rei2, NO_SAVE_REI);
        if (i < 0) {
          if (rei2.status < 0) {
            i = rei2.status;
          }
          rodsLog (LOG_ERROR,
                   "rsDataObjRename: acPostProcForObjRename error for source %s and destination %s,stat=%d",
                   args[0], args[1], i);
          return i;
        }
        /** RAJA ADDED June 1 2009 for pre-post processing rule hooks **/

    }

    if (status >= 0) {
        if (multiCopyFlag > 0) {
    	    status = chlCommit(rsComm);
	    return (status);
	} else {
	    /* enforce physPath consistency */
	    if (srcDataObjInp->oprType == RENAME_DATA_OBJ) {
	        dataObjInfo_t *tmpDataObjInfo;

	        /* update src dataObjInfoHead with dest objPath */
	        tmpDataObjInfo = dataObjInfoHead;
	        while (tmpDataObjInfo != NULL) {
		    rstrcpy (tmpDataObjInfo->objPath, destDataObjInp->objPath,
		      MAX_NAME_LEN);
		    tmpDataObjInfo = tmpDataObjInfo->next;
	        }
	        status = syncDataObjPhyPath (rsComm, destDataObjInp,
		  dataObjInfoHead);
		freeAllDataObjInfo (dataObjInfoHead);
	    } else {
                status = syncCollPhyPath (rsComm, destDataObjInp->objPath);
	    }
	}
    } else {
        return (status);
    }
    if (status==0) status = chlCommit(rsComm);
    return(status);
#else
    return SYS_NO_ICAT_SERVER_ERR;
#endif
}

int
specCollObjRename (rsComm_t *rsComm, dataObjInfo_t *srcDataObjInfo,
dataObjInfo_t *destDataObjInfo)
{
    int status;
    char *newPath;

    if (getStructFileType (srcDataObjInfo->specColl) >= 0) {
	/* structFile type, use the logical subPath */
	newPath = destDataObjInfo->subPath;
    } else {
	/* mounted fs, use phy path */
	newPath = destDataObjInfo->filePath;
    }

    status = l3Rename (rsComm, srcDataObjInfo, newPath);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "specCollObjRename: l3Rename error from %s to %s, status = %d",
          srcDataObjInfo->subPath, newPath, status);
        return (status);
    }
    return (status);
}

int
l3Rename (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo, char *newFileName)
{
    int rescTypeInx;
    fileRenameInp_t fileRenameInp;
    int status;

    if (getStructFileType (dataObjInfo->specColl) >= 0) {
	subStructFileRenameInp_t subStructFileRenameInp;
        memset (&subStructFileRenameInp, 0, sizeof (subStructFileRenameInp));
        rstrcpy (subStructFileRenameInp.subFile.subFilePath, dataObjInfo->subPath,
          MAX_NAME_LEN);
	rstrcpy (subStructFileRenameInp.newSubFilePath, newFileName, MAX_NAME_LEN);
        rstrcpy (subStructFileRenameInp.subFile.addr.hostAddr, 
	  dataObjInfo->rescInfo->rescLoc, NAME_LEN);
        subStructFileRenameInp.subFile.specColl = dataObjInfo->specColl;
        status = rsSubStructFileRename (rsComm, &subStructFileRenameInp);
    } else {
        rescTypeInx = dataObjInfo->rescInfo->rescTypeInx;

        switch (RescTypeDef[rescTypeInx].rescCat) {
          case FILE_CAT:
            memset (&fileRenameInp, 0, sizeof (fileRenameInp));
            fileRenameInp.fileType = RescTypeDef[rescTypeInx].driverType;
            rstrcpy (fileRenameInp.oldFileName, dataObjInfo->filePath,
              MAX_NAME_LEN);
            rstrcpy (fileRenameInp.newFileName, newFileName,
              MAX_NAME_LEN);
            rstrcpy (fileRenameInp.addr.hostAddr,
              dataObjInfo->rescInfo->rescLoc, NAME_LEN);
            status = rsFileRename (rsComm, &fileRenameInp);
            break;
          default:
            rodsLog (LOG_NOTICE,
              "l3Rename: rescCat type %d is not recognized",
              RescTypeDef[rescTypeInx].rescCat);
            status = SYS_INVALID_RESC_TYPE;
            break;
        }
    }
    return (status);
}

