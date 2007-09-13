/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* initServer.h - common header file for initServer.c
 */



#ifndef INIT_SERVER_H
#define INIT_SERVER_H

#include "rods.h"
#include "sockComm.h"
#include "rsLog.h"

/* server host configuration */

#define RCAT_HOST_FILE  "server.config"
#define HOST_CONFIG_FILE  "irodsHost"
#define RE_RULES_FILE   "reRules"


/* keywords for the RCAT_HOST_FILE */
#define ICAT_HOST_KW		"icatHost"
#define SLAVE_ICAT_HOST_KW	"slaveIcatHost"


/* Keywords for the RULE ENGINE initialization */
#define RE_RULESET_KW           "reRuleSet"
#define RE_FUNCMAPSET_KW        "reFuncMapSet"
#define RE_VARIABLEMAPSET_KW    "reVariableMapSet"


/* definition for initialization state InitialState and IcatConnState */

#define INITIAL_NOT_DONE	0
#define INITIAL_DONE		1


typedef struct hostName {
    char *name;
    struct hostName *next;
} hostName_t;

/* definition for localFlag */
#define UNKNOWN_HOST_LOC -1
#define LOCAL_HOST	0
#define REMOTE_HOST	1
#define REMOTE_GW_HOST  2	/* remote gateway host */

/* definition for rcatEnabled */

#define NOT_RCAT_ENABLED	0
#define LOCAL_ICAT		1
#define LOCAL_SLAVE_ICAT	2
#define REMOTE_ICAT		3

typedef struct zoneInfo {
    char zoneName[NAME_LEN];
    void *rodsServerHost;
    struct zoneInfo *next;
} zoneInfo_t;

typedef struct rodsServerHost {
    hostName_t *hostName;
    int portNum;
    rcComm_t *conn;
    int rcatEnabled;
    int localFlag;
    zoneInfo_t myZoneInfo;
    int status;
    struct rodsServerHost *next;
} rodsServerHost_t;

int
resolveHost (rodsHostAddr_t *addr, rodsServerHost_t **rodsServerHost);
int
initServerInfo (rsComm_t *rsComm);
int
initLocalServerHost (rsComm_t *rsComm);
int
initRcatServerHostByFile (rsComm_t *rsComm);
int
queAddr (rodsServerHost_t *rodsServerHost, char *myHostName);
int
queHostName (rodsServerHost_t *rodsServerHost, char *myHostName, int topFlag);
int
queRodsServerHost (rodsServerHost_t **rodsServerHostHead,
rodsServerHost_t *myRodsServerHost);
int
getAndConnRcatHost (rsComm_t *rsComm, int rcatType, char *rcatZoneHint,
rodsServerHost_t **rodsServerHost);
char *
getConfigDir();
char *
getLogDir();
rodsServerHost_t *
mkServerHost (char *myHostAddr, int portNum);
int
getRcatHost (int rcatType, char *rcatZoneHint,  
rodsServerHost_t **rodsServerHost);
int
initZone (rsComm_t *rsComm);
int
queZone (char *zoneName, rodsServerHost_t *tmpRodsServerHost);
int
initResc (rsComm_t *rsComm);
int
procAndQueRescResult (genQueryOut_t *genQueryOut);
int
printServerHost (rodsServerHost_t *myServerHost);
int
printZoneInfo ();
int
printLocalResc ();
int
getAndConnRcatHost (rsComm_t *rsComm, int rcatType, char *rcatZoneHint,
rodsServerHost_t **rodsServerHost);
int
getAndConnRcatHostNoLogin (rsComm_t *rsComm, int rcatType, char *rcatZoneHint,
rodsServerHost_t **rodsServerHost);
int
getAndDisconnRcatHost (rsComm_t *rsComm, int rcatType, char *rcatZoneHint,
rodsServerHost_t **rodsServerHost);
int
setExecArg (char *commandArgv, char *av[]);
int
initAgent (rsComm_t *rsComm);
void cleanupAndExit (int status);
void signalExit ();
void
rsPipSigalHandler ();
int
initHostConfigByFile (rsComm_t *rsComm);
int
matchHostConfig (rodsServerHost_t *myRodsServerHost);
int
queConfigName (rodsServerHost_t *configServerHost,
rodsServerHost_t *myRodsServerHost);
int
disconnectAllSvrToSvrConn ();
int
disconnRcatHost (rsComm_t *rsComm, int rcatType, char *rcatZoneHint);
int
svrReconnect (rsComm_t *rsComm);

#endif	/* INIT_SERVER_H */
