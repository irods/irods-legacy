/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bunSubs in the COPYRIGHT directory ***/
/* haawBunSubDriver.c - Module of the UK HAAW bundle driver.
 */

#include "haawBunSubDriver.h"

int
haawBunSubCreate (rsComm_t *rsComm, subFile_t *subFile)
{
    return (0);
}

int 
haawBunSubOpen (rsComm_t *rsComm, subFile_t *subFile)
{
    return (0);
}

int 
haawBunSubRead (rsComm_t *rsComm, int fd, void *buf, int len)
{
    if (len > 0) {
        rstrcpy ((char *) buf, "This is a test msg from server", len);
    }

    return (strlen ("This is a test msg from server") + 1);
}

int
haawBunSubWrite (rsComm_t *rsComm, int fd, void *buf, int len)
{
    if (len > 0 && buf != NULL) {
	return (strlen ((char *)buf) + 1);
    } else {
        return (0);
    }
}

int
haawBunSubClose (rsComm_t *rsComm, int fd)
{
    return (0);
}

int
haawBunSubUnlink (rsComm_t *rsComm, subFile_t *subFile)
{
    return (0);
}

int
haawBunSubStat (rsComm_t *rsComm, subFile_t *subFile,
rodsStat_t *bunSubStatOut)
{
    bunSubStatOut->st_size = 1000;
    bunSubStatOut->st_blocks = 10000;

    /* return -1 or give false success */
    return (-1);
}

int
haawBunSubFstat (rsComm_t *rsComm, int fd, rodsStat_t *bunSubStatOut)
{
    bunSubStatOut->st_size = 1000;
    bunSubStatOut->st_blocks = 10000;

    return (0);
}

rodsLong_t
haawBunSubLseek (rsComm_t *rsComm, int fd, rodsLong_t offset, int whence)
{
    return (offset);
}

int
haawBunSubRename (rsComm_t *rsComm, subFile_t *subFile, char *newFileName)
{
    return (123);
}

int
haawBunSubMkdir (rsComm_t *rsComm, subFile_t *subFile)
{
    return (0);
}

int
haawBunSubRmdir (rsComm_t *rsComm, subFile_t *subFile)
{
    return (0);
}

int
haawBunSubOpendir (rsComm_t *rsComm, subFile_t *subFile)
{
    return (0);
}

int
haawBunSubReaddir (rsComm_t *rsComm, int fd, rodsDirent_t *rodsDirent)
{
    /* XXXX a return of >= 0 will cause querySpecColl to go into info loop */ 
    return (-1);
}

int
haawBunSubClosedir (rsComm_t *rsComm, int fd)
{
    return (0);
}

int
haawBunSubTruncate (rsComm_t *rsComm, subFile_t *subFile)
{
    return (0);
}

