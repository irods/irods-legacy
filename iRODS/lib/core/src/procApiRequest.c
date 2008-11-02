/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* procApiRequest.c - process API request
 */
#include "procApiRequest.h"
#include "rcGlobalExtern.h"
#include "rcMisc.h"

/* procApiRequest - This is the main function used by the client API
 * function to issue API requests and recieve output returned from 
 * the server.
 * rcComm_t *conn - the client communication handle.
 * int apiNumber - the API number of this call defined in apiNumber.h.
 * void *inputStruct - pointer to the input struct of this API. If there
 *     is no input struct, a NULL should be entered
 * bytesBuf_t *inputBsBBuf - pointer to the input byte stream. If there
 *     is no input byte stream, a NULL should be entered
 * void **outStruct - Pointer to pointer to the output struct. The outStruct 
 *     will be allocated by this function and the pointer to this struct is
 *     passed back to the caller through this pointer. If there
 *     is no output struct, a NULL should be entered
 * bytesBuf_t *outBsBBuf - pointer to the output byte stream. If there
 *     is no output byte stream, a NULL should be entered
 * 
 */

int
procApiRequest (rcComm_t *conn, int apiNumber, void *inputStruct,
bytesBuf_t *inputBsBBuf, void **outStruct, bytesBuf_t *outBsBBuf)
{
    int status;
    int apiInx;

    if (conn == NULL) {
	return (USER__NULL_INPUT_ERR);
    }

    freeRError (conn->rError);
    conn->rError = NULL;
    
    apiInx = apiTableLookup (apiNumber);
    if (apiInx < 0) {
        rodsLog (LOG_ERROR,
          "procApiRequest: apiTableLookup of apiNumber %d failed", apiNumber);
        return (apiInx);
    }

    status = sendApiRequest (conn, apiInx, inputStruct, inputBsBBuf);
    if (status < 0) {
        rodsLogError (LOG_DEBUG, status,
          "procApiRequest: sendApiRequest failed. status = %d", status);
        return (status);
    }

    conn->apiInx = apiInx;

    status = readAndProcApiReply (conn, apiInx, outStruct, outBsBBuf);
    if (status < 0) {
        rodsLogError (LOG_DEBUG, status,
          "procApiRequest: readAndProcApiReply failed. status = %d", status);
    }

    return (status);
}

int
branchReadAndProcApiReply (rcComm_t *conn, int apiNumber,
void **outStruct, bytesBuf_t *outBsBBuf)
{
    int status;
    int apiInx;

    if (conn == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    apiInx = apiTableLookup (apiNumber);
    if (apiInx < 0) {
        rodsLog (LOG_ERROR,
          "branchReadAndProcApiReply: apiTableLookup of apiNum %d failed", 
	  apiNumber);
        return (apiInx);
    }

    conn->apiInx = apiInx;

    status = readAndProcApiReply (conn, apiInx, outStruct, outBsBBuf);
    if (status < 0) {
        rodsLogError (LOG_DEBUG, status,
          "branchReadAndProcApiReply: readAndProcApiReply failed. status = %d",
	   status);
    }
    return (status);
}

int
sendApiRequest (rcComm_t *conn, int apiInx, void *inputStruct,
bytesBuf_t *inputBsBBuf)
{
    int status;
    bytesBuf_t *inputStructBBuf = NULL;
    bytesBuf_t *myInputStructBBuf;

#if 0	/* XXXXX redo */
    if (conn->reconnTime > 0) {
	/* has been reconnected. need to r-eauthenticate */
        status = clientLogin(conn);
        if (status != 0) {
           rcDisconnect(conn);
        }
	conn->reconnTime = 0;
    }
#endif

    if (RcApiTable[apiInx].inPackInstruct != NULL) {
        if (inputStruct == NULL) {
            return (USER_API_INPUT_ERR);
        }
        status = packStruct ((void *) inputStruct, &inputStructBBuf,
         RcApiTable[apiInx].inPackInstruct, RodsPackTable, 0, conn->irodsProt);

       if (status < 0) {
            rodsLogError (LOG_ERROR, status,
             "sendApiRequest: packStruct error, status = %d", status);
            return status;
        }

        myInputStructBBuf = inputStructBBuf;
    } else {
        myInputStructBBuf = NULL;
    };


    if (RcApiTable[apiInx].inBsFlag <= 0) {
        inputBsBBuf = NULL;
    }

    status = sendRodsMsg (conn->sock, RODS_API_REQ_T, myInputStructBBuf,
      inputBsBBuf, NULL, RcApiTable[apiInx].apiNumber, conn->irodsProt);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
         "sendApiRequest: sendRodsMsg error, status = %d", status);
        if (conn->svrVersion->reconnPort > 0) {
#if 0	/* XXXXX redo */
            if ((status = rcReconnect (conn, RECONN_SEND_OPR)) >= 0) {
                status = sendRodsMsg (conn->sock, RODS_API_REQ_T, 
		  myInputStructBBuf, inputBsBBuf, NULL, 
		  RcApiTable[apiInx].apiNumber, conn->irodsProt);
		if (status >= 0) {
		    fprintf (stderr,
                     "sendApiRequest: reconnected and sendRodsMsg\n");
		} else {
                    rodsLogError (LOG_ERROR, status,
                    "sendApiRequest: reconnected but sendRodsMsg failed");
    		    freeBBuf (inputStructBBuf);
		    return status;
                }
            } else {
                rodsLog (LOG_ERROR,
                 "sendApiRequest: reconnect failed. status = %d \n",
                  status);
    		freeBBuf (inputStructBBuf);
                return status;
            }
#endif
        } else {
    	    freeBBuf (inputStructBBuf);
            return (status);
        }
    }
    freeBBuf (inputStructBBuf);

    return (0);
}

