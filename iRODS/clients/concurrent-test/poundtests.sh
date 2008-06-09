#!/bin/bash -e

# SDL Usage # concurrent tests
# Usage right now: no parameters, verbose set to 1 by default
# numoferrors set to 1 by default right now also

verboseflag=1
numoferrors=1
OS=Darwin

TMP_DIR=TMP
thistest=putget
testdirs="zerofiles smallfiles bigfiles"

# Make data directories of varying file numbers and sizes
makefiles () {

	for dir in $testdirs; do
		test -d $dir || mkdir $dir
	done

	
	case "$dir" in
		"zerofiles")
			for ((i=1;i<=1000;i+=1)); do
				touch zerofiles/zerofile$i
			done
		;;

		"smallfiles")
			for ((i=1;i<=100;i+=1)); do
				echo "abcdefghijklmnopqrstuvwxyz" > smallfiles/smallfile$i
			done
		;;

		#Google how to write big ol' binary file
		"bigfiles")
			for ((i=1;i<=10;i+=1)); do
				for ((j=1;j<=5;j+=1)); do
					cat /mach_kernel >> bigfiles/bigfile$i
				done
			done
		;;
	esac

}

# Test variables
# concurrenttests= this variable is passed in
# numfiles = vary between 0 and 1000
# filesize = vary between 0 and mega

for testdir in $testdirs; do

	i=0
	while [ $i -lt $1 ]; do
		testid=$thistest-$testdir-`date "+%Y%m%d%H%M%S"`
		echo $i of $1:$thistest $testid
		sh -e $thistest $testid $testdir > $testid.irods 2>&1
		# check for failure
		if [ "$?" -ne 0 ]; then
			echo "$testid FAILED, exiting"
			exit 3
		fi

		i=`expr $i +  1`
		sleep 3

		now=`date`
		echo "Success: Test $testid completed at $now" >> $0.log
		echo "$testid ended Successfully"
	done

	#cp $TMP_DIR/$testid.irods $TMP_DIR/$thistest.log
	#cp $TMP_DIR/$thistest.log $thistest.out

done

exit 0


