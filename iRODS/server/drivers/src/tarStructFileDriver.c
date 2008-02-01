/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to subStructFiles in the COPYRIGHT directory ***/
/* tarSubStructFileDriver.c - Module of the tar structFile driver.
 */

#include <libtar.h>

#ifdef HAVE_LIBZ
# include <zlib.h>
#endif

#include <compat.h>

#include "tarSubStructFileDriver.h"
#include "rsGlobalExtern.h"
#include "modColl.h"
#include "apiHeaderAll.h"
#include "objMetaOpr.h"

tartype_t irodstype = { (openfunc_t) irodsTarOpen, (closefunc_t) irodsTarClose,
        (readfunc_t) irodsTarRead, (writefunc_t) irodsTarWrite
};

int
tarSubStructFileCreate (rsComm_t *rsComm, subFile_t *subFile)
{
    int structFileInx;
    specColl_t *specColl;
    int subInx;
    int rescTypeInx;
    int status;
    fileCreateInp_t fileCreateInp;

    specColl = subFile->specColl;
    structFileInx = rsTarStructFileOpen (rsComm, specColl);

    if (structFileInx < 0) {
        rodsLog (LOG_NOTICE,
         "tarSubStructFileCreate: rsTarStructFileOpen error for %s, stat=%d",
          specColl->objPath, structFileInx);
        return (structFileInx);
    }

    subInx = allocTarSubFileDesc ();

    if (subInx < 0) return subInx;

    TarSubFileDesc[subInx].structFileInx = structFileInx;

    memset (&fileCreateInp, 0, sizeof (fileCreateInp));
    status = getSubStructFilePhyPath (fileCreateInp.fileName, specColl,
      subFile->subFilePath);
    if (status < 0) return status;

    rescTypeInx = StructFileDesc[structFileInx].rescInfo->rescTypeInx;
    fileCreateInp.fileType = RescTypeDef[rescTypeInx].driverType;
    rstrcpy (fileCreateInp.addr.hostAddr,
    StructFileDesc[structFileInx].rescInfo->rescLoc, NAME_LEN);
    fileCreateInp.mode = subFile->mode;
    fileCreateInp.flags = subFile->flags;

    status = rsFileCreate (rsComm, &fileCreateInp);

    if (status < 0) {
       rodsLog (LOG_ERROR,
          "specCollCreate: rsFileCreate for %s error, status = %d",
          fileCreateInp.fileName, status);
        return status;
    } else {
        TarSubFileDesc[subInx].fd = status;
        StructFileDesc[structFileInx].openCnt++;
        return (subInx);
    }
}

int 
tarSubStructFileOpen (rsComm_t *rsComm, subFile_t *subFile)
{
    int structFileInx;
    specColl_t *specColl;
    int subInx;
    int rescTypeInx;
    int status;
    fileOpenInp_t fileOpenInp;

    specColl = subFile->specColl;
    structFileInx = rsTarStructFileOpen (rsComm, specColl);

    if (structFileInx < 0) {
        rodsLog (LOG_NOTICE,
         "tarSubStructFileOpen: rsTarStructFileOpen error for %s, status = %d",
          specColl->objPath, structFileInx);
        return (structFileInx);
    }

    subInx = allocTarSubFileDesc ();

    if (subInx < 0) return subInx;

    TarSubFileDesc[subInx].structFileInx = structFileInx;

    memset (&fileOpenInp, 0, sizeof (fileOpenInp));
    status = getSubStructFilePhyPath (fileOpenInp.fileName, specColl,
      subFile->subFilePath);
    if (status < 0) return status;

    rescTypeInx = StructFileDesc[structFileInx].rescInfo->rescTypeInx;
    fileOpenInp.fileType = RescTypeDef[rescTypeInx].driverType;
    rstrcpy (fileOpenInp.addr.hostAddr,
    StructFileDesc[structFileInx].rescInfo->rescLoc, NAME_LEN);
    fileOpenInp.mode = subFile->mode;
    fileOpenInp.flags = subFile->flags;

    status = rsFileOpen (rsComm, &fileOpenInp);

    if (status < 0) {
       rodsLog (LOG_ERROR,
          "specCollOpen: rsFileOpen for %s error, status = %d",
          fileOpenInp.fileName, status);
        return status;
    } else {
        TarSubFileDesc[subInx].fd = status;
        StructFileDesc[structFileInx].openCnt++;
        return (subInx);
    }
}

int 
tarSubStructFileRead (rsComm_t *rsComm, int subInx, void *buf, int len)
{
    fileReadInp_t fileReadInp;
    int status;
    bytesBuf_t fileReadOutBBuf;

    if (subInx < 1 || subInx >= NUM_TAR_SUB_FILE_DESC ||
      TarSubFileDesc[subInx].inuseFlag == 0) {
        rodsLog (LOG_ERROR,
         "tarSubStructFileRead: subInx %d out of range", subInx);
        return (SYS_STRUCT_FILE_DESC_ERR);
    }

    memset (&fileReadInp, 0, sizeof (fileReadInp));
    memset (&fileReadOutBBuf, 0, sizeof (fileReadOutBBuf));
    fileReadInp.fileInx = TarSubFileDesc[subInx].fd;
    fileReadInp.len = len;
    fileReadOutBBuf.buf = buf;
    status = rsFileRead (rsComm, &fileReadInp, &fileReadOutBBuf);

    return (status);
}

