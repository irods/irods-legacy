#
#
# This Perl script controls the iRODS servers.
#
# Usage is:
#	perl irodsctl.pl [options] [commands]
#
# Help options:
#	--help      Show a list of options and commands
#
# Verbosity options:
# 	--quiet     Suppress all messages
# 	--verbose   Output all messages (default)
#
# Commands:
# 	start       Start the iRODS and Postgres servers
# 	stop        Stop the iRODS and Postgres servers
#	restart     Stop and start the iRODS and Postgres servers
#
#	status      Report the running status of iRODS and Postgres servers
#	test        Test the iRODS installation
#
#	optimize    Optimize the Postgres tables used by iRODS
#	vacuum      Same as 'optimize'
#
#	icatcreate  Create the database tables
#	icatdrop    Delete the database tables
#	initialize  Initialize iRODS (typically only on fresh install)
#

use File::Spec;
use File::Copy;
use Cwd;
use Cwd "abs_path";
use Config;





########################################################################
#
# Confirm execution from the top-level iRODS directory.
#
$IRODS_HOME = cwd( );	# Might not be actual iRods home.  Fixed below.

# Where is the configuration directory for iRODS?  This is where
# support scripts are kept.
$configDir = File::Spec->catdir( $IRODS_HOME, "config" );
if ( ! -e $configDir )
{
	# Configuration directory does not exist.  Perhaps this
	# script was run from the config or install subdirectories.
	# Look up one directory.
	$IRODS_HOME = File::Spec->updir( );
	$configDir  = File::Spec->catdir( $IRODS_HOME, "config" );
	if ( ! -e $configDir )
	{
		# Nope.  Complain.
		print( "Usage error:\n" );
		print( "    Please run this script from the top-level directory\n" );
		print( "    of the iRODS distribution.\n" );
		exit( 1 );
	}
}

# Make the $IRODS_HOME path absolute.
$IRODS_HOME = abs_path( $IRODS_HOME );
$configDir  = abs_path( $configDir );





########################################################################
#
# Initialize.
#

# Get the script name.  We'll use it for some print messages.
my $scriptName = $0;

# Load support scripts.
my $perlScriptsDir = File::Spec->catdir( $IRODS_HOME, "scripts", "perl" );
require File::Spec->catfile( $perlScriptsDir, "utils_paths.pl" );
require File::Spec->catfile( $perlScriptsDir, "utils_print.pl" );
require File::Spec->catfile( $perlScriptsDir, "utils_file.pl" );
require File::Spec->catfile( $perlScriptsDir, "utils_platform.pl" );
require File::Spec->catfile( $perlScriptsDir, "utils_config.pl" );

# Get the path to Perl.  We'll use it for running other Perl scripts.
my $perl = $Config{"perlpath"};
if ( !defined( $perl ) || $perl eq "" )
{
	# Not defined.  Find it.
	$perl = findCommand( "perl" );
}

# Determine the execution environment.
my $thisUserID = $<;
my $thisHost   = getCurrentHostName( );

# Set the number of seconds to sleep after starting or stopping the
# database or iRODS servers.  This gives them time to start or stop before
# we do anything more.  The actual number here is a guess.  Different
# CPU speeds, host loads, or database versions may make this too little
# or too much.
my $databaseStartStopDelay = 4;		# Seconds





########################################################################
#
# Load and validate irods.config.
#
if ( loadIrodsConfig( ) == 0 )
{
	# Configuration failed to load or validate.  An error message
	# has already been output.
	exit( 1 );
}





########################################################################
#
# Set common paths.
#
# Directory containing the servers
$serverBinDir = File::Spec->catdir( $IRODS_HOME, "server", "bin" );

# The iRODS server
$irodsServer  = File::Spec->catfile( $serverBinDir, "irodsServer" );

# Directory containing server configuration 'server.config'.
$irodsServerConfigDir = File::Spec->catdir( $IRODS_HOME, "server", "config" );

# Directory for the server log.
$irodsLogDir    = File::Spec->catdir( $IRODS_HOME, "server", "log" );

