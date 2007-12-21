/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* bunSubClose.h  
 */

#ifndef SUB_STRUCT_FILE_CLOSE_H
#define SUB_STRUCT_FILE_CLOSE_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "bundleDriver.h"
#include "subStructFileRead.h"

#if defined(RODS_SERVER)
#define RS_SUB_STRUCT_FILE_CLOSE rsBunSubClose
/* prototype for the server handler */
int
rsBunSubClose (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubCloseInp);

int
_rsBunSubClose (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubCloseInp);
int
remoteBunSubClose (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubCloseInp,
rodsServerHost_t *rodsServerHost);
#else
#define RS_SUB_STRUCT_FILE_CLOSE NULL
#endif

/* prototype for the client call */
int
rcBunSubClose (rcComm_t *conn, bunSubFdOprInp_t *bunSubCloseInp);

#endif	/* SUB_STRUCT_FILE_CLOSE_H */