int
tarSubStructFileWrite (rsComm_t *rsComm, int subInx, void *buf, int len)
{
    fileWriteInp_t fileWriteInp;
    int status;
    bytesBuf_t fileWriteOutBBuf;

    if (subInx < 1 || subInx >= NUM_TAR_SUB_FILE_DESC ||
      TarSubFileDesc[subInx].inuseFlag == 0) {
        rodsLog (LOG_ERROR,
         "tarSubStructFileWrite: subInx %d out of range", subInx);
        return (SYS_STRUCT_FILE_DESC_ERR);
    }

    memset (&fileWriteInp, 0, sizeof (fileWriteInp));
    memset (&fileWriteOutBBuf, 0, sizeof (fileWriteOutBBuf));
    fileWriteInp.fileInx = TarSubFileDesc[subInx].fd;
    fileWriteInp.len = fileWriteOutBBuf.len = len;
    fileWriteOutBBuf.buf = buf;
    status = rsFileWrite (rsComm, &fileWriteInp, &fileWriteOutBBuf);

    if (status > 0) {
	specColl_t *specColl;
	int status1;
	int structFileInx = TarSubFileDesc[subInx].structFileInx;
	/* cache has been written */
	specColl = StructFileDesc[structFileInx].specColl;
	if (specColl->cacheDirty == 0) {
	    specColl->cacheDirty = 1;    
	    status1 = modTarCollInfo2 (rsComm, specColl);
	    if (status1 < 0) return status1;
        }
    }
    return (status);
}

int
tarSubStructFileClose (rsComm_t *rsComm, int subInx)
{
    fileCloseInp_t fileCloseInp;
    int structFileInx;
    int status;

    if (subInx < 1 || subInx >= NUM_TAR_SUB_FILE_DESC ||
      TarSubFileDesc[subInx].inuseFlag == 0) {
        rodsLog (LOG_ERROR,
         "tarSubStructFileClose: subInx %d out of range",
          subInx);
        return (SYS_STRUCT_FILE_DESC_ERR);
    }

    fileCloseInp.fileInx = TarSubFileDesc[subInx].fd;
    status = rsFileClose (rsComm, &fileCloseInp);

    structFileInx = TarSubFileDesc[subInx].structFileInx;
    StructFileDesc[structFileInx].openCnt++;
    freeTarSubFileDesc (subInx);

    return (status);
}

int
tarSubStructFileUnlink (rsComm_t *rsComm, subFile_t *subFile)
{
    int structFileInx;
    specColl_t *specColl;
    int rescTypeInx;
    int status;
    fileUnlinkInp_t fileUnlinkInp;

    specColl = subFile->specColl;
    structFileInx = rsTarStructFileOpen (rsComm, specColl);

    if (structFileInx < 0) {
        rodsLog (LOG_NOTICE,
         "tarSubStructFileCreate: rsTarStructFileOpen error for %s, stat=%d",
          specColl->objPath, structFileInx);
        return (structFileInx);
    }

    memset (&fileUnlinkInp, 0, sizeof (fileUnlinkInp));
    status = getSubStructFilePhyPath (fileUnlinkInp.fileName, specColl,
      subFile->subFilePath);
    if (status < 0) return status;

    rescTypeInx = StructFileDesc[structFileInx].rescInfo->rescTypeInx;

    fileUnlinkInp.fileType = RescTypeDef[rescTypeInx].driverType;
    rstrcpy (fileUnlinkInp.addr.hostAddr,
    StructFileDesc[structFileInx].rescInfo->rescLoc, NAME_LEN);

    status = rsFileUnlink (rsComm, &fileUnlinkInp);
    if (status >= 0) {
        specColl_t *specColl;
        int status1;
        /* cache has been written */
        specColl = StructFileDesc[structFileInx].specColl;
        if (specColl->cacheDirty == 0) {
            specColl->cacheDirty = 1;
            status1 = modTarCollInfo2 (rsComm, specColl);
            if (status1 < 0) return status1;
        }
    }

    return (0);
}

int
tarSubStructFileStat (rsComm_t *rsComm, subFile_t *subFile,
rodsStat_t **subStructFileStatOut)
{
    specColl_t *specColl;
    int structFileInx;
    int rescTypeInx;
    int status; 
    fileStatInp_t fileStatInp;

    specColl = subFile->specColl;
    structFileInx = rsTarStructFileOpen (rsComm, specColl);

    if (structFileInx < 0) {
        rodsLog (LOG_NOTICE,
         "tarSubStructFileStat: rsTarStructFileOpen error for %s, status = %d",
	  specColl->objPath, structFileInx);
        return (structFileInx);
    }

    memset (&fileStatInp, 0, sizeof (fileStatInp));

    status = getSubStructFilePhyPath (fileStatInp.fileName, specColl, 
      subFile->subFilePath);
    if (status < 0) return status;

    rescTypeInx = StructFileDesc[structFileInx].rescInfo->rescTypeInx;
    fileStatInp.fileType = RescTypeDef[rescTypeInx].driverType;
    rstrcpy (fileStatInp.addr.hostAddr,  
    StructFileDesc[structFileInx].rescInfo->rescLoc, NAME_LEN);

    status = rsFileStat (rsComm, &fileStatInp, subStructFileStatOut);

    return (status);
}

