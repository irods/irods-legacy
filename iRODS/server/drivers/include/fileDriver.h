/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* fileDriver.h - common header file for file driver
 */



#ifndef FILE_DRIVER_H
#define FILE_DRIVER_H

#include <dirent.h>

#include "rods.h"
#include "rcConnect.h"
#include "objInfo.h"
#include "msParam.h"

typedef struct {
    fileDriverType_t	driverType; 
    int         	(*fileCreate)();
    int         	(*fileOpen)();
    int         	(*fileRead)();
    int         	(*fileWrite)();
    int         	(*fileClose)();
    int         	(*fileUnlink)();
    int         	(*fileStat)();
    int         	(*fileFstat)();
    rodsLong_t  	(*fileLseek)();
    int         	(*fileFsync)();
    int         	(*fileMkdir)();
    int         	(*fileChmod)();
    int         	(*fileRmdir)();
    int         	(*fileOpendir)();
    int         	(*fileClosedir)();
    int         	(*fileReaddir)();
    int         	(*fileStage)();
    int         	(*fileRename)();
    rodsLong_t  	(*fileGetFsFreeSpace)();
    int         	(*fileTruncate)();
    int			(*fileStageToCache)();
    int			(*fileSyncToArch)();
} fileDriver_t;

int
fileIndexLookup (fileDriverType_t myType);
int
fileCreate (fileDriverType_t myType, rsComm_t *rsComm, char *fileName,
int mode, rodsLong_t mySize);
int
fileOpen (fileDriverType_t myType, rsComm_t *rsComm, char *fileName, int flags, int mode);
int
fileRead (fileDriverType_t myType, rsComm_t *rsComm, int fd, void *buf,
int len);
int
fileWrite (fileDriverType_t myType, rsComm_t *rsComm, int fd, void *buf,
int len);
int
fileClose (fileDriverType_t myType, rsComm_t *rsComm, int fd);
int
fileUnlink (fileDriverType_t myType, rsComm_t *rsComm, char *filename);
int
fileStat (fileDriverType_t myType, rsComm_t *rsComm, char *filename,
struct stat *statbuf);
int
fileFstat (fileDriverType_t myType, rsComm_t *rsComm, int fd,
struct stat *statbuf);
rodsLong_t
fileLseek (fileDriverType_t myType, rsComm_t *rsComm, int fd,
rodsLong_t offset, int whence);
int
fileMkdir (fileDriverType_t myType, rsComm_t *rsComm, char *filename, int mode);
int
fileChmod (fileDriverType_t myType, rsComm_t *rsComm, char *filename, int mode);
int
fileRmdir (fileDriverType_t myType, rsComm_t *rsComm, char *filename);
int
fileOpendir (fileDriverType_t myType, rsComm_t *rsComm, char *filename,
void **outDirPtr);
int
fileClosedir (fileDriverType_t myType, rsComm_t *rsComm, void *dirPtr);
int
fileReaddir (fileDriverType_t myType, rsComm_t *rsComm, void *dirPtr,
struct dirent **direntPtr);
int
fileStage (fileDriverType_t myType, rsComm_t *rsComm, char *path, int flag);
int
fileRename (fileDriverType_t myType, rsComm_t *rsComm, char *oldFileName,
char *newFileName);
rodsLong_t
fileGetFsFreeSpace (fileDriverType_t myType, rsComm_t *rsComm,
char *path, int flag);
int
fileFsync (fileDriverType_t myType, rsComm_t *rsComm, int fd);
int
fileTruncate (fileDriverType_t myType, rsComm_t *rsComm, char *path,
rodsLong_t dataSize);
int
fileStageToCache (fileDriverType_t myType, rsComm_t *rsComm,
fileDriverType_t cacheFileType, int mode, int flag,
char *filename, char *cacheFilename, keyValPair_t *condInput);
int
fileSyncToArch (fileDriverType_t myType, rsComm_t *rsComm, 
fileDriverType_t cacheFileType, int mode, int flag,
char *filename, char *cacheFilename, keyValPair_t *condInput);
#endif	/* FILE_DRIVER_H */
