/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* rcConnect.c - client connect call to server 
 * 
 */

#include "rcConnect.h"
#include "rcGlobal.h"

#ifdef windows_platform
#include "startsock.h"
#endif

#ifndef windows_platform
#include <pthread.h>
#endif

rcComm_t *
rcConnect (char *rodsHost, int rodsPort, char *userName, char *rodsZone,
int reconnFlag, rErrMsg_t *errMsg)
{

    rcComm_t *conn;

#ifdef windows_platform
    if(0 != startWinsock())
	{
		conn = NULL;
		/*error -34*/
		return conn;
	}
#endif

#ifndef windows_platform
    if (reconnFlag != RECONN_TIMEOUT && getenv (RECONNECT_ENV) != NULL) {
        reconnFlag = RECONN_TIMEOUT;
    }
#endif

    conn = _rcConnect (rodsHost, rodsPort, userName, rodsZone, NULL, NULL,
      errMsg, 0, reconnFlag);

    return (conn);
}

rcComm_t *
_rcConnect (char *rodsHost, int rodsPort, 
char *proxyUserName, char *proxyRodsZone,
char *clientUserName, char *clientRodsZone, rErrMsg_t *errMsg, int connectCnt,
int reconnFlag)
{
    rcComm_t *conn;
    int status;
    char *tmpStr;

#ifndef windows_platform
    if (ProcessType == CLIENT_PT)
        signal (SIGPIPE, (void (*)(int)) rcPipSigHandler);
#endif

    conn = (rcComm_t*)malloc (sizeof (rcComm_t));

    memset (conn, 0, sizeof (rcComm_t));
 
    if (errMsg != NULL) {
	memset (errMsg, 0, sizeof (rErrMsg_t));
    }

    if ((tmpStr = getenv (IRODS_PROT)) != NULL) {
	conn->irodsProt = (irodsProt_t)atoi(tmpStr);
    } else {
        conn->irodsProt = NATIVE_PROT;
    }

    status = setUserInfo (proxyUserName, proxyRodsZone, 
      clientUserName, clientRodsZone, &conn->clientUser, &conn->proxyUser);

    if (status < 0) {
	if (errMsg != NULL) {
	    errMsg->status = status;
	    snprintf (errMsg->msg, ERR_MSG_LEN - 1,
	      "_rcConnect: setUserInfo failed\n");
	}
	return NULL;
    }
  
    status = setRhostInfo (conn, rodsHost, rodsPort);

    if (status < 0) {
        if (errMsg != NULL) {
        rodsLogError (LOG_ERROR, status,
         "_rcConnect: setRhostInfo error, irodHost is probably not set correctly");
            errMsg->status = status;
            snprintf (errMsg->msg, ERR_MSG_LEN - 1,
              "_rcConnect: setRhostInfo failed\n");
        }
        return NULL;
    }

    status = connectToRhost (conn, connectCnt, reconnFlag);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
	 "_rcConnect: connectToRhost error, server on %s is probably down",
	 conn->host);
        if (errMsg != NULL) {
            errMsg->status = status;
            snprintf (errMsg->msg, ERR_MSG_LEN - 1,
              "_rcConnect: connectToRhost failed\n");
        }
        return NULL;
    }

#ifndef windows_platform
    if (reconnFlag == RECONN_TIMEOUT && conn->svrVersion != NULL &&
       conn->svrVersion->reconnPort > 0) {
        pthread_mutex_init (&conn->lock, NULL);
        pthread_cond_init (&conn->cond, NULL);
        status = pthread_create  (&conn->reconnThr, pthread_attr_default,
              (void *(*)(void *)) cliReconnManager,
              (void *) conn);

        if (status < 0) {
            rodsLog (LOG_ERROR, "_rcConnect: pthread_create failed, stat=%d",
              status);
        }
    }
#endif

    return (conn);
}
 

int
setUserInfo (
char *proxyUserName, char *proxyRodsZone,
char *clientUserName, char *clientRodsZone,
userInfo_t *clientUser, userInfo_t *proxyUser)
{
    char *myUserName;
    char *myRodsZone;

    rstrcpy (proxyUser->userName, proxyUserName, NAME_LEN);
    if (clientUserName != NULL) {
	rstrcpy (clientUser->userName, clientUserName, NAME_LEN);
    } else if ((myUserName = getenv (CLIENT_USER_NAME_KEYWD)) != NULL) {
	rstrcpy (clientUser->userName, myUserName, NAME_LEN);
    } else {
        rstrcpy (clientUser->userName, proxyUserName, NAME_LEN);
    }

    rstrcpy (proxyUser->rodsZone, proxyRodsZone, NAME_LEN);
    if (clientRodsZone != NULL) {
        rstrcpy (clientUser->rodsZone, clientRodsZone, NAME_LEN);
    } else if ((myRodsZone = getenv (CLIENT_RODS_ZONE_KEYWD)) != NULL) {
        rstrcpy (clientUser->rodsZone, myRodsZone, NAME_LEN);
    } else {
        rstrcpy (clientUser->rodsZone, proxyRodsZone, NAME_LEN);
    }

    return (0);
}

