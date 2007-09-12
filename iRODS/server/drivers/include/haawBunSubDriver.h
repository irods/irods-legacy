/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bundles in the COPYRIGHT directory ***/

/* haawBunSubDriver.h - header bundle for haawBunSubDriver.c
 */



#ifndef HAAW_BUNSUB_DRIVER_H_H
#define HAAW_BUNSUB_DRIVER_H_H

#include "rods.h"
#include "bundleDriver.h"

int
haawBunSubCreate (rsComm_t *rsComm, subFile_t *subFile);
int 
haawBunSubOpen (rsComm_t *rsComm, subFile_t *subFile); 
int 
haawBunSubRead (rsComm_t *rsComm, int fd, void *buf, int len);
int
haawBunSubWrite (rsComm_t *rsComm, int fd, void *buf, int len);
int 
haawBunSubClose (rsComm_t *rsComm, int fd);
int 
haawBunSubUnlink (rsComm_t *rsComm, subFile_t *subFile); 
int
haawBunSubStat (rsComm_t *rsComm, subFile_t *subFile,
rodsStat_t *bunSubStatOut); 
int
haawBunSubFstat (rsComm_t *rsComm, int fd, rodsStat_t *bunSubStatOut);
rodsLong_t
haawBunSubLseek (rsComm_t *rsComm, int fd, rodsLong_t offset, int whence);
int 
haawBunSubRename (rsComm_t *rsComm, subFile_t *subFile, char *newFileName);
int
haawBunSubMkdir (rsComm_t *rsComm, subFile_t *subFile);
int
haawBunSubRmdir (rsComm_t *rsComm, subFile_t *subFile);
int
haawBunSubOpendir (rsComm_t *rsComm, subFile_t *subFile);
int
haawBunSubReaddir (rsComm_t *rsComm, int fd, rodsDirent_t *rodsDirent);
int
haawBunSubClosedir (rsComm_t *rsComm, int fd);
int
haawBunSubTruncate (rsComm_t *rsComm, subFile_t *subFile);

#endif	/* HAAW_BUNSUB_DRIVER_H_H */
