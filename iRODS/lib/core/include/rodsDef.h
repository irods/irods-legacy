/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* rodsdef.h - common header file for rods
 */



#ifndef RODS_DEF_H
#define RODS_DEF_H

#include <stdio.h>
#include <errno.h>
#if defined(solaris_platform)
#include <fcntl.h>
#endif
#include <ctype.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>



#if defined(solaris_platform)
#include <arpa/inet.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include "dirent.h"
#endif

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/file.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/param.h>
#include <pwd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#define HEADER_TYPE_LEN 16
#define TIME_LEN        32
#define NAME_LEN        64
#define LONG_NAME_LEN	256
#define MAX_PATH_ALLOWED 1024
/* MAX_NAME_LEN actually has space for a few extra characters to match
   the way it is often used in the code. */
#define MAX_NAME_LEN   (MAX_PATH_ALLOWED+64)

/* XXXXX have to change it from 2700 to 2704 (8 bytes boundary) to get
 * packing of execMyRuleInp_t working for 64 bit address. need to reviit
 * later
 */
#define META_STR_LEN  2704

#define SHORT_STR_LEN 32  /* for dataMode, perhaps others */

#ifndef NULL
#define NULL    0
#endif

#define PTR_ARRAY_MALLOC_LEN    10	/* the len to allocate each time */

/* definition for the global variable int ProcessType */

#define CLIENT_PT	0	/* client process type */
#define SERVER_PT	1	/* server process type */
#define AGENT_PT	2	/* agent process type */
#define RE_SERVER_PT	3	/* reServer type */
#define XMSG_SERVER_PT	4	/* xmsgServer type */

/* definition for rcat type */

#define MASTER_RCAT	0
#define SLAVE_RCAT	1
#define MAX_SZ_FOR_SINGLE_BUF     (32*1024*1024)
#define MIN_SZ_FOR_PARA_TRAN     (1*1024*1024)
#define TRANS_BUF_SZ    (4*1024*1024)
#define TRANS_SZ        (40*1024*1024)

#ifdef PARA_OPR
#define MAX_NUM_TRAN_THR        4
#define MAX_NUM_CONFIG_TRAN_THR        16
#else
#define MAX_NUM_TRAN_THR        1
#define MAX_NUM_CONFIG_TRAN_THR        1
#endif
#define SZ_PER_TRAN_THR         (32*1024*1024)

/* definition for numThreads input */

#define NO_THREADING	-1
#define AUTO_THREADING   0	/* default */

#define NO_SAVE_REI  0
#define SAVE_REI  1

#define SELECT_TIMEOUT_FOR_CONN	60	/* 60 sec wait for connection */ 
/* this is the return value for the rcExecMyRule call indicating the
 * server is requesting the client to client to perform certain task */ 
#define SYS_SVR_TO_CLI_MSI_REQUEST 99999995
#define SYS_SVR_TO_CLI_COLL_STAT 99999996
#define SYS_CLI_TO_SVR_COLL_STAT_REPLY 99999997

/* definition for iRODS server to client action request from a microservice. 
 * these definitions are put in the "label" field of MsParam */  

#define CL_PUT_ACTION	"CL_PUT_ACTION"
#define CL_GET_ACTION	"CL_GET_ACTION"
#define CL_ZONE_OPR_INX	"CL_ZONE_OPR_INX"

/* the following defines the RSYNC_MODE_KW */
#define LOCAL_TO_IRODS	"LOCAL_TO_IRODS"
#define IRODS_TO_LOCAL	"IRODS_TO_LOCAL"
#define IRODS_TO_IRODS	"IRODS_TO_IRODS"

/* definition for public user */
#define PUBLIC_USER_NAME	"public"

/* definition for anonymous user */
#define ANONYMOUS_USER "anonymous"

/* protocol */
typedef enum {
    NATIVE_PROT,
    XML_PROT
} irodsProt_t;

/* myRead/myWrite type */

typedef enum {
    SOCK_TYPE,
    FILE_DESC_TYPE
} irodsDescType_t;

#define DEF_IRODS_PROT	NATIVE_PROT

/* general struct for a buffer of bytes */

typedef struct {
    int len;	/* len in bytes in buf */
    void *buf;
} bytesBuf_t;

/* The msg header for all communication between client and server */

typedef struct msgHeader {
        char type[HEADER_TYPE_LEN];
        int msgLen;     /* Length of the main msg */
	int errorLen;   /* Length of the error struct */
        int bsLen;      /* Length of optional byte stream */
	int intInfo;    /* an additional integer info, for API, it is the
			 * apiReqNum */
} msgHeader_t;

