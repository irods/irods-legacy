/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* miscServerFunct.c - misc server functions
 */



#include "miscServerFunct.h"
#include "dataObjOpen.h"
#include "dataObjLseek.h"	
#include "dataObjOpr.h"
#include "dataObjClose.h"
#include "dataObjWrite.h"
#include "dataObjRead.h"
#include "rcPortalOpr.h"
#ifdef PARA_OPR
#include <pthread.h>
#endif
#if !defined(solaris_platform)
char *__loc1;
#endif /* linux_platform */

int
svrToSvrConnectNoLogin (rsComm_t *rsComm, rodsServerHost_t *rodsServerHost)
{
    int portNum;
    rErrMsg_t errMsg;
    int status;
    int reconnFlag;


    if (rodsServerHost->conn == NULL) { /* a connection already */
	if (getenv ("svrPortReconnect") != NULL) {
	    reconnFlag = 1;
	} else {
	    reconnFlag = 0;
	}
        rodsServerHost->conn = _rcConnect (rodsServerHost->hostName->name,
          rodsServerHost->portNum,
          rsComm->myEnv.rodsUserName, rsComm->myEnv.rodsZone,
          rsComm->clientUser.userName, rsComm->clientUser.rodsZone, &errMsg,
	   rsComm->connectCnt, reconnFlag);

        if (rodsServerHost->conn == NULL) {
            if (errMsg.status < 0) {
                return (errMsg.status);
            } else {
                return (SYS_SVR_TO_SVR_CONNECT_FAILED - errno);
            }
        }
    }

    return (rodsServerHost->localFlag);
}

int
svrToSvrConnect (rsComm_t *rsComm, rodsServerHost_t *rodsServerHost)
{
    int portNum;
    rErrMsg_t errMsg;
    int status;
    int remoteFlag;

    status = svrToSvrConnectNoLogin (rsComm, rodsServerHost);

    if (status < 0) {
	return status;
    }
 
    status = clientLogin (rodsServerHost->conn);
    if (status < 0) {
        rodsLog (LOG_NOTICE,
          "getAndConnRcatHost: clientLogin to %s failed",
          rodsServerHost->hostName->name);
        return (status);
    } else {
        return (rodsServerHost->localFlag);
    }
}

int
createSrvPortal (rsComm_t *rsComm, portList_t *thisPortList)
{
    int lsock = -1;
    int lport = 0;
    struct sockaddr_in sin;
    int status;
    int length = sizeof (sin);
    char *laddr = NULL;

    if ((lsock = sockOpenForInConn (rsComm, &lport, &laddr)) < 0) {
        rodsLog (LOG_NOTICE,
         "setupSrvPortal() -- sockOpenForInConn () failed: errno=%d",
          errno);
        return lsock;
    }

#if 0
    length = sizeof (sin);
    if (getsockname (lsock, (struct sockaddr *) &sin, &length)) {
        rodsLog (LOG_NOTICE,
         "createSrvPortal() -- getsockname() failed: errno=%d",
          errno);
        return SYS_SOCK_BIND_ERR - errno;
    }
    lport = ntohs (sin.sin_port);
    if (getsockname (rsComm->sock, (struct sockaddr *) &sin, &length)) {
        rodsLog (LOG_NOTICE,
         "setupSrvPortal() -- getsockname() of rsComm failed: errno=%d",
          errno);
        return SYS_SOCK_BIND_ERR - errno;
    }

    laddr = rods_inet_ntoa (sin.sin_addr);
#endif

    thisPortList->sock = lsock;
    thisPortList->cookie = random ();
    rstrcpy (thisPortList->hostAddr, laddr, LONG_NAME_LEN);
    free (laddr);
    thisPortList->portNum = lport;
    thisPortList->windowSize = rsComm->windowSize;

    listen (lsock, SOMAXCONN);

    return (lsock);
}

int
acceptSrvPortal (rsComm_t *rsComm, portList_t *thisPortList)
{
    int myFd = -1;
    int myCookie;
    int nbytes;
    fd_set basemask;
    int nSockets, nSelected;

    nSockets = thisPortList->sock + 1;
    FD_ZERO(&basemask);
    FD_SET(thisPortList->sock, &basemask);
    struct timeval selectTimeout;


    selectTimeout.tv_sec = SELECT_TIMEOUT_FOR_CONN;
    selectTimeout.tv_usec = 0;

    while ((nSelected = select(nSockets, &basemask,
      (fd_set *) NULL, (fd_set *) NULL, &selectTimeout)) < 0) {
        if (errno == EINTR) {
            rodsLog (LOG_ERROR, "acceptSrvPortal: select interrupted\n");
            continue;
        }
        rodsLog (LOG_ERROR, "acceptSrvPortal: select select failed, errno = %d",
          errno);
    }
    myFd = accept (thisPortList->sock, 0, 0);
    if (myFd < 0) {
        rodsLog (LOG_NOTICE,
         "acceptSrvPortal() -- accept() failed: errno=%d",
          errno);
        return SYS_SOCK_ACCEPT_ERR - errno;
    } else {
	rodsSetSockOpt (myFd, rsComm->windowSize);
    }
#ifdef _WIN32
    nbytes = recv (myFd,&myCookie,sizeof(myCookie),0);
#else
    nbytes = read (myFd, &myCookie,sizeof (myCookie));
#endif
    myCookie = ntohl (myCookie);
    if (nbytes != sizeof (myCookie) || myCookie != thisPortList->cookie) {
        rodsLog (LOG_NOTICE,
         "acceptSrvPortal: cookie err, bytes read=%d,cookie=%d,inCookie=%d",
          nbytes, thisPortList->cookie, myCookie);
	CLOSE_SOCK (myFd);
        return SYS_PORT_COOKIE_ERR;
    }
    return (myFd);
}

