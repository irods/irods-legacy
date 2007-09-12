/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/

/* bundleDriver.c - The general driver for all bundle types. */

#include "bundleDriver.h"
#include "bundleDriverTable.h"
#include "apiHeaderAll.h"	/* XXXXX no needed open bundle api done */

int
bunSubCreate (rsComm_t *rsComm, subFile_t *subFile)
{
    bunType_t myType;
    int bunSubInx;
    int fd;

    myType = subFile->specColl->type;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return (bunSubInx);
    }
    
    fd = BundleDriverTable[bunSubInx].bunSubCreate (rsComm, subFile);
    return (fd);
}


int
bunSubOpen (rsComm_t *rsComm, subFile_t *subFile)
{
    bunType_t myType;
    int bunSubInx;
    int fd;
    
    myType = subFile->specColl->type;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return (bunSubInx);
    }

    fd = BundleDriverTable[bunSubInx].bunSubOpen (rsComm, subFile);
    return (fd);
}

int
bunSubRead (bunType_t myType, rsComm_t *rsComm, int fd, void *buf,
int len)
{
    int bunSubInx;
    int status;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return (bunSubInx);
    }

    status = BundleDriverTable[bunSubInx].bunSubRead (rsComm, fd, buf, len);
    return (status);
}

int
bunSubWrite (bunType_t myType, rsComm_t *rsComm, int fd, void *buf, int len)
{
    int bunSubInx;
    int status;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return (bunSubInx);
    }

    status = BundleDriverTable[bunSubInx].bunSubWrite (rsComm, fd, buf, len);
    return (status);
}

int
bunSubClose (bunType_t myType, rsComm_t *rsComm, int fd)
{
    int bunSubInx;
    int status;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return (bunSubInx);
    }

    status = BundleDriverTable[bunSubInx].bunSubClose (rsComm, fd);
    return (status);
}

int
bunSubUnlink (rsComm_t *rsComm, subFile_t *subFile)
{
    bunType_t myType;
    int bunSubInx;
    int status;

    myType = subFile->specColl->type;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return (bunSubInx);
    }

    status = BundleDriverTable[bunSubInx].bunSubUnlink (rsComm, subFile);
    return (status);
}

int
bunSubStat (rsComm_t *rsComm, subFile_t *subFile,
rodsStat_t *bunSubStatOut)
{
    bunType_t myType;
    int bunSubInx;
    int status;

    myType = subFile->specColl->type;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return (bunSubInx);
    }

    status = BundleDriverTable[bunSubInx].bunSubStat (rsComm, subFile,
      bunSubStatOut);
    return (status);
}

int
bunSubFstat (bunType_t myType, rsComm_t *rsComm, int fd,
rodsStat_t *bunSubStatOut)
{
    int bunSubInx;
    int status;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return (bunSubInx);
    }

    status = BundleDriverTable[bunSubInx].bunSubFstat (rsComm, fd, 
      bunSubStatOut);
    return (status);
}

rodsLong_t
bunSubLseek (bunType_t myType, rsComm_t *rsComm, int fd,
rodsLong_t offset, int whence)
{
    int bunSubInx;
    rodsLong_t status;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return ((rodsLong_t) bunSubInx);
    }

    status = BundleDriverTable[bunSubInx].bunSubLseek (rsComm, fd, offset,
      whence);
    return (status);
}

int
bunSubRename (rsComm_t *rsComm, subFile_t *subFile, char *newFileName)
{
    bunType_t myType;
    int bunSubInx;
    int status;

    myType = subFile->specColl->type;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return (bunSubInx);
    }

    status = BundleDriverTable[bunSubInx].bunSubRename (rsComm, subFile,
      newFileName);
    return (status);
}

int
bunSubMkdir (rsComm_t *rsComm, subFile_t *subFile)
{
    bunType_t myType;
    int bunSubInx;
    int status;

    myType = subFile->specColl->type;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return (bunSubInx);
    }

    status = BundleDriverTable[bunSubInx].bunSubMkdir (rsComm, subFile);
    return (status);
}

int
bunSubRmdir (rsComm_t *rsComm, subFile_t *subFile)
{
    bunType_t myType;
    int bunSubInx;
    int status;

    myType = subFile->specColl->type;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return (bunSubInx);
    }

    status = BundleDriverTable[bunSubInx].bunSubRmdir (rsComm, subFile);
    return (status);
}

int
bunSubOpendir (rsComm_t *rsComm, subFile_t *subFile)
{
    bunType_t myType;
    int bunSubInx;
    int status;

    myType = subFile->specColl->type;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return (bunSubInx);
    }

    status = BundleDriverTable[bunSubInx].bunSubOpendir (rsComm, subFile);
    return (status);
}

int
bunSubReaddir (bunType_t myType, rsComm_t *rsComm, int fd, 
rodsDirent_t *rodsDirent)
{
    int bunSubInx;
    rodsLong_t status;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return ((rodsLong_t) bunSubInx);
    }

    status = BundleDriverTable[bunSubInx].bunSubReaddir (rsComm, fd,
      rodsDirent);
    return (status);
}

int
bunSubClosedir (bunType_t myType, rsComm_t *rsComm, int fd)
{
    int bunSubInx;
    rodsLong_t status;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return ((rodsLong_t) bunSubInx);
    }

    status = BundleDriverTable[bunSubInx].bunSubClosedir (rsComm, fd); 
    return (status);
}

int
bunSubTruncate (rsComm_t *rsComm, subFile_t *subFile)
{
    bunType_t myType;
    int bunSubInx;
    int status;

    myType = subFile->specColl->type;

    if ((bunSubInx = bunSubIndexLookup (myType)) < 0) {
        return (bunSubInx);
    }

    status = BundleDriverTable[bunSubInx].bunSubTruncate (rsComm, subFile);
    return (status);
}

int
bunSubIndexLookup (bunType_t myType)
{
    int i;

    for (i = 0; i < NumBundleDriver; i++) {
        if (myType == BundleDriverTable[i].type)
            return (i);
    }

    rodsLog (LOG_ERROR, "bundleIndexLookup error for type %d", myType);

    return (FILE_INDEX_LOOKUP_ERR);
}

