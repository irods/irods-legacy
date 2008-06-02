#!/bin/sh -x

# SDL Usage
# Usage right now: no parameters, verbose set to 1 by default
# numoferrors set to 1 by default right now also

verboseflag=1
numoferrors=1
OS=Darwin

TMP_DIR=TMP
thistest=putget

testid=$thistest-`date "+%Y%m%d%H%M%S"`

echo "Starting $thistest $testid ... "

sh -ex $thistest $testid > $TMP_DIR/$thistest.$testid.irods 2>&1

# check for failure
if [ "$?" -ne 0 ]; then
	echo "$thistest script FAILED, exiting"
	exit 3
fi

cp $TMP_DIR/$thistest.$testid.irods $TMP_DIR/$thistest.log
cp $TMP_DIR/$thistest.log $thistest.out


wc $thistest.out $TMP_DIR/$thistest.$testid.irods

# now clean up

#echo "Performing rm $TMP_DIR/test1.$testid.irods"
#rm $TMP_DIR/$thistest.$testid.irods
#echo "Successfull rm"
now=`date`
echo "Success: Test $testid completed at $now" >> $thistest.log
echo "Runtest1 ended Successfully"
exit 0


