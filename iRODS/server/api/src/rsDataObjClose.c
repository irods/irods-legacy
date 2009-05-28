/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See dataObjClose.h for a description of this API call.*/

/* XXXXXX take me out. For testing only */
/* #define TEST_QUE_RULE	1 */

#include "dataObjClose.h"
#include "rodsLog.h"
#include "regReplica.h"
#include "modDataObjMeta.h"
#include "dataObjOpr.h"
#include "objMetaOpr.h"
#include "dataObjUnlink.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"
#include "reDefines.h"
#include "ruleExecSubmit.h"
#include "subStructFileRead.h"
#include "subStructFileStat.h"
#include "subStructFileClose.h"
#include "regDataObj.h"

#ifdef LOG_TRANSFERS
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

int
rsDataObjClose (rsComm_t *rsComm, openedDataObjInp_t *dataObjCloseInp)
{
    int status;
    int srcL1descInx;
    openedDataObjInp_t myDataObjCloseInp;
    int l1descInx;
    ruleExecInfo_t rei;
#ifdef TEST_QUE_RULE
    bytesBuf_t *packedReiAndArgBBuf = NULL;;
    ruleExecSubmitInp_t ruleExecSubmitInp;
#endif

    l1descInx = dataObjCloseInp->l1descInx;
    if (l1descInx <= 2 || l1descInx >= NUM_L1_DESC) {
       rodsLog (LOG_NOTICE,
         "rsDataObjClose: l1descInx %d out of range",
         l1descInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }

    if (L1desc[l1descInx].remoteZoneHost != NULL) {
	/* cross zone operation */
	dataObjCloseInp->l1descInx = L1desc[l1descInx].remoteL1descInx;
	status = rcDataObjClose (L1desc[l1descInx].remoteZoneHost->conn, 
	  dataObjCloseInp);
	dataObjCloseInp->l1descInx = l1descInx;
    } else {
        status = _rsDataObjClose (rsComm, dataObjCloseInp);

        if (status >= 0) {
	    /* note : this may overlap with acPostProcForPut or 
	     * acPostProcForCopy */
	    if (L1desc[l1descInx].openType == CREATE_TYPE) {
                initReiWithDataObjInp (&rei, rsComm,
                  L1desc[l1descInx].dataObjInp);
                rei.doi = L1desc[l1descInx].dataObjInfo;
                status = applyRule ("acPostProcForCreate", NULL, &rei,
                  NO_SAVE_REI);
                /* doi might have changed */
                L1desc[l1descInx].dataObjInfo = rei.doi;
	    } else if (L1desc[l1descInx].openType == OPEN_FOR_READ_TYPE ||
	      L1desc[l1descInx].openType == OPEN_FOR_WRITE_TYPE) {
                initReiWithDataObjInp (&rei, rsComm,
                  L1desc[l1descInx].dataObjInp);
                rei.doi = L1desc[l1descInx].dataObjInfo;
                status = applyRule ("acPostProcForOpen", NULL, &rei,
                  NO_SAVE_REI);
                /* doi might have changed */
                L1desc[l1descInx].dataObjInfo = rei.doi;
	    }

	    if (L1desc[l1descInx].oprType == PUT_OPR || 
	      L1desc[l1descInx].openType == CREATE_TYPE ||
    	      (L1desc[l1descInx].openType == OPEN_FOR_WRITE_TYPE && 
	      (L1desc[l1descInx].bytesWritten > 0 ||
      	      dataObjCloseInp->bytesWritten > 0))) {
                initReiWithDataObjInp (&rei, rsComm, 
		  L1desc[l1descInx].dataObjInp);
                rei.doi = L1desc[l1descInx].dataObjInfo;
#ifdef TEST_QUE_RULE
	        status = packReiAndArg (rsComm, &rei, NULL, 0, 
	          &packedReiAndArgBBuf);
	        if (status < 0) {
		    rodsLog (LOG_ERROR,
		     "rsDataObjClose: packReiAndArg error");
	        } else {
		    memset (&ruleExecSubmitInp, 0, sizeof (ruleExecSubmitInp));
		    rstrcpy (ruleExecSubmitInp.ruleName, "acPostProcForPut",
		      NAME_LEN);
		    rstrcpy (ruleExecSubmitInp.userName, rei.uoic->userName,
		      NAME_LEN);
		    ruleExecSubmitInp.packedReiAndArgBBuf = packedReiAndArgBBuf;
		    getNowStr (ruleExecSubmitInp.exeTime);
		    status = rsRuleExecSubmit (rsComm, &ruleExecSubmitInp);
		    free (packedReiAndArgBBuf);
		    if (status < 0) {
		        rodsLog (LOG_ERROR,
		         "rsDataObjClose: rsRuleExecSubmit failed");
		    }
	        }
#else
	        status = applyRule ("acPostProcForPut", NULL, &rei, 
	          NO_SAVE_REI);
	        /* doi might have changed */
	        L1desc[l1descInx].dataObjInfo = rei.doi;
#endif
	    } else if (L1desc[l1descInx].oprType == COPY_DEST) {
                initReiWithDataObjInp (&rei, rsComm, 
		  L1desc[l1descInx].dataObjInp);
		  rei.doi = L1desc[l1descInx].dataObjInfo;
		  status = applyRule ("acPostProcForCopy", NULL, &rei,
		    NO_SAVE_REI);
            /* doi might have changed */
                L1desc[l1descInx].dataObjInfo = rei.doi;
	    }
	}
    }

    srcL1descInx = L1desc[l1descInx].srcL1descInx;
    if ((L1desc[l1descInx].oprType == REPLICATE_DEST ||
     L1desc[l1descInx].oprType == COPY_DEST) &&
      srcL1descInx > 2) {
        memset (&myDataObjCloseInp, 0, sizeof (myDataObjCloseInp));
        myDataObjCloseInp.l1descInx = srcL1descInx;
        rsDataObjClose (rsComm, &myDataObjCloseInp);
    }

    freeL1desc (l1descInx);

    return (status);
}

#ifdef LOG_TRANSFERS
/* The log-transfers feature is manually enabled by editing
   server/Makefile to include a 'CFLAGS += -DLOG_TRANSFERS'.  We do
   not recommend enabling this as it will add a lot of entries to the
   server log file and increase overhead, but when enabled file
   transfers will be logged.  Currently, the operations logged are
   get, put, replicate, and rsync.  The start time is when the
   file is openned and the end time is when closed (but before the
   post-processing rule is executed).

   Each server will log it's transfers in it's own log file.  

   See the code for details of what is logged.  You may want to tailor
   this code to your specific interests.
*/
int
logTransfer (char *oprType, char *objPath, rodsLong_t fileSize, 
           struct timeval *startTime, char *rescName, char *userName,
	     char *hostName)
{
    struct timeval diffTime;
    char myDir[MAX_NAME_LEN], myFile[MAX_NAME_LEN];
    float transRate, sizeInMb, timeInSec;
    int status;
    struct timeval endTime;

    (void)gettimeofday(&endTime, (struct timezone *)0);


    if ((status = splitPathByKey (objPath, myDir, myFile, '/')) < 0) {
        rodsLogError (LOG_NOTICE, status,
          "printTiming: splitPathByKey for %s error, status = %d",
          objPath, status);
        return (status);
    }

    diffTime.tv_sec = endTime.tv_sec - startTime->tv_sec;
    diffTime.tv_usec = endTime.tv_usec - startTime->tv_usec;

    if (diffTime.tv_usec < 0) {
	diffTime.tv_sec --;
	diffTime.tv_usec += 1000000;
    }

    timeInSec = (float) diffTime.tv_sec + ((float) diffTime.tv_usec /
     1000000.0);

    if (fileSize <= 0) {
	transRate = 0.0;
	sizeInMb = 0.0;
    } else {
	sizeInMb = (float) fileSize / 1048600.0;
        if (timeInSec == 0.0) {
	    transRate = 0.0;
        } else {
	    transRate = sizeInMb / timeInSec;
	}
    }

    if (fileSize < 0) {
        rodsLog (LOG_NOTICE, "Transfer %.3f sec %s %s %s %s %s\n",
	    timeInSec, oprType, rescName, objPath, hostName);
    } else {
        rodsLog (LOG_NOTICE, "Transfer %10.3f MB | %.3f sec | %6.3f MB/s %s %s %s %s %s\n",
            sizeInMb, timeInSec, transRate, oprType, rescName, 
	    objPath, userName, hostName);
    }
    return (0);
}
#endif

int
_rsDataObjClose (rsComm_t *rsComm, openedDataObjInp_t *dataObjCloseInp)
{
    int status = 0;
    int l1descInx, l3descInx;
    keyValPair_t regParam;
    rodsLong_t newSize;
    char tmpStr[MAX_NAME_LEN];
    dataObjInfo_t *myDataObjInfo;
    modDataObjMeta_t modDataObjMetaInp;

    l1descInx = dataObjCloseInp->l1descInx;

    l3descInx = L1desc[l1descInx].l3descInx;

    if (l3descInx > 2) {
        /* it could be -ive for parallel I/O */
        status = l3Close (rsComm, l1descInx);

        if (status < 0) {
            rodsLog (LOG_NOTICE,
             "_rsDataObjClose: l3Close of %d failed, status = %d",
             l3descInx, status);
            return (status);
        }
    }

    if (L1desc[l1descInx].oprStatus < 0) {
        /* an error has occurred */ 
	return L1desc[l1descInx].oprStatus;
    }

    if (L1desc[l1descInx].bytesWritten <= 0 && 
      dataObjCloseInp->bytesWritten <= 0 &&
      L1desc[l1descInx].oprType != REPLICATE_DEST &&
      L1desc[l1descInx].oprType != PHYMV_DEST) {
        /* no write */
#ifdef LOG_TRANSFERS
       if (L1desc[l1descInx].oprType == GET_OPR) {
	  logTransfer("get", L1desc[l1descInx].dataObjInfo->objPath,
		      L1desc[l1descInx].dataObjInfo->dataSize,
		      &L1desc[l1descInx].openStartTime,
		      L1desc[l1descInx].dataObjInfo->rescName,
		      rsComm->clientUser.userName,
		      inet_ntoa (rsComm->remoteAddr.sin_addr));
       }
#endif
        return (status);
    }

    if (L1desc[l1descInx].stageFlag == NO_STAGING) {
	/* don't check for size if it is DO_STAGING type because the
	 * fileStat call may not be supported */ 
        newSize = getSizeInVault (rsComm, L1desc[l1descInx].dataObjInfo);

        /* check for consistency of the write operation */

        if (L1desc[l1descInx].dataSize > 0) { 
            if (newSize != L1desc[l1descInx].dataSize) {
	        rodsLog (LOG_NOTICE,
	          "_rsDataObjClose: size in vault %lld != target size %lld",
	            newSize, L1desc[l1descInx].dataSize);
	        return (SYS_COPY_LEN_ERR);
	    }
	}
    } else {
	newSize = L1desc[l1descInx].bytesWritten;
    }

    memset (&regParam, 0, sizeof (regParam));

    myDataObjInfo = L1desc[l1descInx].dataObjInfo;

    if (strlen (myDataObjInfo->chksum) > 0 && 
      L1desc[l1descInx].oprType != REPLICATE_DEST &&
      L1desc[l1descInx].oprType != PHYMV_DEST) {
	/* overwriting an old copy. need to verify the chksum again */ 
	L1desc[l1descInx].chksumFlag = VERIFY_CHKSUM;
    } 

    if (L1desc[l1descInx].chksumFlag > 0) {
        /* dataObjChksum will verify if VERIFY_CHKSUM is set.
         * then call addKeyVal */
        status = dataObjChksum (rsComm, l1descInx, &regParam);

        if (status < 0) {
            return (status);
        }
    }

    if (L1desc[l1descInx].oprType == REPLICATE_DEST || 
      L1desc[l1descInx].oprType == PHYMV_DEST) {
	dataObjInfo_t *destDataObjInfo, *srcDataObjInfo;
	int srcL1descInx;
	regReplica_t regReplicaInp;
	char *chksumStr = NULL;

  	destDataObjInfo = L1desc[l1descInx].dataObjInfo;
	srcL1descInx = L1desc[l1descInx].srcL1descInx;
	if (srcL1descInx <= 2) {
            rodsLog (LOG_NOTICE,
              "_rsDataObjClose: srcL1descInx %d out of range",
              srcL1descInx);
             return (SYS_FILE_DESC_OUT_OF_RANGE);
	}
	srcDataObjInfo = L1desc[srcL1descInx].dataObjInfo;
	if (strlen (srcDataObjInfo->chksum) > 0 &&
	  srcDataObjInfo->replStatus > 0) {
	    /* the source has chksum. Must verify chksum */ 

            status = _dataObjChksum (rsComm, destDataObjInfo, &chksumStr);
            if (status < 0) {
                rodsLog (LOG_NOTICE,
                 "_rsDataObjClose: _dataObjChksum error for %s, status = %d",
                  destDataObjInfo->objPath, status);
            } else {
		if (strcmp (srcDataObjInfo->chksum, chksumStr) != 0) {
		    free (chksumStr); 
                    rodsLog (LOG_NOTICE,
                     "_rsDataObjClose: chksum mismatch for for %s",
                     destDataObjInfo->objPath);
		    return USER_CHKSUM_MISMATCH;
		}
	    }
	}
	if (L1desc[l1descInx].oprType == PHYMV_DEST) {
	    /* a phymv */
            if (chksumStr != NULL) {
                addKeyVal (&regParam, CHKSUM_KW, chksumStr);
            }
	    addKeyVal (&regParam, FILE_PATH_KW, destDataObjInfo->filePath);
	    addKeyVal (&regParam, RESC_NAME_KW, destDataObjInfo->rescName);
	    addKeyVal (&regParam, RESC_GROUP_NAME_KW, 
	      destDataObjInfo->rescGroupName);
	    if (getValByKey (&L1desc[l1descInx].dataObjInp->condInput, 
	      IRODS_ADMIN_KW) != NULL) {
		addKeyVal (&regParam, IRODS_ADMIN_KW, "");
	    }
            modDataObjMetaInp.dataObjInfo = destDataObjInfo;
            modDataObjMetaInp.regParam = &regParam;
            status = rsModDataObjMeta (rsComm, &modDataObjMetaInp);
            if (status >= 0) {
		int status1;
	        status1 = l3Close (rsComm, srcL1descInx);
		if (status1 < 0) {
                    rodsLog (LOG_NOTICE,
              	      "_rsDataObjClose: l3Close of %s error. status = %d",
              	      srcDataObjInfo->objPath, status1);
		}

                l3Unlink (rsComm, srcDataObjInfo);
	        freeL1desc (srcL1descInx);
	    }
        } else if (L1desc[l1descInx].replStatus & OPEN_EXISTING_COPY) {
	    /* overwrite an existing repl */
            snprintf (tmpStr, MAX_NAME_LEN, "%d", srcDataObjInfo->replStatus);
             addKeyVal (&regParam, REPL_STATUS_KW, tmpStr);
            snprintf (tmpStr, MAX_NAME_LEN, "%lld", srcDataObjInfo->dataSize);
            addKeyVal (&regParam, DATA_SIZE_KW, tmpStr);
	    if (chksumStr != NULL) {
		addKeyVal (&regParam, CHKSUM_KW, chksumStr);
	    }
            if (getValByKey (&L1desc[l1descInx].dataObjInp->condInput,
              IRODS_ADMIN_KW) != NULL) {
                addKeyVal (&regParam, IRODS_ADMIN_KW, "");
            }
            modDataObjMetaInp.dataObjInfo = destDataObjInfo;
            modDataObjMetaInp.regParam = &regParam;
            status = rsModDataObjMeta (rsComm, &modDataObjMetaInp);
        } else {
	    memset (&regReplicaInp, 0, sizeof (regReplicaInp));
	    if (destDataObjInfo->dataId <= 0)
		destDataObjInfo->dataId = srcDataObjInfo->dataId;
	    regReplicaInp.srcDataObjInfo = srcDataObjInfo;
	    regReplicaInp.destDataObjInfo = destDataObjInfo;
            if (getValByKey (&L1desc[l1descInx].dataObjInp->condInput,
              SU_CLIENT_USER_KW) != NULL) {
                addKeyVal (&regReplicaInp.condInput, SU_CLIENT_USER_KW, "");
                addKeyVal (&regReplicaInp.condInput, IRODS_ADMIN_KW, "");
            } else if (getValByKey (&L1desc[l1descInx].dataObjInp->condInput,
              IRODS_ADMIN_KW) != NULL) {
                addKeyVal (&regReplicaInp.condInput, IRODS_ADMIN_KW, "");
            }
            status = rsRegReplica (rsComm, &regReplicaInp);
            clearKeyVal (&regReplicaInp.condInput);
	}
	if (chksumStr != NULL)
	    free (chksumStr);

        clearKeyVal (&regParam);

	if (status < 0) {
	    l3Unlink (rsComm, myDataObjInfo);
            rodsLog (LOG_NOTICE,
              "_rsDataObjClose: rsRegReplica or rsModDataObjMeta of %s error. status = %d",
              destDataObjInfo->objPath, status);
	    return (status);
	}
    } else if (L1desc[l1descInx].dataObjInfo->specColl == NULL) {
	if (l3descInx < 2 &&
	  L1desc[l1descInx].dataObjInpReplFlag == 1 &&
	  getValByKey (&L1desc[l1descInx].dataObjInp->condInput,
	  REPL_DATA_OBJ_INP_KW) != NULL &&
	  L1desc[l1descInx].replStatus == NEWLY_CREATED_COPY) {
	    /* the comes from a cross zone copy. have not been
	     * registered yet */
            status = svrRegDataObj (rsComm, myDataObjInfo);
            if (status < 0) {
                rodsLog (LOG_NOTICE,
                  "_rsDataObjClose: svrRegDataObj for %s failed, status = %d",
                  myDataObjInfo->objPath, status);
            }
	}
	      
        if (myDataObjInfo == NULL || myDataObjInfo->dataSize != newSize) {
            snprintf (tmpStr, MAX_NAME_LEN, "%lld", newSize);
            addKeyVal (&regParam, DATA_SIZE_KW, tmpStr);
	}

	if (L1desc[l1descInx].replStatus & OPEN_EXISTING_COPY) {
	    addKeyVal (&regParam, ALL_REPL_STATUS_KW, tmpStr);
            snprintf (tmpStr, MAX_NAME_LEN, "%d", (int) time (NULL));
            addKeyVal (&regParam, DATA_MODIFY_KW, tmpStr);
	} else {
            snprintf (tmpStr, MAX_NAME_LEN, "%d", NEWLY_CREATED_COPY); 
             addKeyVal (&regParam, REPL_STATUS_KW, tmpStr);
        }
        modDataObjMetaInp.dataObjInfo = L1desc[l1descInx].dataObjInfo;
        modDataObjMetaInp.regParam = &regParam;

        status = rsModDataObjMeta (rsComm, &modDataObjMetaInp);

        clearKeyVal (&regParam);

        if (status < 0) {
            return (status);
        }

        if (status < 0) {
            return (status);
        }
    }

    /* XXXXXX need to replicate to moreRescGrpInfo */

    /* for post processing */
    L1desc[l1descInx].bytesWritten = myDataObjInfo->dataSize = newSize;

#ifdef LOG_TRANSFERS
    /* transfer logging */
    if (L1desc[l1descInx].oprType == PUT_OPR ||
        L1desc[l1descInx].oprType == CREATE_OPR) {
        logTransfer("put", L1desc[l1descInx].dataObjInfo->objPath,
                   L1desc[l1descInx].dataObjInfo->dataSize,
                   &L1desc[l1descInx].openStartTime,
                   L1desc[l1descInx].dataObjInfo->rescName,
                   rsComm->clientUser.userName,
                   inet_ntoa (rsComm->remoteAddr.sin_addr));

    }
    if (L1desc[l1descInx].oprType == GET_OPR) {
       logTransfer("get", L1desc[l1descInx].dataObjInfo->objPath,
                   L1desc[l1descInx].dataObjInfo->dataSize,
                   &L1desc[l1descInx].openStartTime,
                   L1desc[l1descInx].dataObjInfo->rescName,
                   rsComm->clientUser.userName,
                   inet_ntoa (rsComm->remoteAddr.sin_addr));
    }
    if (L1desc[l1descInx].oprType == REPLICATE_OPR ||
        L1desc[l1descInx].oprType == REPLICATE_DEST) {
       logTransfer("replicate", L1desc[l1descInx].dataObjInfo->objPath,
                   L1desc[l1descInx].dataObjInfo->dataSize,
                   &L1desc[l1descInx].openStartTime,
                   L1desc[l1descInx].dataObjInfo->rescName,
                   rsComm->clientUser.userName,
                   inet_ntoa (rsComm->remoteAddr.sin_addr));
    }
    /* Not sure if this happens, irsync might show up as 'put' */
    if (L1desc[l1descInx].oprType == RSYNC_OPR) {
       logTransfer("rsync", L1desc[l1descInx].dataObjInfo->objPath,
                   L1desc[l1descInx].dataObjInfo->dataSize,
                   &L1desc[l1descInx].openStartTime,
                   L1desc[l1descInx].dataObjInfo->rescName,
                   rsComm->clientUser.userName,
                   inet_ntoa (rsComm->remoteAddr.sin_addr));
    }
#endif

    return (status);
}

int
l3Close (rsComm_t *rsComm, int l1descInx)
{
    int rescTypeInx;
    fileCloseInp_t fileCloseInp;
    int status;

    dataObjInfo_t *dataObjInfo;
    dataObjInfo = L1desc[l1descInx].dataObjInfo;

    if (getStructFileType (dataObjInfo->specColl) >= 0) {
        subStructFileFdOprInp_t subStructFileCloseInp;
        memset (&subStructFileCloseInp, 0, sizeof (subStructFileCloseInp));
	subStructFileCloseInp.type = dataObjInfo->specColl->type;
        subStructFileCloseInp.fd = L1desc[l1descInx].l3descInx;
        rstrcpy (subStructFileCloseInp.addr.hostAddr, dataObjInfo->rescInfo->rescLoc,
          NAME_LEN);
        status = rsSubStructFileClose (rsComm, &subStructFileCloseInp);
    } else {
        rescTypeInx = L1desc[l1descInx].dataObjInfo->rescInfo->rescTypeInx;

        switch (RescTypeDef[rescTypeInx].rescCat) {
          case FILE_CAT:
            memset (&fileCloseInp, 0, sizeof (fileCloseInp));
            fileCloseInp.fileInx = L1desc[l1descInx].l3descInx;
            status = rsFileClose (rsComm, &fileCloseInp);
            break;

          default:
            rodsLog (LOG_NOTICE,
              "l3Close: rescCat type %d is not recognized",
              RescTypeDef[rescTypeInx].rescCat);
            status = SYS_INVALID_RESC_TYPE;
            break;
	}
    }
    return (status);
}

int
_l3Close (rsComm_t *rsComm, int rescTypeInx, int l3descInx)
{
    fileCloseInp_t fileCloseInp;
    int status;

    switch (RescTypeDef[rescTypeInx].rescCat) {
      case FILE_CAT:
        memset (&fileCloseInp, 0, sizeof (fileCloseInp));
        fileCloseInp.fileInx = l3descInx;
        status = rsFileClose (rsComm, &fileCloseInp);
        break;

      default:
        rodsLog (LOG_NOTICE,
          "_l3Close: rescCat type %d is not recognized",
          RescTypeDef[rescTypeInx].rescCat);
        status = SYS_INVALID_RESC_TYPE;
        break;
    }
    return (status);
}

int
l3Stat (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo, rodsStat_t **myStat)
{
    int rescTypeInx;
    fileStatInp_t fileStatInp;
    int status;

    if (getStructFileType (dataObjInfo->specColl) >= 0) {
        subFile_t subFile;
        memset (&subFile, 0, sizeof (subFile));
        rstrcpy (subFile.subFilePath, dataObjInfo->subPath,
          MAX_NAME_LEN);
        rstrcpy (subFile.addr.hostAddr, dataObjInfo->rescInfo->rescLoc,
          NAME_LEN);
        subFile.specColl = dataObjInfo->specColl;
        status = rsSubStructFileStat (rsComm, &subFile, myStat);
    } else {
        rescTypeInx = dataObjInfo->rescInfo->rescTypeInx;

        switch (RescTypeDef[rescTypeInx].rescCat) {
          case FILE_CAT:
            memset (&fileStatInp, 0, sizeof (fileStatInp));
            fileStatInp.fileType = RescTypeDef[rescTypeInx].driverType;
            rstrcpy (fileStatInp.fileName, dataObjInfo->filePath, 
	      MAX_NAME_LEN);
	    rstrcpy (fileStatInp.addr.hostAddr,  
	      dataObjInfo->rescInfo->rescLoc, NAME_LEN);
            status = rsFileStat (rsComm, &fileStatInp, myStat);
            break;

          default:
            rodsLog (LOG_NOTICE,
              "l3Stat: rescCat type %d is not recognized",
              RescTypeDef[rescTypeInx].rescCat);
            status = SYS_INVALID_RESC_TYPE;
            break;
	}
    }
    return (status);
}

#ifdef COMPAT_201
int
rsDataObjClose201 (rsComm_t *rsComm, dataObjCloseInp_t *dataObjCloseInp)
{
    openedDataObjInp_t openedDataObjInp;
    int status;

    bzero (&openedDataObjInp, sizeof (openedDataObjInp));

    openedDataObjInp.l1descInx = dataObjCloseInp->l1descInx;
    openedDataObjInp.bytesWritten = dataObjCloseInp->bytesWritten;

    status = rsDataObjClose (rsComm, &openedDataObjInp);

    return status;
}
#endif