int
tarSubStructFileFstat (rsComm_t *rsComm, int subInx, 
rodsStat_t **subStructFileStatOut)
{
    fileFstatInp_t fileFstatInp;
    int status;

    if (subInx < 1 || subInx >= NUM_TAR_SUB_FILE_DESC ||
      TarSubFileDesc[subInx].inuseFlag == 0) {
        rodsLog (LOG_ERROR,
         "tarSubStructFileFstat: subInx %d out of range", subInx);
        return (SYS_STRUCT_FILE_DESC_ERR);
    }

    memset (&fileFstatInp, 0, sizeof (fileFstatInp));
    fileFstatInp.fileInx = TarSubFileDesc[subInx].fd;
    status = rsFileFstat (rsComm, &fileFstatInp, subStructFileStatOut);

    return (status);
}

rodsLong_t
tarSubStructFileLseek (rsComm_t *rsComm, int subInx, rodsLong_t offset, 
int whence)
{
    fileLseekInp_t fileLseekInp;
    int status;
    fileLseekOut_t *fileLseekOut = NULL;

    if (subInx < 1 || subInx >= NUM_TAR_SUB_FILE_DESC ||
      TarSubFileDesc[subInx].inuseFlag == 0) {
        rodsLog (LOG_ERROR,
         "tarSubStructFileLseek: subInx %d out of range", subInx);
        return (SYS_STRUCT_FILE_DESC_ERR);
    }

    memset (&fileLseekInp, 0, sizeof (fileLseekInp));
    fileLseekInp.fileInx = TarSubFileDesc[subInx].fd;
    fileLseekInp.offset = offset;
    fileLseekInp.whence = whence;
    status = rsFileLseek (rsComm, &fileLseekInp, &fileLseekOut);

    if (status < 0) {
        return ((rodsLong_t) status);
    } else {
        rodsLong_t offset = fileLseekOut->offset;
        free (fileLseekOut);
        return (offset);
    }
}

int
tarSubStructFileRename (rsComm_t *rsComm, subFile_t *subFile, 
char *newFileName)
{
    int structFileInx;
    specColl_t *specColl;
    int rescTypeInx;
    int status;
    fileRenameInp_t fileRenameInp;

    specColl = subFile->specColl;
    structFileInx = rsTarStructFileOpen (rsComm, specColl);

    if (structFileInx < 0) {
        rodsLog (LOG_NOTICE,
         "tarSubStructFileRename: rsTarStructFileOpen error for %s, stat=%d",
          specColl->objPath, structFileInx);
        return (structFileInx);
    }

    rescTypeInx = StructFileDesc[structFileInx].rescInfo->rescTypeInx;

    memset (&fileRenameInp, 0, sizeof (fileRenameInp));
    fileRenameInp.fileType = RescTypeDef[rescTypeInx].driverType;
    status = getSubStructFilePhyPath (fileRenameInp.oldFileName, specColl,
      subFile->subFilePath);
    if (status < 0) return status;
    status = getSubStructFilePhyPath (fileRenameInp.newFileName, specColl,
      newFileName);
    if (status < 0) return status;
    rstrcpy (fileRenameInp.addr.hostAddr,
      StructFileDesc[structFileInx].rescInfo->rescLoc, NAME_LEN);
    status = rsFileRename (rsComm, &fileRenameInp);

    if (status >= 0) {
        int status1;
	/* use the specColl in StructFileDesc */
	specColl = StructFileDesc[structFileInx].specColl;
        /* cache has been written */
        if (specColl->cacheDirty == 0) {
            specColl->cacheDirty = 1;
            status1 = modTarCollInfo2 (rsComm, specColl);
            if (status1 < 0) return status1;
        }
    }

    return status;
}

int
tarSubStructFileMkdir (rsComm_t *rsComm, subFile_t *subFile)
{
    specColl_t *specColl;
    int structFileInx;
    int rescTypeInx;
    int status;
    fileMkdirInp_t fileMkdirInp;

    specColl = subFile->specColl;
    structFileInx = rsTarStructFileOpen (rsComm, specColl);

    if (structFileInx < 0) {
        rodsLog (LOG_NOTICE,
         "tarSubStructFileOpendir: rsTarStructFileOpen error for %s,stat=%d",
          specColl->objPath, structFileInx);
        return (structFileInx);
    }

    rescTypeInx = StructFileDesc[structFileInx].rescInfo->rescTypeInx;
    fileMkdirInp.fileType = RescTypeDef[rescTypeInx].driverType;
    status = getSubStructFilePhyPath (fileMkdirInp.dirName, specColl,
      subFile->subFilePath);
    if (status < 0) return status;

    rstrcpy (fileMkdirInp.addr.hostAddr,
      StructFileDesc[structFileInx].rescInfo->rescLoc, NAME_LEN);
    fileMkdirInp.mode = subFile->mode;
    status = rsFileMkdir (rsComm, &fileMkdirInp);

    if (status >= 0) {
        int status1;
        /* use the specColl in StructFileDesc */
        specColl = StructFileDesc[structFileInx].specColl;
        /* cache has been written */
        if (specColl->cacheDirty == 0) {
            specColl->cacheDirty = 1;
            status1 = modTarCollInfo2 (rsComm, specColl);
            if (status1 < 0) return status1;
        }
    }
    return status;
}

