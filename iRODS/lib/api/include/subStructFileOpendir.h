/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* bunSubOpendir.h  
 */

#ifndef BUN_SUB_OPENDIR_H
#define BUN_SUB_OPENDIR_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "bundleDriver.h"

#if defined(RODS_SERVER)
#define RS_BUN_SUB_OPENDIR rsBunSubOpendir
/* prototype for the server handler */
int
rsBunSubOpendir (rsComm_t *rsComm, subFile_t *subFile);
int
_rsBunSubOpendir (rsComm_t *rsComm, subFile_t *subFile);
int
remoteBunSubOpendir (rsComm_t *rsComm, subFile_t *subFile,
rodsServerHost_t *rodsServerHost);
#else
#define RS_BUN_SUB_OPENDIR NULL
#endif

/* prototype for the client call */
int
rcBunSubOpendir (rcComm_t *conn, subFile_t *subFile);

#endif	/* BUN_SUB_OPENDIR_H */
