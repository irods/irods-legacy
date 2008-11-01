/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* sockComm.c - sock communication routines 
 */

#include "sockComm.h"
#include "rcMisc.h"
#include "rcGlobalExtern.h"
#include "miscServerFunct.h"
#ifdef RBUDP_TRANSFER
#include "QUANTAnet_rbudpBase_c.h"
#endif  /* RBUDP_TRANSFER */


#ifdef _WIN32
#include <mmsystem.h>
int win_connect_timeout;
MMRESULT win_connect_timer_id;
#endif

#ifndef _WIN32

#include <setjmp.h>
jmp_buf Jcenv;

void
connToutHandler (int sig)
{
    alarm (0);
    longjmp (Jcenv, 1);
}
#endif  /* _WIN32 */

/* open sock for incoming connection */
int 
sockOpenForInConn (rsComm_t *rsComm, int *portNum, char **addr, int proto)
{
    struct sockaddr_in  mySockAddr;
    int sock;
    int status;
    int svrPortRangeStart, svrPortRangeEnd;
    char *tmpPtr;
 
    if (proto != SOCK_DGRAM && proto != SOCK_STREAM) {
        rodsLog (LOG_ERROR,
         "sockOpenForInConn() -- invalid input protocol %d", proto);
        return SYS_INVALID_PROTOCOL_TYPE;
    }

    memset((char *) &mySockAddr, 0, sizeof mySockAddr);

    sock = socket (AF_INET, proto, 0);

    if (sock < 0) {
	status = SYS_SOCK_OPEN_ERR - errno;
	rodsLogError (LOG_NOTICE, status,
	  "sockOpenForInConn: open socket error. status = %d", status);
	return (status);
    }

    /* For SOCK_DGRAM, done in checkbuf */
    if (proto == SOCK_STREAM) {
        rodsSetSockOpt (sock, rsComm->windowSize);
    }

    mySockAddr.sin_family = AF_INET;

    /* if portNum is <= 0 and env svrPortRangeStart is set, pick a port
     * in the range.
     */

    if (*portNum <= 0 && (tmpPtr = getenv ("svrPortRangeStart")) != NULL) {
	int portRangeCount;
	int bindCnt = 0;
        int myPortNum;

	svrPortRangeStart = atoi (tmpPtr);
	if ((tmpPtr = getenv ("svrPortRangeEnd")) != NULL) { 
	    svrPortRangeEnd = atoi (tmpPtr);
	    if (svrPortRangeEnd < svrPortRangeStart) {
                rodsLog (LOG_ERROR,
                  "sockOpenForInConn: PortRangeStart %d > PortRangeEnd %d",
		  svrPortRangeStart, svrPortRangeEnd);
		svrPortRangeEnd = svrPortRangeStart + DEF_NUMBER_SVR_PORT - 1;
	    }
	} else {
	    svrPortRangeEnd = svrPortRangeStart + DEF_NUMBER_SVR_PORT - 1;
	}
	portRangeCount = svrPortRangeEnd - svrPortRangeStart + 1;

	myPortNum = svrPortRangeStart + random() % portRangeCount;
	while (bindCnt < portRangeCount) {
	    if (myPortNum > svrPortRangeEnd) {
		myPortNum = svrPortRangeStart;
	    }
            mySockAddr.sin_port = htons(myPortNum);

            if ((status = bind (sock, (struct sockaddr *) &mySockAddr,
              sizeof mySockAddr)) >= 0) {
		*portNum = myPortNum;
        	rodsLog (LOG_DEBUG,
	  	 "sockOpenForInConn: port number = %d", myPortNum);
		break;
	    }
	    bindCnt ++;
	    myPortNum ++;
	}
 
    } else {
        mySockAddr.sin_port = htons(*portNum);
        status = bind (sock, (struct sockaddr *) &mySockAddr, 
          sizeof mySockAddr);
    }

    if (status < 0) { 
	status = SYS_SOCK_BIND_ERR - errno;
        rodsLog (LOG_NOTICE,
	  "sockOpenForInConn: bind socket error. portNum = %d, errno = %d", 
	  *portNum, errno);
        return (status);   
    }

    if (addr != NULL) {
        struct sockaddr_in sin;
#if defined(aix_platform)
	size_t length = sizeof (sin);
#elif defined(windows_platform)
	int length;
#else
        uint length = sizeof (sin);
#endif
        if (getsockname (sock, (struct sockaddr *) &sin, &length)) {
            rodsLog (LOG_NOTICE,
            "sockOpenForInConn() -- getsockname() failed: errno=%d", errno);
            return SYS_SOCK_BIND_ERR - errno;
	}
	*portNum = ntohs (sin.sin_port);
        *addr =  strdup (rods_inet_ntoa (sin.sin_addr));
    }

    return (sock);
}

/* rsAcceptConn - Server accept connection */

int 
rsAcceptConn (rsComm_t *svrComm)
{
    socklen_t len;
    int newSock;
    int status;

    len = sizeof (svrComm->remoteAddr);
    newSock = accept (svrComm->sock, 
      (struct sockaddr *) &svrComm->remoteAddr, &len);

    if (newSock < 0) {
        status = SYS_SOCK_ACCEPT_ERR - errno;
	rodsLogError (LOG_NOTICE, status,
	  "rsAcceptConn: accept error for socket %d, status = %d", 
	 svrComm->sock, status);
    }
    
    return (newSock);
}

