HDF5/iRODS module
------------------------------------------------------------------------
The HDF5/iRODS module is to support interactive access to HDF5 files
stored at iRODS server. This module is disabled by default. To use this
module, you must follow the steps below

	1.  Download and install szip, zlib, and HDF5 libraries from
                http://www.hdfgroup.org/HDF5/release/obtain5.html

                if you are using HDF5 1.8.x version, you must use
                the 5-1.8.x-16API build. If you are building HDF5 1.8.x
                from the source, you must use the HDF5 1.6 compatibility 
                flag in the configuration, such as
                ./configure --with-default-api-version=v16 ...

	2.  Edit the 'Makefile' in this module and set the follow variables:
                hdf5Dir, szlibDir, and zlibDir.

                For example,
                hdf5Dir=/home/srb/linux32/hdf5
                szlibDir=/home/srb/ext_lib/szip
                zlibDir=/home/srb/ext_lib/zlib

	3.  Recompile iRODS by typing 'make'.  You do not need to
	    run any of the install scripts again.

	4.  Restart iRODS by runing 'irodsctl restart'.
