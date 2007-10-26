#!/usr/bin/perl

# Copyright (c), The Regents of the University of California            ***
# For more information please refer to files in the COPYRIGHT directory ***

# Start the irodsServer (which will start the agents).
# The command line argument, if provided, is the irods server bin directory,
# and if not provided it is assumed to be the current directory.
# If irodsServer is not in that directory, then the location of this
# script is used.


($arg1)=@ARGV;
if ($arg1) {
    $serverBinDir = $arg1;
    chdir "$arg1" || die "Can't chdir to input dir: $arg1";
}
else {
    $serverBinDir = `pwd`;
    chomp($serverBinDir);
}

# If the irodsServer is not in the provided serverBinDir then try
# the directory that this script resides in.
if (!-e "$serverBinDir/irodsServer") {
    $oldSvr="$serverBinDir/irodsServer";
    $startDir=`pwd`;
    chomp($startDir);
    if (index($0, "/") eq 0) {
	$tmp= $0;
    }
    else {
	$tmp=$startDir . "/" . $0;
	chomp($tmp);
    }
    $i = rindex($tmp, "/");
    $serverBinDir = substr($tmp, 0, $i);
    if (!-e "$serverBinDir/irodsServer") {
	print "Cannot find irodsServer\n";
	print $oldSvr . " does not exist and\n";
	print "$serverBinDir/irodsServer" . " does not exist\n";
	exit -1;
    }
    chdir "$serverBinDir" || die "Can't chdir to: $serverBinDir";
}

# This is the umask for the server operation.
umask 077;

# irodsConfigDir is the irods server configuration directory where 
# the server configuration files are located. The default
# is "../config". 
$irodsConfigDir=$serverBinDir . "/../config";
$irodsLogDir=$serverBinDir . "/../log";

# irodsEnvFile - The path for the .irodsEnv file which contains the Env info
# of the user starting the irodsServer. The default is ~/.irods/.irodsEnv.
# $irodsEnvFile=/tmp/.irodsEnv;

# irodsPort defines the port number the irodsServer is listening on.
# The default port number is set in the .irodEnv file of the irods
# admin starting the server. The irodsPort value set here (if set)
# overrides the value set in the .irodEnv file.
#
# $irodsPort=5678;

# spLogLevel defines the verbosity level of the server log. The levels are:
# 9-LOG_SQL, 8-LOG_SYS_FATAL, 7-LOG_SYS_WARNING, 6-LOG_ERROR, 5-LOG_NOTICE,
# 4- LOG_DEBUG, 3-LOG_DEBUG3, 2-LOG_DEBUG2, 1-LOG_DEBUG1. The lower the
# level, the more verbose is the log. The default level is 5-LOG_NOTICE
# $spLogLevel=3;

# spLogSql defines if sql will be logged or not.
# the default value is no logging.
# $spLogSql=1;

# svrPortRangeStart and svrPortRangeEnd - A range of port numbers can be 
# specified for the server's parallel I/O communication port. 
# svrPortRangeStart specifies the first allowable port number and 
# svrPortRangeEnd specifies the end of the range. 
# $svrPortRangeStart=20000
# $svrPortRangeEnd=20199;

# reServerOnIes - Specifies that the delayed rule exec server (irodsReServer) 
# to be run on the same host as the IES (ICAT enabled server). Please note that
# only one irodsReServer should be run in each zone. reServerOnIes is on
# by default
$reServerOnIes=1;

# reServerOnThisServer - Specifies that the delayed rule exec server 
# (irodsReServer) to be run on this server. This allows the irodsReServer
# to run on a non-IES host. reServerOnThisServer is off by default
# $reServerOnThisServer=1;

# reServerOption - Addition option for irodsReServer. By default, irodsReServer
# runs with no option. The -v option specifies the verbose mode. The -D option
# specifies the log directory for this server if the default log directory is 
# not desirable. 
# $reServerOption="-cD /a/b/c/myLogDir";

# svrPortReconnect - Specifies whether the agent will create a reconnect
# socket/port for reconnection in case the client server connection is
# broken due to timeout or other reason. The default is on. 
$svrPortReconnect=1;

# RETESTFLAG - option for logging
# use 1 to make it log
# $RETESTFLAG=1;

$ENV{'irodsConfigDir'}=$irodsConfigDir;
$ENV{'irodsLogDir'}=$irodsLogDir;
if ($irodsEnvFile)      { $ENV{'irodsEnvFile'}=$irodsEnvFile; }
if ($irodsPort)         { $ENV{'irodsPort'}=$irodsPort; }
if ($spLogLevel)        { $ENV{'spLogLevel'}=$spLogLevel; }
if ($spLogSql)        { $ENV{'spLogSql'}=$spLogSql; }
if ($svrPortRangeStart) { $ENV{'svrPortRangeStart'}=$svrPortRangeStart; }
if ($svrPortRangeEnd)   { $ENV{'svrPortRangeEnd'}=$svrPortRangeEnd; }
if ($reServerOnIes)   { $ENV{'reServerOnIes'}=$reServerOnIes; }
if ($reServerOnThisServer)   { $ENV{'reServerOnThisServer'}=$reServerOnThisServer; }
if ($reServerOption)   { $ENV{'reServerOption'}=$reServerOption; }
if ($svrPortReconnect)  {$ENV{'svrPortReconnect'}=$svrPortReconnect; }
if ($RETESTFLAG)  {$ENV{'RETESTFLAG'}=$RETESTFLAG; }

print "Starting: $serverBinDir/irodsServer\n";
`$serverBinDir/irodsServer`;
$status=$?;
if ($status) {
    print "irodsServer failed to start\n";
    exit -1;
}
print "Sleeping 2 seconds\n";
sleep 2;
$curLogFile=`ls -1tr $irodsLogDir|grep rodsLog|tail -1`;
#print $curLogFile;
$tailLog=`tail $irodsLogDir/$curLogFile`;
print "tail of current log file:\n" . $tailLog;
$hostOS=`uname -s`;
chomp($hostOS);
if ("$hostOS" eq "Darwin") {
    $ps=`ps -ax | grep -v grep | grep irodsServer`;
}
else {
    $ps=`ps -ef | grep -v grep | grep irodsServer`;
}
print "\nps of running servers:\n" . $ps;


