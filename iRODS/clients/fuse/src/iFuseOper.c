/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include "irodsFs.h"
#include "iFuseOper.h"
#include "iFuseLib.h"
#include "miscUtil.h"

extern iFuseConn_t DefConn;
extern rodsEnv MyRodsEnv;
extern iFuseDesc_t IFuseDesc[];

int 
irodsGetattr (const char *path, struct stat *stbuf)
{
    int status;
    dataObjInp_t dataObjInp;
    rodsObjStat_t *rodsObjStatOut = NULL;

    rodsLog (LOG_DEBUG, "irodsGetattr: ");

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    status = parseRodsPathStr ((char *) (path + 1) , &MyRodsEnv, 
      dataObjInp.objPath);
    if (status < 0) {
	rodsLogError (LOG_ERROR, status, 
	  "irodsGetattr: parseRodsPathStr of %s error", path);
	/* use ENOTDIR for this type of error */
	return -ENOTDIR;
    }
    getIFuseConn (&DefConn, &MyRodsEnv);
    status = rcObjStat (DefConn.conn, &dataObjInp, &rodsObjStatOut);
    relIFuseConn (&DefConn);
    if (status < 0) {
	if (status != USER_FILE_DOES_NOT_EXIST) {
            rodsLogError (LOG_ERROR, status, 
	      "irodsGetattr: rcObjStat of %s error", path);
	}
	return -ENOENT;
    }

    if (rodsObjStatOut->objType == COLL_OBJ_T) {
	stbuf->st_mode = S_IFDIR | DEF_DIR_MODE;
        stbuf->st_size = DIR_SZ;
        stbuf->st_ctime = atoi (rodsObjStatOut->createTime);
        stbuf->st_mtime = atoi (rodsObjStatOut->modifyTime);
    } else {
	stbuf->st_mode = S_IFREG | DEF_FILE_MODE;
        stbuf->st_size = rodsObjStatOut->objSize;

        stbuf->st_blksize = FILE_BLOCK_SZ;
        stbuf->st_blocks = (stbuf->st_size / FILE_BLOCK_SZ) + 1;

        stbuf->st_ctime = atoi (rodsObjStatOut->createTime);
        stbuf->st_mtime = atoi (rodsObjStatOut->modifyTime);
        stbuf->st_atime = atoi (rodsObjStatOut->modifyTime);
    }

    freeRodsObjStat (rodsObjStatOut);
    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();

    return 0;
}

int 
irodsReadlink (const char *path, char *buf, size_t size)
{
    rodsLog (LOG_DEBUG, "irodsReadlink: ");
    return (0);
}

int 
irodsReaddir (const char *path, void *buf, fuse_fill_dir_t filler, 
off_t offset, struct fuse_file_info *fi)
{
    char collPath[MAX_NAME_LEN];
    collHandle_t collHandle;
    collEnt_t collEnt;
    int status;
#if 0
    rodsArguments_t rodsArgs;
    genQueryInp_t genQueryInp;
    genQueryOut_t *genQueryOut = NULL;
    int continueInx, i, status;
    char *dataName, *collName;
    sqlResult_t *tmpResult;
    int len;
#endif

    rodsLog (LOG_DEBUG, "irodsReaddir: ");

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    status = parseRodsPathStr ((char *) (path + 1), &MyRodsEnv, collPath);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status, 
	  "irodsReaddir: parseRodsPathStr of %s error", path);
        /* use ENOTDIR for this type of error */
        return -ENOTDIR;
    }

    getIFuseConn (&DefConn, &MyRodsEnv);
    status = rclOpenCollection (DefConn.conn, collPath, 0, &collHandle);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "getCollUtil: rclOpenCollection of %s error. status = %d",
          collPath, status);
        return status;
    }
    while ((status = rclReadCollection (DefConn.conn, &collHandle, &collEnt)) 
      >= 0) {
	char myDir[MAX_NAME_LEN], mySubDir[MAX_NAME_LEN];

        if (collEnt.objType == DATA_OBJ_T) {
	    filler (buf, collEnt.dataName, NULL, 0);
        } else if (collEnt.objType == COLL_OBJ_T) {
	    splitPathByKey (collEnt.collName, myDir, mySubDir, '/');
	    filler (buf, mySubDir, NULL, 0);
        }
    }
    rclCloseCollection (&collHandle);
    relIFuseConn (&DefConn);

    return (0);
