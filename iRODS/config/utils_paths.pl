#
# Perl

#
# Standard directory and file paths for iRODS Perl scripts.
#
# Usage:
# 	require config/utils_paths.pl
#
# Dependencies:
# 	none
#
# This file initializes variables for common file paths used by
# the iRODS Perl scripts.
#
# This script sets global variables.
#

use File::Spec;

$version{"utils_paths.pl"} = "1.1";





#
# iRODS installation directories
#
$binDir           = File::Spec->catdir( $IRODS_HOME, "bin" );
$installDir       = File::Spec->catdir( $IRODS_HOME, "install" );
$modulesDir       = File::Spec->catdir( $IRODS_HOME, "modules" );

$icommandsBinDir  = File::Spec->catdir( $IRODS_HOME, "clients", "icommands", "bin" );
$icommandsTestDir = File::Spec->catdir( $IRODS_HOME, "clients", "icommands", "test" );

$serverBinDir     = File::Spec->catdir( $IRODS_HOME, "server",  "bin" );
$serverSqlDir     = File::Spec->catdir( $IRODS_HOME, "server",  "icat", "src" );
$serverTestBinDir = File::Spec->catdir( $IRODS_HOME, "server",  "test", "bin" );
$serverConfigDir  = File::Spec->catdir( $IRODS_HOME, "server",  "config" );

$logDir           = File::Spec->catdir( $IRODS_HOME, "install" );





#
# iRODS user directories
#
$userIrodsDir     = File::Spec->catfile( $ENV{"HOME"}, ".irods" );
$userIrodsFile    = File::Spec->catfile( $userIrodsDir, ".irodsEnv" );





#
# iRODS commands
#
$iinit  = File::Spec->catfile( $icommandsBinDir, "iinit" );
$iexit  = File::Spec->catfile( $icommandsBinDir, "iexit" );
$iadmin = File::Spec->catfile( $icommandsBinDir, "iadmin" );
$ils    = File::Spec->catfile( $icommandsBinDir, "ils" );
$iput   = File::Spec->catfile( $icommandsBinDir, "iput" );
$iget   = File::Spec->catfile( $icommandsBinDir, "iget" );
$irm    = File::Spec->catfile( $icommandsBinDir, "irm" );
$ichmod = File::Spec->catfile( $icommandsBinDir, "ichmod" );

$istart = File::Spec->catfile( $serverBinDir, "start.pl" );
$istop  = File::Spec->catfile( $serverBinDir, "stop.pl" );





#
# iRODS configuration files
#
$irodsConfig = File::Spec->catfile( $configDir, "irods.config" );
$configMk    = File::Spec->catfile( $configDir, "config.mk" );





return( 1 );
