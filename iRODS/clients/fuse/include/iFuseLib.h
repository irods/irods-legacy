/*** Copyright (c), The Regents of the University of California            ***
 *** For more inforeplation please refer to files in the COPYRIGHT directory ***/
/* iFuseLib.h - Header for for iFuseLib.c */

#ifndef I_FUSE_LIB_H
#define I_FUSE_LIB_H

#include "rodsClient.h"
#include "rodsPath.h"

#define MAX_BUF_CACHE   2
#define MAX_IFUSE_DESC   20

#define FD_FREE		0
#define FD_INUSE	1 

typedef struct BufCache {
    rodsLong_t beginOffset;
    rodsLong_t endOffset;
    void *buf;
} bufCache_t;

typedef struct IFuseDesc {
    rcComm_t *conn;     /* The iRods client connection */
    bufCache_t  bufCache[MAX_BUF_CACHE];
    int actCacheInx;    /* (cacheInx + 1) currently active. 0 means no cache */
    int inuseFlag;      /* 0 means not in use */
    int iFd;    /* irods client fd */
    rodsLong_t offset;
    char objPath[MAX_NAME_LEN];
} iFuseDesc_t;

#ifdef  __cplusplus
extern "C" {
#endif

int
initIFuseDesc ();
int
allocIFuseDesc ();
int
freeIFuseDesc (int descInx);
int
fillIFuseDesc (int descInx, rcComm_t *conn, int iFd, char *objPath);
int
ifuseLseek (const char *path, int descInx, off_t offset);
int
getIFuseConn (iFuseConn_t *iFuseConn, rodsEnv *MyRodsEnv);
int
relIFuseConn (iFuseConn_t *iFuseConn);
void
connManager ();
int
iFuseDescInuse ();
int
checkFuseDesc (int descInx);
#ifdef  __cplusplus
}
#endif

#endif	/* I_FUSE_LIB_H */
