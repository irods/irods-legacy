/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* See ticketAdmin.h for a description of this API call.*/

#include "ticketAdmin.h"
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"

int
rsTicketAdmin (rsComm_t *rsComm, ticketAdminInp_t *ticketAdminInp )
{
    rodsServerHost_t *rodsServerHost;
    int status;

    rodsLog(LOG_DEBUG, "ticketAdmin");

    status = getAndConnRcatHost(rsComm, MASTER_RCAT, NULL, &rodsServerHost);
    if (status < 0) {
       return(status);
    }

    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
       status = _rsTicketAdmin (rsComm, ticketAdminInp);
#else
       status = SYS_NO_RCAT_SERVER_ERR;
#endif
    }
    else {
       if (strcmp(ticketAdminInp->arg1,"session")==0 ) {
	  ticketAdminInp->arg3 = rsComm->clientAddr;
       }
       status = rcTicketAdmin(rodsServerHost->conn,
                            ticketAdminInp);
    }

    if (status < 0) { 
       rodsLog (LOG_NOTICE,
                "rsTicketAdmin: rcTicketAdmin failed");
    }
    return (status);
}

#ifdef RODS_CAT
int
_rsTicketAdmin(rsComm_t *rsComm, ticketAdminInp_t *ticketAdminInp )
{
    int status;

    status = chlModTicket(rsComm, ticketAdminInp->arg1, 
			  ticketAdminInp->arg2, ticketAdminInp->arg3, 
			  ticketAdminInp->arg4, ticketAdminInp->arg5);
    return(status);
}
#endif
