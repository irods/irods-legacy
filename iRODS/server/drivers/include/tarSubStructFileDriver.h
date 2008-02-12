/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to structFiles in the COPYRIGHT directory ***/

/* tarSubStructFileDriver.h - header structFile for tarSubStructFileDriver.c
 */



#ifndef TAR_STRUCT_FILE_DRIVER_H_H
#define TAR_STRUCT_FILE_DRIVER_H_H

#include "rods.h"
#include "structFileDriver.h"

int
tarSubStructFileCreate (rsComm_t *rsComm, subFile_t *subFile);
int 
tarSubStructFileOpen (rsComm_t *rsComm, subFile_t *subFile); 
int 
tarSubStructFileRead (rsComm_t *rsComm, int fd, void *buf, int len);
int
tarSubStructFileWrite (rsComm_t *rsComm, int fd, void *buf, int len);
int 
tarSubStructFileClose (rsComm_t *rsComm, int fd);
int 
tarSubStructFileUnlink (rsComm_t *rsComm, subFile_t *subFile); 
int
tarSubStructFileStat (rsComm_t *rsComm, subFile_t *subFile,
rodsStat_t **subStructFileStatOut); 
int
tarSubStructFileFstat (rsComm_t *rsComm, int fd, 
rodsStat_t **subStructFileStatOut);
rodsLong_t
tarSubStructFileLseek (rsComm_t *rsComm, int fd, rodsLong_t offset, int whence);
int 
tarSubStructFileRename (rsComm_t *rsComm, subFile_t *subFile, char *newFileName);
int
tarSubStructFileMkdir (rsComm_t *rsComm, subFile_t *subFile);
int
tarSubStructFileRmdir (rsComm_t *rsComm, subFile_t *subFile);
int
tarSubStructFileOpendir (rsComm_t *rsComm, subFile_t *subFile);
int
tarSubStructFileReaddir (rsComm_t *rsComm, int fd, rodsDirent_t **rodsDirent);
int
tarSubStructFileClosedir (rsComm_t *rsComm, int fd);
int
tarSubStructFileTruncate (rsComm_t *rsComm, subFile_t *subFile);
int
rsTarStructFileOpen (rsComm_t *rsComm, specColl_t *specColl);
int
stageTarStructFile (int structFileInx);
int
mkTarCacheDir (int structFileInx);
int
irodsTarOpen (char *pathname, int oflags, int mode);
int
irodsTarClose (int fd);
int
irodsTarRead (int fd, char *buf, int len);
int
irodsTarWrite (int fd, char *buf, int len);
int
verifyStructFileDesc (int structFileInx, char *tarPathname,
specColl_t **specColl);
int
encodeIrodsTarfd (int upperInt, int lowerInt);
int
decodeIrodsTarfd (int inpInt, int *upperInt, int *lowerInt);
int
extractTarFile (int structFileInx);
int
matchStructFileDesc (specColl_t *specColl);
int
getSubStructFilePhyPath (char *phyPath, specColl_t *specColl,
char *subFilePath);
int
tarStructFileSync (rsComm_t *rsComm, structFileOprInp_t *structFileOprInp);
int
syncCacheDirToTarfile (int structFileInx);
#endif	/* TAR_STRUCT_FILE_DRIVER_H_H */
