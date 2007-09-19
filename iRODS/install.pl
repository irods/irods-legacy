#!/usr/bin/perl
# On SDSC Solaris systems, change the above line to use this path:
# #!/usr/local/bin/perl5.8

# Copyright (c), The Regents of the University of California            ***
# For more information please refer to files in the COPYRIGHT directory ***

# See install/install.help (or run install.pl help) for command line
# options, etc.

# To run:
# 1) Unpack the release tar file (gzip -d irods.tar, tar xf irods.tar) 
#    or at SDSC checkout a current cvs tree.
#
# 2) cd into the top directory and run 'install.pl' (no changes are needed).
#    It will prompt for a few parameters or you can edit install/install.config.

# It will download postgres and odbc, and build them and put the tar
# files and binaries one level up and in a subdir: ../iRodsPostgres.
# If this already exists, it will ask you if you want to reuse it
# rather than rebuilding postgres and odbc from scratch.

# You can also use this script to start and stop the postgres and RODS
# servers, via ./install.pl start and ./install.pl stop.  There is
# also a ./install.pl ps that lists the postgres and rods processes
# (servers).  And a "./install.pl test" to run some basic tests.
# Run 'install.pl help') for a description of all the
# options.

use Sys::Hostname;

$startDir=`pwd`;
chomp($startDir);

# ***********************************************************************

($arg1)=@ARGV;
if ($arg1 eq "-h" or $arg1 eq "-help" or $arg1 eq "help" or $arg1 eq "h") {
    usage();
    exit(0);
}

# include install/install.config or create it if need be.
if (-e "install/install.config") {
    require "install/install.config";
} 
else {
    runCmdNoLog(0,"cp install/install.config.example install/install.config");
    printf("Do you want to want to use the default settings,\n");
    printf("or edit install/install.config before proceeding?\n");
    printf("Enter yes to proceed without editing:");
    $cmd=<STDIN>;
    chomp($cmd);
    if ($cmd ne "yes") {
	die("Aborted by user");
    }
    require "install/install.config";
}

$thisUser = `whoami`;
chomp($thisUser);

$DB_NAME="ICAT";                  # Name of the Postgres database to create 
                                  # and use as the ICAT database.
$ADMIN_NAME_BOOT="rodsBoot";      # Name of the rods-admin as ingested via sql
$ADMIN_BOOT_PW="RODS";            # Passward for the above user (changed later)

$POSTGRES_ADMIN_NAME="$thisUser"; # Normally the postgres admin is the same
                                  # as the RODS admin unix username, but you
                                  # can set it to another value if needed,
                                  # e.g.: using an existing postgres.
$ZONE_NAME="tempZone";            # The local Zone as set up by the sql

if (!$DB_PASSWORD) {
    $DB_PASSWORD = $ADMIN_PW;
}

$POSTGRES_DEFAULT_PORT="5432";
$POSTGRES_PW_FILE=".pgpass";

$DB_KEY="123";                    # Arbitrary key used to scramble the db pw
$SubsetMode = "0";  
$shTstFile="install.pl.testFile.shell.chk.842365"; # file to rm/make for a test
$unixODBC=0;
if (index($ODBC_FILE,"unixODBC")>=0) {
    $unixODBC=1;
}

####################################

$clBin="clients/icommands/bin";  # Relative path in RODS tree


$postgresTarFile = $POSTGRES_FILE;
if (rindex($POSTGRES_FILE,"gz") gt 0) {
    $postgresTarFile = substr($POSTGRES_FILE,0,-3);
}

$postgresDirFull="$postgresDirWork/" . substr($postgresTarFile,0,-4);

if ($postgresInstallDir) {
# User has specified the postgresInstallDir in the config file, so reuse
# that one.
    $usingExistingPostgres=1;
    $SubsetMode="1";
#   $postgresData = "$postgresInstallDir/data2";
}
else {
# This is the subdirectory into which install.pl will install postgresql and
# you can also change this.
    $postgresInstallDir = "$postgresDirWork/pgsql";
    $postgresData = "$postgresInstallDir/data";
}

$RODS_CONFIGURE_OPTIONS="--enable-psghome=$postgresInstallDir";

$postgresBin = "$postgresInstallDir/bin";

if ($SubsetMode lt "1") {
    $createDbOpts="";
}
else {
    # Need to specify the Username when using an existing postgres.
    # You may need to add -W so that it will prompt for a password.
    # You will need to add -h hostname.domain here if postgres is remote.
    $createDbOpts="-U $POSTGRES_ADMIN_NAME";
}

# If you want to redo the installation steps, you can remove the stateFile.
$stateFile="install.state";
$state=0;     # Major state/steps: 
              #  A - build and install postgres
              #  B - build and install odbc
              #  C - build irods
              #  D - configure and run postgres server, create db
              #  E - create the ICAT database
              #  F - finish ICAT and set up user environment

$subState=0;  # Substep within the major state, 1 thru n

$fullStateFile = "$startDir/installLogs/$stateFile";
if (!-e "$startDir/installLogs") {
    mkdir ("$startDir/installLogs", 0700);
}

$uid=$<;
if ($uid eq "0") {
    print "This script should not be run as root.\n";
    print "Postgres will not install as root\n";
    print "and the iRODS server should be run as a non-root user\n";
    die("Running as root");
}

# Needed for Postgres commands:
$ENV{'PGDATA'}="$postgresData";
$oldPath=$ENV{'PATH'};
$ENV{'PATH'}="$postgresBin:$oldPath";
$oldLibPath=$ENV{'LD_LIBRARY_PATH'};  
if (!$oldLibPath) {
#   create LD_LIBRARY_PATH to have postgres
    $ENV{'LD_LIBRARY_PATH'}="$postgresInstallDir/lib";  
}
else {
#   or add it to LD_LIBRARY_PATH (may have GSI libraries defined, for example)
    $ENV{'LD_LIBRARY_PATH'}="$oldLibPath:$postgresInstallDir/lib";
}

# The the OS type and set some parameters accordingly
$thisOS=`uname -s`;
chomp($thisOS);
$gmake="gmake";
$psOptions="-el";
if ($thisOS eq "Darwin") {
# this gmake alias is needed for Darwin
    $gmake="alias gmake=make\ngmake";
    $psOptions="-auxg";
    $thisProcessorType=`uname -p`;
    $intel=0;
    if (index($thisProcessorType,"386")>=0) {
	$intel=1;
    }
}
else {
# On non-Mac OSes, test for gmake
    `$gmake -v`;
    if ($?!=0) {       # No gmake
        $gmake="make"; # Assume gmake is installed as make
    }
}
if ($thisOS eq "SunOS") {
    $psOptions="-ef";
}
$redirectForm=1;
if ($thisOS eq "SunOS" or $thisOS eq "AIX") {
    $redirectForm=2;
}
if ($thisOS eq "Linux") {
# Some Linux hosts use a different shell
    unlink($shTstFile);
    `ls >& $shTstFile`;
    if ($?) { 
	print "Ignore above error message (Bad fd number); it was a test\n";
	print "to determine the correct command (will use 2nd form).\n";
	$redirectForm=2;
    }
    unlink($shTstFile);
}

