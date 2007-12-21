/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* bunSubRead.h  
 */

#ifndef SUB_STRUCT_FILE_READ_H
#define SUB_STRUCT_FILE_READ_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "bundleDriver.h"

typedef struct BunSubFdOpr {
    rodsHostAddr_t addr;
    bunType_t type;
    int fd;
    int len;
} bunSubFdOprInp_t;

#define BunSubFdOpr_PI "struct RHostAddr_PI; int type; int fd; int len;"

#if defined(RODS_SERVER)
#define RS_SUB_STRUCT_FILE_READ rsBunSubRead
/* prototype for the server handler */
int
rsBunSubRead (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubReadInp,
bytesBuf_t *bunSubReadOutBBuf);
int
_rsBunSubRead (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubReadInp,
bytesBuf_t *bunSubReadOutBBuf);
int
remoteBunSubRead (rsComm_t *rsComm, bunSubFdOprInp_t *bunSubReadInp,
bytesBuf_t *bunSubReadOutBBuf, rodsServerHost_t *rodsServerHost);
#else
#define RS_SUB_STRUCT_FILE_READ NULL
#endif

/* prototype for the client call */
int
rcBunSubRead (rcComm_t *conn, bunSubFdOprInp_t *bunSubReadInp,
bytesBuf_t *bunSubReadOutBBuf);

#endif	/* SUB_STRUCT_FILE_READ_H */
