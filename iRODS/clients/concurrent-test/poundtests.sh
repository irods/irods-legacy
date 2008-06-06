#!/bin/sh -e

# SDL Usage # concurrent tests
# Usage right now: no parameters, verbose set to 1 by default
# numoferrors set to 1 by default right now also

verboseflag=1
numoferrors=1
OS=Darwin

TMP_DIR=TMP
thistest=putget
testdirs="butter bigfiles"



# Test variables
# concurrenttests= this variable is passed in
# numfiles = vary between 0 and 1000
# filesize = vary between 0 and mega

for testdir in $testdirs; do

	testid=$thistest-$testdir-`date "+%Y%m%d%H%M%S"`

	i=0
	while [ $i -lt $1 ]; do
		sh -ex $thistest $testid $testdir > $testid.irods 2>&1
		# check for failure
		if [ "$?" -ne 0 ]; then
			echo "$testid FAILED, exiting"
			exit 3
		fi

		i=`expr $i +  1`
		sleep 3
	done
	sleep 10
	sh -ex $thistest $testid $testdir

	#sh -ex $thistest $testid > $TMP_DIR/$testid.irods 2>&1

	# check for failure
	if [ "$?" -ne 0 ]; then
		echo "$testid FAILED, exiting"
		exit 3
	fi

	#cp $TMP_DIR/$testid.irods $TMP_DIR/$thistest.log
	#cp $TMP_DIR/$thistest.log $thistest.out


	now=`date`
	echo "Success: Test $testid completed at $now" >> $0.log
	echo "$testid ended Successfully"

done

exit 0


