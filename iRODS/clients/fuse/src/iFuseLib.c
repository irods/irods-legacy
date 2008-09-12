/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* iFuseLib.c - The misc lib functions for the iRods/Fuse server. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>
#include "irodsFs.h"
#include "iFuseLib.h"
#include "iFuseOper.h"

static pthread_mutex_t DescLock;
pthread_t ConnManagerThr;

char FuseCacheDir[MAX_NAME_LEN];

/* some global variables */
extern iFuseDesc_t IFuseDesc[];

extern iFuseConn_t DefConn;
extern rodsEnv MyRodsEnv;

static int ConnManagerStarted = 0;

pathCacheQue_t NonExistPathArray[NUM_PATH_HASH_SLOT];
pathCacheQue_t PathArray[NUM_PATH_HASH_SLOT];
newlyCreatedFile_t NewlyCreatedFile;
char *ReadCacheDir = NULL;

static specialPath_t SpecialPath[] = {
    {"/tls", 4},
    {"/i686", 5},
    {"/sse2", 5},
    {"/lib", 4},
    {"/librt.so.1", 11},
    {"/libacl.so.1", 12},
    {"/libselinux.so.1", 16},
    {"/libc.so.6", 10},
    {"/libpthread.so.0", 16},
    {"/libattr.so.1", 13},
};

static int NumSpecialPath = sizeof (SpecialPath) / sizeof (specialPath_t);

int
initPathCache ()
{
    bzero (NonExistPathArray, sizeof (NonExistPathArray));
    bzero (PathArray, sizeof (NonExistPathArray));
    bzero (&NewlyCreatedFile, sizeof (NewlyCreatedFile));
    return (0);
}

