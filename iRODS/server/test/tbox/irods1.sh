#!/bin/sh

set -x 

myhost=`hostname`
mydate=`date`
mypwd=`pwd`

echo "$myhost:$mypwd $mydate irods1.sh starting"
../irods2.sh 2>&1
# remember error code
error1=$?

echo "$myhost:$mypwd $mydate irods1.sh ending (starting sleep), error1=($error1)"

cd $mypwd

# avoid rapid infinite loop
sleep 300

exit $error1
