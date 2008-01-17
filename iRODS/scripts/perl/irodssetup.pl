#
# Perl

#
# Build and set up iRODS.
#
# Usage is:
#	perl irodssetup.pl [options]
#
# This script automatically builds everything you need for iRODS
# and sets up a default configuration.  It prompts for any information
# it needs.
#

use File::Spec;
use File::Copy;
use Cwd;
use Cwd "abs_path";
use Config;

$version{"irodssetup.pl"} = "1.1";





#
# Design Notes:
#	This script is a front-end to other scripts.  Its purpose is
#	to provide an "easy" prompt-based way to build iRODS.  It
#	does not prompt for every possible configuration choice - only
#	those essential to the build.  Advanced users may edit the
#	configuration files directly instead.
#
#	The bulk of this script is prompts.  The real work amounts
#	to running:
#		scripts/installPostgres
#			Install Postgres, if needed.
#		scripts/configure
#			Configure the build of iRODS.
#		make
#			Build iRODS.
#		scripts/finishSetup
#			Configure the database and test.
#





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

# Get the path to Perl.  We'll use it for running other Perl scripts.
my $perl = $Config{"perlpath"};
if ( !defined( $perl ) || $perl eq "" )
{
	# Not defined.  Find it.
	$perl = findCommand( "perl" );
}

# Determine the execution environment.  These values are used
# later to select different options for different OSes, or to
# print information to the log file or various configuration files.
my $thisOS     = getCurrentOS( );
my $thisUser   = getCurrentUser( );
my $makePath   = getCurrentMake( );

my $thisUserID = $<;
my $thisHost   = getCurrentHostName( );

# Paths to other scripts
my $installPostgres = File::Spec->catfile( $IRODS_HOME, "scripts", "installPostgres" );
my $configure       = File::Spec->catfile( $IRODS_HOME, "scripts", "configure" );
my $setupFinish     = File::Spec->catfile( $IRODS_HOME, "scripts", "finishSetup" );

# Paths to other files
my $installPostgresConfig = File::Spec->catfile( $configDir, "installPostgres.config" );
my $makeLog               = File::Spec->catfile( $logDir, "installMake.log" );
my $installPostgresLog    = File::Spec->catfile( $logDir, "installPostgres.log" );
my $finishSetupLog        = File::Spec->catfile( $logDir, "finishSetup.log" );





########################################################################
#
# Check script usage.
#
setPrintVerbose( 1 );

# Don't allow root to run this.
if ( $thisUserID == 0 )
{
	printError( "Usage error:\n" );
	printError( "    This script should *not* be run as root.\n" );
	exit( 1 );
}





########################################################################
#
# Install
#
$irodsAccount           = undef;
$irodsPassword          = undef;
$installDataServer      = 0;
$installCatalogServer   = 0;
$catalogServerHost      = undef;
$installDatabaseServer  = 0;
$deleteDatabaseData     = 0;
$priorDatabaseExists    = 0;
$databaseServerHost     = undef;
$databaseServerPort     = undef;
$databaseServerAccount  = undef;
$databaseServerPassword = undef;
$databaseServerPath     = undef;
$databaseServerOdbcType = undef;
$databaseServerExclusive = 0;





# Prompt for information.
#	Asks lots of questions and sets global variables
promptUser( );


# Run the installation.
#	Runs 'irodsctl' to stop servers, if any
prepare( );


# Install database for the catalog server.
#	Runs 'installPostgres'
installDatabase( );


# Configure iRODS
#	Runs 'configure'
configureIrods( );


# Build iRODS
#	Runs 'make'
buildIrods( );


# Finish setting up iRODS
#	Runs 'setupFinish'
#
finishSetup( );


# Done!
printSubtitle( "\nDone.  iRODS is now set up.\n\n" );

printNotice( "To use the iRODS 'i-commands', update your PATH:\n" );
printNotice( "    For csh users:\n" );
printNotice( "        set path=($icommandsBinDir \$path)\n" );
printNotice( "    For sh or bash users:\n" );
printNotice( "        PATH=$icommandsBinDir:\$PATH\n" );

if ( $installDataServer )
{
	printNotice( "\nTo start and stop the servers, use 'irodsctl':\n" );
	printNotice( "    irodsctl start\n" );
	printNotice( "    irodsctl stop\n" );
	printNotice( "    irodsctl restart\n" );
	printNotice( "Add '--help' for a list of commands.\n" );
}
printNotice( "\nPlease see the iRODS documentation for additional notes on how to manage\n" );
printNotice( "the servers and adjust the configuration.\n" );

