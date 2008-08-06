/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "rcPortalOpr.h"
#include "dataObjWrite.h"
#include "dataObjRead.h"
#include "dataObjOpr.h"
#include "rodsLog.h"
#ifdef PARA_OPR
#include <pthread.h>
#endif

int
sendTranHeader (int sock, int oprType, int flags, rodsLong_t offset,
rodsLong_t length)
{
    transferHeader_t myHeader;
    int retVal;

    myHeader.oprType = htonl (oprType);
    myHeader.flags = htonl (flags);
    myHtonll (offset, (rodsLong_t *) &myHeader.offset);
    myHtonll (length, (rodsLong_t *) &myHeader.length);

    retVal = myWrite (sock, (void *) &myHeader, sizeof (myHeader), 
      SOCK_TYPE, NULL);

    if (retVal != sizeof (myHeader)) {
        rodsLog (LOG_ERROR,
         "sendTranHeader: toWrite = %d, written = %d",
          sizeof (myHeader), retVal);
        if (retVal < 0)
            return (retVal);
        else
            return (SYS_COPY_LEN_ERR);
    } else {
        return (0);
    }
}

int
rcvTranHeader (int sock, transferHeader_t *myHeader)
{
    int retVal;
    transferHeader_t tmpHeader;

    retVal = myRead (sock, (void *) &tmpHeader, sizeof (tmpHeader),
      SOCK_TYPE, NULL);

    if (retVal != sizeof (tmpHeader)) {
        rodsLog (LOG_ERROR,
         "rcvTranHeader: toread = %d, read = %d",
          sizeof (tmpHeader), retVal);
        if (retVal < 0)
            return (retVal);
        else
            return (SYS_COPY_LEN_ERR);
    }

    myHeader->oprType = htonl (tmpHeader.oprType);
    myHeader->flags = htonl (tmpHeader.flags);
    myNtohll (tmpHeader.offset, &myHeader->offset);
    myNtohll (tmpHeader.length, &myHeader->length);

    return (0);
}

int
fillBBufWithFile (rcComm_t *conn, bytesBuf_t *myBBuf, char *locFilePath, 
rodsLong_t dataSize)
{
    int in_fd, status;

    if (dataSize > 10 * MAX_SZ_FOR_SINGLE_BUF) {
	rodsLog (LOG_ERROR,
	  "fillBBufWithFile: dataSize %lld too large", dataSize);
	return (USER_FILE_TOO_LARGE);
    } else if (dataSize > MAX_SZ_FOR_SINGLE_BUF) {
        rodsLog (LOG_NOTICE,
          "fillBBufWithFile: dataSize %lld too large", dataSize);
    }

#ifdef windows_platform
	in_fd = iRODSNt_bopen(locFilePath, O_RDONLY,0);
#else
    in_fd = open (locFilePath, O_RDONLY, 0, FILE_DESC_TYPE, NULL);
#endif
    if (in_fd < 0) { /* error */
	status = USER_FILE_DOES_NOT_EXIST - errno;
	rodsLogError (LOG_ERROR, status,
	"cannot open file %s, status = %d", locFilePath, status);
	return (status);
    }
    

    myBBuf->buf = malloc (dataSize);
    myBBuf->len = dataSize;
    conn->transStat.bytesWritten = dataSize;

    status = myRead (in_fd, myBBuf->buf, (int) dataSize, FILE_DESC_TYPE,
      NULL);

    close (in_fd);

    return (status); 
}

