/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* bunSubStat.h  
 */

#ifndef SUB_STRUCT_FILE_STAT_H
#define SUB_STRUCT_FILE_STAT_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "bundleDriver.h"

#if defined(RODS_SERVER)
#define RS_SUB_STRUCT_FILE_STAT rsBunSubStat
/* prototype for the server handler */
int
rsBunSubStat (rsComm_t *rsComm, subFile_t *subFile, rodsStat_t **bunSubStatOut);
int
_rsBunSubStat (rsComm_t *rsComm, subFile_t *subFile, 
rodsStat_t **bunSubStatOut);
int
remoteBunSubStat (rsComm_t *rsComm, subFile_t *subFile,
rodsStat_t **bunSubStatOut, rodsServerHost_t *rodsServerHost);
#else
#define RS_SUB_STRUCT_FILE_STAT NULL
#endif

/* prototype for the client call */
int
rcBunSubStat (rcComm_t *conn, subFile_t *subFile,
rodsStat_t **bunSubStatOut);

#endif	/* SUB_STRUCT_FILE_STAT_H */