exit( 0 );





########################################################################





# @brief	Prompt the user for configuration choices
#
# This function prompts the user to configure the build.  The answers
# are used to set global variables that guide the rest of the setup.
#
# Status and error messages are output directly.
# On a failure, this function exits the script.
#
sub promptUser()
{
	#
	# Prompt for features in groups:
	#	iRODS features:
	#		Install an iRODS server?
	#		Include an iCAT catalog?
	#			iCAT host name?
	#		iRODS account name?
	#		iRODS account password?
	#
	#	Database features:
	#		Install Postgres?
	#		Database type?
	#		Database host name?
	#		Database account name?
	#		Database account password?
	#		Path to database files
	#
	printTitle( "Set up iRODS\n" );
	printTitle( "------------------------------------------------------------------------\n" );
	printNotice(
		"iRODS is a flexible data archive management system that supports many\n",
		"different site configurations.  This script will ask you a few questions,\n",
		"then automatically build and configure iRODS.\n",
		"\n",
		"There are four main components to iRODS:\n",
		"\n",
		"    1.  An iRODS server that manages stored data.\n",
		"\n",
		"    2.  An iCAT catalog that manages metadata about the data.\n",
		"\n",
		"    3.  A database used by the catalog.\n",
		"\n",
		"    4.  A set of 'i-commands' for command-line access to your data.\n",
		"\n",
		"You can build some, or all of these, in a few standard configurations.\n",
		"For new users, we recommend that you build everything.\n",
		"\n" );


	# iRODS features
	printSubtitle(
		"iRODS configuration:\n",
		"--------------------\n" );

	# Install iRODS server?
	$installDataServer = askYesNo( "    Build an iRODS server?  (yes/no) " );

	if ( $installDataServer == 0 )
	{
		# Don't install the iRODS server here.  Just build the i-commands.
		printNotice(
			"\n",
			"\n",
			"If the iRODS server is not built, the iCAT catalog and database\n",
			"are not needed.  This script will only build the 'i-commands'.\n" );
	}
	else
	{
		# Do install the iRODS server here.  With an iCAT?
		$installCatalogServer = askYesNo( "    Include an iCAT catalog?  (yes/no) " );
		$catalogServerHost  = $thisHost;

		if ( ! $installCatalogServer )
		{
			# Don't install the iCAT server here.  Get the name of the host.
			$installDatabaseServer  = 0;

			printNotice(
				"\n",
				"The iRODS server on this host needs to know the name of another\n",
				"host that includes an iCAT catalog.  The iRODS server and\n",
				"'i-commands' will be configured to use it.\n",
				"\n" );
			$catalogServerHost = askString(
				"    What host is running iRODS with an iCAT catalog?  (hostname) ",
				"        Sorry, but the host name cannot be left empty.\n" );

			printNotice(
				"\n",
				"The build process needs to access an existing account in that\n",
				"iCAT catalog.\n",
				"\n" );

			# Prompt for an iRODS account name and make sure it has
			# no special characters
			while ( 1 )
			{
				$irodsAccount = askString(
					 "    Enter the name of the iRODS account to use?  (username) ",
					 "Sorry, but the account name cannot be left empty.\n" );
				last if ( $irodsAccount !~ /[^a-zA-Z0-9_\-\.]/ );
				printError( "Sorry, but the account name can only include the characters\n",
					"a-Z A-Z 0-9, dash, period, and underscore.  Example:  rods.\n" );
			}

			# Prompt for an iRODS account password and make sure it has no special characters
			while ( 1 )
			{
				$irodsPassword = askString(
					 "    Enter the password for the iRODS account?  (password) ",
					 "Sorry, but the account password cannot be left empty.\n" );
				last if ( $irodsPassword !~ /[^a-zA-Z0-9_\-\.]/ );
				printError( "Sorry, but the account password can only include the characters\n",
					"a-Z A-Z 0-9, dash, period, and underscore.  Example:  rods.\n" );
			}
		}
		else
		{
			# Do install with the iCAT.  What iRODS password?
			printNotice(
				"\n",
				"For security reasons, the build process will create a new iRODS\n",
				"administrator account named 'rods' for managing the system.\n",
				"\n" );

			# Always use the conventional 'rods' account name.
			$irodsAccount = "rods";

			# Prompt for an iRODS account password and make sure it has no special characters
			while ( 1 )
			{
				$irodsPassword = askString(
					"    Enter a new password for the iRODS account?  (password) ",
					"Sorry, but the account password cannot be left empty.\n" );
				last if ( $irodsPassword !~ /[^a-zA-Z0-9_\-\.]/ );
				printError( "Sorry, but the account password can only include the characters\n",
					"a-Z A-Z 0-9, dash, period, and underscore.  Example:  rods.\n" );
			}
		}
	}



	# Database features
	if ( $installDataServer && $installCatalogServer )
	{
		printSubtitle(
			"\n",
			"\n",
			"Database configuration:\n",
			"-----------------------\n" );

		# Do install with an iCAT.  It needs access to a database.  Install Postgres?
		printNotice(
			"The iCAT uses a database to store metadata.  You can build and configure\n",
			"a new Postgres database now or use an existing database.\n",
			"\n" );
		$installDatabaseServer = askYesNo( "    Build Postgres?  (yes/no) " );

		if ( ! $installDatabaseServer )
		{
			# Don't install Postgres.  Use an existing database.  Where?
			printNotice(
				"\n",
				"To use an existing database, the iCAT needs to know what type of\n",
				"database to use, where the database is running, and how to access it.\n",
				"If it is a Postgres database, it must be running on this host.\n",
				"\n" );
			while ( 1 )
			{
				$databaseServerType = askString(
					"    What type of database is it?  (postgres or oracle) ",
					"Sorry, but the database type cannot be left empty.\n" );
				if ( $databaseServerType =~ /^postgres/i )
				{
					$databaseServerType = "postgres";
					$databaseServerPort = "5432";
					last;
				}
				elsif ( $databaseServerType =~ /^oracle/i )
				{
					$databaseServerType = "oracle";
					$databaseServerHost = $thisHost;
# Not needed(?)$databaseServerPort = "1247";
					last;
				}
#				elsif ( $databaseServerType =~ /^mysql/i )
#				{
#					$databaseServerType = "mysql";
#					$databaseServerPort = "3306";
#					last;
#				}
				printError(
					"Sorry, but iRODS only works with Postgres or Oracle databases\n",
					"at this time.  Please select one of these two.\n" );
			}

			if ( $databaseServerType eq "postgres" )
			{
				printNotice(
					"\n",
					"To compile iRODS, we need access to the Postgres include and\n",
					"library files.\n",
					"\n" );

				# Always use the local host.
				$databaseServerHost = "localhost";

				# Where is Postgres?
				$databaseServerPath = askString(
					"    Where is Postgres installed?  (directory path) ",
					"Sorry, but the path cannot be left empty.\n" );

				# What type of ODBC is it using?
				while ( 1 )
				{
					printNotice(
					"\n",
					"To access Postgres, we need to know which type of ODBC is\n",
					"available.  By default, iRODS prior to 1.0 (install.pl) would use\n",
					"postgres ODBC and iRODS 1.0 and beyond defaults to using unix ODBC.\n",
					"\n" );

					$databaseServerOdbcType = askString(
						"    What type of ODBC access is used?  (unix or postgres) ",
						"Sorry, but the ODBC type cannot be left empty.\n" );

					if ( $databaseServerOdbcType =~ /^unix/i )
					{
						$databaseServerOdbcType = "unix";
						last;
					}
					if ( $databaseServerOdbcType =~ /^postgres/i )
					{
						$databaseServerOdbcType = "postgres";
						last;
					}
					printError(
						"Sorry, but iRODS only works with UNIX or Postgres ODBC drivers\n",
						"for Postgres.  Please select one of these two.\n" );
				}

				# Prompt for an account name and make sure it has no special characters
				while ( 1 )
				{
					$databaseServerAccount = askString(
							 "    What Postgres account name should iRODS use?  (username) ",
							 "Sorry, but the account name cannot be left empty.\n" );
					last if ( $databaseServerAccount !~ /[^a-zA-Z0-9_\-\.]/ );
					printError( "Sorry, but the account name can only include the characters\n",
						"a-Z A-Z 0-9, dash, period, and underscore.  Example:  user123.\n" );
				}

				# Prompt for a password and make sure it has no special characters
				while ( 1 )
				{
					$databaseServerPassword = askString(
						"    What is the password for this account?  (password) ",
						"Sorry, but the account password cannot be left empty.\n" );
					last if ( $databaseServerPassword !~ /[^a-zA-Z0-9_\-\.]/ );
					printError( "Sorry, but the account password can only include the characters\n",
						"a-Z A-Z 0-9, dash, period, and underscore.  Example:  rods.\n" );
				}

				# Ask if we should control that database
				printNotice(
					"\n",
					"The iRODS scripts can be configured to start and stop the Postgres database\n",
					"along with the iRODS servers.  Only enable this if your Postgres database\n",
					"is not being used for anything else besides iRODS.\n",
					"\n" );
				$databaseServerExclusive = askYesNo(
						"    Configure iRODS scripts to start and stop Postgres?  (yes/no) " );
			}
			elsif ( $databaseServerType eq "oracle" )
			{
				# Where is Oracle?
				$databaseServerPath = $ENV{"ORACLE_HOME"};
				if ( !defined( $databaseServerPath ) || $databaseServerPath eq "" ) 
				{
					 printNotice(
						"\n",
						"To compile iRODS, we need access to the Oracle include and library files.\n",
						"\n" );
					 $databaseServerPath = askString(
						"    Where is Oracle installed?  (ORACLE_HOME (directory path)) ",
						"Sorry, but the path cannot be left empty.\n" );
				}

				# Prompt for schema@instance and make sure the input has that form
				while ( 1 )
				{
					$databaseServerAccount = askString(
							 "    What Oracle account name should iRODS use?  (schema\@instance) ",
							 "Sorry, but the account name cannot be left empty.\n" );
					my @parts = split( '@', $databaseServerAccount );
					if ( $#parts != 1 )	# 2 parts required
					{
						printError( "Sorry, but the account name must include the schema name,\n",
							"an \@ sign, and the instance name.  Example: icat\@hostname.com.\n" );
						next;
					}
					if ( $parts[0] =~ /[^a-zA-Z0-9_]/ )
					{
						printError( "Sorry, but the schema name can only include the characters\n",
							"a-Z A-Z 0-9, dash, period, and underscore.  Example:  icat.\n" );
						next;
					}
					if ( $parts[1] =~ /[^a-zA-Z0-9_]/ )
					{
						printError( "Sorry, but the instance name can only include the characters\n",
							"a-Z A-Z 0-9, dash, period, and underscore.  Example:  hostname.com.\n" );
						next;
					}
					last;
				}

				# Prompt for a password and make sure it has no special characters
				while ( 1 )
				{
					$databaseServerPassword = askString(
						"    What is the password for this account?  (password) ",
						"Sorry, but the account password cannot be left empty.\n" );
					last if ( $databaseServerPassword !~ /[^a-zA-Z0-9_\-\.]/ );
					printError( "Sorry, but the account password can only include the characters\n",
						"a-Z A-Z 0-9, dash, period, and underscore.  Example:  rods.\n" );
				}
			}
		}
		else
		{
			# Do install Postgres.
			printNotice(
				"\n",
				"You can select the directory for Postgres:\n",
				"\n",
				"    * If you are creating a new iRODS installation, select a\n",
				"      new directory.  Postgres will be automatically downloaded,\n",
				"      built, and installed there.\n",
				"\n",
				"    * If you are upgrading an iRODS installation and wish to\n",
				"      re-use an existing database, enter the path to that Postgres\n",
				"      directory.\n",
				"\n" );

			# Where is it?
			$databaseServerPath = askString(
				"    Where should Postgres be installed?  (directory path) ",
				"Sorry, but the path cannot be left empty.\n" );


			# Clean it out first?
			my $dataDir = File::Spec->catdir( $databaseServerPath, "pgsql", "data" );
			if ( -e $dataDir )
			{
				$priorDatabaseExists = 1;
				printNotice(
					"\n",
					"There is a Postgres installation in that directory already.  Reinstalling\n",
					"Postgres can reuse the existing database or delete it first.  Deleting the\n",
					"database will remove all prior data and accounts.  This cannot be undone.\n",
					"\n" );
				$deleteDatabaseData = askYesNo(
					"    Permanently delete prior Postgres data?  (yes/no) " );
			}

			# Always use the default Postgres account name = the user name
			$databaseServerAccount = $thisUser;

			if ( $priorDatabaseExists == 1 && $deleteDatabaseData == 0 )
			{
				# Since there is a prior database and we aren't deleting it,
				# the password the user gives must match that in the prior
				# database
				printNotice(
					"\n",
					"This install will use the prior Postgres installation's data\n",
					"and accounts, including an administrator account for '$thisUser'.\n",
					"\n" );

				# Prompt for a password
				while ( 1 )
				{
					$databaseServerPassword = askString(
						"    What is the password for the prior database account?  (password) ",
						"Sorry, but the account password cannot be left empty.\n" );
					last if ( $databaseServerPassword !~ /[^a-zA-Z0-9_]/ );
					printError( "Sorry, but the account password can only include the characters\n",
						"a-Z A-Z 0-9 and underscore.  Example:  rods.\n" );
				}
			}
			else
			{
				# Either there is no prior database, or we are deleting it.
				# In this case, the password is new.
				printNotice(
					"\n",
					"For security reasons, the new database will create an adminstrator\n",
					"account for '$thisUser' and assign a password.\n",
					"\n" );

				# Prompt for a password
				while ( 1 )
				{
					$databaseServerPassword = askString(
						"    Enter a password for the new database account?  (password) ",
						"Sorry, but the account password cannot be left empty.\n" );
					last if ( $databaseServerPassword !~ /[^a-zA-Z0-9_]/ );
					printError( "Sorry, but the account password can only include the characters\n",
						"a-Z A-Z 0-9 and underscore.  Example:  rods.\n" );
				}
			}
		}
	}




	########################################################################
	#
	# Confirm the user is ready.
	#
	printSubtitle(
		"\n",
		"\n",
		"Confirmation:\n",
		"-------------\n" );
	printNotice( "The iRODS build and setup is ready to begin.\n\n" );

	if ( $installDataServer )
	{
		printNotice( "    iRODS server:  build\n" );
		printNotice( "                       account '$irodsAccount'\n" );
		printNotice( "                       password '$irodsPassword'\n" );
		printNotice( "                       path '" . cwd( ) . "'\n" );

		if ( $installCatalogServer )
		{
			printNotice( "    iCAT catalog:  build\n" );
			if ( $installDatabaseServer )
			{
				if ( $deleteDatabaseData )
				{
					printNotice( "    Postgres:      delete prior data and reinstall\n" );
				}
				elsif ( $priorDatabaseExists )
				{
					printNotice( "    Postgres:      keep prior dtaa and reinstall\n" );
				}
				else
				{
					printNotice( "    Postgres:      install a new database\n" );
				}
				printNotice( "                       enable iRODS scripts to start/stop database\n" );
			}
			elsif ( $databaseServerType eq "postgres" )
			{
				printNotice( "    Postgres:      use existing on '$databaseServerHost'\n" );
				if ( $databaseServerExclusive )
				{
					printNotice( "                       enable iRODS scripts to start/stop database\n" );
				}
				else
				{
					printNotice( "                       do not enable iRODS scripts to start/stop database\n" );
				}
			}
			elsif ( $databaseServerType eq "oracle" )
			{
				printNotice( "    Oracle:        use existing on '$databaseServerHost'\n" );
				printNotice( "                       do not enable iRODS scripts to start/stop database\n" );
			}
			printNotice( "                       account '$databaseServerAccount'\n" );
			printNotice( "                       password '$databaseServerPassword'\n" );
			printNotice( "                       path '$databaseServerPath'\n" );
		}
		else
		{
			printNotice( "    iCAT catalog:  use existing on '$catalogServerHost'\n" );
		}
	}
	printNotice( "    I-commands:    build\n" );

	if ( $installDataServer && $installCatalogServer && !$installDatabaseServer )
	{
		printNotice(
			"\n",
			"Please be sure that the $databaseServerType database is started and ready\n",
			"ready before continuing.\n" );
	}
	if ( $installDataServer && !$installCatalogServer )
	{
		printNotice(
			"\n",
			"Please be sure that the iRODS+iCAT server on $catalogServerHost is started\n",
			"and ready before continuing.\n" );
	}


	printNotice( "\n" );
	$ready = askYesNo( "    Ready?  (yes/no) " );
	if ( $ready == 0 )
	{
		printError( "Abort.\n" );
		exit( 1 );
	}
}





# @brief	Prepare to install
#
# Stop the servers, if there are any.  Ignore errors.
#
sub prepare( )
{
	# On a fresh install, there won't be any servers running.
	# But on a re-install, there might be.  Stop them.
	#
	# Don't tell the user we're stopping servers, because that is
	# just confusing to a new user who doesn't know there are servers
	# or who knows there shouldn't be any because this is the first
	# install.  So, we just say we are "Preparing".
	#
	# Ignore error messages.  If this is the first install, then
	# 'irodsctl' won't work because configuration files won't be
	# right yet.  That's fine because it probably means there are
	# no servers to stop anyway.
	#
	printSubtitle( "\n\nPreparing...\n" );
	`$irodsctl --quiet stop 2>&1`;
	# Ignore errors.
}





# @brief	Install the database, if needed
#
# This function checks if the user has asked for a Postgres
# database to be installed.  If so, it updates the install script's
# configuration file with the user's choices, then runs
# the install script.
#
# Status and error messages are output directly.
# On a failure, this function exits the script.
#
sub installDatabase()
{
	if ( $installDatabaseServer )
	{
		printSubtitle( "\nInstalling Postgres database...\n" );

		# If there isn't a config file yet, copy the template.
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

		# Change/add settings to the file to reflect the user's
		# choices.
		my %configure = (
			"POSTGRES_SRC_DIR",		$databaseServerPath,
			"POSTGRES_HOME",		File::Spec->catdir( $databaseServerPath, "pgsql" ),
			"POSTGRES_ADMIN_NAME",		$databaseServerAccount,
			"POSTGRES_ADMIN_PASSWORD",	$databaseServerPassword );
		replaceVariablesInFile( $installPostgresConfig,
			"perl", 1, %configure );

		# Run the Postgres install script.
		#
		# Note that since we don't need to redirect the script's
		# output to a variable, we don't need a subshell to run
		# the script.  So, we use system() instead of exec() or
		# backquotes.  This skips the subshell, sends output to
		# the user's screen, and is more efficient.
		#
		# Arguments passed to the script:
		# 	--noask
		# 		Don't prompt the user for anything.
		# 		We have already collected the data the
		# 		script needs and added it to the Postgres
		# 		config file.
		#
		# 	--noheader
		# 		Don't output a header message.  We already
		# 		have and the script's own header would just
		# 		add clutter.
		#
		# 	--indent
		# 		Indent the script's own messages so that
		# 		they look nice together with this script's
		# 		messages.
		#
		#	--forceinit
		#		Force the installation to delete a
		#		prior installation's data first and to
		#		reset the configuration files.
		#
		my @options = ( "--noask", "--noheader", "--indent" );
		if ( $deleteDatabaseData )
		{
			push( @options, "--forceinit" );
		}
		system( "$installPostgres", @options );
		if ( $? ne 0 )
		{
			printError( "Installation failed.  Please see the log file for details:\n" );
			printError( "    $installPostgresLog\n" );
			exit( 1 );
		}

		# The Postgres install script automatically transfers its settings
		# into the irods.config script used as a starting point by the
		# configure script run next.
	}
	else
	{
		# No need to install a database.
		# Update the irods.config file accordingly.
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
		chmod( 0600, $installPostgresConfig );

		my %configure = (
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
}





# @brief	Configure iRODS
#
# Use the user's choices to intialize the 'irods.config' file
# and choose options for the 'configure' script, then run that
# script.
#
# If a catalog server is not needed, disable it on the configure.
# This prevents it from being compiled, and started whenever
# iRODS is started.
#
# Status and error messages are output directly.
# On a failure, this function exits the script.
#
sub configureIrods( )
{
	printSubtitle( "\nConfiguring iRODS...\n" );
	@options = ();
	if ( ! $installCatalogServer )
	{
		push( @options, "--disable-icat" );
		push( @options, "--disable-psgcat" );
		push( @options, "--disable-oracat" );
		push( @options, "--icat-host=" . $catalogServerHost );
	}
	else
	{
		push( @options, "--enable-icat" );
		if ( $databaseServerType eq "postgres" )
		{
			push( @options, "--enable-psgcat" );
		}
		elsif ( $databaseServerType eq "oracle" )
		{
			push( @options, "--enable-oracat" );
		}
		if ( $databaseServerOdbcType eq "unix" )
		{
			push( @options, "--enable-newodbc" );
		}
		elsif ( $databaseServerOdbcType eq "postgres" )
		{
			push( @options, "--enable-oldodbc" );
		}
	}

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
	chmod( 0600, $installPostgresConfig );

	# Add iRODS variables to the config file.  These both may
	# be empty if we are just installing the i-commands.
	my %configure = (
		"IRODS_ADMIN_NAME",	$irodsAccount,
		"IRODS_ADMIN_PASSWORD",	$irodsPassword );
	replaceVariablesInFile( $irodsConfig, "perl", 0, %configure );


	# Run the configure script.
	#
	# See the note earlier about using system() instead of exec() or
	# backquotes.
	#
	# Arguments passed to the script:
	# 	--noheader
	# 		Don't output a header message.  We already
	# 		have and the script's own header would just
	# 		add clutter.
	#
	# 	--indent
	# 		Indent the script's own messages so that
	# 		the look nice together with this script's
	# 		messages.
	#
	# Plus options set above based upon user choices.
	#
	system( $configure,
		"--noheader",		# Don't output a header message
		"--indent",		# Indent everything
		@options );		# Enable/disable iCAT
	if ( $? ne 0 )
	{
		printError( "Configure failed.\n" );
		# There is no log file from the configure script.
		# Its errors are printed out directly.
		exit( 1 );
	}
}





# @brief	Compile iRODS
#
# After configuration, the makefiles have been set up with
# the user's choices.  Run make to compile iRODS.
#
# Compile everything.  Route the make output to the log file
# so that it doesn't spew lots of detail at the user.
#
# Status and error messages are output directly.
# On a failure, this function exits the script.
#
sub buildIrods( )
{
	printSubtitle( "\nCompiling iRODS...\n" );

	unlink( $makeLog );

	my $totalSteps = 2;
	if ( $installDataServer )
	{
		$totalSteps++;
	}
	my $currentStep = 0;

	# Build the library and icommands
	++$currentStep;
	printSubtitle( "\n        Step $currentStep of $totalSteps:  Compiling library and i-commands...\n" );
	`$makePath icommands >> $makeLog 2>&1`;
	if ( $? ne 0 )
	{
		printError( "Compilation failed.  Please see the log file for details:\n" );
		printError( "    $makeLog\n" );
		exit( 1 );
	}

	if ( $installDataServer )
	{
		# Build the server
		++$currentStep;
		printSubtitle( "\n        Step $currentStep of $totalSteps:  Compiling iRODS server...\n" );
		`$makePath server >> $makeLog 2>&1`;
		if ( $? ne 0 )
		{
			printError( "Compilation failed.  Please see the log file for details:\n" );
			printError( "    $makeLog\n" );
			exit( 1 );
		}
	}

	# Build the tests
	++$currentStep;
	printSubtitle( "\n        Step $currentStep of $totalSteps:  Compiling tests...\n" );
	`$makePath test >> $makeLog 2>&1`;
	if ( $? ne 0 )
	{
		printError( "Compilation failed.  Please see the log file for details:\n" );
		printError( "    $makeLog\n" );
		exit( 1 );
	}
}




# @brief	Finish setting up iRODS
#
# After building iRODS, its time to install the iRODS tables
# and adjust configuration files.
#
# Status and error messages are output directly.
# On a failure, this function exits the script.
#
sub finishSetup( )
{
	if ( $installDataServer == 0 )
	{
		# The iRODS server is not being installed so there
		# is no server or user configuration to be done.
		return;
	}


	printSubtitle( "\nSetting up iRODS...\n" );

	# Run the completion script.
	#
	# See the note earlier about using system() instead of exec() or
	# backquotes.
	#
	# Arguments passed to the script:
	# 	--noask
	# 		Don't prompt the user for anything.
	# 		We have already collected the data the
	# 		script needs and added it to the Postgres
	# 		config file.
	#
	# 	--noheader
	# 		Don't output a header message.  We already
	# 		have and the script's own header would just
	# 		add clutter.
	#
	# 	--indent
	# 		Indent the script's own messages so that
	# 		the look nice together with this script's
	# 		messages.
	#
	system( $setupFinish,
		"--noask",
		"--noheader",
		"--indent" );
	if ( $? ne 0 )
	{
		printError( "Set up failed.  Please see the log file for details:\n" );
		printError( "    $finishSetupLog\n" );
		exit( 1 );
	}
}
