/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* bunSubReaddir.h  
 */

#ifndef SUB_STRUCT_FILE_READDIR_H
#define SUB_STRUCT_FILE_READDIR_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "bundleDriver.h"
#include "subStructFileRead.h"

#if defined(RODS_SERVER)
#define RS_SUB_STRUCT_FILE_READDIR rsBunSubReaddir
/* prototype for the server handler */
int
rsBunSubReaddir (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubReaddirInp,
rodsDirent_t **rodsDirent);
int
_rsBunSubReaddir (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubReaddirInp,
rodsDirent_t **rodsDirent);
int
remoteBunSubReaddir (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubReaddirInp,
rodsDirent_t **rodsDirent, rodsServerHost_t *rodsServerHost);
#else
#define RS_SUB_STRUCT_FILE_READDIR NULL
#endif

/* prototype for the client call */
int
rcBunSubReaddir (rcComm_t *conn, bunSubFdOprInp_t *bunSubReaddirInp,
rodsDirent_t **rodsDirent);

#endif	/* SUB_STRUCT_FILE_READDIR_H */
