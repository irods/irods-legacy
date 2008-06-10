/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* rsApiHandler.c - The server API Handler. handle RODS_API_REQ_T type
 * messages
 */

#include "rsApiHandler.h"
#include "modDataObjMeta.h"
#include "rcMisc.h"
#include "miscServerFunct.h"
#include "regReplica.h"
#include "unregDataObj.h"

int
rsApiHandler (rsComm_t *rsComm, int apiNumber, bytesBuf_t *inputStructBBuf,
bytesBuf_t *bsBBuf)
{
    int apiInx;
    int status = 0;
    char *myInStruct = NULL;
    funcPtr myHandler = NULL;
    void *myOutStruct = NULL;
    bytesBuf_t myOutBsBBuf;
    int retVal = 0;
    int numArg = 0;
    void *myArgv[4];
    
    memset (&myOutBsBBuf, 0, sizeof (bytesBuf_t));
    memset (&rsComm->rError, 0, sizeof (rError_t));

    apiInx = apiTableLookup (apiNumber);
    if (apiInx < 0) {
	sendApiReply (rsComm, apiInx, apiInx, myOutStruct, &myOutBsBBuf);
	rodsLog (LOG_NOTICE,
	  "rsApiHandler: apiTableLookup of apiNumber %d failed", apiNumber);
	return (apiInx);
    }
 
    rsComm->apiInx = apiInx;

    status = chkApiPermission (rsComm, apiInx);
    if (status < 0) {
        rodsLog (LOG_NOTICE,
          "rsApiHandler: User has no permission for apiNumber %d", apiNumber);
	sendApiReply (rsComm, apiInx, status, myOutStruct, &myOutBsBBuf);
        return (status);
    }
    
    /* some sanity check */

    if (inputStructBBuf->len > 0 && RsApiTable[apiInx].inPackInstruct == NULL) {
	rodsLog (LOG_NOTICE,
          "rsApiHandler: input struct error for apiNumber %d", apiNumber);
	sendApiReply (rsComm, apiInx, SYS_API_INPUT_ERR, myOutStruct, 
	  &myOutBsBBuf);
	return (SYS_API_INPUT_ERR);
    }
 
    if (inputStructBBuf->len <= 0 && RsApiTable[apiInx].inPackInstruct != NULL){
	rodsLog (LOG_NOTICE,
          "rsApiHandler: input struct error for apiNumber %d", apiNumber);
	sendApiReply (rsComm, apiInx, SYS_API_INPUT_ERR, myOutStruct, 
	  &myOutBsBBuf);
	return (SYS_API_INPUT_ERR);
    }
 
    if (bsBBuf->len > 0 && RsApiTable[apiInx].inBsFlag <= 0) {        
	rodsLog (LOG_NOTICE,
          "rsApiHandler: input byte stream error for apiNumber %d", apiNumber);
	sendApiReply (rsComm, apiInx, SYS_API_INPUT_ERR, myOutStruct, 
	  &myOutBsBBuf);
        return (SYS_API_INPUT_ERR);
    }

#if 0	/* input is optional */
    if (bsBBuf->len <= 0 && RsApiTable[apiInx].inBsFlag > 0){
        rodsLog (LOG_NOTICE,
          "rsApiHandler: input byte stream error for apiNumber %d", apiNumber);
	sendApiReply (rsComm, apiInx, SYS_API_INPUT_ERR, myOutStruct, 
	  &myOutBsBBuf);
        return (SYS_API_INPUT_ERR);
    }
#endif

    if (inputStructBBuf->len > 0) {
        status = unpackStruct (inputStructBBuf->buf, (void **) &myInStruct,
          RsApiTable[apiInx].inPackInstruct, RodsPackTable, rsComm->irodsProt);
	if (status < 0) {
            rodsLog (LOG_NOTICE,
              "rsApiHandler: unpackStruct error for apiNumber %d, status = %d",
	      apiNumber, status);
	    sendApiReply (rsComm, apiInx, status, myOutStruct, 
	      &myOutBsBBuf);
	    return (status);
	}
    }

    /* ready to call the handler functions */

    myHandler = RsApiTable[apiInx].svrHandler;

    if (RsApiTable[apiInx].inPackInstruct != NULL) {
	myArgv[numArg] = myInStruct;
	numArg++;
    };
 
    if (RsApiTable[apiInx].inBsFlag != 0) {
        myArgv[numArg] = bsBBuf;
        numArg++;
    };

    if (RsApiTable[apiInx].outPackInstruct != NULL) {
        myArgv[numArg] = (void *) &myOutStruct;
        numArg++;
    };

    if (RsApiTable[apiInx].outBsFlag != 0) {
        myArgv[numArg] = (void *) &myOutBsBBuf;
        numArg++;
    };

    if (numArg == 0) {
	 retVal = (*myHandler) (rsComm);
    } else if (numArg == 1) {
         retVal = (*myHandler) (rsComm, myArgv[0]);
    } else if (numArg == 2) {
         retVal = (*myHandler) (rsComm, myArgv[0], myArgv[1]);
    } else if (numArg == 3) {
         retVal = (*myHandler) (rsComm, myArgv[0], myArgv[1], myArgv[2]);
    } else if (numArg == 4) {
         retVal = (*myHandler) (rsComm, myArgv[0], myArgv[1], myArgv[2],
	  myArgv[3]);
    }

#if 0
	/* XXXXX this is a hack to reduce mem leak. Need a more generalized 
	 * solution */
	if (apiNumber == GEN_QUERY_AN) {
	    clearGenQueryInp ((genQueryInp_t *) myInStruct);
	} else if (apiNumber == MOD_DATA_OBJ_META_AN) {
	    clearModDataObjMetaInp ((modDataObjMeta_t *) myInStruct);
        } else if (apiNumber == REG_REPLICA_AN) {
	    clearRegReplicaInp ((regReplica_t *) myInStruct);
        } else if (apiNumber == UNREG_DATA_OBJ_AN) {
	    clearUnregDataObj ((unregDataObj_t *) myInStruct);
	}
#endif
    if (myInStruct != NULL) {
        /* XXXXX this is a hack to reduce mem leak. Need a more generalized
         * solution */
	if (strcmp (RsApiTable[apiInx].inPackInstruct, "GenQueryInp_PI") == 0) {
	    clearGenQueryInp ((genQueryInp_t *) myInStruct);
        } else if (strcmp (RsApiTable[apiInx].inPackInstruct, 
	  "ModDataObjMeta_PI")  == 0) {
            clearModDataObjMetaInp ((modDataObjMeta_t *) myInStruct);
	} else if (strcmp (RsApiTable[apiInx].inPackInstruct, 
	  "RegReplica_PI")  == 0) {
            clearRegReplicaInp ((regReplica_t *) myInStruct);
        } else if (strcmp (RsApiTable[apiInx].inPackInstruct, 
	  "UnregDataObj_PI")  == 0) {
            clearUnregDataObj ((unregDataObj_t *) myInStruct);
        } else if (strcmp (RsApiTable[apiInx].inPackInstruct,
	  "DataObjInp_PI")  == 0) {
	    clearDataObjInp ((dataObjInp_t *) myInStruct);
        } else if (strcmp (RsApiTable[apiInx].inPackInstruct,
	  "DataObjCopyInp_PI")  == 0) {
            clearDataObjCopyInp ((dataObjCopyInp_t *) myInStruct);
	}
        free (myInStruct);
        myInStruct = NULL;
    }

    if (retVal != SYS_NO_HANDLER_REPLY_MSG) {
        status = sendAndProcApiReply 
	  (rsComm, apiInx, retVal, myOutStruct, &myOutBsBBuf);
    }

    if (retVal >= 0 && status < 0) {
	return (status);
    } else {
	return (retVal);
    }
}

