#
# Perl

# Start the iRODS server.
#
# Usage is:
#	perl start.pl
#

use File::Spec;
use Cwd;
use Cwd "abs_path";





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





# Make sure the server is available
my $serverBinDir = File::Spec->catdir( $IRODS_HOME, "server", "bin" );
my $irodsServer = File::Spec->catfile( $serverBinDir, "irodsServer" );
if ( ! -e $irodsServer )
{
	print( "Usage problem:\n" );
	print( "    The 'irodsServer' application could not be found.  Have the\n" );
	print( "    iRODS servers been compiled?\n" );
	exit( 1 );
}




# Prepare
chdir( $serverBinDir );
umask( 077 );

# Directory containing server configuration 'server.config'.
$irodsConfigDir = File::Spec->catdir( $IRODS_HOME, "server", "config" );

# Directory for the server log.
$irodsLogDir    = File::Spec->catdir( $IRODS_HOME, "server", "log" );

# Optional iRODS environment override.  By default, the user's file
# in '~/.irods/.irodsEnv' is used.
# $irodsEnvFile = "/tmp/.irodsEnv";

# irodsPort defines the port number the irodsServer is listening on.
# The default port number is set in the .irodEnv file of the irods
# admin starting the server. The irodsPort value set here (if set)
# overrides the value set in the .irodEnv file.
# $irodsPort = "5678";

# spLogLevel defines the verbosity level of the server log. The levels are:
#	9-LOG_SQL
#	8-LOG_SYS_FATAL
#	7-LOG_SYS_WARNING
#	6-LOG_ERROR
#	5-LOG_NOTICE
#	4-LOG_DEBUG
#	3-LOG_DEBUG3
#	2-LOG_DEBUG2
#	1-LOG_DEBUG1.
# The lower the level, the more verbose is the log. The default level is 5-LOG_NOTICE
# $spLogLevel = "3";

# spLogSql defines if sql will be logged or not.  The default is no logging.
# $spLogSql = "1";

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
$reServerOnIes = 1;

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

				  $ENV{'irodsConfigDir'}      = $irodsConfigDir;
if ($irodsEnvFile)		{ $ENV{'irodsEnvFile'}        = $irodsEnvFile; }
if ($irodsPort)			{ $ENV{'irodsPort'}           = $irodsPort; }
if ($spLogLevel)		{ $ENV{'spLogLevel'}          = $spLogLevel; }
if ($spLogSql)			{ $ENV{'spLogSql'}            = $spLogSql; }
if ($svrPortRangeStart)		{ $ENV{'svrPortRangeStart'}   = $svrPortRangeStart; }
if ($svrPortRangeEnd)		{ $ENV{'svrPortRangeEnd'}     = $svrPortRangeEnd; }
if ($reServerOnIes)		{ $ENV{'reServerOnIes'}       = $reServerOnIes; }
if ($reServerOnThisServer)	{ $ENV{'reServerOnThisServer'}= $reServerOnThisServer; }
if ($reServerOption)		{ $ENV{'reServerOption'}      = $reServerOption; }
if ($svrPortReconnect)		{ $ENV{'svrPortReconnect'}    = $svrPortReconnect; }
if ($RETESTFLAG)		{ $ENV{'RETESTFLAG'}          = $RETESTFLAG; }




# Start the server
system( $irodsServer );
if ( $? )
{
	print( "iRODS server failed to start\n" );
	exit -1;
}

@pids = getProcessIds( "irodsServer" );
if ( $#pids < 0 )
{
	print( "iRODS server failed to start.\n" );
	exit( -1 );
}
foreach $pid (@pids)
{
	print( "Process $pid.  iRODS server is running.\n" );
}

exit( 0 );
