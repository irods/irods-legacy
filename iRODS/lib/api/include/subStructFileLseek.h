/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* bunSubLseek.h  
 */

#ifndef BUN_SUB_LSEEK_H
#define BUN_SUB_LSEEK_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "bundleDriver.h"
#include "fileLseek.h"

typedef struct BunSubLseekInp {
    rodsHostAddr_t addr;
    bunType_t type;
    int fd;
    rodsLong_t offset;
    int whence;
} bunSubLseekInp_t;

#define BunSubLseekInp_PI "struct RHostAddr_PI; int type; int fd; double offset; int whence;"

#if defined(RODS_SERVER)
#define RS_BUN_SUB_LSEEK rsBunSubLseek
/* prototype for the server handler */
int
rsBunSubLseek (rsComm_t *rsComm, bunSubLseekInp_t *bunSubLseekInp,
fileLseekOut_t **bunSubLseekOut);
int
_rsBunSubLseek (rsComm_t *rsComm, bunSubLseekInp_t *bunSubLseekInp,
fileLseekOut_t **bunSubLseekOut);
int
remoteBunSubLseek (rsComm_t *rsComm, bunSubLseekInp_t *bunSubLseekInp, 
fileLseekOut_t **bunSubLseekOut, rodsServerHost_t *rodsServerHost);
#else
#define RS_BUN_SUB_LSEEK NULL
#endif

/* prototype for the client call */
int
rcBunSubLseek (rcComm_t *conn, bunSubLseekInp_t *bunSubLseekInp,
fileLseekOut_t **bunSubLseekOut);

#endif	/* BUN_SUB_LSEEK_H */
