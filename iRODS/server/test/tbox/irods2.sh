#!/bin/sh

set -x

myhost=`hostname`
mydate=`date`
mypwd=`pwd`

echo "$myhost:$mypwd $mydate irods2.sh starting"

#cd IRODS_BUILD

pgsql/bin/pg_ctl stop
pgsql/bin/pg_ctl status
pgsql/bin/pg_ctl kill

pgsql/bin/pg_ctl stop  -D pgsql/data
pgsql/bin/pg_ctl status -D pgsql/data
pgsql/bin/pg_ctl kill -D pgsql/data

set +x
/bin/rm -rf iRODS
/bin/rm -f /tmp/testRule_* /tmp/testSurvey_*
set -x

# Once a day, download and build postgres too
myhour=`date | cut -b12-13`
if [ $myhour =  13 ] ; then
  rm -rf pg* post* iRODS
  rm -f /tmp/postgresql* /tmp/unixODBC*
fi

rm -f irods.tar
echo $PATH
os=`uname -s`
if [ $os = "Darwin" ] ; then
curl http://users.sdsc.edu/~schroede/irods.tar > irods.tar
else
wget http://users.sdsc.edu/~schroede/irods.tar
fi

tar xf irods.tar
rm -f irods.tar

pgsql=`ls -d pgsql`

#cd /tbox/IRODS_BUILD/iRODS
cd iRODS
if [ $pgsql = "pgsql" ] ; then
  echo reusing pgsql
  ./irodssetup < ../../input1.txt.reuse.pg
else
  echo building a new pgsql
  ./irodssetup < ../../input1.txt.no.pg
fi

# remember error code
error1=$?

#./irodsctl start (don't start, is running from install
#                  and it will mess up the irodsServer.[port]
#                  file and start another REserver).
# remember error code
error2=0

error3=0
if [ $error1 -eq 0 ] ; then
  ./irodsctl testWithoutConfirmation
# remember error code
  error3=$?
fi 

# Record the test logs into the main log file available via the web
ls -lt server/test/bin/*.log
cat server/test/bin/icatTest.log
cat server/test/bin/icatMiscTest.log
cat server/test/bin/moveTest.log
ls -lt /tmp/testRule_* /tmp/testSurvey_*
cat /tmp/testRule_* /tmp/testSurvey_*

# Record the build log into the main log file available via the web
ls -lt installLogs/installMake.log
cat installLogs/installMake.log

./irodsctl stop
# remember error code
error4=$?

cd ..

pgsql/bin/pg_ctl stop  -D pgsql/data
pgsql/bin/pg_ctl status -D pgsql/data
#pgsql/bin/pg_ctl kill -D pgsql/data

mypwd=`pwd`

mydate=`date`
total=$(( $error1 + $error2 + $error3 + $error4 ))
echo "$mydate $mypwd/irods2.sh exiting total=($total)"

exit $total
