/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* rodsAgent.h - header file for rodsAgent
 */



#ifndef RODS_AGENT_H
#define RODS_AGENT_H

#include "rods.h"
#include "rsGlobal.h"   /* server global */
#include "rcGlobalExtern.h"     /* client global */
#include "rsLog.h"
#include "rodsLog.h"
#include "sockComm.h"
#include "rsMisc.h"
#include "getRodsEnv.h"
#include "rcConnect.h"
#include "initServer.h"

#define MAINTENENCE_CONFIG_FILE "rods.allow"
#define MAX_MSG_READ_RETRY	1	
#define READ_RETRY_SLEEP_TIME	1	

struct allowedUser {
    char *userName;
    char *rodsZone;
    struct allowedUser *next;
};

int agentMain (rsComm_t *rsComm);
int
setAllowedUser (struct allowedUser **allowedUserHead);
int
chkAllowedUser (struct allowedUser *allowedUserHead, char *userName,
char *domainName);

#endif	/* RODS_AGENT_H */