# Also on OS X, check and possibly increase the stack size from 512K to 2MB
# Skip on Intel Macs as the calls do not work and are not needed (and
# could do something bad).
if ($thisOS eq "Darwin" and !$intel) {
    $stackLimit = getStackLimit();
    if ($stackLimit < 1800000) {
        print "Your current stack size limit is $stackLimit\n";
        print "This is too small for many of the iRODS commands\n";
        print "Increasing stack size limit\n";
        setStackLimit(2000000);
        $stackLimit = getStackLimit();
        print "The stack size limit for this process (and children) is now $stackLimit\n";
        print "You will need it set it for your shell.\n";
        print "For tcsh would be 'limit stacksize 2000'\n";
    }
}

setHostVars(); # set a few host-specific variables

$rodsDone=0;
readState();

if ($usingExistingPostgres) {
    $DB_NAME="MCAT";
    if ($state eq "A" or $state eq "B") {
	$state="C";
	$subState="0";

	printf("This script is configured to use an existing $DB_NAME database and\n");
	printf("will try to drop the iCAT tables (if they exist) and recreate them.\n");
	printf("Enter yes to do so, no to abort:");
	$cmd=<STDIN>;
	chomp($cmd);
	if ($cmd ne "yes") {
	    die("Aborted by user");
	}
	print "chdir'ing to: server/icat/src\n";
	chdir "server/icat/src" || die "Can't chdir to server/icat/src";
	unlink("myinstall.drop.results.1.psg");
	unlink("myinstall.drip.results.2.psg");

	if ($redirectForm eq "2") {
	    runCmdNoLog(0,"$postgresInstallDir/bin/psql $DB_NAME < icatDropSysTables.sql > myinstall.drop.results.1.psg 2>&1");
	    runCmdNoLog(0,"$postgresInstallDir/bin/psql $DB_NAME < icatDropCoreTables.sql > myinstall.drop.results.psg 2>&1");
	}
	else {
	    runCmdNoLog(0,"$postgresInstallDir/bin/psql $DB_NAME < icatDropCoreTables.sql >& myinstall.drop.results.1.psg");
	    runCmdNoLog(0,"$postgresInstallDir/bin/psql $DB_NAME < icatDropSysTables.sql   >& myinstall.drop.results.2.psg");
	}
	print "chdir'ing to: $startDir\n";
	chdir "$startDir" || die "Can't chdir to $startDir";
    }
}

if ($arg1 ne "ps") {
    if (!$ADMIN_PW) {
	if ($state eq "A" or $state eq "B" or $state eq "C" or
	    $state eq "D" or $state eq "E" or $state eq "F") {
	    printf("Enter an iRODS Admin password to use (this will be set for user '$ADMIN_NAME'):");
	    $ADMIN_PW=<STDIN>;
	    chomp($ADMIN_PW);
	    runCmdNoLog(0,"perl config/replines.pl install/install.config 'ADMIN_PW=' '\$ADMIN_PW=\"$ADMIN_PW\";'");
	}
	if (!$DB_PASSWORD) {
	    $DB_PASSWORD = $ADMIN_PW;
	}
    }
}

if ($arg1 eq "drop") {  # drop the db, rebuild iRODS, and reinstall
    printf("Are you sure you want to drop the $DB_NAME database, rebuild iRODS, and restart?\n");
    printf("Enter yes to proceed:");
    $cmd=<STDIN>;
    chomp($cmd);
    if ($cmd ne "yes") {
	die("Cancelled drop");
    }
    runCmdNoLog(1,"$postgresBin/dropdb $DB_NAME");
    stopServers();
    $state="C";
    $subState=0;
    saveState();
    $SubsetMode = "1";
}

if ($state eq "A" or $state eq "B") {
    if (!$arg1) {
	$postmaster=`ps $psOptions | grep postmas | grep -v grep`;
	if ($postmaster) {
	    checkPostgresUser();
	    if ($myPostgres) {
		if (!$POSTGRES_PORT) {
		    print "You are running another postgres on this host already:\n";
		    print "Drop the $DB_NAME database, recreate, and reuse this one?:\n";
		    print "$postmaster";
		    printf("Enter yes to proceed:");
		    $cmd=<STDIN>;
		    chomp($cmd);
		    if ($cmd ne "yes") {
			die("There is an existing postgresql server");
		    }
		    runCmdNoLog(1,"$postgresBin/dropdb $DB_NAME");
		    $state="C";
		    $subState=0;
		    saveState();
		    $SubsetMode = "1";
		    $didPgRestart="1";
		}
	    }
	    else {
		print "There is another postgres already running on this host:\n";
		print "$postmaster";
		die("Exiting");
	    }
	}
    }
}

if ($state eq "A" and $subState eq "0") {
    if (!$arg1) {
	if (-e $postgresInstallDir && !$didPgRestart) {
	    printf("Do you want to reuse and start the postgresql in $startDir/../iRodsPostgres?\n");
	    print "And drop the $DB_NAME database, recreate it?\n";
	    printf("Enter yes to do so, no to rebuild:");
	    $cmd=<STDIN>;
	    chomp($cmd);
	    if ($cmd eq "yes") {
		runCmdNoLog(0, "$postgresBin/pg_ctl start -o '-i' -l $postgresInstallDir/pgsql.log");
		printf("Sleeping briefly\n");
		sleep(2);
		runCmdNoLog(1,"$postgresBin/dropdb $DB_NAME");
		runCmdNoLog(1, "$postgresBin/pg_ctl stop");
		printf("Sleeping briefly\n");
		sleep(2);
		runCmdNoLog(0, "rm -rf $postgresData");
		$state="C";
		$subState=0;
		saveState();
	    }
	}
    }
}

if ($arg1 eq "stop") {
    stopServers();
    die ("Done");
}
if ($arg1 eq "clean") {
    print "Do you really want to stop processes and remove the entire\n";
    print "installation; everything that was built with install.pl?\n";
    printf("Enter yes to proceed:");
    $cmd=<STDIN>;
    chomp($cmd);
    if ($cmd ne "yes") {
	die("Aborted by user");
    }
    stopServers();

    print "The following commands are about to be run:\n";
    print "   rm -rf $RESOURCE_DIR\n";
    print "   rm -f server/icat/src/myinstall*\n";
    print "   rm -f installLogs/install*.log\n";
    print "   rm -f $fullStateFile\n";
    print "   $gmake clean\n";
    printf("Enter yes (again) to do so and remove these directories and binaries:");
    $cmd=<STDIN>;
    chomp($cmd);
    if ($cmd ne "yes") {
	die("Aborted by user");
    }
    runCmdNoLog(0, "rm -rf $RESOURCE_DIR");
    runCmdNoLog(0, "rm -f server/icat/src/myinstall*");
    runCmdNoLog(0, "rm -f installLogs/install*.log");
    runCmdNoLog(0, "rm -f $fullStateFile");
    runCmdNoLog(0, "$gmake clean");

    die("RODS installation removed");
}
if ($arg1 eq "start") {
    $Servers = "";
    if ($SubsetMode lt "1") {
	runCmdNoLog(0, "$postgresBin/pg_ctl start -o '-i' -l $postgresInstallDir/pgsql.log");
	$Servers = "Postgres";
    }
    if ($rodsDone eq "1") {
    # have completed iRODS installation, start it too
	printf("sleeping briefly\n");
	sleep(2);
	startIrodsServer();
    }
    die("Done starting " . $Servers . " servers");
}

