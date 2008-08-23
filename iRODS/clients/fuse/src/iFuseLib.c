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

static pthread_mutex_t DescLock;
pthread_t ConnManagerThr;


/* some global variables */
extern iFuseDesc_t IFuseDesc[];

extern iFuseConn_t DefConn;
extern rodsEnv MyRodsEnv;

static int ConnManagerStarted = 0;

pathCacheQue_t NonExistPathArray[NUM_PATH_HASH_SLOT];
pathCacheQue_t PathArray[NUM_PATH_HASH_SLOT];

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

#if 0
int
matchPathInNonExistPathCache (char *inPath, pathCacheQue_t **myque,
pathCache_t **outPathCache)
{
    int mysum, myslot;
    int status;

    if (inPath == NULL) {
        rodsLog (LOG_ERROR,
          "matchPathInNonExistPathCache: input inPath is NULL");
	*myque = NULL;
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    mysum = pathSum (inPath);
    myslot = getHashSlot (mysum, NUM_PATH_HASH_SLOT);
    *myque = &NonExistPathArray[myslot];

    chkCacheExpire (*myque);
    status = matchPathInPathSlot (*myque, inPath, outPathCache);

    return status;
}
#endif

int
matchPathInPathCache (char *inPath, pathCacheQue_t *pathQueArray,
pathCacheQue_t **myque, pathCache_t **outPathCache)
{
    int mysum, myslot;
    int status;

    if (inPath == NULL) {
        rodsLog (LOG_ERROR,
          "matchPathInPathCache: input inPath is NULL");
        *myque = NULL;
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    mysum = pathSum (inPath);
    myslot = getHashSlot (mysum, NUM_PATH_HASH_SLOT);
    *myque = &pathQueArray[myslot];

    chkCacheExpire (*myque);
    status = matchPathInPathSlot (*myque, inPath, outPathCache);

    return status;
}

int
addPathToCache (char *inPath, pathCacheQue_t *pathQueArray,
struct stat *stbuf)
{
    pathCacheQue_t *pathCacheQue;
    int mysum, myslot;
    int status;

    /* XXXX if (isSpecialPath ((char *) inPath) != 1) return 0; */
    mysum = pathSum (inPath);
    myslot = getHashSlot (mysum, NUM_PATH_HASH_SLOT);
    pathCacheQue = &pathQueArray[myslot];
    status = addToCacheSlot (inPath, pathCacheQue, stbuf);

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
	    free (tmpPathCache);
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
	    free (tmpPathCache);
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
struct stat *stbuf)
{
    pathCache_t *tmpPathCache;
    
    if (pathCacheQue == NULL || inPath == NULL) {
        rodsLog (LOG_ERROR,
          "addToCacheSlot: input pathCacheQue or inPath is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    tmpPathCache = malloc (sizeof (pathCache_t));
    bzero (tmpPathCache, sizeof (pathCache_t));
    rstrcpy (tmpPathCache->filePath, inPath, MAX_NAME_LEN);
    tmpPathCache->cachedTime = time (0);
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
    return (0);
}

int
fillIFuseDesc (int descInx, rcComm_t *conn, int iFd, char *objPath,
char *localPath)
{ 
    IFuseDesc[descInx].conn = conn;
    IFuseDesc[descInx].iFd = iFd;
    if (objPath != NULL) {
        rstrcpy (IFuseDesc[descInx].objPath, objPath, MAX_NAME_LEN);
    }
    if (localPath != NULL) {
        rstrcpy (IFuseDesc[descInx].localPath, localPath, MAX_NAME_LEN);
    }
 return (0);
}

/* need to call getIFuseConn before calling ifuseLseek */ 
int
ifuseLseek (const char *path, int descInx, off_t offset)
{
    int status;

    if (IFuseDesc[descInx].offset != offset) {
        fileLseekInp_t dataObjLseekInp;
        fileLseekOut_t *dataObjLseekOut = NULL;

        dataObjLseekInp.fileInx = IFuseDesc[descInx].iFd;
        dataObjLseekInp.offset = offset;
        dataObjLseekInp.whence = SEEK_SET;

        status = rcDataObjLseek (DefConn.conn, &dataObjLseekInp, 
	  &dataObjLseekOut);

        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "ifuseLseek: rcDataObjLseek of %s error", path);
            return status;
        } else {
	    if (dataObjLseekOut != NULL)
                free (dataObjLseekOut);
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

