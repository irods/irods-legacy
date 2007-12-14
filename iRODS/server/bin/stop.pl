#
# Perl

#
# Shut down the iRODS server and agent.
#
# Usage is:
#	perl stop.pl
#

use File::Spec;
use Cwd;
use Cwd "abs_path";





#
# Design Notes:  deciding what to kill
# 	In the simple (common) case, this host is running one iRODS
# 	server and it's child servers.  Running "ps" and filtering
# 	its output for processes with known server names gets us a
# 	list of process IDs of servers.  Then we kill them.
#
# 	There are several gotchas that this script currently does
# 	not handle:
# 		1.  iRODS servers owned by other users.
# 		2.  iRODS servers with changed process names.
# 		3.  Multiple iRODS servers running, but only want
# 		    to kill one.
#
#	If there are iRODS servers running that are owned by other
#	users, this script will get their process IDs, but killing
#	them will fail.  An error message is output and the script
#	exits with a non-zero exit code.
#
#	If there are iRODS servers with changed process names
#	(possible if they are started from Perl), then this script's
#	filter of "ps" output won't find them.  This script won't
#	be able to kill them and it will output a wrong message that
#	there are no servers running.
#
#	If there are multiple iRODS servers running, this script
#	will try to kill them all.  Currently there is no way to
#	tell this script which ones to kill.  Further, there is
#	no clear way for the user to know which ones to kill
#	based upon port number, or something else.
#
#	These are all design flaws.  There needs to be a way for
#	the iRODS servers to be discovered without doing "ps",
#	which is inherently a weak method.  Once such a way exists,
#	then this script can intelligently choose which servers
#	to kill, and which ones to ignore.
#





# Server names
%servers = (
	# Nice name		process name
	"iRODS agents" =>	"irodsAgent",
	"iRODS rule servers" =>	"irodsReServer",
	"iRODS servers" =>	"irodsServer"
);





# Find the iRODS home directory and load the support script.
#	This script may have been executed from the iRODS home, or
#	from one of its subdirectories.
$IRODS_HOME = cwd( );
my $configFile = File::Spec->catfile( $IRODS_HOME, "config", "utils_platform.pl" );
while ( ! -e $configFile )
{
	my $newHome = abs_path( File::Spec->catdir( $IRODS_HOME, ".." ) );
	if ( $newHome eq $IRODS_HOME )
	{
		print( "Usage problem:\n" );
		print( "    Please run this script from the iRODS home directory.\n" );
		exit( 1 );
	}
	$IRODS_HOME = $newHome;
	$configFile = File::Spec->catfile( $IRODS_HOME, "config", "utils_platform.pl" );
}

require $configFile;





# Find and kill the server process IDs
my $found = 0;
foreach $serverType (keys %servers)
{
	my $processName = $servers{$serverType};
	my @pids = getProcessIds( $processName );
	next if ( $#pids < 0 );
	foreach $pid (@pids)
	{
		$found = 1;
		kill( 'SIGINT', $pid );
	}
}
if ( ! $found )
{
	print( "There are no iRODS servers running.\n" );
	exit( 0 );
}





# Repeat to catch stragglers.  This time use kill -9.
foreach $serverType (keys %servers)
{
	my $processName = $servers{$serverType};
	my @pids = getProcessIds( $processName );
	next if ( $#pids < 0 );
	foreach $pid (@pids)
	{
		kill( 9, $pid );
	}
}





# Report if there are any left.
my $didNotDie = 0;
foreach $serverType (keys %servers)
{
	my $processName = $servers{$serverType};
	@pids = getProcessIds( $processName );
	if ( $#pids >= 0 )
	{
		$didNotDie = 1;
	}
}
if ( $didNotDie )
{
	print( "Some servers could not be killed.  They may be owned\n" );
	print( "by another user or there could be a problem.\n" );
	exit( 1 );
}

# Done
exit( 0 );
