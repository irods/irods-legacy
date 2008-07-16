/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* fileDriverTable.h - header file for the global file driver table
 */



#ifndef FILE_DRIVER_TABLE_H
#define FILE_DRIVER_TABLE_H

#include "rods.h"
#include "fileDriver.h"
#include "unixFileDriver.h"
#include "miscServerFunct.h"

#define NO_FILE_DRIVER_FUNCTIONS intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,longNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport,intNoSupport, intNoSupport, longNoSupport, intNoSupport 

fileDriver_t FileDriverTable[] = {
    {UNIX_FILE_TYPE, unixFileCreate, unixFileOpen, unixFileRead, unixFileWrite,
    unixFileClose, unixFileUnlink, unixFileStat, unixFileFstat, unixFileLseek,
    unxiFileFsync, unixFileMkdir, unixFileChmod, unixFileRmdir, unixFileOpendir,
    unixFileClosedir, unixFileReaddir, unixFileStage, unixFileRename,
    unixFileGetFsFreeSpace, unixFileTruncate},
#ifdef HPSS
    {HPSS_FILE_TYPE, hpssFileCreate, hpssFileOpen, hpssFileRead, hpssFileWrite,
    hpssFileClose, hpssFileUnlink, hpssFileStat, hpssFileFstat, hpssFileLseek,
    unxiFileFsync, hpssFileMkdir, hpssFileChmod, hpssFileRmdir, hpssFileOpendir,
    hpssFileClosedir, hpssFileReaddir, hpssFileStage, hpssFileRename,
    hpssFileGetFsFreeSpace, hpssFileTruncate},
#else
    {HPSS_FILE_TYPE, NO_FILE_DRIVER_FUNCTIONS},
#endif
};

int NumFileDriver = sizeof (FileDriverTable) / sizeof (fileDriver_t);

#endif	/* FILE_DRIVER_TABLE_H */