if ($arg1 eq "ps") {
    $rods=`ps $psOptions | grep rods | grep -v grep`;
    print "Running irods server processes:\n";
    print $rods;
    $post=`ps $psOptions | grep post | grep -v grep`;
    print "Running postgres server processes:\n";
    print $post;
    die("Done listing processes");
}

if ($arg1 eq "vacuum" or $arg1 eq "v")  {
    stopIrodsServer();  # to avoid vacuumdb hanging on a semaphore
    runCmdNoLog(0, "$postgresBin/vacuumdb -f -z $DB_NAME");
    print $cmdStdout;
    printf("sleeping briefly\n");
    sleep(2);
    startIrodsServer();
    printf("Done running postgresql vacuumdb");
    exit 0;
}

if ($arg1 eq "test")  {
    if ($state ne "G") {
	print "The iRODS system must be completely installed\n";
	print "before you can run the test command.\n";
	die("Not ready for 'test'");
    }
    runTests();
    exit 0;
}

if ($arg1) {
    if ($arg1 ne "drop") {  # drop is the only command line argument that
                            # falls thru to here, so if it is not drop, it's
                            # invalid.
	usage();
	die("invalid command line");
    }
}

if ($state eq "A") {
    buildPostgres();
}

if ($state eq "B") {
    if ($unixODBC) {
	buildUnixOdbc();
    }
    else {
	buildOdbc();
    }
}

if ($state eq "C") {
    buildIrods();
}

if ($state eq "D") {
    runPostgres();
}

if ($state eq "E") {
    installIcatDB();
}

if ($state eq "F") {
    finishIcatAndSetupEnv();
}

# No remote zones yet
#if ($state eq "G") {
#    doZones();
#}

print "To use the iRODS icommands set your path to include the binaries:\n";
print "  set path=($startDir/clients/icommands/bin \$path)\n";
print "and then iinit, iput, ils, etc should work.\n";
print "Run 'install.pl help' for a list of install.pl options (ps, stop, start, etc).\n";
print "If your ICAT becomes slow, try running 'install.pl vacuum'.\n";
print "To improve security, you may want to remove ~/.pgpass which has the\n";
print "postgresql admin password.\n";
print "All done\n";

# set up $hostName and $hostFullNetName and $hostFullNetAddr
sub setHostVars() {
    if ($LOCALHOST) {
#       user specified to use localhost loopback, just set parameters
	$hostName = "localhost";
	$hostFullNetName = "localhost";
	$hostFullNetAddr = "127.0.0.1";
    }
    else {
	$hostName = `uname -n`; # on my Mac, this gives for example: dhcp-mac-016.sdsc.edu
	chomp($hostName);
	if ($thisOS eq "SunOS") {
	    $tmp=`nslookup $hostName`;
	    $i = index($tmp,"Name:");
	    $tmp = substr($tmp,$i);
	    $j = index($tmp, "Address:");
	    $hostFullNetName = substr($tmp,5,$j-5); # e.g. zuri.sdsc.edu
	    chomp($hostFullNetName);
	    chomp($hostFullNetName);
	    $i = rindex($hostFullNetName, " ");
	    $hostFullNetName = substr($hostFullNetName, $i+1);
	    $hostFullNetName =~ s/\t//;
	    $i = index($tmp, "Alias");
	    if ($i > 0) {
		$tmp=substr($tmp,0,$i);
	    }
	    $i = rindex($tmp," ");
	    $hostFullNetAddr = substr($tmp,$i+1);  # e.g. 132.249.32.192
	    chomp($hostFullNetAddr);
	    chomp($hostFullNetAddr);
	}
	else {
# run host to get full host.domain name, which
# returns for example: zuri.sdsc.edu has address 132.249.32.192
# grep for "has address" to avoid extra lines on some hosts (such as
# "mail is handled by")
	    $tmp=`host $hostName | grep "has address"`;  
	    $i = index($tmp, "has address");
	    if ($i > 0) {
		$i = index($tmp," ");
		$hostFullNetName = substr($tmp,0,$i); # zuri.sdsc.edu
		$i=rindex($tmp, " ");
		$testchar = substr($tmp, $i, 1);
		if ($testchar ne " ") {
# This seems to be an perl bug on some hosts.
# But the following workaround seems to work.
# My guess is that it is a bug in perl string-buffer management.
		    print "Perl rindex error detected; retrying\n";
		    print "testchar=:$testchar:\n";
		    $tmp3=$tmp;
		    $i5 = rindex($tmp3," ");
		    print "initial i=$i, retry i=$i5\n";
		    if ($i == $i5) {
			print "Retry failed, Perl rindex failed";
			die ("perl rindex problem");
		    }
		    print "rindex workaround succeeded (it appears), continuing\n";
		    $i=$i5;
		}
		$hostFullNetAddr = substr($tmp,$i+1);  # 132.249.32.192
		chomp($hostFullNetAddr);
	    }
	    else {
		$tmp=`host $hostName | grep "Aliases:"`;  
		$i = index($tmp, "Aliases:");
		if ($i < 0) {
#                   try ping (works at nmi)
		    $ping=`ping -c1 -n $hostName`;
		    $i = index($ping, "bytes from ");
		    $j = index($ping, ":", $i);
		    $k = $i + 11; # skip past "bytes from "
		    $ii = index($ping, "PING ");
		    $jj = index($ping, "(");
		    if ($i > 0 and $j > $k and $ii >=0 and $jj > $ii) {
			$hostFullNetName=substr($ping, $ii+5, $jj-$ii-5-1);
			$hostFullNetAddr = substr($ping, $k, $j-$k);
		    }
		    else {
			print "Lookup of the local host, $hostName, failed\n";
			die "Host DNS lookup failed";
		    }
#		    print "using localhost.\n";
#		    $hostFullNetName="localhost.localdomain";
#		    $hostFullNetAddr = "127.0.0.1";
		}
		else {
		    $k = index($tmp,$hostName,$i);
		    $j = index($tmp,",",$k);
		    $hostFullNetName = substr($tmp,$k,$j-$k); # zuri.sdsc.edu
		    $i = index($tmp,",");
		    $j = rindex($tmp," ",$i);
		    $hostFullNetAddr=substr($tmp,$j+1,$i-$j-1);#132.249.32.192
		    chomp($hostFullNetAddr);
		}
	    }
	}
    }
    print "This host is $hostName\n";
    print "This host full network name is $hostFullNetName\n";
    print "This host full network address is $hostFullNetAddr\n";
    $homeDir=$ENV{'HOME'};
    print "Your home directory is $homeDir\n";
    if (!$homeDir) {  # No home, so set it to here
	$ENV{'HOME'}=$startDir;
	$homeDir=$startDir;
	print "For while this script runs, your home directory is $homeDir\n";
    }
}