int
readMsgHeader (int sock, msgHeader_t *myHeader)
{
    int nbytes;
    int myLen;
    char tmpBuf[MAX_NAME_LEN]; 
    msgHeader_t *outHeader;
    int status;
    
    /* read the header length packet */

    nbytes = myRead (sock, (void *) &myLen, sizeof (myLen), SOCK_TYPE, NULL);

    if (nbytes != sizeof (myLen)) {
	rodsLog (LOG_ERROR,
         "readMsgHeader:header read- read %d bytes, expect %d, status = %d",
         nbytes, sizeof (myLen), SYS_HEADER_READ_LEN_ERR - errno);
         return (SYS_HEADER_READ_LEN_ERR - errno);
    }

    myLen =  ntohl (myLen);

    if (myLen > MAX_NAME_LEN || myLen <= 0) {
        rodsLog (LOG_ERROR,
         "readMsgHeader: header length %d out of range",
         myLen);
         return (SYS_HEADER_READ_LEN_ERR);
    }

    nbytes = myRead (sock, (void *) tmpBuf, myLen, SOCK_TYPE, NULL);

    if (nbytes != myLen) {
        rodsLog (LOG_ERROR,
         "readMsgHeader:header read- read %d bytes, expect %d, status = %d",
         nbytes, myLen, SYS_HEADER_READ_LEN_ERR - errno);
         return (SYS_HEADER_READ_LEN_ERR - errno);
    }

    if (getRodsLogLevel () <= LOG_DEBUG3) {
        printf ("received header: len = %d\n%s\n", myLen, tmpBuf);
    }

    /* always use XML_PROT for the startup pack */
    status = unpackStruct ((void *) tmpBuf, (void **) &outHeader,
      "MsgHeader_PI", RodsPackTable, XML_PROT);

    if (status < 0) {
        rodsLogError (LOG_ERROR,  status,
         "readMsgHeader:unpackStruct error. status = %d",
         status);
	return (status);
    }

    *myHeader = *outHeader;

    free (outHeader);

    return (0);
}

int
writeMsgHeader (int sock, msgHeader_t *myHeader)
{
    int nbytes;
    int status;
    int myLen;
    bytesBuf_t *headerBBuf = NULL;

    /* always use XML_PROT for the Header */
    status = packStruct ((void *) myHeader, &headerBBuf,
      "MsgHeader_PI", RodsPackTable, 0, XML_PROT);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
         "writeMsgHeader: packStruct error, status = %d", status);
        return status;
    }

    if (getRodsLogLevel () <= LOG_DEBUG3) {
        printf ("sending header: len = %d\n%s\n", headerBBuf->len, 
	  (char *) headerBBuf->buf);
    }

    myLen = htonl (headerBBuf->len);

    nbytes = myWrite (sock, (void *) &myLen, sizeof (myLen), SOCK_TYPE, NULL);

    if (nbytes != sizeof (myLen)) {
        rodsLog (LOG_ERROR,
         "writeMsgHeader: wrote %d bytes for myLen , expect %d, status = %d",
         nbytes, sizeof (myLen), SYS_HEADER_WRITE_LEN_ERR - errno);
         return (SYS_HEADER_WRITE_LEN_ERR - errno);
     }

    /* now send the header */

    nbytes = myWrite (sock, headerBBuf->buf, headerBBuf->len, SOCK_TYPE, NULL);

    if (headerBBuf->len != nbytes) {
        rodsLog (LOG_NOTICE,
         "writeMsgHeader: wrote %d bytes, expect %d, status = %d",
         nbytes, headerBBuf->len, SYS_HEADER_WRITE_LEN_ERR - errno);
         return (SYS_HEADER_WRITE_LEN_ERR - errno);
     }

     freeBBuf (headerBBuf);

     return (0);
}

int 
myRead (int sock, void *buf, int len, irodsDescType_t irodsDescType,
 int *bytesRead)
{
    int nbytes;
    int toRead;
    char *tmpPtr;

    toRead = len;
    tmpPtr = (char *) buf;

    if (bytesRead != NULL)
        *bytesRead = 0;

    while (toRead > 0) {
#ifdef _WIN32
	if (irodsDescType == SOCK_TYPE) {
            nbytes = recv(sock, tmpPtr, toRead, 0);
	} else {
            nbytes = read (sock, (void *) tmpPtr, toRead);
	}
#else
        nbytes = read (sock, (void *) tmpPtr, toRead);
#endif
        if (nbytes <= 0) {
            if (errno == EINTR) {
                /* interrupted */
                errno = 0;
                nbytes = 0;
            } else {
                break;
            }
        }

        toRead -= nbytes;
        tmpPtr += nbytes;
        if (bytesRead != NULL)
            *bytesRead += nbytes;
    }
    return (len - toRead);
}

int 
myWrite (int sock, void *buf, int len, irodsDescType_t irodsDescType,
int *bytesWritten)
{
    int nbytes;
    int toWrite;
    char *tmpPtr;

    toWrite = len;
    tmpPtr = (char *) buf;

    if (bytesWritten != NULL)
        *bytesWritten = 0;

    while (toWrite > 0) {
#ifdef _WIN32
        if (irodsDescType == SOCK_TYPE) {
            nbytes = send (sock, tmpPtr, toWrite, 0);
	} else {
            nbytes = write (sock, (void *) tmpPtr, toWrite);
	}
#else
        nbytes = write (sock, (void *) tmpPtr, toWrite);
#endif
        if (nbytes <= 0) {
	    if (errno == EINTR) {
		/* interrupted */
		errno = 0;
		nbytes = 0;
	    } else {
                break;
	    }
	}
        toWrite -= nbytes;
        tmpPtr += nbytes;
	if (bytesWritten != NULL)
	    *bytesWritten += nbytes;
    }
    return (len - toWrite);
}

