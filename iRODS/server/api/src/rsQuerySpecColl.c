/*** Copyright (c), The Unregents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsQuerySpecColl.c
 */

#include "querySpecColl.h"
#include "rcMisc.h"
#include "fileOpendir.h"
#include "fileReaddir.h"
#include "fileClosedir.h"
#include "objMetaOpr.h"
#include "dataObjClose.h"
#include "bunSubOpendir.h"
#include "bunSubReaddir.h"
#include "bunSubClosedir.h"
#include "fileStat.h"
#include "genQuery.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"

int
rsQuerySpecColl (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
genQueryOut_t **genQueryOut)
{
    int specCollInx;
    int status;
    int continueFlag; 	/* continue query */ 

    if ((specCollInx = dataObjInp->openFlags) <= 0) {
	specCollInx = openSpecColl (rsComm, dataObjInp, -1);
        if (specCollInx < 0) {
            rodsLog (LOG_NOTICE,
              "rsQuerySpecColl: openSpecColl error for %s, status = %d",
              dataObjInp->objPath, specCollInx);
            return (specCollInx);
        }
	continueFlag = 0;
    } else {
	continueFlag = 1;
    }
	
    initOutForQuerySpecColl (genQueryOut);

    status = _rsQuerySpecColl (rsComm, specCollInx, dataObjInp,
      *genQueryOut, continueFlag);

    if (status < 0) {
	freeGenQueryOut (genQueryOut);
    }
    return (status);
}

int
openSpecColl (rsComm_t *rsComm, dataObjInp_t *dataObjInp, int parentInx)
{
    int specCollInx;
    dataObjInfo_t *dataObjInfo = NULL;
    int status;
    int l3descInx;

    status = resolveSpecColl (rsComm, dataObjInp, &dataObjInfo, 0);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
          "rsQuerySpecColl: resolveSpecColl error for %s, status = %d",
          dataObjInp->objPath, status);
	return (status);
    }

    l3descInx = l3Opendir (rsComm, dataObjInfo);

    if (l3descInx < 0) {
        rodsLog (LOG_NOTICE,
          "openSpecColl: specCollOpendir error for %s, status = %d",
          dataObjInp->objPath, status);
        return (l3descInx);
    }
    specCollInx = allocSpecCollDesc ();
    if (specCollInx < 0) {
	freeDataObjInfo (dataObjInfo);
	return (specCollInx);
    }
    SpecCollDesc[specCollInx].l3descInx = l3descInx;
    SpecCollDesc[specCollInx].dataObjInfo = dataObjInfo; 
    SpecCollDesc[specCollInx].parentInx = parentInx;

    return (specCollInx);
}

int
initOutForQuerySpecColl (genQueryOut_t **genQueryOut)
{
    genQueryOut_t *myGenQueryOut;

    /* will do collection, dataName, createTime, modifyTime, objSize */

    myGenQueryOut = *genQueryOut = 
      (genQueryOut_t *) malloc (sizeof (genQueryOut_t));

    memset (myGenQueryOut, 0, sizeof (genQueryOut_t));

    myGenQueryOut->attriCnt = 5;

    myGenQueryOut->sqlResult[0].attriInx = COL_COLL_NAME;
    myGenQueryOut->sqlResult[0].len = MAX_NAME_LEN;    
    myGenQueryOut->sqlResult[0].value = 
      malloc (MAX_NAME_LEN * MAX_SPEC_COLL_ROW);
    memset (myGenQueryOut->sqlResult[0].value, 0, 
      MAX_NAME_LEN * MAX_SPEC_COLL_ROW);
    myGenQueryOut->sqlResult[1].attriInx = COL_DATA_NAME;
    myGenQueryOut->sqlResult[1].len = MAX_NAME_LEN; 
    myGenQueryOut->sqlResult[1].value = 
      malloc (MAX_NAME_LEN * MAX_SPEC_COLL_ROW);
    memset (myGenQueryOut->sqlResult[1].value, 0, 
      MAX_NAME_LEN * MAX_SPEC_COLL_ROW);
    myGenQueryOut->sqlResult[2].attriInx = COL_COLL_CREATE_TIME;
    myGenQueryOut->sqlResult[2].len = NAME_LEN;
    myGenQueryOut->sqlResult[2].value =
      malloc (NAME_LEN * MAX_SPEC_COLL_ROW);
    memset (myGenQueryOut->sqlResult[2].value, 0,
      NAME_LEN * MAX_SPEC_COLL_ROW); 
    myGenQueryOut->sqlResult[3].attriInx = COL_COLL_MODIFY_TIME;
    myGenQueryOut->sqlResult[3].len = NAME_LEN;
    myGenQueryOut->sqlResult[3].value =
      malloc (NAME_LEN * MAX_SPEC_COLL_ROW);
    memset (myGenQueryOut->sqlResult[3].value, 0,
      NAME_LEN * MAX_SPEC_COLL_ROW);     
    myGenQueryOut->sqlResult[4].attriInx = COL_DATA_SIZE;
    myGenQueryOut->sqlResult[4].len = NAME_LEN;
    myGenQueryOut->sqlResult[4].value =
      malloc (NAME_LEN * MAX_SPEC_COLL_ROW);
    memset (myGenQueryOut->sqlResult[4].value, 0,
      NAME_LEN * MAX_SPEC_COLL_ROW);

    myGenQueryOut->continueInx = -1;

    return (0);
}