# Finish updating the ICAT and set up the user environment.
# This includes running the the server using the boot admin, then stopping
# it, and then restarting it as the final admin.
sub finishIcatAndSetupEnv() {

# Adjust the server.config file with local settings
    if ($subState eq "0") {
	runCmdNoLog(0, "$clBin/iadmin spass $DB_PASSWORD $DB_KEY"); # scramble pw
	print $cmdStdout;
	$scrambledPW=substr($cmdStdout, index("$cmdStdout", "Scrambled form is:")+18);
	chomp($scrambledPW);
	$txt = `cat server/config/server.config | grep -v 'icatHost zuri' | grep -v DBUser`;
	unlink ("server/config/server.config");
	writeFile("server/config/server.config", $txt);
	appendToFile("server/config/server.config", "icatHost  $hostFullNetName\nDBUsername $thisUser\nDBPassword $scrambledPW\nDBKey $DB_KEY");
	runCmdNoLog(0, "chmod 600 server/config/server.config");
	$subState++;
	saveState();
    }

# Set up the environment to use the bootstrap admin acct, start the server,
# and add the main collections (directories) via the admin tool.

    if ($subState eq "1") {
	$ENV{'irodsEnvFile'}="$startDir/install/irodsEnv.boot";
	startIrodsServer();
	printf("Sleeping briefly\n");
	sleep(2);
	runCmdNoLog(0, "$clBin/iinit $ADMIN_BOOT_PW");
#	runCmdNoLog(0, "$clBin/ils"); # test password and communication
#	runCmdNoLog(0, "$clBin/iadmin mkdir /home");
#	runCmdNoLog(0, "$clBin/iadmin mkdir /trash");
#	runCmdNoLog(0, "$clBin/iadmin mkdir /trash/home");
#	runCmdNoLog(0, "$clBin/iadmin mkdir /trash/$ZONE_NAME");
#	runCmdNoLog(0, "$clBin/iadmin mkdir /trash/$ZONE_NAME/home");
	runCmdNoLog(0, "$clBin/iadmin mkdir /$ZONE_NAME");
	runCmdNoLog(0, "$clBin/iadmin mkdir /$ZONE_NAME/home");
	runCmdNoLog(0, "$clBin/iadmin mkdir /$ZONE_NAME/trash");
	runCmdNoLog(0, "$clBin/iadmin mkdir /$ZONE_NAME/trash/home");
	runCmdNoLog(0, "$clBin/iexit full");
	$subState++;
	saveState();
    }

# Set up the admin account and reset the passwords for both admin accts
    if ($subState eq "2") {
	$ENV{'irodsEnvFile'}="$startDir/install/irodsEnv.boot";
	runCmdNoLog(0, "$clBin/iinit $ADMIN_BOOT_PW");
#	runCmdNoLog(0, "$clBin/ils"); # test it
	runCmdNoLog(1, "$clBin/iadmin mkuser $ADMIN_NAME rodsadmin");
	runCmdNoLog(0, "$clBin/ichmod own $ADMIN_NAME /$ZONE_NAME /$ZONE_NAME/home /$ZONE_NAME/trash /$ZONE_NAME/trash/home");
	runCmdNoLog(0, "$clBin/iadmin moduser $ADMIN_NAME password $ADMIN_PW");
	runCmdNoLog(0, "$clBin/iadmin moduser $ADMIN_NAME_BOOT password $ADMIN_PW");
	runCmdNoLog(0, "$clBin/iexit full");
	$subState++;
	saveState();
    }

# Create the .irods directory and env file
    if ($subState eq "3") {
 	if (-e "$homeDir/.irods/.irodsEnv") {
	    printf("You already have a ~/.rods/.rodsEnv, appending to it.\n");
            printf("Please check it later.\n");
	    printf("Updating ");
 	}
 	else {
	    print "mkdir'ing: $homeDir/.rods\n";
 	    mkdir("$homeDir/.irods", 0700);
	    printf("Creating ");
	}
	printf("$homeDir/.irods/.irodsEnv file\n");
	$date=`date`;
	chomp($date);
	appendToFile("$homeDir/.irods/.irodsEnv", "# Lines below here added by install.pl on $date :\nirodsHost \'$hostFullNetName\'\nirodsPort $rodsPort\nirodsDefResource=$RESOURCE_NAME\nirodsHome=/$ZONE_NAME/home/$ADMIN_NAME\nirodsCwd=/$ZONE_NAME/home/$ADMIN_NAME\nirodsUserName '$ADMIN_NAME'\nirodsZone '$ZONE_NAME'\n");
	$subState++;
	saveState();
    }

    $ENV{'irodsEnvFile'}=''; # Now start using the new user account, etc

# Stop and restart the server under the new environment
    if ($subState eq "4") {
	stopIrodsServer();
	startIrodsServer();
	printf("Sleeping briefly\n");
	sleep(2);
	$subState++;
	saveState();
    }

# create the resource
    if ($subState eq "5") {
	runCmdNoLog(0, "$clBin/iinit $ADMIN_PW");
        # iadmin mkresc Name Type Class netAddr defPath Zone (make Resource)
	runCmdNoLog(0, "$clBin/iadmin mkresc $RESOURCE_NAME 'unix file system' archive $hostFullNetName $RESOURCE_DIR $ZONE_NAME");
	$subState++;
	saveState();
    }

# Simple put/get test
    if ($subState eq "6") {
	unlink("testFile1");
	writeFile("testFile1", "adsfsafdjlkasdfalsdfjlasdjfs");
	runCmdNoLog(0, "$clBin/iinit $ADMIN_PW");
	runCmdNoLog(0, "$clBin/iput testFile1");
	runCmdNoLog(0, "mv testFile1 testFile1.orig");
	runCmdNoLog(0, "$clBin/iget testFile1");
	runCmdNoLog(0, "diff testFile1 testFile1.orig");
	runCmdNoLog(0, "$clBin/irm testFile1");
	unlink("testFile1");
	unlink("testFile1.orig");
	$subState++;
	saveState();
    }

    $state++;
    $subState=0;
    saveStateQuiet();
    chdir "$startDir" || die "Can't chdir to $startDir";

    return; 
}


