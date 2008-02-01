#include "rodsPath.h"
#include "rodsErrorTable.h"
#include "rodsLog.h"
#include "lsUtil.h"
#include "miscUtil.h"

int
lsUtil (rcComm_t *conn, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs,
rodsPathInp_t *rodsPathInp)
{
    int i;
    int status; 
    int savedStatus = 0;
    genQueryInp_t genQueryInp;


    if (rodsPathInp == NULL) {
	return (USER__NULL_INPUT_ERR);
    }

    initCondForLs (myRodsEnv, myRodsArgs, &genQueryInp);

    for (i = 0; i < rodsPathInp->numSrc; i++) {
	if (rodsPathInp->srcPath[i].objType == UNKNOWN_OBJ_T || 
	  rodsPathInp->srcPath[i].objState == UNKNOWN_ST) {
	    getRodsObjType (conn, &rodsPathInp->srcPath[i]);
	    if (rodsPathInp->srcPath[i].objState == NOT_EXIST_ST) {
                rodsLog (LOG_ERROR,
                  "lsUtil: srcPath %s does not exist",
                  rodsPathInp->srcPath[i].outPath);
		savedStatus = USER_INPUT_PATH_ERR;
		continue;
	    }
	}

	if (rodsPathInp->srcPath[i].objType == DATA_OBJ_T) {
	    status = lsDataObjUtil (conn, &rodsPathInp->srcPath[i], 
	     myRodsEnv, myRodsArgs, &genQueryInp);
	} else if (rodsPathInp->srcPath[i].objType ==  COLL_OBJ_T) {
	    status = lsCollUtil (conn, &rodsPathInp->srcPath[i],
              myRodsEnv, myRodsArgs);
	} else {
	    /* should not be here */
	    rodsLog (LOG_ERROR,
	     "lsUtil: invalid ls objType %d for %s", 
	     rodsPathInp->srcPath[i].objType, rodsPathInp->srcPath[i].outPath);
	    return (USER_INPUT_PATH_ERR);
	}
	/* XXXX may need to return a global status */
	if (status < 0 && status != CAT_NO_ROWS_FOUND && 
	  status != SYS_SPEC_COLL_OBJ_NOT_EXIST) {
	    rodsLogError (LOG_ERROR, status,
             "lsUtil: ls error for %s, status = %d", 
	      rodsPathInp->srcPath[i].outPath, status);
	    savedStatus = status;
	} 
    }
    if (savedStatus < 0) {
        return (savedStatus);
    } else if (status == CAT_NO_ROWS_FOUND || 
      status == SYS_SPEC_COLL_OBJ_NOT_EXIST) {
        return (0);
    } else {
        return (status);
    }
}

int
lsDataObjUtil (rcComm_t *conn, rodsPath_t *srcPath, 
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
genQueryInp_t *genQueryInp)
{
    int status = 0;
 

    if (rodsArgs->longOption == True) {
	if (srcPath->rodsObjStat != NULL &&
	  srcPath->rodsObjStat->specColl != NULL) {
	    lsSpecDataObjUtilLong (conn, srcPath,
	      myRodsEnv, rodsArgs);
	} else {
            lsDataObjUtilLong (conn, srcPath->outPath, myRodsEnv, rodsArgs, 
	      genQueryInp);
	}
    } else {
	printLsStrShort (srcPath->outPath);
        if (rodsArgs->accessControl == True) {
            printDataAcl (conn, srcPath->dataId);
        }
    }

    if (srcPath == NULL) {
       rodsLog (LOG_ERROR,
          "lsDataObjUtil: NULL srcPath input");
        return (USER__NULL_INPUT_ERR);
    }

    return (status);
}

int
printLsStrShort (char *srcPath)
{
    char srcElement[MAX_NAME_LEN];

    getLastPathElement (srcPath, srcElement);

    printf ("  %s\n", srcPath);

    return 0;
}

