/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* xmsgLib.c - library routines for irodsXmsg
 */

#include <pthread.h>
#include "xmsgLib.h"
#include "rsApiHandler.h"

pthread_mutex_t ReqQueCondMutex;
pthread_cond_t ReqQueCond;
pthread_cond_t ReqQueCond;
pthread_t ProcReqThread;
xmsgReq_t *XmsgReqHead = NULL;

int 
initThreadEnv ()
{
    pthread_mutex_init (&ReqQueCondMutex, NULL);
    pthread_cond_init (&ReqQueCond, NULL);

    return (0);
}

int
addXmsgStructToQue (xmsgStruct_t *xmsgStruct, xmsgQue_t *xmsgQue)
{

    if (xmsgStruct == NULL || xmsgQue == NULL) {
        rodsLog (LOG_ERROR,
          "addXmsgToQue: input xmsgStruct or xmsgQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    xmsgStruct->next = xmsgStruct->prev = NULL;

    if (xmsgQue->head == NULL) {
	xmsgQue->head = xmsgQue->tail = xmsgStruct;
    } else {
	/* que it on top */
	xmsgQue->head->prev = xmsgStruct;
	xmsgStruct->next = xmsgQue->head;
	xmsgQue->head = xmsgStruct;
    }

    return (0);
}

int
rmXmsgStructFromQue (xmsgStruct_t *xmsgStruct, xmsgQue_t *xmsgQue)
{
    if (xmsgStruct == NULL || xmsgQue == NULL) {
        rodsLog (LOG_ERROR,
          "addXmsgToQue: input xmsgStruct or xmsgQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (xmsgStruct->prev == NULL) {
	/* at head */
	xmsgQue->head = xmsgStruct->next;
    } else {
	xmsgStruct->prev->next = xmsgStruct->next;
    }

    if (xmsgStruct->next == NULL) {
	/* at tail */
        xmsgQue->tail = xmsgStruct->prev;
    } else {
	xmsgStruct->next->prev = xmsgStruct->prev;
    }
 
    xmsgStruct->prev = xmsgStruct->next = NULL;

    return (0);
}

int
addXmsgStructToHQue (xmsgStruct_t *xmsgStruct, xmsgQue_t *xmsgQue)
{

    if (xmsgStruct == NULL || xmsgQue == NULL) {
        rodsLog (LOG_ERROR,
          "addXmsgToQue: input xmsgStruct or xmsgQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    xmsgStruct->hnext = xmsgStruct->hprev = NULL;

    if (xmsgQue->head == NULL) {
	xmsgQue->head = xmsgQue->tail = xmsgStruct;
    } else {
	/* que it on top */
	xmsgQue->head->hprev = xmsgStruct;
	xmsgStruct->hnext = xmsgQue->head;
	xmsgQue->head = xmsgStruct;
    }

    return (0);
}

int
rmXmsgStructFromHQue (xmsgStruct_t *xmsgStruct, xmsgQue_t *xmsgQue)
{
    if (xmsgStruct == NULL || xmsgQue == NULL) {
        rodsLog (LOG_ERROR,
          "addXmsgToQue: input xmsgStruct or xmsgQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (xmsgStruct->hprev == NULL) {
	/* at head */
	xmsgQue->head = xmsgStruct->hnext;
    } else {
	xmsgStruct->hprev->hnext = xmsgStruct->hnext;
    }

    if (xmsgStruct->hnext == NULL) {
	/* at tail */
        xmsgQue->tail = xmsgStruct->hprev;
    } else {
	xmsgStruct->hnext->hprev = xmsgStruct->hprev;
    }
 
    xmsgStruct->hprev = xmsgStruct->hnext = NULL;

    return (0);
}

/* add incoming request to the bottom of the link list */ 

int
addReqToQue (int sock)
{
    xmsgReq_t *myXmsgReq, *tmpXmsgReq;

    pthread_mutex_lock (&ReqQueCondMutex);
    myXmsgReq = calloc (1, sizeof (xmsgReq_t));

    myXmsgReq->sock = sock;

    if (XmsgReqHead == NULL) {
	XmsgReqHead = myXmsgReq;
    } else {
        tmpXmsgReq = XmsgReqHead;
	while (tmpXmsgReq->next != NULL) {
	    tmpXmsgReq = tmpXmsgReq->next;
	}
	tmpXmsgReq->next = myXmsgReq;
    }
    pthread_cond_signal (&ReqQueCond);
    pthread_mutex_unlock (&ReqQueCondMutex);

    return (0);
}

xmsgReq_t *
getReqFromQue ()
{
    xmsgReq_t *myXmsgReq = NULL;

    while (myXmsgReq == NULL) {
        pthread_mutex_lock (&ReqQueCondMutex);
        if (XmsgReqHead != NULL) {
            myXmsgReq = XmsgReqHead;
            XmsgReqHead = XmsgReqHead->next;
            pthread_mutex_unlock (&ReqQueCondMutex);
            break;
	}
	pthread_cond_wait (&ReqQueCond, &ReqQueCondMutex);
        if (XmsgReqHead == NULL) {
	    pthread_mutex_unlock (&ReqQueCondMutex);
	    continue;
	} else {
            myXmsgReq = XmsgReqHead;
    	    XmsgReqHead = XmsgReqHead->next;
	    pthread_mutex_unlock (&ReqQueCondMutex);
	    break;
	}
    }

    return (myXmsgReq);
}

int
startXmsgThreads ()
{
    int status;

    status = pthread_create(&ProcReqThread, NULL, 
      (void *(*)(void *)) procReqRoutine, (void *) NULL);

    return (status);
}

void
procReqRoutine ()
{
    xmsgReq_t *myXmsgReq = NULL;
    startupPack_t *startupPack;
    rsComm_t rsComm;
    int status;
    fd_set sockMask;
    struct timeval msgTimeout;

    while (1) {
	myXmsgReq = getReqFromQue ();
	if (myXmsgReq == NULL) {
            rodsLog (LOG_ERROR,
              "procReqRoutine: getReqFromQue error");
	    continue;
	}

        status = readStartupPack (myXmsgReq->sock, &startupPack);
	if (status < 0) {
            rodsLog (LOG_ERROR,
              "procReqRoutine: readStartupPack error, status = %d", status);
	    free (myXmsgReq);
            continue;
	}
	memset (&rsComm, 0, sizeof (rsComm));
	initRsCommWithStartupPack (&rsComm, startupPack);
	rsComm.sock = myXmsgReq->sock;
        status = sendVersion (rsComm.sock, 0, 0, NULL, 0);

        if (status < 0) {
            sendVersion (rsComm.sock, SYS_AGENT_INIT_ERR, 0, NULL, 0);
	    free (myXmsgReq);
            continue;
        }
        FD_ZERO(&sockMask);
	memset (&msgTimeout, 0, sizeof (msgTimeout));
	msgTimeout.tv_sec = REQ_MSG_TIMEOUT_TIME;
	while (1) {
	    int numSock;

    	    FD_SET (rsComm.sock, &sockMask);
            while ((numSock = select (rsComm.sock + 1, &sockMask,
              (fd_set *) NULL, (fd_set *) NULL, &msgTimeout)) < 0) {
                if (errno == EINTR) {
                    rodsLog (LOG_NOTICE, 
		      "procReqRoutine: select() interrupted");
                    FD_SET(rsComm.sock, &sockMask);
                    continue;
                } else {
		    break;	/* timedout or something */
                }
	    }
	    if (numSock < 0) break;
            status = readAndProcClientMsg (&rsComm, 0);
	    if (status < 0) break;
        }
    }
}