#if 0
    memset (&rodsArgs, 0, sizeof (rodsArgs));

    status = queryDataObjInColl (DefConn.conn, collPath, &rodsArgs, &genQueryInp,
      &genQueryOut);

    if (status < 0 && status != CAT_NO_ROWS_FOUND) {
        rodsLogError (LOG_ERROR, status,
          "irodsReaddir: queryDataObjInColl error for %s", collPath);
	return -ENOTDIR;
    }

    while (status >= 0) {
        if ((tmpResult = getSqlResultByInx (genQueryOut, COL_DATA_NAME)) == 
	  NULL) {
            rodsLog (LOG_ERROR,
              "irodsReaddir: getSqlResultByInx for COL_DATA_NAME failed for %s",
	      collPath);
            return -ENOTDIR;
	} else {
	    dataName = tmpResult->value;
	    len = tmpResult->len;
	}

	for (i = 0;i < genQueryOut->rowCnt; i++) {
	    filler (buf, dataName, NULL, 0);
	    dataName += len;
	}
        continueInx = genQueryOut->continueInx;

        freeGenQueryOut (&genQueryOut);

        if (continueInx > 0) {
            /* More to come */
            genQueryInp.continueInx = continueInx;
	    getIFuseConn (&DefConn, &MyRodsEnv);
            status =  rcGenQuery (DefConn.conn, &genQueryInp, &genQueryOut);
	    relIFuseConn (&DefConn);
        } else {
            break;
        }
    }

    status = queryCollInColl (DefConn.conn, collPath, &rodsArgs, &genQueryInp,
      &genQueryOut);

    if (status < 0 && status != CAT_NO_ROWS_FOUND) {
        rodsLogError (LOG_ERROR, status,
          "irodsReaddir: queryCollInColl error for %s", collPath);
	return -ENOTDIR;
    }

    while (status >= 0) {
        if ((tmpResult = getSqlResultByInx (genQueryOut, COL_COLL_NAME)) == 
	  NULL) {
            rodsLog (LOG_ERROR,
              "irodsReaddir: getSqlResultByInx for COL_COLL_NAME failed for %s",
	      collPath);
            return -ENOTDIR;
        } else {
            collName = tmpResult->value;
	    len = tmpResult->len;
        }

	for (i = 0;i < genQueryOut->rowCnt; i++) {
	    char myParent[MAX_NAME_LEN], myChild[MAX_NAME_LEN];
	    splitPathByKey (collName, myParent, myChild, '/');
	    filler (buf, myChild, NULL, 0);
	    collName += len;
	}
        continueInx = genQueryOut->continueInx;

        freeGenQueryOut (&genQueryOut);

        if (continueInx > 0) {
            /* More to come */
            genQueryInp.continueInx = continueInx;
	    getIFuseConn (&DefConn, &MyRodsEnv);
            status =  rcGenQuery (DefConn.conn, &genQueryInp, &genQueryOut);
	    relIFuseConn (&DefConn);
        } else {
            break;
        }
    }

    return (0);
#endif
}

int 
irodsMknod (const char *path, mode_t mode, dev_t rdev)
{
    dataObjInp_t dataObjInp;
    dataObjCloseInp_t dataObjCloseInp;
    int status;

    rodsLog (LOG_DEBUG, "irodsMknod: ");

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    status = parseRodsPathStr ((char *) (path + 1) , &MyRodsEnv,
      dataObjInp.objPath);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status, 
	  "irodsMknod: parseRodsPathStr of %s error", path);
        /* use ENOTDIR for this type of error */
        return -ENOTDIR;
    }

    if (strlen (MyRodsEnv.rodsDefResource) > 0) {
        addKeyVal (&dataObjInp.condInput, RESC_NAME_KW, 
	  MyRodsEnv.rodsDefResource);
    }

    addKeyVal (&dataObjInp.condInput, DATA_TYPE_KW, "generic");
    dataObjInp.createMode = DEF_FILE_CREATE_MODE;
    dataObjInp.openFlags = O_WRONLY;
    dataObjInp.dataSize = -1;

    getIFuseConn (&DefConn, &MyRodsEnv);
    status = rcDataObjCreate (DefConn.conn, &dataObjInp);
    relIFuseConn (&DefConn);

    clearKeyVal (&dataObjInp.condInput);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "irodsMknod: rcDataObjCreate of %s error", path);
        return -ENOENT;
    }

    memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
    dataObjCloseInp.l1descInx = status;
    getIFuseConn (&DefConn, &MyRodsEnv);
    rcDataObjClose (DefConn.conn, &dataObjCloseInp);
    relIFuseConn (&DefConn);

    return (0);
}