int
lsDataObjUtilLong (rcComm_t *conn, char *srcPath, rodsEnv *myRodsEnv, 
rodsArguments_t *rodsArgs, genQueryInp_t *genQueryInp)
{
    int status;
    genQueryOut_t *genQueryOut = NULL;
    char myColl[MAX_NAME_LEN], myData[MAX_NAME_LEN];
    char condStr[MAX_NAME_LEN];

    setQueryInpForLong (rodsArgs, genQueryInp);

    memset (myColl, 0, MAX_NAME_LEN);
    memset (myData, 0, MAX_NAME_LEN);

    if ((status = splitPathByKey (
      srcPath, myColl, myData, '/')) < 0) {
        rodsLogError (LOG_ERROR, status,
          "setQueryInpForLong: splitPathByKey for %s error, status = %d",
          srcPath, status);
        return (status);
    }

    snprintf (condStr, MAX_NAME_LEN, "='%s'", myColl);
    addInxVal (&genQueryInp->sqlCondInp, COL_COLL_NAME, condStr);
    snprintf (condStr, MAX_NAME_LEN, "='%s'", myData);
    addInxVal (&genQueryInp->sqlCondInp, COL_DATA_NAME, condStr);

    status =  rcGenQuery (conn, genQueryInp, &genQueryOut);

    if (status < 0) {
	if (status == CAT_NO_ROWS_FOUND) {
	    rodsLog (LOG_ERROR, "%s does not exist", srcPath);
	} else {
            rodsLogError (LOG_ERROR, status,
	      "lsDataObjUtilLong: rcGenQuery error for %s", srcPath);
	}
	return (status);
    }
    printLsLong (conn, rodsArgs, genQueryOut);

    return (0);
}