int setAuthInfo (char *rodsAuthScheme,
char *authStr, char *rodsServerDn,
userInfo_t *clientUser, userInfo_t *proxyUser, int flag)
{
    int authScheme;

    /* do some sanity check */

    if (strcmp (rodsAuthScheme, PASSWORD_AUTH_KEYWD) == 0) {
        if (authStr == NULL || strlen (authStr) == 0)
            return (USER_AUTH_STRING_EMPTY);

        authScheme = PASSWORD;
    } else {
        return (USER_AUTH_SCHEME_ERR);
    }

    rstrcpy (proxyUser->authInfo.authScheme, rodsAuthScheme, NAME_LEN);
    rstrcpy (clientUser->authInfo.authScheme, rodsAuthScheme, NAME_LEN);
    proxyUser->authInfo.flag = flag;
    clientUser->authInfo.flag = flag;

    if (authScheme == PASSWORD) {
        rstrcpy (proxyUser->authInfo.authStr, authStr, NAME_LEN);
        rstrcpy (clientUser->authInfo.authStr, authStr, NAME_LEN);
    }

    return (0);
}

int
setRhostInfo (rcComm_t *conn, char *rodsHost, int rodsPort)
{
    int status;

    if (rodsHost == NULL || strlen (rodsHost) == 0) {
	return (USER_RODS_HOST_EMPTY);
    }

    rstrcpy (conn->host, rodsHost, NAME_LEN);
    conn->portNum = rodsPort;

    status = setSockAddr (&conn->remoteAddr, rodsHost, rodsPort);

    return (status);
}

int 
setSockAddr (struct sockaddr_in *remoteAddr, char *rodsHost, int rodsPort)
{
    struct hostent *myHostent;

    myHostent = gethostbyname (rodsHost);

    if (myHostent == NULL || myHostent->h_addrtype != AF_INET) {
	rodsLog (LOG_ERROR, "unknown hostname: %s", rodsHost);
	return (USER_RODS_HOSTNAME_ERR - errno);
    }

    memcpy (&remoteAddr->sin_addr, myHostent->h_addr, 
      myHostent->h_length);
    remoteAddr->sin_family = AF_INET;
    remoteAddr->sin_port = htons((unsigned short) (rodsPort));

    return (0);
}

int
rcDisconnect (rcComm_t *conn)
{
    int status;

    if (conn == NULL) {
	return (0);
    }

    /* send disconnect msg to agent */
    status = sendRodsMsg (conn->sock, RODS_DISCONNECT_T, NULL, NULL, NULL, 0,
      conn->irodsProt);

#ifdef windows_platform
	closesocket(conn->sock);
#else
    close (conn->sock);
#endif

    status = freeRcComm (conn);

    return (status);
}

int
freeRcComm (rcComm_t *conn)
{
    int status;

    if (conn == NULL) {
        return (0);
    }

    status = cleanRcComm (conn);
    free (conn);

    return status;
}

int 
cleanRcComm (rcComm_t *conn)
{

    if (conn == NULL) {
        return (0);
    }

    freeRError (conn->rError);
    conn->rError = NULL;

    if (conn->svrVersion != NULL) { 
	free (conn->svrVersion);
	conn->svrVersion = NULL;
    }

    return (0);
}
void rcPipSigHandler ()
{
    fprintf (stderr,
     "Client Caught broken pipe signal. Connection to server may be down\n");
}

