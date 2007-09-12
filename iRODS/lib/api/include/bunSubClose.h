/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* bunSubClose.h  
 */

#ifndef BUN_SUB_CLOSE_H
#define BUN_SUB_CLOSE_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "bundleDriver.h"
#include "bunSubRead.h"

#if defined(RODS_SERVER)
#define RS_BUN_SUB_CLOSE rsBunSubClose
/* prototype for the server handler */
int
rsBunSubClose (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubCloseInp);

int
_rsBunSubClose (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubCloseInp);
int
remoteBunSubClose (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubCloseInp,
rodsServerHost_t *rodsServerHost);
#else
#define RS_BUN_SUB_CLOSE NULL
#endif

/* prototype for the client call */
int
rcBunSubClose (rcComm_t *conn, bunSubFdOprInp_t *bunSubCloseInp);

#endif	/* BUN_SUB_CLOSE_H */
