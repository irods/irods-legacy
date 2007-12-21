/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* bunSubRename.h  
 */

#ifndef SUB_STRUCT_FILE_RENAME_H
#define SUB_STRUCT_FILE_RENAME_H

/* This is Object File I/O type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "bundleDriver.h"

typedef struct BunSubRenameInp {
    subFile_t subFile;
    char newSubFilePath[MAX_NAME_LEN];
} bunSubRenameInp_t;

#define BunSubRenameInp_PI "struct SubFile_PI; str newSubFilePath[MAX_NAME_LEN];"
#if defined(RODS_SERVER)
#define RS_SUB_STRUCT_FILE_RENAME rsBunSubRename
/* prototype for the server handler */
int
rsBunSubRename (rsComm_t *rsComm, bunSubRenameInp_t *bunSubRenameInp);
int
_rsBunSubRename (rsComm_t *rsComm, bunSubRenameInp_t *bunSubRenameInp);
int
remoteBunSubRename (rsComm_t *rsComm, bunSubRenameInp_t *bunSubRenameInp,
rodsServerHost_t *rodsServerHost);
#else
#define RS_SUB_STRUCT_FILE_RENAME NULL
#endif

/* prototype for the client call */
int
rcBunSubRename (rcComm_t *conn, bunSubRenameInp_t *bunSubRenameInp);

#endif	/* SUB_STRUCT_FILE_RENAME_H */
