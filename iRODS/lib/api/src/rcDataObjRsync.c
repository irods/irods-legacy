/* This is script-generated code.  */ 
/* See dataObjRsync.h for a description of this API call.*/

#include "dataObjRsync.h"
#include "dataObjPut.h"
#include "dataObjGet.h"

int
rcDataObjRsync (rcComm_t *conn, dataObjInp_t *dataObjInp) 
{
    int status;
    msParamArray_t *outParamArray = NULL;
    char *locFilePath;

    status = _rcDataObjRsync (conn, dataObjInp, &outParamArray);

    if (status == SYS_SVR_TO_CLI_PUT_ACTION) {
        if ((locFilePath = getValByKey (&dataObjInp->condInput,
          RSYNC_DEST_PATH_KW)) == NULL) {
	    return USER_INPUT_PATH_ERR;
	} else {
	    status = rcDataObjPut (conn, dataObjInp, locFilePath);
	    if (status >= 0) {
		return SYS_RSYNC_TARGET_MODIFIED;
	    } else {
	        return status;
	    }
	}
    } else if (status == SYS_SVR_TO_CLI_GET_ACTION) {
        if ((locFilePath = getValByKey (&dataObjInp->condInput,
          RSYNC_DEST_PATH_KW)) == NULL) {
            return USER_INPUT_PATH_ERR;
        } else {
            status = rcDataObjGet (conn, dataObjInp, locFilePath);
            if (status >= 0) {
                return SYS_RSYNC_TARGET_MODIFIED;
            } else {
                return status;
            }
	}
    }

    /* below is for backward compatibility */
    while (status == SYS_SVR_TO_CLI_MSI_REQUEST) {
	/* it is a server request */
        msParam_t *myMsParam;
        dataObjInp_t *dataObjInp = NULL;
	int l1descInx; 

	myMsParam = getMsParamByLabel (outParamArray, CL_ZONE_OPR_INX);
	if (myMsParam == NULL) {
	    l1descInx = -1;
	} else {
	    l1descInx = *(int*) myMsParam->inOutStruct;
	}

	if ((myMsParam = getMsParamByLabel (outParamArray, CL_PUT_ACTION))
	  != NULL) { 

	    dataObjInp = (dataObjInp_t *) myMsParam->inOutStruct;
	    if ((locFilePath = getValByKey (&dataObjInp->condInput, 
	      RSYNC_DEST_PATH_KW)) == NULL) {
		if (l1descInx >= 0) {
		    rcOprComplete (conn, l1descInx);
		} else {
		        rcOprComplete (conn, USER_FILE_DOES_NOT_EXIST);
		}
	    } else {
	        status = rcDataObjPut (conn, dataObjInp, locFilePath);
                if (l1descInx >= 0) {
                    rcOprComplete (conn, l1descInx);
                } else {
		    rcOprComplete (conn, status);
		}
	    }
	} else if ((myMsParam = getMsParamByLabel (outParamArray, 
	  CL_GET_ACTION)) != NULL) {
            dataObjInp = (dataObjInp_t *) myMsParam->inOutStruct;
            if ((locFilePath = getValByKey (&dataObjInp->condInput,
              RSYNC_DEST_PATH_KW)) == NULL) {
                if (l1descInx >= 0) {
                    rcOprComplete (conn, l1descInx);
                } else {
                    rcOprComplete (conn, USER_FILE_DOES_NOT_EXIST);
		}
            } else {
                status = rcDataObjGet (conn, dataObjInp, locFilePath);
                if (l1descInx >= 0) {
                    rcOprComplete (conn, l1descInx);
                } else {
                    rcOprComplete (conn, status);
		}
            }
	} else {
            if (l1descInx >= 0) {
                rcOprComplete (conn, l1descInx);
            } else {
                rcOprComplete (conn, SYS_SVR_TO_CLI_MSI_NO_EXIST);
	    }
	}
	/* free outParamArray */
	if (dataObjInp != NULL) {
	    clearKeyVal (&dataObjInp->condInput); 
	}
	clearMsParamArray (outParamArray, 1);
	free (outParamArray);

	/* read the reply from the eariler call */

        status = branchReadAndProcApiReply (conn, DATA_OBJ_RSYNC_AN, 
        (void **)&outParamArray, NULL);
        if (status < 0) {
            rodsLogError (LOG_DEBUG, status,
              "rcDataObjRsync: readAndProcApiReply failed. status = %d", 
	      status);
        }
    }

    return (status);
}

int
_rcDataObjRsync (rcComm_t *conn, dataObjInp_t *dataObjInp, 
msParamArray_t **outParamArray)
{
    int status;

    status = procApiRequest (conn, DATA_OBJ_RSYNC_AN, dataObjInp, NULL,
        (void **)outParamArray, NULL);

    return (status);
}
