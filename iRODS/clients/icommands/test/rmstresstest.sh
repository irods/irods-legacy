#!/bin/csh -e
set binDir = ../bin
set defResc = demoResc
if ("$2" == "" ) then
 echo "Usage: $0 <coll-start-number> <coll-count-number>"
 echo "  each collection will be named sttst.num and will have the"
 echo "  contents of the directory stresstest.dir which has 1000 files"
 echo " run this from icommands/test"
 echo " needs */home/*/stresstest collection"
 exit 1
endif

set j = `expr $1 +  $2`
set i = $1

$binDir/icd
$binDir/ils
$binDir/icd stresstest
$binDir/ipwd
echo "------------------"
set startd = `date`
while ( $i < $j )
   echo "Timing irm -rf sttst.$i   --- 1000 files"
   time $binDir/irm -rf sttst.$i
   set i = `expr $i +  1`
end
$binDir/icd
echo "Size with ils -lr stresstest"
$binDir/ils -lr stresstest |wc
echo "Timing ils -lr stresstest"
time $binDir/ils -lr stresstest >/dev/null
echo "Timing irm -rf stresstest"
time $binDir/irm -rf stresstest
echo "------------------"
set endd = `date`
echo "Start: $startd"
echo "End  : $endd"
exit 0
