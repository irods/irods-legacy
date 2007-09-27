/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* irodsReServer.h - header file for irodsReServer
 */



#ifndef IRODS_RE_SERVER_H
#define IRODS_RE_SERVER_H

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

/* definition for runMode */

#define SINGLE_PASS		0
#define IRODS_SERVER		1
#define STANDALONE_SERVER	2

#define RE_SERVER_SLEEP_TIME    120
#define RE_SERVER_EXEC_TIME     1800

/* definition for flagval flags */

#define v_FLAG  0x1

int
logFileOpen (int runMode, char *logDir);
void
daemonize (int runMode, int logFd);
int
initRsComm (rsComm_t *rsComm);
int
reServerMain (rsComm_t *rsComm);
int
reSvrSleep (rsComm_t *rsComm);
#endif	/* IRODS_RE_SERVER_H */
