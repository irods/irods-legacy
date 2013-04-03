#!/bin/sh
# Simple script to build boost-irods, configure irods to use it, rebuild
# irods with it, and run the test suite.
#
# For now, the buildboost.sh has been commented out as an initial step
# in switching to a system-installed boost.  We no longer have to build
# boost.
set -x
date
cd /tbox/IRODS_BUILD/iRODS
make clean
# buildboost.sh
date

cd config
mv config.mk config.mk.old
cat config.mk.old | sed 's/# USE_BOOST=1/USE_BOOST=1/g' > config.mk
cd ..

#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/tbox/IRODS_BUILD/iRODS/boost_irods/lib

make
date

./irodsctl restart
./irodsctl testWithoutConfirmation
res=$?
date
exit $res
