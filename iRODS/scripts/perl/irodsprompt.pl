#
# Perl

#
# Prompt and create iRODS install configuration files.
#
# Usage is:
#	perl irodsprompt.pl [options]
#
# This script runs through a series of prompts for the user to
# select installation options.  This information is used to
# create or modify configuration files prior to an install.
#

use File::Spec;
use File::Copy;
use Cwd;
use Cwd "abs_path";
use Config;

$version{"irodsprompt.pl"} = "1.0";





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
require File::Spec->catfile( $perlScriptsDir, "utils_prompt.pl" );
require File::Spec->catfile( $perlScriptsDir, "utils_config.pl" );

# Determine the execution environment.
my $thisUser   = getCurrentUser( );
my $thisHost   = getCurrentHostName( );

# Paths to other files
my $installPostgresConfig = File::Spec->catfile( $configDir, "installPostgres.config" );





########################################################################
#
# Check script usage.
#
setPrintVerbose( 1 );

# Don't allow root to run this.
if ( $< == 0 )
{
	printError( "Usage error:\n" );
	printError( "    This script should *not* be run as root.\n" );
	exit( 1 );
}

foreach $arg (@ARGV)
{
	# Options
	if ( $arg =~ /^-?-?h(elp)?$/ )		# Help / usage
	{
		printUsage( );
		exit( 0 );
	}

	printError( "Unknown command:  $arg\n" );
	printUsage( );
	exit( 1 );
}





########################################################################
#
# Defaults
#	All of the following defaults may be overwritten by values in
#	an existing iRODS configuration file.  During prompts, these
#	values, or those from the configuration file, provide default
#	choices.  An "undef" value omits a default and requires that
#	the user enter something.
#

# Default values used when there isn't an existing irods.config,
# or when irods.config has empty values and prompting finds that
# values are needed.
$DEFAULT_irodsAccount		= "rods";
$DEFAULT_irodsPassword  	= "rods";
$DEFAULT_irodsPort		= "1247";
$DEFAULT_irodsDbName		= "ICAT";
$DEFAULT_irodsDbKey		= "123";
$DEFAULT_irodsResourceName	= "demoResc";
$DEFAULT_irodsResourceDir	= "$IRODS_HOME/Vault";
$DEFAULT_irodsZone		= "tempZone";
# No default remote iCAT host

$DEFAULT_databaseServerType	= "postgres";
$DEFAULT_databaseServerOdbcType	= "unix";
$DEFAULT_databaseServerPort	= "5432";
$DEFAULT_databaseServerAccount	= $thisUser;
# No default database password
# No default database directory

# Set the initial iRods configuration to the defaults
$irodsAccount		= $DEFAULT_irodsAccount;	# Prompt.
$irodsPassword		= $DEFAULT_irodsPassword;	# Prompt.
$irodsPort		= $DEFAULT_irodsPort;		# Prompt [advanced].
$irodsDbName		= $DEFAULT_irodsDbName;		# Prompt [advanced].
$irodsDbKey		= $DEFAULT_irodsDbKey;		# Prompt [advanced].
$irodsResourceName	= $DEFAULT_irodsResourceName;	# Prompt [advanced].
$irodsResourceDir	= $DEFAULT_irodsResourceDir;	# Prompt [advanced].
$irodsZone		= $DEFAULT_irodsZone;		# Prompt [advanced].
$catalogServerHost	= undef;			# Prompt.

# Set the initial database configuration to the defaults
$databaseServerType	= $DEFAULT_databaseServerType;	# Prompt.
$databaseServerOdbcType	= $DEFAULT_databaseServerOdbcType; # Prompt.
$databaseServerPath	= undef;			# Prompt.
$databaseServerExclusive = 1;				# Prompt.

$databaseServerHost	= "localhost";			# Prompt.
$databaseServerPort	= $DEFAULT_databaseServerPort;	# Prompt [advanced].
$databaseServerAccount	= $DEFAULT_databaseServerAccount;# Prompt.
$databaseServerPassword	= undef;			# Prompt.

# Actions to take
$installDataServer	= 1;				# Prompt.
$installCatalogServer	= 1;				# Prompt.
$installDatabaseServer	= 1;				# Prompt.
$priorDatabaseExists	= 0;				# Prompt.
$deleteDatabaseData	= 0;				# Prompt.







########################################################################
#
# Prompt
#

# Intro
printTitle(
	"iRODS configuration setup\n",
	"----------------------------------------------------------------\n" );

printNotice(
	"This script prompts you for key iRODS configuration options.\n",
	"Default values (if any) are shown in square brackets [ ] at each\n",
	"prompt.  Press return to use the default, or enter a new value.\n",
	"\n",
	"\n" );