int
readStartupPack (int sock, startupPack_t **startupPack)
{
    int status;
    msgHeader_t myHeader;
    bytesBuf_t inputStructBBuf, bsBBuf, errorBBuf;

    status = readMsgHeader (sock, &myHeader);

   if (status < 0) {
        rodsLogError (LOG_NOTICE, status,
	  "readStartupPack: readMsgHeader error. status = %d", status);
	return (status);
    }

    memset (&bsBBuf, 0, sizeof (bytesBuf_t));  
    status = readMsgBody (sock, &myHeader, &inputStructBBuf, &bsBBuf, 
      &errorBBuf, XML_PROT);
    if (status < 0) {
        rodsLogError (LOG_NOTICE, status,
          "readStartupPack: readMsgBody error. status = %d", status);
        return (status);
    }

    /* some sanity check */

    if (strcmp (myHeader.type, RODS_CONNECT_T) != 0) {
	if (inputStructBBuf.buf != NULL)
	    free (inputStructBBuf.buf);
        if (bsBBuf.buf != NULL)
            free (inputStructBBuf.buf);
        if (errorBBuf.buf != NULL)
            free (inputStructBBuf.buf);
        rodsLog (LOG_NOTICE,
	  "readStartupPack: wrong mag type - %s, expect %s",
          myHeader.type, RODS_CONNECT_T);
          return (SYS_HEADER_TPYE_LEN_ERR);
    }
 
    if (myHeader.bsLen != 0) {
        if (bsBBuf.buf != NULL)
            free (inputStructBBuf.buf);
	rodsLog (LOG_NOTICE, "readStartupPack: myHeader.bsLen = %d is not 0",
	  myHeader.bsLen);
    }

    if (myHeader.errorLen != 0) {
        if (errorBBuf.buf != NULL)
            free (inputStructBBuf.buf);
        rodsLog (LOG_NOTICE, 
	 "readStartupPack: myHeader.errorLen = %d is not 0",
          myHeader.errorLen);
    }

    if (myHeader.msgLen > sizeof (startupPack_t) * 2 || myHeader.msgLen <= 0) {
        if (inputStructBBuf.buf != NULL)
            free (inputStructBBuf.buf);
        rodsLog (LOG_NOTICE, 
	  "readStartupPack: problem with myHeader.msgLen = %d",
          myHeader.msgLen);
          return (SYS_HEADER_READ_LEN_ERR);
    }

    /* always use XML_PROT for the startup pack */
    status = unpackStruct (inputStructBBuf.buf, (void **) startupPack, 
      "StartupPack_PI", RodsPackTable, XML_PROT);

    clearBBuf (&inputStructBBuf);

    if (status < 0) {
        rodsLogError (LOG_NOTICE,  status,
         "readStartupPack:unpackStruct error. status = %d",
	 status);
    } 
    return (status);
}

int
readVersion (int sock, version_t **myVersion)
{
    int status;
    msgHeader_t myHeader;
    bytesBuf_t inputStructBBuf, bsBBuf, errorBBuf;

    status = readMsgHeader (sock, &myHeader);

   if (status < 0) {
        rodsLogError (LOG_NOTICE, status,
	  "readVersion: readMsgHeader error. status = %d", status);
	return (status);
    }

    memset (&bsBBuf, 0, sizeof (bytesBuf_t));
    status = readMsgBody (sock, &myHeader, &inputStructBBuf, &bsBBuf,
      &errorBBuf, XML_PROT);
    if (status < 0) {
        rodsLogError (LOG_NOTICE, status,
          "readVersion: readMsgBody error. status = %d", status);
        return (status);
    }

    /* some sanity check */

    if (strcmp (myHeader.type, RODS_VERSION_T) != 0) {
        if (inputStructBBuf.buf != NULL)
            free (inputStructBBuf.buf);
        if (bsBBuf.buf != NULL)
            free (inputStructBBuf.buf);
        if (errorBBuf.buf != NULL)
            free (inputStructBBuf.buf);
        rodsLog (LOG_NOTICE,
	  "readVersion: wrong mag type - %s, expect %s",
          myHeader.type, RODS_VERSION_T);
          return (SYS_HEADER_TPYE_LEN_ERR);
    }
 
    if (myHeader.bsLen != 0) {
        if (bsBBuf.buf != NULL)
            free (inputStructBBuf.buf);
	rodsLog (LOG_NOTICE, "readVersion: myHeader.bsLen = %d is not 0",
	  myHeader.bsLen);
    }

    if (myHeader.errorLen != 0) {
        if (errorBBuf.buf != NULL)
            free (inputStructBBuf.buf);
        rodsLog (LOG_NOTICE, "readVersion: myHeader.errorLen = %d is not 0",
          myHeader.errorLen);
    }

    if (myHeader.msgLen > sizeof (version_t) * 2 || myHeader.msgLen <= 0) {
        if (inputStructBBuf.buf != NULL)
            free (inputStructBBuf.buf);
        rodsLog (LOG_NOTICE, 
	  "readVersion: problem with myHeader.msgLen = %d",
          myHeader.msgLen);
          return (SYS_HEADER_READ_LEN_ERR);
    }

    /* alway use XML for version */
    status = unpackStruct (inputStructBBuf.buf, (void **) myVersion, 
      "Version_PI", RodsPackTable, XML_PROT);

    free (inputStructBBuf.buf);

    if (status < 0) {
        rodsLogError (LOG_NOTICE, status,
         "readStartupPack:unpackStruct error. status = %d",
	 status);
    } 
    return (status);
}

