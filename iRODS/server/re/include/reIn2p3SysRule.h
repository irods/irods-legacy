/*** Copyright (c), CCIN2P3            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* reIn2p3SysRule.h - header file for reIn2p3SysRule */


#ifndef RE_IN2P3_SYS_RULE_H
#define RE_IN2P3_SYS_RULE_H

#include "rods.h"
#include "rsGlobalExtern.h"   /* server global */
#include "rcGlobalExtern.h"   /* client global */
#include "reGlobalsExtern.h"
#include "rsLog.h"
#include "rodsLog.h"
#include "sockComm.h"
#include "rsMisc.h"
#include "getRodsEnv.h"
#include "rcConnect.h"
#include "initServer.h"

#define NFIELDS 4 /* number of fields in HostControlAccess file: <user> <group> <IP address> <subnet mask> */
#define MAXLEN 100
#define MAXSTR 30
#define MAXLIST 40 /* max number of entries in the access list tab. */
#define IPV4 4 /* IP address: 4 bytes. */

#define HOST_ACCESS_CONTROL_FILE	"HostAccessControl"

int checkIPaddress(char *IP, unsigned char IPcomp[IPV4]);
int checkHostAccessControl(char *username, char *hostclient, char *groupsname);
int msiCheckHostAccessControl (ruleExecInfo_t *rei);

#endif	/* RE_IN2P3_SYS_RULE_H */
