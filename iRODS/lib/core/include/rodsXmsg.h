/*** Copyright (c), The Regents of the University of California            ***
 *** For more inforeplation please refer to files in the COPYRIGHT directory ***/
/* rodsXmsg.h - Header for for xmsg  */

#ifndef RODS_XMSG_H
#define RODS_XMSG_H

#define DEF_EXPIRE_INT	(3600 * 24)	/* default expire interval */

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct {
    uint expireTime;                    /* unix time of ticket expiration */
} getXmsgTicketInp_t;

typedef struct XmsgTicketInfo {
    uint sendTicket;
    uint rcvTicket;
    uint expireTime;                    /* unix time of ticket expiration */
} xmsgTicketInfo_t;

typedef struct SendXmsgInfo {
    uint msgNumber;
    char msgType[HEADER_TYPE_LEN];      /* msg type, 16 char */
    uint numRcv;                        /* number of receiver */
    char *msg;                          /* the msg */
    int numDel;                         /* number of msg to deliver */
    char **deliAddress;                  /* array of str pointer of addr */
    uint *deliPort;                      /* array of port number to deliver */
    char *miscInfo;                     /* for expiration, etc */
} sendXmsgInfo_t;

typedef struct {
    xmsgTicketInfo_t ticket;            /* xmsgTicketInfo from getXmsgTicket */
    sendXmsgInfo_t *sendXmsgInfo;
} sendXmsgInp_t;

/* xmsg struct */

typedef struct IrodsXmsg {
    sendXmsgInfo_t *sendXmsgInfo;
    uint sendTime;                      /* unix time of the send */
    char sendUserName[NAME_LEN];        /* userName@zone of clientUser */
    char sendAddr[NAME_LEN];            /* sender's network address*/
    struct IrodsXmsg *prev;		/* the link list with XmsgReqHead */
    struct IrodsXmsg *next;
    struct IrodsXmsg *tnext;		/* the link list within the same
					 * ticket (diff msgNumber) */
    void *ticketMsgStruct;		/* points to the ticketMsgStruct_t this
					 * xmsg belongs */
} irodsXmsg_t;

typedef struct XmsgQue {
    irodsXmsg_t *head;
    irodsXmsg_t *tail;
} xmsgQue_t;

typedef struct TicketMsgStruct {
    xmsgTicketInfo_t ticket;
    irodsXmsg_t *xmsgHead;     /* the link list of msg with the same ticket */
    struct TicketMsgStruct *hprev;    /* the hash link list. sort by rcvTicket 
				       */
    struct TicketMsgStruct *hnext;
    void *xmsgHashQue;		/* points to the xmsgHashQue_t this ticket
				 * belongs */
} ticketMsgStruct_t;

/* queue of msg hashed to the same slot */
typedef struct TicketHashQue {
    ticketMsgStruct_t *head;
    ticketMsgStruct_t *tail;
} ticketHashQue_t;

typedef struct XmsgReq {
    int sock;
    struct XmsgReq *next;
} xmsgReq_t;

#ifdef  __cplusplus
}
#endif

#endif	/* RODS_XMSG_H */