/* header length XML tag */
#define MSG_HEADER_LEN_TAG	"MsgHeaderLen"

/* msg type */
#define RODS_CONNECT_T    "RODS_CONNECT"
#define RODS_VERSION_T    "RODS_VERSION"
#define RODS_API_REQ_T    "RODS_API_REQ"
#define RODS_DISCONNECT_T    "RODS_DISCONNECT"
#define RODS_RECONNECT_T    "RODS_RECONNECT"
#define RODS_REAUTH_T     "RODS_REAUTH"
#define RODS_API_REPLY_T    "RODS_API_REPLY"

/* The strct sent with RODS_CONNECT type by client */
typedef struct startupPack {
    irodsProt_t irodsProt;
    int reconnFlag;
    int connectCnt;
    char proxyUser[NAME_LEN];
    char proxyRodsZone[NAME_LEN];
    char clientUser[NAME_LEN];
    char clientRodsZone[NAME_LEN];
    char relVersion[NAME_LEN];
    char apiVersion[NAME_LEN];
    char option[NAME_LEN];
} startupPack_t;

/* env variable for the client protocol */
#define IRODS_PROT	"irodsProt"
 
/* env variable for the startup pack */

#define SP_NEW_SOCK	"spNewSock"
#define SP_CONNECT_CNT	"spConnectCnt"
#define SP_PROTOCOL	"spProtocol"
#define SP_RECONN_FLAG	"spReconnFlag"
#define SP_PROXY_USER	"spProxyUser"
#define SP_PROXY_RODS_ZONE "spProxyRodsZone" 
#define SP_CLIENT_USER	"spClientUser"
#define SP_CLIENT_RODS_ZONE "spClientRodsZone" 
#define SP_REL_VERSION	"spRelVersion"
#define SP_API_VERSION	"spApiVersion"
#define SP_OPTION	"spOption"
#define SP_LOG_SQL	"spLogSql"
#define SP_LOG_LEVEL	"spLogLevel"
#define SERVER_BOOT_TIME "serverBootTime"

/* The strct sent with RODS_VERSION type by server */

typedef struct {
    int status;		/* if > 0, contains the reconnection port */ 
    char relVersion[NAME_LEN];
    char apiVersion[NAME_LEN];
    int reconnPort;
    char reconnAddr[LONG_NAME_LEN];
    int cookie;
} version_t;

/* struct that defines a Host Addr */

typedef struct {
    char hostAddr[LONG_NAME_LEN];
    char zoneName[NAME_LEN];
    int portNum;
    int dummyInt;	/* make it to 64 bit boundary */
} rodsHostAddr_t;

/* definition for restartState */

#define PATH_MATCHING		0x1
#define LAST_PATH_MATCHED	0x2
#define MATCHED_RESTART_COLL    0x4
#define OPR_RESUMED    		0x8

/* struct that defines restart operation */

typedef struct {
    char restartFile[MAX_NAME_LEN];
    int fd;		/* df of the opened restartFile */
    int doneCnt;	/* a count of number of files done in the coll/dir */
    char collection[MAX_NAME_LEN];      /* the coll/dir being restarted */
    char lastDonePath[MAX_NAME_LEN];	/* the last path that is done */
    int curCnt;
    int restartState;
} rodsRestart_t;  

/* definition for handler function */
#ifdef windows_platform
#ifdef __cplusplus
typedef int((*funcPtr)(...));
#else
typedef int((*funcPtr)());
#endif
#else
typedef int((*funcPtr)());
#endif

/* some platform does not support vfork */
#if defined(sgi_platform) || defined(aix_platform)
#define RODS_FORK() fork()
#else
#define RODS_FORK() vfork()
#endif

#define VAULT_PATH_POLICY	"VAULT_PATH_POLICY"	/* msParam lable */
/* definition for vault filePath scheme */
typedef enum {
    GRAFT_PATH_S,
    RANDOM_S,
} vaultPathScheme_t;

#define DEF_VAULT_PATH_SCHEME	GRAFT_PATH_S
#define DEF_ADD_USER_FLAG	1
#define DEF_TRIM_DIR_CNT	1

typedef struct {
    vaultPathScheme_t scheme;
    int addUserName;
    int trimDirCnt;	/* for GRAFT_PATH_S only. Number of directories to 
			 * trim */
} vaultPathPolicy_t;

#endif	/* RODS_DEF_H */