int 
irodsMkdir (const char *path, mode_t mode)
{
    collInp_t collCreateInp;
    int status;

    rodsLog (LOG_DEBUG, "irodsMkdir: ");

    memset (&collCreateInp, 0, sizeof (collCreateInp));

    status = parseRodsPathStr ((char *) (path + 1) , &MyRodsEnv,
      collCreateInp.collName);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status, 
	  "irodsMkdir: parseRodsPathStr of %s error", path);
        /* use ENOTDIR for this type of error */
        return -ENOTDIR;
    }

    getIFuseConn (&DefConn, &MyRodsEnv);
    status = rcCollCreate (DefConn.conn, &collCreateInp);
    relIFuseConn (&DefConn);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "irodsMkdir: rcCollCreate of %s error", path);
        return -ENOENT;
    }

    return (0);
}

int 
irodsUnlink (const char *path)
{
    dataObjInp_t dataObjInp;
    int status;

    rodsLog (LOG_DEBUG, "irodsUnlink: ");

    memset (&dataObjInp, 0, sizeof (dataObjInp));

    status = parseRodsPathStr ((char *) (path + 1) , &MyRodsEnv,
      dataObjInp.objPath);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status, 
	  "irodsUnlink: parseRodsPathStr of %s error", path);
        /* use ENOTDIR for this type of error */
        return -ENOTDIR;
    }

    addKeyVal (&dataObjInp.condInput, FORCE_FLAG_KW, "");

    getIFuseConn (&DefConn, &MyRodsEnv);
    status = rcDataObjUnlink (DefConn.conn, &dataObjInp);
    relIFuseConn (&DefConn);

    clearKeyVal (&dataObjInp.condInput);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "irodsUnlink: rcDataObjUnlink of %s error", path);
        return -ENOENT;
    }

    return (0);
}

int 
irodsRmdir (const char *path)
{
    collInp_t collInp;
    int status;

    rodsLog (LOG_DEBUG, "irodsRmdir: ");

    memset (&collInp, 0, sizeof (collInp));

    status = parseRodsPathStr ((char *) (path + 1) , &MyRodsEnv,
      collInp.collName);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status, 
	  "irodsRmdir: parseRodsPathStr of %s error", path);
        /* use ENOTDIR for this type of error */
        return -ENOTDIR;
    }

    addKeyVal (&collInp.condInput, FORCE_FLAG_KW, "");

    getIFuseConn (&DefConn, &MyRodsEnv);
    status = rcRmColl (DefConn.conn, &collInp);
    relIFuseConn (&DefConn);

    clearKeyVal (&collInp.condInput);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "irodsRmdir: rcRmColl of %s error", path);
        return -ENOENT;
    }

    return (0);
}

int 
irodsSymlink (const char *from, const char *to)
{
    rodsLog (LOG_DEBUG, "irodsSymlink: ");
    return (0);
}