int
isSpecialPath (char *inPath)
{
    int len;
    char *endPtr;
    int i;

    if (inPath == NULL) {
        rodsLog (LOG_ERROR,
          "isSpecialPath: input inPath is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    len = strlen (inPath);
    endPtr = inPath + len;
    for (i = 0; i < NumSpecialPath; i++) {
	if (len < SpecialPath[i].len) continue;
	if (strcmp (SpecialPath[i].path, endPtr - SpecialPath[i].len) == 0)
	    return (1);
    }
    return 0;
}

int
matchPathInPathCache (char *inPath, pathCacheQue_t *pathQueArray,
pathCache_t **outPathCache)
{
    int mysum, myslot;
    int status;
    pathCacheQue_t *myque;

    if (inPath == NULL) {
        rodsLog (LOG_ERROR,
          "matchPathInPathCache: input inPath is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    mysum = pathSum (inPath);
    myslot = getHashSlot (mysum, NUM_PATH_HASH_SLOT);
    myque = &pathQueArray[myslot];

    chkCacheExpire (myque);
    status = matchPathInPathSlot (myque, inPath, outPathCache);

    return status;
}

int
addPathToCache (char *inPath, pathCacheQue_t *pathQueArray,
struct stat *stbuf, pathCache_t **outPathCache)
{
    pathCacheQue_t *pathCacheQue;
    int mysum, myslot;
    int status;

    /* XXXX if (isSpecialPath ((char *) inPath) != 1) return 0; */
    mysum = pathSum (inPath);
    myslot = getHashSlot (mysum, NUM_PATH_HASH_SLOT);
    pathCacheQue = &pathQueArray[myslot];
    status = addToCacheSlot (inPath, pathCacheQue, stbuf, outPathCache);

    return (status);
}

int
rmPathFromCache (char *inPath, pathCacheQue_t *pathQueArray)
{
    pathCacheQue_t *pathCacheQue;
    int mysum, myslot;
    pathCache_t *tmpPathCache;

    /* XXXX if (isSpecialPath ((char *) inPath) != 1) return 0; */
    mysum = pathSum (inPath);
    myslot = getHashSlot (mysum, NUM_PATH_HASH_SLOT);
    pathCacheQue = &pathQueArray[myslot];

    tmpPathCache = pathCacheQue->top;
    while (tmpPathCache != NULL) {
        if (strcmp (tmpPathCache->filePath, inPath) == 0) {
            if (tmpPathCache->prev == NULL) {
                /* top */
                pathCacheQue->top = tmpPathCache->next;
            } else {
                tmpPathCache->prev->next = tmpPathCache->next;
            }
            if (tmpPathCache->next == NULL) {
		/* bottom */
		pathCacheQue->bottom = tmpPathCache->prev;
	    } else {
		tmpPathCache->next->prev = tmpPathCache->prev;
	    }
	    freePathCache (tmpPathCache);
	    return 1;
	}
        tmpPathCache = tmpPathCache->next;
    }
    return 0;
}

int
getHashSlot (int value, int numHashSlot)
{
    int mySlot = value % numHashSlot;

    return (mySlot);
}

int
matchPathInPathSlot (pathCacheQue_t *pathCacheQue, char *inPath, 
pathCache_t **outPathCache)
{
    pathCache_t *tmpPathCache;

    *outPathCache = NULL;
    if (pathCacheQue == NULL) {
        rodsLog (LOG_ERROR,
          "matchPathInPathSlot: input pathCacheQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    tmpPathCache = pathCacheQue->top;
    while (tmpPathCache != NULL) {
	if (strcmp (tmpPathCache->filePath, inPath) == 0) {
	    *outPathCache = tmpPathCache;
	    return 1;
	}
	tmpPathCache = tmpPathCache->next;
    }
    return (0);
}

int
chkCacheExpire (pathCacheQue_t *pathCacheQue)
{
    pathCache_t *tmpPathCache;

    uint curTime = time (0);
    if (pathCacheQue == NULL) {
        rodsLog (LOG_ERROR,
          "chkCacheExpire: input pathCacheQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    tmpPathCache = pathCacheQue->top;
    while (tmpPathCache != NULL) {
	if (curTime >= tmpPathCache->cachedTime + CACHE_EXPIRE_TIME) {
	    /* cache expired */
	    pathCacheQue->top = tmpPathCache->next;
	    freePathCache (tmpPathCache);
	    tmpPathCache = pathCacheQue->top;
            if (tmpPathCache != NULL) {
                tmpPathCache->prev = NULL;
            } else {
		pathCacheQue->bottom = NULL;
		return (0);
	    }
	} else {
	    /* not expired */
	    return (0);
	}
    }
    return (0);
}
	     
int
addToCacheSlot (char *inPath, pathCacheQue_t *pathCacheQue, 
struct stat *stbuf, pathCache_t **outPathCache)
{
    pathCache_t *tmpPathCache;
    
    if (pathCacheQue == NULL || inPath == NULL) {
        rodsLog (LOG_ERROR,
          "addToCacheSlot: input pathCacheQue or inPath is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    tmpPathCache = malloc (sizeof (pathCache_t));
    if (outPathCache != NULL) *outPathCache = tmpPathCache;
    bzero (tmpPathCache, sizeof (pathCache_t));
    tmpPathCache->filePath = strdup (inPath);
    tmpPathCache->cachedTime = time (0);
    tmpPathCache->pathCacheQue = pathCacheQue;
    if (stbuf != NULL) {
	tmpPathCache->stbuf = *stbuf;
    }
    /* queue it to the bottom */
    if (pathCacheQue->top == NULL) {
	pathCacheQue->top = pathCacheQue->bottom = tmpPathCache;
    } else {
	pathCacheQue->bottom->next = tmpPathCache;
	tmpPathCache->prev = pathCacheQue->bottom;
	pathCacheQue->bottom = tmpPathCache;
    }
    return (0);
}

int
pathSum (char *inPath)
{
    int len, i;
    int mysum = 0;

    if (inPath == NULL) {
        rodsLog (LOG_ERROR,
          "pathSum: input inPath is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    len = strlen (inPath);

    for (i = 0; i < len; i++) {
	mysum += inPath[i];
    }

    return mysum; 
}

int
initIFuseDesc ()
{
    pthread_mutex_init (&DescLock, NULL);
    memset (IFuseDesc, 0, sizeof (iFuseDesc_t) * MAX_IFUSE_DESC);
    return (0);
}

int
allocIFuseDesc ()
{
    int i;

    pthread_mutex_lock (&DescLock);
    for (i = 3; i < MAX_IFUSE_DESC; i++) {
        if (IFuseDesc[i].inuseFlag <= FD_FREE) {
            IFuseDesc[i].inuseFlag = FD_INUSE;
            pthread_mutex_unlock (&DescLock);
            return (i);
        };
    }
    pthread_mutex_unlock (&DescLock);

    rodsLog (LOG_ERROR, 
      "allocIFuseDesc: Out of iFuseDesc");

    return (SYS_OUT_OF_FILE_DESC);
}

int
iFuseDescInuse ()
{
    int i;

    for (i = 3; i < MAX_IFUSE_DESC; i++) {
	if (IFuseDesc[i].inuseFlag == FD_INUSE)
	    return 1;
    }
    return (0);
} 

int
freePathCache (pathCache_t *tmpPathCache)
{
    if (tmpPathCache == NULL) return 0;
    if (tmpPathCache->filePath != NULL) free (tmpPathCache->filePath);
    if (tmpPathCache->locCacheState == HAVE_READ_CACHE &&
      tmpPathCache->locCachePath != NULL) {
	unlink (tmpPathCache->locCachePath);
        free (tmpPathCache->locCachePath);
    }
    free (tmpPathCache);
    return (0);
}

int
freeIFuseDesc (int descInx)
{
    int i;

    if (descInx < 3 || descInx >= MAX_IFUSE_DESC) {
        rodsLog (LOG_ERROR,
         "freeIFuseDesc: descInx %d out of range", descInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }

    pthread_mutex_lock (&DescLock);
    for (i = 0; i < MAX_BUF_CACHE; i++) {
        if (IFuseDesc[descInx].bufCache[i].buf != NULL) {
	    free (IFuseDesc[descInx].bufCache[i].buf);
	}
    }
    if (IFuseDesc[descInx].objPath != NULL)
	free (IFuseDesc[descInx].objPath);

    if (IFuseDesc[descInx].localPath != NULL)
	free (IFuseDesc[descInx].localPath);

    memset (&IFuseDesc[descInx], 0, sizeof (iFuseDesc_t));
    pthread_mutex_unlock (&DescLock);

    return (0);
}

int 
checkFuseDesc (int descInx)
{
    if (descInx < 3 || descInx >= MAX_IFUSE_DESC) {
        rodsLog (LOG_ERROR,
         "checkFuseDesc: descInx %d out of range", descInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }

    if (IFuseDesc[descInx].inuseFlag != FD_INUSE) {
        rodsLog (LOG_ERROR,
         "checkFuseDesc: descInx %d is not inuse", descInx);
        return (SYS_BAD_FILE_DESCRIPTOR);
    }
    if (IFuseDesc[descInx].iFd <= 0) {
        rodsLog (LOG_ERROR,
         "checkFuseDesc:  iFd %d of descInx %d <= 0", 
	  IFuseDesc[descInx].iFd, descInx);
        return (SYS_BAD_FILE_DESCRIPTOR);
    }

    return (0);
}

int
fillIFuseDesc (int descInx, rcComm_t *conn, int iFd, char *objPath,
char *localPath)
{ 
    IFuseDesc[descInx].conn = conn;
    IFuseDesc[descInx].iFd = iFd;
    if (objPath != NULL) {
        /* rstrcpy (IFuseDesc[descInx].objPath, objPath, MAX_NAME_LEN); */
        IFuseDesc[descInx].objPath = strdup (objPath);
    }
    if (localPath != NULL) {
        /* rstrcpy (IFuseDesc[descInx].localPath, localPath, MAX_NAME_LEN); */
        IFuseDesc[descInx].localPath = strdup (localPath);
    }
    return (0);
}

/* need to call getIFuseConn before calling ifuseClose */
int
ifuseClose (char *path, int descInx)
{
    int status = 0;
    int savedStatus = 0;

    if (IFuseDesc[descInx].locCacheState == NO_FILE_CACHE) {
        dataObjCloseInp_t dataObjCloseInp;

        memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
        dataObjCloseInp.l1descInx = IFuseDesc[descInx].iFd;

        status = rcDataObjClose (DefConn.conn, &dataObjCloseInp);
    } else {	/* cached */
        if (IFuseDesc[descInx].bytesWritten > 0) {
            int goodStat = 0;
            if (IFuseDesc[descInx].newFlag > 0 || 
	      IFuseDesc[descInx].locCacheState == HAVE_NEWLY_CREATED_CACHE) {
                pathCache_t *tmpPathCache;
                /* newly created. Just update the size */
                if (matchPathInPathCache ((char *) path, PathArray,
                 &tmpPathCache) == 1 && tmpPathCache->locCachePath != NULL) {
                    status = updatePathCacheStat (tmpPathCache);
                    if (status >= 0) goodStat = 1;
		    status = ifusePut (path, tmpPathCache->locCachePath,
		      IFuseDesc[descInx].createMode, 
		      tmpPathCache->stbuf.st_size);
		    if (status < 0) {
		        rodsLog (LOG_ERROR,
                          "ifuseClose: ifusePut of %s error, status = %d",
                           path, status);
			savedStatus = -EBADF;
		    }
                } else {
                    /* should not be here. but cache may be removed that we
		     * may have to deal with it */
                    rodsLog (LOG_ERROR,
                      "ifuseClose: IFuseDesc indicated a newly created cache, but does not exist for %s",
                       path);
		    savedStatus = -EBADF;
	        }
            } else {
	        /* should not be here */
                rodsLog (LOG_ERROR,
                  "ifuseClose: bytesWritten to not newly created cache for %s",
                   path);
		savedStatus = -EBADF;
	    }

            if (goodStat == 0) rmPathFromCache ((char *) path, PathArray);
        }
	status = close (IFuseDesc[descInx].iFd);
	if (status < 0) {
	    status = (errno ? (-1 * errno) : -1);
	} else {
	    status = savedStatus;
	}
    }
    return (status);
}

int
ifusePut (char *path, char *locCachePath, int mode, rodsLong_t srcSize)
{
    dataObjInp_t dataObjInp;
    int status;

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    status = parseRodsPathStr ((char *) (path + 1) , &MyRodsEnv,
      dataObjInp.objPath);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "ifusePut: parseRodsPathStr of %s error", path);
        /* use ENOTDIR for this type of error */
        return -ENOTDIR;
    }
    dataObjInp.dataSize = srcSize;
    dataObjInp.createMode = mode;
    dataObjInp.openFlags = O_RDWR;
    addKeyVal (&dataObjInp.condInput, FORCE_FLAG_KW, "");
    addKeyVal (&dataObjInp.condInput, DATA_TYPE_KW, "generic");
    if (strlen (MyRodsEnv.rodsDefResource) > 0) {
        addKeyVal (&dataObjInp.condInput, DEST_RESC_NAME_KW,
          MyRodsEnv.rodsDefResource);
    }

    status = rcDataObjPut (DefConn.conn, &dataObjInp, locCachePath);
    return (status);
}

/* need to call getIFuseConn before calling ifuseWrite */
int
ifuseWrite (char *path, int descInx, char *buf, size_t size,
off_t offset)
{
    int status, myError;
    char irodsPath[MAX_NAME_LEN];
    dataObjWriteInp_t dataObjWriteInp;
    bytesBuf_t dataObjWriteInpBBuf;


    if (IFuseDesc[descInx].locCacheState == NO_FILE_CACHE) {
        dataObjWriteInpBBuf.buf = (void *) buf;
        dataObjWriteInpBBuf.len = size;
        dataObjWriteInp.l1descInx = IFuseDesc[descInx].iFd;
        dataObjWriteInp.len = size;

        status = rcDataObjWrite (DefConn.conn, &dataObjWriteInp,
          &dataObjWriteInpBBuf);
        if (status < 0) {
            if ((myError = getUnixErrno (status)) > 0) {
                return (-myError);
            } else {
                return -ENOENT;
            }
        } else if (status != size) {
            rodsLog (LOG_ERROR,
              "ifuseWrite: rcDataObjWrite of %s error, wrote %d, toWrite %d",
               path, status, size);
            return -ENOENT;
        }
        IFuseDesc[descInx].offset += status;
    } else {
        status = write (IFuseDesc[descInx].iFd, buf, size);

        if (status < 0) return (errno ? (-1 * errno) : -1);
        IFuseDesc[descInx].offset += status;
	if (IFuseDesc[descInx].offset >= MAX_NEWLY_CREATED_CACHE_SIZE) {
	    int irodsFd; 
	    int status1;
	    struct stat stbuf;
	    char *mybuf;
	    rodsLong_t myoffset;

	    /* need to write it to iRods */
	    irodsFd = dataObjCreateByFusePath (path, 
	      IFuseDesc[descInx].createMode, irodsPath);
	    if (irodsFd < 0) {
                rodsLog (LOG_ERROR,
                  "ifuseWrite: dataObjCreateByFusePath of %s error, stat=%d",
                 path, irodsFd);
		close (IFuseDesc[descInx].iFd);
		rmPathFromCache ((char *) path, PathArray);
                return -ENOENT;
	    }
	    status1 = fstat (IFuseDesc[descInx].iFd, &stbuf);
            if (status1 < 0) {
		close (IFuseDesc[descInx].iFd);
		rmPathFromCache ((char *) path, PathArray);
		return (errno ? (-1 * errno) : -1);
	    }
	    mybuf = malloc (stbuf.st_size);
	    status1 = read (IFuseDesc[descInx].iFd, mybuf, stbuf.st_size);
            if (status1 < 0) {
		close (IFuseDesc[descInx].iFd);
                rmPathFromCache ((char *) path, PathArray);
                return (errno ? (-1 * errno) : -1);
            }
            dataObjWriteInpBBuf.buf = (void *) mybuf;
            dataObjWriteInpBBuf.len = stbuf.st_size;
            dataObjWriteInp.l1descInx = irodsFd;
            dataObjWriteInp.len = stbuf.st_size;

            status1 = rcDataObjWrite (DefConn.conn, &dataObjWriteInp,
              &dataObjWriteInpBBuf);
	    free (mybuf);
            close (IFuseDesc[descInx].iFd);
            rmPathFromCache ((char *) path, PathArray);

            if (status1 < 0) {
                if ((myError = getUnixErrno (status1)) > 0) {
                    status1 = (-myError);
                } else {
                    status1 = -ENOENT;
                }
		IFuseDesc[descInx].iFd = 0;
                return (status1);
	    } else {
		IFuseDesc[descInx].iFd = irodsFd;
		IFuseDesc[descInx].locCacheState = NO_FILE_CACHE;
	    }
	    /* one last thing - seek to the right offset */
            myoffset = IFuseDesc[descInx].offset;
	    IFuseDesc[descInx].offset = 0;
            if ((status1 = ifuseLseek ((char *) path, descInx, myoffset)) 
	      < 0) {
                if ((myError = getUnixErrno (status1)) > 0) {
                    return (-myError);
                } else {
                    return -ENOENT;
                }
	    }
	}
    }
    IFuseDesc[descInx].bytesWritten += status;

    return status;
}

/* need to call getIFuseConn before calling ifuseRead */
int
ifuseRead (char *path, int descInx, char *buf, size_t size, 
off_t offset)
{
    int status;

    if (IFuseDesc[descInx].locCacheState == NO_FILE_CACHE) {
        dataObjReadInp_t dataObjReadInp;
	bytesBuf_t dataObjReadOutBBuf;
	int myError;

        dataObjReadOutBBuf.buf = buf;
        dataObjReadOutBBuf.len = size;
        dataObjReadInp.l1descInx = IFuseDesc[descInx].iFd;
        dataObjReadInp.len = size;

        status = rcDataObjRead (DefConn.conn, &dataObjReadInp, 
	  &dataObjReadOutBBuf);
        if (status < 0) {
            if ((myError = getUnixErrno (status)) > 0) {
                return (-myError);
            } else {
                return -ENOENT;
            }
	}
    } else {
	status = read (IFuseDesc[descInx].iFd, buf, size);

	if (status < 0) return (errno ? (-1 * errno) : -1);
    }
    IFuseDesc[descInx].offset += status;

    return status;
}

/* need to call getIFuseConn before calling ifuseLseek */ 
int
ifuseLseek (char *path, int descInx, off_t offset)
{
    int status;

    if (IFuseDesc[descInx].offset != offset) {
	if (IFuseDesc[descInx].locCacheState == NO_FILE_CACHE) {
            fileLseekInp_t dataObjLseekInp;
            fileLseekOut_t *dataObjLseekOut = NULL;

            dataObjLseekInp.fileInx = IFuseDesc[descInx].iFd;
            dataObjLseekInp.offset = offset;
            dataObjLseekInp.whence = SEEK_SET;

            status = rcDataObjLseek (DefConn.conn, &dataObjLseekInp, 
	      &dataObjLseekOut);
            if (dataObjLseekOut != NULL) free (dataObjLseekOut);
	} else {
	    rodsLong_t lstatus;
	    lstatus = lseek (IFuseDesc[descInx].iFd, offset, SEEK_SET);
	    if (lstatus >= 0) {
		status = 0;
	    } else {
		status = lstatus;
	    }
	}

        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "ifuseLseek: lseek of %s error", path);
            return status;
        } else {
            IFuseDesc[descInx].offset = offset;
        }

    }
    return (0);
}

int
getIFuseConn (iFuseConn_t *iFuseConn, rodsEnv *myRodsEnv)
{
    int status;
    rErrMsg_t errMsg;

    pthread_mutex_lock (&iFuseConn->lock);

    if (DefConn.conn == NULL) {
        DefConn.conn = rcConnect (myRodsEnv->rodsHost, myRodsEnv->rodsPort,
          myRodsEnv->rodsUserName, myRodsEnv->rodsZone, 1, &errMsg);

        if (DefConn.conn == NULL) {
            rodsLogError (LOG_ERROR, errMsg.status, 
	      "getIFuseConn: rcConnect failure %s", errMsg.msg);
	    if (errMsg.status < 0) {
		return (errMsg.status);
	    } else {
                return (-1);
	    }
        }

        status = clientLogin (DefConn.conn);
        if (status != 0) {
            rcDisconnect (DefConn.conn);
            return (status);
        }
    }


    if (ConnManagerStarted < 5 && ++ConnManagerStarted == 5) {
	/* don't do it the first time */
        status = pthread_create  (&ConnManagerThr, pthread_attr_default,
                  (void *(*)(void *)) connManager,
                  (void *) NULL);

        if (status < 0) {
            rodsLog (LOG_ERROR, "pthread_create failure, status = %d", status);
            rcDisconnect (DefConn.conn);
	}
    }

    DefConn.actTime = time (NULL);

    return 0;
}

int
relIFuseConn (iFuseConn_t *iFuseConn)
{
    pthread_mutex_unlock (&iFuseConn->lock);
    DefConn.actTime = time (NULL);
    return 0;
}

void
connManager ()
{
    time_t curTime;

    while (1) {
        pthread_mutex_lock (&DefConn.lock);
        if (&DefConn.conn != NULL) {
            curTime = time (NULL);
            if (curTime - DefConn.actTime > IFUSE_CONN_TIMEOUT &&
	      iFuseDescInuse () == 0) {
                rcDisconnect (DefConn.conn);
                DefConn.conn = NULL;
            }
        }
        pthread_mutex_unlock (&DefConn.lock);
        rodsSleep (CONN_MANAGER_SLEEP_TIME, 0);
    }
}

int
addNewlyCreatedToCache (char *path, int descInx, int mode)
{
    uint cachedTime = time (0);

    closeNewlyCreatedCache ();
    rstrcpy (NewlyCreatedFile.filePath, path, MAX_NAME_LEN);
    NewlyCreatedFile.descInx = descInx;
    NewlyCreatedFile.cachedTime = cachedTime;
    fillFileStat (&NewlyCreatedFile.stbuf, mode, 0, cachedTime, cachedTime,
      cachedTime);

    IFuseDesc[descInx].newFlag = 1;
    addPathToCache (path, PathArray, &NewlyCreatedFile.stbuf, NULL);
    return (0);
}
    

int
closeIrodsFd (int fd)
{
    dataObjCloseInp_t dataObjCloseInp;

    dataObjCloseInp.l1descInx = fd;
    getIFuseConn (&DefConn, &MyRodsEnv);
    rcDataObjClose (DefConn.conn, &dataObjCloseInp);
    relIFuseConn (&DefConn);

    return (0);
}

int
closeNewlyCreatedCache ()
{
    if (strlen (NewlyCreatedFile.filePath) > 0) {
	struct fuse_file_info fi;
	int descInx = NewlyCreatedFile.descInx;
	
	fi.fh = descInx;
	irodsRelease (NewlyCreatedFile.filePath, &fi);
	bzero (&NewlyCreatedFile, sizeof (NewlyCreatedFile));
    }
    return (0);
}

int
getDescInxInNewlyCreatedCache (char *path, int flags)
{
    if (strcmp (path, NewlyCreatedFile.filePath) == 0) {
	if ((flags & O_RDWR) == 0 && (flags & O_WRONLY) == 0) {
	    closeNewlyCreatedCache ();
	    return -1;
	} else if (checkFuseDesc (NewlyCreatedFile.descInx) >= 0) {
	    int descInx = NewlyCreatedFile.descInx;
	    bzero (&NewlyCreatedFile, sizeof (NewlyCreatedFile));
	    return descInx;
	} else {
	    bzero (&NewlyCreatedFile, sizeof (NewlyCreatedFile));
	    return -1;
	}
    } else {
	closeNewlyCreatedCache ();
	return -1;
    }
}

int
fillFileStat (struct stat *stbuf, uint mode, rodsLong_t size, uint ctime,
uint mtime, uint atime)
{
    if (mode >= 0100)
        stbuf->st_mode = S_IFREG | mode;
    else
        stbuf->st_mode = S_IFREG | DEF_FILE_MODE;
    stbuf->st_size = size;

    stbuf->st_blksize = FILE_BLOCK_SZ;
    stbuf->st_blocks = (stbuf->st_size / FILE_BLOCK_SZ) + 1;

    stbuf->st_ctime = ctime;
    stbuf->st_mtime = mtime;
    stbuf->st_atime = atime;
    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();

    return 0;
}

int
fillDirStat (struct stat *stbuf, uint ctime, uint mtime, uint atime)
{
    stbuf->st_mode = S_IFDIR | DEF_DIR_MODE;
    stbuf->st_size = DIR_SZ;

    stbuf->st_ctime = ctime;
    stbuf->st_mtime = mtime;
    stbuf->st_atime = atime;
    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();

    return 0;
}

int 
updatePathCacheStat (pathCache_t *tmpPathCache)
{
    int status;

    if (tmpPathCache->locCacheState != NO_FILE_CACHE &&
      tmpPathCache->locCachePath != NULL) {
	struct stat stbuf;
	status = stat (tmpPathCache->locCachePath, &stbuf);
	if (status < 0) {
	    return (errno ? (-1 * errno) : -1);
	} else {
	    /* update the size */
	    tmpPathCache->stbuf.st_size = stbuf.st_size; 
	    return 0; 
	}
    } else {
	return 0;
    }
}

/* need to call getIFuseConn before calling irodsMknodWithCache */
int
irodsMknodWithCache (char *path, mode_t mode, char *cachePath)
{
    int status;
    int fd;

    if ((status = getFileCachePath (path, cachePath)) < 0)
        return status;

    fd = creat (cachePath, mode);
    if (fd < 0) {
        rodsLog (LOG_ERROR,
          "irodsMknodWithCache: local cache creat error for %s, errno = %d",
          cachePath, errno);
        return(errno ? (-1 * errno) : -1);
    } else {
        return fd;
    }
}

/* need to call getIFuseConn before calling irodsOpenWithReadCache */
int
irodsOpenWithReadCache (char *path, int flags)
{
    pathCache_t *tmpPathCache = NULL;
    struct stat stbuf;
    int status;
    dataObjInp_t dataObjInp;
    char cachePath[MAX_NAME_LEN];
    int fd, descInx;

    /* do only O_RDONLY (0) */
    if ((flags & (O_WRONLY | O_RDWR)) != 0) return -1;

    if (_irodsGetattr (path, &stbuf, &tmpPathCache) < 0 ||
      tmpPathCache == NULL) return -1;

    /* too big to cache */
    if (stbuf.st_size > MAX_READ_CACHE_SIZE) return -1;	

    if (tmpPathCache->locCachePath == NULL) {

        rodsLog (LOG_DEBUG, "irodsOpenWithReadCache: caching %s", path);

        memset (&dataObjInp, 0, sizeof (dataObjInp));
        if ((status = getFileCachePath (path, cachePath)) < 0) 
	    return status;
        /* get the file to local cache */
        status = parseRodsPathStr ((char *) (path + 1) , &MyRodsEnv,
          dataObjInp.objPath);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "irodsOpenWithReadCache: parseRodsPathStr of %s error", path);
            /* use ENOTDIR for this type of error */
            return -ENOTDIR;
        }
        dataObjInp.openFlags = flags;
        dataObjInp.dataSize = stbuf.st_size;

        status = rcDataObjGet (DefConn.conn, &dataObjInp, cachePath);

        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "irodsOpenWithReadCache: rcDataObjGet of %s error", 
	      dataObjInp.objPath);

	    return status; 
	}
        tmpPathCache->locCachePath = strdup (cachePath);
        tmpPathCache->locCacheState = HAVE_READ_CACHE;
    } else {
        rodsLog (LOG_DEBUG, "irodsOpenWithReadCache: read cache match for %s",
          path);
    }

    fd = open (tmpPathCache->locCachePath, flags);
    if (fd < 0) {
        rodsLog (LOG_ERROR,
          "irodsOpenWithReadCache: local cache open error for %s, errno = %d",
          tmpPathCache->locCachePath, errno);
	return(errno ? (-1 * errno) : -1);
    }

    descInx = allocIFuseDesc ();
    if (descInx < 0) {
        rodsLogError (LOG_ERROR, descInx,
          "irodsOpenWithReadCache: allocIFuseDesc of %s error", path);
        return -ENOENT;
    }
    fillIFuseDesc (descInx, DefConn.conn, fd, dataObjInp.objPath,
      (char *) path);
    IFuseDesc[descInx].locCacheState = HAVE_READ_CACHE;

    return descInx;
}

int
getFileCachePath (char *inPath, char *cacehPath)
{
    char myDir[MAX_NAME_LEN], myFile[MAX_NAME_LEN];
    struct stat statbuf;

    if (inPath == NULL || cacehPath == NULL) {
        rodsLog (LOG_ERROR,
          "getFileCachePath: input inPath or cacehPath is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    splitPathByKey (inPath, myDir, myFile, '/');

    while (1)
    {
        snprintf (cacehPath, MAX_NAME_LEN, "%s/%s.%d", FuseCacheDir,
          myFile, (int) random ());
        if (stat (cacehPath, &statbuf) < 0) break;
    }
    return 0;
}

int
setAndMkFileCacheDir ()
{
    char *tmpStr, *tmpDir;
    struct passwd *myPasswd;
    int status;

    myPasswd = getpwuid(getuid());

    if ((tmpStr = getenv (FuseCacheDir)) != NULL && strlen (tmpStr) > 0) {
	tmpDir = tmpStr;
    } else {
	tmpDir = FUSE_CACHE_DIR;
    }

    snprintf (FuseCacheDir, MAX_NAME_LEN, "%s/%s", tmpDir,
      myPasswd->pw_name);

    if ((status = mkdirR ("/", FuseCacheDir, DEF_DIR_MODE)) < 0) {
        rodsLog (LOG_ERROR,
          "setAndMkFileCacheDir: mkdirR of %s error. status = %d");
    }

    return (status);

}

int
dataObjCreateByFusePath (char *path, int mode, char *outIrodsPath)
{
    dataObjInp_t dataObjInp;
    int status;

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    status = parseRodsPathStr ((char *) (path + 1) , &MyRodsEnv,
      dataObjInp.objPath);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "dataObjCreateByFusePath: parseRodsPathStr of %s error", path);
        /* use ENOTDIR for this type of error */
        return -ENOTDIR;
    }

    if (strlen (MyRodsEnv.rodsDefResource) > 0) {
        addKeyVal (&dataObjInp.condInput, RESC_NAME_KW,
          MyRodsEnv.rodsDefResource);
    }

    addKeyVal (&dataObjInp.condInput, DATA_TYPE_KW, "generic");
    /* dataObjInp.createMode = DEF_FILE_CREATE_MODE; */
    dataObjInp.createMode = mode;
    dataObjInp.openFlags = O_RDWR;
    dataObjInp.dataSize = -1;

    status = rcDataObjCreate (DefConn.conn, &dataObjInp);
    clearKeyVal (&dataObjInp.condInput);
    if (status >= 0 && outIrodsPath != NULL)
	rstrcpy (outIrodsPath, dataObjInp.objPath, MAX_NAME_LEN);

    return status;
}
