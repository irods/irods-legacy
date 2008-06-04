#!/bin/sh -x

# SDL Usage # concurrent tests
# Usage right now: no parameters, verbose set to 1 by default
# numoferrors set to 1 by default right now also

verboseflag=1
numoferrors=1
OS=Darwin

TMP_DIR=TMP
thistest=putget

# Test variables
# concurrenttests= this variable is passed in
# numfiles = vary between 0 and 1000
# filesize = vary between 0 and mega

testid=$thistest-`date "+%Y%m%d%H%M%S"`

echo "Starting $thistest $testid ... "

i=1
while [ $i -lt $1 ]
do
   sh $thistest $testid butter
   i=`expr $i +  1`
   sleep 3
done
sleep 10
sh $thistest $testid butter

#sh -ex $thistest $testid > $TMP_DIR/$testid.irods 2>&1

# check for failure
if [ "$?" -ne 0 ]; then
	echo "$thistest script FAILED, exiting"
	exit 3
fi

cp $TMP_DIR/$testid.irods $TMP_DIR/$thistest.log
cp $TMP_DIR/$thistest.log $thistest.out

# clean up locally
#rm $TMP_DIR/$testid.irods

now=`date`
echo "Success: Test $testid completed at $now" >> $0.log
echo "$thistest ended Successfully"
exit 0


