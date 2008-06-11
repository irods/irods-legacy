#!/bin/bash -e

# SDL Usage # concurrent tests
# 

verboseflag=1
numoferrors=1
OS=Darwin

TMP_DIR=TMP
# putget is the shell script this script calls
thistest=putget
testdirs="zerofiles smallfiles bigfiles"

usage () {
	echo "Usage: $0 <numtests>"
	exit 1
}

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
					## SDL this needs to be changed
					cat /mach_kernel >> bigfiles/bigfile$i
				done
			done
		;;
	esac

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
		sh -e $thistest $testid $testdir > $testid.irods 2>&1
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