int
rodsSetSockOpt (int sock, int windowSize)
{
    int status;
    int savedStatus = 0;
    int temp;

    if (windowSize <= 0) {
	windowSize = SOCK_WINDOW_SIZE;
    } else if (windowSize < MIN_SOCK_WINDOW_SIZE) {
        rodsLog (LOG_NOTICE,
         "rodsSetSockOpt: the input windowSize %d is too small, default to %d",
         windowSize, MIN_SOCK_WINDOW_SIZE);
	windowSize = MIN_SOCK_WINDOW_SIZE;
    } else if (windowSize > MAX_SOCK_WINDOW_SIZE) {
        rodsLog (LOG_NOTICE, 
         "rodsSetSockOpt: the input windowSize %d is too large, default to %d",
         windowSize, MAX_SOCK_WINDOW_SIZE);
        windowSize = MAX_SOCK_WINDOW_SIZE;
    }

#ifdef _WIN32
    status = setsockopt (sock, SOL_SOCKET, SO_SNDBUF, 
      (char*)&windowSize, sizeof (windowSize));
    if (status < 0) 
	savedStatus = status;

    status = setsockopt (sock, SOL_SOCKET, SO_RCVBUF, 
      (char*)&windowSize, sizeof (windowSize));
    if (status < 0) 
        savedStatus = status;

    temp = 1;
    status = setsockopt (sock, IPPROTO_TCP, TCP_NODELAY,
      (char*)&temp, sizeof (temp));
    if (status < 0)
        savedStatus = status;
#else
    status = setsockopt (sock, SOL_SOCKET, SO_SNDBUF, 
      &windowSize, sizeof (windowSize));
    if (status < 0) 
	savedStatus = status;

    status = setsockopt (sock, SOL_SOCKET, SO_RCVBUF, 
      &windowSize, sizeof (windowSize));
    if (status < 0) 
        savedStatus = status;

    temp = 1;
    status = setsockopt (sock, IPPROTO_TCP, TCP_NODELAY,
      &temp, sizeof (temp));
    if (status < 0)
        savedStatus = status;
#endif

    return (savedStatus);
}

int 
connectToRhostPortal (char *rodsHost, int rodsPort, 
int cookie, int windowSize)
{
    int status, nbytes;
    struct sockaddr_in remoteAddr;
    int sock, myCookie;

    status = setSockAddr (&remoteAddr, rodsHost, rodsPort);
    if (status < 0) {
	rodsLog (LOG_NOTICE,
	  "connectToRhostByHostName: setSockAddr error for %s, errno = %d",
	  errno); 
	return (status);
    }
    sock = connectToRhostWithRaddr (&remoteAddr, windowSize, 0);

    if (sock < 0) {
	return (sock);
    }

    myCookie = htonl (cookie);
    nbytes = myWrite (sock, &myCookie, sizeof (myCookie), SOCK_TYPE, NULL);

    if (nbytes != sizeof (myCookie)) {
	CLOSE_SOCK (sock);
	return (SYS_PORT_COOKIE_ERR);
    }
    
    return (sock);
}
 
int
connectToRhost (rcComm_t *conn, int connectCnt, int reconnFlag)
{
    int status;

    conn->sock = connectToRhostWithRaddr (&conn->remoteAddr, 
      conn->windowSize, 1);

    if (conn->sock < 0) {
	rodsLogError (LOG_NOTICE, conn->sock,
          "connectToRhost: connect to host %s on port %d failed, status = %d",
          conn->host, conn->portNum, conn->sock);
        return conn->sock;
    }

    setConnAddr (conn);

    status = sendStartupPack (conn, connectCnt, reconnFlag);

    if (status < 0) {
        rodsLogError (LOG_NOTICE, status,
          "connectToRhost: sendStartupPack to %s failed, status = %d",
          conn->host, status);
        return status;
    }

    status = readVersion (conn->sock, &conn->svrVersion);

    if (status < 0) {
        rodsLogError (LOG_NOTICE, status,
          "connectToRhost: readVersion to %s failed, status = %d",
          conn->host, status);
        return status;
    }

    if (conn->svrVersion->status < 0) {
        rodsLogError (LOG_NOTICE, conn->svrVersion->status,
          "connectToRhost: error returned from host %s status = %d",
          conn->host, conn->svrVersion->status);
        return conn->svrVersion->status;
    }

    return 0;
}

int
connectToRhostWithRaddr (struct sockaddr_in *remoteAddr, int windowSize, 
int timeoutFlag)
{
    int sock;
    int status;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock <= 0) {	/* try again */
        sock = socket(AF_INET, SOCK_STREAM, 0);
    }

    if (sock <= 0) {
        rodsLog (LOG_NOTICE,
         "connectToRhostWithRaddr() - socket() failed: errno=%d",
           errno);
        return (USER_SOCK_OPEN_ERR - errno);
    }

    if (timeoutFlag > 0) {
        status = connectToRhostWithTout (sock, (struct sockaddr *) remoteAddr);
    } else {
	status = connect (sock, (struct sockaddr *) remoteAddr, 
	  sizeof (struct sockaddr));
    }

    if (status < 0) {
#ifdef _WIN32
        closesocket (sock);
#else
        close (sock);
#endif /* WIN32 */
	return (status);
    }

    rodsSetSockOpt (sock, windowSize);

#ifdef PORTNAME_solaris
    flag = fcntl (sock, F_GETFL);
    if (flag & O_NONBLOCK)
        fcntl (sock, F_SETFL, (flag & (~O_NONBLOCK)));
#endif

    return (sock);

}

#ifdef _WIN32
void CALLBACK my_timeout_handler(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
   win_connect_timeout = 1;
}
#endif

