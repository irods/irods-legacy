/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* The module is written by Jean-Yves Nief of CCIN2P3 */

/* univMSSDriver.h - header file for univMSSDriver.c
 */


#ifndef UNIV_MSS_DRIVER_H
#define UNIV_MSS_DRIVER_H

#define DEFAULT_ACL 750 /* to be modified in the near future */
#define UNIV_MSS_INTERF_SCRIPT "univMSSInterface.sh"

#include "rods.h"
#include "rcConnect.h"
#include "msParam.h"

int
univMSSFileUnlink (rsComm_t *rsComm, char *filename);
int
univMSSFileMkdir (rsComm_t *rsComm, char *filename, int mode);
int
univMSSFileChmod (rsComm_t *rsComm, char *filename, int mode);
int
univMSSStageToCache (rsComm_t *rsComm, fileDriverType_t cacheFileType,
int mode, int flags, char *filename,
char *cacheFilename, rodsLong_t dataSize,
keyValPair_t *condInput);
int
univMSSSyncToArch (rsComm_t *rsComm, fileDriverType_t cacheFileType,
int mode, int flags, char *filename,
char *cacheFilename,  rodsLong_t dataSize,
keyValPair_t *condInput);

#endif	/* UNIV_MSS_DRIVER_H */