int 
sendAndProcApiReply ( rsComm_t *rsComm, int apiInx, int status, 
void *myOutStruct, bytesBuf_t *myOutBsBBuf)
{
    int retval;

    retval = sendApiReply (rsComm, apiInx, status, myOutStruct, myOutBsBBuf);

    clearBBuf (myOutBsBBuf);
    if (myOutStruct != NULL) {
	free (myOutStruct);
    }
    freeRErrorContent (&rsComm->rError);

    /* check for portal operation */

    if (rsComm->portalOpr != NULL) {
	handlePortalOpr (rsComm);
	clearKeyVal (&rsComm->portalOpr->dataOprInp.condInput);
	free (rsComm->portalOpr);
	rsComm->portalOpr = NULL;
    }

    return (retval);
}

int
sendApiReply (rsComm_t *rsComm, int apiInx, int retVal, 
void *myOutStruct, bytesBuf_t *myOutBsBBuf)
{
    int status;
    bytesBuf_t *outStructBBuf = NULL;
    bytesBuf_t *myOutStructBBuf;
    bytesBuf_t *rErrorBBuf = NULL;
    bytesBuf_t *myRErrorBBuf;
    int retryCnt = 0;

    if (retVal == SYS_HANDLER_DONE_NO_ERROR) {
	/* not actually an error */
	retVal = 0;
    }

    if (RsApiTable[apiInx].outPackInstruct != NULL && myOutStruct != NULL) {

        status = packStruct ((char *) myOutStruct, &outStructBBuf,
          RsApiTable[apiInx].outPackInstruct, RodsPackTable, FREE_POINTER, 
	  rsComm->irodsProt);

       if (status < 0) {
            rodsLog (LOG_NOTICE,
             "sendApiReply: packStruct error, status = %d", status);
            sendRodsMsg (rsComm->sock, RODS_API_REPLY_T, NULL,
              NULL, NULL, status, rsComm->irodsProt);
            return status;
	}

	myOutStructBBuf = outStructBBuf;
    } else {
	myOutStructBBuf = NULL;
    }

    if (RsApiTable[apiInx].outBsFlag == 0) {
        myOutBsBBuf = NULL;
    }

    if (rsComm->rError.len > 0) {
        status = packStruct ((char *) &rsComm->rError, &rErrorBBuf,
          "RError_PI", RodsPackTable, 0, rsComm->irodsProt);

       if (status < 0) {
            rodsLog (LOG_NOTICE,
             "sendApiReply: packStruct error, status = %d", status);
            sendRodsMsg (rsComm->sock, RODS_API_REPLY_T, NULL,
              NULL, NULL, status, rsComm->irodsProt);
            return status;
        }

        myRErrorBBuf = rErrorBBuf;
    } else {
        myRErrorBBuf = NULL;
    }

    while (retryCnt <= SEND_RCV_RETRY_CNT) {
        status = sendRodsMsg (rsComm->sock, RODS_API_REPLY_T, myOutStructBBuf,
          myOutBsBBuf, myRErrorBBuf, retVal, rsComm->irodsProt);
	if (status >= 0 || rsComm->reconnSock == 0) {
	    break;
	} else {
	    rodsSleep (SEND_RCV_SLEEP_TIME, 0);
        }
	retryCnt ++;
    }
      
    if (status < 0) {
	int savedStatus = status;

        rodsLog (LOG_NOTICE,
         "sendApiReply: sendRodsMsg error, status = %d", status);
        /* attempt to accept reconnect. ENOENT result  from
                     * user cntl-C */
#if 0	/* XXXXX testing only */
        if (errno != ENOENT && rsComm->reconnSock > 0) {
#else
        if (rsComm->reconnSock > 0) {
#endif
            status = svrReconnect (rsComm);
            if (status >= 0) {
                /* success */
                rodsLog (LOG_NOTICE,
                  "sendApiReply: Reconnected");
    		status = sendRodsMsg (rsComm->sock, RODS_API_REPLY_T, 
		 myOutStructBBuf, myOutBsBBuf, myRErrorBBuf, retVal, 
		 rsComm->irodsProt);
                if (status >= 0) {
                    rodsLog (LOG_NOTICE,
                      "sendApiReply: Reconnected and sendRodsMsg");
                } else {
                    rodsLog (LOG_NOTICE,
                      "sendApiReply: Reconnected but sendRodsMsg failed");
                    return (savedStatus);
                }
            } else {
                rodsLog (LOG_NOTICE,
                  "sendApiReply: Reconnection failed");
                return (savedStatus);
            }
        } else {
            return (status);
        }
    }

    freeBBuf (outStructBBuf);
    freeBBuf (rErrorBBuf);
	
    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "sendApiReply: sendRodsMsg error, status = %d", status);
        return status;
    }

    return (0);
}

