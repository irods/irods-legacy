/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to structFiles in the COPYRIGHT directory ***/

/* structFileDriverTable.h - header structFile for the global structFile driver table
 */



#ifndef STRUCT_FILE_DRIVER_TABLE_H
#define STRUCT_FILE_DRIVER_TABLE_H

#include "rods.h"
#include "structFileDriver.h"
#include "haawSubStructFileDriver.h"

structFileDriver_t StructFileDriverTable[] = {
    {HAAW_STRUCT_FILE, haawSubStructFileCreate, haawSubStructFileOpen, haawSubStructFileRead, 
      haawSubStructFileWrite, haawSubStructFileClose, haawSubStructFileUnlink, haawSubStructFileStat, 
      haawSubStructFileFstat, haawSubStructFileLseek, haawSubStructFileRename, haawSubStructFileMkdir,
      haawSubStructFileRmdir, haawSubStructFileOpendir, haawSubStructFileReaddir,
      haawSubStructFileClosedir, haawSubStructFileTruncate},
};

int NumStructFileDriver = sizeof (StructFileDriverTable) / sizeof (structFileDriver_t);

#endif	/* STRUCT_FILE_DRIVER_TABLE_H */
