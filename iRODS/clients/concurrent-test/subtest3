#!/bin/csh -ex

#####################################################################################
#  test3, uses some of the srb commands on Tickets, in order to see if they are all 
#  working according to their design purpose.
#  
#  Unless otherwise stated, all commands with # in front of them, don't seem to be 
#  working properly.
#
#  By: Arcot Rajasekar
#  Modified By: Roman Olschanowsky
#####################################################################################

storeset=`cat ResourcesToTest`
HOST=`cat $HOME/.srb/.MdasEnv | grep srbHost | awk '{print substr($0,9)}'`

if [ "$OS" = "Windows_NT" ]
then
  binDir=../../NT/scommands/Debug
else
  binDir=../../utilities/bin
fi

DATA_FILES="data_1 data_2 data_3 data_4 datadb1.xml dataempty"
TMP_DIR=../TMP
i=1

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
#  Make new test directory in srb space.
#####################################################################################

$binDir/Smkdir Stest3Dir$testid
$binDir/Scd Stest3Dir$testid
for store1 in $storeset ; do
  $binDir/Smkdir S_sub1
  $binDir/Sls
  $binDir/Scd S_sub1

#####################################################################################
#  Put local data files into newly created directories in srb space, and assign each
#  of them a ticket. Then get info on tickets.
#####################################################################################
  for file in $DATA_FILES ; do
      $binDir/Sput -S $store1 $file "$file&COPIES=all"
      $binDir/Sticket -F $TMP_DIR/roman$i.$testid -N 20 -D $file
#      $binDir/Stls -F $TMP_DIR/roman$i.$testid -H $HOST
      $binDir/Stls -F $TMP_DIR/roman$i.$testid
#      $binDir/SgetT -u -F $TMP_DIR/roman$i.$testid
      $binDir/SgetT -Y 0 -F $TMP_DIR/roman$i.$testid
      i=`expr $i + 1`
  done

  $binDir/Scd ..

#####################################################################################
#  Display various information about objects in srb space
#####################################################################################
  $binDir/SgetD  -Y 0 -L 500 "S_sub1/*"
  $binDir/Sls -lr

  $binDir/Smkdir S_sub2

#####################################################################################
#  Move all files from Sub1 to Sub2
#####################################################################################
  for file in $DATA_FILES ; do
      $binDir/Smv S_sub1/$file S_sub2
  done

  $binDir/Sls -lr

#####################################################################################
#  Remove a ticket, and test if tickets are working properly.
#####################################################################################
  $binDir/Sls -lr
#  $binDir/SgetD  -Y 0 -L 200 "S_sub1/*"
  $binDir/SgetD  -Y 0 -L 200 "S_sub2/*"

  i=1
  j=1

  while [ $j -lt 4 ]
  do
if [ $j -eq 3 ]
then  $binDir/Srmticket -F $TMP_DIR/roman2.$testid
fi

    for file in $DATA_FILES ; do
#      $binDir/Stcat -T $TMP_DIR/roman$i.$testid $HOST > $TMP_DIR/$file.$testid.tmp
      $binDir/Stcat -T $TMP_DIR/roman$i.$testid > $TMP_DIR/$file.$testid.tmp
      wc $file $TMP_DIR/$file.$testid.tmp
      rm $TMP_DIR/$file.$testid.tmp
      i=`expr $i + 1`
    done
    i=1
    j=`expr $j + 1`
  done

#####################################################################################
#  Do all the clean up, remove data files and directories
#####################################################################################
  rm $TMP_DIR/roman?.$testid
  $binDir/Srmdir S_sub1
  $binDir/Scd S_sub2
  $binDir/Srm "data*"
  $binDir/Scd ..
  $binDir/Srmdir S_sub2
  $binDir/Sls -lr
done

#####################################################################################
#  Remove test directory and end session
#####################################################################################
$binDir/Scd ..
$binDir/Srmdir Stest3Dir$testid
$binDir/Srmtrash
$binDir/Sexit
exit 0
