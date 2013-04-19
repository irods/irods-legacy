#!/bin/bash
# Simple script to build with FUSE and run the fuse tests.  It is
# assumed that FUSE has been installed via a package manager (so the
# include path and LD_LIBRARY_PATH do not need to be set), that
# this is being started from the iRODS top directory ('iRODS'),
# and that the i-command environment is set up.
#
set -x
STDERR_FILE=test.stderr

date

topDir=`pwd`
echo $topDir
export IRODS_HOME=$topDir

date

cd config
mv config.mk config.mk.old.fuse

cat config.mk.old.fuse | sed 's/# IRODS_FS = 1/IRODS_FS = 1/g' > config.mk
cd ..

make
cd clients/fuse
make

cd ../..
./irodsctl restart

cd clients/fuse/test
./test1.sh 2> $STDERR_FILE
if [ -s $STDERR_FILE ] ; then
echo "$STDERR_FILE has data."
cat $STDERR_FILE
exit -1
fi ;

./test2.sh 2> $STDERR_FILE
if [ -s $STDERR_FILE ] ; then
echo "$STDERR_FILE has data."
cat $STDERR_FILE
exit -2
fi ;

./test2.1.sh 2> $STDERR_FILE
if [ -s $STDERR_FILE ] ; then
echo "$STDERR_FILE has data."
cat $STDERR_FILE
exit -3
fi ;

./test2.1.cleanup.sh 2> $STDERR_FILE
if [ -s $STDERR_FILE ] ; then
echo "$STDERR_FILE has data."
cat $STDERR_FILE
exit -4
fi ;

res=$?
date
exit $res
