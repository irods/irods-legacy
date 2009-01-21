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
#ifndef windows_platform
#define HOST_CONFIG_FILE  "irodsHost"
#define RE_RULES_FILE   "reRules"
#else
#define HOST_CONFIG_FILE  "irodsHost.txt"
#define RE_RULES_FILE   "reRules.txt"
#endif


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
#define REMOTE_ZONE_HOST  3     /* host in remote zone */

/* definition for rcatEnabled */

#define NOT_RCAT_ENABLED	0
#define LOCAL_ICAT		1
#define LOCAL_SLAVE_ICAT	2
#define REMOTE_ICAT		3

/* definition for runMode */

#define SINGLE_PASS             0
#define IRODS_SERVER            1
#define STANDALONE_SERVER       2

typedef struct rodsServerHost {
    hostName_t *hostName;
    rcComm_t *conn;
    int rcatEnabled;
    int localFlag;
    int status;
    void *zoneInfo;
    struct rodsServerHost *next;
} rodsServerHost_t;

typedef struct zoneInfo {
    char zoneName[NAME_LEN];
    int portNum;
    rodsServerHost_t *masterServerHost;
    rodsServerHost_t *slaveServerHost;
    struct zoneInfo *next;
} zoneInfo_t;

/* definitions for Server ID information */
#define MAX_FED_RSIDS  5
#define LOCAL_ZONE_SID_KW       "LocalZoneSID"
#define REMOTE_ZONE_SID_KW      "RemoteZoneSID"
#define SID_KEY_KW              "SIDKey"


int
resolveHost (rodsHostAddr_t *addr, rodsServerHost_t **rodsServerHost);
int
resolveHostByDataObjInfo (dataObjInfo_t *dataObjInfo,
rodsServerHost_t **rodsServerHost);
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
mkServerHost (char *myHostAddr, char *zoneName);
int
getRcatHost (int rcatType, char *rcatZoneHint,  
rodsServerHost_t **rodsServerHost);
int
initZone (rsComm_t *rsComm);
int
queZone (char *zoneName, int portNum, rodsServerHost_t *masterServerHost,
rodsServerHost_t *slaveServerHost);
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
int
initRsComm (rsComm_t *rsComm);
void
daemonize (int runMode, int logFd);
int
logFileOpen (int runMode, char *logDir, char *logFileName);
int
initRsCommWithStartupPack (rsComm_t *rsComm, startupPack_t *startupPack);
int
getLocalZoneInfo (zoneInfo_t **outZoneInfo);
int
getZoneInfo (char *rcatZoneHint, zoneInfo_t **myZoneInfo);
int
getAndConnRemoteZone (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
rodsServerHost_t **rodsServerHost, char *remotZoneOpr);
int
getRemoteZoneHost (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
rodsServerHost_t **rodsServerHost, char *remotZoneOpr);
int
convZoneSockError (int inStatus);
#endif	/* INIT_SERVER_H */
