/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rsGlobal.h - header file for globals in the server
 */

#ifndef RS_GLOBAL_H
#define RS_GLOBAL_H

#include "apiTable.h"	/* contains global RsApiTable */ 
#include "initServer.h"
#include "fileOpr.h"
#include "dataObjOpr.h"
#ifdef TAR_STRUCT_FILE
#include "tarSubStructFileDriver.h"
#endif

int LogFd = -1;		/* the log file descriptor */
char *CurLogfileName = NULL;        /* the path of the current logfile */

rodsServerHost_t *LocalServerHost = NULL;
rodsServerHost_t *ServerHostHead = NULL;
rodsServerHost_t *HostConfigHead = NULL;
zoneInfo_t *ZoneInfoHead = NULL;
rescGrpInfo_t *RescGrpInfo = NULL;
rescGrpInfo_t *CachedRescGrpInfo = NULL;

/* global fileDesc */

fileDesc_t FileDesc[NUM_FILE_DESC];
l1desc_t L1desc[NUM_L1_DESC];
specCollDesc_t SpecCollDesc[NUM_SPEC_COLL_DESC];

/* global Rule Engine File Initialization String */

char reRuleStr[LONG_NAME_LEN];
char reFuncMapStr[LONG_NAME_LEN];
char reVariableMapStr[LONG_NAME_LEN];

/* The stat of the Agent initialization */

int InitialState = INITIAL_NOT_DONE;
rsComm_t *ThisComm = NULL;

#ifdef RODS_CAT
int IcatConnState = INITIAL_NOT_DONE;
#endif

specCollCache_t *SpecCollCacheHead = NULL;

structFileDesc_t StructFileDesc[NUM_STRUCT_FILE_DESC];
#ifdef TAR_STRUCT_FILE
tarSubFileDesc_t TarSubFileDesc[NUM_TAR_SUB_FILE_DESC];
#endif

#endif	/* RS_GLOBAL_H */