int
connectToRhostWithTout (int sock, struct sockaddr *sin)
{
    int timeoutCnt = 0;
    int status;
    long arg;
    fd_set myset;
    struct timeval tv;

#ifdef _WIN32
    /* A Windows console app has very limited timeout functionality.
     * An pseudo timeout is implemented.
     */
	/*
	int connectCnt;
	int win_connect_timeout_cb;
    int win_connect_timeout = 0;
    int win_connect_timer_id;
     win_connect_timeout_cb = 0;
	 win_connect_timer_id = 
     timeSetEvent(CONNECT_TIMEOUT*1000, 0, my_timeout_handler, 0, 
     TIME_ONESHOT);
	connectCnt = 0;
	*/

	 status = 0;

    while ((timeoutCnt < MAX_CONN_RETRY_CNT) && (!win_connect_timeout)) {
        if ((status = connect (sock, sin, sizeof (struct sockaddr))) < 0) {
            timeoutCnt ++;
            rodsSleep (0, 200000);
        } else {
            break;
        }
    }
	if(status != 0)
	{
		return USER_SOCK_CONNECT_TIMEDOUT;
	}

	return 0;

	/*
    if(win_connect_timeout) {
        fprintf(stderr,
	 "portalConnect: connect msg timed out for pid %d\n", getpid ());
        status = USER_SOCK_CONNECT_ERR;
    } else {
        timeKillEvent(win_connect_timer_id);
    }
	*/

#else
    /* redo the timeout using select */
    /* Set non-blocking */
    if((arg = fcntl (sock, F_GETFL, NULL)) < 0) {
	rodsLog (LOG_ERROR,
	 "connectToRhostWithTout: fcntl F_GETFL error, errno = %d", errno);
	return (USER_SOCK_CONNECT_ERR);
    }
    arg |= O_NONBLOCK;
    if (fcntl (sock, F_SETFL, arg) < 0) {
        rodsLog (LOG_ERROR,
         "connectToRhostWithTout: fcntl F_SETFL error, errno = %d", errno);
        return (USER_SOCK_CONNECT_ERR);
    }

    while (timeoutCnt < MAX_CONN_RETRY_CNT) {
        status = connect (sock, sin, sizeof (struct sockaddr));
	if (status >= 0) break;
	if (errno == EINPROGRESS || errno == EINTR) {
            tv.tv_sec = CONNECT_TIMEOUT_TIME;
            tv.tv_usec = 0;
            FD_ZERO (&myset);
            FD_SET (sock, &myset);
            status = select (sock + 1, NULL, &myset, NULL, &tv);
            if (status < 0) {
		if (errno != EINTR) {
                    rodsLog (LOG_NOTICE,
                     "connectToRhostWithTout: connect error, errno = %d", 
		      errno);
		    timeoutCnt++;
		}
		continue;
	    } else if (status > 0) {
		int myval;
#if defined(aix_platform)
        	size_t mylen = sizeof (int);
#else
		uint mylen = sizeof (int);
#endif
                if (getsockopt (sock, SOL_SOCKET, SO_ERROR, (void*) (&myval), 
		  &mylen) < 0) {
        	    rodsLog (LOG_ERROR,
         	      "connectToRhostWithTout: getsockopt error, errno = %d", 
		      errno);
        	    return (USER_SOCK_CONNECT_ERR - errno);
		}
                /* Check the returned value */
                if (myval) {
                    rodsLog (LOG_NOTICE,
                      "connectToRhostWithTout: connect error, errno = %d", 
                      myval);
                    timeoutCnt++;
		    status = USER_SOCK_CONNECT_ERR - myval;
		    continue;
                } else {
                    break;
		}
	    } else {
		/* timed out */
		status = USER_SOCK_CONNECT_TIMEDOUT;
		break;
	    }
	} else {
            rodsLog (LOG_NOTICE,
             "connectToRhostWithTout: connect error, errno = %d", errno);
            timeoutCnt++;
	    status = USER_SOCK_CONNECT_ERR - errno;
	    continue;
	}
    }

    if (status < 0) {
	if (status == -1) 
	    return USER_SOCK_CONNECT_ERR;
	else 
	    return status;
    }
		
    /* Set to blocking again */
    if((arg = fcntl (sock, F_GETFL, NULL)) < 0) {
        rodsLog (LOG_ERROR,
         "connectToRhostWithTout: fcntl F_GETFL error, errno = %d", errno);
        return (USER_SOCK_CONNECT_ERR);
    }

    arg &= (~O_NONBLOCK);
    if( fcntl(sock, F_SETFL, arg) < 0) {
        rodsLog (LOG_ERROR,
         "connectToRhostWithTout: fcntl F_SETFL error, errno = %d", errno);
        return (USER_SOCK_CONNECT_ERR);
    }

#endif
    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "connectToRhostWithTout: connect failed. errno = %d \n",
          errno);
        status = USER_SOCK_CONNECT_ERR - errno;
    }
    return (status);
}

int
setConnAddr (rcComm_t *conn)
{
    int status1, status2;

    status1 = setLocalAddr (conn->sock, &conn->localAddr);

    status2 = setRemoteAddr (conn->sock, &conn->remoteAddr);

    if (status1 < 0) {
	return status1;
    } else if (status2 < 0) {
	return status2;
    } else {
	return 0;
    }
}

int
setRemoteAddr (int sock, struct sockaddr_in *remoteAddr)
{
#if defined(aix_platform)
    size_t      laddrlen = sizeof(struct sockaddr);
#elif defined(windows_platform)
	int laddrlen = sizeof(struct sockaddr);
#else
    uint         laddrlen = sizeof(struct sockaddr);
#endif

    /* fill in the server address. This is for case where the conn->host
     * is not a real host but an address that select a host from a pool
     * of hosts */
    if (getpeername(sock, (struct sockaddr *) remoteAddr,
        &laddrlen) < 0) {
	rodsLog (LOG_NOTICE,
          "setLocalAddr() -- getpeername() failed: errno=%d", errno);

	return (USER_RODS_HOSTNAME_ERR);
    }

    return (0);
}

