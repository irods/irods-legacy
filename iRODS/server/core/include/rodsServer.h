/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/*-------------------------------------------------------------------------
 *
 * rodsServer.h-- Header file for rodsServer.c
 *
 *
 *
 *-------------------------------------------------------------------------
 */

#ifndef RODS_SERVER_H
#define RODS_SERVER_H

#include <stdarg.h>
#ifndef windows_platform
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#endif

#include "rods.h"
#include "rsGlobal.h"	/* server global */
#include "rcGlobalExtern.h"	/* client global */
#include "rsLog.h"
#include "rodsLog.h"
#include "sockComm.h"
#include "rsMisc.h"
#include "rsIcatOpr.h"
#include "getRodsEnv.h"
#include "rcConnect.h"
#include "initServer.h"


extern char *optarg;
extern int optind, opterr, optopt;
#ifndef windows_platform
#define AGENT_EXE	"irodsAgent"	/* the agent's executable */
#else /* windows */
#define AGENT_EXE   "irodsAgent.exe"
#endif
#define MAX_EXEC_ENV	10	/* max number of env for execv */
#define MAX_SVR_SVR_CONNECT_CNT 7  /* avoid recurive connect */

#define MIN_AGENT_TIMEOUT_TIME 7200

#define MAX_ACCEPT_ERR_CNT 50

/* Managing the spawned agents */

typedef struct agentProc {
    int pid;
    char proxyUser[NAME_LEN];
    char clientUser[NAME_LEN];
    struct agentProc *next;
} agentProc_t;

int serverize (char *logDir);
int serverMain (char *logDir);
int
procChildren (agentProc_t **agentProcHead);
agentProc_t *
getAgentProcByPid (int childPid, agentProc_t **agentProcHead);

void
#if defined(linux_platform) || defined(aix_platform) || defined(solaris_platform) || defined(linux_platform)
serverExit (int sig);
#else
serverExit ();
#endif

void
usage (char *prog);

int
initServerMain (rsComm_t *svrComm);
int
initServer (rsComm_t *svrComm);
int
setRsCommFromRodsEnv (rsComm_t *rsComm);
int
spawnAgent (int newSock, startupPack_t *startupPack,
agentProc_t **agentProcHead);
int
execAgent (int newSock, startupPack_t *startupPack);
int
queAgentProc (int childPid, startupPack_t *startupPack,
agentProc_t **agentProcHead);
int
getAgentProcCnt (agentProc_t *agentProcHead);
int
recordServerProcess(rsComm_t *svrComm);
int
initAgentMain (rsComm_t *rsComm);
#endif	/* RODS_SERVER_H */
