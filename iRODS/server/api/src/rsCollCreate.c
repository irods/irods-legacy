/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See collCreate.h for a description of this API call.*/

#include "collCreate.h"
#include "rodsLog.h"
#include "regColl.h"
#include "icatDefines.h"
#include "fileMkdir.h"
#include "subStructFileMkdir.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "objMetaOpr.h"

int
rsCollCreate (rsComm_t *rsComm, collInp_t *collCreateInp)
{
    int status;
    rodsServerHost_t *rodsServerHost = NULL;

    status = getAndConnRcatHost (rsComm, MASTER_RCAT, collCreateInp->collName,
                                &rodsServerHost);
    if (status < 0) {
       return(status);
    }

    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
        dataObjInp_t dataObjInp;
        dataObjInfo_t *dataObjInfo = NULL;

        /* for STRUCT_FILE_COLL to make a directory in the structFile, the
         * COLLECTION_TYPE_KW must be set */

	memset (&dataObjInp, 0, sizeof (dataObjInp));
	rstrcpy (dataObjInp.objPath, collCreateInp->collName, MAX_NAME_LEN);
        status = resolveSpecColl (rsComm, &dataObjInp, &dataObjInfo, 1);
	if (status >= 0) {
	    if (status == COLL_OBJ_T) {
		return (0);
	    } else if (status == DATA_OBJ_T) {
		return (USER_INPUT_PATH_ERR);
	    }
	} else if (status == SYS_SPEC_COLL_OBJ_NOT_EXIST) { 
            /* for STRUCT_FILE_COLL to make a directory in the structFile, the
             * COLLECTION_TYPE_KW must be set */
#if 0
	    if (getValByKey (&collCreateInp->condInput, COLLECTION_TYPE_KW) ==
              NULL && dataObjInfo->specColl->class == STRUCT_FILE_COLL) {
#endif
	    if (getSpecCollOpr (&collCreateInp->condInput, 
	      dataObjInfo->specColl) == NORMAL_OPR_ON_BUN_COLL) {
        	status = _rsRegColl (rsComm, collCreateInp);
	    } else {
	        status = l3Mkdir (rsComm, dataObjInfo);
	    }
	    freeDataObjInfo (dataObjInfo);
	    return (status);
	} else {
            status = _rsRegColl (rsComm, collCreateInp);
	}
#else
        status = SYS_NO_RCAT_SERVER_ERR;
#endif
    } else {
        status = rcCollCreate (rodsServerHost->conn, collCreateInp);
    }

    return (status);
}

int
l3Mkdir (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo)
{
    int rescTypeInx;
    fileMkdirInp_t fileMkdirInp;
    int status;

    if (getStructFileType (dataObjInfo->specColl) >= 0) {
        subFile_t subFile;
        memset (&subFile, 0, sizeof (subFile));
        rstrcpy (subFile.subFilePath, dataObjInfo->subPath,
          MAX_NAME_LEN);
        rstrcpy (subFile.addr.hostAddr, dataObjInfo->rescInfo->rescLoc,
          NAME_LEN);
        subFile.specColl = dataObjInfo->specColl;
        status = rsSubStructFileMkdir (rsComm, &subFile);
    } else {
        rescTypeInx = dataObjInfo->rescInfo->rescTypeInx;

        switch (RescTypeDef[rescTypeInx].rescCat) {
          case FILE_CAT:
            memset (&fileMkdirInp, 0, sizeof (fileMkdirInp));
            fileMkdirInp.fileType = RescTypeDef[rescTypeInx].driverType;
            rstrcpy (fileMkdirInp.dirName, dataObjInfo->filePath,
              MAX_NAME_LEN);
            rstrcpy (fileMkdirInp.addr.hostAddr,
              dataObjInfo->rescInfo->rescLoc, NAME_LEN);
            fileMkdirInp.mode = DEFAULT_DIR_MODE;
            status = rsFileMkdir (rsComm, &fileMkdirInp);
            break;

          default:
            rodsLog (LOG_NOTICE,
              "l3Mkdir: rescCat type %d is not recognized",
              RescTypeDef[rescTypeInx].rescCat);
            status = SYS_INVALID_RESC_TYPE;
            break;
        }
    }
    return (status);
}

