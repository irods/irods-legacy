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
# Initialize.
#

# Get the script name.  We'll use it for some print messages.
my $scriptName = $0;

# Get the path to Perl.  We'll use it for running other Perl scripts.
my $perl  = $ENV{"_"};
if ( !defined( $perl ) || $perl eq "" )
{
	# Not defined.  Just use "perl" on the user's path.
	$perl = "perl";
}

# Load support scripts.
require File::Spec->catfile( $configDir, "utils_platform.pl" );
require File::Spec->catfile( $configDir, "utils_print.pl" );
require File::Spec->catfile( $configDir, "utils_file.pl" );
require File::Spec->catfile( $configDir, "utils_paths.pl" );
require File::Spec->catfile( $configDir, "utils_config.pl" );

# Determine the execution environment.  These values are used
# later to select different options for different OSes, or to
# print information to the log file or various configuration files.
my $thisOS     = getCurrentOS( );
my $thisUser   = getCurrentUser( );
my $thisUserID = $<;
my $thisHost   = getCurrentHostName( );
my %thisHostAddresses = getCurrentHostAddresses( );

# Set the number of seconds to sleep after starting or stopping the
# database server.  This gives it time to start or stop before
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
# Check usage.
#	Command-line options select whether to be verbose or not.
#
#	Command-line commands tell us what to do.  The main commands
#	start and stop the servers.
#
$scriptName = $0;
setPrintVerbose( 1 );

# Make sure we are not being run as root.
$uid = $<;
if ( $uid == 0 )
{
	printError( "Usage error:\n" );
	printError( "    This script should *not* be run as root.\n" );
	exit( 1 );
}