int
_rsQuerySpecColl (rsComm_t *rsComm, int specCollInx, 
dataObjInp_t *dataObjInp, genQueryOut_t *genQueryOut, int continueFlag)
{
    int status;
    rodsDirent_t *rodsDirent = NULL;
    dataObjInfo_t *dataObjInfo;
    rodsStat_t *fileStatOut = NULL;
    fileStatInp_t fileStatInp;
    int rowCnt;
    objType_t selObjType;
    char *tmpStr;
    dataObjInp_t newDataObjInp;
    int recurFlag;

    if (SpecCollDesc[specCollInx].inuseFlag != FD_INUSE) {
        rodsLog (LOG_ERROR,
          "_rsQuerySpecColl: Input specCollInx %d not active", specCollInx);
	return (BAD_INPUT_DESC_INDEX);
    }

    if ((tmpStr = getValByKey (&dataObjInp->condInput, SEL_OBJ_TYPE_KW)) != 
      NULL) {
	if (strcmp (tmpStr, "dataObj") == 0) {
	    selObjType = DATA_OBJ_T;
	} else {
	    selObjType = COLL_OBJ_T;
	}
    } else {
	selObjType = UNKNOWN_OBJ_T;
    }

    if (getValByKey (&dataObjInp->condInput, RECURSIVE_OPR__KW) != NULL) {
	recurFlag = 1;
    } else {
	recurFlag = 0;
    }

    dataObjInfo = SpecCollDesc[specCollInx].dataObjInfo;

    while (genQueryOut->rowCnt < MAX_SPEC_COLL_ROW) {
	dataObjInfo_t myDataObjInfo;
	rodsDirent_t myRodsDirent;

	status = specCollReaddir (rsComm, specCollInx, &rodsDirent);

	if (status < 0) {
	    break;
	}
	
	myRodsDirent = *rodsDirent;
	free (rodsDirent);

	if (strcmp (myRodsDirent.d_name, ".") == 0 ||
	  strcmp (myRodsDirent.d_name, "..") == 0) {
	    continue;
	} 

	myDataObjInfo = *dataObjInfo;
	
        snprintf (myDataObjInfo.filePath, MAX_NAME_LEN, "%s/%s",
          dataObjInfo->filePath, myRodsDirent.d_name);
        snprintf (myDataObjInfo.subPath, MAX_NAME_LEN, "%s/%s",
          dataObjInfo->objPath, myRodsDirent.d_name);
	status = l3Stat (rsComm, &myDataObjInfo, &fileStatOut);
        if (status < 0) {
            rodsLog (LOG_ERROR,
             "_rsQuerySpecColl: l3Stat for %s error, status = %d",
             fileStatInp.fileName, status);
           /* XXXXX need clean up */
            return (status);
        }
	if ((fileStatOut->st_mode & S_IFREG) != 0) {     /* a file */
	    if (selObjType == COLL_OBJ_T) {
		free (fileStatOut);
		continue;
	    }
	    rowCnt = genQueryOut->rowCnt;
	    rstrcpy (&genQueryOut->sqlResult[0].value[MAX_NAME_LEN * rowCnt], 
	      dataObjInfo->objPath, MAX_NAME_LEN);
	    rstrcpy (&genQueryOut->sqlResult[1].value[MAX_NAME_LEN * rowCnt], 
	      myRodsDirent.d_name, MAX_NAME_LEN);
            snprintf (&genQueryOut->sqlResult[2].value[NAME_LEN * rowCnt],
              NAME_LEN, "%d", fileStatOut->st_ctim);
            snprintf (&genQueryOut->sqlResult[3].value[NAME_LEN * rowCnt],
              NAME_LEN, "%d", fileStatOut->st_mtim);
            snprintf (&genQueryOut->sqlResult[4].value[NAME_LEN * rowCnt],
              NAME_LEN, "%lld", fileStatOut->st_size);

            free (fileStatOut);

            genQueryOut->rowCnt++;

	} else {
            if (selObjType != DATA_OBJ_T) {
	        rowCnt = genQueryOut->rowCnt;
	        snprintf (
	          &genQueryOut->sqlResult[0].value[MAX_NAME_LEN * rowCnt],
	          MAX_NAME_LEN, "%s/%s", dataObjInfo->objPath, 
	          myRodsDirent.d_name);
		snprintf (&genQueryOut->sqlResult[2].value[NAME_LEN * rowCnt],
	          NAME_LEN, "%d", fileStatOut->st_ctim);
	        snprintf (&genQueryOut->sqlResult[3].value[NAME_LEN * rowCnt],
	          NAME_LEN, "%d", fileStatOut->st_mtim);
	        snprintf (&genQueryOut->sqlResult[4].value[NAME_LEN * rowCnt],
	          NAME_LEN, "%lld", fileStatOut->st_size);
	        genQueryOut->rowCnt++;
	    }
	    free (fileStatOut);

            if (recurFlag > 0) {
		/* need to drill down */
		int newSpecCollInx; 
		newDataObjInp = *dataObjInp;
                snprintf (newDataObjInp.objPath,
                  MAX_NAME_LEN, "%s/%s", dataObjInfo->objPath,
                  myRodsDirent.d_name);
		newSpecCollInx = 
		  openSpecColl (rsComm, &newDataObjInp, specCollInx);
        	if (newSpecCollInx < 0) {
            	    rodsLog (LOG_ERROR,
              	      "_rsQuerySpecColl: openSpecColl err for %s, stat = %d",
              	      newDataObjInp.objPath, newSpecCollInx);
		    status = newSpecCollInx;
		    break;
		}
		status = _rsQuerySpecColl (rsComm, newSpecCollInx, 
		  &newDataObjInp, genQueryOut, 0);
		if (status < 0) {
		    break;
		}
	    }
        }
    }

    if (status == EOF || status == CAT_NO_ROWS_FOUND) {
        status = 0;
    }

    if (genQueryOut->rowCnt < MAX_SPEC_COLL_ROW) {
	int parentInx;
	/* get to the end or error */
	specCollClosedir (rsComm, specCollInx);
	parentInx = SpecCollDesc[specCollInx].parentInx;
	freeSpecCollDesc (specCollInx);
	if (status >= 0 && recurFlag && continueFlag && parentInx > 0) {
            newDataObjInp = *dataObjInp; 
            rstrcpy (newDataObjInp.objPath, 
	      SpecCollDesc[parentInx].dataObjInfo->objPath, MAX_NAME_LEN);
            status = _rsQuerySpecColl (rsComm, parentInx,
              &newDataObjInp, genQueryOut, continueFlag);
	}
        if (status == EOF || status == CAT_NO_ROWS_FOUND) {
            status = 0;
        }
    } else {
	/* more to come */
	if (genQueryOut->continueInx < 0) {
	    /* if one does not already exist */
	    genQueryOut->continueInx = specCollInx;
	}
    }

    if (status >= 0 && genQueryOut->rowCnt == 0) {
	status = CAT_NO_ROWS_FOUND;
    }  
    
    return (status);
}