sub installIcatDB() {

#   Run ICAT postgres table insert script

    if ($subState eq "0" and $SubsetMode lt "2") {
	print "chdir'ing to: server/icat/src\n";
	chdir "server/icat/src" || die "Can't chdir to server/icat/src";
	unlink("myinstall.results.1.psg");
	unlink("myinstall.results.2.psg");
	unlink("myinstall.results.3.psg");
	unlink("myinstall.results.4.psg");
	if ($redirectForm eq "2") {
	    runCmdNoLog(0,"$postgresInstallDir/bin/psql $DB_NAME < icatCoreTables.sql > myinstall.results.1.psg 2>&1");
	    runCmdNoLog(0,"$postgresInstallDir/bin/psql $DB_NAME < icatSysTables.sql > myinstall.results.2.psg 2>&1");
	    runCmdNoLog(0,"$postgresInstallDir/bin/psql $DB_NAME < icatCoreInserts.sql > myinstall.results.3.psg 2>&1");
	    runCmdNoLog(0,"$postgresInstallDir/bin/psql $DB_NAME < icatSysInserts.sql > myinstall.results.4.psg 2>&1");
	}
	else {
	    runCmdNoLog(0,"$postgresInstallDir/bin/psql $DB_NAME < icatCoreTables.sql >& myinstall.results.1.psg");
	    runCmdNoLog(0,"$postgresInstallDir/bin/psql $DB_NAME < icatSysTables.sql   >& myinstall.results.2.psg");
	    runCmdNoLog(0,"$postgresInstallDir/bin/psql $DB_NAME < icatCoreInserts.sql >& myinstall.results.3.psg");
	    runCmdNoLog(0,"$postgresInstallDir/bin/psql $DB_NAME < icatSysInserts.sql  >& myinstall.results.4.psg");
	}
	$testGrep=`grep -i error myinstall.results*`;
	if ($testGrep) {
	    die "Error inserting tables and indexes, ERROR string in logs";
	}
	$subState++;
	saveState();
	print "chdir'ing to: $startDir\n";
	chdir "$startDir" || die "Can't chdir to $startDir";
    }

# Set up a odbcinst.ini file in the postgresInstallDir/etc directory.
# Normally postgres will use a ~/.odbc.ini file but it can also use
# this which is more convenient for our installation.

    if ($subState eq "1") {
# Need to move old .odbc.ini file if it exists since it would override
# the settings in the .../etc/odbcinst.ini or .../etc/odbc.ini file.
	if (-e "$homeDir/.odbc.ini") {
           $dateStr=`date | sed 's/ /_/g'`;
           chomp($dateStr);
           runCmdNoLog(1,"mv $homeDir/.odbc.ini $homeDir/.odbc.ini.old.$dateStr");
        }

	if ($SubsetMode lt "1") {
	    print "mkdir'ing: $postgresInstallDir/etc\n";
	    mkdir("$postgresInstallDir/etc", 0766);

	    if ($unixODBC) {
		runCmdNoLog(0,"echo '[PostgreSQL]\nDriver=$postgresInstallDir/lib/libodbcpsql.so\nDebug=0\nCommLog=0\nServername=$hostFullNetName\nDatabase=$DB_NAME\nReadOnly=no\n' > $postgresInstallDir/etc/odbc.ini");
	    }
	    else {
		runCmdNoLog(0,"echo '[PostgreSQL]\nDebug=0\nCommLog=0\nServername=$hostFullNetName\nDatabase=$DB_NAME\n' > $postgresInstallDir/etc/odbcinst.ini");
	    }
	    if ($POSTGRES_PORT) {
		runCmdNoLog(0,"echo 'Port=$POSTGRES_PORT\n' >> $postgresInstallDir/etc/odbcinst.ini");
	    }
	}

	$subState++;
	saveState();
    }

#   Test communication to postgres
    if ($subState eq "2") {
	runCmd(0,"./server/test/bin/test_cll $thisUser $DB_PASSWORD");
	$subState++;
	saveState();
    }

# (may want to have some ICAT tests here someday)

    $state++;
    $subState=0;
    chdir "$startDir" || die "Can't chdir to $startDir";
    return; 

}

sub runPostgres() {
    if ($SubsetMode ge "1") {
	print "Skipping all runPostgres steps as this script is configured to use an existing one.\n";
	$state++;
	$subState=0;
	chdir "$startDir" || die "Can't chdir to $startDir";
	return;
    }

    if ($subState eq "0") {
        # Previously, this script would create the data subdir, but
        # that can cause problems in that the mode can't be set right
        # and initdb would fail trying to chmod it.  Seems to work OK
        # to just skip the mkdir.

	if ($thisOS eq "Darwin") {
# On Macs, lc-collate=C isn't needed and caused problems with not enough
# shared memory.
	    runCmd(0,"$postgresBin/initdb -D $postgresData");
	}
	else {
# Include --lc-collate=C to make sure postgres returns sorted items
# in the order needed.
	    runCmd(0,"$postgresBin/initdb --lc-collate=C -D $postgresData");
	}

	if ($thisOS eq "Darwin") {
        # Mac gets error in postges starting if LC_TIME isn't commented out
	    unlink("$postgresData/postgresql.conf.new");
	    runCmdNoLog(0, "cat $postgresData/postgresql.conf | sed s/LC_TIME/#LC_TIME/g > $postgresData/postgresql.conf.new");
            runCmdNoLog(0, "cp -f $postgresData/postgresql.conf.new $postgresData/postgresql.conf");
	} 
	$subState++;
	saveState();
    }

    if ($subState eq "1") {
#       Set up pg config so odbc will need to use md5-password
	runCmdNoLog(0,"echo host all all $hostFullNetAddr 255.255.255.255 md5 | cat >> $postgresData/pg_hba.conf");
	$subState++;
	saveState();
    }

    if ($subState eq "2") {
	if (-e "$homeDir/$POSTGRES_PW_FILE") {
	    printf("Warning, moving your existing $POSTGRES_PW_FILE.");
	    unlink("$homeDir/$POSTGRES_PW_FILE.orig");
	    runCmdNoLog(0, "mv $homeDir/$POSTGRES_PW_FILE $homeDir/$POSTGRES_PW_FILE.orig");
	}
	if ($POSTGRES_PORT) {
	    $thePort=$POSTGRES_PORT;
	}
	else {
	    $thePort=$POSTGRES_DEFAULT_PORT;
	}
	printf("Writing $homeDir/$POSTGRES_PW_FILE\n");
	writeFile("$homeDir/$POSTGRES_PW_FILE", "*:$thePort:$DB_NAME:$thisUser:$DB_PASSWORD");
	runCmdNoLog(0, "chmod 600 $homeDir/$POSTGRES_PW_FILE");
	$subState++;
	saveState();
    }

    if ($subState eq "3") {
        runCmdNoLog(0, "$postgresBin/pg_ctl start -o '-i' -l $postgresInstallDir/pgsql.log");
	$subState++;
	saveState();
	print "sleeping a few seconds\n"; # 8.0.1 seems to need > 2
	sleep 6;
    }
    if ($subState eq "4") {
	runCmd(0,"$postgresBin/createdb $createDbOpts $DB_NAME");
	$subState++;
	saveState();
    }

    if ($subState eq "5") {
	unlink("irodsPGtempInput.tmp");
	writeFile("irodsPGtempInput.tmp", "alter user $thisUser with password '$DB_PASSWORD'");
	runCmd(0,"$postgresBin/psql $DB_NAME < irodsPGtempInput.tmp");
	unlink("irodsPGtempInput.tmp");
	$subState++;
	saveState();
    }

    if ($subState eq "6") {
# This is needed on some hosts (noticed at NMI build and test) to avoid 
# 'client encoding mismatch' with ODBC.
	unlink("irodsPGtempInput.tmp");
	writeFile("irodsPGtempInput.tmp", "alter user $thisUser set client_encoding to LATIN1");
	runCmd(0,"$postgresBin/psql $DB_NAME < irodsPGtempInput.tmp");
	unlink("irodsPGtempInput.tmp");
	$subState++;
	saveState();
    }

# Set up pg config so psql will also need to use a password
    if ($subState eq "7") {
	runCmdNoLog(0,"perl config/replines.pl $postgresData/pg_hba.conf 'local   all' 'local   all         all                               md5'");

	$subState++;
	saveState();
    }

    $state++;
    $subState=0;
    chdir "$startDir" || die "Can't chdir to $startDir";
}