int
svrPortalPutGet (rsComm_t *rsComm)
{
    portalOpr_t *myPortalOpr;
    dataOprInp_t *dataOprInp;
    portList_t *thisPortList;
    rodsLong_t size0, size1, offset0;
    int lsock, lport, portalFd;
    int i, status;
    int numThreads;
    portalTransferInp_t myInput[MAX_NUM_CONFIG_TRAN_THR];
#ifdef PARA_OPR
    pthread_t tid[MAX_NUM_CONFIG_TRAN_THR];
#endif
    int oprType;
    int flags = 0;
    int retVal = 0;
    
    myPortalOpr = rsComm->portalOpr;

    if (myPortalOpr == NULL) {
        rodsLog (LOG_NOTICE, "svrPortalPut: NULL myPortalOpr");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    oprType = myPortalOpr->oprType;
    dataOprInp = &myPortalOpr->dataOprInp;

    if (getValByKey (&dataOprInp->condInput, STREAMING_KW)!= NULL) {
	flags |= STREAMING_FLAG;
    }

    numThreads = dataOprInp->numThreads;

    if (numThreads <= 0 || numThreads > MAX_NUM_CONFIG_TRAN_THR) {
        rodsLog (LOG_NOTICE, 
	  "svrPortalPut: numThreads %d out of range");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    memset (myInput, 0, sizeof (myInput));
#ifdef PARA_OPR
    memset (tid, 0, sizeof (tid));
#endif

    size0 = dataOprInp->dataSize / numThreads;
    size1 = dataOprInp->dataSize - size0 * (numThreads - 1);
    offset0 = dataOprInp->offset;

    thisPortList = &myPortalOpr->portList;

    if (thisPortList == NULL) {
        rodsLog (LOG_NOTICE, "svrPortalPut: NULL portList");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    lsock = thisPortList->sock;

    /* accept the first connection */
    portalFd = acceptSrvPortal (rsComm, thisPortList);
    if (portalFd < 0) {
	rodsLog (LOG_NOTICE, 
	  "svrPortalPut: acceptSrvPortal error. errno = %d", 
	  errno);
	
        CLOSE_SOCK (lsock);

        return (portalFd);
    }

    if (oprType == PUT_OPR) {
        fillPortalTransferInp (&myInput[0], rsComm,
         portalFd, dataOprInp->destL3descInx, 0, dataOprInp->destRescTypeInx,
          0, size0, offset0, flags);
    } else {
        fillPortalTransferInp (&myInput[0], rsComm,
         dataOprInp->srcL3descInx, portalFd, dataOprInp->srcRescTypeInx, 0,
          0, size0, offset0, flags);
    }

    if (numThreads == 1) {
        if (oprType == PUT_OPR) {
            partialDataPut (&myInput[0]);
	} else {
            partialDataGet (&myInput[0]);
	}
        CLOSE_SOCK (lsock);

	return (myInput[0].status);
    } else {
#ifdef PARA_OPR
	rodsLong_t mySize = 0;
	rodsLong_t myOffset = 0;

        for (i = 1; i < numThreads; i++) {
	    int l3descInx;

    	    portalFd = acceptSrvPortal (rsComm, thisPortList);
    	    if (portalFd < 0) {
        	rodsLog (LOG_NOTICE,
          	"svrPortalPut: acceptSrvPortal error. errno = %d",
          	 errno);

        	CLOSE_SOCK (lsock);

        	return (portalFd);
    	    }
	    myOffset += size0;
	    if (i < numThreads - 1) {
		mySize = size0;
	    } else {
		mySize = size1;
	    }

	    if (oprType == PUT_OPR) {
	        /* open the file */ 
	        l3descInx = l3OpenByHost (rsComm, dataOprInp->destRescTypeInx, 
	         dataOprInp->destL3descInx, O_WRONLY); 
    	        fillPortalTransferInp (&myInput[i], rsComm,
		 portalFd, l3descInx, 0, dataOprInp->destRescTypeInx,
	          i, mySize, myOffset, flags);
                pthread_create (&tid[i], pthread_attr_default,
                 (void *(*)(void *)) partialDataPut, (void *) &myInput[i]);
	    } else {	/* a get */
                l3descInx = l3OpenByHost (rsComm, dataOprInp->srcRescTypeInx,
                 dataOprInp->srcL3descInx, O_RDONLY);
                fillPortalTransferInp (&myInput[i], rsComm,
		 l3descInx, portalFd, dataOprInp->srcRescTypeInx, 0,
                  i, mySize, myOffset, flags);
                pthread_create (&tid[i], pthread_attr_default,
                 (void *(*)(void *)) partialDataGet, (void *) &myInput[i]);
	    }
	}

        /* spawn the first thread. do this last so the file will not be
	 * closed */
	if (oprType == PUT_OPR) {
            pthread_create (&tid[0], pthread_attr_default,
             (void *(*)(void *)) partialDataPut, (void *) &myInput[0]);
	} else {
            pthread_create (&tid[0], pthread_attr_default,
             (void *(*)(void *)) partialDataGet, (void *) &myInput[0]);
        }

        for ( i = 0; i < numThreads; i++) {
	    if (tid[i] != 0)
                pthread_join (tid[i], NULL);
            if (myInput[i].status < 0) {
                retVal = myInput[i].status;
            }
        }
	return (retVal);

#else	/* PARA_OPR */
        CLOSE_SOCK (lsock);
	return (SYS_PARA_OPR_NO_SUPPORT);
#endif	/* PARA_OPR */
    }
}

int
fillPortalTransferInp (portalTransferInp_t *myInput, rsComm_t *rsComm,
int srcFd, int destFd, int srcRescTypeInx, int destRescTypeInx,
int threadNum, rodsLong_t size, rodsLong_t offset, int flags)
{
    if (myInput == NULL) 
        return (SYS_INTERNAL_NULL_INPUT_ERR);

    myInput->rsComm = rsComm;
    myInput->destFd = destFd;
    myInput->srcFd = srcFd;
    myInput->destRescTypeInx = destRescTypeInx;
    myInput->srcRescTypeInx = srcRescTypeInx;

    myInput->threadNum = threadNum;
    myInput->size = size;
    myInput->offset = offset;
    myInput->flags = flags;

    return (0);
}


void
partialDataPut (portalTransferInp_t *myInput)
{
    int destL3descInx, srcFd, destRescTypeInx;
    char *buf;
    int retryCnt = 0;
    int bytesWritten;
    rodsLong_t bytesToGet;
    rodsLong_t myOffset = 0;

#ifdef PARA_TIMING
    time_t startTime, afterSeek, afterTransfer,
      endTime;
    startTime=time (0);
#endif

    if (myInput == NULL) {
	rodsLog (LOG_SYS_FATAL, "partialDataPut: NULL myInput");
	if (myInput->threadNum > 0)
            _l3Close (myInput->rsComm, destRescTypeInx, destL3descInx);
        CLOSE_SOCK (srcFd);
	return;
    }
 
    myInput->status = 0;
    destL3descInx = myInput->destFd;
    srcFd = myInput->srcFd;
    destRescTypeInx = myInput->destRescTypeInx;

    if (myInput->offset != 0) {
        myOffset = _l3Lseek (myInput->rsComm, destRescTypeInx, 
	  destL3descInx, myInput->offset, SEEK_SET);
        if (myOffset < 0) {
	    myInput->status = myOffset;
            rodsLog (LOG_NOTICE,
	      "_partialDataPut: _objSeek error, status = %d ",
              myInput->status);
	    if (myInput->threadNum > 0)
                _l3Close (myInput->rsComm, destRescTypeInx, destL3descInx);
            CLOSE_SOCK (srcFd);
            return;
        }
    }
    buf = malloc (TRANS_BUF_SZ);

#ifdef PARA_TIMING
    afterSeek=time(0);
#endif

    bytesToGet = myInput->size;

    while (bytesToGet > 0) {
        int toread0, toread1;
        int bytesRead;

#ifdef PARA_TIMING
        time_t tstart, tafterRead, tafterWrite;
        tstart=time(0);
#endif
	if (myInput->flags & STREAMING_FLAG) {
	    toread0 = bytesToGet;
	} else if (bytesToGet > TRANS_SZ) {
	    toread0 = TRANS_SZ;
        } else {
            toread0 = bytesToGet;
        }

	myInput->status = sendTranHeader (srcFd, PUT_OPR, myInput->flags,
	  myOffset, toread0);

	if (myInput->status < 0) {
	    rodsLog (LOG_NOTICE, 
	      "partialDataPut: sendTranHeader error. status = %d", 
	      myInput->status);
	    if (myInput->threadNum > 0)
                _l3Close (myInput->rsComm, destRescTypeInx, destL3descInx);
            CLOSE_SOCK (srcFd);
	    free (buf);
	    return;
	} 

	while (toread0 > 0) {
	    int toread1;

	    if (toread0 > TRANS_BUF_SZ) {
		toread1 = TRANS_BUF_SZ;
	    } else {
		toread1 = toread0;
	    }
            bytesRead = myRead (srcFd, buf, toread1, SOCK_TYPE, NULL);

#ifdef PARA_TIMING
            tafterRead=time(0);
#endif
            if (bytesRead == toread1) {
                if ((bytesWritten = _l3Write (myInput->rsComm, destRescTypeInx,
		  destL3descInx, buf, bytesRead)) != bytesRead) {
		    rodsLog (LOG_NOTICE,
                     "_partialDataPut:Bytes written %d don't match read %d",
                      bytesWritten, bytesRead);

                    if (bytesWritten < 0) {
                        myInput->status = bytesWritten;
                    } else {
                        myInput->status = SYS_COPY_LEN_ERR;
                    }
                    break;
                }
                bytesToGet -= bytesWritten;
		toread0 -= bytesWritten;
                myOffset += bytesWritten;
            } else if (bytesRead < 0) {
                myInput->status = bytesRead;
                break;
            } else {        /* toread > 0 */
		rodsLog (LOG_NOTICE,
                 "_partialDataPut: toread %d bytes, %d bytes read",
                   toread1, bytesRead);
		myInput->status = SYS_COPY_LEN_ERR;
                break;
            }
#ifdef PARA_TIMING
            tafterWrite=time(0);
	    rodsLog (LOG_NOTICE,
              "Thr %d: sz=%d netReadTm=%d diskWriteTm=%d",
              myInput->threadNum, bytesWritten, tafterRead-tstart,
              tafterWrite-tafterRead);
#endif
	}	/* while loop toread0 */
	if (myInput->status < 0)
            break;
    }           /* while loop bytesToGet */
#ifdef PARA_TIMING
    afterTransfer=time(0);
#endif
    free (buf);
    sendTranHeader (srcFd, DONE_OPR, 0, 0, 0);
    if (myInput->threadNum > 0)
        _l3Close (myInput->rsComm, destRescTypeInx, destL3descInx);
    CLOSE_SOCK (srcFd);
#ifdef PARA_TIMING
    endTime=time(0);
    rodsLog (LOG_NOTICE,
      "Thr %d: seekTm=%d transTm=%d endTm=%d",
      myInput->threadInx,
      afterSeek-afterConn, afterTransfer-afterSeek, endTime-afterTransfer);
#endif
    return;
}

void
partialDataGet (portalTransferInp_t *myInput)
{
    int srcL3descInx, destFd, srcRescTypeInx;
    char *buf;
    int retryCnt = 0;
    int bytesWritten;
    rodsLong_t bytesToGet;
    rodsLong_t myOffset = 0;

#ifdef PARA_TIMING
    time_t startTime, afterSeek, afterTransfer,
      endTime;
    startTime=time (0);
#endif

    if (myInput == NULL) {
        rodsLog (LOG_SYS_FATAL, "partialDataGet: NULL myInput");
        if (myInput->threadNum > 0)
            _l3Close (myInput->rsComm, srcRescTypeInx, srcL3descInx);
        CLOSE_SOCK (destFd);
        return;
    }

    myInput->status = 0;
    srcL3descInx = myInput->srcFd;
    destFd = myInput->destFd;
    srcRescTypeInx = myInput->srcRescTypeInx;

    if (myInput->offset != 0) {
        myOffset = _l3Lseek (myInput->rsComm, srcRescTypeInx,
          srcL3descInx, myInput->offset, SEEK_SET);
        if (myOffset < 0) {
            myInput->status = myOffset;
            rodsLog (LOG_NOTICE,
              "_partialDataGet: _objSeek error, status = %d ",
              myInput->status);
            if (myInput->threadNum > 0)
                _l3Close (myInput->rsComm, srcRescTypeInx, srcL3descInx);
            CLOSE_SOCK (destFd);
            return;
        }
    }
    buf = malloc (TRANS_BUF_SZ);

#ifdef PARA_TIMING
    afterSeek=time(0);
#endif

    bytesToGet = myInput->size;

    while (bytesToGet > 0) {
        int toread0, toread1;
        int bytesRead;

#ifdef PARA_TIMING
        time_t tstart, tafterRead, tafterWrite;
        tstart=time(0);
#endif
        if (myInput->flags & STREAMING_FLAG) {
            toread0 = bytesToGet;
        } else if (bytesToGet > TRANS_SZ) {
            toread0 = TRANS_SZ;
        } else {
            toread0 = bytesToGet;
        }

        myInput->status = sendTranHeader (destFd, GET_OPR, myInput->flags,
          myOffset, toread0);

        if (myInput->status < 0) {
            rodsLog (LOG_NOTICE,
              "partialDataGet: sendTranHeader error. status = %d",
              myInput->status);
            if (myInput->threadNum > 0)
                _l3Close (myInput->rsComm, srcRescTypeInx, srcL3descInx);
            CLOSE_SOCK (destFd);
            free (buf);
            return;
        }

        while (toread0 > 0) {
            int toread1;

            if (toread0 > TRANS_BUF_SZ) {
                toread1 = TRANS_BUF_SZ;
            } else {
                toread1 = toread0;
            }
	    bytesRead = _l3Read (myInput->rsComm, srcRescTypeInx,
             srcL3descInx, buf, toread1);

#ifdef PARA_TIMING
            tafterRead=time(0);
#endif
            if (bytesRead == toread1) {
                if ((bytesWritten = myWrite (destFd, buf, bytesRead,
		  SOCK_TYPE, NULL))
                  != bytesRead) {
                    rodsLog (LOG_NOTICE,
                     "_partialDataGet:Bytes written %d don't match read %d",
                      bytesWritten, bytesRead);

                    if (bytesWritten < 0) {
                        myInput->status = bytesWritten;
                    } else {
                        myInput->status = SYS_COPY_LEN_ERR;
                    }
                    break;
                }
                bytesToGet -= bytesWritten;
                toread0 -= bytesWritten;
                myOffset += bytesWritten;
            } else if (bytesRead < 0) {
                myInput->status = bytesRead;
                break;
            } else {        /* toread > 0 */
                rodsLog (LOG_NOTICE,
                 "_partialDataGet: toread %d bytes, %d bytes read",
                   toread1, bytesRead);
                myInput->status = SYS_COPY_LEN_ERR;
                break;
            }
#ifdef PARA_TIMING
            tafterWrite=time(0);
            rodsLog (LOG_NOTICE,
              "Thr %d: sz=%d netReadTm=%d diskWriteTm=%d",
              myInput->threadNum, bytesWritten, tafterRead-tstart,
              tafterWrite-tafterRead);
#endif
        }       /* while loop toread0 */
        if (myInput->status < 0)
            break;
    }           /* while loop bytesToGet */
#ifdef PARA_TIMING
    afterTransfer=time(0);
#endif
    free (buf);
    sendTranHeader (destFd, DONE_OPR, 0, 0, 0);
    if (myInput->threadNum > 0)
        _l3Close (myInput->rsComm, srcRescTypeInx, srcL3descInx);
    CLOSE_SOCK (destFd);
#ifdef PARA_TIMING
    endTime=time(0);
    rodsLog (LOG_NOTICE,
      "Thr %d: seekTm=%d transTm=%d endTm=%d",
      myInput->threadInx,
      afterSeek-afterConn, afterTransfer-afterSeek, endTime-afterTransfer);
#endif
    return;
}

void
remToLocPartialCopy (portalTransferInp_t *myInput)
{
    transferHeader_t myHeader;
    int destL3descInx, srcFd, destRescTypeInx;
    void *buf;
    rodsLong_t curOffset = 0;
    rodsLong_t myOffset = 0;
    int toRead, bytesRead, bytesWritten;

#ifdef PARA_DEBUG
    printf ("remToLocPartialCopy: thread %d at start\n", myInput->threadNum);
#endif
    if (myInput == NULL) {
        rodsLog (LOG_NOTICE,
         "remToLocPartialCopy: NULL input");
        return;
    }

    myInput->status = 0;
    destL3descInx = myInput->destFd;
    srcFd = myInput->srcFd;
    destRescTypeInx = myInput->destRescTypeInx;
    myInput->bytesWritten = 0;

    buf = malloc (TRANS_BUF_SZ);

    while (myInput->status >= 0) {
        rodsLong_t toGet;

        myInput->status = rcvTranHeader (srcFd, &myHeader);

#ifdef PARA_DEBUG
        printf ("remToLocPartialCopy: thread %d after rcvTranHeader\n",
          myInput->threadNum);
        printf ("remToLocPartialCopy: thread %d header offset %lld, len %lld\n",
          myInput->threadNum, myHeader.offset, myHeader.length);

#endif

        if (myInput->status < 0) {
            break;
        }

        if (myHeader.oprType == DONE_OPR) {
            break;
        }
        if (myHeader.offset != curOffset) {
            curOffset = myHeader.offset;
            myOffset = _l3Lseek (myInput->rsComm, destRescTypeInx,
              destL3descInx, myHeader.offset, SEEK_SET);
            if (myOffset < 0) {
                myInput->status = myOffset;
                rodsLog (LOG_NOTICE,
                  "remToLocPartialCopy: _objSeek error, status = %d ",
                  myInput->status);
                break;
	    }
        }

        toGet = myHeader.length;
        while (toGet > 0) {

            if (toGet > TRANS_BUF_SZ) {
                toRead = TRANS_BUF_SZ;
            } else {
                toRead = toGet;
            }

            bytesRead = myRead (srcFd, buf, toRead,
		  SOCK_TYPE, NULL);
            if (bytesRead != toRead) {
                rodsLog (LOG_NOTICE,
                  "remToLocPartialCopy: toGet %lld, bytesRead %d",
                  toGet, bytesRead);
		if (bytesRead < 0) {
		    myInput->status = bytesRead;
		} else {
                    myInput->status = SYS_COPY_LEN_ERR - errno;
		}
                break;
            }

	    bytesWritten = _l3Write (myInput->rsComm, destRescTypeInx,
              destL3descInx, buf, bytesRead);

            if (bytesWritten != bytesRead) {
                rodsLog (LOG_NOTICE,
                 "_partialDataPut:Bytes written %d don't match read %d",
                  bytesWritten, bytesRead);

                if (bytesWritten < 0) {
                    myInput->status = bytesWritten;
                } else {
                    myInput->status = SYS_COPY_LEN_ERR;
                }
                break;
            }

            toGet -= bytesWritten;
        }
        curOffset += myHeader.length;
        myInput->bytesWritten += myHeader.length;
    }

    free (buf);
    if (myInput->threadNum > 0)
        _l3Close (myInput->rsComm, destRescTypeInx, destL3descInx);
    CLOSE_SOCK (srcFd);
}

/* remLocCopy - This routine is very similar to rcPartialDataGet.
 */

int
remLocCopy (rsComm_t *rsComm, dataCopyInp_t *dataCopyInp)
{
    portalOprOut_t *portalOprOut;
    dataOprInp_t *dataOprInp;
    portList_t *myPortList;
    int i, sock, myFd;
    int numThreads;
    portalTransferInp_t myInput[MAX_NUM_CONFIG_TRAN_THR];
    pthread_t tid[MAX_NUM_CONFIG_TRAN_THR];
    int retVal = 0;
    rodsLong_t dataSize;
    int oprType;

    if (dataCopyInp == NULL) {
        rodsLog (LOG_NOTICE,
          "remLocCopy: NULL dataCopyInp input");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    portalOprOut = &dataCopyInp->portalOprOut;
    dataOprInp = &dataCopyInp->dataOprInp;

    oprType = dataOprInp->oprType;
    numThreads = portalOprOut->numThreads;
    dataSize = dataOprInp->dataSize;

    if (numThreads > MAX_NUM_CONFIG_TRAN_THR || numThreads <= 0) {
       rodsLog (LOG_NOTICE,
         "remLocCopy: numThreads %d out of range",
         numThreads);
        return (SYS_INVALID_PORTAL_OPR);
    }


    myPortList = &portalOprOut->portList;

    memset (tid, 0, sizeof (tid));
    memset (myInput, 0, sizeof (myInput));

    sock = connectToRhostPortal (myPortList->hostAddr,
      myPortList->portNum, myPortList->cookie, rsComm->windowSize);
    if (sock < 0) {
        return (sock);
    }

    if (oprType == COPY_TO_LOCAL_OPR) {
        fillPortalTransferInp (&myInput[0], rsComm,
         sock, dataOprInp->destL3descInx, 0, dataOprInp->destRescTypeInx,
          0, 0, 0, 0);
    } else {
        fillPortalTransferInp (&myInput[0], rsComm,
         dataOprInp->srcL3descInx, sock, dataOprInp->srcRescTypeInx, 0,
          0, 0, 0, 0);
    }

   if (numThreads == 1) {
	if (oprType == COPY_TO_LOCAL_OPR) {
            remToLocPartialCopy (&myInput[0]);
	} else {
	    locToRemPartialCopy (&myInput[0]);
	}
        if (myInput[0].status < 0) {
            return (myInput[0].status);
        } else {
            if (myInput[0].bytesWritten == dataSize) {
                return (0);
            } else {
                rodsLog (LOG_NOTICE,
                  "remLocCopy:bytesWritten %lld dataSize %lld mismatch",
                  myInput[0].bytesWritten, dataSize);
                return (SYS_COPY_LEN_ERR);
            }
        }
    } else {
#ifdef PARA_OPR
        rodsLong_t totalWritten = 0;

        for (i = 1; i < numThreads; i++) {
            sock = connectToRhostPortal (myPortList->hostAddr,
              myPortList->portNum, myPortList->cookie, rsComm->windowSize);
            if (sock < 0) {
                return (sock);
            }
	    if (oprType == COPY_TO_LOCAL_OPR) {
                myFd = l3OpenByHost (rsComm, dataOprInp->destRescTypeInx,
                 dataOprInp->destL3descInx, O_WRONLY);
                if (myFd < 0) {    /* error */
                    retVal = myFd;
                    rodsLog (LOG_NOTICE,
                    "remLocCopy: cannot open file, status = %d", 
	             myFd);
                    CLOSE_SOCK (sock);
                    continue;
                }

                fillPortalTransferInp (&myInput[i], rsComm,
                 sock, myFd, 0, dataOprInp->destRescTypeInx,
                 i, 0, 0, 0);

                pthread_create (&tid[i], pthread_attr_default,
                 (void *(*)(void *)) remToLocPartialCopy, (void *) &myInput[i]);
	    } else {
                myFd = l3OpenByHost (rsComm, dataOprInp->srcRescTypeInx,
                 dataOprInp->srcL3descInx, O_RDONLY);
                if (myFd < 0) {    /* error */
                    retVal = myFd;
                    rodsLog (LOG_NOTICE,
                    "remLocCopy: cannot open file, status = %d",
                     myFd);
                    CLOSE_SOCK (sock);
                    continue;
                }

                fillPortalTransferInp (&myInput[i], rsComm,
                 myFd, sock, dataOprInp->destRescTypeInx, 0,
                 i, 0, 0, 0);

                pthread_create (&tid[i], pthread_attr_default,
                 (void *(*)(void *)) locToRemPartialCopy, (void *) &myInput[i]);
            }
	}

	if (oprType == COPY_TO_LOCAL_OPR) {
            pthread_create (&tid[0], pthread_attr_default,
             (void *(*)(void *)) remToLocPartialCopy, (void *) &myInput[0]);
	} else {
            pthread_create (&tid[0], pthread_attr_default,
             (void *(*)(void *)) locToRemPartialCopy, (void *) &myInput[0]);
	}


        if (retVal < 0) {
            return (retVal);
        }

        for ( i = 0; i < numThreads; i++) {
            if (tid[i] != 0) {
                pthread_join (tid[i], NULL);
            }
            totalWritten += myInput[i].bytesWritten;
            if (myInput[i].status < 0) {
                retVal = myInput[i].status;
            }
        }
        if (retVal < 0) {
            return (retVal);
        } else {
            if (dataSize <= 0 || totalWritten == dataSize) {
                return (0);
            } else {
                rodsLog (LOG_NOTICE,
                 "remLocCopy: totalWritten %lld dataSize %lld mismatch",
                  totalWritten, dataSize);
                return (SYS_COPY_LEN_ERR);
            }
        }
#else   /* PARA_OPR */
        return (SYS_PARA_OPR_NO_SUPPORT);
#endif  /* PARA_OPR */
    }
}

int
sameHostCopy (rsComm_t *rsComm, dataCopyInp_t *dataCopyInp)
{
    dataOprInp_t *dataOprInp;
    int i, out_fd, in_fd;
    int numThreads;
    portalTransferInp_t myInput[MAX_NUM_CONFIG_TRAN_THR];
    pthread_t tid[MAX_NUM_CONFIG_TRAN_THR];
    int retVal = 0;
    rodsLong_t dataSize;
    rodsLong_t size0, size1, offset0;

    if (dataCopyInp == NULL) {
        rodsLog (LOG_NOTICE,
          "sameHostCopy: NULL dataCopyInp input");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    dataOprInp = &dataCopyInp->dataOprInp;

    numThreads = dataOprInp->numThreads;

    dataSize = dataOprInp->dataSize;

    if (numThreads > MAX_NUM_CONFIG_TRAN_THR || numThreads <= 0) {
       rodsLog (LOG_NOTICE,
         "sameHostCopy: numThreads %d out of range",
         numThreads);
        return (SYS_INVALID_PORTAL_OPR);
    }


    memset (tid, 0, sizeof (tid));
    memset (myInput, 0, sizeof (myInput));

    size0 = dataOprInp->dataSize / numThreads;
    size1 = dataOprInp->dataSize - size0 * (numThreads - 1);
    offset0 = dataOprInp->offset;

    fillPortalTransferInp (&myInput[0], rsComm,
     dataOprInp->srcL3descInx, dataOprInp->destL3descInx, 
     dataOprInp->srcRescTypeInx, dataOprInp->destRescTypeInx,
      0, size0, offset0, 0);

    if (numThreads == 1) {
        sameHostPartialCopy (&myInput[0]);
        return (myInput[0].status);
    } else {
#ifdef PARA_OPR
        rodsLong_t totalWritten = 0;
        rodsLong_t mySize = 0;
        rodsLong_t myOffset = 0;

        for (i = 1; i < numThreads; i++) {
            int l3descInx;

            myOffset += size0;
            if (i < numThreads - 1) {
                mySize = size0;
            } else {
                mySize = size1;
            }

            out_fd = l3OpenByHost (rsComm, dataOprInp->destRescTypeInx,
             dataOprInp->destL3descInx, O_WRONLY);
            if (out_fd < 0) {    /* error */
                retVal = out_fd;
                rodsLog (LOG_NOTICE,
                 "sameHostCopy: cannot open dest file, status = %d", 
		 out_fd);
                continue;
            }

            in_fd = l3OpenByHost (rsComm, dataOprInp->srcRescTypeInx,
             dataOprInp->srcL3descInx, O_RDONLY);
            if (in_fd < 0) {    /* error */
                retVal = out_fd;
                rodsLog (LOG_NOTICE,
                 "sameHostCopy: cannot open src file, status = %d", in_fd);
                continue;
            }
            fillPortalTransferInp (&myInput[i], rsComm,
             in_fd, out_fd, 
	     dataOprInp->srcRescTypeInx, dataOprInp->destRescTypeInx,
              i, mySize, myOffset, 0);

            pthread_create (&tid[i], pthread_attr_default,
             (void *(*)(void *)) sameHostPartialCopy, 
	     (void *) &myInput[i]);
        }

        pthread_create (&tid[0], pthread_attr_default,
         (void *(*)(void *)) sameHostPartialCopy, (void *) &myInput[0]);


        if (retVal < 0) {
            return (retVal);
        }

        for ( i = 0; i < numThreads; i++) {
            if (tid[i] != 0) {
                pthread_join (tid[i], NULL);
            }
            totalWritten += myInput[i].bytesWritten;
            if (myInput[i].status < 0) {
                retVal = myInput[i].status;
            }
        }
        if (retVal < 0) {
            return (retVal);
        } else {
            if (dataSize <= 0 || totalWritten == dataSize) {
                return (0);
            } else {
                rodsLog (LOG_NOTICE,
                 "sameHostCopy: totalWritten %lld dataSize %lld mismatch",
                  totalWritten, dataSize);
                return (SYS_COPY_LEN_ERR);
            }
        }
#else   /* PARA_OPR */
        return (SYS_PARA_OPR_NO_SUPPORT);
#endif  /* PARA_OPR */
    }
}

void
sameHostPartialCopy (portalTransferInp_t *myInput)
{
    transferHeader_t myHeader;
    int destL3descInx, srcL3descInx, destRescTypeInx, srcRescTypeInx;
    void *buf;
    rodsLong_t curOffset = 0;
    rodsLong_t myOffset = 0;
    rodsLong_t toCopy;
    int bytesRead, bytesWritten;

#ifdef PARA_DEBUG
    printf ("onsameHostPartialCopy: thread %d at start\n", 
      myInput->threadNum);
#endif
    if (myInput == NULL) {
        rodsLog (LOG_NOTICE,
         "onsameHostPartialCopy: NULL input");
        return;
    }

    myInput->status = 0;
    destL3descInx = myInput->destFd;
    srcL3descInx = myInput->srcFd;
    destRescTypeInx = myInput->destRescTypeInx;
    srcRescTypeInx = myInput->srcRescTypeInx;
    myInput->bytesWritten = 0;

    if (myInput->offset != 0) {
        myOffset = _l3Lseek (myInput->rsComm, destRescTypeInx,
          destL3descInx, myInput->offset, SEEK_SET);
        if (myOffset < 0) {
            myInput->status = myOffset;
            rodsLog (LOG_NOTICE,
              "sameHostPartialCopy: _objSeek error, status = %d ",
              myInput->status);
            if (myInput->threadNum > 0) {
                _l3Close (myInput->rsComm, destRescTypeInx, destL3descInx);
                _l3Close (myInput->rsComm, srcRescTypeInx, srcL3descInx);
            }
            return;
        }
        myOffset = _l3Lseek (myInput->rsComm, srcRescTypeInx,
          srcL3descInx, myInput->offset, SEEK_SET);
        if (myOffset < 0) {
            myInput->status = myOffset;
            rodsLog (LOG_NOTICE,
              "sameHostPartialCopy: _objSeek error, status = %d ",
              myInput->status);
            if (myInput->threadNum > 0) {
                _l3Close (myInput->rsComm, destRescTypeInx, destL3descInx);
                _l3Close (myInput->rsComm, srcRescTypeInx, srcL3descInx);
	    }
            return;
        }
    }

    buf = malloc (TRANS_BUF_SZ);

    toCopy = myInput->size;

    while (toCopy > 0) {
        int toRead;

        if (toCopy > TRANS_BUF_SZ) {
	    toRead = TRANS_BUF_SZ;
	} else {
	    toRead = toCopy;
	}

        bytesRead = _l3Read (myInput->rsComm, srcRescTypeInx,
         srcL3descInx, buf, toRead);

        if (bytesRead <= 0) {
            rodsLog (LOG_NOTICE,
              "sameHostPartialCopy: toCopy %lld, bytesRead %d",
              toCopy, bytesRead);
	    if (bytesRead < 0) {
		myInput->status = bytesRead;
	    } else {
                myInput->status = SYS_COPY_LEN_ERR - errno;
	    }
            break;
        }

	bytesWritten = _l3Write (myInput->rsComm, destRescTypeInx,
          destL3descInx, buf, bytesRead);

        if (bytesWritten != bytesRead) {
            rodsLog (LOG_NOTICE,
             "sameHostPartialCopy:Bytes written %d don't match read %d",
              bytesWritten, bytesRead);

            if (bytesWritten < 0) {
                myInput->status = bytesWritten;
            } else {
                myInput->status = SYS_COPY_LEN_ERR;
            }
            break;
        }

        toCopy -= bytesWritten;
        myInput->bytesWritten += bytesWritten;
    }

    free (buf);
    if (myInput->threadNum > 0) {
        _l3Close (myInput->rsComm, destRescTypeInx, destL3descInx);
        _l3Close (myInput->rsComm, srcRescTypeInx, srcL3descInx);
    }
}

void
locToRemPartialCopy (portalTransferInp_t *myInput)
{
    transferHeader_t myHeader;
    int srcL3descInx, destFd, srcRescTypeInx;
    void *buf;
    rodsLong_t curOffset = 0;
    rodsLong_t myOffset = 0;
    int toRead, bytesRead, bytesWritten;

#ifdef PARA_DEBUG
    printf ("locToRemPartialCopy: thread %d at start\n", myInput->threadNum);
#endif
    if (myInput == NULL) {
        rodsLog (LOG_NOTICE,
         "locToRemPartialCopy: NULL input");
        return;
    }

    myInput->status = 0;
    srcL3descInx = myInput->srcFd;
    destFd = myInput->destFd;
    srcRescTypeInx = myInput->srcRescTypeInx;
    myInput->bytesWritten = 0;

    buf = malloc (TRANS_BUF_SZ);

    while (myInput->status >= 0) {
        rodsLong_t toGet;

        myInput->status = rcvTranHeader (destFd, &myHeader);

#ifdef PARA_DEBUG
        printf ("locToRemPartialCopy: thread %d after rcvTranHeader\n",
          myInput->threadNum);
#endif

        if (myInput->status < 0) {
            break;
        }

        if (myHeader.oprType == DONE_OPR) {
            break;
        }
#ifdef PARA_DEBUG
	printf ("locToRemPartialCopy:thread %d header offset %lld, len %lld",
	  myHeader.offset, myHeader.length);
#endif

        if (myHeader.offset != curOffset) {
            curOffset = myHeader.offset;
            myOffset = _l3Lseek (myInput->rsComm, srcRescTypeInx,
              srcL3descInx, myHeader.offset, SEEK_SET);
            if (myOffset < 0) {
                myInput->status = myOffset;
                rodsLog (LOG_NOTICE,
                  "locToRemPartialCopy: _objSeek error, status = %d ",
                  myInput->status);
                break;
            }
        }

        toGet = myHeader.length;
        while (toGet > 0) {

            if (toGet > TRANS_BUF_SZ) {
                toRead = TRANS_BUF_SZ;
            } else {
                toRead = toGet;
            }

	    bytesRead = _l3Read (myInput->rsComm, srcRescTypeInx,
              srcL3descInx, buf, toRead);

            if (bytesRead != toRead) {
                rodsLog (LOG_NOTICE,
                  "locToRemPartialCopy: toGet %lld, bytesRead %d",
                  toGet, bytesRead);
                if (bytesRead < 0) {
                    myInput->status = bytesRead;
                } else {
                    myInput->status = SYS_COPY_LEN_ERR - errno;
                }
                break;
            }

	    bytesWritten = myWrite (destFd, buf, bytesRead,
		  SOCK_TYPE, NULL);


            if (bytesWritten != bytesRead) {
                rodsLog (LOG_NOTICE,
                 "_partialDataPut:Bytes written %d don't match read %d",
                  bytesWritten, bytesRead);

                if (bytesWritten < 0) {
                    myInput->status = bytesWritten;
                } else {
                    myInput->status = SYS_COPY_LEN_ERR;
                }
                break;
            }

            toGet -= bytesWritten;
        }
        curOffset += myHeader.length;
        myInput->bytesWritten += myHeader.length;
    }

    free (buf);
    if (myInput->threadNum > 0)
        _l3Close (myInput->rsComm, srcRescTypeInx, srcL3descInx);
    CLOSE_SOCK (destFd);
}

int
isUserPrivileged(rsComm_t *rsComm)
{

  if (rsComm->clientUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
  }
  if (rsComm->proxyUser.authInfo.authFlag < LOCAL_PRIV_USER_AUTH) {
      return(CAT_INSUFFICIENT_PRIVILEGE_LEVEL);
  }

  return(0);
}

#if !defined(solaris_platform)
char *regcmp (char *pat, char *end)
{
  return(NULL);
}

char *regex (char *rec, char *text, ...)
{
  return(NULL);
}
#endif  /* linux_platform */
