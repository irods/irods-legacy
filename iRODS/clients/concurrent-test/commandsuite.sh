#!/bin/sh 

# SDL determine usage()
# right now only verboseflag can be set by user
# right now hardcoded to 1

echo "SDL Starting iRODS concurrent test suite"

## SDL we need to check and see if iRODS is even running


numoferrors=1 
verboseFlag=1
binDir=../utilities/bin
TMP_DIR=TMP
# where is $OS set?
OS=Darwin

testid=concurrenttest-`date "+%Y%m%d%H%M%S"`

# These are the three sub test scripts this script runs
#testscripts="putget.sh test2.sh test3.sh"
testscripts="subtest1"

if [ ! -d $TMP_DIR ]; then
mkdir $TMP_DIR
fi

for testscript in $testscripts ; do

	if [ $verboseFlag -eq 1 ]; then
  		echo "Starting $testscript..."
	fi

	subtestid=$testid-$testscript

	# Run the subtest
	sh -ex $testscript $subtestid > $TMP_DIR/$subtestid.irods 2>&1
	# sh -ex $testscript $subtestid 2>&1 | tee $TMP_DIR/$subtestid.irods
	cp $TMP_DIR/$subtestid.irods $TMP_DIR/$subtestid.log
	cp $TMP_DIR/$subtestid.log $subtestid.result.out

	# forget about mydiff for now - let's just get the skeleton working
	# SDL what the heck is going on here?

	# compare results
	#echo "Starting mydiff $testscript.result.out $TMP_DIR/$testid.irods"
	wc $subtestid.result.out $TMP_DIR/$subtestid.irods
	./mydiff $subtestid.result.out $TMP_DIR/$subtestid.irods 


done


# SDL now clean up locally

exit 0



