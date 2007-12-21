/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to subStructFiles in the COPYRIGHT directory ***/

/* subStructFileDriver.h - common header subStructFile for subStructFile driver
 */



#ifndef STRUCT_FILE_DRIVER_H
#define STRUCT_FILE_DRIVER_H

#include "rods.h"
#include "rcConnect.h"
#include "objInfo.h"

typedef struct {
    structFileType_t		type; 
    int         	(*subStructFileCreate)();
    int         	(*subStructFileOpen)();
    int         	(*subStructFileRead)();
    int         	(*subStructFileWrite)();
    int         	(*subStructFileClose)();
    int         	(*subStructFileUnlink)();
    int         	(*subStructFileStat)();
    int         	(*subStructFileFstat)();
    rodsLong_t  	(*subStructFileLseek)();
    int         	(*subStructFileRename)();
    int         	(*subStructFileMkdir)();
    int         	(*subStructFileRmdir)();
    int         	(*subStructFileOpendir)();
    int         	(*subStructFileReaddir)();
    int         	(*subStructFileClosedir)();
    int         	(*subStructFileTruncate)();
} structFileDriver_t;

int
subStructFileIndexLookup (structFileType_t myType);
int
subStructFileCreate (rsComm_t *rsComm, subFile_t *subFile);
int
subStructFileOpen (rsComm_t *rsComm, subFile_t *subFile);
int
subStructFileRead (structFileType_t myType, rsComm_t *rsComm, int fd, void *buf, int len);
int
subStructFileWrite (structFileType_t myType, rsComm_t *rsComm, int fd, void *buf, int len);
int
subStructFileClose (structFileType_t myType, rsComm_t *rsComm, int fd);
int
subStructFileUnlink (rsComm_t *rsComm, subFile_t *subFile);
int
subStructFileStat (rsComm_t *rsComm, subFile_t *subFile,
rodsStat_t *subStructFileStatOut);
int
subStructFileFstat (structFileType_t myType, rsComm_t *rsComm, int fd,
rodsStat_t *subStructFileStatOut);
rodsLong_t
subStructFileLseek (structFileType_t myType, rsComm_t *rsComm, int fd,
rodsLong_t offset, int whence);
int
subStructFileRename (rsComm_t *rsComm, subFile_t *subFile, char *newFileName);
int
subStructFileMkdir (rsComm_t *rsComm, subFile_t *subFile);
int
subStructFileRmdir (rsComm_t *rsComm, subFile_t *subFile);
int
subStructFileReaddir (structFileType_t myType, rsComm_t *rsComm, int fd, 
rodsDirent_t *rodsDirent);
int
subStructFileClosedir (structFileType_t myType, rsComm_t *rsComm, int fd);
int
subStructFileTruncate (rsComm_t *rsComm, subFile_t *subFile);
int
subStructFileOpendir (rsComm_t *rsComm, subFile_t *subFile);

#endif	/* STRUCT_FILE_DRIVER_H */