sub buildPostgres() {
    test64Addr();  # before starting, check on 64-bit addressing

    print "chdir'ing to: $postgresDirWork\n";
    mkdir($postgresDirWork, 0700);
    chdir "$postgresDirWork" || die "Can't chdir to $postgresDirWork";

    if ($SubsetMode ge "1") {
	print "Skipping postgres build as this script is configured to use an existing one.\n";
	$state++;
	$subState=0;
	chdir "$startDir" || die "Can't chdir to $startDir";
	return;
    }

    $PG2=substr($POSTGRES_FILE,0,-3);
    if (!-e $POSTGRES_FILE && !-e $PG2) {
	if (-e "ftp.in") {
	    unlink("ftp.in");
	}
	$pgVersion="8.2.4";
	$ix=index($POSTGRES_FILE, ".tar");
	$ix2=index($POSTGRES_FILE, "postgresql-");
	if ($ix > 0 && $ix2 == 0) {
	    $pgVersion=substr($POSTGRES_FILE, 11, $ix-11); 
               # the '11' is the length of the string 'postgresql-'
	}
	printf("Getting $POSTGRES_FILE via FTP\n");
	runCmdNoLog(0, "echo 'user anonymous schroede\@sdsc.edu\ncd postgresql/source/v$pgVersion\nbin\nget $POSTGRES_FILE\nquit' > ftp.in");
	runCmdNoLog(0, "ftp -n ftp8.us.postgresql.org < ftp.in");
	unlink("ftp.in");
    }
    if ($subState eq "0") {
	$subState++;
	saveState();
    }

    if (rindex($POSTGRES_FILE,"gz") gt 0) {
	if ($subState eq "1") {
	    if (-e $postgresTarFile) {
		print "Skipping gzip -d as the uncompressed tar file already exists\n";
	    }
	    else {
		runCmd(0,"gzip -d $POSTGRES_FILE");
	    }
	}
    }
    else {
	print "Skipping gzip -d as the tar file is already uncompressed\n";
    }
    if ($subState eq "1") {
	$subState++;
	saveState();
    }

    if ($subState eq "2") {
	runCmd(0,"tar xf $postgresTarFile");
	$subState++;
	saveState();
    }

    print "chdir'ing to: $postgresDirFull\n";
    chdir "$postgresDirFull" || die "Can't chdir to $postgresDirFull";

    if ($subState eq "3") {
        $postgresConf="./configure --prefix=$postgresInstallDir --enable-odbc --without-readline";
	if ($thisOS eq "SunOS") {
            $postgresConf = "$postgresConf" . " --without-zlib";
	}
	if ($POSTGRES_PORT) {
            $postgresConf = "$postgresConf" . " --with-pgport=$POSTGRES_PORT";
	}
        runCmd(0,"$postgresConf");
	$subState++;
	saveState();
    }

    if ($subState eq "4") {
	runCmd(0,"$gmake");
	$subState++;
	saveState();
    }

    if ($subState eq "5") {
	runCmd(0,"$gmake install");
	$subState++;
	saveState();
    }

    $state++;
    $subState=0;
    chdir "$startDir" || die "Can't chdir to $startDir";
}

sub buildOdbc() {

    print "chdir'ing to: $postgresDirWork\n";
    chdir "$postgresDirWork" || die "Can't chdir to $postgresDirWork";

    if ($SubsetMode ge "1") {
	print "Skipping odbc build as this script is configured to use an existing one\n";
	$state++;
	$subState=0;
	chdir "$startDir" || die "Can't chdir to $startDir";
	return;
    }

    $ODBC2=substr($ODBC_FILE,0,-3);
    if (!-e $ODBC_FILE && !-e $ODBC2) {
	if (-e "ftp.in") {
	    unlink("ftp.in");
	}

	printf("Getting $ODBC_FILE via FTP\n");
	runCmdNoLog(0, "echo 'user anonymous schroede\@sdsc.edu\ncd postgresql/odbc/versions/src\nbin\nget $ODBC_FILE\nquit' > ftp.in");
	runCmdNoLog(0, "ftp -n ftp8.us.postgresql.org < ftp.in");
	unlink("ftp.in");
    }
    if ($subState eq "0") {
	$subState++;
	saveState();
    }


    $odbcTarFile = $ODBC_FILE;
    if (rindex($ODBC_FILE,"gz") gt 0) {
	$odbcTarFile = substr($ODBC_FILE,0,-3);
	if ($subState eq "1") {
	    if (-e $odbcTarFile) {
		print "Skipping gzip -d as the uncompressed tar file already exists\n";
	    }
	    else {
		runCmd(0,"gzip -d $ODBC_FILE");
	    }
	}
    }
    if ($subState eq "1") {
	$subState++;
	saveState();
    }
    $odbcDir=substr($odbcTarFile,0,-4);

    $postgresInt="$postgresDirFull/src/interfaces";

    print "chdir'ing to: $postgresInt\n";
    chdir "$postgresInt" || die "Can't chdir to $postgresInt";

    print "mkdir'ing: odbc\n";
    mkdir("odbc", 0700);
    print "chdir'ing to: odbc\n";
    chdir "odbc" || die "Can't chdir to odbc";

    if ($subState eq "2") {
	runCmd(0,"tar xf $postgresDirWork/$odbcTarFile");
	runCmdNoLog(0,"mv $odbcDir/* .");
	$subState++;
	saveState();
    }

    if ($subState eq "3") {
	$configLine="./configure --prefix=$postgresInstallDir --enable-static";
	if ($thisOS eq "SunOS") {
	    $configLine=$configLine . " --disable-pthreads";
	}
	runCmd(0,"$configLine");
	if ($thisOS eq "SunOS") {
# Patch ODBC's Makefile to include the needed nsl and socket libraries
           if (`grep nsl Makefile` eq "") {
	       runCmdNoLog(0,"cat Makefile | sed 's/LIBS =/LIBS =-lnsl -lsocket/g' > Makefile2");
	       unlink("Makefile");
	       runCmdNoLog(0,"mv Makefile2 Makefile");
	   }
	} 
	$subState++;
	saveState();
    }

    if ($subState eq "4") {
	runCmd(0,"$gmake");
	$subState++;
	saveState();
    }

    if ($subState eq "5") {
	runCmd(0,"$gmake install");
	if (-e "$postgresInstallDir/lib/psqlodbc.a") { # if psqlodbc.a  exists
#           copy it to proper lib file (seems to be needed on some machines).
	    runCmd(0,"cp $postgresInstallDir/lib/psqlodbc.a $postgresInstallDir/lib/libpsqlodbc.a");
	}
	runCmd(0,"cp iodbc.h isql.h isqlext.h $postgresInstallDir/include");
	$subState++;
	saveState();
    }

    if ($thisOS eq "Darwin") {
	if ($subState eq "6") {
# Thru trial and error, I found that these commands will create a link
# library with no missing externals.  This cp is a hack, but I'm not
# sure the right way to create what we want (it avoids a missing
# external of _globals).  Also, this whole thing of creating a .a file
# from all the .o's via libtool is odd, but the odbc configure/make
# doesn't seem to handle it right.
	    runCmdNoLog(0,"cp psqlodbc.lo psqlodbc.o");   
	    runCmdNoLog(0,"libtool -o libpsqlodbc.a *.o");
	    runCmdNoLog(0,"cp libpsqlodbc.a $postgresInstallDir/lib");
	    chdir "$postgresInstallDir/lib" || die "Can't chdir to $postgresInstallDir/lib";
	    runCmdNoLog(0,"ranlib libpsqlodbc.a");
	    $subState++;
	    saveState();
	}
    }

    $state++;
    $subState=0;
    chdir "$startDir" || die "Can't chdir to $startDir";
}

