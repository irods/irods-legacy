/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* bunSubRmdir.h  
 */

#ifndef BUN_SUB_RMDIR_H
#define BUN_SUB_RMDIR_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "bundleDriver.h"

#if defined(RODS_SERVER)
#define RS_BUN_SUB_RMDIR rsBunSubRmdir
/* prototype for the server handler */
int
rsBunSubRmdir (rsComm_t *rsComm, subFile_t *subFile);
int
_rsBunSubRmdir (rsComm_t *rsComm, subFile_t *subFile);
int
remoteBunSubRmdir (rsComm_t *rsComm, subFile_t *subFile,
rodsServerHost_t *rodsServerHost);
#else
#define RS_BUN_SUB_RMDIR NULL
#endif

/* prototype for the client call */
int
rcBunSubRmdir (rcComm_t *conn, subFile_t *subFile);

#endif	/* BUN_SUB_RMDIR_H */
