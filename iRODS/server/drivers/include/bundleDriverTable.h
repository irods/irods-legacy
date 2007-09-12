/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to bundles in the COPYRIGHT directory ***/

/* bundleDriverTable.h - header bundle for the global bundle driver table
 */



#ifndef BUNDLE_DRIVER_TABLE_H
#define BUNDLE_DRIVER_TABLE_H

#include "rods.h"
#include "bundleDriver.h"
#include "haawBunSubDriver.h"

bundleDriver_t BundleDriverTable[] = {
    {HAAW_BUNDLE, haawBunSubCreate, haawBunSubOpen, haawBunSubRead, 
      haawBunSubWrite, haawBunSubClose, haawBunSubUnlink, haawBunSubStat, 
      haawBunSubFstat, haawBunSubLseek, haawBunSubRename, haawBunSubMkdir,
      haawBunSubRmdir, haawBunSubOpendir, haawBunSubReaddir,
      haawBunSubClosedir, haawBunSubTruncate},
};

int NumBundleDriver = sizeof (BundleDriverTable) / sizeof (bundleDriver_t);

#endif	/* BUNDLE_DRIVER_TABLE_H */
