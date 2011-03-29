/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* rcPortalOpr.h - header file for rcPortalOpr.c
 */



#ifndef RC_PORTAL_OPR_H
#define RC_PORTAL_OPR_H

#include "rods.h"
#include "rodsError.h"
#include "objInfo.h"
#include "dataObjInpOut.h"
#ifdef RBUDP_TRANSFER
#include "QUANTAnet_rbudpBase_c.h"
#include "QUANTAnet_rbudpSender_c.h"
#include "QUANTAnet_rbudpReceiver_c.h"
#endif
#ifdef  __cplusplus
extern "C" {
#endif

#define MAX_PROGRESS_CNT	8

typedef struct RcPortalTransferInp {
    rcComm_t *conn;
    int destFd;
    int srcFd;
    int threadNum;
    int status;
    rodsLong_t	bytesWritten;
} rcPortalTransferInp_t;
    
typedef enum {
    RBUDP_CLIENT,
    RBUDP_SERVER
} rbudpProcType_t;

int
fillRcPortalTransferInp (rcComm_t *conn, rcPortalTransferInp_t *myInput, 
int destFd, int srcFd, int threadNum);
int
putFileToPortal (rcComm_t *conn, portalOprOut_t *portalOprOut, 
char *locFilePath, rodsLong_t dataSize);
int
getFileFromPortal (rcComm_t *conn, portalOprOut_t *portalOprOut, 
char *locFilePath, rodsLong_t dataSize);
void
rcPartialDataPut (rcPortalTransferInp_t *myInput);
void
rcPartialDataGet (rcPortalTransferInp_t *myInput);
int
rcvTranHeader (int sock, transferHeader_t *myHeader);

int
sendTranHeader (int sock, int oprType, int flags, rodsLong_t offset,
rodsLong_t length);
int
fillBBufWithFile (rcComm_t *conn, bytesBuf_t *myBBuf, char *locFilePath, 
rodsLong_t dataSize);
int
putFile (rcComm_t *conn, int l1descInx, char *locFilePath,
rodsLong_t dataSize);
int
getIncludeFile (rcComm_t *conn, bytesBuf_t *dataObjOutBBuf, char *locFilePath);
int
getFile (rcComm_t *conn, int l1descInx, char *locFilePath,
rodsLong_t dataSize);
#ifdef RBUDP_TRANSFER
int
putFileToPortalRbudp (portalOprOut_t *portalOprOut,                
char *locFilePath, int locFd, rodsLong_t dataSize, int veryVerbose,
int sendRate, int packetSize);
int
getFileToPortalRbudp (portalOprOut_t *portalOprOut,                
char *locFilePath, int locFd, rodsLong_t dataSize, int veryVerbose,
int packetSize);
int
initRbudpClient (rbudpBase_t *rbudpBase, portList_t *myPortList);
#endif  /* RBUDP_TRANSFER */
int
initFileRestart (rcComm_t *conn, char *fileName, rodsLong_t fileSize,
int numThr);
int
writeLfRestartFile (fileRestart_t *fileRestart);
int
clearLfRestartFile (fileRestart_t *fileRestart);
#ifdef  __cplusplus
}
#endif

#endif	/* RC_PORTAL_OPR_H */
