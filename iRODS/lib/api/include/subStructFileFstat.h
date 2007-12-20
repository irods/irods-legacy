/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* bunSubFstat.h  
 */

#ifndef BUN_SUB_FSTAT_H
#define BUN_SUB_FSTAT_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "bundleDriver.h"
#include "bunSubRead.h"

#if defined(RODS_SERVER)
#define RS_BUN_SUB_FSTAT rsBunSubFstat
/* prototype for the server handler */
int
rsBunSubFstat (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubFstatInp,
rodsStat_t **bunSubStatOut);
int
_rsBunSubFstat (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubFstatInp,
rodsStat_t **bunSubStatOut);
int
remoteBunSubFstat (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubFstatInp,
rodsStat_t **bunSubStatOut, rodsServerHost_t *rodsServerHost);
#else
#define RS_BUN_SUB_FSTAT NULL
#endif

/* prototype for the client call */
int
rcBunSubFstat (rcComm_t *conn, bunSubFdOprInp_t *bunSubFstatInp,
rodsStat_t **bunSubStatOut);

#endif	/* BUN_SUB_FSTAT_H */
