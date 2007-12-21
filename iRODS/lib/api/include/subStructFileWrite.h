/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* bunSubWrite.h  
 */

#ifndef SUB_STRUCT_FILE_WRITE_H
#define SUB_STRUCT_FILE_WRITE_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "bundleDriver.h"
#include "subStructFileRead.h"

#if defined(RODS_SERVER)
#define RS_SUB_STRUCT_FILE_WRITE rsBunSubWrite
/* prototype for the server handler */
int
rsBunSubWrite (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubWriteInp,
bytesBuf_t *bunSubWriteOutBBuf);
int
_rsBunSubWrite (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubWriteInp,
bytesBuf_t *bunSubWriteOutBBuf);
int
remoteBunSubWrite (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubWriteInp,
bytesBuf_t *bunSubWriteOutBBuf, rodsServerHost_t *rodsServerHost);
#else
#define RS_SUB_STRUCT_FILE_WRITE NULL
#endif

/* prototype for the client call */
int
rcBunSubWrite (rcComm_t *conn, bunSubFdOprInp_t *bunSubWriteInp,
bytesBuf_t *bunSubWriteOutBBuf);

#endif	/* SUB_STRUCT_FILE_WRITE_H */
