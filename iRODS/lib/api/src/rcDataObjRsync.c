/* This is script-generated code.  */ 
/* See dataObjRsync.h for a description of this API call.*/

#include "dataObjRsync.h"
#include "oprComplete.h"
#include "dataObjPut.h"
#include "dataObjGet.h"

int
rcDataObjRsync (rcComm_t *conn, dataObjInp_t *dataObjInp) 
{
    int status;
    msParamArray_t *outParamArray = NULL;

    status = procApiRequest (conn, DATA_OBJ_RSYNC_AN, dataObjInp, NULL, 
        (void **)&outParamArray, NULL);
 
    while (status == SYS_SVR_TO_CLI_MSI_REQUEST) {
	/* it is a server request */
	char *locFilePath;
        msParam_t *myMsParam;
        dataObjInp_t *dataObjInp = NULL;


	if ((myMsParam = getMsParamByLabel (outParamArray, CL_PUT_ACTION))
	  != NULL) { 

	    dataObjInp = (dataObjInp_t *) myMsParam->inOutStruct;
	    if ((locFilePath = getValByKey (&dataObjInp->condInput, 
	      RSYNC_DEST_PATH_KW)) == NULL) {
		rcOprComplete (conn, USER_FILE_DOES_NOT_EXIST);
	    } else {
	        status = rcDataObjPut (conn, dataObjInp, locFilePath);
		rcOprComplete (conn, status);
	    }
	} else if ((myMsParam = getMsParamByLabel (outParamArray, 
	  CL_GET_ACTION)) != NULL) {
            dataObjInp = (dataObjInp_t *) myMsParam->inOutStruct;
            if ((locFilePath = getValByKey (&dataObjInp->condInput,
              RSYNC_DEST_PATH_KW)) == NULL) {
                rcOprComplete (conn, USER_FILE_DOES_NOT_EXIST);
            } else {
                status = rcDataObjGet (conn, dataObjInp, locFilePath);
                rcOprComplete (conn, status);
            }
	} else {
	    rcOprComplete (conn, SYS_SVR_TO_CLI_MSI_NO_EXIST);
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