int 
specCollReaddir (rsComm_t *rsComm, int specCollInx, rodsDirent_t **rodsDirent)
{
    fileReaddirInp_t fileReaddirInp;
    specColl_t *specColl;
    int status;
    dataObjInfo_t *dataObjInfo = SpecCollDesc[specCollInx].dataObjInfo;

    if (dataObjInfo == NULL || (specColl = dataObjInfo->specColl) == NULL) {
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (getBunType (dataObjInfo->specColl) >= 0) {
        bunSubFdOprInp_t bunSubReaddirInp;
        memset (&bunSubReaddirInp, 0, sizeof (bunSubReaddirInp));
        bunSubReaddirInp.type = dataObjInfo->specColl->type;
        bunSubReaddirInp.fd = SpecCollDesc[specCollInx].l3descInx;
        rstrcpy (bunSubReaddirInp.addr.hostAddr, 
	  dataObjInfo->rescInfo->rescLoc, NAME_LEN);
        status = rsBunSubReaddir (rsComm, &bunSubReaddirInp, 
	  rodsDirent);
    } else if (specColl->class == MOUNTED_COLL) {
        fileReaddirInp.fileInx = SpecCollDesc[specCollInx].l3descInx;
        status = rsFileReaddir (rsComm, &fileReaddirInp, rodsDirent);
    } else {
       rodsLog (LOG_ERROR,
          "specCollReaddir: Unknown specColl class = %d",
          specColl->class);
	status = SYS_UNKNOWN_SPEC_COLL_CLASS;
    }

    return (status);
}

int 
specCollClosedir (rsComm_t *rsComm, int specCollInx)
{
    fileClosedirInp_t fileClosedirInp;
    specColl_t *specColl;
    int status;
    dataObjInfo_t *dataObjInfo = SpecCollDesc[specCollInx].dataObjInfo;

    if (dataObjInfo == NULL || (specColl = dataObjInfo->specColl) == NULL) {
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (getBunType (dataObjInfo->specColl) >= 0) {
        bunSubFdOprInp_t bunSubClosedirInp;
        memset (&bunSubClosedirInp, 0, sizeof (bunSubClosedirInp));
        bunSubClosedirInp.type = dataObjInfo->specColl->type;
        bunSubClosedirInp.fd = SpecCollDesc[specCollInx].l3descInx;
        rstrcpy (bunSubClosedirInp.addr.hostAddr, 
	  dataObjInfo->rescInfo->rescLoc, NAME_LEN);
        status = rsBunSubClosedir (rsComm, &bunSubClosedirInp); 
    } else if (specColl->class == MOUNTED_COLL) {
        fileClosedirInp.fileInx = SpecCollDesc[specCollInx].l3descInx;
        status = rsFileClosedir (rsComm, &fileClosedirInp);
    } else {
       rodsLog (LOG_ERROR,
          "specCollClosedir: Unknown specColl class = %d",
          specColl->class);
	status = SYS_UNKNOWN_SPEC_COLL_CLASS;
    }

    return (status);
}

int 
l3Opendir (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo)
{
    fileOpendirInp_t fileOpendirInp;
    int status;
    int rescTypeInx;

    if (dataObjInfo == NULL) return (SYS_INTERNAL_NULL_INPUT_ERR);

    if (getBunType (dataObjInfo->specColl) >= 0) {
	status = 0;
        subFile_t bunSubOpendirInp;
        memset (&bunSubOpendirInp, 0, sizeof (bunSubOpendirInp));
        rstrcpy (bunSubOpendirInp.subFilePath, dataObjInfo->subPath,
          MAX_NAME_LEN);
        rstrcpy (bunSubOpendirInp.addr.hostAddr,
          dataObjInfo->rescInfo->rescLoc, NAME_LEN);
        bunSubOpendirInp.specColl = dataObjInfo->specColl;
        status = rsBunSubOpendir (rsComm, &bunSubOpendirInp);
    } else {
        rescTypeInx = dataObjInfo->rescInfo->rescTypeInx;

        switch (RescTypeDef[rescTypeInx].rescCat) {
          case FILE_CAT:
            memset (&fileOpendirInp, 0, sizeof (fileOpendirInp));
            rstrcpy (fileOpendirInp.dirName, dataObjInfo->filePath, 
	      MAX_NAME_LEN);
            fileOpendirInp.fileType = RescTypeDef[rescTypeInx].driverType;
            rstrcpy (fileOpendirInp.addr.hostAddr,
              dataObjInfo->rescInfo->rescLoc, NAME_LEN);
            status = rsFileOpendir (rsComm, &fileOpendirInp);
            if (status < 0) {
               rodsLog (LOG_ERROR,
                  "specCollOpendir: rsFileOpendir for %s error, status = %d",
                  dataObjInfo->filePath, status);
            }
            break;
          default:
            rodsLog (LOG_NOTICE,
              "l3Opendir: rescCat type %d is not recognized",
              RescTypeDef[rescTypeInx].rescCat);
            status = SYS_INVALID_RESC_TYPE;
            break;
        }
    }
    return (status);
}

#if 0
int 
specCollOpendir (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo)
{
    specColl_t *specColl;
    fileOpendirInp_t fileOpendirInp;
    int dirFd;
    int rescTypeInx;

    if (dataObjInfo == NULL || (specColl = dataObjInfo->specColl) == NULL) {
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
 
    if (specColl->class == MOUNTED_COLL) {
        memset (&fileOpendirInp, 0, sizeof (fileOpendirInp));

        rescTypeInx = dataObjInfo->rescInfo->rescTypeInx;
        rstrcpy (fileOpendirInp.dirName, dataObjInfo->filePath, MAX_NAME_LEN);
        fileOpendirInp.fileType = RescTypeDef[rescTypeInx].driverType;
        rstrcpy (fileOpendirInp.addr.hostAddr,  
	  dataObjInfo->rescInfo->rescLoc, NAME_LEN);
        dirFd = rsFileOpendir (rsComm, &fileOpendirInp);
        if (dirFd < 0) {
           rodsLog (LOG_ERROR,
              "specCollOpendir: rsFileOpendir for %s error, status = %d",
              dataObjInfo->filePath, dirFd);
	}
    } else {
	/* XXXXXX need to do the same for bundle */
	dirFd = 0;
    }
    return (dirFd);
}
#endif