if ( -e $irodsConfig )
{
	# An irods.config file already exists.  Ask if the
	# user would like to use it as-is or change it via
	# prompts.
	printNotice(
		"A prior iRODS configuration file was found.  This script can\n",
		"prompt you for changes, or use the same configuration again.\n",
		"\n" );
	my $answer = promptYesNo(
		"Use the existing iRODS configuration",
		"yes" );

	if ( $answer == 1 )
	{
		# Skip prompting and re-use the configuration files.
		exit( 0 );
	}
	printNotice( "\n\n" );

	# Load the irods.config file and use it for defaults
	# for all prompts.  Don't validate since at this point
	# it may have incomplete information - such as a DB
	# directory that doesn't exist until we run that part
	# of the install.
	if ( loadIrodsConfigNoValidate() == 0 )
	{
		# Configuration failed to load.  An error message
		# has already been output.
		exit( 1 );
	}


	# Map irods.config values to local variables.
	$irodsAccount      = $IRODS_ADMIN_NAME;
	$irodsPassword     = $IRODS_ADMIN_PASSWORD;
	$catalogServerHost = $IRODS_ICAT_HOST;
	$irodsPort         = $IRODS_PORT;
	$irodsDbName       = $DB_NAME;
	$irodsDbKey        = $DB_KEY;
	$irodsResourceName = $RESOURCE_NAME;
	$irodsResourceDir  = $RESOURCE_DIR;
	$irodsZone         = $ZONE_NAME;

	$databaseServerType      = $DATABASE_TYPE;
	$databaseServerOdbcType  = $DATABASE_ODBC_TYPE;
	$databaseServerPath      = $DATABASE_HOME;
	$databaseServerExclusive = $DATABASE_EXCLUSIVE_TO_IRODS;

	$databaseServerHost      = $DATABASE_HOST;
	$databaseServerPort      = $DATABASE_PORT;
	$databaseServerAccount   = $DATABASE_ADMIN_NAME;
	$databaseServerPassword  = $DATABASE_ADMIN_PASSWORD;

	$installCatalogServer = 1;
	if ( $catalogServerHost ne "" )
	{
		# An iCAT host has been given, which means this
		# host won't have an iCAT.
		$installCatalogServer = 0;
	}
	$installDataServer = 1;
	if ( $irodsDbName eq "" || $irodsResourceName eq "" ||
		$irodsResourceDir eq "" || $irodsZone eq "" )
	{
		# Some essential information about an iRODS
		# server install is missing, so it appears
		# that we should not install an iRODS data
		# server.
		$installDataServer = 0;
	}

	$installDatabaseServer = 1;
	if ( $databaseServerType =~ /oracle/i )
	{
		# Configured for Oracle, so we definately
		# are not installing the database here.
		$installDatabaseServer = 0;
	}
}


# Standard or advanced?
printNotice(
	"For flexibility, iRODS has a lot of configuration options.  Most\n",
	"sites should use the standard settings, but some sites may need\n",
	"more control.\n",
	"\n" );
$advanced = promptYesNo(
	"Prompt for advanced settings",
	"no" );
printSubtitle( "\n\n" );


# Prompt
promptForIrodsConfiguration( );
printSubtitle( "\n\n" );

promptForDatabaseConfiguration( );
printSubtitle( "\n\n" );

# Adjust paths to be absolute
if ( defined( $databaseServerPath ) && $databaseServerPath ne "" )
{
	$databaseServerPath = File::Spec->rel2abs( $databaseServerPath );
}
if ( defined( $irodsResourceDir ) && $irodsResourceDir ne "" )
{
	$irodsResourceDir   = File::Spec->rel2abs( $irodsResourceDir );
}

if ( promptForConfirmation( ) == 0 )
{
	printError( "Abort.\n" );
	exit( 1 );
}

# Set configuration files
configureNewPostgresDatabase( );
configureIrods( );
printNotice( "Saved.\n" );


if ( $installDataServer && $installCatalogServer &&
	!$installDatabaseServer )
{
	printNotice(
		"\n",
		"Please be sure that the $databaseServerType database on '$databaseServerHost'\n",
		"is started and ready before continuing.\n" );
}
if ( $installDataServer && !$installCatalogServer )
{
	printNotice(
		"\n",
		"Please be sure that the iRODS + iCAT server on '$catalogServerHost'\n",
		"is started and ready before continuing.\n" );
}


exit( 0 );





########################################################################