int
setLocalAddr (int sock, struct sockaddr_in *localAddr)
{
#if defined(aix_platform)
    size_t      laddrlen = sizeof(struct sockaddr);
#elif defined(windows_platform)
    int         laddrlen = sizeof(struct sockaddr);
#else
    uint         laddrlen = sizeof(struct sockaddr);
#endif


    /* fill in the client address */
    if (getsockname (sock, (struct sockaddr *) localAddr,
        &laddrlen) < 0) {
	rodsLog (LOG_NOTICE,
          "setLocalAddr() -- getsockname() failed: errno=%d",
            errno);
	return USER_RODS_HOSTNAME_ERR;
    }
    return ntohs (localAddr->sin_port);
}

int
sendStartupPack (rcComm_t *conn, int connectCnt, int reconnFlag)
{
    startupPack_t startupPack;
    int status;
    bytesBuf_t *startupPackBBuf = NULL;
    

    /* setup the startup pack */

    startupPack.irodsProt = conn->irodsProt;
    startupPack.connectCnt = connectCnt;
    startupPack.reconnFlag = reconnFlag;

    rstrcpy (startupPack.proxyUser, conn->proxyUser.userName, NAME_LEN);
    rstrcpy (startupPack.proxyRodsZone, conn->proxyUser.rodsZone, NAME_LEN);
    rstrcpy (startupPack.clientUser, conn->clientUser.userName, NAME_LEN);
    rstrcpy (startupPack.clientRodsZone, conn->clientUser.rodsZone, 
     NAME_LEN);
    rstrcpy (startupPack.relVersion, RODS_REL_VERSION,  NAME_LEN);
    rstrcpy (startupPack.apiVersion, RODS_API_VERSION,  NAME_LEN);
    startupPack.option[0] = '\0';

    /* always use XML_PROT for the startupPack */
    status = packStruct ((void *) &startupPack, &startupPackBBuf,
      "StartupPack_PI", RodsPackTable, 0, XML_PROT);

    if (status < 0) {
	rodsLogError (LOG_NOTICE, status,
         "sendStartupPack: packStruct error, status = %d", status);
        return status;
    }

    status = sendRodsMsg (conn->sock, RODS_CONNECT_T, startupPackBBuf, 
      NULL, NULL, 0, XML_PROT);

    freeBBuf (startupPackBBuf);

   if (status < 0) {
        rodsLogError (LOG_NOTICE, status,
         "sendStartupPack: sendRodsMsg error, status = %d", status);
        return status;
    }

    return (status);
}

int
sendVersion (int sock, int versionStatus, int reconnPort, 
char *reconnAddr, int cookie)
{
    version_t myVersion;
    int status;
    bytesBuf_t *versionBBuf = NULL;


    /* setup the version struct */

    memset (&myVersion, 0, sizeof (myVersion));
    myVersion.status = versionStatus;
    rstrcpy (myVersion.relVersion, RODS_REL_VERSION,  NAME_LEN);
    rstrcpy (myVersion.apiVersion, RODS_API_VERSION,  NAME_LEN);
    if (reconnAddr != NULL) {
	myVersion.reconnPort = reconnPort;
	rstrcpy (myVersion.reconnAddr, reconnAddr, LONG_NAME_LEN);
	myVersion.cookie = cookie;
    }

    /* alway use XML for version */
    status = packStruct ((char *) &myVersion, &versionBBuf,
      "Version_PI", RodsPackTable, 0, XML_PROT);

   if (status < 0) {
        rodsLogError (LOG_NOTICE, status,
         "sendVersion: packStruct error, status = %d", status);
        return status;
    }

    status = sendRodsMsg (sock, RODS_VERSION_T, versionBBuf, NULL, NULL, 0,
      XML_PROT);

    freeBBuf (versionBBuf);

    if (status < 0) {
        rodsLogError (LOG_NOTICE, status,
         "sendVersion: sendRodsMsg error, status = %d", status);
        return status;
    }

    return (status);
}


int
sendRodsMsg (int sock, char *msgType, bytesBuf_t *msgBBuf, 
bytesBuf_t *byteStreamBBuf, bytesBuf_t *errorBBuf, int intInfo, 
irodsProt_t irodsProt)
{
    int status;
    msgHeader_t msgHeader;
    int bytesWritten;

    memset (&msgHeader, 0, sizeof (msgHeader));

    rstrcpy (msgHeader.type, msgType, HEADER_TYPE_LEN);

    if (msgBBuf == NULL) {
	msgHeader.msgLen = 0;
    } else {
	msgHeader.msgLen = msgBBuf->len;
    }

    if (byteStreamBBuf == NULL) {
        msgHeader.bsLen = 0;
    } else {
        msgHeader.bsLen = byteStreamBBuf->len;
    }
 
    if (errorBBuf == NULL) {
        msgHeader.errorLen = 0;
    } else { 
        msgHeader.errorLen = errorBBuf->len;
    }

    msgHeader.intInfo = intInfo;

    status = writeMsgHeader (sock, &msgHeader);

    if (status < 0)
	return (status);

    /* send the rest */

    if (msgHeader.msgLen > 0) {
        if (irodsProt == XML_PROT && getRodsLogLevel () <= LOG_DEBUG3) {
            printf ("sending msg: \n%s\n", (char *) msgBBuf->buf);
        }
        status = myWrite (sock, msgBBuf->buf, msgBBuf->len, SOCK_TYPE, NULL);
        if (status < 0) 
            return (status);
    }

    if (msgHeader.errorLen > 0) {
        if (irodsProt == XML_PROT && getRodsLogLevel () <= LOG_DEBUG3) {
            printf ("sending error msg: \n%s\n", (char *) errorBBuf->buf);
        }
        status = myWrite (sock, errorBBuf->buf, errorBBuf->len, SOCK_TYPE, 
	  NULL);
        if (status < 0)
            return (status);
    }
    if (msgHeader.bsLen > 0) {
        status = myWrite (sock, byteStreamBBuf->buf,
          byteStreamBBuf->len, SOCK_TYPE, &bytesWritten);
        if (status < 0)
            return (status);
    }

    return (0);
}

