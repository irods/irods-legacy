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
ticketHashQue_t XmsgHashQue[NUM_HASH_SLOT];
xmsgQue_t XmsgQue;

int 
initThreadEnv ()
{
    pthread_mutex_init (&ReqQueCondMutex, NULL);
    pthread_cond_init (&ReqQueCond, NULL);

    return (0);
}

int
addXmsgToXmsgQue (irodsXmsg_t *xmsg, xmsgQue_t *xmsgQue)
{

    if (xmsg == NULL || xmsgQue == NULL) {
        rodsLog (LOG_ERROR,
          "addXmsgToQue: input xmsg or xmsgQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    xmsg->next = xmsg->prev = NULL;

    if (xmsgQue->head == NULL) {
	xmsgQue->head = xmsgQue->tail = xmsg;
    } else {
	/* que it on top */
	xmsgQue->head->prev = xmsg;
	xmsg->next = xmsgQue->head;
	xmsgQue->head = xmsg;
    }

    return (0);
}

int
rmXmsgFromXmsgQue (irodsXmsg_t *xmsg, xmsgQue_t *xmsgQue)
{
    if (xmsg == NULL || xmsgQue == NULL) {
        rodsLog (LOG_ERROR,
          "addXmsgToQue: input xmsg or xmsgQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (xmsg->prev == NULL) {
	/* at head */
	xmsgQue->head = xmsg->next;
    } else {
	xmsg->prev->next = xmsg->next;
    }

    if (xmsg->next == NULL) {
	/* at tail */
        xmsgQue->tail = xmsg->prev;
    } else {
	xmsg->next->prev = xmsg->prev;
    }
 
    xmsg->prev = xmsg->next = NULL;

    return (0);
}

int
addXmsgToTicketMsgStruct (irodsXmsg_t *xmsg, 
ticketMsgStruct_t *ticketMsgStruct)
{
    irodsXmsg_t *tmpXmsg;

    if (xmsg == NULL || ticketMsgStruct == NULL) {
        rodsLog (LOG_ERROR,
          "addXmsgToTicketMsgStruct: input xmsg or ticketMsgStruct is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (ticketMsgStruct->xmsgHead == NULL) {
	ticketMsgStruct->xmsgHead = xmsg;
	xmsg->tnext = NULL;
    } else {
	/* link it to the end */
	tmpXmsg = ticketMsgStruct->xmsgHead;
	while (tmpXmsg->next != NULL) {
	    tmpXmsg = tmpXmsg->next;
	}
	tmpXmsg->next = tmpXmsg;
	tmpXmsg->next = NULL;
    }

    return (0);
}

int
addTicketToHQue (xmsgTicketInfo_t *ticket, ticketHashQue_t *ticketHQue)
{
    int status;

    ticketMsgStruct_t *tmpTicketMsgStruct;

    if (ticket == NULL || ticketHQue == NULL) {
        rodsLog (LOG_ERROR,
          "addTicketToHQue: input ticket or ticketHQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    tmpTicketMsgStruct = calloc (1, sizeof (ticketMsgStruct_t));

    /* copy the content of the ticket */

    tmpTicketMsgStruct->ticket = *ticket;
    status = addTicketMsgStructToHQue (tmpTicketMsgStruct, ticketHQue);

    if (status < 0) {
	free (tmpTicketMsgStruct);
    }

    return (status);
}

int
addTicketMsgStructToHQue (ticketMsgStruct_t *ticketMsgStruct, 
ticketHashQue_t *ticketHQue)
{
    ticketMsgStruct_t *tmpTicketMsgStruct;

    if (ticketMsgStruct == NULL || ticketHQue == NULL) {
        rodsLog (LOG_ERROR,
          "addTicketMsgStructToHQue: ticketMsgStruct or ticketHQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    ticketMsgStruct->hnext = ticketMsgStruct->hprev = NULL;

    if (ticketHQue->head == NULL) {
	ticketHQue->head = ticketHQue->tail = ticketMsgStruct;
	return (0);
    }

    
    /* que in decending order of rcvTicket */
    tmpTicketMsgStruct = ticketHQue->head;
    while (tmpTicketMsgStruct != NULL) {
	if (ticketMsgStruct->ticket.rcvTicket == 
	  tmpTicketMsgStruct->ticket.rcvTicket) {
	    return (SYS_DUPLICATE_XMSG_TICKET);
	} else if (ticketMsgStruct->ticket.rcvTicket > 
	    tmpTicketMsgStruct->ticket.rcvTicket) {
	    break;
	} else {
	    tmpTicketMsgStruct = tmpTicketMsgStruct->hnext;
	}
    }
    if (tmpTicketMsgStruct == NULL) {
	/* reached the end */
	ticketHQue->tail->hnext = ticketMsgStruct;
	ticketMsgStruct->hprev = ticketHQue->tail;
	ticketHQue->tail = ticketMsgStruct;
    } else if (tmpTicketMsgStruct == ticketHQue->head) {
	/* need to put ticketMsgStruct at the head */
	ticketHQue->head->hprev = ticketMsgStruct;
	ticketMsgStruct->hnext = ticketHQue->head;
	ticketHQue->head = ticketMsgStruct;
    } else {
	/* in the middle */
	ticketMsgStruct->hprev = tmpTicketMsgStruct->hprev;
	ticketMsgStruct->hnext = tmpTicketMsgStruct;
	tmpTicketMsgStruct->hprev->hnext = ticketMsgStruct;
	tmpTicketMsgStruct->hprev = tmpTicketMsgStruct;
    }
	
    return (0);
}

int
rmTicketMsgStructFromHQue (ticketMsgStruct_t *ticketMsgStruct, 
ticketHashQue_t *ticketHQue)
{
    if (ticketMsgStruct == NULL || ticketHQue == NULL) {
        rodsLog (LOG_ERROR,
          "rmTicketMsgStructFromHQue: ticketMsgStruct or ticketHQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (ticketMsgStruct->hprev == NULL) {
	/* at head */
	ticketHQue->head = ticketMsgStruct->hnext;
    } else {
	ticketMsgStruct->hprev->hnext = ticketMsgStruct->hnext;
    }

    if (ticketMsgStruct->hnext == NULL) {
	/* at tail */
        ticketHQue->tail = ticketMsgStruct->hprev;
    } else {
	ticketMsgStruct->hnext->hprev = ticketMsgStruct->hprev;
    }
 
    ticketMsgStruct->hprev = ticketMsgStruct->hnext = NULL;

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
	close (rsComm.sock);
	free (myXmsgReq);
    }
}

/* The hash function which use rcvTicket as the key. It take the modulo of
 * rcvTicket/NUM_HASH_SLOT
 */

int
ticketHashFunc (uint rcvTicket)
{
    int mySlot = rcvTicket % NUM_HASH_SLOT;

    return (mySlot); 
}

int
initXmsgHashQue ()
{
    memset (XmsgHashQue, 0, NUM_HASH_SLOT * sizeof (ticketHashQue_t));
    memset (&XmsgQue, 0, sizeof (XmsgQue));

    return (0);
}

int
getTicketMsgStructByTicket (uint rcvTicket, 
ticketMsgStruct_t **outTicketMsgStruct)
{
    int hashSlotNum;
    ticketMsgStruct_t *tmpTicketMsgStruct;

    hashSlotNum = ticketHashFunc (rcvTicket);

    tmpTicketMsgStruct = XmsgHashQue[hashSlotNum].head;

    while (tmpTicketMsgStruct != NULL) {
	if (rcvTicket == tmpTicketMsgStruct->ticket.rcvTicket) {
	    *outTicketMsgStruct = tmpTicketMsgStruct;
	    return 0;
	} else if (rcvTicket > tmpTicketMsgStruct->ticket.rcvTicket) {
	    *outTicketMsgStruct = NULL;
	    return SYS_UNMATCHED_XMSG_TICKET;
	} else {
	    tmpTicketMsgStruct = tmpTicketMsgStruct->hnext;
	}
    }

    /* no match */
    *outTicketMsgStruct = NULL;
    return SYS_UNMATCHED_XMSG_TICKET;
}

