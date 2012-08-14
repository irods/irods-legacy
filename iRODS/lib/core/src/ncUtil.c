/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#ifndef windows_platform
#include <sys/time.h>
#endif
#include "rodsPath.h"
#include "rodsErrorTable.h"
#include "rodsLog.h"
#include "miscUtil.h"
#include "ncUtil.h"

int
ncUtil (rcComm_t *conn, rodsEnv *myRodsEnv, rodsArguments_t *myRodsArgs,
rodsPathInp_t *rodsPathInp)
{
    int i;
    int status; 
    int savedStatus = 0;
    ncOpenInp_t ncOpenInp;


    if (rodsPathInp == NULL) {
	return (USER__NULL_INPUT_ERR);
    }

    initCondForNcOper (myRodsEnv, myRodsArgs, &ncOpenInp);

    for (i = 0; i < rodsPathInp->numSrc; i++) {
	if (rodsPathInp->srcPath[i].objType == UNKNOWN_OBJ_T) {
	    getRodsObjType (conn, &rodsPathInp->srcPath[i]);
	    if (rodsPathInp->srcPath[i].objState == NOT_EXIST_ST) {
                rodsLog (LOG_ERROR,
                  "ncUtil: srcPath %s does not exist",
                  rodsPathInp->srcPath[i].outPath);
		savedStatus = USER_INPUT_PATH_ERR;
		continue;
	    }
	}

	if (rodsPathInp->srcPath[i].objType == DATA_OBJ_T) {
            rmKeyVal (&ncOpenInp.condInput, TRANSLATED_PATH_KW);
	   status = ncOperDataObjUtil (conn, 
             rodsPathInp->srcPath[i].outPath, myRodsEnv, myRodsArgs, 
             &ncOpenInp);
	} else if (rodsPathInp->srcPath[i].objType ==  COLL_OBJ_T) {
            addKeyVal (&ncOpenInp.condInput, TRANSLATED_PATH_KW, "");
	    status = ncOperCollUtil (conn, rodsPathInp->srcPath[i].outPath,
              myRodsEnv, myRodsArgs, &ncOpenInp);
	} else {
	    /* should not be here */
	    rodsLog (LOG_ERROR,
	     "ncUtil: invalid nc objType %d for %s", 
	     rodsPathInp->srcPath[i].objType, rodsPathInp->srcPath[i].outPath);
	    return (USER_INPUT_PATH_ERR);
	}
	/* XXXX may need to return a global status */
	if (status < 0) {
	    rodsLogError (LOG_ERROR, status,
             "ncUtil: nc error for %s, status = %d", 
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
ncOperDataObjUtil (rcComm_t *conn, char *srcPath, 
rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
ncOpenInp_t *ncOpenInp)
{
    int status, status1;
    ncCloseInp_t ncCloseInp;
    ncInqInp_t ncInqInp;
    ncInqOut_t *ncInqOut = NULL;
    int ncid;

 
    if (srcPath == NULL) {
       rodsLog (LOG_ERROR,
          "ncOperDataObjUtil: NULL srcPath input");
        return (USER__NULL_INPUT_ERR);
    }

    rstrcpy (ncOpenInp->objPath, srcPath, MAX_NAME_LEN);

    status = rcNcOpen (conn, ncOpenInp, &ncid);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "ncOperDataObjUtil: rcNcOpen error for %s", ncOpenInp->objPath);
        return status;
    }

    /* do the general inq */
    bzero (&ncInqInp, sizeof (ncInqInp));
    ncInqInp.ncid = ncid;
    ncInqInp.paramType = NC_ALL_TYPE;
    ncInqInp.flags = NC_ALL_FLAG;

    status = rcNcInq (conn, &ncInqInp, &ncInqOut);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "ncOperDataObjUtil: rcNcInq error for %s", ncOpenInp->objPath);
        return status;
    }

    if (rodsArgs->option == False) {
        /* stdout */
        if (rodsArgs->header == True) {
            status = dumpNcHeader (conn, ncOpenInp->objPath, ncid, ncInqOut);
        }
        if (rodsArgs->dim == True) {
            status = dumpNcDimVar (conn, ncOpenInp->objPath, ncid, 
              rodsArgs->ascitime, ncInqOut);
        }
    } else {
        /* output is a NETCDF file */
        status = dumpNcInqOutToNcFile (conn, ncid, ncInqOut, 
          rodsArgs->optionString);
    }

    bzero (&ncCloseInp, sizeof (ncCloseInp_t));
    ncCloseInp.ncid = ncid;
    status1 = rcNcClose (conn, &ncCloseInp);
    if (status1 < 0) {
        rodsLogError (LOG_ERROR, status1,
          "ncOperDataObjUtil: rcNcClose error for %s", ncOpenInp->objPath);
        return status1;
    }
    return status;
}