int
readAndProcApiReply (rcComm_t *conn, int apiInx, void **outStruct, 
bytesBuf_t *outBsBBuf)
{
    int status;
    msgHeader_t myHeader;
    /* bytesBuf_t outStructBBuf, errorBBuf, myOutBsBBuf; */
    bytesBuf_t outStructBBuf, errorBBuf;

    memset (&outStructBBuf, 0, sizeof (bytesBuf_t));
    memset (&outStructBBuf, 0, sizeof (bytesBuf_t));
    /* memset (&myOutBsBBuf, 0, sizeof (bytesBuf_t)); */

    /* some sanity check */

    if (RcApiTable[apiInx].outPackInstruct != NULL && outStruct == NULL) {
        rodsLog (LOG_ERROR,
          "readAndProcApiReply: outStruct error for apiNumber %d", 
	  RcApiTable[apiInx].apiNumber);
        return (USER_API_INPUT_ERR);
    }

    if (RcApiTable[apiInx].outBsFlag > 0 && outBsBBuf == NULL) {
        rodsLog (LOG_ERROR,
          "readAndProcApiReply: outBsBBuf error for apiNumber %d",
          RcApiTable[apiInx].apiNumber);
        return (USER_API_INPUT_ERR);
    }

    status = readMsgHeader (conn->sock, &myHeader);

    if (status < 0) {
	int savedStatus = status;
        rodsLogError (LOG_ERROR, status,
          "readAndProcApiReply: readMsgHeader error. status = %d", status);

	if (conn->svrVersion->reconnPort > 0) {
#if 0	/* XXXXXX redo */
            if ((status = rcReconnect (conn, RECONN_RCV_OPR)) >= 0) {
		status = readMsgHeader (conn->sock, &myHeader);
		if (status >= 0) {
		    fprintf (stderr,
                     "readAndProcApiReply: reconnected and readMsgHeader\n");
		} else {
        	    rodsLogError (LOG_ERROR, status,
                     "readAndProcApiReply:reconnected but readMsgHeader failed");
		    return (savedStatus);
	        }
	    } else {
	        rodsLog (LOG_ERROR,
                 "readAndProcApiReply: reconnect failed. status = %d \n",
                  status);
                return savedStatus;
	    }
#endif
	} else {
            return (savedStatus);
	}
    }

    status = readMsgBody (conn->sock, &myHeader, &outStructBBuf, outBsBBuf,
      &errorBBuf, conn->irodsProt);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "readAndProcApiReply: readMsgBody error. status = %d", status);
        return (status);
    }

    if (strcmp (myHeader.type, RODS_API_REPLY_T) == 0) {
	status = procApiReply (conn, apiInx, outStruct, outBsBBuf,
	 &myHeader, &outStructBBuf, NULL, &errorBBuf); 
    }

    clearBBuf (&outStructBBuf);
    /* clearBBuf (&myOutBsBBuf); */
    clearBBuf (&errorBBuf);

    return (status);
}

