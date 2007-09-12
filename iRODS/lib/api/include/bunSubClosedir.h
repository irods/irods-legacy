/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* bunSubClosedir.h  
 */

#ifndef BUN_SUB_CLOSEDIR_H
#define BUN_SUB_CLOSEDIR_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "bundleDriver.h"
#include "bunSubRead.h"

#if defined(RODS_SERVER)
#define RS_BUN_SUB_CLOSEDIR rsBunSubClosedir
/* prototype for the server handler */
int
rsBunSubClosedir (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubClosedirInp);

int
_rsBunSubClosedir (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubClosedirInp);
int
remoteBunSubClosedir (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubClosedirInp,
rodsServerHost_t *rodsServerHost);
#else
#define RS_BUN_SUB_CLOSEDIR NULL
#endif

/* prototype for the client call */
int
rcBunSubClosedir (rcComm_t *conn, bunSubFdOprInp_t *bunSubClosedirInp);

#endif	/* BUN_SUB_CLOSEDIR_H */