# iRODS Server names
%servers = (
	# Nice name		process name
	"iRODS agents" =>	"(irodsAge)(nt)?",
	"iRODS rule servers" =>	"(irodsReS)(erver)?",
	"iRODS servers" =>	"(irodsSer)(ver)?"

	# Process names need to be able to match on just 8 characters,
	# due to limitations of SysV /bin/ps (such as Solaris) which
	# only shows the 1st 8 characters.
);






########################################################################
#
# Overrides for sysadmins that edit this file.
#
# While we encourage you to use the defaults, you can override them
# here and they'll be used the next time the iRODS servers are started
# using this script.
#
# By default, the user's file in '~/.irods/.irodsEnv' is used.  Override
# it by setting a file name here.
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

				  $ENV{'irodsConfigDir'}      = $irodsServerConfigDir;
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





########################################################################
#
# Check usage.
#	Command-line options select whether to be verbose or not.
#
#	Command-line commands tell us what to do.  The main commands
#	start and stop the servers.
#
$scriptName = $0;
setPrintVerbose( 1 );

# Make sure we are not being run as root.
if ( $thisUserID == 0 )
{
	printError( "Usage error:\n" );
	printError( "    This script should *not* be run as root.\n" );
	exit( 1 );
}

$numberCommands = 0;
$doTestExitValue=0;
foreach $arg (@ARGV)
{
	# Options
	if ( $arg =~ /^-?-?h(elp)$/ )		# Help / usage
	{
		printUsage( );
		exit( 0 );
	}
	if ( $arg =~ /^-?-?q(uiet)$/ )		# Suppress output
	{
		setPrintVerbose( 0 );
		next;
	}
	if ( $arg =~ /^-?-?v(erbose)$/ )	# Enable output
	{
		setPrintVerbose( 1 );
		next;
	}



	# Database commands
	if ( $arg =~ /^-?-?(dbstart)$/i )	# Start database
	{
		$numberCommands++;
		printSubtitle( "Starting database server...\n" );
		if ( ! $controlDatabase )
		{
			printError(
				"    The database is not configured for exclusive use by iRODS and\n",
				"    cannot be started by this script.\n" );
		}
		else
		{
			startDatabase( );
		}
		next;
	}
	if ( $arg =~ /^-?-?(dbstop)$/i )	# Stop database
	{
		$numberCommands++;
		printSubtitle( "Stopping database server...\n" );
		if ( ! $controlDatabase )
		{
			printError(
				"    The database is not configured for exclusive use by iRODS and\n",
				"    cannot be stopped by this script.\n" );
		}
		else
		{
			stopDatabase( );
		}
		next;
	}
	if ( $arg =~ /^-?-?(dbrestart)$/i )	# Restart database
	{
		$numberCommands++;
		if ( ! $controlDatabase )
		{
			printError(
				"    The database is not configured for exclusive use by iRODS and\n",
				"    cannot be restarted by this script.\n" );
		}
		else
		{
			printSubtitle( "Stopping database server...\n" );
			stopDatabase( );
			printSubtitle( "Starting database server...\n" );
			startDatabase( );
		}
		next;
	}
	if ( $arg =~ /^-?-?((opt(imize)?)|(vac(uu?m)))$/i ) # Optimize tables
	{
		$numberCommands++;
		doOptimize( );
		next;
	}
	if ( $arg =~ /^-?-?(drop)$/ )	# Drop iCAT tables from database
	{
		$numberCommands++;
		doDrop( );
		next;
	}



	# iRODS commands
	if ( $arg =~ /^-?-?(istart)$/i )	# Start iRODS
	{
		$numberCommands++;
		printSubtitle( "Starting iRODS server...\n" );
		startIrods( );
		next;
	}
	if ( $arg =~ /^-?-?(istop)$/i )		# Stop iRODS
	{
		$numberCommands++;
		printSubtitle( "Stopping iRODS server...\n" );
		stopIrods( );
		next;
	}
	if ( $arg =~ /^-?-?(irestart)$/i )	# Restart iRODS
	{
		$numberCommands++;
		printSubtitle( "Stopping iRODS server...\n" );
		stopIrods( );	# OK to fail.  Server might not be started
		printSubtitle( "Starting iRODS server...\n" );
		startIrods( );
		next;
	}



	# Commands
	if ( $arg =~ /^-?-?(start)$/i )		# Start database and iRODS
	{
		$numberCommands++;
		if ( $controlDatabase )
		{
			printSubtitle( "Starting database server...\n" );
			startDatabase( ) || exit( 1 );
		}
		printSubtitle( "Starting iRODS server...\n" );
		startIrods( );
		next;
	}
	if ( $arg =~ /^-?-?(stop)$/i )		# Stop database and iRODS
	{
		$numberCommands++;
		printSubtitle( "Stopping iRODS server...\n" );
		my $status = stopIrods( );
		if ( $controlDatabase )
		{
			printSubtitle( "Stopping database server...\n" );
			stopDatabase( );
		}
		next;
	}
	if ( $arg =~ /^-?-?(restart)$/i )	# Restart database and iRODS
	{
		$numberCommands++;
		printSubtitle( "Stopping iRODS server...\n" );
		stopIrods( );	# OK to fail.  Server might not be started
		if ( $controlDatabase )
		{
			printSubtitle( "Stopping database server...\n" );
			stopDatabase( );
			printSubtitle( "Starting database server...\n" );
			startDatabase( ) || exit( 1 );
		}
		printSubtitle( "Starting iRODS server...\n" );
		startIrods( );
		next;
	}
	if ( $arg =~ /^-?-?(stat(us)?)$/i )	# Status of iRODS and database
	{
		$numberCommands++;
		doStatus( );
		next;
	}
	if ( $arg =~ /^-?-?test$/ )		# Run iRODS tests
	{
		$numberCommands++;
		doTest( );
		next;
	}

	printError( "Unknown command:  $arg\n" );
	printUsage( );
	exit( 1 );
}

