/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* getXmsgTicket.c
 */

#include "getXmsgTicket.h"
#include "xmsgLib.h"

extern ticketHashQue_t *XmsgHashQue;

int
rsGetXmsgTicket (rsComm_t *rsComm, getXmsgTicketInp_t *getXmsgTicketInp,
xmsgTicketInfo_t **outXmsgTicketInfo)
{
    int status;
    int hashSlotNum;

    *outXmsgTicketInfo = calloc (1, sizeof (xmsgTicketInfo_t));

    (*outXmsgTicketInfo)->sendTicket = random();
    if ((*outXmsgTicketInfo)->expireTime > 0) {
	(*outXmsgTicketInfo)->expireTime = getXmsgTicketInp->expireTime;
    } else {
        (*outXmsgTicketInfo)->expireTime = time (0) + DEF_EXPIRE_INT;
    }

    while (1) {
        (*outXmsgTicketInfo)->rcvTicket = random();
        hashSlotNum = ticketHashFunc ((*outXmsgTicketInfo)->rcvTicket);
        status = addTicketToHQue (
	  *outXmsgTicketInfo, &XmsgHashQue[hashSlotNum]);
	if (status != SYS_DUPLICATE_XMSG_TICKET) {
	    break;
	}
    }

    if (status < 0) {
	free (*outXmsgTicketInfo);
	*outXmsgTicketInfo = NULL;
    }

    return (status);
}