int
putFileToPortal (rcComm_t *conn, portalOprOut_t *portalOprOut, 
char *locFilePath, rodsLong_t dataSize)
{
    portList_t *myPortList;
    int i, sock, in_fd;
    int numThreads; 
    rcPortalTransferInp_t myInput[MAX_NUM_CONFIG_TRAN_THR];
#ifdef PARA_OPR
    pthread_t tid[MAX_NUM_CONFIG_TRAN_THR];
#endif
    int retVal = 0;

    if (portalOprOut == NULL || portalOprOut->numThreads <= 0) {
        rodsLog (LOG_ERROR,
         "putFileToPortal: invalid portalOprOut");
        return (SYS_INVALID_PORTAL_OPR);
    }

    numThreads = portalOprOut->numThreads;

    myPortList = &portalOprOut->portList;

    if (portalOprOut->numThreads > MAX_NUM_CONFIG_TRAN_THR) {
        for (i = 0; i < portalOprOut->numThreads; i++) {
            sock = connectToRhostPortal (myPortList->hostAddr,
              myPortList->portNum, myPortList->cookie, myPortList->windowSize);
            if (sock > 0) {
                close (sock);
            }
        }
       rodsLog (LOG_ERROR,
         "putFileToPortal: numThreads %d too large", 
	 portalOprOut->numThreads);
        return (SYS_INVALID_PORTAL_OPR);
    }

#ifdef PARA_OPR
    memset (tid, 0, sizeof (tid));
#endif
    memset (myInput, 0, sizeof (myInput));

   if (numThreads == 1) {
        sock = connectToRhostPortal (myPortList->hostAddr, 
	  myPortList->portNum, myPortList->cookie, myPortList->windowSize);
        if (sock < 0) {
	    return (sock);
        }
        in_fd = open (locFilePath, O_RDONLY, 0);
        if (in_fd < 0) { /* error */
            retVal = USER_FILE_DOES_NOT_EXIST - errno;
            rodsLogError (LOG_ERROR, retVal,
             "cannot open file %s, status = %d", locFilePath, retVal);
            return (retVal);
        }
	fillRcPortalTransferInp (conn, &myInput[0], sock, in_fd, 0);
	rcPartialDataPut (&myInput[0]);
	if (myInput[0].status < 0) {
	    return (myInput[0].status);
	} else {
	    if (dataSize <= 0 || myInput[0].bytesWritten == dataSize) {
		return (0);
	    } else {
		rodsLog (LOG_ERROR,
		  "putFileToPortal: bytesWritten %lld dataSize %lld mismatch",
		  myInput[0].bytesWritten, dataSize);
	        return (SYS_COPY_LEN_ERR);
	    }
	}
    } else {
#ifdef PARA_OPR
        rodsLong_t totalWritten = 0;

	for (i = 0; i < numThreads; i++) {
            sock = connectToRhostPortal (myPortList->hostAddr,
              myPortList->portNum, myPortList->cookie, myPortList->windowSize);
            if (sock < 0) {
                return (sock);
            }
            in_fd = open (locFilePath, O_RDONLY, 0);
            if (in_fd < 0) { 	/* error */
                retVal = USER_FILE_DOES_NOT_EXIST - errno;
                rodsLogError (LOG_ERROR, retVal,
                 "cannot open file %s, status = %d", locFilePath, retVal);
		continue;
            }
            fillRcPortalTransferInp (conn, &myInput[i], sock, in_fd, i);
            pthread_create (&tid[i], pthread_attr_default,
             (void *(*)(void *)) rcPartialDataPut, (void *) &myInput[i]);
        }
	if (retVal < 0)
	    return (retVal);

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
                rodsLog (LOG_ERROR,
                  "putFileToPortal: totalWritten %lld dataSize %lld mismatch",
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
fillRcPortalTransferInp (rcComm_t *conn, rcPortalTransferInp_t *myInput, 
int destFd, int srcFd, int threadNum)
{
    if (myInput == NULL)
        return (SYS_INTERNAL_NULL_INPUT_ERR);

    myInput->conn = conn;
    myInput->destFd = destFd;
    myInput->srcFd = srcFd;
    myInput->threadNum = threadNum;

    return (0);
}

void
rcPartialDataPut (rcPortalTransferInp_t *myInput)
{
    transferHeader_t myHeader;
    int destFd;
    int srcFd;
    void *buf;
    transStat_t *myTransStat;
    rodsLong_t curOffset = 0;

#ifdef PARA_DEBUG
    printf ("rcPartialDataPut: thread %d at start\n", myInput->threadNum);
#endif
    if (myInput == NULL) {
	rodsLog (LOG_ERROR,
	 "rcPartialDataPut: NULL input");
	return;
    }

    myTransStat = &myInput->conn->transStat;

    destFd = myInput->destFd;
    srcFd = myInput->srcFd;

    buf = malloc (TRANS_BUF_SZ);

    myInput->bytesWritten = 0;
    while (myInput->status >= 0) {
	rodsLong_t toPut;

        myInput->status = rcvTranHeader (destFd, &myHeader);

#ifdef PARA_DEBUG
        printf ("rcPartialDataPut: thread %d after rcvTranHeader\n", 
          myInput->threadNum);
#endif

        if (myInput->status < 0) {
	    break;
        }

	if (myHeader.oprType == DONE_OPR) {
	    break;
        }
	if (myHeader.offset != curOffset) {
	    curOffset = myHeader.offset;
	    if (lseek (srcFd, curOffset, SEEK_SET) < 0) {
		myInput->status = UNIX_FILE_LSEEK_ERR - errno;
		rodsLogError (LOG_ERROR, myInput->status,
		  "rcPartialDataPut: lseek to %lld error, status = %d",
		  curOffset, myInput->status);
		break;
	    }
	}

	toPut = myHeader.length;
	while (toPut > 0) {
	    int toRead, bytesRead, bytesWritten;

	    if (toPut > TRANS_BUF_SZ) {
		toRead = TRANS_BUF_SZ;
	    } else {
		toRead = toPut;
	    } 

	    bytesRead = myRead (srcFd, buf, toRead, FILE_DESC_TYPE, 
	      &bytesRead);
	    if (bytesRead != toRead) {
		myInput->status = SYS_COPY_LEN_ERR - errno;
		rodsLogError (LOG_ERROR, myInput->status,
		  "rcPartialDataPut: toPut %lld, bytesRead %d",
		  toPut, bytesRead);   
		break;
	    }
	    bytesWritten = myWrite (destFd, buf, bytesRead, SOCK_TYPE,
	      &bytesWritten);

	    if (bytesWritten != bytesRead) {
                myInput->status = SYS_COPY_LEN_ERR - errno;
		rodsLogError (LOG_ERROR, myInput->status,
                  "rcPartialDataPut: toWrite %d, bytesWritten %d, errno = %d",
                  bytesRead, bytesWritten, errno);
                break;
	    }
	    toPut -= bytesWritten;
	}
	curOffset += myHeader.length;
	myInput->bytesWritten += myHeader.length;
	/* should lock this. But window browser is the only one using it */ 
	myTransStat->bytesWritten += myHeader.length;
    }

    free (buf);
    close (srcFd);
    CLOSE_SOCK (destFd);
}

int
putFile (rcComm_t *conn, int l1descInx, char *locFilePath, 
rodsLong_t dataSize)
{
    int in_fd, status;
    bytesBuf_t dataObjWriteInpBBuf;
    dataObjWriteInp_t dataObjWriteInp;
    int bytesWritten;
    rodsLong_t totalWritten = 0;
    int bytesRead;

    in_fd = open (locFilePath, O_RDONLY, 0);
    if (in_fd < 0) { /* error */
        status = USER_FILE_DOES_NOT_EXIST - errno;
        rodsLogError (LOG_ERROR, status,
        "cannot open file %s, status = %d", locFilePath, status);
        return (status);
    }

    dataObjWriteInpBBuf.buf = malloc (TRANS_BUF_SZ);
    dataObjWriteInpBBuf.len = 0;
    dataObjWriteInp.l1descInx = l1descInx;

    while ((dataObjWriteInpBBuf.len =
      myRead (in_fd, dataObjWriteInpBBuf.buf, TRANS_BUF_SZ, FILE_DESC_TYPE,
      &bytesRead)) > 0) {
        /* Write to the data object */

        dataObjWriteInp.len = dataObjWriteInpBBuf.len;
        bytesWritten = rcDataObjWrite (conn, &dataObjWriteInp,
          &dataObjWriteInpBBuf);
        if (bytesWritten < dataObjWriteInp.len) {
           rodsLog (LOG_ERROR,
	    "putFile: Read %d bytes, Wrote %d bytes.\n ",
            dataObjWriteInp.len, bytesWritten);
	    free (dataObjWriteInpBBuf.buf);
	    close (in_fd);
	    return (SYS_COPY_LEN_ERR);
        } else {
            totalWritten += bytesWritten;
	    conn->transStat.bytesWritten = totalWritten;
	}
    }

    free (dataObjWriteInpBBuf.buf);
    close (in_fd);

    if (dataSize <= 0 || totalWritten == dataSize) {
        return (0);
    } else {
        rodsLog (LOG_ERROR,
          "putFile: totalWritten %lld dataSize %lld mismatch",
          totalWritten, dataSize);
        return (SYS_COPY_LEN_ERR);
    }
}

int
getIncludeFile (rcComm_t *conn, bytesBuf_t *dataObjOutBBuf, char *locFilePath)
{
    int status, out_fd, bytesWritten;

    if (strcmp (locFilePath, STDOUT_FILE_NAME) == 0) {
	if (dataObjOutBBuf->len <= 0) {
	    return (0);
	}
	bytesWritten = fwrite (dataObjOutBBuf->buf, dataObjOutBBuf->len,
	  1, stdout);
	if (bytesWritten == 1)
	    bytesWritten = dataObjOutBBuf->len;
    } else { 
#ifdef windows_platform
		out_fd = iRODSNt_bopen(locFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0640);
#else
        out_fd = open (locFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0640);
#endif
        if (out_fd < 0) { /* error */
            status = USER_FILE_DOES_NOT_EXIST - errno;
            rodsLogError (LOG_ERROR, status,
            "cannot open file %s, status = %d", locFilePath, status);
            return (status);
        }

        if (dataObjOutBBuf->len <= 0) {
	    close (out_fd);
            return 0;
        } 

        bytesWritten = myWrite (out_fd, dataObjOutBBuf->buf, 
	  dataObjOutBBuf->len, FILE_DESC_TYPE, NULL);

        close (out_fd);
    }
    if (bytesWritten != dataObjOutBBuf->len) {
       rodsLog (LOG_ERROR,
        "getIncludeFile: Read %d bytes, Wrote %d bytes.\n ",
        dataObjOutBBuf->len, bytesWritten);
        return (SYS_COPY_LEN_ERR);
    } else {
	conn->transStat.bytesWritten = bytesWritten;
        return (0);
    }
}

int
getFile (rcComm_t *conn, int l1descInx, char *locFilePath,
rodsLong_t dataSize)
{
    int out_fd, status;
    bytesBuf_t dataObjReadInpBBuf;
    dataObjReadInp_t dataObjReadInp;
    int bytesWritten, bytesRead;
    rodsLong_t totalWritten = 0;

    if (strcmp (locFilePath, STDOUT_FILE_NAME) == 0) {
	/* streaming to stdout */
        out_fd =1;
    } else {
        out_fd = open (locFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0640);
    }
 
    if (out_fd < 0) { /* error */
        status = USER_FILE_DOES_NOT_EXIST - errno;
        rodsLogError (LOG_ERROR, status,
        "cannot open file %s, status = %d", locFilePath, status);
        return (status);
    }

    dataObjReadInpBBuf.buf = malloc (TRANS_BUF_SZ);
    dataObjReadInpBBuf.len = dataObjReadInp.len = TRANS_BUF_SZ;
    dataObjReadInp.l1descInx = l1descInx;

    while ((bytesRead = rcDataObjRead (conn, &dataObjReadInp, 
      &dataObjReadInpBBuf)) > 0) {

	if (out_fd == 1) {
            bytesWritten = fwrite (dataObjReadInpBBuf.buf, bytesRead,
              1, stdout);
            if (bytesWritten == 1)
		bytesWritten = bytesRead;
	} else {
            bytesWritten = myWrite (out_fd, dataObjReadInpBBuf.buf, 
	      bytesRead, FILE_DESC_TYPE, NULL);
	}

        if (bytesWritten != bytesRead) {
           rodsLog (LOG_ERROR,
            "getFile: Read %d bytes, Wrote %d bytes.\n ",
            bytesRead, bytesWritten);
            free (dataObjReadInpBBuf.buf);
	    if (out_fd != 1) 
                close (out_fd);
            return (SYS_COPY_LEN_ERR);
        } else {
            totalWritten += bytesWritten;
	    conn->transStat.bytesWritten = totalWritten;
        }
    }

    free (dataObjReadInpBBuf.buf);
    if (out_fd != 1)
        close (out_fd);

    if (dataSize <= 0 || totalWritten == dataSize) {
        return (0);
    } else {
        rodsLog (LOG_ERROR,
          "getFile: totalWritten %lld dataSize %lld mismatch",
          totalWritten, dataSize);
        return (SYS_COPY_LEN_ERR);
    }
}

int
getFileFromPortal (rcComm_t *conn, portalOprOut_t *portalOprOut, 
char *locFilePath, rodsLong_t dataSize)
{
    portList_t *myPortList;
    int i, sock, out_fd;
    int numThreads;
    rcPortalTransferInp_t myInput[MAX_NUM_CONFIG_TRAN_THR];
#ifdef PARA_OPR
    pthread_t tid[MAX_NUM_CONFIG_TRAN_THR];
#endif
    int retVal = 0;

    if (portalOprOut == NULL || portalOprOut->numThreads <= 0) {
        rodsLog (LOG_ERROR,
         "getFileFromPortal: invalid portalOprOut");
        return (SYS_INVALID_PORTAL_OPR);
    }

    numThreads = portalOprOut->numThreads;

    myPortList = &portalOprOut->portList;

    if (portalOprOut->numThreads > MAX_NUM_CONFIG_TRAN_THR) {
	/* drain the connection or it will be stuck */
	for (i = 0; i < numThreads; i++) {
            sock = connectToRhostPortal (myPortList->hostAddr,
              myPortList->portNum, myPortList->cookie, myPortList->windowSize);
	    if (sock > 0) {
		close (sock);
	    }
	}
        rodsLog (LOG_ERROR,
         "getFileFromPortal: numThreads %d too large", numThreads);
        return (SYS_INVALID_PORTAL_OPR);
    }

#ifdef PARA_OPR
    memset (tid, 0, sizeof (tid));
#endif
    memset (myInput, 0, sizeof (myInput));

   if (numThreads == 1) {
        sock = connectToRhostPortal (myPortList->hostAddr,
          myPortList->portNum, myPortList->cookie, myPortList->windowSize);
        if (sock < 0) {
            return (sock);
        }
        out_fd = open (locFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0640);
        if (out_fd < 0) { /* error */
	    retVal = USER_FILE_DOES_NOT_EXIST - errno;
            rodsLogError (LOG_ERROR, retVal,
            "cannot open file %s, status = %d", locFilePath, retVal);
            return (retVal);
        }
        fillRcPortalTransferInp (conn, &myInput[0], out_fd, sock, 0640);
        rcPartialDataGet (&myInput[0]);
        if (myInput[0].status < 0) {
            return (myInput[0].status);
        } else {
            if (dataSize <= 0 || myInput[0].bytesWritten == dataSize) {
                return (0);
            } else {
                rodsLog (LOG_ERROR,
                  "getFileFromPortal:bytesWritten %lld dataSize %lld mismatch",
                  myInput[0].bytesWritten, dataSize);
                return (SYS_COPY_LEN_ERR);
            }
        }
    } else {
#ifdef PARA_OPR
        rodsLong_t totalWritten = 0;

        for (i = 0; i < numThreads; i++) {
            sock = connectToRhostPortal (myPortList->hostAddr,
              myPortList->portNum, myPortList->cookie, myPortList->windowSize);
            if (sock < 0) {
                return (sock);
            }
	    if (i == 0) { 
                out_fd = open (locFilePath, O_WRONLY | O_CREAT | O_TRUNC,0640);
	    } else {
                out_fd = open (locFilePath, O_WRONLY, 0640);
	    }
            if (out_fd < 0) {    /* error */
                retVal = USER_FILE_DOES_NOT_EXIST - errno;
                rodsLogError (LOG_ERROR, retVal,
                "cannot open file %s, status = %d", locFilePath, retVal);
		CLOSE_SOCK (sock);
		continue;
            }
            fillRcPortalTransferInp (conn, &myInput[i], out_fd, sock, i);
            pthread_create (&tid[i], pthread_attr_default,
             (void *(*)(void *)) rcPartialDataGet, (void *) &myInput[i]);
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
                rodsLog (LOG_ERROR,
                  "getFileFromPortal: totalWritten %lld dataSize %lld mismatch",
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
rcPartialDataGet (rcPortalTransferInp_t *myInput)
{
    transferHeader_t myHeader;
    int destFd;
    int srcFd;
    void *buf;
    transStat_t *myTransStat;
    rodsLong_t curOffset = 0;

#ifdef PARA_DEBUG
    printf ("rcPartialDataGet: thread %d at start\n", myInput->threadNum);
#endif
    if (myInput == NULL) {
        rodsLog (LOG_ERROR,
         "rcPartialDataGet: NULL input");
        return;
    }

    myTransStat = &myInput->conn->transStat;

    destFd = myInput->destFd;
    srcFd = myInput->srcFd;

    buf = malloc (TRANS_BUF_SZ);

    myInput->bytesWritten = 0;
    while (myInput->status >= 0) {
        rodsLong_t toGet;

        myInput->status = rcvTranHeader (srcFd, &myHeader);

#ifdef PARA_DEBUG
        printf ("rcPartialDataGet: thread %d after rcvTranHeader\n",
          myInput->threadNum);
#endif

        if (myInput->status < 0) {
            break;
        }

        if (myHeader.oprType == DONE_OPR) {
            break;
        }
        if (myHeader.offset != curOffset) {
            curOffset = myHeader.offset;
            if (lseek (destFd, curOffset, SEEK_SET) < 0) {
                myInput->status = UNIX_FILE_LSEEK_ERR - errno;
                rodsLogError (LOG_ERROR, myInput->status,
                  "rcPartialDataGet: lseek to %lld error, status = %d",
                  curOffset, myInput->status);
                break;
            }
        }

        toGet = myHeader.length;
        while (toGet > 0) {
            int toRead, bytesRead, bytesWritten;

            if (toGet > TRANS_BUF_SZ) {
                toRead = TRANS_BUF_SZ;
            } else {
                toRead = toGet;
            }

            bytesRead = myRead (srcFd, buf, toRead, SOCK_TYPE, &bytesRead);
            if (bytesRead != toRead) {
                myInput->status = SYS_COPY_LEN_ERR - errno;
                rodsLogError (LOG_ERROR, myInput->status,
                  "rcPartialDataGet: toGet %lld, bytesRead %d",
                  toGet, bytesRead);
                break;
            }
            bytesWritten = myWrite (destFd, buf, bytesRead, FILE_DESC_TYPE,
	      &bytesWritten);

            if (bytesWritten != bytesRead) {
                myInput->status = SYS_COPY_LEN_ERR - errno;
                rodsLogError (LOG_ERROR, myInput->status,
                  "rcPartialDataGet: toWrite %d, bytesWritten %d",
                  bytesRead, bytesWritten);
                break;
            }
            toGet -= bytesWritten;
        }
        curOffset += myHeader.length;
        myInput->bytesWritten += myHeader.length;
        /* should lock this. But window browser is the only one using it */
        myTransStat->bytesWritten += myHeader.length;
    }

    free (buf);
    close (destFd);
    CLOSE_SOCK (srcFd);
}

#ifdef RBUDP_TRANSFER
int
putFileToPortalRbudp (rcComm_t *conn, portalOprOut_t *portalOprOut, 
char *locFilePath, rodsLong_t dataSize, int veryVerbose)
{
    portList_t *myPortList;
    int status;
    rbudpSender_t rbudpSender;
    int sendRate, packetSize;
    char *tmpStr;

    if (portalOprOut == NULL || portalOprOut->numThreads != 1) {
        rodsLog (LOG_ERROR,
         "putFileToPortal: invalid portalOprOut");
        return (SYS_INVALID_PORTAL_OPR);
    }

    myPortList = &portalOprOut->portList;

    bzero (&rbudpSender, sizeof (rbudpSender));
    status = initRbudpClient (&rbudpSender.rbudpBase, myPortList);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "putFileToPortalRbudp: initRbudpClient error for %s", 
	  myPortList->hostAddr);
        return (status);
    }
    rbudpSender.rbudpBase.verbose = veryVerbose;
    if ((tmpStr = getenv (RBUDP_SEND_RATE_KW)) != NULL) {
	sendRate = atoi (tmpStr);
    } else {
	sendRate = DEF_UDP_SEND_RATE;
    }
    if ((tmpStr = getenv (RBUDP_PACK_SIZE_KW)) != NULL) {
	packetSize = atoi (tmpStr);
    } else {
	packetSize = DEF_UDP_PACKET_SIZE;
    }

    status = sendfile (&rbudpSender, sendRate, packetSize, 
      locFilePath);

    sendClose (&rbudpSender);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "putFileToPortalRbudp: sendfile error for %s", 
	  myPortList->hostAddr);
        return (status);
    }
    return (status);
}

int
getFileToPortalRbudp (rcComm_t *conn, portalOprOut_t *portalOprOut, 
char *locFilePath, rodsLong_t dataSize, int veryVerbose)
{
    portList_t *myPortList;
    int status;
    rbudpReceiver_t rbudpReceiver;
    int packetSize;
    char *tmpStr;

    if (portalOprOut == NULL || portalOprOut->numThreads != 1) {
        rodsLog (LOG_ERROR,
         "getFileToPortalRbudp: invalid portalOprOut");
        return (SYS_INVALID_PORTAL_OPR);
    }

    myPortList = &portalOprOut->portList;

    bzero (&rbudpReceiver, sizeof (rbudpReceiver));
    status = initRbudpClient (&rbudpReceiver.rbudpBase, myPortList);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "getFileToPortalRbudp: initRbudpClient error for %s", 
	  myPortList->hostAddr);
        return (status);
    }
    rbudpReceiver.rbudpBase.verbose = veryVerbose;
    if ((tmpStr = getenv (RBUDP_PACK_SIZE_KW)) != NULL) {
	packetSize = atoi (tmpStr);
    } else {
	packetSize = DEF_UDP_PACKET_SIZE;
    }

    status = getfile (&rbudpReceiver, NULL, locFilePath, packetSize);

    recvClose (&rbudpReceiver);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "getFileToPortalRbudp: getfile error for %s", 
	  myPortList->hostAddr);
        return (status);
    }
    return (status);
}

