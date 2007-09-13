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

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct RcPortalTransferInp {
    rcComm_t *conn;
    int destFd;
    int srcFd;
    int threadNum;
    int status;
    rodsLong_t	bytesWritten;
} rcPortalTransferInp_t;
    
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

#ifdef  __cplusplus
}
#endif

#endif	/* RC_PORTAL_OPR_H */