int
initCondForNcOper (rodsEnv *myRodsEnv, rodsArguments_t *rodsArgs, 
ncOpenInp_t *ncOpenInp)
{

    if (ncOpenInp == NULL) {
       rodsLog (LOG_ERROR,
          "initCondForNcOper: NULL ncOpenInp input");
        return (USER__NULL_INPUT_ERR);
    }

    memset (ncOpenInp, 0, sizeof (ncOpenInp_t));

    if (rodsArgs == NULL) {
        return (0);
    }
    if ((rodsArgs->dim + rodsArgs->header + rodsArgs->var + rodsArgs->option)
      == False) {
        rodsArgs->dim = rodsArgs->header = True;
    }

    bzero (ncOpenInp, sizeof (ncOpenInp_t));
    ncOpenInp->mode = NC_NOWRITE;
    addKeyVal (&ncOpenInp->condInput, NO_STAGING_KW, "");

    return (0);
}

int
ncOperCollUtil (rcComm_t *conn, char *srcColl, rodsEnv *myRodsEnv, 
rodsArguments_t *rodsArgs, ncOpenInp_t *ncOpenInp)
{
    int status;
    int savedStatus = 0;
    collHandle_t collHandle;
    collEnt_t collEnt;
    char srcChildPath[MAX_NAME_LEN];

    if (srcColl == NULL) {
       rodsLog (LOG_ERROR,
          "ncOperCollUtil: NULL srcColl input");
        return (USER__NULL_INPUT_ERR);
    }

    if (rodsArgs->recursive != True) {
        rodsLog (LOG_ERROR,
        "ncOperCollUtil: -r option must be used for getting %s collection",
         srcColl);
        return (USER_INPUT_OPTION_ERR);
    }

    if (rodsArgs->verbose == True) {
        fprintf (stdout, "C- %s:\n", srcColl);
    }

    status = rclOpenCollection (conn, srcColl, 0, &collHandle);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "ncOperCollUtil: rclOpenCollection of %s error. status = %d",
          srcColl, status);
        return status;
    }
    if (collHandle.rodsObjStat->specColl != NULL &&
      collHandle.rodsObjStat->specColl->collClass != LINKED_COLL) {
        /* no reg for mounted coll */
        rclCloseCollection (&collHandle);
        return 0;
    }
    while ((status = rclReadCollection (conn, &collHandle, &collEnt)) >= 0) {
        if (collEnt.objType == DATA_OBJ_T) {
            snprintf (srcChildPath, MAX_NAME_LEN, "%s/%s",
              collEnt.collName, collEnt.dataName);

            status = ncOperDataObjUtil (conn, srcChildPath,
             myRodsEnv, rodsArgs, ncOpenInp);
            if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                  "ncOperCollUtil: ncOperDataObjUtil failed for %s. status = %d",
                  srcChildPath, status);
                /* need to set global error here */
                savedStatus = status;
                status = 0;
            }
        } else if (collEnt.objType == COLL_OBJ_T) {
            ncOpenInp_t childNcOpen;
            childNcOpen = *ncOpenInp;
            if (collEnt.specColl.collClass != NO_SPEC_COLL) continue;
            status = ncOperCollUtil (conn, collEnt.collName, myRodsEnv,
              rodsArgs, &childNcOpen);
            if (status < 0 && status != CAT_NO_ROWS_FOUND) {
                savedStatus = status;
            }
	}
    }
    rclCloseCollection (&collHandle);

    if (savedStatus < 0) {
	return (savedStatus);
    } else if (status == CAT_NO_ROWS_FOUND) {
	return (0);
    } else {
        return (status);
    }
}