int
initRbudpClient (rbudpBase_t *rbudpBase, portList_t *myPortList)
{ 
    int  tcpSock;
    int tcpPort, udpPort;
    int status;
    struct sockaddr_in localUdpAddr;
    int udpLocalPort;

    if ((udpPort = getUdpPortFromPortList (myPortList)) == 0) {
        rodsLog (LOG_ERROR,
         "putFileToPortalRbudp: udpPort == 0");
        return (SYS_INVALID_PORTAL_OPR);
    }
   
    tcpPort = getTcpPortFromPortList (myPortList);

    tcpSock = connectToRhostPortal (myPortList->hostAddr,
      tcpPort, myPortList->cookie, myPortList->windowSize);
    if (tcpSock < 0) {
        return (tcpSock);
    }

    rbudpBase->udpSockBufSize = UDPSOCKBUF;
    rbudpBase->tcpPort = tcpPort;
    rbudpBase->tcpSockfd = tcpSock;
    rbudpBase->hasTcpSock = 1;
    rbudpBase->udpRemotePort = udpPort;

    /* connect to the server's UDP port */
    status = passiveUDP (rbudpBase, myPortList->hostAddr);

    if (status < 0) {
        rodsLog (LOG_ERROR,
         "initRbudpClient: passiveUDP connect to %s error. status = %d", 
	  myPortList->hostAddr, status);
        return (SYS_UDP_CONNECT_ERR + status);
    }

    /* inform the server of the UDP port */
    rbudpBase->udpLocalPort = 
      setLocalAddr (rbudpBase->udpSockfd, &localUdpAddr);
    if (rbudpBase->udpLocalPort < 0) 
        return rbudpBase->udpLocalPort;
    udpLocalPort = htonl (rbudpBase->udpLocalPort);
    status = writen (rbudpBase->tcpSockfd, (char *) &udpLocalPort, 
      sizeof (udpLocalPort));
    if (status != sizeof (udpLocalPort)) {
        rodsLog (LOG_ERROR,
         "initRbudpClient: writen error. towrite %d, bytes written %d ",
          sizeof (udpLocalPort), status);
        return (SYS_UDP_CONNECT_ERR);
    }

    return 0;
}
#endif  /* RBUDP_TRANSFER */