sub buildUnixOdbc() {

    print "chdir'ing to: $postgresDirWork\n";
    chdir "$postgresDirWork" || die "Can't chdir to $postgresDirWork";

    if ($SubsetMode ge "1") {
	print "Skipping odbc build as this script is configured to use an existing one\n";
	$state++;
	$subState=0;
	chdir "$startDir" || die "Can't chdir to $startDir";
	return;
    }

    if ($subState eq "0") {
	$ODBC2=substr($ODBC_FILE,0,-3);
	if (!-e $ODBC_FILE && !-e $ODBC2) {
	    printf("Getting $ODBC_FILE via wget\n");
	    runCmd(0, 
               "wget http://www.unixodbc.org/$ODBC_FILE");
	}
	$subState++;
	saveState();
    }

    $odbcTarFile = $ODBC_FILE;
    if (rindex($ODBC_FILE,"gz") gt 0) {
	$odbcTarFile = substr($ODBC_FILE,0,-3);
	if ($subState eq "1") {
	    if (-e $odbcTarFile) {
		print "Skipping gzip -d as the uncompressed tar file already exists\n";
	    }
	    else {
		runCmd(0,"gzip -d $ODBC_FILE");
	    }
	}
    }

    if ($subState eq "1") {
	$subState++;
	saveState();
    }

    $odbcDir=substr($odbcTarFile,0,-4);
    $odbcFull="$postgresDirWork/$odbcDir";
    if ($subState eq "2") {
	if (-e $odbcFull) {
	    print "Skipping tar xf $odbcTarFile as $odbcFull already exists\n";
	}
	else {
	    runCmd(0,"tar xf $odbcTarFile");
	}
	$subState++;
	saveState();
    }

    print "chdir'ing to: $odbcFull\n";
    chdir "$odbcFull" || die "Can't chdir to $odbcFull";
    if ($subState eq "3") { 
        $configLine = "./configure --enable-gui=no --prefix=$postgresInstallDir  --enable-static";
	if ($thisOS eq "SunOS") {
	    $configLine=$configLine . " --disable-pthreads"; # Needed?
	}
	runCmd(0,"$configLine");
	if ($thisOS eq "SunOS") {
# Patch ODBC's Makefile to include the needed nsl and socket libraries
#           if (`grep nsl Makefile` eq "") {
#	       runCmdNoLog(0,"cat Makefile | sed 's/LIBS =/LIBS =-lnsl -lsocket/g' > Makefile2");
#	       unlink("Makefile");
#	       runCmdNoLog(0,"mv Makefile2 Makefile");
#	   }
	} 
	$subState++;
	saveState();
    }

    if ($subState eq "4") {
	runCmd(0,"$gmake");
	$subState++;
	saveState();
    }

    if ($subState eq "5") {
	runCmd(0,"$gmake install");
	$subState++;
	saveState();
    }

    $state++;
    $subState=0;
    chdir "$startDir" || die "Can't chdir to $startDir";
}

sub buildIrods() {

    if ($subState eq "0") {
	$RODS_CONFIGURE_OPTIONS_PORT="";
	if ($RODS_PORT) {
	    $RODS_CONFIGURE_OPTIONS_PORT="--enable-rodsport=" . "$RODS_PORT";
	}
	if ($unixODBC) {
	    $ODBC_OPT="--enable-newodbc";
	}
	else {
	    $ODBC_OPT="--enable-oldodbc";
	}
	runCmd(0,"./configure $RODS_CONFIGURE_OPTIONS $ODBC_OPT $RODS_CONFIGURE_OPTIONS_PORT");
	$subState++;
	saveState();
    }

    if ($subState eq "1") {
	runCmd(0,"$gmake");
	$subState++;
	saveState();
    }


    if ($subState eq "2") {
	chdir "$startDir/clients/icommands" || 
	    die "Can't chdir to $startDir/clients/icommands";
	runCmd(0,"$gmake");
	$subState++;
	saveState();
    }

    $state++;
    $subState=0;
    chdir "$startDir" || die "Can't chdir to $startDir";
}

# run a command, and log its output to a file.
# if option is 0 (normal), check the exit code 
sub runCmd {
    my($option,$cmd) = @_;
    $workingState=$subState+1; # State/step we are currently working on
    $fullCmd = "$cmd > $startDir/installLogs/install$state$workingState.log";
    print "running: $fullCmd \n";
    `$fullCmd`;
    $cmdStat=$?;
    if ($option == 0) {
	if ($cmdStat!=0) {
	    print "The following command failed:";
	    print "$fullCmd \n";
	    print "Exit code= $cmdStat \n";
	    die("command failed");
	}
    }
}

# run a command (without a log)
# if option is 0 (normal), check the exit code 
sub runCmdNoLog {
    my($option, $cmd) = @_;
    print "running: $cmd \n";
    $cmdStdout=`$cmd`;
    $cmdStat=$?;
    if ($option == 0) {
	if ($cmdStat!=0) {
	    print "The following command failed:";
	    print "$cmd \n";
	    print "Exit code= $cmdStat \n";
	    die("command failed");
	}
    }
}


# run a command once or twice, and log its output to a file.
# If the first attempt fails, sleep a second and try again; if it
# fails then, it fails.
sub runCmdTwice {
    my($option,$cmd) = @_;
    $workingState=$subState+1; # State/step we are currently working on
    $fullCmd = "$cmd > $startDir/installLogs/install$state$workingState.log";
    print "running: $fullCmd \n";
    `$fullCmd`;
    $cmdStat=$?;
    if ($option == 0) {
	if ($cmdStat!=0) {
	    print "First try failed, sleeping a second to try again\n";
	    sleep 1;
	    $fullCmd = "$cmd >> $startDir/install$state$workingState.log";
	    `$fullCmd`;
	    $cmdStat=$?;
	    if ($cmdStat!=0) {
		print "The following command failed:";
		print "$fullCmd \n";
		print "Exit code= $cmdStat \n";
		die("command failed");
	    }
	}
    }
}


sub saveState {
    open(F, ">$fullStateFile");
    $_ = $state;
    print F;
    $_ = $subState;
    print F;
    close(F);
    $dateStr=`date`;
    chomp($dateStr);
    print "Step $state $subState completed at $dateStr.\n";
}

sub saveStateQuiet {
    open(F, ">$fullStateFile");
    $_ = $state;
    print F;
    $_ = $subState;
    print F;
    close(F);
}

sub readState {
    if (open(F, "<$fullStateFile")) {
	read(F, $state, 1);
	read(F,$subState,2);
	close(F);
	chomp($subState); # remove possible trailing \n (vi forces one)
    }
    else {
	$state="A";
	$subState=0;
    }
    if ($state eq "G" or $state eq "H") {
	$rodsDone=1;
    }
}