# @brief	Prompt for the iRODS configuration
#
# Through a series of prompts, collect information for an iRODS
# configuration.  Results are returned through global variables.
#
sub promptForIrodsConfiguration( )
{
	# Intro
	if ( $advanced )
	{
		printSubtitle(
			"iRODS configuration (advanced)\n",
			"------------------------------\n" );
	}
	else
	{
		printSubtitle(
			"iRODS configuration\n",
			"-------------------\n" );

	}
	printNotice(
		"iRODS includes an iRODS data server and an iCAT metadata catalog.\n",
		"\n",
		"Most sites should build both of these.\n",
		"\n",
		"For multi-server sites, you may build the iRODS data server alone,\n",
		"and direct it to connect to an existing shared iCAT metadata\n",
		"catalog.\n",
		"\n",
		"For sites that already have iRODS installed, you may skip building\n",
		"the iRODS server and iCAT, and just build the command-line tools.\n",
		"\n" );


	# Build the iRODS data server?
	$installDataServer = promptYesNo(
		"Build an iRODS server",
		(($installDataServer == 1) ? "yes" : "no") );
	if ( $installDataServer == 0 )
	{
		# Don't install the iRODS server.  Just the iCommands.
		$irodsAccount		= undef;
		$irodsPassword		= undef;
		$catalogServerHost	= undef;
		$irodsPort		= undef;
		$irodsDbName		= undef;
		$irodsDbKey		= undef;
		$irodsResourceName	= undef;
		$irodsResourceDir	= undef;
		$irodsZone		= undef;

		$installCatalogServer	= 0;
		$installDatabaseServer	= 0;
		$priorDatabaseExists	= 0;
		$deleteDatabaseData	= 0;

		$databaseServerType	= undef;
		$databaseServerOdbcType	= undef;
		$databaseServerPath	= undef;
		$databaseServerExclusive = 0;

		$databaseServerHost	= undef;
		$databaseServerPort	= undef;
		$databaseServerAccount	= undef;
		$databaseServerPassword	= undef;
		return;
	}


	# Include the iCAT metadata catalog?
	$installCatalogServer = promptYesNo(
		"Include an iCAT catalog",
		(($installCatalogServer == 1) ? "yes" : "no") );
	if ( $installCatalogServer == 0 )
	{
		# Don't include the iCAT catalog.  Get the iCAT host, etc.
		$installDatabaseServer	= 0;
		$priorDatabaseExists	= 0;
		$deleteDatabaseData	= 0;

		$databaseServerType	= undef;
		$databaseServerOdbcType	= undef;
		$databaseServerPath	= undef;
		$databaseServerExclusive = 0;

		$databaseServerHost	= undef;
		$databaseServerPort	= undef;
		$databaseServerAccount	= undef;
		$databaseServerPassword	= undef;

		printNotice(
			"\n",
			"When an iCAT catalog is not included, the iRODS data server needs\n",
			"to connect to another iRODS server that includes an iCAT catalog.\n",
			"\n" );
		$catalogServerHost = promptHostName(
			"Host running iRODS with an iCAT catalog",
			$catalogServerHost );

# Resource name
		printNotice(
			"\n",
			"A name is needed for the storage resource that will be on this host,\n",
			"and it needs to be different from other defined resource names.\n",
			"\n" );
		$irodsResourceName = promptString(
						  "Resource name", 
			((!defined($irodsResourceName)||$irodsResourceName eq $DEFAULT_irodsResourceName) ?
				"demoResc2" : $irodsResourceName) );

# Resource directory
		$irodsResourceDir = promptString(
			"Resource storage area directory",
			((!defined($irodsResourceDir)||$irodsResourceDir eq "") ?
				$DEFAULT_irodsResourceDir : $irodsResourceDir) );

## TODO:  What about a port # for that iRODS server?
## can it differ from the to-be-installed iRODS server?

		# iRODS account name and password.
		printNotice(
			"\n",
			"The build process needs to use an existing iRODS administrator\n",
			"account for that host's iRODS data server and iCAT catalog.\n",
			"\n" );
		$irodsAccount = promptIdentifier(
			 "Existing iRODS login name",
			((!defined($irodsAccount)||$irodsAccount eq "") ?
				$DEFAULT_irodsAccount : $irodsAccount) );

		$irodsPassword = promptIdentifier(
			 "Password",
			((!defined($irodsPassword)||$irodsPassword eq "") ?
				$DEFAULT_irodsPassword : $irodsPassword) );
	}
	else
	{
		# Include the iCAT.
		$catalogServerHost = undef;	# No external host needed

		# iRODS account name and password.
		printNotice(
			"\n",
			"The build process will create a new iRODS administrator account\n",
			"for managing the system.\n",
			"\n" );

		$irodsAccount = promptIdentifier(
			 "New iRODS login name",
			((!defined($irodsAccount)||$irodsAccount eq "") ?
				$DEFAULT_irodsAccount : $irodsAccount) );

		$irodsPassword = promptIdentifier(
			"Password",
			((!defined($irodsPassword)||$irodsPassword eq "") ?
				$DEFAULT_irodsPassword : $irodsPassword) );
	}

	if ( $advanced )
	{
		# iRODS port
		printNotice(
			"\n",
			"Sites that run multiple iRODS servers on the same host my give\n",
			"each one a different port number.\n",
			"\n" );
		$irodsPort = promptInteger(
			"Port",
			((!defined($irodsPort)||$irodsPort eq "") ?
				$DEFAULT_irodsPort : $irodsPort) );


		# iRODS zone
		printNotice(
			"\n",
			"Sites may group iRODS servers and iCAT catalogs into different\n",
			"'zones' for managing different data collections.  Each zone\n",
			"has a name.\n",
			"\n" );
		$irodsZone = promptString(
			"iRODS zone name",
			((!defined($irodsZone)||$irodsZone eq "") ?
				$DEFAULT_irodsZone : $irodsZone) );


		# iRODS database name
		printNotice(
			"\n",
			"Sites that share a single database server between multiple iCAT\n",
			"catalogs may use a different database name for each catalog.\n",
			"\n" );
		$irodsDbName = promptString(
			"iRODS database name",
			((!defined($irodsDbName)||$irodsDbName eq "") ?
				$DEFAULT_irodsDbName : $irodsDbName) );


		# iRODS database key
		printNotice(
			"\n",
			"iRODS scrambles passwords stored in text files.  Scrambling uses\n",
			"an integer seed key.\n",
			"\n" );
		$irodsDbKey = promptInteger(
			"iRODS scramble key",
			((!defined($irodsDbKey)||$irodsDbKey eq "") ?
				$DEFAULT_irodsDbKey : $irodsDbKey) );


		# iRODS resource name and directory
		printNotice(
			"\n",
			"iRODS groups stored data into 'resources'.  Each resource has\n",
			"a name and a local directory in which the data is stored.\n",
			"\n" );
		$irodsResourceName = promptString(
			"Resource name",
			((!defined($irodsResourceName)||$irodsResourceName eq "") ?
				$DEFAULT_irodsResourceName : $irodsResourceName) );
		$irodsResourceDir = promptString(
			"Directory",
			((!defined($irodsResourceDir)||$irodsResourceDir eq "") ?
				$DEFAULT_irodsResourceDir : $irodsResourceDir) );
	}
	else
	{
		# Fill in with defaults if no prior value exists.

		# If there was no prior irods.config, then all of these
		# values were left at their defaults and this code is
		# redundant.
		#
		# But if there was a prior irods.config file, then we
		# don't know what these are set to and whether they
		# are consistent with the above prompted choices.
		# Since the user didn't ask for Advanced prompts,
		# we need to restore the defaults for missing values.

		$irodsPort = ((!defined($irodsPort)||$irodsPort eq "") ?
			$DEFAULT_irodsPort : $irodsPort);
		$irodsZone = ((!defined($irodsZone)||$irodsZone eq "") ?
			$DEFAULT_irodsZone : $irodsZone);
		$irodsDbName = ((!defined($irodsDbName)||$irodsDbName eq "") ?
			$DEFAULT_irodsDbName : $irodsDbName);
		$irodsDbKey = ((!defined($irodsDbKey)||$irodsDbKey eq "") ?
			$DEFAULT_irodsDbKey : $irodsDbKey);
		$irodsResourceName = ((!defined($irodsResourceName)||$irodsResourceName eq "") ?
			$DEFAULT_irodsResourceName : $irodsResourceName);
		$irodsResourceDir = ((!defined($irodsResourceDir)||$irodsResourceDir eq "") ?
			$DEFAULT_irodsResourceDir : $irodsResourceDir);
	}
}





