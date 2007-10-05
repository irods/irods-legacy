/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* xmsgLib.h - header file for xmsgLib
 */



#ifndef XMSG_LIB_H
#define XMSG_LIB_H

#include "rods.h"
#include "rsGlobalExtern.h"   /* server global */
#include "rcGlobalExtern.h"     /* client global */
#include "rsLog.h" 
#include "rodsLog.h"
#include "sockComm.h"
#include "rsMisc.h"
#include "getRodsEnv.h"
#include "rcConnect.h"
#include "initServer.h"

#define REQ_MSG_TIMEOUT_TIME	5	/* 5 sec timeout for req msg */

typedef struct XmsgStruct {
    irodsXmsg_t *xmsg;
    struct XmsgStruct *prev;     /* the link list. Last msg queued at top */
    struct XmsgStruct *next;
    struct XmsgStruct *hprev;    /* the hash link list. sort by rcvTicket and
                                  * then msgNumber */
    struct XmsgStruct *hnext;
} xmsgStruct_t;

typedef struct XmsgQue {
    xmsgStruct_t *head;
    xmsgStruct_t *tail;
} xmsgQue_t;

typedef struct XmsgReq {
    int sock;
    struct XmsgReq *next;
} xmsgReq_t; 

int 
initThreadEnv ();
int
addXmsgToQue (irodsXmsg_t *xmsg, xmsgQue_t *xmsgQue);
int
rmXmsgStructFromQue (xmsgStruct_t *xmsgStruct, xmsgQue_t *xmsgQue);
int
addXmsgStructToHQue (xmsgStruct_t *xmsgStruct, xmsgQue_t *xmsgQue);
int
rmXmsgStructFromHQue (xmsgStruct_t *xmsgStruct, xmsgQue_t *xmsgQue);
int
addReqToQue (int sock);
xmsgReq_t *getReqFromQue ();
int
startXmsgThreads ();
void
procReqRoutine ();
#endif	/* XMSG_LIB_H */

