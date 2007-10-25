/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rcvXmsg.c
 */

#include "rcvXmsg.h"
#include "xmsgLib.h"

extern ticketHashQue_t XmsgHashQue[];
extern xmsgQue_t XmsgQue;

int
rsRcvXmsg (rsComm_t *rsComm, rcvXmsgInp_t *rcvXmsgInp, 
rcvXmsgOut_t **rcvXmsgOut)
{
    int status;
    ticketMsgStruct_t *ticketMsgStruct = NULL;

    status = getTicketMsgStructByTicket (rcvXmsgInp->rcvTicket, 
      &ticketMsgStruct);

    if (status < 0) {
	return status;
    }

    /* get the msg */
 
    *rcvXmsgOut = calloc (1, sizeof (rcvXmsgOut_t));
#if 0	/* XXXXX do this later */
    status = getXmsgInTicketMsgStruct (rcvXmsgInp->msgNumber, ticketMsgStruct,
      *rcvXmsgOut);
#endif

    return (status);
}