# @brief	Prompt for the database configuration
#
# Through a series of prompts, collect information for a database
# configuration.  Results are returned through global variables.
#
sub promptForDatabaseConfiguration()
{
	# Install Postgres?
	# Database type?
	# Database host name?
	# Database account name?
	# Database account password?
	# Path to database files

	# Database features
	if ( $installCatalogServer == 0 )
	{
		# No catalog server, so no database needed.
		# No further prompts.
		return;
	}


	# Intro
	if ( $advanced )
	{
		printSubtitle(
			"Database configuration (advanced)\n",
			"---------------------------------\n" );
	}
	else
	{
		printSubtitle(
			"Database configuration\n",
			"----------------------\n" );
	}

	printNotice(
		"The iCAT catalog uses a database to store metadata.  You have\n",
		"three choices:\n",
		"\n",
		"    1.  Download and build a new Postgres database.\n",
		"    2.  Upgrade an existing Postgres database.\n",
		"    3.  Use an existing Postgres or Oracle database.\n",
		"\n" );

	# The loop below walks through three prompts for the
	# above three cases.  Normally, we ask about 1, then
	# 2, then 3.
	#
	# If $databaseServerType is already set to "oracle", it
	# can only be because that was chosen in a prior existing
	# irods.config file.  It's a good bet that the user wants
	# to keep this choice, so do the prompts here in a different
	# order for this case to give a fast path for Oracle.
	my $showPrompt = ($databaseServerType =~ /oracle/i) ? 3 : 1;
	my $oldDatabaseServerType = $databaseServerType;
	my $dontPrompt3Again = 0;
	while ( 1 )
	{
		if ( $showPrompt == 1 )
		{
			# New Postgres?
			$installDatabaseServer = promptYesNo(
				"Download and build a new Postgres database",
				(($installDatabaseServer == 0) ? "no" : "yes") );
			if ( $installDatabaseServer == 1 )
			{
				# Definitive answer.  It's a new
				# Postgres install.  Prompt for it,
				# then we're done.
				$databaseServerType = "postgres";
				$databaseServerExclusive = 1;
				promptForNewPostgresConfiguration( 0 );	# not an upgrade
				return;
			}

			# Not that choice.  Try the next one.
			$showPrompt = 2;
		}

		if ( $showPrompt == 2 )
		{
			# Upgrade existing Postgres?
			my $upgrade = promptYesNo(
				"Upgrade an existing Postgres database",
				(($databaseServerType ne "" && $databaseServerType ne "postgres") ?
					"no" : "yes" ) );
			if ( $upgrade == 1 )
			{
				# Definitive answer.  It's an upgrade
				# of an existing Postgres install.
				# Prompt for it, then we're done.
				#
				# A Postgres upgrade is actually the
				# same as installing a new Postgres.
				# In either case, the installation
				# prompts before overwriting prior
				# data.  We present this as a
				# separate "upgrade" choice for clarity.
				$installDatabaseServer = 1;
				$databaseServerType = "postgres";
				$databaseServerExclusive = 1;
				promptForNewPostgresConfiguration( 1 );	# upgrade
				return;
			}

			# Not that choice.  Try the next one.
			$showPrompt = 3;
		}

		if ( $showPrompt == 3 )
		{
			# What existing database should be used?
			if ( $dontPrompt3Again == 0 )
			{
				if ( $databaseServerType !~ /oracle/i )
				{
					# Explain if a prior install
					# hasn't already indicated
					# that Oracle is the choice.
					printNotice(
						"\n",
						"To use an existing database, the iCAT needs to know what type\n",
						"of database to use, where the database is running, and how to\n",
						"access it.  If it is a Postgres database, it must be running\n",
						"on this host.\n",
						"\n" );
				}
				$databaseServerType = promptString(
					"Database type (postgres or oracle)",
					(($databaseServerType ne "") ?
					$databaseServerType : $DEFAULT_databaseServerType) );
			}
			if ( $databaseServerType =~ /^p(ostgres)?/i )
			{
				# Definitive answer.  It's
				# a use of an existing
				# Postgres install.  Prompt
				# for it, then we're done.
				$databaseServerType = "postgres";
				if ( $oldDatabaseServerType !~ /$databaseServerType/i )
				{
					# The database choice changed,
					# probably from Oracle to
					# Postgres.
					#
					# The prior database parameters
					# are almost certainly wrong.
					# Undefine them or set them to
					# defaults (which are for Postgres).
					$databaseServerPath = undef;
					$databaseServerHost = undef;
					$databaseServerPort = $DEFAULT_databaseServerPort;
					$databaseServerAccount = $DEFAULT_databaseServerAccount;
					$databaseServerPassword = undef;
					$databaseServerOdbcType	= $DEFAULT_databaseServerOdbcType;

					# Because the prior DB was
					# probably oracle, the order of
					# these prompts was adjusted to
					# make the DB type prompt first.
					# Now, with an answer of 'postgres',
					# we don't know if the user wants
					# a new install or to use an
					# existing install.  So, start
					# the prompts over.
					$oldDatabaseServerType = "postgres";
					$installDatabaseServer = 1;
					$databaseServerExclusive = 1;
					$showPrompt = 1;

					# And next time we get here, don't
					# prompt for the DB type again.
					# This only happens if the original
					# config file said "Oracle".  The
					# first prompt asked if they wanted
					# to stick with Oracle, and the user
					# said "Postgres".  And then we
					# back to prompt 1, where they say
					# no, then 2, where they say no.
					# Which brings us back here.  And
					# now the user has already answered
					# this prompt, so don't ask again.
					#
					# This is a very rare case.
					$dontPrompt3Again = 1;
					next;
				}
				if ( $dontPrompt3Again == 1 )
				{
					printNotice(
						"\n",
						"Using an existing Postgres database.\n",
						"\n" );
				}
				$databaseServerExclusive = 0;
				promptForExistingPostgresDatabase( );
				return;
			}
			elsif ( $databaseServerType =~ /^o(racle)?/i )
			{
				# Definitive answer.  It's
				# a use of an existing
				# Oracle install.  Prompt
				# for it, then we're done.
				$databaseServerType = "oracle";
				if ( $oldDatabaseServerType !~ /$databaseServerType/i )
				{
					# The database choice changed,
					# probably from Postgres to
					# Oracle.
					#
					# The prior database parameters
					# are almost certainly wrong.
					$databaseServerPath = undef;
					$databaseServerHost = undef;
					$databaseServerPort = undef;
					$databaseServerAccount = undef;
					$databaseServerPassword = undef;
				}
				$databaseServerExclusive = 0;
				promptForExistingOracleDatabase( );
				return;
			}
#			elsif ( $databaseServerType =~ /^mysql/i )
#			{
#				promptForExistingMysqlDatabase( );
#				return;
#			}
			printError(
				"    Sorry, but iRODS only works with Postgres or Oracle\n",
				"    databases.  Please select one of these two.\n\n" );

			# Not that choice.  Try the first one
			# again.
			$showPrompt = 1;
		}
	}
}