# Tell the user how to use the script if they didn't provide
# any commands.
if ( $numberCommands == 0 )
{
	printUsage( );
	exit( 1 );
}

# Done!
if ( $doTestExitValue != 0)
{
	exit( $doTestExitValue );
}
exit( 0 );





########################################################################
#
# Commands
#	Do the job.
#

#
# @brief	Print server status.
#
# Get a process list and look for the iRODS servers and,
# optionally, the database server.  Report them if found.
#
sub doStatus
{
	# Always print status messages, even if verbosity
	# has been disabled.
	my $verbosity = isPrintVerbose( );
	setPrintVerbose( 1 );

	my %serverPids = getIrodsProcessIds( );
	if ( (scalar keys %serverPids) == 0 )
	{
		printSubtitle( "iRODS servers:\n" );
		printStatus( "No servers running\n" );
	}
	else
	{
		foreach $serverType (keys %serverPids)
		{
			my @pids = @{$serverPids{$serverType}};
			printSubtitle( $serverType . ":\n" );
			foreach $pid (@pids)
			{
				printStatus( "Process $pid\n" );
			}
		}
	}

	# Report on database servers even if they are
	# not under our control.
	printSubtitle( "Database servers:\n" );
	my $databasePID = getDatabaseProcessId( );
	if ( defined( $databasePID ) )
	{
		printStatus( "Process $databasePID \n" );
	}
	else
	{
		printStatus( "No servers running\n" );
	}
	setPrintVerbose( $verbosity );
}





#
# @brief	Optimize database.
#
# Optimize the database.  Exactly what this means varies with the
# type of database.
#
sub doOptimize
{
	if ( $DATABASE_TYPE ne "postgres" )
	{
		printError( "Database optimization is only available when iRODS is\n" );
		printError( "configured to use Postgres.\n" );
		return;
	}

	printSubtitle( "Optimizing iRODS database...\n" );

	# Stop the iRODS server to avoid vacuumdb hanging on a semaphore
	printStatus( "Stopping iRODS server...\n" );
	if ( stopIrods( ) == 0 )
	{
		# Failed.  Message already output.
		exit( 1 );
	}

	printStatus( "Optimizing database...\n" );
	my $output = `$vacuumdb -f -z $DB_NAME 2>&1`;
	if ( $? != 0 )
	{
		printError( "Postgres optimization failed.\n" );
		printError( "    ", $output );
		# Continue and restart iRODS anyway.
	}

	printStatus( "Starting iRODS server...\n" );
	startIrods( ) || exit( 1 );
}





