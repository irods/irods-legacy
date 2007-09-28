/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* irodsXmsgServer.h - header file for irodsXmsgServer
 */



#ifndef IRODS_XMSG_SERVER_H
#define IRODS_XMSG_SERVER_H

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

typedef struct IrodsXmsg {
    uint sendTicket;
    uint rcvTicket;
    uint msgNumber;
    uint sendTime;			/* unix time of the send */
    uint numRcv;			/* nmber of receiver */
    uint msgType[HEADER_TYPE_LEN];	/* msg type, 16 char */
    char sendUserName[NAME_LEN];	/* userName@zone of clientUser */
    char sendAddr[NAME_LEN];		/* sender's network address*/
    char *msg;				/* the msg */
    int numDel;				/* number of msg to deliver */
    char **delAddress;			/* array of str pointer of addr */
    uint **delPort;			/* array of port number to deliver */
    char *miscInfo;			/* for expiration, etc */
} irodsXmsg_t;

typedef struct XmsgStruct {
    irodsXmsg_t *xmsg;
    struct XmsgStruct *prev;	 /* the link list. Last msg queued at top */  
    struct XmsgStruct *next;	
    struct XmsgStruct *hprev;    /* the hash link list. sort by rcvTicket and
				  * then msgNumber */
    struct XmsgStruct *hnext;
} xmsgStruct_t;

typedef struct XmsgQue {
    xmsgStruct_t *head;
    xmsgStruct_t *tail;
} xmsgQue_t;

#endif	/* IRODS_XMSG_SERVER_H */