int
procApiReply (rcComm_t *conn, int apiInx, void **outStruct,
bytesBuf_t *outBsBBuf,
msgHeader_t *myHeader, bytesBuf_t *outStructBBuf, bytesBuf_t *myOutBsBBuf, 
bytesBuf_t *errorBBuf)
{
    int status;
    int retVal;

    if (errorBBuf->len > 0) {
        status = unpackStruct (errorBBuf->buf, (void **) &conn->rError,
          "RError_PI", RodsPackTable, conn->irodsProt);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
             "readAndProcApiReply:unpackStruct error. status = %d",
             status);
        }
    }

    retVal = myHeader->intInfo;

    /* some sanity check */

    if (RcApiTable[apiInx].outPackInstruct != NULL && outStruct == NULL) {
        rodsLog (LOG_ERROR,
          "readAndProcApiReply: outStruct error for apiNumber %d",
          RcApiTable[apiInx].apiNumber);
	if (retVal < 0)
	    return retVal;
	else
            return (USER_API_INPUT_ERR);
    }

    if (RcApiTable[apiInx].outBsFlag > 0 && outBsBBuf == NULL) {
        rodsLog (LOG_ERROR,
          "readAndProcApiReply: outBsBBuf error for apiNumber %d",
          RcApiTable[apiInx].apiNumber);
        if (retVal < 0)
            return retVal;
        else
            return (USER_API_INPUT_ERR);
    }

    /* handle outStruct */
    if (outStructBBuf->len > 0) {
	if (outStruct != NULL) {
            status = unpackStruct (outStructBBuf->buf, (void **) outStruct,
              RcApiTable[apiInx].outPackInstruct, RodsPackTable, 
	      conn->irodsProt);
            if (status < 0) {
                rodsLogError (LOG_ERROR, status,
                 "readAndProcApiReply:unpackStruct error. status = %d",
                 status);
                if (retVal < 0)
                    return retVal;
                else
	            return (status);
            }
	} else {
            rodsLog (LOG_ERROR,
              "readAndProcApiReply: got unneeded outStruct for apiNumber %d",
              RcApiTable[apiInx].apiNumber);
	}
    }

    if (myOutBsBBuf != NULL && myOutBsBBuf->len > 0) {
        if (outBsBBuf != NULL) {
	    /* copy to out */
	    *outBsBBuf = *myOutBsBBuf;
	    memset (myOutBsBBuf, 0, sizeof (bytesBuf_t));
	} else {
            rodsLog (LOG_ERROR,
              "readAndProcApiReply: got unneeded outBsBBuf for apiNumber %d",
              RcApiTable[apiInx].apiNumber);
	}
    }

    return retVal;
}

int 
cliGetCollOprStat (rcComm_t *conn, collOprStat_t *collOprStat, int vFlag, 
int retval)
{
    int status = retval;

    while (status == SYS_SVR_TO_CLI_COLL_STAT) {
        /* more to come */
        if (collOprStat != NULL) {
            if (vFlag != 0) {
		printf ("num files done = %d, ", collOprStat->filesCnt);
		if (collOprStat->totalFileCnt <= 0) {
                    printf ("totalFileCnt = UNKNOWN, ");
		} else {
                    printf ("totalFileCnt = %d, ", collOprStat->totalFileCnt);
		}
		printf ("bytesWritten = %lld, last file done: %s\n",
                  collOprStat->bytesWritten, collOprStat->lastObjPath);
            }
            free (collOprStat);
            collOprStat = NULL;
        }
	status = _cliGetCollOprStat (conn, &collOprStat);
    }

    if (collOprStat != NULL) {
        free (collOprStat);
    }

    return (status);
}

int
_cliGetCollOprStat (rcComm_t *conn, collOprStat_t **collOprStat)
{
    int myBuf;
    int status;

    myBuf = htonl (SYS_CLI_TO_SVR_COLL_STAT_REPLY);
    status = myWrite (conn->sock, (void *) &myBuf, 4, SOCK_TYPE, NULL);
    status = readAndProcApiReply (conn, conn->apiInx,
      (void **) collOprStat, NULL);

    return (status);
}