#
# @brief	Drop database tables.
#
sub doDrop
{
	if ( isPrintVerbose( ) )
	{
		printNotice( "Droping the iCAT database tables will destroy all metadata about files\n" );
		printNotice( "stored by iRODS.  This cannot be undone.\n" );
		printNotice( "\n" );
		if ( askYesNo( "    Continue (yes/no)?  " ) == 0 )
		{
			printNotice( "Canceled.\n" );
			exit( 1 );
		}
	}
	printSubtitle( "Dropping database...\n" );


	# Stop iRODS first.  Continue even if this fails.
	# Failure probably means iRODS was already stopped.
	printStatus( "Stopping iRODS server...\n" );
	if ( stopIrods( ) == 0 )
	{
		printStatus( "    Skipped.  iRODS server already stopped.\n" );
	}

	# Start the database, if it isn't already running.
	if ( $controlDatabase )
	{
		printStatus( "Starting database server...\n" );
		my $status = startDatabase( );
		if ( $status == 0 )
		{
			printError( "Cannot start database server.\n" );
			exit( 1 );
		}
		if ( $status == 2 )
		{
			printStatus( "    Skipped.  Database server already running.\n" );
		}
	}


	printStatus( "Dropping database...\n" );
	my $output = `$dropdb $DB_NAME 2>&1`;
	if ( $? != 0 )
	{
		if ( $output =~ /does not exist/ )
		{
			# Common error.  The tables were already dropped.
			printError( "There is no iCAT database to drop.\n" );
		}
		else
		{
			printError( "iCAT database drop failed:\n" );
			my @lines = split( "\n", $output );
			foreach $line ( @lines )
			{
				$line =~ s/dropdb: //;
				printError( "    $line\n" );
			}
			exit( 1 );
		}
	}
	else
	{
		printNotice( "iCAT tables dropped.\n" );
	}
}





#
# @brief	Test installation.
#
# Run the standard iRODS tests.
#
sub doTest
{
	# Start the servers first, if needed.
	my %serverPids = getIrodsProcessIds( );
	if ( (scalar keys %serverPids) == 0 )
	{
		printSubtitle( "Starting servers...\n" );
		if ( $controlDatabase )
		{
			printStatus( "Starting database server...\n" );
			startDatabase( ) || exit( 1 );
		}
		printStatus( "Starting iRODS server...\n" );
		startIrods( ) || exit( 1 );
	}

	# Always verbose during testing.
	my $verbosity = isPrintVerbose( );
	setPrintVerbose( 1 );

	# Test iCommands
	printSubtitle( "Testing iCommands...\n" );
	doTestIcommands( );

	# Test iCAT
	printSubtitle( "\nTesting iCAT...\n" );
	doTestIcat( );

	printNotice( "\nDone.\n" );

	setPrintVerbose( $verbosity );
}





#
# @brief	Test the iCommands.
#
# Test the iCommands.
#
sub doTestIcommands
{
	# Create input for the iCommands test.
	#	The test script asks for two items of user input:
	#		The path to the iCommands.
	#		The user password.
	#
	#	An empty answer for the first one assumes that the
	#	commands are on the PATH environment variable.
	#	Earlier, setupEnvironment() has done this.
	#
	#	The user's password is added now.  To keep things
	#	a bit secure, the temp file is chmod-ed to keep
	#	it closed.  It's deleted when the tests are done.
	my $program     = File::Spec->catfile( $icommandsTestDir, "testiCommands.pl" );
	my $tmpDir      = File::Spec->tmpdir( );
	my $passwordTmp = File::Spec->catfile( $tmpDir, "irods_test_$$.tmp" );
	my $logFile     = File::Spec->catfile( $tmpDir, "testSurvey_" . hostname( ) . ".log" );
	my $outputFile  = File::Spec->catfile( $tmpDir, "testSurvey_" . hostname( ) . ".txt" );

	printToFile( $passwordTmp, "\n$IRODS_ADMIN_PASSWORD\n" );
	chmod( 0600, $passwordTmp );


	# Run the iCommands test.
	# 	The test writes a "testSurvey" to the current
	# 	directory, along with some temp files.  To
	# 	avoid cluttering up wherever this script lives,
	# 	we move to a temp directory first and run the
	# 	script there.
	my $startDir = cwd( );
	chdir( $tmpDir );
	my $output = `$perl $program < $passwordTmp 2>&1`;
	unlink( $passwordTmp );
	chdir( $startDir );
	printToFile( $outputFile, $output );


	# Count failed tests:
	#	The log lists all successfull and failed tests.
	#	Failures are too cryptic to repeat here, but we
	#	can count the failures and let the user know
	#	there's more information in the log file.
	my $failsFound = 0;
	open( LOG, "<$logFile" );
	foreach $line ( <LOG> )
	{
		if ( $line =~ /error code/ )
		{
			++$failsFound;
		}
	}
	close( LOG );


	if ( $failsFound )
	{
		printError( "$failsFound tests failed.  Check log files for details:\n" );
		printError( "    Log:     $logFile\n" );
		printError( "    Output:  $outputFile\n" );
		$doTestExitValue++;
	}
	else
	{
		printStatus( "All tests were successful.\n" );
		printStatus( "Check log files for details:\n" );
		printStatus( "    Log:     $logFile\n" );
		printStatus( "    Output:  $outputFile\n" );
	}
}





