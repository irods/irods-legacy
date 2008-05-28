#!/bin/sh

# usage: sh commandsuite.sh [m] [num]    
# m=mute
# num=number of times to run
# this is messy let's get rid of this for now and just run some tests

echo "SDL Starting iRODS test suite"

## SDL we need to check and see if iRODS is even running


# These are the three sub test scripts this script runs

numoferrors=1 
verboseFlag=1
binDir=../utilities/bin
TMP_DIR=TMP
# where is $OS set?
OS=Darwin

echo "$OS = " $OS

#testscripts="putget.sh test2.sh test3.sh"
testscripts="putget.sh"

# SDL test for pre-existence of this directory
mkdir $TMP_DIR

for testscript in $testscripts ; do

	if [ $verboseFlag -eq 1 ]; then
  		echo "Starting $testscript..."
	fi

	testid=$testscript-`date "+%Y%m%d%H%M%S"`
	echo testid = $testid

	sh -ex $testscript $testid 2>&1 | tee $TMP_DIR/$testscript.$testid.irods
	cp $TMP_DIR/$testscript.$testid.irods $TMP_DIR/$testscript.log

	# Make new comparison file
	cp $TMP_DIR/$testscript.log $testscript.result.out
	sh -ex $testscript.sh $testid > $TMP_DIR/$testscript.$testid.irods 2>&1
	cp $TMP_DIR/$testscript.$testid.irods $TMP_DIR/$testscript.log

	# compare results

	echo "Starting mydiff $testscript.result.out $TMP_DIR/$testscript.$testid.irods"

	wc $testscript.result.out $TMP_DIR/$testscript.$testid.irods
	./mydiff $testscript.result.out $TMP_DIR/$testscript.$testid.irods $numoferrors

done


# now clean up locally

exit 0



