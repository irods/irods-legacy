#!/bin/bash -e

# SDL Usage # concurrent tests
# 

set -x 

verboseflag=1
numoferrors=1
OS=Darwin

TMP_DIR=TMP
thistest=putget
testdirs="zerofiles smallfiles bigfiles"

thisdir=`pwd`
echo thisdir: $thisdir


usage () {
	echo "Usage: $0 <numtests>"
	exit 1
}

# Make data directories of varying file numbers and sizes
makefiles () {

	for dir in $testdirs; do
		test -d $dir || mkdir $dir

	case "$dir" in
		"zerofiles")
			echo "making zerofiles"
			for ((i=1;i<=100;i+=1)); do
				touch zerofiles/zerofile$i
			done
		;;

		"smallfiles")
			echo "making smallfiles"
			for ((i=1;i<=10;i+=1)); do
				echo "abcdefghijklmnopqrstuvwxyz" > smallfiles/smallfile$i
			done
		;;

		"bigfiles")
			cd src; make clean; make; cd ..
			echo "making bigfiles"
			$thisdir/src/writebigfile
			for ((i=1;i<=1;i+=1)); do
				cp bigfile bigfiles/bigfile$i				
			done
			/bin/rm bigfile
		;;
	esac

	done
}

echo "beginning poundtest"

if [ "$1" = "" ]; 
then 
	usage 
fi

# make our test directories
makefiles

for testdir in $testdirs; do

	i=0
	while [ $i -lt $1 ]; do
		testid=$thistest-$testdir-`date "+%Y%m%d%H%M%S"`
		numtest=`expr $i + 1`
		echo $numtest of $1:$thistest $testid
		sh -ex $thistest $testid $testdir > $testid.irods 2>&1
		# check for failure
		if [ "$?" -ne 0 ]; then
			echo "$testid FAILED, exiting"
			exit 3
		fi

		i=`expr $i +  1`
		numtest=`expr $i +  1`
		sleep 3

		now=`date`
		echo "Success: Test $testid completed at $now" >> $0.log
		echo "$testid ended Successfully"
	done

	#cp $TMP_DIR/$testid.irods $TMP_DIR/$thistest.log
	#cp $TMP_DIR/$thistest.log $thistest.out

done

exit 0