# @brief	Prompt for a new Postgres database configuration
#
# Through a series of prompts, collect information for a new
# installation of Postgres.  Results are returned through global variables.
#
# @param	$upgrade	with a 1 value, present prompts as if this
# 				is an upgrade, otherwise present prompts
# 				as if this is a new install
#
sub promptForNewPostgresConfiguration( $ )
{
	my ($upgrade) = @_;

	$databaseServerHost = 'localhost';	# Always

	# Intro
	if ( $upgrade == 0 )
	{
		printNotice(
			"\n",
			"A new copy of Postgres will be automatically downloaded, built,\n",
			"and installed into a new directory of your choice.\n",
			"\n" );
	}
	else
	{
		printNotice(
			"\n",
			"To upgrade Postgres, a new copy of Postgres will be automatically\n",
			"downloaded, built, and installed into an existing Postgres\n",
			"directory.\n",
			"\n" );
	}



	# Get the installation directory.

	# For a new install, the user should select a new directory.
	# But if they select one that already has Postgres, ask if
	# they want to upgrade.

	# For an upgrade, the user should select a directory that
	# already has Postgres.  But if they don't, the confirm a
	# switch to a new install.

	# Keep asking until its clear what to do.
	
	$priorDatabaseExists = 0;
	while ( 1 )
	{
		# Prompt for the directory
		if ( $upgrade == 0 )
		{
			$databaseServerPath = promptString(
				"New Postgres directory",
				$databaseServerPath );
		}
		else
		{
			$databaseServerPath = promptString(
				"Existing Postgres directory",
				$databaseServerPath );
		}

		# Check if there is a Postgres install there
		my $dataDir = File::Spec->catdir( $databaseServerPath,
			"pgsql", "data" );
		if ( -e $dataDir )
		{
			# The directory exists.
			if ( $upgrade == 0 )
			{
				# But the user said they wanted a
				# new install.  Confirm.
				printNotice(
					"\n",
					"Postgres is already installed in that directory.  You can\n",
					"upgrade it to a new version, or select a different directory.\n",
					"\n" );

				my $switchUpgrade = promptYesNo(
					"Upgrade an existing Postgres database",
					"yes" );

				if ( $switchUpgrade == 0 )
				{
					# They don't want to upgrade.
					# Then ask them for another directory.
					printNotice(
						"\n",
						"Please select another directory for a new installation\n",
						"of Postgres.\n",
						"\n" );
					next;
				}
				# Otherwise they're fine with an upgrade.
				$upgrade = 1;
			}

			# The directory exists and they want an upgrade.
			$priorDatabaseExists = 1;

			# Ask if they'd like to keep the data
			printNotice(
				"\n",
				"You can reuse the existing Postgres data or delete it first.\n",
				"Deleting the data will remove all prior content and accounts.\n",
				"This cannot be undone.\n",
				"\n" );
			$deleteDatabaseData = promptYesNo(
				"Permanently delete prior Postgres data",
				"no" );
			last;
		}

		# The directory doesn't exist.
		$priorDatabaseExists = 0;
		if ( $upgrade == 1 )
		{
			# But they asked for an upgrade.  See if they'd
			# like to do a new install insteasd.
			printNotice(
				"\n",
				"The directory does not contain an installation of Postgres\n",
				"to upgrade.\n",
				"\n" );
			my $switchUpgrade = promptYesNo(
				"Install a new copy of Postgres there",
				"yes" );
			if ( $switchUpgrade == 0 )
			{
				# They don't want to install.
				# Then ask them for another directory.
				printNotice(
					"\n",
					"Please select another directory.\n",
					"\n" );
				next;
			}
			# Otherwise they're fine with a new install.
			$upgrade = 0;
		}
		last;
	}


	if ( $priorDatabaseExists == 1 && $deleteDatabaseData == 0 )
	{
		# Since there is a prior database and we aren't deleting it,
		# the password the user gives must match that in the prior
		# database
		printNotice(
			"\n",
			"This install will use the prior Postgres installation's data\n",
			"and accounts.\n",
			"\n" );
		$databaseServerAccount = promptIdentifier(
			"Existing database login name",
			((!defined($databaseServerAccount)||$databaseServerAccount eq "") ?
				$DEFAULT_databaseServerAccount : $databaseServerAccount) );
		$databaseServerPassword = promptIdentifier(
			"Password",
			$databaseServerPassword );
	}
	else
	{
		# Either there is no prior database, or we are deleting it.
		# In this case, the password is new.
		printNotice(
			"\n",
			"Installation of a new database will create an adminstrator account.\n",
			"\n" );
		$databaseServerAccount = promptIdentifier(
			"New database login name",
			((!defined($databaseServerAccount)||$databaseServerAccount eq "") ?
				$DEFAULT_databaseServerAccount : $databaseServerAccount) );
		$databaseServerPassword = promptIdentifier(
			"Password",
			$databaseServerPassword );
	}


	# There is no prompt for the ODBC type for a new install.
	# Instead, the ODBC type is set in the installPostgres.config
	# file.  During the Postgres install, that ODBC choice is
	# copied back into irods.config.


	if ( $advanced )
	{
		# Database port
		printNotice(
			"\n",
			"For sites with multiple databases, Postgres can be configured\n",
			"to use a custom port.\n",
			"\n" );
		$databaseServerPort = promptInteger(
			"Port",
			((!defined($databaseServerPort)||$databaseServerPort eq "") ?
				$DEFAULT_databaseServerPort : $databaseServerPort) );
	}
	else
	{
		# Fill in with defaults if no prior value exists.

		# If there was no prior irods.config, then the
		# database port is already set to the default.
		#
		# But if there was a prior irods.config file, and
		# it was built with a different set of choices,
		# and the user didn't ask for Advanced prompts,
		# it is possible the database port isn't set.
		# So, set it to the default.
		$databaseServerPort = ((!defined($databaseServerPort)||$databaseServerPort eq "") ?
			$DEFAULT_databaseServerPort : $databaseServerPort);
	}

	if ( $upgrade )
	{
		# Ask if we should control that database
		printNotice(
			"\n",
			"iRODS can be configured to start and stop the Postgres database\n",
			"along with the iRODS servers.  Enable this only if your database\n",
			"is not being used for anything else besides iRODS.\n",
			"\n" );
		$databaseServerExclusive = promptYesNo(
			"Start and stop the database along with iRods",
			(($databaseServerExclusive==1)?"yes":"no") );
	}
	else
	{
		$databaseServerExclusive = 0;
	}
}





