/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* rcConnect.h - common header file for client connect
 */



#ifndef RC_CONNECT_H
#define RC_CONNECT_H

#include "rodsDef.h"
#include "rodsVersion.h"
#include "rodsError.h"
#include "rodsLog.h"
#include "stringOpr.h"
#include "rodsType.h"
#include "rodsUser.h"
#include "getRodsEnv.h"
#include "objInfo.h"
#include "dataObjInpOut.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* The client connection handle */

typedef struct {
    irodsProt_t irodsProt;
    char host[NAME_LEN];
    int sock;
    int portNum;
    int loggedIn;	/* already logged in ? */
    struct sockaddr_in  localAddr;   /* local address */
    struct sockaddr_in  remoteAddr;  /* remote address */
    userInfo_t proxyUser;
    userInfo_t clientUser;
    version_t *svrVersion;	/* the server's version */
    rError_t *rError;
    int flag;
    transStat_t transStat;
    int apiInx;
    int status;
    int windowSize;
    time_t reconnTime;
} rcComm_t;

typedef struct {
    int orphanCnt;
    int nonOrphanCnt;
} perfStat_t;

/* the server connection handle. probably should go somewhere else */
typedef struct {
    irodsProt_t irodsProt;
    int sock;
    int connectCnt;
    struct sockaddr_in  localAddr;   /* local address */
    struct sockaddr_in  remoteAddr;  /* remote address */
    userInfo_t proxyUser;
    userInfo_t clientUser;
    rodsEnv myEnv;	/* the local user */
    version_t cliVersion;      /* the client's version */
    char option[NAME_LEN];
    rError_t rError;
    portalOpr_t *portalOpr;
    int apiInx;
    int status;
    perfStat_t perfStat;
    int windowSize;
    int reconnFlag;
    int reconnSock;
    int reconnPort;
    char *reconnAddr;
    int cookie;
    time_t reconnTime;
    reconnOpr_t reconnOpr;
} rsComm_t;

void rcPipSigHandler ();

rcComm_t *
rcConnect (char *rodsHost, int rodsPort, char *userName, char *rodsZone,
int reconnFlag, rErrMsg_t *errMsg);

rcComm_t *
_rcConnect (char *rodsHost, int rodsPort,
char *proxyUserName, char *proxyRodsZone,
char *clientUserName, char *clientRodsZone, rErrMsg_t *errMsg, int connectCnt,
int reconnFlag);

int
setUserInfo (
char *proxyUserName, char *proxyRodsZone,
char *clientUserName, char *clientRodsZone,
userInfo_t *clientUser, userInfo_t *proxyUser);

int
setRhostInfo (rcComm_t *conn, char *rodsHost, int rodsPort);
int 
setSockAddr (struct sockaddr_in *remoteAddr, char *rodsHost, int rodsPort);

int setAuthInfo (char *rodsAuthScheme,
char *authStr, char *rodsServerDn,
userInfo_t *clientUser, userInfo_t *proxyUser, int flag);

int
rcDisconnect (rcComm_t *conn);
int
freeRcComm (rcComm_t *conn);
int
cleanRcComm (rcComm_t *conn);
/* XXXX putting clientLogin here for now. Should be in clientLogin.h */
int
clientLogin(rcComm_t *conn);

int
clientLoginWithPassword(rcComm_t *conn, char* password);
int
rcReconnect (rcComm_t *conn, reconnOpr_t reconnOpr);
#ifdef  __cplusplus
}
#endif

#endif	/* RC_CONNECT_H */