int 
irodsRename (const char *from, const char *to)
{
    dataObjCopyInp_t dataObjRenameInp;
    int status;

    rodsLog (LOG_DEBUG, "irodsRename: ");

    /* test rcDataObjRename */

    memset (&dataObjRenameInp, 0, sizeof (dataObjRenameInp));

    status = parseRodsPathStr ((char *) (from + 1) , &MyRodsEnv,
      dataObjRenameInp.srcDataObjInp.objPath);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status, 
	  "irodsRename: parseRodsPathStr of %s error", from);
        /* use ENOTDIR for this type of error */
        return -ENOTDIR;
    }

    status = parseRodsPathStr ((char *) (to + 1) , &MyRodsEnv,
      dataObjRenameInp.destDataObjInp.objPath);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status, 
	  "irodsRename: parseRodsPathStr of %s error", to);
        /* use ENOTDIR for this type of error */
        return -ENOTDIR;
    }

    dataObjRenameInp.srcDataObjInp.oprType =
      dataObjRenameInp.destDataObjInp.oprType = RENAME_DATA_OBJ;

    getIFuseConn (&DefConn, &MyRodsEnv);
    status = rcDataObjRename (DefConn.conn, &dataObjRenameInp);
    relIFuseConn (&DefConn);

    if (status < 0) {
        dataObjRenameInp.srcDataObjInp.oprType =
          dataObjRenameInp.destDataObjInp.oprType = RENAME_COLL;
        getIFuseConn (&DefConn, &MyRodsEnv);
        status = rcDataObjRename (DefConn.conn, &dataObjRenameInp);
        relIFuseConn (&DefConn);
	if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "irodsRename: rcDataObjRename of %s to %s error", from, to);
            return -ENOENT;
	}
    }

    return (0);
}

int 
irodsLink (const char *from, const char *to)
{
    rodsLog (LOG_DEBUG, "irodsLink: ");
    return (0);
}

int 
irodsChmod (const char *path, mode_t mode)
{
    rodsLog (LOG_DEBUG, "irodsChmod: ");
    return (0);
}

int 
irodsChown (const char *path, uid_t uid, gid_t gid)
{
    rodsLog (LOG_DEBUG, "irodsChown: ");
    return (0);
}

int 
irodsTruncate (const char *path, off_t size)
{
    dataObjInp_t dataObjInp;
    int status;

    rodsLog (LOG_DEBUG, "irodsTruncate: ");

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    status = parseRodsPathStr ((char *) (path + 1) , &MyRodsEnv,
      dataObjInp.objPath);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status, 
	  "irodsTruncate: parseRodsPathStr of %s error", path);
        /* use ENOTDIR for this type of error */
        return -ENOTDIR;
    }

    dataObjInp.dataSize = size;

    getIFuseConn (&DefConn, &MyRodsEnv);
    status = rcDataObjTruncate (DefConn.conn, &dataObjInp);
    relIFuseConn (&DefConn);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "irodsTruncate: rcDataObjTruncate of %s error", path);
        return -ENOENT;
    }

    return (0);
}

int 
irodsFlush (const char *path, struct fuse_file_info *fi)
{
    rodsLog (LOG_DEBUG, "irodsFlush: ");
    return (0);
}

int 
irodsUtimens (const char *path, const struct timespec ts[2])
{
    rodsLog (LOG_DEBUG, "irodsUtimens: ");
    return (0);
}

int 
irodsOpen (const char *path, struct fuse_file_info *fi)
{
    dataObjInp_t dataObjInp;
    int status;
    int fd;
    int descInx;

    rodsLog (LOG_DEBUG, "irodsOpen: ");

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    status = parseRodsPathStr ((char *) (path + 1) , &MyRodsEnv,
      dataObjInp.objPath);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status, 
	  "irodsOpen: parseRodsPathStr of %s error", path);
        /* use ENOTDIR for this type of error */
        return -ENOTDIR;
    }

    dataObjInp.openFlags = fi->flags;

    getIFuseConn (&DefConn, &MyRodsEnv);
    fd = rcDataObjOpen (DefConn.conn, &dataObjInp);
    relIFuseConn (&DefConn);

    if (fd < 0) {
        rodsLogError (LOG_ERROR, status,
          "irodsOpen: rcDataObjOpen of %s error", path);
        return -ENOENT;
    } else {
	descInx = allocIFuseDesc ();
        if (descInx < 0) {
            rodsLogError (LOG_ERROR, descInx,
              "irodsOpen: allocIFuseDesc of %s error", path);
            return -ENOENT;
	}
	fillIFuseDesc (descInx, DefConn.conn, fd, dataObjInp.objPath);
	fi->fh = descInx;

        return (0);
    }
}

int 
irodsRead (const char *path, char *buf, size_t size, off_t offset, 
struct fuse_file_info *fi)
{
    int descInx;
    int status, myError;
    dataObjReadInp_t dataObjReadInp;
    bytesBuf_t dataObjReadOutBBuf;

    rodsLog (LOG_DEBUG, "irodsRead: ");

    descInx = fi->fh;