#
# @brief	Test the iCAT.
#
# Test the iCAT queries.
#
sub doTestIcat
{
	# Connect to iRODS.
	my $output  = `$iinit $IRODS_ADMIN_PASSWORD 2>&1`;
	if ( $? != 0 )
	{
		printError( "Cannot connect to iRODS.\n" );
		printError( "    ", $output );
		exit( 1 );
	}


	# Create a temporary file
	my $outputFile   = File::Spec->catfile( File::Spec->tmpdir( ), "icatSurvey_" . hostname( ) . ".txt" );

	# Enable CATSQL debug mode in Server by changing user's env file
	$homeDir=$ENV{'HOME'};
	$userEnvFile=  $homeDir . "/.irods/.irodsEnv";
	$originalEnvText = `cat $userEnvFile`;
	appendToFile( $userEnvFile, 
		      "\nirodsDebug CATSQL\n" );

	# Run tests
	#	While the output could be put anywhere, the 'checkIcatLog'
	#	script presumes the output is in the same directory as the
	#	test scripts.  So we have to put it there.
	my $icatTestLog = File::Spec->catfile( $serverTestBinDir, "icatTest.log" );
	my $icatMiscTestLog = File::Spec->catfile( $serverTestBinDir, "icatMiscTest.log" );
	my $moveTestLog = File::Spec->catfile( $serverTestBinDir, "moveTest.log" );

	my $startDir = cwd( );
	chdir( $serverTestBinDir );
	$icatFailure=0;
	$output = `$perl icatTest.pl 2>&1`;
	if ( $? != 0 ) { $icatFailure=1; }
	printToFile( $icatTestLog, $output);
	$output = `$perl icatMiscTest.pl 2>&1`;
	if ( $? != 0 ) { $icatFailure=1; }
	printToFile( $icatMiscTestLog, $output );
	$output=`$perl moveTest.pl 2>&1`;
	if ( $? != 0 ) { $icatFailure=1; }
	printToFile( $moveTestLog, $output);
	
	# If the above all succeeded, check logs to see if all SQL was tested
	$output = `checkIcatLog.pl 2>&1`;
	if ( $? != 0 ) { $icatFailure=1; }
	printToFile( $outputFile, $output );
	my @lines = split( "\n", $output );
	my $totalLine = undef;
	foreach $line (@lines)
	{
		if ( $line =~ /Total/ )
		{
			$totalLine = $line;
			last;
		}
	}


	# Restore the iRODS environment
	printToFile("$userEnvFile", $originalEnvText);

	chdir( $startDir );

	if ($icatFailure) {
	    printError( "One or more ICAT tests failed.\n" );
	    $doTestExitValue++;
	}
	else {
	    printStatus("All ICAT tests completed successfully.\n");
	}
	printStatus( "Test report:\n" );
	printStatus( "    $totalLine\n" );
	printStatus( "Check log files for details:\n" );
	printStatus( "    Logs:     $icatTestLog\n" );
	printStatus( "              $icatMiscTestLog\n" );
	printStatus( "              $moveTestLog\n" );
	printStatus( "    Summary:  $outputFile\n" );
}




















