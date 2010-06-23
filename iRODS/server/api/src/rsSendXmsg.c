/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* sendXmsg.c
 */

#include "sendXmsg.h"
#include "xmsgLib.h"

extern ticketHashQue_t XmsgHashQue[];
extern xmsgQue_t XmsgQue;

int
rsSendXmsg (rsComm_t *rsComm, sendXmsgInp_t *sendXmsgInp)
{
    int status;
    ticketMsgStruct_t *ticketMsgStruct = NULL;
    irodsXmsg_t *irodsXmsg;
    

    status = getTicketMsgStructByTicket (sendXmsgInp->ticket.rcvTicket, 
      &ticketMsgStruct);

    if (status < 0) {
	clearSendXmsgInfo (&sendXmsgInp->sendXmsgInfo);
	return status;
    }

    /* match sendTicket */
    if (ticketMsgStruct->ticket.sendTicket != sendXmsgInp->ticket.sendTicket) {
	/* unmatched sendTicket */
	rodsLog (LOG_ERROR,
	  "rsSendXmsg: sendTicket mismatch, input %d, in cache %d",
	    sendXmsgInp->ticket.sendTicket, ticketMsgStruct->ticket.sendTicket);
	return (SYS_UNMATCHED_XMSG_TICKET);
    }

    /* create a irodsXmsg_t */
 
    irodsXmsg = calloc (1, sizeof (irodsXmsg_t));
    irodsXmsg->sendXmsgInfo = calloc (1, sizeof (sendXmsgInfo_t));
    *irodsXmsg->sendXmsgInfo = sendXmsgInp->sendXmsgInfo;
    irodsXmsg->sendTime = time (0);
    rstrcpy (irodsXmsg->sendUserName, rsComm->clientUser.userName, NAME_LEN);
    rstrcpy (irodsXmsg->sendAddr,sendXmsgInp->sendAddr, NAME_LEN);
    addXmsgToXmsgQue (irodsXmsg, &XmsgQue);

    status = addXmsgToTicketMsgStruct (irodsXmsg, ticketMsgStruct);

    return (status);
}