sub writeFile {
    my($file, $text) = @_;
    open(F, ">$file");
    $_ = $text;
    print F;
    close F;
}

sub appendToFile {
    my($file, $text) = @_;
    open(F, ">>$file");
    $_ = $text;
    print F;
    close F;
}

sub getStackLimit {
    $SYS_getrlimit=194; # from /usr/include/sys/syscall.h
    $RLIMIT_STACK=3;    # from /usr/include/sys/resource.h
    $rlimit = pack(i2,0,0);
    $f=syscall($SYS_getrlimit,$RLIMIT_STACK,$rlimit);
    if ($f != 0) {
	print "Warning, syscall to getrlimit failed\n";
	return("0");
    }
    my($result1, $result2)=unpack(i2,$rlimit);
    return($result2);
}

sub setStackLimit {
    my($newValue)= @_;
    $SYS_setrlimit=195; # from /usr/include/sys/syscall.h
    $RLIMIT_STACK=3;    # from /usr/include/sys/resource.h
    $rlimit = pack(i2,0,$newValue);
    $f=syscall($SYS_setrlimit,$RLIMIT_STACK,$rlimit);
    if ($f != 0) {
	print "Warning, syscall to setrlimit failed\n";
    }
}

# Stop the iRODS servers
sub stopIrodsServer {
    runCmdNoLog(0, "yes | perl $startDir/server/bin/stop.pl");
#   print $cmdStdout;
}

# Start iRODS Server
sub startIrodsServer {
    runCmdNoLog(0, "perl $startDir/server/bin/start.pl");
#   print $cmdStdout;
}

# Depending on the state, stop running Postgres and iRODS servers
sub stopServers {
    $Servers = "";
    if ($rodsDone eq "1") {
    # have completed iRODS installation, stop it too
	stopIrodsServer();
	$Servers = "RODS";
    }
    if ($SubsetMode lt "1" and !$usingExistingPostgres) {
	runCmdNoLog(1, "$postgresBin/pg_ctl stop");
	if ($Servers) {
	    $Servers = $Servers . " and Postgres";
	}
	else {
	    $Servers = $Servers . "Postgres";
	}
    }
    print "Done stopping " . $Servers . " servers\n";
}

# Test if this host uses 64-bit addressing, and if so, print some
# helpful information and quit.
sub test64Addr {
  unlink("installrods64test.c");
  `echo "extern void exit(int status); int main() { char *foo; if (sizeof(foo)==8) exit(1); exit(0); }" > installrods64test.c`;
  `cc installrods64test.c -o installrods64test`;
  `./installrods64test`;
  $stat=$?;
  if ($stat == 0) {
      # not a 64 bit machine
      unlink("installrods64test.c");
      unlink("installrods64test");
      return;
  }
  if ($stat == -1) {
      # the cc command failed, try gcc
      `gcc installrods64test.c -o installrods64test`;
      `./installrods64test`;
      $stat=$?;
  }
  unlink("installrods64test.c");
  unlink("installrods64test");
  if ($stat == 0) {
      return;  # not a 64 bit machine
  }
  if ($stat==256) {
      # exit code of 1 (shifted a few bits); this is a 64 bit machine
      print "This host uses 64 bit addressing.\n";
      if ($unixODBC eq "1") {
	  print "Using unixODBC, so it should work OK\n";
	  return;
      }
      print "You need to use unixODBC as the ODBC libraray on 64-bit hosts\n";
      die("64 bit");
  }
  return;
}

sub runTests {

#
# Add the icommands bin directory to the path in case it is not there.
#
    $oldPath=$ENV{'PATH'};
    $ENV{'PATH'}="$startDir/clients/icommands/bin:$oldPath";

#
# First, run the clients/icommands/test scripts
#
    print "chdir'ing to: clients/icommands/test\n";
    chdir "clients/icommands/test" || 
	die "Can't chdir to clients/icommands/test";

#   Make the input file, default path and provide the pw
    unlink("testiCommands.input.39585");
    writeFile("testiCommands.input.39585", "\n$ADMIN_PW\n");

#   run testiCommands.pl
    printTime();
    runCmdNoLog(0, "testiCommands.pl < testiCommands.input.39585");
    unlink("testiCommands.input.39585");

#   Check the log
    $simpleHostname = hostname();
    $log = `cat testSurvey_$simpleHostname.log`;
    $ix = index($log, "List of failed tests:");
    $ix2 = index($log, "-", $ix);
    if ($ix2 > 0 || $ix <= 0) {
	print "Some of the testiCommands.pl tests failed\n";
	print "Check the log file in clients/icommands/test\n";
	exit 2;
    }
    print "Running iinit again since 'iexit full' is run in the above test\n";
    `iinit $ADMIN_PW`;

    chdir "../../.." || 
	die "Can't chdir to ../../..";

#
# Now run the server/test scripts
#
#   Put the debug option into the .irodsEnv file
    runCmdNoLog(0, "echo irodsDebug CATSQL >> $homeDir/.irods/.irodsEnv");

#   Set the path to include the new icommands and the cwd
    print "Setting PATH to include new i-commands and '.'\n";
    $oldPath=$ENV{'PATH'};
    $ENV{'PATH'}=".:$startDir/clients/icommands/bin:$oldPath";

#   Set the LD_LIBRARY_PATH
    $oldLibPath=$ENV{'LD_LIBRARY_PATH'};  
    if (!$oldLibPath) {
	$ENV{'LD_LIBRARY_PATH'}="$postgresInstallDir/lib";  
    }
    else {
	$ENV{'LD_LIBRARY_PATH'}="$oldLibPath:$postgresInstallDir/lib";
    }

#   Run the tests
    print "chdir'ing to: server/test/bin\n";
    chdir "server/test/bin" || die "Can't chdir to server/test/bin";
    unlink("icatTest.log");
    printTime();
    runCmdNoLog(0, "icatTest.pl >& icatTest.log");
    printTime();
    runCmdNoLog(0, "icatMiscTest.pl >& icatMiscTest.log");
    printTime();
    runCmdNoLog(0, "moveTest.pl >& moveTest.log");
    printTime();
    runCmdNoLog(0, "checkIcatLog.pl");
    print $cmdStdout;

#   Remove the debug line from the env file
    print "Removing irodsDebug CATSQL line from $homeDir/.irods/.irodsEnv\n";
    $txt = `cat $homeDir/.irods/.irodsEnv | grep -v CATSQL`;
    unlink ("~/.irods/.irodsEnv");
    writeFile("$homeDir/.irods/.irodsEnv", $txt);

    printTime();
    print "All tests completed successfully\nSUCCESS\n";

}

sub checkPostgresUser {
    if ($thisOS eq "Darwin") {
	$myPostgres=`ps | grep postmaster`;
    }
    if ($thisOS eq "SunOS") {
	$myPostgres=`ps -el | grep postgres | grep $uid`;
    }
    if ($thisOS eq "Linux") {
	$myPostgres=`ps -a | grep postmaster`;
    }
}

sub printTime {
    $date=`date`;
    chomp($date);
    print ("At $date ");
}


sub usage {
    $help = `cat install/install.help`;
    print $help;
}