int
chkApiPermission (rsComm_t *rsComm, int apiInx)
{
    int clientUserAuth;
    int proxyUserAuth;
    int xmsgSvrOnly;
    int xmsgSvrAlso;

    clientUserAuth = RsApiTable[apiInx].clientUserAuth;

    xmsgSvrOnly = clientUserAuth & XMSG_SVR_ONLY;
    xmsgSvrAlso = clientUserAuth & XMSG_SVR_ALSO;

    if (ProcessType == XMSG_SERVER_PT) {
        if ((xmsgSvrOnly + xmsgSvrAlso) == 0) {
            rodsLog (LOG_ERROR,
             "chkApiPermission: xmsgServer not allowed to handle api %d",
	      RsApiTable[apiInx].apiNumber);
            return (SYS_NO_API_PRIV);
	}
    } else if (xmsgSvrOnly != 0) {
        rodsLog (LOG_ERROR,
         "chkApiPermission: non xmsgServer not allowed to handle api %d",
          RsApiTable[apiInx].apiNumber);
        return (SYS_NO_API_PRIV);
    }

    clientUserAuth = clientUserAuth & 0xfff;	/* take out XMSG_SVR_* flags */

    if (clientUserAuth > rsComm->clientUser.authInfo.authFlag) {
	return (SYS_NO_API_PRIV);
    }

    proxyUserAuth = RsApiTable[apiInx].proxyUserAuth & 0xfff;
    if (proxyUserAuth > rsComm->proxyUser.authInfo.authFlag) {
	return (SYS_NO_API_PRIV);
    }
    return  (0);
}

