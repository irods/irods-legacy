#!/bin/bash

# Usage is:
#	zone-info [-m |-mf]
#
# This shell script collects and displays some basic information on
# the local iRODS instance.
#
# This should be run from the irods admin account on the ICAT-enabled
# host, with the i-commands in the path and 'iinit' run.
# If run from the build directory, it not need to prompt for the 
# config.mk file location.
#
# Without options: collect and show information about the local
#    iRODS instance.
# Options: -m   email the results to the DICE team, using output 
#               from a previous run if available.
#          -mf  email the results to the DICE team, but always re-running
#               the commands to get current results (f: force).
#

set -e   # exit if anything fails

outFile="/tmp/izoneinfo.txt"
tmpFile="/tmp/zoneInfoTmpFile123"


if [ ! -z "$1" ]; then
    if [ $1 = "-m" ]; then
	if [ ! -f $outFile ]; then
#           run this same command, without without -m and with -quiet 
	    $0 -quiet
#           then email the results
	    mailx -s zone-info schroeder@diceresearch.org < $outFile
	    echo "Thank you for providing this information to DICE"
	    exit 0

	else 
#           email previous run's output
	    mailx -s zone-information schroeder@diceresearch.org < $outFile
	    echo "Thank you for providing this information to DICE"
	    exit 0
	fi
    fi
    if [ $1 = "-mf" ]; then
#       run this same command, without -mf and with -quiet 
	$0 -quiet
#       then email the results
	mailx -s zone-info schroeder@diceresearch.org < $outFile
	echo "Thank you for providing this information to DICE"
	exit 0
    fi
fi

rm -f $outFile

# Look for config.mk starting at this command's path and up one level,
# searching down 2 levels.  If not there, try same with current directory.
# If not there, ask for iRODS dir.

startDir=`echo $0 | sed s/izoneinfo.sh//g`
if [ -z $startDir ]; then
    startDir="./"
fi
maxDepth="-maxdepth 2"
os=`uname -s`
if [ $os = "SunOS" ]; then
    maxDepth=""
fi
config=`find $startDir $maxDepth -name config.mk`
if [ -z $config ]; then
   config=`find $startDir.. $maxDepth -name config.mk`
fi
if [ ! -f $config ]; then
    $startdir=`pwd`
    config=`find $startDir $maxDepth -name config.mk`
    if [ -z $config ]; then
	config=`find $startDir/.. $maxDepth -name config.mk`
    fi
fi
if [ -z $config ]; then
    echo "Could not find config.mk file (near this command or cwd)"
    printf "Please enter the full path of the iRODS build directory:"
    read startDir
    config=`find $startDir $maxDepth -name config.mk`
    if [ -z $config ]; then
	config=`find $startDir/.. $maxDepth -name config.mk`
    fi
fi
if [ -z $config ]; then
    echo "Can not find config.mk"
    exit 1
fi

#
# Do a series of iquest commands to get basic information
#

zoneName=`iquest "%s" "select ZONE_NAME where ZONE_TYPE = 'local'"`
echo "Local zone is: $zoneName" | tee -a $outFile


set +e   # the query will fail if no remote zones are defined
tmp=`iquest "%s" "select ZONE_NAME where ZONE_TYPE = 'remote'" 2> /dev/null`
remoteZoneNames=`echo $tmp | sed 's/------------------------------------------------------------//g' | sed 's/ZONE_NAME = //g'`
set -e   # exit if anything fails
echo "Remote zones are: $remoteZoneNames" | tee -a $outFile

users=`iquest "%s" "select count(USER_ID) where USER_TYPE <> 'rodsgroup'"`
echo "Number of users: $users" | tee -a $outFile

groups=`iquest "%s" "select count(USER_NAME) where USER_TYPE = 'rodsgroup'"`
echo "Number of user groups: $groups" | tee -a $outFile

rescs=`iquest "%s" "select count(RESC_ID)"`
echo "Number of resources: $rescs" | tee -a $outFile

colls=`iquest "%s" "select count(COLL_ID)"`
echo "Number of collections: $colls" | tee -a $outFile

dataObjs=`iquest "%s" "select count(DATA_ID)"`
echo "Number of data-objects: $dataObjs" | tee -a $outFile

rm -rf $tmpFile
dataDistribution=`iquest "%20s bytes in %12s files in '%s'" "SELECT sum(DATA_SIZE),count(DATA_NAME),RESC_NAME" > $tmpFile`
echo "Data distribution by resource:" | tee -a $outFile
cat $tmpFile | tee -a $outFile
rm -rf $tmpFile 

#
# Check the config file for various settings
#
echo Using config file: $config | tee -a $outFile

platform=`grep OS_platform $config | grep = | sed 's/_platform//g'`
echo $platform | tee -a $outFile

set +e
gsi=`grep GSI_AUTH $config | grep = | grep -v Uncomment | grep -v "#"`
set -e
if [ -z "$gsi" ]; then
  echo "no GSI" | tee -a $outFile
else
  echo "GSI enabled" | tee -a $outFile
fi

set +e
krb=`grep KRB_AUTH $config | grep = | grep -v Uncomment | grep -v "#"`
set -e
if [ -z "$krb" ]; then
  echo "no Kerberos" | tee -a $outFile
else
  echo "Kerberos enabled" | tee -a $outFile
fi

set +e
fuse=`grep IRODS_FS $config | grep = | grep -v Uncomment | grep -v "#"`
set -e
if [ -z "$fuse" ]; then
  echo "no FUSE" | tee -a $outFile
else
  echo "FUSE enabled" | tee -a $outFile
fi

modules=`grep MODULES $config | grep = | grep -v "#"`
echo $modules | tee -a $outFile

set +e
pgdb=`grep PSQICAT $config | grep = | grep -v "#"`
oradb=`grep ORAICAT $config | grep = | grep -v "#"`
mysqldb=`grep MYICAT $config | grep = | grep -v "#"`
set -e
if [ ! -z "$pgdb" ]; then
    echo "PostgreSQL ICAT" | tee -a $outFile
fi
if [ ! -z "$oradb" ]; then
    echo "Oracle ICAT" | tee -a $outFile
fi
if [ ! -z "$mysqldb" ]; then
    echo "MySQL ICAT" | tee -a $outFile
fi

#
# Get version, etc, from imiscsvrinfo
#
svrinfo=`imiscsvrinfo`
#printf "SvrInfo:" | tee -a $outFile
echo "SvrInfo:" | tee -a $outFile
echo $svrinfo | tee -a $outFile

#
# And the date/time
#
date | tee -a $outFile

# 
# Request sending this to DICE
if [ -z "$1" ]; then
    echo "To send this information to DICE, please run 'izoneinfo.sh -m'"
fi

exit 0
