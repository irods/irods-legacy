#!/bin/sh -ex

#####################################################################################
#  test2, uses some of the srb commands on srb Objects inside of containers, in order 
#  to see if they are all working according to their design purpose.
#  
#  Unless otherwise stated, all commands with # in front of them, don't seem to be 
#  working properly.
#
#  By: Arcot Rajasekar
#  Modified By: Roman Olschanowsky
#  More modifications to irods: Susan Lindsey
#####################################################################################

storeset=`cat ResourcesToTest`


DATA_FILES="data_1 data_2 data_3 data_4 datadb1.xml dataempty"
TMP_DIR=../TMP

#####################################################################################
#  Make unique names for directories using current date and time
#####################################################################################

if [ "$1" = "" ]
then testid=`date "+%Y%m%d%H%M%S"`
else testid=$1
fi

testdate=`date "+%Y-%m-%d"`
echo "TESTID = $testid"
echo "TESTDATE = $testdate"

#####################################################################################
#  Initialize srb client environment, and get User information
#####################################################################################
$binDir/Sinit -v
$binDir/SgetU -d -Y 0

#####################################################################################
#  Make new test directory in srb space
#####################################################################################

$binDir/imkdir Itest2Dir$testid
$binDir/icd Itest2Dir$testid
for store1 in $storeset ; do
  $binDir/imkdir Isub1
  $binDir/ils -lr
  $binDir/icd Isub1

#####################################################################################
#  Put local data files into newly created container in srb space.
#####################################################################################

  $binDir/Smkcont -S $store1 Container$testid

  for file in $DATA_FILES ; do
      $binDir/Sput -c Container$testid -S $store1 $file "$file&COPIES=all" 
  done

  $binDir/Scd ..

# Raja doesn't want me to use Srename, he is having problems with it
# So I'll try move  $binDir/Srename -c Container$testid Container.2.$testid

# Doesn't work either  $binDir/Smv Container$testid Container.2.$testid

# Currently we can not move or rename a container

  $binDir/Slscont Container$testid
#takes too long  $binDir/Slscont -l

#####################################################################################
#  Display various information about objects in srb space
#####################################################################################
  $binDir/SgetD  -Y 0 -L 500 "Isub1/*"
  $binDir/ils -lr

#####################################################################################
#  Get all data files from srb space, store them in temp directory, and compare them
#  with another copy of same file to see if they match.
#####################################################################################
  for file in $DATA_FILES ; do
      $binDir/iget Isub1/$file $TMP_DIR/$file.$testid.tmp
      diff $file $TMP_DIR/$file.$testid.tmp
      wc $file $TMP_DIR/$file.$testid.tmp
      rm $TMP_DIR/$file.$testid.tmp
  done

  $binDir/Smkdir Isub2g

#####################################################################################
#  Move all files from Sub1 to Sub2
#####################################################################################
  for file in $DATA_FILES ; do
     $binDir/imv Isub1/$file Isub2g
  done

  $binDir/ils -lr
 
#####################################################################################
#  Copy all data across different platforms, then get data to local environment, and
#  compare with local copies of same data.
#####################################################################################

  for store in $storeset ; do
    $binDir/Smkcont -S $store1 Container$testid.$store

    for file in $DATA_FILES ; do
       $binDir/icp -c Container$testid.$store -S $store I_sub2/$file I_sub1/$file.$store
    done

    $binDir/Ssyncont Container$testid.$store

    $binDir/Sls -lr
#takes too long    $binDir/Slscont 

    for file in $DATA_FILES ; do
      $binDir/Sget I_sub1/$file.$store $TMP_DIR/$file.$testid.tmp
      diff $file $TMP_DIR/$file.$testid.tmp
      wc $file $TMP_DIR/$file.$testid.tmp
      rm $TMP_DIR/$file.$testid.tmp   
    done

    $binDir/Scd I_sub1
    $binDir/Srm "data*"
    $binDir/Srmtrash
    $binDir/Scd ..
    $binDir/Srmcont Container$testid.$store
  done

#####################################################################################
#  Do all the clean up, remove data files, directories, and containers
#####################################################################################
  $binDir/irm -r Isub1
  $binDir/irmtrash
  $binDir/Srmcont Container$testid
#  $binDir/Slscont "Container$testid*"
  $binDir/ils -lr
done

#####################################################################################
#  Remove test directory and end session
#####################################################################################
$binDir/icd ..
$binDir/irm Stest2Dir$testid
$binDir/irmtrash
$binDir/iexit
exit 0
