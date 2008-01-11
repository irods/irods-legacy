#
# Perl

#
# Configure the iRODS system before compiling or installing.
#
# Usage is:
# 	perl configure.pl [options]
#
# Options:
#	--help		List all options
#	
#	Options vary depending upon the source distribution.
#	Please use --help to get a list of current options.
#
# Script options select the database to use and its location,
# the iRODS server components, and optional iRODS modules to
# build into the system.
#
# The script analyzes your OS to determine configuration parameters
# that are OS and CPU specific.
#
# Configuration results are written to:
#
# 	config/config.mk
# 		A Makefile include file used during compilation
# 		of iRODS.
#
# 	config/irods.config
#		A parameter file used by iRODS scripts to start
#		and stop iRODS, etc.
#
# 	irodsctl
#		A shell script for running the iRODS control
#		Perl script to start/stop servers.
#

use File::Spec;
use File::Copy;
use File::Basename;
use Cwd;
use Cwd 'abs_path';
use Config;

$version{"configure.pl"} = "1.2";






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
	# script was run from the scripts or scripts/perl subdirectories.
	# Look up one directory.
	$IRODS_HOME = File::Spec->updir( );
	$configDir  = File::Spec->catdir( $IRODS_HOME, "config" );
	if ( ! -e $configDir )
	{
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

# Determine the execution environment.  These values are used
# later to select different options for different OSes, or to
# print information to various configuration files.
my $thisOS     = getCurrentOS( );
my $thisProcessor = getCurrentProcessor( );
my $thisUser   = getCurrentUser( );
my $thisUserID = $<;
my $thisHost   = getCurrentHostName( );
my %thisHostAddresses = getCurrentHostAddresses( );

# Set the number of installation steps and the starting step.
# The current step increases after each major part of the
# install.  These are used as a progress indicator for the
# user, but otherwise have no meaning.
$totalSteps  = 5;
$currentStep = 0;





########################################################################
#
# Check script usage.
#
setPrintVerbose( 1 );

if ( $thisUserID == 0 )
{
	printError( "Usage error:\n" );
	printError( "    This script should *not* be run as root.\n" );
	exit( 1 );
}





########################################################################
#
# Collect available modules.
#	Build a list of available modules.  Each module has
#	enable/disable options.  The default setting is determined
#	by the module's "info.txt" file.
#
$startDir = cwd( );
%modules = ( );
if ( -d $modulesDir )
{
	# Make a list of all modules in the directory
	chdir( $modulesDir );
	while ( defined( $module = <*> ) )
	{
		# A module must have an 'info.txt' file describing the
		# module in order for it to be configurable by this script
		my $infoPath = File::Spec->catfile(
			$modulesDir, $module, "info.txt" );
		if ( -e $infoPath )
		{
			my $value = getPropertyValue( $infoPath, "enabled", 0 );
			if ( $value =~ /ye?s?/i )
			{
				$modules{ $module } = "yes";
			}
			else
			{
				$modules{ $module } = "no";
			}
		}
	}
    	chdir( $startDir );
}





########################################################################
#
# Print a help message then exit before going further.
#	We have to print this after collecting the module list
#	above so that we can list enable/disable options for
#	those modules.
#
foreach $arg ( @ARGV )
{
	if ( !( $arg =~ /-?-?h(elp)/) )
	{
		next;
	}

	printTitle( "Configure iRODS\n" );
	printTitle( "------------------------------------------------------------------------\n" );
	printNotice( "This script configures iRODS for the current host based upon\n" );
	printNotice( "CPU and OS attributes and command-line arguments.\n" );
	printNotice( "\n" );
	printNotice( "Usage is:\n" );
	printNotice( "    configure [options]\n" );
	printNotice( "\n" );
	printNotice( "Help options:\n" );
	printNotice( "    --help                      Show this help information\n" );
	printNotice( "\n" );
	printNotice( "Verbosity options:\n" );
	printNotice( "    --quiet                     Suppress all messages\n" );
	printNotice( "    --verbose                   Output all messages (default)\n" );
	printNotice( "\n" );
	printNotice( "iCAT options:\n" );
	printNotice( "    --enable-icat               Enable iRODS metadata catalog files\n" );
	printNotice( "    --disable-icat              Disable iRODS metadata catalog files\n" );
	printNotice( "    --icat-host=<HOST>          Use iRODS+iCAT server on host\n" );
	printNotice( "    --enable-psgcat             Enable Postgres database catalog\n" );
	printNotice( "    --disable-psgcat            Disable Postgres database catalog\n" );
	printNotice( "    --enable-oracat             Enable Oracle database catalog\n" );
	printNotice( "    --disable-oracat            Disable Oracle database catalog\n" );
	printNotice( "\n" );
	printNotice( "    --enable-psghome=<DIR>      Set the Postgres directory\n" );
	printNotice( "    --enable-newodbc            Use the new ODBC interface\n" );
	printNotice( "    --enable-oldodbc            Use the old ODBC interface\n" );
	printNotice( "\n" );
	printNotice( "iRODS options:\n" );
	printNotice( "    --enable-parallel           Enable parallel computation\n" );
	printNotice( "    --disable-parallel          Disable parallel computation\n" );
	printNotice( "    --enable-file64bit          Enable large files\n" );
	printNotice( "    --disable-file64bit         Disable large files\n" );
	if ( scalar keys %modules > 0 )
	{
		printNotice( "\n" );
		printNotice( "Module options:\n" );
		foreach $module (keys %modules)
		{
			my $infoPath = File::Spec->catfile( $modulesDir, $module, "info.txt" );
			my $brief = getPropertyValue( $infoPath, "brief" );
			printNotice( "    --enable-$module        $brief\n" );
			printNotice( "    --disable-$module       $brief\n" );
		}
	}
	exit( 0 );
}





########################################################################
#
# Set a default configuration.
#
# Some or all of these may be overridden by the iRODS configuration file.
# After that, they form the starting point of further configuration and
# are written back to the iRODS configuration files at the end of this
# script.
#
%configuration = ( );

$configuration{ "RODS_CAT" } = "";	# Enable iCAT
$configuration{ "PSQICAT" }  = "";	# Enable Postgres iCAT
$configuration{ "ORAICAT" }  = "";	# Disable Oracle iCAT
$configuration{ "NEW_ODBC" } = "1";	# New ODBC drivers
$configuration{ "PARA_OPR" } = "";	# Parallel

$configuration{ "IRODS_HOME" } = $IRODS_HOME;
$configuration{ "IRODS_PORT" } = "1247";
$configuration{ "IRODS_ADMIN_NAME" } = "rods";
$configuration{ "IRODS_ADMIN_PASSWORD" } = "rods";
$configuration{ "IRODS_ICAT_HOST" } = "";

$configuration{ "DATABASE_TYPE" } = "";			# No database
$configuration{ "DATABASE_ODBC_TYPE" } = "";		# No ODBC!?
$configuration{ "DATABASE_EXCLUSIVE_TO_IRODS" } ="0";	# Database just for iRODS
$configuration{ "DATABASE_HOME" } = "$IRODS_HOME/../iRodsPostgres";	# Database directory

$configuration{ "DATABASE_HOST" } = "";			# Database host
$configuration{ "DATABASE_PORT" } = "5432";		# Database port
$configuration{ "DATABASE_ADMIN_NAME" } = $thisUser;	# Database admin
$configuration{ "DATABASE_ADMIN_PASSWORD" } = "";	# Database admin password





########################################################################
#
# Load and validate irods.config.
#	This function sets a large number of important global variables
#	based upon values in the irods.config file.  Those include the
#	type of database in use (if any), the path to that database,
#	the host and port for the database, and the initial account
#	name and password for the database and iRODS.
#
#	This function also validates that the values look reasonable and
#	prints messages if they do not.
#
#	A complication is that the config file also sets $IRODS_HOME.
#	This is intended for use by other scripts run *after* this
#	configuration script.  So, at this point $IRODS_HOME in the
#	file is invalid.  So, we ignore it and restore the value we've
#	already determined.
#
my $savedIRODS_HOME = $IRODS_HOME;
copyTemplateIfNeeded( $irodsConfig );
if ( loadIrodsConfig( ) == 0 )
{
	# Configuration failed to load or validate.  An error message
	# has already been output.
	exit( 1 );
}
$IRODS_HOME = $savedIRODS_HOME;


# Set configuration variables based upon irods.config.
#	We intentionally *do not* transfer $IRODS_HOME from the
#	configuration file.  During an initial install, that value
#	is probably empty or wrong.  Instead, use the directory
#	relative to where we are executing.
#
#	All other values in irods.config provide defaults.  When
#	this configure script is done, these values, as modified
#	by command-line arguments, are written back to irods.config
#	to provide new defaults for the next time (if ever) that
#	this script is run.
$configuration{ "IRODS_PORT" }           = $IRODS_PORT;
$configuration{ "IRODS_ADMIN_NAME" }     = $IRODS_ADMIN_NAME;
$configuration{ "IRODS_ADMIN_PASSWORD" } = $IRODS_ADMIN_PASSWORD;

$configuration{ "DATABASE_TYPE" }        = $DATABASE_TYPE;
$configuration{ "DATABASE_ODBC_TYPE" }   = $DATABASE_ODBC_TYPE;
$configuration{ "DATABASE_EXCLUSIVE_TO_IRODS" } = $DATABASE_EXCLUSIVE_TO_IRODS;
$configuration{ "DATABASE_HOME" }        = $DATABASE_HOME;

$configuration{ "DATABASE_HOST" }        = $DATABASE_HOST;
$configuration{ "DATABASE_PORT" }        = $DATABASE_PORT;
$configuration{ "DATABASE_ADMIN_NAME" }  = $DATABASE_ADMIN_NAME;
$configuration{ "DATABASE_ADMIN_PASSWORD" } = $DATABASE_ADMIN_PASSWORD;


if ( $DATABASE_ODBC_TYPE =~ /unix/i )
{
	$configuration{ "NEW_ODBC" } = "1";	# New ODBC drivers
}
else
{
	$configuration{ "NEW_ODBC" } = "";	# Old ODBC drivers
}

if ( $DATABASE_TYPE =~ /postgres/i )
{
	# Postgres.
	$configuration{ "RODS_CAT" } = "1";	# Enable iCAT
	$configuration{ "PSQICAT" }  = "1";	# Enable Postgres iCAT
	$configuration{ "ORAICAT" }  = "";	# Disable Oracle iCAT
}
elsif ( $DATABASE_TYPE =~ /oracle/i )
{
	# Oracle.
	$configuration{ "RODS_CAT" } = "1";	# Enable iCAT
	$configuration{ "PSQICAT" }  = "";	# Disable Postgres iCAT
	$configuration{ "ORAICAT" }  = "1";	# Enable Oracle iCAT
}
else
{
	# Unknown or no database.  No iCAT.
	$configuration{ "RODS_CAT" } = "";	# Disable iCAT
	$configuration{ "PSQICAT" }  = "";	# Disable Postgres iCAT
	$configuration{ "ORAICAT" }  = "";	# Disable Oracle iCAT
}





########################################################################
#
# Check command line argument(s).   Use them to set the configuration.
#
$noHeader = 0;
foreach $arg ( @ARGV )
{
	# Postgres iCAT
	if ( $arg =~ /--disable-psgi?cat/ )
	{
		$configuration{ "PSQICAT" } = "";
		$configuration{ "RODS_CAT" } = "";
		next;
	}
	if ( $arg =~ /--enable-psgi?cat/ )
	{
		$configuration{ "PSQICAT" } = "1";
		$configuration{ "RODS_CAT" } = "1";
		$configuration{ "ORAICAT" } = "";
		next;
	}

	# Oracle iCAT
	if ( $arg =~ /--disable-orai?cat/ )
	{
		$configuration{ "ORAICAT" } = "";
		$configuration{ "RODS_CAT" } = "";
		next;
	}
	if ( $arg =~ /--enable-orai?cat/ )
	{
		$configuration{ "PSQICAT" } = "";
		$configuration{ "RODS_CAT" } = "1";
		$configuration{ "ORAICAT" } = "1";
		next;
	}

	# iCAT
	if ( $arg =~ /--disable-icat/ )
	{
		$configuration{ "PSQICAT" } = "";
		$configuration{ "RODS_CAT" } = "";
		$configuration{ "ORAICAT" } = "";
		next;
	}
	if ( $arg =~ /--enable-icat/ )
	{
		$configuration{ "PSQICAT" } = "1";	# Default to Postgres
		$configuration{ "RODS_CAT" } = "1";
		$configuration{ "ORAICAT" } = "";
		next;
	}
	if ( $arg =~ /--icat-host=(.*)/ )
	{
		$configuration{ "IRODS_ICAT_HOST" } = $1;
		next;
	}


	# Postgres install directory
	if ( $arg =~ /--enable-psghome=(.*)/ )
	{
		my $psgdir = $1;
		$configuration{ "PSQICAT" } = "1";	# Default to Postgres
		$configuration{ "RODS_CAT" } = "1";
		$configuration{ "ORAICAT" } = "";
		my $default = $configuration{ "DATABASE_HOME" };
		my $psgdir_abs  = abs_path( $psgdir );
		my $default_abs = abs_path( $default );
		if ( !( $psgdir_abs =~ $default_abs ) )
		{
			# Different directory.  Assume not exclusive use.
			$configuration{ "DATABASE_HOME" } = $psgdir;
			$configuration{ "DATABASE_EXCLUSIVE_TO_IRODS" } ="0";
		}
		next;
	}

	# Parallel execution
	if ( $arg =~ /--disable-parallel/ )
	{
		$configuration{ "PARA_OPR" } = "";
		next;
	}
	if ( $arg =~ /--enable-parallel/ )
	{
		$configuration{ "PARA_OPR" } = "1";
		next;
	}

	# 64-bit file accesses
	if ( $arg =~ /--disable-file64bit/ )
	{
		$configuration{ "FILE_64BITS" } = "";
		next;
	}
	if ( $arg =~ /--enable-file64bit/ )
	{
		$configuration{ "FILE_64BITS" } = "1";
		next;
	}

	# 64-bit addressing
	if ( $arg =~ /--disable-addr64bit/ )
	{
		$configuration{ "ADDR_64BITS" } = "";
		next;
	}
	if ( $arg =~ /--enable-addr64bit/ )
	{
		$configuration{ "ADDR_64BITS" } = "1";
		next;
	}

	# New or old ODBC code
	if ( $arg =~ /--enable-newodbc/ )
	{
		$configuration{ "NEW_ODBC" } = "1";
		next;
	}
	if ( $arg =~ /--enable-oldodbc/ )
	{
		$configuration{ "NEW_ODBC" } = "";
		next;
	}

	# iRODS server port
	if ( $arg =~ /--enable-i?rodsport=(.*)/ )
	{
		$configuration{ "IRODS_PORT" } = $1;
		next;
	}

	# Modules
	my $modargfound = 0;
	foreach $module ( keys %modules )
	{
		if ( $arg =~ /--disable-$module/ )
		{
			$modules{ $module } = "no";
			$modargfound = 1;
			last;
		}
		elsif ( $arg =~ /--enable-$module/ )
		{
			$modules{ $module } = "yes";
			$modargfound = 1;
			last;
		}
	}
	if ( $modargfound )
	{
		next;
	}

	if ( $arg =~ /-?-?q(uiet)/ )
	{
		setPrintVerbose( 0 );
		next;
	}

	if ( $arg =~ /-?-?v(erbose)/ )
	{
		setPrintVerbose( 1 );
		next;
	}
	if ( $arg =~ /^-?-?indent$/i )		# Indent everything
	{
		setMasterIndent( "        " );
		next;
	}

	if ( $arg =~ /^-?-?noheader$/i )	# Suppress header message
	{
		$noHeader = 1;
		next;
	}

	# Unknown argument
	printError( "Unknown option:  $arg\n" );
	printError( "Use --help for a list of options.\n" );
	exit( 1 );
}


if ( ! $noHeader )
{
	printTitle( "Configure iRODS\n" );
	printTitle( "------------------------------------------------------------------------\n" );
}





########################################################################
#
# Check module dependencies.
#
# For each enabled module, make sure all of the modules it
# depends upon are also enabled.
#
@modulesNeeded = ( );
foreach $module (keys %modules)
{
	if ( $modules{$module} eq "yes" )
	{
		my $infoPath = File::Spec->catfile( $modulesDir, $module, "info.txt" );
		my $deplist = getPropertyValue( $infoPath, "dependencies" );
		if ( defined( $deplist ) )
		{
			my @depends = split( " ", $deplist );
			foreach $depend (@depends)
			{
				if ( ! defined( $modules{$depend} ) ||
					$modules{$depend} eq "no" )
				{
					push( @modulesNeeded, $depend );
				}
			}
		}
	}
}

if ( scalar @modulesNeeded > 0 )
{
	printError( "Configuration error:\n" );
	printError( "    The following modules are depended upon by enabled modules\n" );
	printError( "    but were not enabled:\n" );
	foreach $module (@modulesNeeded)
	{
		printError( "        $module\n" );
	}
	printError( "\n" );
	printError( "    Please review your configuration and either enable these\n" );
	printError( "    modules, or disable the ones that require them.\n" );
	printError( "\n" );
	printError( "Abort.  Please re-run this script with updated options.\n" );
	exit( 1 );
}

$currentStep++;
printSubtitle( "\nStep $currentStep of $totalSteps:  Enabling modules...\n" );
if ( scalar keys %modules > 0 )
{
	my $tmp = "";
	foreach $module (keys %modules )
	{
		if ( $modules{$module} =~ "yes" )
		{
			$tmp .= " $module";
			printStatus( "$module\n" );
		}
	}
	$configuration{ "MODULES"} = $tmp;
}
else
{
	printStatus( "    Skipped.  No modules enabled.\n" );
	$configuration{ "MODULES"} = "";
}





########################################################################
#
# Verify database setup.
#
$currentStep++;
printSubtitle( "\nStep $currentStep of $totalSteps:  Verifying configuration...\n" );
if ( $configuration{ "RODS_CAT" } ne "1" )
{
	# No iCAT.  No database.
	$configuration{ "DATABASE_TYPE" } = "";
	$configuration{ "DATABASE_HOME" } = "";
	$configuration{ "DATABASE_EXCLUSIVE_TO_IRODS" } ="0";
	$configuration{ "DATABASE_HOST" } = "";
	$configuration{ "DATABASE_PORT" } = "";
	$configuration{ "DATABASE_ADMIN_NAME" } = "";
	$configuration{ "DATABASE_ADMIN_PASSWORD" } = "";

	printStatus( "No database configured.\n" );
}
elsif ( $configuration{ "PSQICAT" } eq "1" )
{
	# Configuration has enabled Postgres.  Make sure the
	# rest of the configuration matches.
	$configuration{ "DATABASE_TYPE" } = "postgres";
	$configuration{ "POSTGRES_HOME" } = $DATABASE_HOME;

	$databaseHome = $configuration{ "DATABASE_HOME" };
	if ( ! -e $databaseHome )
	{
		printError( "\n" );
		printError( "Configuration problem:\n" );
		printError( "    Cannot find the Postgres home directory.\n" );
		printError( "        Directory:  $databaseHome\n" );
		printError( "\n" );
		printError( "Abort.  Please re-run this script after fixing this problem.\n" );
		exit( 1 );
	}
	my $databaseBin = File::Spec->catdir( $databaseHome, "bin" );
	if ( ! -e $databaseBin )
	{
		# A common error/confusion is to give a database
		# home directory that is one up from the one to use.
		# For instance, if an iRODS install has put Postgres
		# in "here", then "here/pgsql/bin" is the bin directory,
		# not "here/bin".  Check for this and silently adjust.
		my $databaseBin2 = File::Spec->catdir( $databaseHome, "pgsql", "bin" );
		if ( ! -e $databaseBin2 )
		{
			# That didn't work either.  Complain, but
			# use the first tried bin directory in the
			# message.
			printError( "\n" );
			printError( "Configuration problem:\n" );
			printError( "    Cannot find the Postgres bin directory.\n" );
			printError( "        Directory:  $databaseBin\n" );
			printError( "\n" );
			printError( "Abort.  Please re-run this script after fixing this problem.\n" );
			exit( 1 );
		}
		# That worked.  Adjust.
		$DATABASE_HOME = File::Spec->catdir( $databaseHome, "pgsql" );
		$databaseHome  = $DATABASE_HOME;
		$configuration{ "POSTGRES_HOME" } = $DATABASE_HOME;
		$configuration{ "DATABASE_HOME" } = $DATABASE_HOME;
	}

	printStatus( "Postgres database found.\n" );
}
elsif ( $configuration{ "ORAICAT" } eq "1" )
{
	# Configuration has enabled Oracle.  Make sure the
	# rest of the configuration matches.
	$configuration{ "DATABASE_TYPE" } = "oracle";
	$configuration{ "ORACLE_HOME" } = $DATABASE_HOME;

	$databaseHome = $configuration{ "DATABASE_HOME" };
	if ( ! -e $databaseHome )
	{
		printError( "\n" );
		printError( "Configuration problem:\n" );
		printError( "    Cannot find the Oracle home directory.\n" );
		printError( "        Directory:  $databaseHome\n" );
		printError( "\n" );
		printError( "Abort.  Please re-run this script after fixing this problem.\n" );
		exit( 1 );
	}

	printStatus( "Oracle database found in $databaseHome\n" );
}
else
{
	# Configuration has no iCAT.
	$configuration{ "DATABASE_TYPE" } = "";
	$configuration{ "RODS_CAT" } = "";

	printStatus( "No database configured.\n" );
}





########################################################################
#
# Check host characteristics.
#
$currentStep++;
printSubtitle( "\nStep $currentStep of $totalSteps:  Checking host system...\n" );


# What OS?
if ( $thisOS =~ /linux/i )
{
	$configuration{ "OS_platform" } = "linux_platform";
	printStatus( "Host OS is Linux.\n" );
}
elsif ( $thisOS =~ /(sunos)|(solaris)/i )
{
	if ( $thisProcessor =~ /i.86/i )	# such as i386, i486, i586, i686
	{
		$configuration{ "OS_platform" } = "solaris_pc_platform";
		printStatus( "Host OS is Solaris (PC).\n" );
	}
	else	# probably "sun4u" (sparc)
	{
		$configuration{ "OS_platform" } = "solaris_platform";
		printStatus( "Host OS is Solaris (Sparc).\n" );
	}
}
elsif ( $thisOS =~ /aix/i )
{
	$configuration{ "OS_platform" } = "aix_platform";
	printStatus( "Host OS is AIX.\n" );
}
elsif ( $thisOS =~ /irix/i )
{
	$configuration{ "OS_platform" } = "sgi_platform";
	printStatus( "Host OS is SGI.\n" );
}
elsif ( $thisOS =~ /darwin/i )
{
	$configuration{ "OS_platform" } = "osx_platform";
	printStatus( "Host OS is Mac OS X.\n" );
}
else
{
	printError( "\n" );
	printError( "Configuration problem:\n" );
	printError( "    Unrecognized OS type:  $thisOS\n" );
	printError( "\n" );
	printError( "Abort.  Please contact the iRODS developers for more information\n" );
	printError( "on support for this OS.\n" );
	exit( 1 );
}


# 64-bit addressing?
#
# Skip this check if a command-line option was given to enable
# 64-bit addressing.
#
if ( defined( $configuration{ "ADDR_64BITS" } ) )
{
	printStatus( "64-bit addressing enabled.\n" );
}
else
{
	if ( is64bit( ) )
	{
		printStatus( "64-bit addressing supported and automatically enabled.\n" );
		$configuration{ "ADDR_64BITS" } = "1";
	}
	else
	{
		printStatus( "64-bit addressing not supported and automatically disabled.\n" );
		$configuration{ "ADDR_64BITS" } = "";
	}
}





########################################################################
#
# Update files.
#
$currentStep++;
printSubtitle( "\nStep $currentStep of $totalSteps:  Updating configuration files...\n" );


# Update config.mk
printStatus( "Updating config.mk...\n" );
my $output;
my $status = copyTemplateIfNeeded( $configMk );
if ( $status == 0 )
{
	printError( "\nConfiguration problem:\n" );
	printError( "    Cannot find the configuration template:\n" );
	printError( "        File:  $configMk.in\n" );
	printError( "    Is the iRODS installation complete?\n" );
	printError( "\nAbort.  Please re-run this script when the problem is fixed.\n" );
	exit( 1 );
}
if ( $status == 2 )
{
	printStatus( "    Created $configMk\n" );
}
($status, $output) = replaceVariablesInFile( $configMk, "make", 0, %configuration );
if ( $status == 0 )
{
	printError( "\nConfiguration problem:\n" );
	printError( "    Could not update configuration file.\n" );
	printError( "        File:   $configMk\n" );
	printError( "        Error:  $output\n" );
	printError( "\nAbort.  Please re-run this script when the problem is fixed.\n" );
	exit( 1 );
}


# Update irods.config
printStatus( "Updating irods.config...\n" );
($status, $output) = replaceVariablesInFile( $irodsConfig, "perl", 0, %configuration );
if ( $status == 0 )
{
	printError( "\nConfiguration problem:\n" );
	printError( "    Could not update configuration file.\n" );
	printError( "        File:   $irodsConfig\n" );
	printError( "        Error:  $output\n" );
	printError( "\nAbort.  Please re-run this script when the problem is fixed.\n" );
	exit( 1 );
}
# Make sure irods.config is not world/other readable since it contains
# an administrator's password.
chmod( 0600, $irodsConfig );


# Update irodsctl script
printStatus( "Updating irodsctl...\n" );
($status, $output) = replaceVariablesInFile( $irodsctl, "shell", 0, %configuration );
if ( $status == 0 )
{
	printError( "\nConfiguration problem:\n" );
	printError( "    Could not update script.\n" );
	printError( "        File:   $irodsctl\n" );
	printError( "        Error:  $output\n" );
	printError( "\nAbort.  Please re-run this script when the problem is fixed.\n" );
	exit( 1 );
}
# Make sure script is executable.  Don't give world/other permissions
# since it is meant for admin use only.
chmod( 0700, $irodsctl );





########################################################################
#
# Clean out any previously-compiled files.
#
$currentStep++;
printSubtitle( "\nStep $currentStep of $totalSteps:  Cleaning out previously compiled files...\n" );

chdir( $IRODS_HOME );
($status,$output) = make( "clean" );
if ( $status != 0 )
{
	printError( "Configuration problem:\n" );
	printError( "    A problem occurred when cleaning out old compiled files\n" );
	printError( "    with 'make clean'.  Is there a problem in the Makefile?\n" );
	printError( "        ", $output );
	printError( "\n" );
	printError( "Abort.  Please re-run this script when the problem is fixed.\n" );
	exit( 1 );
}




# Done!
if ( ! $noHeader )
{
	printNotice( "\nDone.\n" );
}


exit( 0 );