# @brief	Prompt for the configuration of an existing Postgres database
#
# Through a series of prompts, collect information for an existing Postgres
# installation.  Results are returned through global variables.
#
sub promptForExistingPostgresDatabase( )
{
	$databaseServerHost = 'localhost';	# Always
	if ( $databaseServerOdbcType eq "" )
	{
		$databaseServerOdbcType = $DEFAULT_databaseServerOdbcType;
	}

	# Intro
	printNotice(
		"\n",
		"The iRODS build needs access to the include and library files\n",
		"in the existing Postgres installation.\n",
		"\n" );

	# Where is Postgres?
	$databaseServerPath = promptString(
		"Existing Postgres directory",
		$databaseServerPath );


	# What type of ODBC is it using?
	while ( 1 )
	{
		printNotice(
			"\n",
			"To access Postgres, iRODS needs to know which type of ODBC is\n",
			"available.  iRODS prior to 1.0 used the 'postgres' ODBC.\n",
			"For iRODS 1.0 and beyond, the 'unix' ODBC is preferred.\n",
			"\n" );

		$databaseServerOdbcType = promptString(
			"ODBC type (unix or postgres)",
			$databaseServerOdbcType );
		if ( $databaseServerOdbcType =~ /^u(nix)?/i )
		{
			$databaseServerOdbcType = "unix";
			last;
		}
		if ( $databaseServerOdbcType =~ /^p(ostgres)?/i )
		{
			$databaseServerOdbcType = "postgres";
			last;
		}
		printError(
			"    Sorry, but iRODS only works with UNIX or Postgres ODBC\n",
			"    drivers.  Please select one of these two.\n" );
	}


	# Account name and password
	printNotice(
		"\n",
		"iRODS will need to use an existing database account.\n",
		"\n" );
	$databaseServerAccount = promptIdentifier(
		 "Existing database login name",
		((!defined($databaseServerAccount)||$databaseServerAccount eq "") ?
			$DEFAULT_databaseServerAccount : $databaseServerAccount) );
	$databaseServerPassword = promptIdentifier(
		"Password",
		$databaseServerPassword );


	if ( $advanced )
	{
		# Port.
		printNotice(
			"\n",
			"For sites with multiple databases, iRODS can use a Postgres\n",
			"database configured to use a custom port.\n",
			"\n" );
		$databaseServerPort = promptInteger(
			"Port",
			$databaseServerPort );
	}
	else
	{
		# Fill in with defaults if no prior value exists.

		# If there was no prior irods.config, then the
		# database port is already set to the default.
		#
		# But if there was a prior irods.config file, and
		# it was built with a different set of choices,
		# and the user didn't ask for Advanced prompts,
		# it is possible the database port isn't set.
		# So, set it to the default.
		$databaseServerPort = ((!defined($databaseServerPort)||$databaseServerPort eq "") ?
			$DEFAULT_databaseServerPort : $databaseServerPort);
	}

	# Ask if we should control that database
	printNotice(
		"\n",
		"iRODS can be configured to start and stop the Postgres database\n",
		"along with the iRODS servers.  Enable this only if your database\n",
		"is not being used for anything else besides iRODS.\n",
		"\n" );
	$databaseServerExclusive = promptYesNo(
		"Start and stop the database along with iRods",
		(($databaseServerExclusive==1)?"yes":"no") );
}





