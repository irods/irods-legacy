# $1 = fuse mount point $2 = (optional) stdout
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
if [ "$2" == "stdout" ]
then
$IRODS_HOME/clients/fuse/bin/irodsFs -d $1 
else
$IRODS_HOME/clients/fuse/bin/irodsFs -d $1 > debug.out 2> debug.err
fi