#if 0	/* XXXXX redo */
int
rcReconnect (rcComm_t *conn, reconnOpr_t reconnOpr)
{
    struct sockaddr_in remoteAddr;
    struct hostent *myHostent;
    int status;
    int reconCnt = 0;
    reconnMsg_t reconnMsg;
    bytesBuf_t *reconnMsgBBuf = NULL;

    if (conn->svrVersion == NULL || conn->svrVersion->reconnPort <= 0) {
	return (USER__NULL_INPUT_ERR);
    }

    rodsLog (LOG_ERROR, "rcReconnect: Reconnecting to %s port %d",
      conn->svrVersion->reconnAddr, conn->svrVersion->reconnPort);

    irodsCloseSock (conn->sock);
    conn->sock = 0;
    myHostent = gethostbyname (conn->svrVersion->reconnAddr);

    if (myHostent == NULL || myHostent->h_addrtype != AF_INET) {
        rodsLog (LOG_ERROR, "rcReconnect: unknown hostname: %s", 
	  conn->svrVersion->reconnAddr);
        return (USER_RODS_HOSTNAME_ERR - errno);
    }

    memcpy (&remoteAddr.sin_addr, myHostent->h_addr,
      myHostent->h_length);
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons((unsigned short) conn->svrVersion->reconnPort);

    while (reconCnt < MAX_RECONN_RETRY_CNT) {
        conn->sock = connectToRhostWithRaddr (&remoteAddr, conn->windowSize, 0);

        if (conn->sock < 0) {
            reconCnt ++;
            rodsSleep (RECONNECT_SLEEP_TIME, 0);
	} else {
	    break;
	}
    }

    if (conn->sock < 0) {
        rodsLogError (LOG_ERROR, conn->sock,
          "rcReconnect: connect to host %s on port %d failed, status = %d",
          conn->host, conn->portNum, conn->sock);
        return conn->sock;
    }

    setConnAddr (conn);

    conn->reconnTime = time (0);

    /* send the reconnMsg */

    reconnMsg.reconnOpr = reconnOpr;
    reconnMsg.cookie = conn->svrVersion->cookie;

    /* alway use XML for version */
    status = packStruct ((char *) &reconnMsg, &reconnMsgBBuf,
      "ReconnMsg_PI", RodsPackTable, 0, XML_PROT);
 
    status = sendRodsMsg (conn->sock, RODS_RECONNECT_T, reconnMsgBBuf,
      NULL, NULL, 0, XML_PROT);

    freeBBuf (reconnMsgBBuf);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcReconnect: sendRodsMsg of reconnect msg failed, status = %d",
          status);
    }
    return (status);
}
#endif

rcComm_t *
rcConnectXmsg (rodsEnv *myRodsEnv, rErrMsg_t *errMsg)
{
    rcComm_t *conn;

    if (myRodsEnv == NULL) {
	fprintf (stderr, "rcConnectXmsg: NULL myRodsEnv input\n");
	return (NULL);
    }

    conn = rcConnect (myRodsEnv->xmsgHost, myRodsEnv->xmsgPort,
      myRodsEnv->rodsUserName, myRodsEnv->rodsZone, 0, errMsg);

    return (conn);
}

#ifndef windows_platform
void
cliReconnManager (rcComm_t *conn)
{
    int status;
    struct sockaddr_in remoteAddr;
    struct hostent *myHostent;
    reconnMsg_t reconnMsg;
    reconnMsg_t *reconnMsgOut = NULL;

    if (conn == NULL || conn->svrVersion == NULL ||
       conn->svrVersion->reconnPort <= 0) {
        return;
    }

    conn->reconnTime = time (0) + RECONN_TIMEOUT_TIME;

    while (1) {
	time_t curTime = time (0);

	if (curTime < conn->reconnTime) 
	    rodsSleep (conn->reconnTime - curTime, 0);

        pthread_mutex_lock (&conn->lock);
        /* need to check clientState */
        while (conn->clientState != PROCESSING_STATE) {
            /* have to wait until the client stop sending */
            conn->reconnThrState = CONN_WAIT_STATE;
            rodsLog (LOG_DEBUG,
              "cliReconnManager: clientState = %d", conn->clientState);
            pthread_cond_wait (&conn->cond, &conn->lock);
        }
        rodsLog (LOG_DEBUG,
          "cliReconnManager: Reconnecting clientState = %d", 
	  conn->clientState);

        conn->reconnThrState = PROCESSING_STATE;
	/* connect to server's reconn thread */

        myHostent = gethostbyname (conn->svrVersion->reconnAddr);

        if (myHostent == NULL || myHostent->h_addrtype != AF_INET) {
            rodsLog (LOG_ERROR, "cliReconnManager: unknown hostname: %s",
              conn->svrVersion->reconnAddr);
            return;
        }

        memcpy (&remoteAddr.sin_addr, myHostent->h_addr,
          myHostent->h_length);
        remoteAddr.sin_family = AF_INET;
        remoteAddr.sin_port = 
	  htons((unsigned short) conn->svrVersion->reconnPort);

        conn->reconnectedSock = 
	  connectToRhostWithRaddr (&remoteAddr, conn->windowSize, 0);

        if (conn->reconnectedSock < 0) {
	    pthread_cond_signal (&conn->cond);
            pthread_mutex_unlock (&conn->lock);
            rodsLog (LOG_ERROR, 
	      "cliReconnManager: connect to host %s failed, status = %d",
              conn->svrVersion->reconnAddr, conn->reconnectedSock);
            rodsSleep (RECONNECT_SLEEP_TIME, 0);
            continue;
        }

        bzero (&reconnMsg, sizeof (procState_t));
        reconnMsg.procState = conn->clientState;
	reconnMsg.cookie = conn->svrVersion->cookie;
        status = sendReconnMsg (conn->reconnectedSock, &reconnMsg);

        if (status < 0) {
	    close (conn->reconnectedSock);
	    conn->reconnectedSock = 0;
	    pthread_cond_signal (&conn->cond);
            pthread_mutex_unlock (&conn->lock);
            rodsLog (LOG_ERROR,
              "cliReconnManager: sendReconnMsg to host %s failed, status = %d",
              conn->svrVersion->reconnAddr, status);
            rodsSleep (RECONNECT_SLEEP_TIME, 0);
            continue;
        }

        if ((status = readReconMsg (conn->reconnectedSock, &reconnMsgOut)) 
	  < 0) {
            close (conn->reconnectedSock);
            conn->reconnectedSock = 0;
	    pthread_cond_signal (&conn->cond);
            pthread_mutex_unlock (&conn->lock);
            rodsLog (LOG_ERROR,
              "cliReconnManager: readReconMsg to host %s failed, status = %d",
              conn->svrVersion->reconnAddr, status);
            rodsSleep (RECONNECT_SLEEP_TIME, 0);
            continue;
        }

        conn->agentState = reconnMsgOut->procState;
	free (reconnMsgOut);
	reconnMsgOut = NULL;
        conn->reconnTime = time (0) + RECONN_TIMEOUT_TIME;
        if (conn->clientState == PROCESSING_STATE) {
            rodsLog (LOG_DEBUG,
              "cliReconnManager: svrSwitchConnect. cliState = %d,agState=%d",
              conn->clientState, conn->agentState);
            cliSwitchConnect (conn);
        } else {
            rodsLog (LOG_DEBUG,
              "cliReconnManager: Not calling svrSwitchConnect,  clientState = %d", 
              conn->clientState);
	}
	pthread_cond_signal (&conn->cond);
        pthread_mutex_unlock (&conn->lock);
    }
}