int
tarSubStructFileRmdir (rsComm_t *rsComm, subFile_t *subFile)
{
    specColl_t *specColl;
    int structFileInx;
    int rescTypeInx;
    int status;
    fileMkdirInp_t fileMkdirInp;

    specColl = subFile->specColl;
    structFileInx = rsTarStructFileOpen (rsComm, specColl);

    if (structFileInx < 0) {
        rodsLog (LOG_NOTICE,
         "tarSubStructFileOpendir: rsTarStructFileOpen error for %s,stat=%d",
          specColl->objPath, structFileInx);
        return (structFileInx);
    }

    rescTypeInx = StructFileDesc[structFileInx].rescInfo->rescTypeInx;
    fileMkdirInp.fileType = RescTypeDef[rescTypeInx].driverType;
    status = getSubStructFilePhyPath (fileMkdirInp.dirName, specColl,
      subFile->subFilePath);
    if (status < 0) return status;

    rstrcpy (fileMkdirInp.addr.hostAddr,
      StructFileDesc[structFileInx].rescInfo->rescLoc, NAME_LEN);
    fileMkdirInp.mode = subFile->mode;
    status = rsFileMkdir (rsComm, &fileMkdirInp);

    if (status >= 0) {
        int status1;
        /* use the specColl in StructFileDesc */
        specColl = StructFileDesc[structFileInx].specColl;
        /* cache has been written */
        if (specColl->cacheDirty == 0) {
            specColl->cacheDirty = 1;
            status1 = modTarCollInfo2 (rsComm, specColl);
            if (status1 < 0) return status1;
        }
    }
    return status;
}

int
tarSubStructFileOpendir (rsComm_t *rsComm, subFile_t *subFile)
{
    specColl_t *specColl;
    int structFileInx;
    int subInx;
    int rescTypeInx;
    int status;
    fileOpendirInp_t fileOpendirInp;

    specColl = subFile->specColl;
    structFileInx = rsTarStructFileOpen (rsComm, specColl);

    if (structFileInx < 0) {
        rodsLog (LOG_NOTICE,
         "tarSubStructFileOpendir: rsTarStructFileOpen error for %s,stat=%d",
          specColl->objPath, structFileInx);
        return (structFileInx);
    }

    subInx = allocTarSubFileDesc ();

    if (subInx < 0) return subInx;

    TarSubFileDesc[subInx].structFileInx = structFileInx;
    
    memset (&fileOpendirInp, 0, sizeof (fileOpendirInp));
    status = getSubStructFilePhyPath (fileOpendirInp.dirName, specColl, 
      subFile->subFilePath);
    if (status < 0) return status;

    rescTypeInx = StructFileDesc[structFileInx].rescInfo->rescTypeInx;
    fileOpendirInp.fileType = RescTypeDef[rescTypeInx].driverType;
    rstrcpy (fileOpendirInp.addr.hostAddr,
    StructFileDesc[structFileInx].rescInfo->rescLoc, NAME_LEN);

    status = rsFileOpendir (rsComm, &fileOpendirInp);
    if (status < 0) {
       rodsLog (LOG_ERROR,
          "specCollOpendir: rsFileOpendir for %s error, status = %d",
          fileOpendirInp.dirName, status);
        return status;
    } else {
	TarSubFileDesc[subInx].fd = status;
	StructFileDesc[structFileInx].openCnt++;
        return (subInx);
    }
}

int
tarSubStructFileReaddir (rsComm_t *rsComm, int subInx, 
rodsDirent_t **rodsDirent)
{
    fileReaddirInp_t fileReaddirInp;
    int status;

    if (subInx < 1 || subInx >= NUM_TAR_SUB_FILE_DESC ||
      TarSubFileDesc[subInx].inuseFlag == 0) {
        rodsLog (LOG_ERROR,
         "tarSubStructFileReaddir: subInx %d out of range",
	  subInx);
	return (SYS_STRUCT_FILE_DESC_ERR);
    }
       
    fileReaddirInp.fileInx = TarSubFileDesc[subInx].fd;
    status = rsFileReaddir (rsComm, &fileReaddirInp, rodsDirent);

    return (status);
}