# @brief	Prompt for the configuration of an existing Oracle database
#
# Through a series of prompts, collect information for an existing Oracle
# installation.  Results are returned through global variables.
#
sub promptForExistingOracleDatabase( )
{
	$databaseServerType      = "oracle";	# Already set.  No prompt.
	$databaseServerOdbcType  = undef;	# Unused. No prompt.
	$deleteDatabaseData      = 0;		# Never.  No prompt.
	$databaseServerExclusive = 0;		# Never.  No prompt.
	$databaseServerPort      = undef;	# Unused. No prompt.


	printNotice(
		"\n",
		"The iRODS build needs access to the include and library files\n",
		"in the Oracle installation.\n",
		"\n" );
	# Where is Oracle?  If ORACLE_HOME is set, use it.
	$databaseServerPath = promptString(
		"Existing Oracle directory",
		((!defined($databaseServerPath)||$databaseServerPath eq "") ?
			$ENV{"ORACLE_HOME"} : $databaseServerPath) );


	# Prompt for schema@instance and make sure the input has that form
	printNotice(
		"\n",
		"iRODS will need to use an existing database account.  Oracle\n",
		"account names have the form 'schema\@instance', such as\n",
		"'icat\@example.com'.\n",
		"\n" );
	while ( 1 )
	{
		$databaseServerAccount = promptString(
			 "Existing Oracle login name",
			 $databaseServerAccount );
		my @parts = split( '@', $databaseServerAccount );
		if ( $#parts != 1 )	# 2 parts required
		{
			printError(
				"    Sorry, but the account name must include the schema name,\n",
				"    an \@ sign, and the instance name.\n",
				"\n" );
			next;
		}
		if ( $parts[0] =~ /[^a-zA-Z0-9_\-\.]/ )
		{
			printError(
				"    Sorry, but the schema name can only include the characters\n",
				"    a-Z A-Z 0-9, dash, period, and underscore.\n",
				"\n" );
			next;
		}
		if ( $parts[1] =~ /[^a-zA-Z0-9_\-\.]/ )
		{
			printError(
				"    Sorry, but the instance name can only include the characters\n",
				"    a-Z A-Z 0-9, dash, period, and underscore.\n",
				"\n" );
			next;
		}
		$databaseServerHost = $parts[1];
		last;
	}

	# Password
	$databaseServerPassword = promptIdentifier(
		"Password",
		$databaseServerPassword );
}







# @brief	Prompt for confirmation of the prompted for values
#
# The collected information is shown to the user and they are prompted
# to confirm that they look right.  0 is returned on no, 1 on yes.
#
sub promptForConfirmation( )
{
	printSubtitle(
		"Confirmation\n",
		"------------\n" );


	printNotice(
		"Please confirm your choices.\n\n",
		"    --------------------------------------------------------\n" );

	# iRODS server + iCAT
	if ( $installDataServer )
	{
		if ( !$installCatalogServer )
		{
			printNotice(
				"    Build iRODS data server alone\n",
				"        iCAT host     '$catalogServerHost'\n" );
		}
		else
		{
			printNotice(
				"    Build iRODS data server + iCAT metadata catalog\n" );
		}
		if ( $advanced )
		{
			printNotice(
				"        directory     '$IRODS_HOME'\n",
				"        port          '$irodsPort'\n",
				"        account       '$irodsAccount'\n",
				"        password      '$irodsPassword'\n",
				"        zone          '$irodsZone'\n",
				"        db name       '$irodsDbName'\n",
				"        scramble key  '$irodsDbKey'\n",
				"        resource name '$irodsResourceName'\n",
				"        resource dir  '$irodsResourceDir'\n",
				"\n" );
		}
		else
		{
			printNotice(
				"        directory     '$IRODS_HOME'\n",
				"        account       '$irodsAccount'\n",
				"        password      '$irodsPassword'\n",
				"\n" );
		}
	}

	# Database
	if ( $installDatabaseServer || $databaseServerType ne "" )
	{
		if ( $installDatabaseServer )
		{
			my $msg = "";
			if ( $deleteDatabaseData )
			{
				$msg = "(delete prior data)";
			}
			elsif ( $priorDatabaseExists )
			{
				$msg = "(keep prior data)";
			}
			printNotice(
				"    Build Postgres $msg\n" );
		}
		elsif ( $databaseServerType eq "postgres" )
		{
			printNotice(
				"    Use existing Postgres\n" );
		}
		elsif ( $databaseServerType eq "oracle" )
		{
			printNotice(
				"    Use existing Oracle\n" );
		}

		printNotice(
			"        host          '$databaseServerHost'\n",
			($advanced ?
			"        port          '$databaseServerPort'\n" :
			""),
			"        directory     '$databaseServerPath'\n",
			"        account       '$databaseServerAccount'\n",
			"        password      '$databaseServerPassword'\n" );
		my $msg = "";
		if ( !$databaseServerExclusive )
		{
			$msg = "do not";
		}
		printNotice(
			"        control       $msg start & stop along with iRODS servers\n",
			"\n" );
	}

	# Commands
	printNotice(
		"    Build iRODS command-line tools\n",
		"    --------------------------------------------------------\n" );


	printNotice(
		"\n" );


	return promptYesNo(
		"Save configuration",
		"yes" );
}





# @brief	Set the iRODS configuration file
#
# Use the current set of global variable values, update the irods.config
# file with the user's choices.
#
sub configureIrods( )
{
	# Copy the template if irods.config doesn't exist yet.
	my $status = copyTemplateIfNeeded( $irodsConfig );
	if ( $status == 0 )
	{
		printError( "\nConfiguration problem:\n" );
		printError( "    Cannot copy the iRODS configuration template file:\n" );
		printError( "        File:  $irodsConfig.template\n" );
		printError( "    Permissions problem?  Missing file?\n" );
		printError( "\n" );
		printError( "Abort.\n" );
		exit( 1 );
	}

	# Make sure the file is only readable by the user.
	chmod( 0600, $irodsConfig );

	# Use an existing database.  Set the configuration file
	# to point to the database.
	my %configure = (
		"IRODS_HOME",			$IRODS_HOME,
		"IRODS_PORT",			$irodsPort,
		"IRODS_ADMIN_NAME",		$irodsAccount,
		"IRODS_ADMIN_PASSWORD",		$irodsPassword,
		"IRODS_ICAT_HOST",		$catalogServerHost,
		"DB_NAME",			$irodsDbName,
		"DB_KEY",			$irodsDbKey,
		"RESOURCE_NAME",		$irodsResourceName,
		"RESOURCE_DIR",			$irodsResourceDir,
		"ZONE_NAME",			$irodsZone,

		"DATABASE_TYPE",		$databaseServerType,
		"DATABASE_ODBC_TYPE",		((!defined($databaseServerOdbcType)) ? "" : $databaseServerOdbcType),
		"DATABASE_EXCLUSIVE_TO_IRODS",	"$databaseServerExclusive",
		"DATABASE_HOME",		$databaseServerPath,
		"DATABASE_HOST",		$databaseServerHost,
		"DATABASE_PORT",		$databaseServerPort,
		"DATABASE_ADMIN_NAME",		$databaseServerAccount,
		"DATABASE_ADMIN_PASSWORD",	$databaseServerPassword );
	replaceVariablesInFile( $irodsConfig, "perl", 0, %configure );
}





# @brief	Set the Postgres configuration file
#
# Use the current set of global variable values, update the
# installPostgres.config file with the user's choices.
#
sub configureNewPostgresDatabase( )
{
	# If there is no new database to configure, make sure the
	# installPostgres.config file is gone.
	if ( $installDatabaseServer == 0 )
	{
		if ( -e $installPostgresConfig )
		{
			unlink( $installPostgresConfig );
		}
		return;
	}


	# Copy the template if installPostgres.config doesn't exist yet.
	my $status = copyTemplateIfNeeded( $installPostgresConfig );
	if ( $status == 0 )
	{
		printError( "\nConfiguration problem:\n" );
		printError( "    Cannot copy the Postgres configuration template file:\n" );
		printError( "        File:  $installPostgresConfig.template\n" );
		printError( "    Permissions problem?  Missing file?\n" );
		printError( "\n" );
		printError( "Abort.\n" );
		exit( 1 );
	}

	# Make sure the file is only readable by the user.
	chmod( 0600, $installPostgresConfig );

	# Change/add settings to the file to reflect the user's choices.
	my %configure = (
		"POSTGRES_SRC_DIR",		$databaseServerPath,
		"POSTGRES_HOME",		File::Spec->catdir( $databaseServerPath, "pgsql" ),
		"POSTGRES_PORT",		$databaseServerPort,
		"POSTGRES_ADMIN_NAME",		$databaseServerAccount,
		"POSTGRES_ADMIN_PASSWORD",	$databaseServerPassword );
	if ( $deleteDatabaseData ne "" )
	{
		$configure{"POSTGRES_FORCE_INIT"} = $deleteDatabaseData;
	}
	replaceVariablesInFile( $installPostgresConfig,
		"perl", 1, %configure );
}





# @brief	Print command-line help
#
sub printUsage
{
	my $oldVerbosity = isPrintVerbose( );
	setPrintVerbose( 1 );

	printNotice( "This script prompts for configuration choices when\n" );
	printNotice( "installing iRODS.\n" );
	printNotice( "\n" );
	printNotice( "Usage is:\n" );
	printNotice( "    $scriptName [options]\n" );
	printNotice( "\n" );
	printNotice( "Help options:\n" );
	printNotice( "    --help        Show this help information\n" );

	setPrintVerbose( $oldVerbosity );
}