########################################################################
#
# Functions
#

#
# @brief	Set environment variables for running commands.
#
# The command and library paths are set to point to database and
# iRODS commands.  Environment variables for the database are set
# to indicate it's host, port, etc.
#
sub setupEnvironment
{
	# Execution path.  Add ".", iRODS commands, and database commands
	my $oldPath = $ENV{'PATH'};
	my $newPath = ".:$icommandsBinDir";
	if ( $controlDatabase)
	{
		$newPath .= ":$databaseBinDir";
	}
	$ENV{'PATH'} = "$newPath:$oldPath";


	# Library path.  Add database.
	if ( $controlDatabase )
	{
		my $oldLibPath = $ENV{'LD_LIBRARY_PATH'};  
		if ( ! defined($oldLibPath) || $oldLibPath eq "" )
		{
			$ENV{'LD_LIBRARY_PATH'} = $databaseLibDir;
		}
		else
		{
			$ENV{'LD_LIBRARY_PATH'}="$oldLibPath:$databaseLibDir";
		}
	}


	# Postgres variables
	if ( $controlDatabase && $DATABASE_TYPE eq "postgres" )
	{
		$ENV{"PGDATA"} = $databaseDataDir;
		$ENV{"PGPORT"} = $DATABASE_PORT;
		if ( $DATABASE_HOST !~ "localhost" &&
			$DATABASE_HOST !~ $thisHost )
		{
			$ENV{"PGHOST"} = $DATABASE_HOST;
		}
		$ENV{"PGUSER"} = $DATABASE_ADMIN_NAME;
	}
}





#
# @brief	Start iRODS server
#
# @return	0 = failed
# 		1 = started
#
sub startIrods
{
	# Make sure the server is available
	if ( ! -e $irodsServer )
	{
		printError( "Configuration problem:\n" );
		printError( "    The 'irodsServer' application could not be found.  Have the\n" );
		printError( "    iRODS servers been compiled?\n" );
		exit( 1 );
	}

	# Prepare
	my $startingDir = cwd( );
	chdir( $serverBinDir );
	umask( 077 );

	# Start the server
	my $output = `$irodsServer 2>&1`;
	if ( $? )
	{
		printError( "iRODS server failed to start\n" );
		printError( "    $output\n" );
		return 0;
	}
	chdir( $startingDir );

	# Sleep a bit to give the server time to start and possibly exit
	sleep( $databaseStartStopDelay );

	# Check that it actually started
	my %serverPids = getIrodsProcessIds( );
	if ( (scalar keys %serverPids) == 0 )
	{
		printError( "iRODS server failed to start.\n" );
		return 0;
	}

	return 1;
}





#
# @brief	Stop iRODS server
#
# This function lists all processes that appear to be iRODS servers,
# then tries to kill them.  This can fail if the processes are hung
# and refuse to die, or if they are owned by somebody else.
#
# @return	0 = failed
# 		1 = stopped or nothing to stop
#
sub stopIrods
{
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
		printStatus( "    There are no iRODS servers running.\n" );
		return 1;
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
		my @pids = getProcessIds( $processName );
		if ( $#pids >= 0 )
		{
			$didNotDie = 1;
		}
	}
	if ( $didNotDie )
	{
		printError( "    Some servers could not be killed.  They may be owned\n" );
		printError( "    by another user or there could be a problem.\n" );
		return 0;
	}
	return 1;
}





#
# @brief	Start database server
#
# If the database server is under our control, start it.
# Otherwise do nothing.
#
# @return	0 = failed
# 		1 = started
# 		2 = already started
# 		3 = not under our control
#
sub startDatabase
{
	if ( !$controlDatabase )
	{
		# The database is not under our control.
		return 3;
	}

	if ( $DATABASE_TYPE eq "postgres" )
	{
		# Is Postgres running?
		my $output = `$pgctl status 2>&1`;
		if ( $? == 0 )
		{
			# Running.  Nothing more to do.
			return 2;
		}

		# Start it.
		my $logpath = File::Spec->catfile( $databaseLogDir, "pgsql.log" );
		$output = `$pgctl start -o '-i' -l $logpath 2>&1`;
		if ( $? != 0 )
		{
			printError( "Could not start Postgres database server.\n" );
			printError( "    $output\n" );
			return 0;
		}

		# Give it time to start up
		sleep( $databaseStartStopDelay );
		return 1;
	}

	# Otherwise it's an unknown database type.
	return 3;
}