int
tarSubStructFileClosedir (rsComm_t *rsComm, int subInx)
{
    fileClosedirInp_t fileClosedirInp;
    int structFileInx;
    int status;

    if (subInx < 1 || subInx >= NUM_TAR_SUB_FILE_DESC ||
      TarSubFileDesc[subInx].inuseFlag == 0) {
        rodsLog (LOG_ERROR,
         "tarSubStructFileClosedir: subInx %d out of range",
          subInx);
        return (SYS_STRUCT_FILE_DESC_ERR);
    }
    
    fileClosedirInp.fileInx = TarSubFileDesc[subInx].fd;
    status = rsFileClosedir (rsComm, &fileClosedirInp);
    
    structFileInx = TarSubFileDesc[subInx].structFileInx;
    StructFileDesc[structFileInx].openCnt++;
    freeTarSubFileDesc (subInx);
    
    return (status);
}

int
tarSubStructFileTruncate (rsComm_t *rsComm, subFile_t *subFile)
{
    specColl_t *specColl;
    int structFileInx;
    int rescTypeInx;
    int status;
    fileOpenInp_t fileTruncateInp;

    specColl = subFile->specColl;
    structFileInx = rsTarStructFileOpen (rsComm, specColl);

    if (structFileInx < 0) {
        rodsLog (LOG_NOTICE,
         "tarSubStructFileTruncate: rsTarStructFileOpen error for %s,stat=%d",
          specColl->objPath, structFileInx);
        return (structFileInx);
    }

    rescTypeInx = StructFileDesc[structFileInx].rescInfo->rescTypeInx;
    fileTruncateInp.fileType = RescTypeDef[rescTypeInx].driverType;
    status = getSubStructFilePhyPath (fileTruncateInp.fileName, specColl,
      subFile->subFilePath);
    if (status < 0) return status;

    rstrcpy (fileTruncateInp.addr.hostAddr,
      StructFileDesc[structFileInx].rescInfo->rescLoc, NAME_LEN);
    fileTruncateInp.dataSize = subFile->offset;
    status = rsFileTruncate (rsComm, &fileTruncateInp);

    if (status >= 0) {
        int status1;
        /* use the specColl in StructFileDesc */
        specColl = StructFileDesc[structFileInx].specColl;
        /* cache has been written */
        if (specColl->cacheDirty == 0) {
            specColl->cacheDirty = 1;
            status1 = modTarCollInfo2 (rsComm, specColl);
            if (status1 < 0) return status1;
        }
    }
    return status;
}

int
initStructFileDesc ()
{
    memset (StructFileDesc, 0,
      sizeof (structFileDesc_t) * NUM_STRUCT_FILE_DESC);
    return (0);
}

int
allocStructFileDesc ()
{
    int i;

    for (i = 1; i < NUM_STRUCT_FILE_DESC; i++) {
        if (StructFileDesc[i].inuseFlag == FD_FREE) {
            StructFileDesc[i].inuseFlag = FD_INUSE;
            return (i);
        };
    }

    rodsLog (LOG_NOTICE,
     "allocStructFileDesc: out of StructFileDesc");

    return (SYS_OUT_OF_FILE_DESC);
}

