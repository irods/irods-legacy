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
#include "irodsGuiProgressCallback.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum {
    PROCESSING_STATE,	 /* the process is not sending nor receiving */
    RECEIVING_STATE,
    SENDING_STATE,
    CONN_WAIT_STATE
} procState_t;

typedef struct reconnMsg {
    int status;
    int cookie;
    procState_t procState;
    int flag;
} reconnMsg_t;

typedef enum {
    PROC_LOG_NOT_DONE,  /* the proc logging in log/proc not done yet */
    PROC_LOG_DONE       /* the proc logging in log/proc is done */
} procLogFlag_t;

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
    int reconnectedSock;
    time_t reconnTime;
#ifndef windows_platform
    pthread_t reconnThr;
    pthread_mutex_t lock;
    pthread_cond_t cond;
#endif
    procState_t agentState;
    procState_t clientState;
    procState_t reconnThrState;
    operProgress_t operProgress;
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
    char clientAddr[NAME_LEN]; 	/* str version of remoteAddr */
    userInfo_t proxyUser;
    userInfo_t clientUser;
    rodsEnv myEnv;	/* the local user */
    version_t cliVersion;      /* the client's version */
    char option[NAME_LEN];
    procLogFlag_t procLogFlag;
    rError_t rError;
    portalOpr_t *portalOpr;
    int apiInx;
    int status;
    perfStat_t perfStat;
    int windowSize;
    int reconnFlag;
    int reconnSock;
    int reconnPort;
    int reconnectedSock;
    char *reconnAddr;
    int cookie;

#ifndef windows_platform
    pthread_t reconnThr;
    pthread_mutex_t lock;
    pthread_cond_t cond;
#endif
    procState_t agentState;	
    procState_t clientState;
    procState_t reconnThrState;
    int gsiRequest;
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

char *
getSessionSignitureClientside();

int
clientLoginWithPassword(rcComm_t *conn, char* password);
rcComm_t *
rcConnectXmsg (rodsEnv *myRodsEnv, rErrMsg_t *errMsg);
void
cliReconnManager (rcComm_t *conn);
int
cliChkReconnAtSendStart (rcComm_t *conn);
int
cliChkReconnAtSendEnd (rcComm_t *conn);
int
cliChkReconnAtReadStart (rcComm_t *conn);
int
cliChkReconnAtReadEnd (rcComm_t *conn);

#ifdef  __cplusplus
}
#endif

#endif	/* RC_CONNECT_H */
