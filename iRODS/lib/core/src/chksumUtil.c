/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#ifndef windows_platform
#include <sys/time.h>
#endif
#include "rodsPath.h"
#include "rodsErrorTable.h"
#include "miscUtil.h"
#include "rodsLog.h"
#include "chksumUtil.h"

int
chksumUtil (rcComm_t *conn, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs,
rodsPathInp_t *rodsPathInp)
{
    int i;
    int status; 
    int savedStatus = 0;
    collInp_t collInp;
    dataObjInp_t dataObjInp;


    if (rodsPathInp == NULL) {
	return (USER__NULL_INPUT_ERR);
    }

    status = initCondForChksum (myRodsEnv, myRodsArgs, &dataObjInp, &collInp);

    if (status < 0) {
	return (status);
    }

    for (i = 0; i < rodsPathInp->numSrc; i++) {
	if (rodsPathInp->srcPath[i].objType == UNKNOWN_OBJ_T) {
	    getRodsObjType (conn, &rodsPathInp->srcPath[i]);
	    if (rodsPathInp->srcPath[i].objState == NOT_EXIST_ST) {
                rodsLog (LOG_ERROR,
                  "chksumUtil: srcPath %s does not exist",
                  rodsPathInp->srcPath[i].outPath);
		savedStatus = USER_INPUT_PATH_ERR;
		continue;
	    }
	}

	if (rodsPathInp->srcPath[i].objType == DATA_OBJ_T) {
	    status = chksumDataObjUtil (conn, rodsPathInp->srcPath[i].outPath, 
	     myRodsEnv, myRodsArgs, &dataObjInp);
	} else if (rodsPathInp->srcPath[i].objType ==  COLL_OBJ_T) {
	    status = chksumCollUtil (conn, rodsPathInp->srcPath[i].outPath,
              myRodsEnv, myRodsArgs, &dataObjInp, &collInp);
	} else {
	    /* should not be here */
	    rodsLog (LOG_ERROR,
	     "chksumUtil: invalid chksum objType %d for %s", 
	     rodsPathInp->srcPath[i].objType, rodsPathInp->srcPath[i].outPath);
	    return (USER_INPUT_PATH_ERR);
	}
	/* XXXX may need to return a global status */
	if (status < 0) {
	    rodsLogError (LOG_ERROR, status,
             "chksumUtil: chksum error for %s, status = %d", 
	      rodsPathInp->srcPath[i].outPath, status);
	    savedStatus = status;
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
chksumDataObjUtil (rcComm_t *conn, char *srcPath, 
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjInp_t *dataObjInp)
{
    int status;
    struct timeval startTime, endTime;
    char *chksumStr = NULL;
    char myDir[MAX_NAME_LEN], myFile[MAX_NAME_LEN];
 
    if (srcPath == NULL) {
       rodsLog (LOG_ERROR,
          "chksumDataObjUtil: NULL srcPath input");
        return (USER__NULL_INPUT_ERR);
    }

    if (rodsArgs->verbose == True) {
        (void) gettimeofday(&startTime, (struct timezone *)0);
    }

    rstrcpy (dataObjInp->objPath, srcPath, MAX_NAME_LEN);

    status = rcDataObjChksum (conn, dataObjInp, &chksumStr);

    if (status < 0) {
	rodsLogError (LOG_ERROR, status,
	 "chksumDataObjUtil: rcDataObjChksum error for %s",
	 dataObjInp->objPath);
	return status;
    }

    splitPathByKey (dataObjInp->objPath, myDir, myFile, '/');
    printf ("    %-30.30s    %s\n", myFile, chksumStr);
    free (chksumStr);     
    if (rodsArgs->verbose == True) {
        (void) gettimeofday(&endTime, (struct timezone *)0);
        printTiming (conn, dataObjInp->objPath, -1, NULL,
         &startTime, &endTime);
    }

    return (status);
}

int
initCondForChksum (rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
dataObjInp_t *dataObjInp, collInp_t *collInp)
{
    if (dataObjInp == NULL) {
       rodsLog (LOG_ERROR,
          "initCondForChksum: NULL dataObjInp input");
        return (USER__NULL_INPUT_ERR);
    }

    memset (dataObjInp, 0, sizeof (dataObjInp_t));
    memset (collInp, 0, sizeof (collInp_t));

    if (rodsArgs == NULL) {
	return (0);
    }

    if (rodsArgs->force == True && rodsArgs->verifyChecksum) {
	rodsLog (LOG_ERROR,
	  "initCondForChksum: the 'K' and 'f' option cannot be used together");
	return (USER_OPTION_INPUT_ERR);
    }
	
    if (rodsArgs->all == True && rodsArgs->replNum == True) {
        rodsLog (LOG_ERROR,
          "initCondForChksum: the 'N' and 'a' option cannot be used together"); 
        return (USER_OPTION_INPUT_ERR);
    }

    if (rodsArgs->force == True) { 
        addKeyVal (&dataObjInp->condInput, FORCE_CHKSUM_KW, "");
        addKeyVal (&collInp->condInput, FORCE_CHKSUM_KW, "");
    }

    if (rodsArgs->all == True) {
        addKeyVal (&dataObjInp->condInput, CHKSUM_ALL_KW, "");
        addKeyVal (&collInp->condInput, CHKSUM_ALL_KW, "");
    }

    if (rodsArgs->verifyChecksum == True) {
        addKeyVal (&dataObjInp->condInput, VERIFY_CHKSUM_KW, "");
        addKeyVal (&collInp->condInput, VERIFY_CHKSUM_KW, "");
    }

    if (rodsArgs->replNum == True) {
        addKeyVal (&dataObjInp->condInput, REPL_NUM_KW, 
	  rodsArgs->replNumValue);
    }

    /* XXXXX need to add -u register cond */

    dataObjInp->openFlags = O_RDONLY;

    return (0);
}

int
chksumCollUtil (rcComm_t *conn, char *srcColl, rodsEnv *myRodsEnv, 
rodsArguments_t *rodsArgs, dataObjInp_t *dataObjInp, collInp_t *collInp)
{
    int status;
    int savedStatus = 0;
    int collLen;
    char srcChildPath[MAX_NAME_LEN];
#if 0
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    int rowInx;
    dataObjSqlResult_t dataObjSqlResult;
    dataObjMetaInfo_t dataObjMetaInfo;
#else
    collHandle_t collHandle;
    collEnt_t collEnt;
#endif

    if (srcColl == NULL) {
       rodsLog (LOG_ERROR,
          "chksumCollUtil: NULL srcColl input");
        return (USER__NULL_INPUT_ERR);
    }

    fprintf (stdout, "C- %s:\n", srcColl);

    collLen = strlen (srcColl);

#if 0
    /* Now get all the files */

    memset (&genQueryInp, 0, sizeof (genQueryInp));
    status = queryDataObjInColl (conn, srcColl, rodsArgs, &genQueryInp,
      &genQueryOut);

    if (status != CAT_NO_ROWS_FOUND && rodsArgs->recursive != True) {
       rodsLog (LOG_ERROR,
	 "chksumCollUtil: ichksum -r must be used for collection %s",
		srcColl);
       return (USER_INPUT_OPTION_ERR);
    }

    if (status >= 0) {
        status = genQueryOutToDataObjRes (&genQueryOut, &dataObjSqlResult);
    }

    rowInx = 0;
    while (status >= 0 &&
      (status = getNextDataObjMetaInfo (conn, dataObjInp, &genQueryInp,
      &dataObjSqlResult, &rowInx, &dataObjMetaInfo)) >= 0) {
 

        snprintf (srcChildPath, MAX_NAME_LEN, "%s/%s",
          dataObjMetaInfo.collName, dataObjMetaInfo.dataName);

        status = chksumDataObjUtil (conn, srcChildPath, myRodsEnv, rodsArgs, 
	  dataObjInp);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "chksumCollUtil: chksumDataObjUtil failed for %s. status = %d",
              srcChildPath, status);
            /* need to set global error here */
            savedStatus = status;
	    status = 0;
        }
    }

    clearGenQueryInp (&genQueryInp);
#else
    status = rclOpenCollection (conn, srcColl, RECUR_QUERY_FG,
      &collHandle);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "getCollUtil: rclOpenCollection of %s error. status = %d",
          srcColl, status);
        return status;
    }
    while ((status = rclReadCollection (conn, &collHandle, &collEnt)) >= 0) {
        if (collEnt.objType == DATA_OBJ_T) {
            snprintf (srcChildPath, MAX_NAME_LEN, "%s/%s",
              collEnt.collName, collEnt.dataName);

            status = chksumDataObjUtil (conn, srcChildPath, myRodsEnv, 
	      rodsArgs, dataObjInp);
            if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                  "chksumCollUtil: chksumDataObjUtil failed for %s. status = %d",
                  srcChildPath, status);
                /* need to set global error here */
                savedStatus = status;
                status = 0;
            }
	}
    }
    rclCloseCollection (&collHandle);
#endif
    if (savedStatus < 0) {
	return (savedStatus);
    } else if (status == CAT_NO_ROWS_FOUND) {
	return (0);
    } else {
        return (status);
    }
}

