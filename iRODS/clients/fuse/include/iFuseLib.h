/*** Copyright (c), The Regents of the University of California            ***
 *** For more inforeplation please refer to files in the COPYRIGHT directory ***/
/* iFuseLib.h - Header for for iFuseLib.c */

#ifndef I_FUSE_LIB_H
#define I_FUSE_LIB_H

#include "rodsClient.h"
#include "rodsPath.h"

#define MAX_BUF_CACHE   2
#define MAX_IFUSE_DESC   1024
#define MAX_READ_CACHE_SIZE   (1024*1024)	/* 1 mb */

#define FD_FREE		0
#define FD_INUSE	1 

typedef struct BufCache {
    rodsLong_t beginOffset;
    rodsLong_t endOffset;
    void *buf;
} bufCache_t;

typedef enum { 
    NO_READ_CACHE,
    HAVE_READ_CACHE,
} readCacheState_t;

typedef struct IFuseDesc {
    rcComm_t *conn;     /* The iRods client connection */
    bufCache_t  bufCache[MAX_BUF_CACHE];
    int actCacheInx;    /* (cacheInx + 1) currently active. 0 means no cache */
    int inuseFlag;      /* 0 means not in use */
    int iFd;    /* irods client fd */
    int newFlag;
    rodsLong_t offset;
    rodsLong_t bytesWritten;
    char *objPath;
    char *localPath;
    readCacheState_t readCacheState;
} iFuseDesc_t;

#define NUM_PATH_HASH_SLOT	201
#define CACHE_EXPIRE_TIME	600	/* 10 minutes before expiration */

typedef struct PathCache {
    char* filePath;
    char* locCachePath;
    struct stat stbuf;
    uint cachedTime;
    struct PathCache *prev;
    struct PathCache *next;
    void *pathCacheQue;
    readCacheState_t readCacheState;
} pathCache_t;

typedef struct PathCacheQue {
    pathCache_t *top;
    pathCache_t *bottom;
} pathCacheQue_t;

typedef struct specialPath {
    char *path;
    int len;
} specialPath_t;

typedef struct newlyCreatedFile {
    int descInx;
    char filePath[MAX_NAME_LEN];
    struct stat stbuf;
    uint cachedTime;
} newlyCreatedFile_t;

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
fillIFuseDesc (int descInx, rcComm_t *conn, int iFd, char *objPath,
char *localPath);
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
int
initPathCache ();
int
getHashSlot (int value, int numHashSlot);
int
matchPathInPathSlot (pathCacheQue_t *pathCacheQue, char *inPath,
pathCache_t **outPathCache);
int
chkCacheExpire (pathCacheQue_t *pathCacheQue);
int
addPathToCache (char *inPath, pathCacheQue_t *pathQueArray,
struct stat *stbuf, pathCache_t **outPathCache);
int
addToCacheSlot (char *inPath, pathCacheQue_t *pathCacheQue,
struct stat *stbuf, pathCache_t **outPathCache);
int
pathSum (char *inPath);
int
matchPathInPathCache (char *inPath, pathCacheQue_t *pathQueArray,
pathCache_t **outPathCache);
int
isSpecialPath (char *inPath);
int
rmPathFromCache (char *inPath, pathCacheQue_t *pathQueArray);
int
addNewlyCreatedToCache (char *path, int descInx, int mode);
int
closeNewlyCreatedCache ();
int
closeIrodsFd (int fd);
int
getDescInxInNewlyCreatedCache (char *path, int flags);
int
fillDirStat (struct stat *stbuf, uint ctime, uint mtime, uint atime);
int
fillFileStat (struct stat *stbuf, uint mode, rodsLong_t size, uint ctime,
uint mtime, uint atime);
int
irodsOpenWithReadCache (char *path, int flags);
int
freePathCache (pathCache_t *tmpPathCache);
#ifdef  __cplusplus
}
#endif

#endif	/* I_FUSE_LIB_H */