int
cliChkReconnAtSendStart (rcComm_t *conn)
{
    if (conn->svrVersion != NULL && conn->svrVersion->reconnPort > 0) {
        /* handle reconn */
        pthread_mutex_lock (&conn->lock);
        if (conn->reconnThrState == CONN_WAIT_STATE) {
            rodsLog (LOG_DEBUG,
              "cliChkReconnAtSendStart:ThrState=CONN_WAIT_STATE,clientState=%d",
              conn->clientState);
            conn->clientState = PROCESSING_STATE;
            pthread_cond_signal (&conn->cond);
	    /* wait for reconnManager to get done */ 
	    pthread_cond_wait (&conn->cond, &conn->lock);
        }
        conn->clientState = SENDING_STATE;
        pthread_mutex_unlock (&conn->lock);
    }
    return 0;
}

int
cliChkReconnAtSendEnd (rcComm_t *conn)
{
    if (conn->svrVersion != NULL && conn->svrVersion->reconnPort > 0) {
        /* handle reconn */
        pthread_mutex_lock (&conn->lock);
        conn->clientState = PROCESSING_STATE;
        if (conn->reconnThrState == CONN_WAIT_STATE) {
            pthread_cond_signal (&conn->cond);
        }
        pthread_mutex_unlock (&conn->lock);
    }
    return 0;
}

int
cliChkReconnAtReadStart (rcComm_t *conn)
{
    if (conn->svrVersion != NULL && conn->svrVersion->reconnPort > 0) {
        /* handle reconn */
        pthread_mutex_lock (&conn->lock);
        conn->clientState = RECEIVING_STATE;
        pthread_mutex_unlock (&conn->lock);
    }
    return 0;
}

int
cliChkReconnAtReadEnd (rcComm_t *conn)
{
    if (conn->svrVersion != NULL && conn->svrVersion->reconnPort > 0) {
        /* handle reconn */
        pthread_mutex_lock (&conn->lock);
        conn->clientState = PROCESSING_STATE;
        if (conn->reconnThrState == CONN_WAIT_STATE) {
            rodsLog (LOG_DEBUG,
              "cliChkReconnAtReadEnd:ThrState=CONN_WAIT_STATE, clientState=%d",
              conn->clientState);
            pthread_cond_signal (&conn->cond);
            /* wait for reconnManager to get done */
            pthread_cond_wait (&conn->cond, &conn->lock);
        }
        pthread_mutex_unlock (&conn->lock);
    }
    return 0;
}

#endif