int
printLsLong (rcComm_t *conn, rodsArguments_t *rodsArgs, 
genQueryOut_t *genQueryOut)
{
    int i;
    sqlResult_t *dataName, *replNum, *dataSize, *rescName, 
      *replStatus, *dataModify, *dataOwnerName, *dataId;
    sqlResult_t *chksumStr, *dataPath;
    char *tmpDataName, *tmpReplNum, *tmpDataSize,  *tmpRescName,
      *tmpReplStatus,  *tmpDataModify, *tmpDataOwnerName, *tmpDataId;
    char *tmpChksumStr, *tmpDataPath;

   if (genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    if (rodsArgs->veryLongOption == True) {
        if ((chksumStr = getSqlResultByInx (genQueryOut, COL_D_DATA_CHECKSUM)) 
	  == NULL) {
            rodsLog (LOG_ERROR,
              "printLsLong: getSqlResultByInx for COL_D_DATA_CHECKSUM failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }

        if ((dataPath = getSqlResultByInx (genQueryOut, COL_D_DATA_PATH)) 
          == NULL) {
            rodsLog (LOG_ERROR,
              "printLsLong: getSqlResultByInx for COL_D_DATA_PATH failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }
    }

    if ((dataId = getSqlResultByInx (genQueryOut, COL_D_DATA_ID)) == NULL) {
        rodsLog (LOG_ERROR,
          "printLsLong: getSqlResultByInx for COL_D_DATA_ID failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataName = getSqlResultByInx (genQueryOut, COL_DATA_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
          "printLsLong: getSqlResultByInx for COL_DATA_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((replNum = getSqlResultByInx (genQueryOut, COL_DATA_REPL_NUM)) == 
      NULL) {
        rodsLog (LOG_ERROR,
          "printLsLong: getSqlResultByInx for COL_DATA_REPL_NUM failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataSize = getSqlResultByInx (genQueryOut, COL_DATA_SIZE)) == NULL) {
        rodsLog (LOG_ERROR,
          "printLsLong: getSqlResultByInx for COL_DATA_SIZE failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((rescName = getSqlResultByInx (genQueryOut, COL_D_RESC_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
          "printLsLong: getSqlResultByInx for COL_D_RESC_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((replStatus = getSqlResultByInx (genQueryOut, COL_D_REPL_STATUS)) == 
     NULL) {
        rodsLog (LOG_ERROR,
          "printLsLong: getSqlResultByInx for COL_D_REPL_STATUS failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataModify = getSqlResultByInx (genQueryOut, COL_D_MODIFY_TIME)) ==
     NULL) {
        rodsLog (LOG_ERROR,
          "printLsLong: getSqlResultByInx for COL_D_MODIFY_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataOwnerName = getSqlResultByInx (genQueryOut, COL_D_OWNER_NAME)) ==
     NULL) {
        rodsLog (LOG_ERROR,
          "printLsLong: getSqlResultByInx for COL_D_OWNER_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

   for (i = 0;i < genQueryOut->rowCnt; i++) {
	int intReplStatus;
	char localTimeModify[20];

        tmpDataName = &dataName->value[dataName->len * i];
        tmpReplNum = &replNum->value[replNum->len * i];
        tmpDataSize = &dataSize->value[dataSize->len * i];
        tmpRescName = &rescName->value[rescName->len * i];
        tmpDataOwnerName = &dataOwnerName->value[dataOwnerName->len * i];
        tmpReplStatus = &replStatus->value[replStatus->len * i];
	intReplStatus = atoi (tmpReplStatus); 
	if (intReplStatus == OLD_COPY) {
	    tmpReplStatus = " ";
	} else {
	    tmpReplStatus = "&";
        }
	     
        tmpDataModify = &dataModify->value[dataModify->len * i];
	getLocalTimeFromRodsTime(tmpDataModify, localTimeModify);

        printf (
         "  %-12.12s %3s %-20.20s %8.12s %16.16s %s %s\n",
         tmpDataOwnerName, tmpReplNum, tmpRescName, tmpDataSize, 
	 localTimeModify,
         tmpReplStatus, tmpDataName);
	if (rodsArgs->veryLongOption == True) {
	    tmpChksumStr = &chksumStr->value[chksumStr->len * i];
	    tmpDataPath = &dataPath->value[dataPath->len * i];
	    printf ("    %s    %s\n", tmpChksumStr, tmpDataPath);
	}

	if (rodsArgs->accessControl == True) {
            tmpDataId = &dataId->value[dataId->len * i];
	    printDataAcl (conn, tmpDataId);
	}
    }

    return (0);
}

int
lsSpecDataObjUtilLong (rcComm_t *conn, rodsPath_t *srcPath, rodsEnv *myRodsEnv,
rodsArguments_t *rodsArgs)
{
    char sizeStr[NAME_LEN];
    int status;
    rodsObjStat_t *rodsObjStat = srcPath->rodsObjStat;

    snprintf (sizeStr, NAME_LEN, "%lld", rodsObjStat->objSize);
    status = printSpecLsLong (srcPath->outPath, rodsObjStat->ownerName,
     sizeStr, rodsObjStat->modifyTime, rodsObjStat->specColl,
     rodsArgs);

    return (status);
}


int
printLsShort (rcComm_t *conn,  rodsArguments_t *rodsArgs, 
genQueryOut_t *genQueryOut)
{
    int i;

    sqlResult_t *dataName, *dataId;
    char *tmpDataName, *tmpDataId;

   if (genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    if ((dataId = getSqlResultByInx (genQueryOut, COL_D_DATA_ID)) == NULL) {
        rodsLog (LOG_ERROR,
          "printLsLong: getSqlResultByInx for COL_D_DATA_ID failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataName = getSqlResultByInx (genQueryOut, COL_DATA_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
          "printLsLong: getSqlResultByInx for COL_DATA_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    for (i = 0;i < genQueryOut->rowCnt; i++) {
        tmpDataName = &dataName->value[dataName->len * i];
	printLsStrShort (tmpDataName);

        if (rodsArgs->accessControl == True) {
            tmpDataId = &dataId->value[dataId->len * i];
            printDataAcl (conn, tmpDataId);
        }
    }

    return (0);
}

int
initCondForLs (rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
genQueryInp_t *genQueryInp)
{
    if (genQueryInp == NULL) {
       rodsLog (LOG_ERROR,
          "initCondForLs: NULL genQueryInp input");
        return (USER__NULL_INPUT_ERR);
    }

    memset (genQueryInp, 0, sizeof (genQueryInp_t));

    if (rodsArgs == NULL) {
	return (0);
    }

    return (0);
}

int
lsCollUtil (rcComm_t *conn, rodsPath_t *srcPath, rodsEnv *myRodsEnv, 
rodsArguments_t *rodsArgs)
{
    int savedStatus = 0;
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    int continueInx;
    char *srcColl;
    int status;

    if (srcPath == NULL) {
       rodsLog (LOG_ERROR,
          "lsCollUtil: NULL srcPath input");
        return (USER__NULL_INPUT_ERR);
    }

    srcColl = srcPath->outPath;

    /* print this collection */
    printf ("%s:\n", srcColl);

    if (srcPath->rodsObjStat != NULL &&
      srcPath->rodsObjStat->specColl != NULL) {
	status = lsSpecCollUtil (conn, srcPath, myRodsEnv, rodsArgs);
#if 0	/* XXXXX STRUCT_FILE_COLL type collection does not contain normal 
         * files for now */
	if (srcPath->rodsObjStat->specColl->class == MOUNTED_COLL) {
	    /* for STRUCT_FILE_COLL, we also want to list normal files */
	    return (status);
	}
#else
	return (status);
#endif
    }


#if 0	/* moved */
    srcColl = srcPath->outPath;
    /* print this collection */
    printf ("%s:\n", srcColl);
#endif

    /* get the files in this collection */

    status = queryDataObjInColl (conn, srcColl, rodsArgs, &genQueryInp,
      &genQueryOut);

    if (status < 0 && status != CAT_NO_ROWS_FOUND) {
        rodsLogError (LOG_ERROR, status,
          "lsCollUtil: queryDataObjInColl error for %s", srcColl);
    }

    while (status >= 0) {
        if (rodsArgs->longOption == True) {
            printLsLong (conn, rodsArgs, genQueryOut);
        } else {
            printLsShort (conn, rodsArgs, genQueryOut);
        }


	continueInx = genQueryOut->continueInx;

	freeGenQueryOut (&genQueryOut);

	if (continueInx > 0) {
	    /* More to come */
	    genQueryInp.continueInx = continueInx;
            status =  rcGenQuery (conn, &genQueryInp, &genQueryOut);
	} else {
	    break;
	}
    }

    /* query all sub collections in srcColl and the mk the required
     * subdirectories */

    status = queryCollInColl (conn, srcColl, rodsArgs, &genQueryInp,
      &genQueryOut);

    if (status < 0 && status != CAT_NO_ROWS_FOUND) {
        rodsLogError (LOG_ERROR, status,
          "lsCollUtil: queryCollInColl error for %s", srcColl);
    }

    while (status >= 0) {
        printLsColl (conn, myRodsEnv, rodsArgs, genQueryOut);

        continueInx = genQueryOut->continueInx;

        freeGenQueryOut (&genQueryOut);

        if (continueInx > 0) {
            /* More to come */
            genQueryInp.continueInx = continueInx;
            status =  rcGenQuery (conn, &genQueryInp, &genQueryOut);
        } else {
            break;
        }
    }

    clearGenQueryInp (&genQueryInp);

    if (savedStatus < 0) {
	return (savedStatus);
    } else if (status == CAT_NO_ROWS_FOUND) {
	return (0);
    } else {
        return (status);
    }
}

int
lsSpecCollUtil (rcComm_t *conn, rodsPath_t *srcPath, rodsEnv *myRodsEnv,
rodsArguments_t *rodsArgs)
{
    dataObjInp_t dataObjInp;
    genQueryOut_t *genQueryOut = NULL;
    int continueInx;
    int status;


    memset (&dataObjInp, 0, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, srcPath->outPath, MAX_NAME_LEN);
    dataObjInp.specColl = srcPath->rodsObjStat->specColl;
    /* do the data first */
    addKeyVal (&dataObjInp.condInput, SEL_OBJ_TYPE_KW, "dataObj");

    status = rcQuerySpecColl (conn, &dataObjInp, &genQueryOut);

    while (status >= 0) {
        printSpecLs (conn, rodsArgs, srcPath, genQueryOut);

        continueInx = genQueryOut->continueInx;
        freeGenQueryOut (&genQueryOut);

        if (continueInx > 0) {
            /* More to come */
	    dataObjInp.openFlags = continueInx;
            status = rcQuerySpecColl (conn, &dataObjInp, &genQueryOut);
        } else {
            break;
        }
    }

    if (status < 0 && status != CAT_NO_ROWS_FOUND && 
      status != SYS_SPEC_COLL_OBJ_NOT_EXIST) {
	rodsLogError (LOG_ERROR, status,
	  "lsSpecCollUtil: rcQuerySpecColl error for %s", dataObjInp.objPath);
	return (status);
    }

    /* do the collection second */
    addKeyVal (&dataObjInp.condInput, SEL_OBJ_TYPE_KW, "collection");

    status = rcQuerySpecColl (conn, &dataObjInp, &genQueryOut);

    while (status >= 0) {
        printSpecLs (conn, rodsArgs, srcPath, genQueryOut);

        continueInx = genQueryOut->continueInx;
        freeGenQueryOut (&genQueryOut);

        if (continueInx > 0) {
            /* More to come */
            dataObjInp.openFlags = continueInx;
            status = rcQuerySpecColl (conn, &dataObjInp, &genQueryOut);
        } else {
            break;
        }
    }

    return (status);
}
    
int
printSpecLs (rcComm_t *conn, rodsArguments_t *rodsArgs, rodsPath_t *srcPath, 
genQueryOut_t *genQueryOut)
{
    int i, status;
    sqlResult_t *dataName, *collName, *dataSize, *createTime, *modifyTime;
    char *tmpDataName, *tmpCollName, *tmpCreateTime, *tmpModifyTime, 
      *tmpDataSize;
    specColl_t *specColl = srcPath->rodsObjStat->specColl;

   if (genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    if (genQueryOut->rowCnt <= 0) {
	return 0;
    }

    if ((dataName = getSqlResultByInx (genQueryOut, COL_DATA_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
          "printSpecLs: getSqlResultByInx for COL_DATA_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((collName = getSqlResultByInx (genQueryOut, COL_COLL_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
          "printSpecLs: getSqlResultByInx for COL_COLL_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataSize = getSqlResultByInx (genQueryOut, COL_DATA_SIZE)) == NULL) {
        rodsLog (LOG_ERROR,
          "printSpecLs: getSqlResultByInx for COL_DATA_SIZE failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((createTime = getSqlResultByInx (genQueryOut, COL_COLL_CREATE_TIME)) 
      == NULL) {
        rodsLog (LOG_ERROR,
          "printSpecLs: getSqlResultByInx for COL_COLL_CREATE_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((modifyTime = getSqlResultByInx (genQueryOut, COL_COLL_MODIFY_TIME)) 
      == NULL) {
        rodsLog (LOG_ERROR,
          "printSpecLs: getSqlResultByInx for COL_COLL_MODIFY_TIME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    for (i = 0;i < genQueryOut->rowCnt; i++) {
        tmpDataName = &dataName->value[dataName->len * i];
        tmpCollName = &collName->value[collName->len * i];
        tmpCreateTime = &createTime->value[createTime->len * i];
        tmpModifyTime = &modifyTime->value[modifyTime->len * i];
        tmpDataSize = &dataSize->value[dataSize->len * i];

	if (tmpDataName[0] != '\0') {
	    /* a file */
	    if (rodsArgs->longOption == True) {
		char objPath[MAX_NAME_LEN];

		snprintf (objPath, MAX_NAME_LEN, "%s/%s",
		  tmpCollName, tmpDataName);
		printSpecLsLong (objPath, srcPath->rodsObjStat->ownerName,
		  tmpDataSize, tmpModifyTime, specColl, rodsArgs);
	    } else {
        	printLsStrShort (tmpDataName);
	    }
	} else {
	    /* a collection */
	    rodsPath_t tmpPath;
	    char objType[NAME_LEN];

	    status = getSpecCollTypeStr (specColl, objType);
	    if (status < 0) return (status);
	    if (strcmp (tmpCollName, specColl->collection) == 0) {
		printf ("  C- %s  %s", tmpCollName, objType);
	    } else {
                printf ("  C- %s  %-5.5s", tmpCollName, objType);
	    }
	
            if (rodsArgs->veryLongOption == True) {
		if (specColl->class == MOUNTED_COLL) {
                    printf ("  %s  %s\n", specColl->phyPath, 
		      specColl->resource);
		} else {
                    printf ("  %s\n", specColl->objPath);
		}
            } else {
                printf ("\n");
            }

	    tmpPath = *srcPath;
            snprintf (tmpPath.outPath, MAX_NAME_LEN, "%s",
              tmpCollName);

	    if (rodsArgs->recursive == True) {
	        status = lsSpecCollUtil (conn, &tmpPath, NULL, rodsArgs);
	    }
	}
    }

    return (0);
}

int
printSpecLsLong (char *objPath, char *ownerName, char *objSize, 
char *modifyTime, specColl_t *specColl, rodsArguments_t *rodsArgs)
{
    char localTimeModify[20];
    char srcElement[MAX_NAME_LEN];
    char phySubPath[MAX_NAME_LEN];
    int status;
    char objType[NAME_LEN];

    getLastPathElement (objPath, srcElement);
    getLocalTimeFromRodsTime(modifyTime, localTimeModify);
    if (getSpecCollTypeStr (specColl, objType) < 0) {
	rstrcpy (objType, "UNKNOWN_COLL_TYPE", NAME_LEN);
    }
    printf ("  %-12.12s %-5.5s %-18.18s %8.12s %16.16s   %s\n",
     ownerName, objType, specColl->resource, objSize,
      localTimeModify, srcElement);
    if (rodsArgs->veryLongOption == True) {
        if (specColl->class == MOUNTED_COLL) {
            status = getMountedSubPhyPath (
              specColl->collection,
              specColl->phyPath, objPath, phySubPath);
            if (status < 0) {
                return (status);
            }
            printf ("        %s\n", phySubPath);
        } else {
            printf ("        %s\n", specColl->objPath);
        }
    }
    return (0);
}

int
printLsColl (rcComm_t *conn, rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
genQueryOut_t *genQueryOut)
{
    int i, status;
    sqlResult_t *collName, *collType, *collInfo1, *collInfo2, *ownerName;
    char *tmpCollName, *tmpCollType, *tmpCollInfo1, *tmpCollInfo2,
      *tmpOwnerName;
    int savedStatus;

    if (genQueryOut == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    if ((collName = getSqlResultByInx (genQueryOut, COL_COLL_NAME))
      == NULL) {
        rodsLog (LOG_ERROR,
          "printLsColl: getSqlResultByInx for COL_COLL_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
    } else if ((collType = getSqlResultByInx (genQueryOut,
      COL_COLL_TYPE)) == NULL) {
        rodsLog (LOG_ERROR,
         "printLsColl:getSqlResultByInx for COL_COLL_TYPE failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else if ((collInfo1 = getSqlResultByInx (genQueryOut,
      COL_COLL_INFO1)) == NULL) {
        rodsLog (LOG_ERROR,
         "printLsColl:getSqlResultByInx for COL_COLL_INFO1 failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else if ((collInfo2 = getSqlResultByInx (genQueryOut,
      COL_COLL_INFO2)) == NULL) {
        rodsLog (LOG_ERROR,
         "printLsColl:getSqlResultByInx for COL_COLL_INFO2 failed");
        return (UNMATCHED_KEY_OR_INDEX);
    } else if ((ownerName = getSqlResultByInx (genQueryOut,
      COL_COLL_OWNER_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
         "printLsColl:getSqlResultByInx for COL_COLL_OWNER_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    for (i = 0; i < genQueryOut->rowCnt; i++) {

        tmpCollName = &collName->value[collName->len * i];
        tmpCollType = &collType->value[collType->len * i];
        tmpCollInfo1 = &collInfo1->value[collInfo1->len * i];
        tmpCollInfo2 = &collInfo2->value[collInfo2->len * i];
        tmpOwnerName = &ownerName->value[ownerName->len * i];

	if (tmpCollType[0] == '\0') {
	    printf ("  C- %s\n", tmpCollName);
	} else {
	    if (rodsArgs->veryLongOption == True) {
                printf ("  C- %s  %s  %s  %s\n", tmpCollName, tmpCollType,
		  tmpCollInfo1, tmpCollInfo2);
	    } else {
                printf ("  C- %s  %s\n", tmpCollName, tmpCollType);
	    }
	}
	if (rodsArgs->recursive == True) {
	    rodsPath_t tmpPath;
	    memset (&tmpPath, 0, sizeof (tmpPath));
	    rstrcpy (tmpPath.outPath, tmpCollName, MAX_NAME_LEN);
	    if (tmpCollType[0] == '\0') {
	        status = lsCollUtil (conn, &tmpPath, myRodsEnv, rodsArgs);
	    } else {
                specColl_t specColl;
		rodsObjStat_t rodsObjStat;

		tmpPath.rodsObjStat = &rodsObjStat;
		memset (tmpPath.rodsObjStat, 0, sizeof (rodsObjStat_t));
                tmpPath.rodsObjStat->specColl = &specColl;
                memset (&specColl, 0, sizeof (specColl_t));

		rstrcpy (rodsObjStat.ownerName, tmpOwnerName, NAME_LEN);
		status = resolveSpecCollType (tmpCollType, tmpCollName,
		  tmpCollInfo1, tmpCollInfo2, &specColl);
		if (status < 0) return (status);
#if 0
                rstrcpy (specColl.objType, tmpCollType, NAME_LEN);
                rstrcpy (specColl.collection, tmpCollName, MAX_NAME_LEN);
                rstrcpy (specColl.collInfo1, tmpCollInfo1, MAX_NAME_LEN);
                rstrcpy (specColl.collInfo2, tmpCollInfo2, MAX_NAME_LEN);
#endif

                status = lsSpecCollUtil (conn, &tmpPath, myRodsEnv, rodsArgs);
	    }

	    if (status < 0 && status != CAT_NO_ROWS_FOUND) {
		rodsLogError (LOG_ERROR, status,
		 "printLsColl: lsCollUtil error for %s", tmpCollName);
		savedStatus = status;
	    }
    	}
    }

    if (savedStatus < 0) {
        return (savedStatus);
    } else if (status == CAT_NO_ROWS_FOUND) {
        return (0);
    } else {
        return (status);
    }
}

int
printDataAcl (rcComm_t *conn, char *dataId)
{
    genQueryOut_t *genQueryOut = NULL;
    int status;
    int i;
    sqlResult_t *userName, *dataAccess;
    char *userNameStr, *dataAccessStr;

    status = queryDataObjAcl (conn, dataId, &genQueryOut);

    printf ("        ACL - ");

    if (status < 0) {
	printf ("\n");
        return (status);
    }

    if ((userName = getSqlResultByInx (genQueryOut, COL_USER_NAME)) == NULL) {
        rodsLog (LOG_ERROR,
          "printDataAcl: getSqlResultByInx for COL_USER_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    if ((dataAccess = getSqlResultByInx (genQueryOut, COL_DATA_ACCESS_NAME)) 
      == NULL) {
        rodsLog (LOG_ERROR,
          "printDataAcl: getSqlResultByInx for COL_DATA_ACCESS_NAME failed");
        return (UNMATCHED_KEY_OR_INDEX);
    }

    for (i = 0; i < genQueryOut->rowCnt; i++) {
	userNameStr = &userName->value[userName->len * i];
	dataAccessStr = &dataAccess->value[dataAccess->len * i];
	printf ("%s:%s   ", userNameStr, dataAccessStr);
    }
    
    printf ("\n");

    freeGenQueryOut (&genQueryOut);

    return (status);
}

void 
printCollOrDir (char *myName, objType_t myType, rodsArguments_t *rodsArgs,
specColl_t *specColl)
{
    char *typeStr;

    if (rodsArgs->verbose == False) return;

    if (myType == COLL_OBJ_T)
	typeStr = "C";
    else
        typeStr = "D";

    if (specColl != NULL) {
        char objType[NAME_LEN];
	int status;
        status = getSpecCollTypeStr (specColl, objType);
        if (status < 0) objType[0] = '\0';
        fprintf (stdout, "%s- %s    %-5.5s :\n", typeStr, myName, objType);
    } else {
        fprintf (stdout, "%s- %s :\n", typeStr, myName);
    }
}

