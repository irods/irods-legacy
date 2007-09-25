/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/

/* bunSubDriver.h - common header bunSub for bunSub driver
 */



#ifndef BUNDLE_DRIVER_H
#define BUNDLE_DRIVER_H

#include "rods.h"
#include "rcConnect.h"
#include "objInfo.h"

typedef struct {
    bunType_t		type; 
    int         	(*bunSubCreate)();
    int         	(*bunSubOpen)();
    int         	(*bunSubRead)();
    int         	(*bunSubWrite)();
    int         	(*bunSubClose)();
    int         	(*bunSubUnlink)();
    int         	(*bunSubStat)();
    int         	(*bunSubFstat)();
    rodsLong_t  	(*bunSubLseek)();
    int         	(*bunSubRename)();
    int         	(*bunSubMkdir)();
    int         	(*bunSubRmdir)();
    int         	(*bunSubOpendir)();
    int         	(*bunSubReaddir)();
    int         	(*bunSubClosedir)();
    int         	(*bunSubTruncate)();
} bundleDriver_t;

int
bunSubIndexLookup (bunType_t myType);
int
bunSubCreate (rsComm_t *rsComm, subFile_t *subFile);
int
bunSubOpen (rsComm_t *rsComm, subFile_t *subFile);
int
bunSubRead (bunType_t myType, rsComm_t *rsComm, int fd, void *buf, int len);
int
bunSubWrite (bunType_t myType, rsComm_t *rsComm, int fd, void *buf, int len);
int
bunSubClose (bunType_t myType, rsComm_t *rsComm, int fd);
int
bunSubUnlink (rsComm_t *rsComm, subFile_t *subFile);
int
bunSubStat (rsComm_t *rsComm, subFile_t *subFile,
rodsStat_t *bunSubStatOut);
int
bunSubFstat (bunType_t myType, rsComm_t *rsComm, int fd,
rodsStat_t *bunSubStatOut);
rodsLong_t
bunSubLseek (bunType_t myType, rsComm_t *rsComm, int fd,
rodsLong_t offset, int whence);
int
bunSubRename (rsComm_t *rsComm, subFile_t *subFile, char *newFileName);
int
bunSubMkdir (rsComm_t *rsComm, subFile_t *subFile);
int
bunSubRmdir (rsComm_t *rsComm, subFile_t *subFile);
int
bunSubReaddir (bunType_t myType, rsComm_t *rsComm, int fd, 
rodsDirent_t *rodsDirent);
int
bunSubClosedir (bunType_t myType, rsComm_t *rsComm, int fd);
int
bunSubTruncate (rsComm_t *rsComm, subFile_t *subFile);
int
bunSubOpendir (rsComm_t *rsComm, subFile_t *subFile);

#endif	/* BUNDLE_DRIVER_H */