int
freeStructFileDesc (int structFileInx)
{
    if (structFileInx < 0 || structFileInx >= NUM_STRUCT_FILE_DESC) {
        rodsLog (LOG_NOTICE,
         "freeStructFileDesc: structFileInx %d out of range", structFileInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }

    memset (&StructFileDesc[structFileInx], 0, sizeof (structFileDesc_t));

    return (0);
}

int
initTarSubFileDesc ()
{
    memset (TarSubFileDesc, 0,
      sizeof (tarSubFileDesc_t) * NUM_TAR_SUB_FILE_DESC);
    return (0);
}

int
allocTarSubFileDesc ()
{
    int i;

    for (i = 1; i < NUM_TAR_SUB_FILE_DESC; i++) {
        if (TarSubFileDesc[i].inuseFlag == FD_FREE) {
            TarSubFileDesc[i].inuseFlag = FD_INUSE;
            return (i);
        };
    }

    rodsLog (LOG_NOTICE,
     "allocTarSubFileDesc: out of TarSubFileDesc");

    return (SYS_OUT_OF_FILE_DESC);
}

int
freeTarSubFileDesc (int tarSubFileInx)
{
    if (tarSubFileInx < 0 || tarSubFileInx >= NUM_TAR_SUB_FILE_DESC) {
        rodsLog (LOG_NOTICE,
         "freeTarSubFileDesc: tarSubFileInx %d out of range", tarSubFileInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }

    memset (&TarSubFileDesc[tarSubFileInx], 0, sizeof (tarSubFileDesc_t));

    return (0);
}

int
rsTarStructFileOpen (rsComm_t *rsComm, specColl_t *specColl)
{
    int structFileInx;
    int status;
    specCollCache_t *specCollCache;

    if (specColl == NULL) {
        rodsLog (LOG_NOTICE,
         "rsTarStructFileOpen: NULL specColl input");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    /* look for opened StructFileDesc */

    structFileInx = matchStructFileDesc (specColl);

    if (structFileInx > 0) return structFileInx;
 
    if ((structFileInx = allocStructFileDesc ()) < 0) {
        return (structFileInx);
    }

#if 0
    StructFileDesc[structFileInx].specColl = specColl;
#endif
    /* Have to do this because specColl could come from a remote host */
    if ((status = getSpecCollCache (rsComm, specColl->collection, 0,
      &specCollCache)) >= 0) {
	StructFileDesc[structFileInx].specColl = &specCollCache->specColl;
    } else {
        rodsLog (LOG_NOTICE,
          "rsTarStructFileOpen: getSpecCollCache error for %s, status = %d",
          specColl->collection, status);
	return (status);
    }

    StructFileDesc[structFileInx].rsComm = rsComm;

    status = resolveResc (specColl->resource, 
      &StructFileDesc[structFileInx].rescInfo);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
          "rsTarStructFileOpen: resolveResc error for %s, status = %d",
          specColl->resource, status);
	freeStructFileDesc (structFileInx);
        return (status);
    }

    /* XXXXX need to deal with remote open here */

    status = stageTarStructFile (structFileInx);

    if (status < 0) {
	freeStructFileDesc (structFileInx);
	return status;
    }

    return (structFileInx);
}

int
matchStructFileDesc (specColl_t *specColl)
{
    int i;

    for (i = 1; i < NUM_STRUCT_FILE_DESC; i++) {
        if (StructFileDesc[i].specColl == specColl &&
	  StructFileDesc[i].inuseFlag == FD_INUSE)  {
            return (i);
        };
    }

    return (SYS_OUT_OF_FILE_DESC);

}

int
stageTarStructFile (int structFileInx)
{
    int status;
    specColl_t *specColl; 
    rsComm_t *rsComm;

    rsComm = StructFileDesc[structFileInx].rsComm;
    specColl = StructFileDesc[structFileInx].specColl;
    if (specColl == NULL) {
        rodsLog (LOG_NOTICE,
         "stageTarStructFile: NULL specColl input");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (strlen (specColl->cacheDir) == 0) {
        /* no cache. stage one. make the CacheDir first */
        status = mkTarCacheDir (structFileInx);
	if (status < 0) return status;

        status = extractTarFile (structFileInx);
        if (status < 0) {
            rodsLog (LOG_NOTICE,
             "stageTarStructFile:extract error for %s in cacheDir %s,errno=%d",
	     specColl->objPath, specColl->cacheDir, errno);
	    /* XXXX may need to remove the cacheDir too */
	    return SYS_TAR_STRUCT_FILE_EXTRACT_ERR - errno; 
	}
	/* register the CacheDir */
	status = modTarCollInfo2 (rsComm, specColl);
        if (status < 0) return status;

#if 0
	memset (&modCollInp, 0, sizeof (modCollInp));

        rstrcpy (modCollInp.collName, specColl->collection, MAX_NAME_LEN);
	makeCachedStructFileStr (collInfo2, specColl);
        addKeyVal (&modCollInp.condInput, COLLECTION_TYPE_KW, 
	  TAR_STRUCT_FILE_STR);	/* need this or rsModColl fail */
        addKeyVal (&modCollInp.condInput, COLLECTION_INFO2_KW, collInfo2);
	status = rsModColl (rsComm, &modCollInp);
	if (status < 0) {
	    /* XXXX need to remove the cacheDir too */
            rodsLog (LOG_NOTICE,
             "stageTarStructFile:rsModColl error for specColl %s,status=%d",
             modCollInp.collName, status);
            /* may need to delete the cacheDir */
	    return status;
	}
#endif
    }
    return (0);
}

int
mkTarCacheDir (int structFileInx) 
{
    int i = 0;
    int status;
    fileMkdirInp_t fileMkdirInp;
    int rescTypeInx;
    rescInfo_t *rescInfo;
    rsComm_t *rsComm = StructFileDesc[structFileInx].rsComm;
    specColl_t *specColl = StructFileDesc[structFileInx].specColl;

    if (specColl == NULL) {
        rodsLog (LOG_NOTICE, "mkTarCacheDir: NULL specColl input");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rescInfo = StructFileDesc[structFileInx].rescInfo;

    memset (&fileMkdirInp, 0, sizeof (fileMkdirInp));
    rescTypeInx = rescInfo->rescTypeInx;
    fileMkdirInp.fileType = RescTypeDef[rescTypeInx].driverType;
    fileMkdirInp.mode = DEFAULT_DIR_MODE;
    rstrcpy (fileMkdirInp.addr.hostAddr,  rescInfo->rescLoc, NAME_LEN);

    while (1) {
        snprintf (fileMkdirInp.dirName, MAX_NAME_LEN, "%s.dir%d",
          specColl->phyPath, i);
        status = rsFileMkdir (rsComm, &fileMkdirInp);
        if (status >= 0) {
	    break;
	} else {
	    if (getUnixErrno (status) == EEXIST) {
		i++;
		continue;
	    } else {
                return (status);
	    }
        }
    }
    rstrcpy (specColl->cacheDir, fileMkdirInp.dirName, MAX_NAME_LEN);

    return (0);
}

/* this set of codes irodsTarXYZ are used by libtar to perform file level
 * I/O in iRods */
 
int
irodsTarOpen (char *pathname, int oflags, int mode)
{
    int structFileInx;
    int myMode;
    int status;
    fileOpenInp_t fileOpenInp;
    rescInfo_t *rescInfo;
    specColl_t *specColl = NULL;
    int rescTypeInx;
    int l3descInx;

    /* the upper most 4 bits of mode is the structFileInx */ 
    decodeIrodsTarfd (mode, &structFileInx, &myMode); 
    status = verifyStructFileDesc (structFileInx, pathname, &specColl);
    if (status < 0) return -1;	/* tar lib looks for -1 return */

    rescInfo = StructFileDesc[structFileInx].rescInfo;
    rescTypeInx = rescInfo->rescTypeInx;

    memset (&fileOpenInp, 0, sizeof (fileOpenInp));
    fileOpenInp.fileType = RescTypeDef[rescTypeInx].driverType;
    rstrcpy (fileOpenInp.addr.hostAddr,  rescInfo->rescLoc, NAME_LEN);
    rstrcpy (fileOpenInp.fileName, specColl->phyPath, MAX_NAME_LEN);
    fileOpenInp.mode = myMode;
    fileOpenInp.flags = oflags;
    l3descInx = rsFileOpen (StructFileDesc[structFileInx].rsComm, 
      &fileOpenInp);
    
    if (l3descInx < 0) {
        rodsLog (LOG_NOTICE, 
	  "irodsTarOpen: rsFileOpen of %s in Resc %s error, status = %d",
	  fileOpenInp.fileName, rescInfo->rescName, l3descInx);
        return (-1);	/* libtar only accept -1 */
    }
    return (encodeIrodsTarfd (structFileInx, l3descInx));
}

int
encodeIrodsTarfd (int upperInt, int lowerInt)
{
    /* use the upper 5 of the 6 bits for upperInt */

    if (upperInt > 60) {	/* 0x7c */
        rodsLog (LOG_NOTICE,
          "encodeIrodsTarfd: upperInt %d larger than 60", lowerInt);
        return (SYS_STRUCT_FILE_DESC_ERR);
    }

    if ((lowerInt & 0xfc000000) != 0) {
        rodsLog (LOG_NOTICE,
          "encodeIrodsTarfd: lowerInt %d too large", lowerInt);
        return (SYS_STRUCT_FILE_DESC_ERR);
    }

    return (lowerInt | (upperInt << 26));
    
}

int
decodeIrodsTarfd (int inpInt, int *upperInt, int *lowerInt)
{
    *lowerInt = inpInt & 0x03ffffff;
    *upperInt = (inpInt & 0x7c000000) >> 26;

    return (0);
}

int
irodsTarClose (int fd)
{
    fileCloseInp_t fileCloseInp;
    int structFileInx, l3descInx;
    int status;

    decodeIrodsTarfd (fd, &structFileInx, &l3descInx);
    memset (&fileCloseInp, 0, sizeof (fileCloseInp));
    fileCloseInp.fileInx = l3descInx;
    status = rsFileClose (StructFileDesc[structFileInx].rsComm, 
      &fileCloseInp);

    return (status);
}

int
irodsTarRead (int fd, char *buf, int len)
{
    fileReadInp_t fileReadInp;
    int structFileInx, l3descInx;
    int status;
    bytesBuf_t readOutBBuf;

    decodeIrodsTarfd (fd, &structFileInx, &l3descInx);
    memset (&fileReadInp, 0, sizeof (fileReadInp));
    fileReadInp.fileInx = l3descInx;
    fileReadInp.len = len;
    memset (&readOutBBuf, 0, sizeof (readOutBBuf));
    readOutBBuf.buf = buf;
    status = rsFileRead (StructFileDesc[structFileInx].rsComm, 
      &fileReadInp, &readOutBBuf);

    return (status);

}

int
irodsTarWrite (int fd, char *buf, int len)
{
    fileWriteInp_t fileWriteInp;
    int structFileInx, l3descInx;
    int status;
    bytesBuf_t writeInpBBuf;

    decodeIrodsTarfd (fd, &structFileInx, &l3descInx);
    memset (&fileWriteInp, 0, sizeof (fileWriteInp));
    fileWriteInp.fileInx = l3descInx;
    fileWriteInp.len = len;
    memset (&writeInpBBuf, 0, sizeof (writeInpBBuf));
    writeInpBBuf.buf = buf;
    status = rsFileWrite (StructFileDesc[structFileInx].rsComm, 
      &fileWriteInp, &writeInpBBuf);

    return (status);
}

int 
verifyStructFileDesc (int structFileInx, char *tarPathname, 
specColl_t **specColl)
{
    if (StructFileDesc[structFileInx].inuseFlag <= 0) {
        rodsLog (LOG_NOTICE,
          "verifyStructFileDesc: structFileInx %d not in use", structFileInx);
        return (SYS_STRUCT_FILE_DESC_ERR);
    }
    if (StructFileDesc[structFileInx].specColl == NULL) {
        rodsLog (LOG_NOTICE,
          "verifyStructFileDesc: NULL specColl for structFileInx %d", 
	  structFileInx);
        return (SYS_STRUCT_FILE_DESC_ERR);
    }
    if (strcmp (StructFileDesc[structFileInx].specColl->phyPath, tarPathname)
      != 0) {
        rodsLog (LOG_NOTICE,
          "verifyStructFileDesc: phyPath %s in Inx %d does not match %s",
          StructFileDesc[structFileInx].specColl->phyPath, structFileInx, 
	  tarPathname);
        return (SYS_STRUCT_FILE_DESC_ERR);
    }
    if (specColl != NULL) {
	*specColl = StructFileDesc[structFileInx].specColl;
    }

    return 0;
}

int
extractTarFile (int structFileInx)
{
    TAR *t;
    specColl_t *specColl = StructFileDesc[structFileInx].specColl;
    int myMode;
    int status; 
    

    if (StructFileDesc[structFileInx].inuseFlag <= 0) {
        rodsLog (LOG_NOTICE,
          "extractTarFile: structFileInx %d not in use", structFileInx);
        return (SYS_STRUCT_FILE_DESC_ERR);
    }

    if (specColl == NULL || strlen (specColl->cacheDir) == 0 ||
     strlen (specColl->phyPath) == 0) {
        rodsLog (LOG_NOTICE,
          "extractTarFile: Bad specColl for structFileInx %d ", 
	  structFileInx);
        return (SYS_STRUCT_FILE_DESC_ERR);
    }
    myMode = encodeIrodsTarfd (structFileInx, DEFAULT_FILE_MODE);

    if (myMode < 0) {
        return (myMode);
    }

    status = tar_open (&t, specColl->phyPath, &irodstype, 
      O_RDONLY, myMode, TAR_GNU);

    if (status < 0) {
	rodsLog (LOG_NOTICE,
          "extractTarFile: tar_open error for %s, errno = %d",
          specColl->phyPath, errno);
        return (SYS_TAR_OPEN_ERR - errno);
    }

    if (tar_extract_all (t, specColl->cacheDir) != 0) {
        rodsLog (LOG_NOTICE,
          "extractTarFile: tar_extract_all error for %s, errno = %d",
          specColl->phyPath, errno);
        return (SYS_TAR_EXTRACT_ALL_ERR - errno);
    }

    if (tar_close(t) != 0) {
        rodsLog (LOG_NOTICE,
          "extractTarFile: tar_close error for %s, errno = %d",
          specColl->phyPath, errno);
        return (SYS_TAR_CLOSE_ERR - errno);
    }

    return 0;
}

/* getSubStructFilePhyPath - get the phy path in the cache dir */
int
getSubStructFilePhyPath (char *phyPath, specColl_t *specColl, 
char *subFilePath)
{
    int len;

    /* subFilePath is composed by appending path to the objPath which is
     * the logical path of the tar file. So we need to substitude the
     * objPath with cacheDir */ 


#if 0
    if (strcmp (subFilePath, specColl->collection) == 0) {
	/* the top collection */
	rstrcpy (phyPath, specColl->cacheDir, MAX_NAME_LEN);
    } else {
        len = strlen (specColl->objPath);
        if (strncmp (specColl->objPath, subFilePath, len) != 0) {
            rodsLog (LOG_NOTICE,
             "getSubStructFilePhyPath: objPath %s subFilePath %s mismatch",
	      specColl->objPath, subFilePath); 
            return (SYS_STRUCT_FILE_PATH_ERR);
        }

        snprintf (phyPath, MAX_NAME_LEN, "%s%s", specColl->cacheDir,
         subFilePath + len);
    }
#endif
    len = strlen (specColl->collection);
    if (strncmp (specColl->collection, subFilePath, len) != 0) {
            rodsLog (LOG_NOTICE,
             "getSubStructFilePhyPath: collection %s subFilePath %s mismatch",
              specColl->collection, subFilePath);
            return (SYS_STRUCT_FILE_PATH_ERR);
        }

        snprintf (phyPath, MAX_NAME_LEN, "%s%s", specColl->cacheDir,
         subFilePath + len);


    return (0);
}

int
modTarCollInfo2 (rsComm_t *rsComm, specColl_t *specColl)
{
    int status;
    char collInfo2[MAX_NAME_LEN];
    collInp_t modCollInp;

    memset (&modCollInp, 0, sizeof (modCollInp));
    rstrcpy (modCollInp.collName, specColl->collection, MAX_NAME_LEN);
    makeCachedStructFileStr (collInfo2, specColl);
    addKeyVal (&modCollInp.condInput, COLLECTION_TYPE_KW,
      TAR_STRUCT_FILE_STR); /* need this or rsModColl fail */
    addKeyVal (&modCollInp.condInput, COLLECTION_INFO2_KW, collInfo2);
    status = rsModColl (rsComm, &modCollInp);
    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "tarSubStructFileWrite:rsModColl error for Coll %s,stat=%d",
         modCollInp.collName, status);
    }
    return status;
}

