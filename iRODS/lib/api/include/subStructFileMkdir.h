/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* bunSubMkdir.h  
 */

#ifndef SUB_STRUCT_FILE_MKDIR_H
#define SUB_STRUCT_FILE_MKDIR_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "bundleDriver.h"

#if defined(RODS_SERVER)
#define RS_SUB_STRUCT_FILE_MKDIR rsBunSubMkdir
/* prototype for the server handler */
int
rsBunSubMkdir (rsComm_t *rsComm, subFile_t *subFile);
int
_rsBunSubMkdir (rsComm_t *rsComm, subFile_t *subFile);
int
remoteBunSubMkdir (rsComm_t *rsComm, subFile_t *subFile,
rodsServerHost_t *rodsServerHost);
#else
#define RS_SUB_STRUCT_FILE_MKDIR NULL
#endif

/* prototype for the client call */
int
rcBunSubMkdir (rcComm_t *conn, subFile_t *subFile);

#endif	/* SUB_STRUCT_FILE_MKDIR_H */