#
# @brief	Stop database server
#
# If the database server is under our control, stop it.
# Otherwise do nothing.
#
# @return	0 = failed
# 		1 = stopped
# 		2 = already stopped
# 		3 = not under out control
#
sub stopDatabase
{
	if ( !$controlDatabase )
	{
		# The database is not under our control.
		return 3;
	}

	if ( $DATABASE_TYPE eq "postgres" )
	{
		# Is Postgres running?
		my $output = `$pgctl status 2>&1`;
		if ( $? != 0 )
		{
			# Not running.
			return 2;
		}

		# Stop it.
		$output = `$pgctl stop 2>&1`;
		if ( $? != 0 )
		{
			printError( "Could not stop Postgres database server.\n" );
			printError( "    $output\n" );
			return 0;
		}

		# Give it time to stop
		sleep( $databaseStartStopDelay );
		return 1;
	}

	# Otherwise it's an unknown database type.
	return 3;
}





#
# @brief	Get the process IDs of the iRODS servers.
#
# @return	%serverPids	associative array of arrays of
# 				process ids.  Keys are server
# 				types from %servers.
#
sub getIrodsProcessIds()
{
	my %serverPids = ();
	foreach $serverType (keys %servers)
	{
		my $processName = $servers{$serverType};
		my @pids = getProcessIds( $processName );
		next if ( $#pids < 0 );
		$serverPids{$serverType} = [ @pids ];
	}
	return %serverPids;
}





#
# @brief	Get the process ID of the database server.
#
# @return	$pid			PID of the server
#
sub getDatabaseProcessId
{
	my $pid = undef;

	if ( $DATABASE_TYPE eq "postgres" )
	{
		my @pids = getProcessIds( "(postgres)" );
		if ( $#pids >= 0 )
		{
			# Return the first one only?
			($pid) = $pids[0];
		}
	}

	return $pid;
}





#
# @brief	Print command-line help
#
sub printUsage
{
	my $oldVerbosity = isPrintVerbose( );
	setPrintVerbose( 1 );

	printNotice( "This script controls the iRODS servers.\n" );
	printNotice( "\n" );
	printNotice( "Usage is:\n" );
	printNotice( "    $scriptName [options] [commands]\n" );
	printNotice( "\n" );
	printNotice( "Help options:\n" );
	printNotice( "    --help        Show this help information\n" );
	printNotice( "\n" );
	printNotice( "Verbosity options:\n" );
	printNotice( "    --quiet       Suppress all messages\n" );
	printNotice( "    --verbose     Output all messages (default)\n" );
	printNotice( "\n" );
	printNotice( "iRODS server Commands:\n" );
	printNotice( "    istart        Start the iRODS servers\n" );
	printNotice( "    istop         Stop the iRODS servers\n" );
	printNotice( "    irestart      Restart the iRODS servers\n" );
	printNotice( "\n" );
	printNotice( "Database commands:\n" );
	printNotice( "    dbstart       Start the database servers\n" );
	printNotice( "    dbstop        Stop the database servers\n" );
	printNotice( "    dbrestart     Restart the database servers\n" );
	printNotice( "    dbdrop        Delete the iRODS tables in the database\n" );
	printNotice( "    dboptimize    Optimize the iRODS tables in the database\n" );
	printNotice( "    dbvacuum      Same as 'optimize'\n" );
	printNotice( "\n" );
	printNotice( "General Commands:\n" );
	printNotice( "    start         Start the iRODS and database servers\n" );
	printNotice( "    stop          Stop the iRODS and database servers\n" );
	printNotice( "    restart       Restart the iRODS and database servers\n" );
	printNotice( "    status        Show the status of iRODS and database servers\n" );
	printNotice( "    test          Test the iRODS installation\n" );

	setPrintVerbose( $oldVerbosity );
}
