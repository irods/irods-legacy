# $1 = fuse mount point
export LD_LIBRARY_PATH=/usr/local/lib
$IRODS_HOME/clients/fuse/bin/irodsFs $1