    if (checkFuseDesc (descInx) < 0) {
	return -EBADF;
    }

    if ((status = ifuseLseek (path, descInx, offset)) < 0) {
        if ((myError = getUnixErrno (status)) > 0) {
            return (-myError);
        } else {
            return -ENOENT;
        }
    }

    dataObjReadOutBBuf.buf = buf;
    dataObjReadOutBBuf.len = size;
    dataObjReadInp.l1descInx = IFuseDesc[descInx].iFd;
    dataObjReadInp.len = size;

    getIFuseConn (&DefConn, &MyRodsEnv);
    status = rcDataObjRead (DefConn.conn, &dataObjReadInp, &dataObjReadOutBBuf);
    relIFuseConn (&DefConn);
    if (status < 0) {
        if ((myError = getUnixErrno (status)) > 0) {
	    return (-myError);
	} else {
	    return -ENOENT;
	}
    } else {
	IFuseDesc[descInx].offset += status;
        return (status);
    }
}

int 
irodsWrite (const char *path, const char *buf, size_t size, off_t offset, 
struct fuse_file_info *fi)
{
    int descInx;
    int status, myError;
    dataObjWriteInp_t dataObjWriteInp;
    bytesBuf_t dataObjWriteInpBBuf;

    rodsLog (LOG_DEBUG, "irodsWrite: ");

    descInx = fi->fh;

    if (checkFuseDesc (descInx) < 0) {
        return -EBADF;
    }

    if ((status = ifuseLseek (path, descInx, offset)) < 0) {
        if ((myError = getUnixErrno (status)) > 0) {
            return (-myError);
        } else {
            return -ENOENT;
        }
    }

    dataObjWriteInpBBuf.buf = (void *) buf;
    dataObjWriteInpBBuf.len = size;
    dataObjWriteInp.l1descInx = IFuseDesc[descInx].iFd;
    dataObjWriteInp.len = size;

    getIFuseConn (&DefConn, &MyRodsEnv);
    status = rcDataObjWrite (DefConn.conn, &dataObjWriteInp, 
      &dataObjWriteInpBBuf);
    relIFuseConn (&DefConn);
    if (status < 0) {
        if ((myError = getUnixErrno (status)) > 0) {
            return (-myError);
        } else {
            return -ENOENT;
        }
    } else if (status != size) {
        rodsLog (LOG_ERROR,
          "irodsWrite: rcDataObjWrite of %s error, wrote %d, toWrite %d",
           path, status, size);
        return -ENOENT;
    } else {
        IFuseDesc[descInx].offset += status;
        return (status);
    }
}

int 
irodsStatfs (const char *path, struct statvfs *stbuf)
{
    int status;

    rodsLog (LOG_DEBUG, "irodsStatfs: ");

    if (stbuf == NULL)
	return (0);

    
    /* just fake some number */
    status = statvfs ("/", stbuf);

    stbuf->f_blocks = 2000000000;
    stbuf->f_bfree = stbuf->f_bavail = 1000000000;
    stbuf->f_files = 200000000;
    stbuf->f_ffree = stbuf->f_favail = 100000000;
    stbuf->f_fsid = 777;
    stbuf->f_namemax = MAX_NAME_LEN;

    return (0);
}

int 
irodsRelease (const char *path, struct fuse_file_info *fi)
{
    int descInx;
    int status, myError;
    dataObjCloseInp_t dataObjCloseInp;

    rodsLog (LOG_DEBUG, "irodsRelease: ");

    descInx = fi->fh;

    if (checkFuseDesc (descInx) < 0) {
        return -EBADF;
    }

    memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
    dataObjCloseInp.l1descInx = IFuseDesc[descInx].iFd;

    getIFuseConn (&DefConn, &MyRodsEnv);
    status = rcDataObjClose (DefConn.conn, &dataObjCloseInp);
    relIFuseConn (&DefConn);

    freeIFuseDesc (descInx);

    if (status < 0) {
        if ((myError = getUnixErrno (status)) > 0) {
            return (-myError);
        } else {
            return -ENOENT;
        }
    } else {
        return (0);
    }
}

int 
irodsFsync (const char *path, int isdatasync, struct fuse_file_info *fi)
{
    rodsLog (LOG_DEBUG, "irodsFsync: ");
    return (0);
}

