#!/bin/csh -e
#!/bin/csh -ex
# lcov script for iRODS
set libdir = "--directory ../lib/rbudp/obj --directory ../lib/md5/obj --directory ../lib/core/obj --directory ../lib/api/obj"
set serverdir = "--directory ../server/icat/obj --directory ../server/drivers/obj --directory ../server/core/obj --directory ../server/api/obj --directory ../server/re/obj"
set clientsdir = "--directory ../clients/icommands/obj" 
set fuselib = "--directory ../clients/fuse/obj"
set isioLib = "../lib/isio"
set fortranLib = "../lib/fortran"

# echo $libdir
# echo $serverdir
# echo $clientsdir

if ($#argv > 1) then
  echo "Too many input"
  echo "usage: lcovirods.csh [clean|help]"
  exit 1
else if ($#argv == 1) then
  if ($1 == "clean") then
    lcov $libdir $serverdir $clientsdir --zerocounters
    rm data/*
    exit 0
  else if ($1 == "help") then
    echo "usage: lcovirods.csh [clean|help]"
    exit 0
  else
    echo "unknown input:" $1
    exit 1
  endif
endif

lcov $libdir $serverdir $clientsdir --capture --output-file data/lcov.info
cd data 
genhtml lcov.info


