/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* iFuseLib.c - The misc lib functions for the iRods/Fuse server. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include "irodsFs.h"
#include "iFuseLib.h"

static pthread_mutex_t DescLock;
pthread_t ConnManagerThr;


/* some global variables */
extern iFuseDesc_t IFuseDesc[];

extern iFuseConn_t DefConn;
extern rodsEnv MyRodsEnv;

static int ConnManagerStarted = 0;

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
fillIFuseDesc (int descInx, rcComm_t *conn, int iFd, char *objPath)
{ 
    IFuseDesc[descInx].conn = conn;
    IFuseDesc[descInx].iFd = iFd;
    if (objPath != NULL) {
        rstrcpy (IFuseDesc[descInx].objPath, objPath, MAX_NAME_LEN);
    }
    return (0);
}

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
    int status;

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