int
handlePortalOpr (rsComm_t *rsComm)
{
    int oprType;
    int status;

    if (rsComm == NULL || rsComm->portalOpr == NULL)
	return (0);

    oprType = rsComm->portalOpr->oprType;

    switch (oprType) {
      case PUT_OPR:
      case GET_OPR:
	status = svrPortalPutGet (rsComm);
	break;
      default:
        rodsLog (LOG_NOTICE,
	  "handlePortalOpr: Invalid portal oprType: %d", oprType);
	status = SYS_INVALID_PORTAL_OPR;
	break;
    }
    return (status);
}
    
int
readAndProcClientMsg (rsComm_t *rsComm, int retApiStatus)
{
    int status = 0;
    msgHeader_t myHeader;
    bytesBuf_t inputStructBBuf, bsBBuf, errorBBuf;
    int retryCnt = 0; 

    /* everything else are set in readMsgBody */

    memset (&bsBBuf, 0, sizeof (bsBBuf));

    /* head the header */

    while (retryCnt <= SEND_RCV_RETRY_CNT) {
        retryCnt ++;
        status = readMsgHeader (rsComm->sock, &myHeader);
        if (status >= 0 || rsComm->reconnSock == 0) {
            break;
        } else {
            rodsSleep (SEND_RCV_SLEEP_TIME, 0);
            if (rsComm->reconnTime > 0 && time (0) <= 
              rsComm->reconnTime + 60) {
                /* reconnected recently. try to prevent deadlock */
		if (rsComm->reconnOpr != RECONN_SEND_OPR) {
                    rodsLog (LOG_NOTICE,
                      "readAndProcClientMsg:Agent reading.Client not sending");
		    return (SYS_RECONN_OPR_MISMATCH);
		}
	    }
        }
    }

    if (status < 0) {
	int savedStatus = status;
        rodsLog (LOG_DEBUG,
          "readAndProcClientMsg: readMsgHeader error. status = %d", status);
        /* attempt to accept reconnect. ENOENT result  from
                     * user cntl-C */
#if 0	/* XXXXXX testing only */
        if (errno != ENOENT && rsComm->reconnSock > 0) {
#else
        if (rsComm->reconnSock > 0) {
#endif
            status = svrReconnect (rsComm);
            if (status >= 0) {
                /* success */
	        rodsLog (LOG_NOTICE,
                  "readAndProcClientMsg: Reconnected");
                /* reconnected recently. try to prevent deadlock */
                if (rsComm->reconnOpr != RECONN_SEND_OPR) {
                    rodsLog (LOG_NOTICE,
                      "readAndProcClientMsg:Agent reading.Client not sending");
                    return (SYS_RECONN_OPR_MISMATCH);
                }

		status = readMsgHeader (rsComm->sock, &myHeader);
		if (status >= 0) {
                    rodsLog (LOG_NOTICE,
                      "readAndProcClientMsg: Reconnected and readMsgHeader");
		} else {
                    rodsLog (LOG_NOTICE,
                      "readAndProcClientMsg: Reconnected but readMsgHeader failed");
		    return (savedStatus);
		}
            } else {
	        rodsLog (LOG_NOTICE,
                  "readAndProcClientMsg: Reconnection failed");
		return (savedStatus);
            }
	} else {
            return (status);
	}
    }

    status = readMsgBody (rsComm->sock, &myHeader, &inputStructBBuf,
      &bsBBuf, &errorBBuf, rsComm->irodsProt);
    if (status < 0) {
        rodsLog (LOG_NOTICE,
          "agentMain: readMsgBody error. status = %d", status);
        return (status);
    }

    /* handler switch by msg type */

    if (strcmp (myHeader.type, RODS_API_REQ_T) == 0) {
        status = rsApiHandler (rsComm, myHeader.intInfo, &inputStructBBuf,
          &bsBBuf);
        clearBBuf (&inputStructBBuf);
        clearBBuf (&bsBBuf);
        clearBBuf (&errorBBuf);
	if (retApiStatus > 0) {
	    return (status);
	} else {
	    return (0);
	}
    } else if (strcmp (myHeader.type, RODS_DISCONNECT_T) == 0) {
        rodsLog (LOG_NOTICE,
          "agentMain: received disconnect msg from client");
        return (DISCONN_STATUS);
    } else {
        rodsLog (LOG_NOTICE,
          "agentMain: msg type %s not support by server",
          myHeader.type);
	return (USER_MSG_TYPE_NO_SUPPORT);
    }
}

/* sendAndRecvBranchMsg - Break out the normal mode of  
 * clientReuest/serverReply protocol for handling API. Instead of returning
 * to rsApiHandler() and send a API reply, it sends a reply directly to
 * the client through sendAndProcApiReply. 
 * Then it loops though readAndProcClientMsg() to process addtitional
 * clients requests until the status is SYS_HANDLER_DONE_NO_ERROR
 * which is generated by a rcOprComplete() call by the client. The client
 * must remember to send a rcOprComplete() call or the server will hang
 * in this loop. 
 * The caller of this routine should return a SYS_NO_HANDLER_REPLY_MSG
 * status to rsApiHandler() if the client is not expecting any more
 * reply msg.
 */ 

int
sendAndRecvBranchMsg (rsComm_t *rsComm, int apiInx, int status,
void *myOutStruct, bytesBuf_t *myOutBsBBuf)
{
    int retval;
    int savedApiInx;

    savedApiInx = rsComm->apiInx;
    retval = sendAndProcApiReply (rsComm, apiInx, status,
      myOutStruct, myOutBsBBuf);
    if (retval < 0) {
        rodsLog (LOG_ERROR,
      "sendAndRecvBranchMsg: sendAndProcApiReply error. status = %d", retval);
	rsComm->apiInx = savedApiInx;
        return (retval);
    }

    while (1)  {
        retval = readAndProcClientMsg (rsComm, 1);
        if (retval >= 0 || retval == SYS_NO_HANDLER_REPLY_MSG) {
	    /* more to come */
	    continue;
        } else {
	    rsComm->apiInx = savedApiInx;
	    if (retval == SYS_HANDLER_DONE_NO_ERROR) {
		return 0;
	    } else {
                return (retval);
	    }
        }
    }
}
 
int
svrSendCollOprStat (rsComm_t *rsComm, collOprStat_t *collOprStat)
{
    int myBuf;
    int status;

    status = sendAndProcApiReply (rsComm, rsComm->apiInx,
      SYS_SVR_TO_CLI_COLL_STAT, collOprStat, NULL);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "svrSendCollOprStat: sendAndProcApiReply failed. status = %d",
          status);
	return status;
    }

    /* read 4 bytes */
    status = myRead (rsComm->sock, &myBuf, sizeof (myBuf), SOCK_TYPE, NULL);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "svrSendCollOprStat: read handshake failed. status = %d", status);
    }
    if (ntohl (myBuf) != SYS_CLI_TO_SVR_COLL_STAT_REPLY) {
        rodsLog (LOG_ERROR,
          "svrSendCollOprStat: client reply %d != %d.", 
	  ntohl (myBuf), SYS_CLI_TO_SVR_COLL_STAT_REPLY);
	return (UNMATCHED_KEY_OR_INDEX);
    } 

    return (0);
}