int
rodsSleep (int sec, int microSec)
{
    struct timeval sleepTime;

    sleepTime.tv_sec = sec;
    sleepTime.tv_usec = microSec;

    select (0, NULL, NULL, NULL, &sleepTime);

    return 0;
}

int
readMsgBody (int sock, msgHeader_t *myHeader, bytesBuf_t *inputStructBBuf, 
bytesBuf_t *bsBBuf, bytesBuf_t *errorBBuf, irodsProt_t irodsProt)
{
    int nbytes;
    int bytesRead;

    if (myHeader == NULL) {
	return (SYS_READ_MSG_BODY_INPUT_ERR);
    }
    if (inputStructBBuf != NULL)
	memset (inputStructBBuf, 0, sizeof (bytesBuf_t));

    /* Don't memset bsBBuf because bsBBuf can be reused on the client side */

    if (errorBBuf != NULL)
        memset (errorBBuf, 0, sizeof (bytesBuf_t));

    if (myHeader->msgLen > 0) {
        if (inputStructBBuf == NULL) {
            return (SYS_READ_MSG_BODY_INPUT_ERR);
        }

        inputStructBBuf->buf = malloc (myHeader->msgLen);

        nbytes = myRead (sock, inputStructBBuf->buf, myHeader->msgLen, 
	  SOCK_TYPE, NULL);

        if (irodsProt == XML_PROT && getRodsLogLevel () <= LOG_DEBUG3) {
            printf ("received msg: \n%s\n", (char *) inputStructBBuf->buf);
        }

        if (nbytes != myHeader->msgLen) {
            rodsLog (LOG_NOTICE, 
	      "readMsgBody: inputStruct read error, read %d bytes, expect %d",
               nbytes, myHeader->msgLen);
            free (inputStructBBuf->buf);
            return (SYS_HEADER_READ_LEN_ERR);
	}
	inputStructBBuf->len = myHeader->msgLen;
    }
 
    if (myHeader->errorLen > 0) {
        if (errorBBuf == NULL) {
            return (SYS_READ_MSG_BODY_INPUT_ERR);
        }

        errorBBuf->buf = malloc (myHeader->errorLen);

        nbytes = myRead (sock, errorBBuf->buf, myHeader->errorLen,
	  SOCK_TYPE, NULL);

        if (irodsProt == XML_PROT && getRodsLogLevel () <= LOG_DEBUG3) {
            printf ("received error msg: \n%s\n", (char *) errorBBuf->buf);
        }

        if (nbytes != myHeader->errorLen) {
            rodsLog (LOG_NOTICE,
              "readMsgBody: errorBbuf read error, read %d bytes, expect %d, errno = %d",
             nbytes, myHeader->msgLen, errno);
            free (errorBBuf->buf);
            return (SYS_READ_MSG_BODY_LEN_ERR - errno);
        }
        errorBBuf->len = myHeader->errorLen;
    }

    if (myHeader->bsLen > 0) {
        if (bsBBuf == NULL) {
            return (SYS_READ_MSG_BODY_INPUT_ERR);
        }

	if (bsBBuf->buf == NULL) {
            bsBBuf->buf = malloc (myHeader->bsLen);
	} else if (myHeader->bsLen > bsBBuf->len) {
	    free (bsBBuf->buf);
            bsBBuf->buf = malloc (myHeader->bsLen);
        }

        nbytes = myRead (sock, bsBBuf->buf, myHeader->bsLen, SOCK_TYPE,
	  &bytesRead);

        if (nbytes != myHeader->bsLen) {
            rodsLog (LOG_NOTICE, 
	      "readMsgBody: bsBBuf read error, read %d bytes, expect %d, errno = %d",
             nbytes, myHeader->bsLen, errno);
            free (bsBBuf->buf);
            return (SYS_READ_MSG_BODY_INPUT_ERR - errno);
        }
	bsBBuf->len = myHeader->bsLen;
    }

    return (0);
}

char *
rods_inet_ntoa (struct in_addr in)
{
    char *clHostAddr;

    clHostAddr = inet_ntoa (in);

    if (strcmp (clHostAddr, "127.0.0.1") == 0 ||
     strcmp (clHostAddr, "0.0.0.0") == 0) { /* localhost */
        char sb[LONG_NAME_LEN];
        struct hostent *phe;

        if (gethostname (sb, sizeof (sb)) != 0)
          return(clHostAddr);
        if ((phe = gethostbyname (sb)) == NULL)
            return(clHostAddr);
        clHostAddr = inet_ntoa (*(struct in_addr*) phe->h_addr);
    }

    return (clHostAddr);
}

int
irodsCloseSock (int sock)
{
#ifdef _WIN32
        return (closesocket (sock));
#else
        return (close (sock));
#endif /* WIN32 */
}

