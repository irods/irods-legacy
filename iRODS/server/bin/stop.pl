#!/usr/bin/perl
#
# Copyright (c), The Regents of the University of California            ***
# For more information please refer to files in the COPYRIGHT directory ***
#
# Script to shutdown the irods server and agent
#

# Modify this, if needed, to the irodsServer you wish to shutdown
$SERVER_STR="irodsServer";
$AGENT_STR="irodsAgent";
$RE_SERVER_STR="irodsReServer";

$hostOS=`uname -s`;
chomp($hostOS);
$hostUser= `whoami`;
chomp($hostUser);
print "Host User = $hostUser\n";
if ("$hostOS" eq "Darwin" ) {
    $psOptions="-axlw";
    $pidField="2";
}
else {
    $psOptions="-elf";
    $pidField="4";
}
if ("$hostOS" eq "SunOS" ) {
    $psOptions="-el";
    $SERVER_STR="irodsSer";
    $AGENT_STR="irodsAge";
    $RE_SERVER_STR="irodsReS";
}

# find the server (if any)
$serverLine=`ps $psOptions | egrep "$SERVER_STR" | egrep "$hostUser" | egrep -v grep `;
chomp($serverLine);
$serverPid=`echo "$serverLine" | awk '{ print \$$pidField }'`;
chomp($serverPid);
if (!$serverPid) {
    print "There are no irods Servers running\n";
}
else {
    print $serverLine . "\n";
    print "serverPid: " . $serverPid . "\n";

# Find all irodsAgents (if any) that are children of this irodsServer
    $agentsFull=`ps $psOptions | egrep " $serverPid" | egrep $AGENT_STR | egrep -v "$SERVER_STR" | egrep -v grep`;
    chomp($agentsFull);
    $agentPids = `echo "$agentsFull" | awk '{ print \$$pidField }'`;
    chomp($agentPids);
    $agentPids =~ s/\n/ /g;
    print "agentPids: " . $agentPids . "\n";
}

# Find the irodsReServer (if any) 
$REFull=`ps $psOptions | egrep $RE_SERVER_STR | egrep "$hostUser" | egrep -v grep`;
chomp($REFull);
$REPid = `echo "$REFull" | awk '{ print \$$pidField }'`;
chomp($REPid);
$REPid =~ s/\n/ /g;
print $RE_SERVER_STR . "Pid: " . $REPid . "\n";

$any = $agentPids . $REPid . $serverPid;
if ($any) {
printf("Enter y (or yes) to kill these processes:");
    $cmd=<STDIN>;
    chomp($cmd);
    if ($cmd ne "yes" and $cmd ne "y") {
	die("Aborted by user");
    }
}

if ($agentPids) {
    print "killing: " . $agentPids . "\n";
    `/bin/kill $agentPids`;
    $agentsFull=`ps $psOptions | egrep " $serverPid" | egrep $AGENT_STR | egrep -v "$SERVER_STR" | egrep -v grep`;
    chomp($agentsFull);
    $agentPids = `echo "$agentsFull" | awk '{ print \$$pidField }'`;
    chomp($agentPids);
    if ($agentsPids) {
	print "killing again as some remain: " . $agentPids . "\n";
	`/bin/kill $agentPids`;
    }
}

if ($REPid) {
    print "killing: " . $REPid . "\n";
    `/bin/kill $REPid`;
    $REFull=`ps $psOptions | egrep " $serverPid" | egrep $RE_SERVER_STR | egrep -v "$SERVER_STR" | egrep -v grep`;
    chomp($REFull);
    $REPid = `echo "$REFull" | awk '{ print \$$pidField }'`;
    chomp($REPid);
    $REPid =~ s/\n/ /g;
    if ($REPid) {
	print "killing again as it remains: " . $REPid . "\n";
	`/bin/kill $REPid`;
    }
}

# Kill this irods Server
if ($serverPid) {
    print "killing: " . $serverPid . "\n";
    `/bin/kill $serverPid`;
    $serverPid=`ps $psOptions | egrep "$SERVER_STR" | egrep -v grep | egrep "$hostUser" | awk '{ print \$$pidField }'`;
    chomp($serverPid);
    if ($serverPid) {
	print "killing again as it is still going: " . $serverPid . "\n";
	`/bin/kill $serverPid`;
    }
    $serverPid=`ps $psOptions | egrep "$SERVER_STR" | egrep -v grep | egrep "$hostUser" | awk '{ print \$$pidField }'`;
    chomp($serverPid);
    if ($serverPid) {
	die("irodsServer failed to exit\n");
    }
}
exit(0);