setupEnvironment( );
$numberCommands = 0;
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


	# Commands
	if ( $arg =~ /^-?-?(start)$/i )		# Start database and iRODS
	{
		$numberCommands++;
		doStart( );
		next;
	}
	if ( $arg =~ /^-?-?(stop)$/i )		# Stop database and iRODS
	{
		$numberCommands++;
		doStop( );
		next;
	}
	if ( $arg =~ /^-?-?(restart)$/i )	# Restart database and iRODS
	{
		$numberCommands++;
		doRestart( );
		next;
	}
	if ( $arg =~ /^-?-?(stat(us)?)$/i )	# Status of iRODS and database
	{
		$numberCommands++;
		doStatus( );
		next;
	}
	if ( $arg =~ /^-?-?((opt(imize)?)|(vac(uu?m)))$/i ) # Optimize tables
	{
		$numberCommands++;
		doOptimize( );
		next;
	}
	if ( $arg =~ /^-?-?(icat)?drop$/ )	# Drop iCAT tables from database
	{
		$numberCommands++;
		doDrop( );
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
exit( 0 );





########################################################################
#
# Commands
#	Do the job.
#

#
# @brief	Start iRODS and, optionally, the database.
#
# If there is a local database and we're controlling it, then
# start the database.  In any case, start iRODS.
#
sub doStart
{
	printSubtitle( "Starting servers...\n" );

	printStatus( "Starting database server...\n" );
	startDatabase( ) || exit( 1 );

	if ( $controlDatabase )
	{
		printStatus( "Starting iRODS server...\n" );
		startIrods( ) || exit( 1 );
	}
}





#
# @brief	Stop iRODS and, optionally, the database
#
# Stop iRODS.  If there is a local database and we're
# controlling it, then stop the databse.
#
sub doStop
{
	printSubtitle( "Stopping servers...\n" );

	printStatus( "Stopping iRODS server...\n" );
	my $status = stopIrods( );

	if ( $controlDatabase )
	{
		printStatus( "Stopping database server...\n" );
		stopDatabase( ) || exit( 1 );
	}
	else
	{
		printStatus( "Stopping database server...\n" );
		printStatus( "    Skipped.  Database server is shared with other applications.\n" );
	}

	if ( $status != 0 )
	{
		exit( 1 );
	}
}





#
# @brief	Restart iRODS and, optionally, the database
#
# Stop then start iRODS.  Also stop and start the database if
# it is under our control.
#
sub doRestart
{
	printSubtitle( "Restarting servers...\n" );

	# Stop iRODS and database.  Continue even if it fails.
	# It is OK to restart even if the servers aren't
	# running.
	printStatus( "Stopping iRODS server...\n" );
	stopIrods( );

	if ( $controlDatabase )
	{
		printStatus( "Stopping database server...\n" );
		stopDatabase( );

		printStatus( "Starting database server...\n" );
		startDatabase( ) || exit( 1 );
	}

	printStatus( "Starting iRODS server...\n" );
	startIrods( ) || exit( 1 );
}





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

	printSubtitle( "iRODS servers:\n" );
	my ($serverPID, $reServerPID) = getIrodsProcessIds( );
	if ( defined( $serverPID ) )
	{
		printStatus( "Process $serverPID.  iRODS server running.\n" );
	}
	if ( defined( $reServerPID ) )
	{
		printStatus( "Process $reServerPID.  iRODS rule engine server running.\n" );
	}
	if ( !defined( $serverPID ) && !defined( $reServerPID ) )
	{
		printStatus( "No servers running\n" );
	}


	if ( $controlDatabase )
	{
		printSubtitle( "\nDatabase servers:\n" );
		my $databasePID = getDatabaseProcessId( );
		if ( defined( $databasePID ) )
		{
			printStatus( "Process $databasePID.  Database server running.\n" );
		}
		else
		{
			printStatus( "No servers running\n" );
		}
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
	my ($serverPID, $reServerPID) = getIrodsProcessIds( );
	if ( !defined( $serverPID ) )
	{
		doStart( );
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


	# Create a temporary iRODS environment with debugging enabled
	my $tmpIrodsFile = File::Spec->catfile( File::Spec->tmpdir( ), "irods_env_$$.tmp" );
	my $outputFile   = File::Spec->catfile( File::Spec->tmpdir( ), "icatSurvey_" . hostname( ) . ".txt" );

	if ( -e $userIrodsFile )
	{
		copy( $userIrodsFile, $tmpIrodsFile );
	}
	appendToFile( $tmpIrodsFile, "\nirodsDebug CATSQL\n" );
	$ENV{"irodsEnvFile"} = $tmpIrodsFile;


	# Run tests
	#	While the output could be put anywhere, the 'checkIcatLog'
	#	script presumes the output is in the same directory as the
	#	test scripts.  So we have to put it there.
	my $icatTestLog = File::Spec->catfile( $serverTestBinDir, "icatTest.log" );
	my $icatMiscTestLog = File::Spec->catfile( $serverTestBinDir, "icatMiscTest.log" );
	my $moveTestLog = File::Spec->catfile( $serverTestBinDir, "moveTest.log" );

	my $startDir = cwd( );
	chdir( $serverTestBinDir );
	printToFile( $icatTestLog,     `$perl icatTest.pl 2>&1` );
	printToFile( $icatMiscTestLog, `$perl icatMiscTest.pl 2>&1` );
	printToFile( $moveTestLog,     `$perl moveTest.pl 2>&1` );


	# Check logs
	$output = `checkIcatLog.pl 2>&1`;
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
	$ENV{"irodsEnvFile"} = undef;
	unlink( $tmpIrodsFile );
	chdir( $startDir );

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
	my $output = `$perl $istart 2>&1`;
	if ( $? != 0 )
	{
		printError( "Could not start iRODS server.\n" );
		printError( "    $output\n" );
		return 0;
	}
	return 1;
}





#
# @brief	Stop iRODS server
#
# @return	0 = failed
# 		1 = started
#
sub stopIrods
{
	my $output = `echo yes | $perl $istop`;
	if ( $? != 0 )
	{
		printError( "Could not stop iRODS server.\n" );
		printError( "    $output\n" );
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
# @return	($serverPID,$reServerPID)	PIDs of iRODS servers
#
sub getIrodsProcessIds
{
	my $serverPID = undef;
	my $reServerPID = undef;

	my @pids = getProcessIds( "irodsServer" );
	if ( $#pids >= 0 )
	{
		# Return first one only?  Shouldn't be more.
		$serverPID = $pids[0];
	}

	@pids = getProcessIds( "irodsReServer" );
	if ( $#pids >= 0 )
	{
		# Return first one only?  Shouldn't be more.
		$reServerPID = $pids[0];
	}

	return ($serverPID, $reServerPID);
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
		my @pids = getProcessIds( "\/postgres" );
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
	printNotice( "Commands:\n" );
	printNotice( "    start         Start the iRODS and database servers\n" );
	printNotice( "    stop          Stop the iRODS and database servers\n" );
	printNotice( "    restart       Restart the iRODS and database servers\n" );
	printNotice( "\n" );
	printNotice( "    status        Show the status of iRODS and database servers\n" );
	printNotice( "    test          Test the iRODS installation\n" );
	printNotice( "\n" );
	printNotice( "    optimize      Optimize the iRODS tables in the database\n" );
	printNotice( "    vacuum        Same as 'optimize'\n" );
	printNotice( "\n" );
	printNotice( "    icatdrop      Delete the database tables\n" );

	setPrintVerbose( $oldVerbosity );
}
