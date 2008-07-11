#!/bin/bash -e

# SDL Usage # concurrent tests
# 

#set -x 

verboseflag=1
numoferrors=1
OS=Darwin


TMP_DIR=TMP
thistest=putget
#testdirs="zerofiles smallfiles bigfiles"
testdirs="zerofiles"

irodshome="../../"

numzerofiles=100
numsmallfiles=100
numbigfiles=10

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
			for ((i=1;i<=$numzerofiles;i+=1)); do
				touch zerofiles/zerofile$i
			done
		;;

		"smallfiles")
			echo "making smallfiles"
			for ((i=1;i<=$numsmallfiles;i+=1)); do
				echo "abcdefghijklmnopqrstuvwxyz1234567890" > smallfiles/smallfile$i
			done
		;;

		"bigfiles")
			cd src; make clean; make; cd ..
			echo "making bigfiles"
			$thisdir/src/writebigfile
			for ((i=1;i<=$numbigfiles;i+=1)); do
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
		
		case "$testdir" in
		"zerofiles")
			testid=$thistest-$numzerofiles-$testdir-`date "+%Y%m%d%H%M%S"`
		;;
		"smallfiles")
			testid=$thistest-$numsmallfiles-$testdir-`date "+%Y%m%d%H%M%S"`
		;;
		"bigfiles")
			testid=$thistest-$numbigfiles-$testdir-`date "+%Y%m%d%H%M%S"`
		;;

		esac;
		
		numtest=`expr $i + 1`
		echo $numtest of $1:$thistest $testid
		sh -ex $irodshome/clients/concurrent-test/$thistest $testid $testdir > $testid.irods 2>&1
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

done

#Clean up locally
for testdir in $testdirs; do
	/bin/rm -rf $testdir
done



exit 0


