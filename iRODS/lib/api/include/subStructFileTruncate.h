/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* bunSubTruncate.h  
 */

#ifndef SUB_STRUCT_FILE_TRUNCATE_H
#define SUB_STRUCT_FILE_TRUNCATE_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "bundleDriver.h"
#include "fileTruncate.h"

#if defined(RODS_SERVER)
#define RS_SUB_STRUCT_FILE_TRUNCATE rsBunSubTruncate
/* prototype for the server handler */
int
rsBunSubTruncate (rsComm_t *rsComm, subFile_t *bunSubTruncateInp);
int
_rsBunSubTruncate (rsComm_t *rsComm, subFile_t *bunSubTruncateInp);
int
remoteBunSubTruncate (rsComm_t *rsComm, subFile_t *bunSubTruncateInp, 
rodsServerHost_t *rodsServerHost);
#else
#define RS_SUB_STRUCT_FILE_TRUNCATE NULL
#endif

/* prototype for the client call */
int
rcBunSubTruncate (rcComm_t *conn, subFile_t *bunSubTruncateInp);

#endif	/* SUB_STRUCT_FILE_TRUNCATE_H */