int
readReconMsg (int sock, reconnMsg_t **reconnMsg)
{
    int status;
    msgHeader_t myHeader;
    bytesBuf_t inputStructBBuf, bsBBuf, errorBBuf;

    status = readMsgHeader (sock, &myHeader);

   if (status < 0) {
        rodsLogError (LOG_NOTICE, status,
	  "readReconMsg: readMsgHeader error. status = %d", status);
	return (status);
    }

    memset (&bsBBuf, 0, sizeof (bytesBuf_t));  
    status = readMsgBody (sock, &myHeader, &inputStructBBuf, &bsBBuf, 
      &errorBBuf, XML_PROT);
    if (status < 0) {
        rodsLogError (LOG_NOTICE, status,
          "readReconMsg: readMsgBody error. status = %d", status);
        return (status);
    }

    /* some sanity check */

    if (strcmp (myHeader.type, RODS_RECONNECT_T) != 0) {
	if (inputStructBBuf.buf != NULL)
	    free (inputStructBBuf.buf);
        if (bsBBuf.buf != NULL)
            free (inputStructBBuf.buf);
        if (errorBBuf.buf != NULL)
            free (inputStructBBuf.buf);
        rodsLog (LOG_NOTICE,
	  "readReconMsg: wrong mag type - %s, expect %s",
          myHeader.type, RODS_CONNECT_T);
          return (SYS_HEADER_TPYE_LEN_ERR);
    }
 
    if (myHeader.bsLen != 0) {
        if (bsBBuf.buf != NULL)
            free (inputStructBBuf.buf);
	rodsLog (LOG_NOTICE, "readReconMsg: myHeader.bsLen = %d is not 0",
	  myHeader.bsLen);
    }

    if (myHeader.errorLen != 0) {
        if (errorBBuf.buf != NULL)
            free (inputStructBBuf.buf);
        rodsLog (LOG_NOTICE, 
	 "readReconMsg: myHeader.errorLen = %d is not 0",
          myHeader.errorLen);
    }

    if (myHeader.msgLen <= 0) {
        if (inputStructBBuf.buf != NULL)
            free (inputStructBBuf.buf);
        rodsLog (LOG_NOTICE, 
	  "readReconMsg: problem with myHeader.msgLen = %d",
          myHeader.msgLen);
          return (SYS_HEADER_READ_LEN_ERR);
    }

    /* always use XML_PROT for the startup pack */
    status = unpackStruct (inputStructBBuf.buf, (void **) reconnMsg, 
      "ReconnMsg_PI", RodsPackTable, XML_PROT);

    clearBBuf (&inputStructBBuf);

    if (status < 0) {
        rodsLogError (LOG_NOTICE,  status,
         "readReconMsg:unpackStruct error. status = %d",
	 status);
    } 
    return (status);
}

int
sendReconnMsg (int sock, reconnMsg_t *reconnMsg)
{
    int status;
    bytesBuf_t *reconnMsgBBuf = NULL;

    if (reconnMsg == NULL) return (USER__NULL_INPUT_ERR);

   /* alway use XML for version */
    status = packStruct ((char *) &reconnMsg, &reconnMsgBBuf,
      "ReconnMsg_PI", RodsPackTable, 0, XML_PROT);

    status = sendRodsMsg (sock, RODS_RECONNECT_T, reconnMsgBBuf,
      NULL, NULL, 0, XML_PROT);

    freeBBuf (reconnMsgBBuf);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "sendReconnMsg: sendRodsMsg of reconnect msg failed, status = %d",
          status);
    }
    return (status);
}

int svrSwitchConnect (rsComm_t *rsComm)
{
    reconnMsg_t reconnMsg;

    if (rsComm->reconnectedSock > 0) {
	if (rsComm->clientState == RECEIVING_STATE) {
            reconnMsg_t reconnMsg;
	    bzero (&reconnMsg, sizeof (reconnMsg));
	    sendReconnMsg (rsComm->sock, &reconnMsg);
	    rsComm->clientState = PROCESSING_STATE;
	}
	close (rsComm->sock); 
	rsComm->sock = rsComm->reconnectedSock;
	rsComm->reconnectedSock = 0;
	rodsLog (LOG_NOTICE,
          "reconnManager: svrSwitchConnect. Switch connection");
        return 1;
    } else {
	return 0;
    }
}

int
addUdpPortToPortList (portList_t *thisPortList, int udpport)
{
    /* put udpport in the upper 16 bits of portNum */
    thisPortList->portNum |= udpport << 16;
    return 0;
}

int
getUdpPortFromPortList (portList_t *thisPortList)
{
    int udpport = 0;
    udpport = (thisPortList->portNum & 0xffff0000) >> 16;
    return (udpport);
}

int
getTcpPortFromPortList (portList_t *thisPortList)
{
    return (thisPortList->portNum & 0xffff);
}

int
addUdpSockToPortList (portList_t *thisPortList, int udpsock)
{
    /* put udpport in the upper 16 bits of portNum */
    thisPortList->sock |= udpsock << 16;
    return 0;
}

int
getUdpSockFromPortList (portList_t *thisPortList)
{
    int udpsock = 0;
    udpsock = (thisPortList->sock & 0xffff0000) >> 16;
    return (udpsock);
}

int
getTcpSockFromPortList (portList_t *thisPortList)
{
    return (thisPortList->sock & 0xffff);
}

int
isReadMsgError (int status)
{
    if (status + (status % 1000) == SYS_READ_MSG_BODY_LEN_ERR ||
      status + (status % 1000) == SYS_HEADER_READ_LEN_ERR) {
	return 1;
    } else {
	return 0;
    }
}
