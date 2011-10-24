#!/usr/bin/perl -w
#
# script to test the basic functionnalities of icommands.
# icommands location has to be put in the PATH env variable or their PATH will be asked
# at the beginning of the execution of this script.
#
# usage:   ./testiCommands.pl [debug] [noprompt] [help]
#    noprompt assumes iinit was done before running this script 
#    and will not ask for path and password input.
# 
#
# Copyright (c), CCIN2P3
# For more information please refer to files in the COPYRIGHT directory.

use strict;
use Cwd;
use Sys::Hostname;
use File::stat;
use File::Copy;

#-- Initialization

my $debug;
my $entry;
my @failureList;
my $i;
my $input;
my $irodsdefresource;
my $irodshost;
my $irodshome;
my $irodszone;
my $line;
my @list;
my $misc;
my $nfailure;
my $nsuccess;
my $rc;
my @returnref;
my @successlist;
my @summarylist;
my @tmp_tab;
my $username;
my @words;
# If noprompt_flag is set to 1, it assume iinit was done before running this
# script and will not ask for path and password input. A "noprompt" input 
# will set it.
my $noprompt_flag;
my $arg;

$debug = 0;
$noprompt_flag = 0;
foreach $arg (@ARGV)
{
    if ( $arg =~ "debug" ) {
        $debug = 1;
    } elsif ( $arg =~ "noprompt" ) {
	$noprompt_flag = 1;
    } elsif ( $arg =~ "help" ) {
	print ("usage:   $0 [debug] [noprompt] [help]\n");
	exit( 0 );
    }  else {
	print ("unknown input - $arg \n");
        print ("usage:   $0 [debug] [noprompt] [help]\n");
        exit( 1 );
    }
}

my $dir_w        = cwd();
my $myldir = $dir_w . '/ldir';
my $mylsize;
my $mysdir = '/tmp/irodssdir';
my $myssize;
my $host         = hostname();
if ( $host =~ '.' ) {
	@words = split( /\./, $host );
	$host  = $words[0];
}
my $irodsfile;
my $irodsEnvFile = $ENV{'irodsEnvFile'};
if ($irodsEnvFile) {
    $irodsfile = $irodsEnvFile;
} else {
    $irodsfile    = "$ENV{HOME}/.irods/.irodsEnv";
}
my $ntests       = 0;
my $progname     = $0;

my $outputfile   = "testSurvey_" . $host . ".log";
my $ruletestfile = "testRule_"   . $host . ".irb";
my $sfile2 = $dir_w . '/sfile2';
my $sfile2size;
system ( "cat $progname $progname > $sfile2" );
$sfile2size =  stat ($sfile2)->size;


#-- Find current working directory and make consistency path

$outputfile   = $dir_w . '/' . $outputfile;
$ruletestfile = $dir_w . '/' . $ruletestfile;

if ( $progname !~ '/' ) {
	$progname = $dir_w . '/' . $progname;
} else {
	if ( substr( $progname, 0, 2 ) eq './' ) {
		@words    = split( /\//, $progname );
		$i        = $#words;
		$progname = $dir_w . '/' . $words[$i];
	}
	if ( substr( $progname, 0, 1 ) ne '/' ) {
		$progname = $dir_w . '/' . $progname;
	}
}

#-- Remove ruletestfile

if ( -e $ruletestfile ) { unlink( $ruletestfile ); }

#-- Take debug level

# $debug = shift;
# if ( ! $debug ) {
# 	$debug = 0;
# } else {
# 	$debug = 1;
# }

#-- Print debug

if ( $debug ) {
	print( "\n" );
	print( "MAIN: irodsfile        = $irodsfile\n" );
	print( "MAIN: cwd              = $dir_w\n" );
	print( "MAIN: outputfile       = $outputfile\n" );
	print( "MAIN: ruletestfile     = $ruletestfile\n" );
	print( "MAIN: progname         = $progname\n" );
	print( "\n" );
}

#-- Dump content of $irodsfile to @list

@list = dumpFileContent( $irodsfile );

#-- Loop on content of @list
# The below parsing works in the current environment 
# but there are two shortcomings:
#   1) single quotes are removed, but if there were to be embedded ones,
#      they would be removed too.
#   2) if the name and value are separated by =, the line will not split right.
foreach $line ( @list ) {
 	chomp( $line );
	if ( ! $line ) { next; }
 	if ( $line =~ /irodsUserName/ ) {
		( $misc, $username ) = split( / /, $line );
		$username =~ s/\'//g; #remove all ' chars, if any
		next;
	}
	if ( $line =~ /irodsHome/ ) {
		( $misc, $irodshome ) = split( / /, $line );
		$irodshome =~ s/\'//g; #remove all ' chars, if any
		next;
	}
	if ( $line =~ /irodsZone/ ) {
		( $misc, $irodszone ) = split( / /, $line );
		$irodszone =~ s/\'//g; #remove all ' chars, if any
		next;
	}
	if ( $line =~ /irodsHost/ ) {
		( $misc, $irodshost ) = split( / /, $line );
		$irodshost =~ s/\'//g; #remove all ' chars, if any
		next;
	}
	if ( $line =~ /irodsDefResource/ ) {
		( $misc, $irodsdefresource ) = split( / /, $line );
		$irodsdefresource =~ s/\'//g; #remove all ' chars, if any
	}
}

#-- Print debug

if ( $debug ) {
	print( "MAIN: username         = $username\n" );
	print( "MAIN: irodshome        = $irodshome\n" );
	print( "MAIN: irodszone        = $irodszone\n" );
	print( "MAIN: irodshost        = $irodshost\n" );
	print( "MAIN: irodsdefresource = $irodsdefresource\n" );
}

#-- Environment setup and print to stdout

print( "\nThe results of the test will be written in the file: $outputfile\n\n" );
print( "Warning: you need to be a rodsadmin in order to pass successfully all the tests," );
print( " as some admin commands are being tested." );
if ( ! $noprompt_flag ) {
    print( " If icommands location has not been set into the PATH env variable," );
    print( " please give it now, else press return to proceed.\n" );
    print( "icommands location path: " );
    chomp( $input = <STDIN> );
    if ( $input ) { $ENV{'PATH'} .= ":$input"; }
    print "Please, enter the password of the iRODS user used for the test: ";
    chomp( $input = <STDIN> );
    if ( ! $input ) {
	print( "\nYou should give valid pwd.\n\n");
	exit;
    } else {
	print( "\n" );
    }


    runCmd( "iinit $input" );
}

#-- Test the icommands and eventually compared the result to what is expected.

#-- Basic admin commands

runCmd( "iadmin lt" );
runCmd( "iadmin lz", "", "LIST", $irodszone );
runCmd( "iadmin mkuser testuser1 domainadmin", "", "", "", "iadmin rmuser testuser1" );
runCmd( "iadmin lu testuser1", "", "user_type_name:", "domainadmin" );
runCmd( "iadmin moduser testuser1 type rodsuser" );
runCmd( "iadmin lu testuser1", "", "user_type_name:", "rodsuser" );
runCmd( "iadmin mkuser testuser2 rodsuser", "", "", "", "iadmin rmuser testuser2" );
runCmd( "iadmin lu", "", "LIST", "testuser1,testuser2" );
runCmd( "iadmin mkgroup testgroup", "", "", "", "iadmin rmgroup testgroup" );
runCmd( "iadmin atg testgroup testuser1" );
runCmd( "iadmin atg testgroup testuser2" );
runCmd( "iadmin lg testgroup", "", "LIST", "testuser1,testuser2" );
runCmd( "iadmin rfg testgroup testuser2" );
runCmd( "iadmin lg testgroup", "negtest", "LIST", "testuser1,testuser2" );
runCmd( "iadmin rfg testgroup testuser1" );
runCmd( "iadmin mkresc testresource \"unix file system\" cache $irodshost \"/tmp/foo\"", "", "", "", "iadmin rmresc testresource" );
# runCmd( "iadmin mkresc compresource \"test stage file system\" compound $irodshost \"/tmp/comp\"", "", "", "", "iadmin rmresc compresource" );
runCmd( "iadmin mkresc compresource \"unix file system\" compound $irodshost \"/tmp/comp\"", "", "", "", "iadmin rmresc compresource" );
runCmd( "iadmin lr testresource", "", "resc_name:", "testresource", "irmtrash" );
runCmd( "iadmin lr testresource", "", "resc_type_name:", "unix file system" );
runCmd( "iadmin lr testresource", "", "resc_net:", "$irodshost" );
runCmd( "iadmin modresc testresource comment \"Modify by me $username\"" );
runCmd( "iadmin lr testresource", "", "r_comment:", "Modify by me $username" );
runCmd( "iadmin mkgroup resgroup", "", "", "", "iadmin rmgroup resgroup" );
runCmd( "iadmin atg resgroup $username", "", "", "", "iadmin rfg resgroup $username" );
runCmd( "iadmin atrg resgroup testresource", "", "", "", "iadmin rfrg resgroup testresource" );
runCmd( "iadmin atrg resgroup compresource", "", "", "", "iadmin rfrg resgroup compresource" );
runCmd( "iadmin lrg resgroup", "", "LIST", "testresource, compresource" );

#-- basic clients commands.


# single file test

$myssize = stat ($progname)->size;
runCmd( "ilsresc", "", "LIST", "$irodsdefresource, testresource");
runCmd( "ilsresc -l",  "", "LIST", "$irodsdefresource, testresource");
runCmd( "imiscsvrinfo" );
runCmd( "iuserinfo", "", "name:", $username );
runCmd( "ienv" );
runCmd( "icd $irodshome" );
runCmd( "ipwd",  "", "LIST", "home" );
runCmd( "ihelp ils" );
runCmd( "ierror -14000", "", "LIST", "SYS_API_INPUT_ERR" );
runCmd( "iexecmd hello", "", "LIST", "Hello world" );
runCmd( "ips -v", "", "LIST", "ips" );
runCmd( "iqstat" );
runCmd( "imkdir $irodshome/test", "", "", "", "irm -r $irodshome/test" );
# make a directory of large files
runCmd( "iput -K $progname $irodshome/test/foo1", "", "", "", "irm $irodshome/test/foo1" );
runCmd( "iput -kf $progname $irodshome/test/foo1" );
runCmd( "ils -l $irodshome/test/foo1", "", "LIST", "foo1, $myssize" );
runCmd( "iadmin ls $irodshome/test", "", "LIST", "foo1" );
runCmd( "ils -A $irodshome/test/foo1", "", "LIST", "$username#$irodszone:own" );
runCmd( "ichmod read testuser1 $irodshome/test/foo1" );
runCmd( "ils -A $irodshome/test/foo1", "", "LIST", "testuser1#$irodszone:read" );
runCmd( "irepl -B -R testresource $irodshome/test/foo1" );
runCmd( "ils -l $irodshome/test/foo1", "", "LIST", "1 testresource" );
# overwrite a copy 
runCmd( "itrim -S  $irodsdefresource -N1 $irodshome/test/foo1" );
runCmd( "ils -L $irodshome/test/foo1", "negtest", "LIST", "$irodsdefresource" );
runCmd( "iphymv -R  $irodsdefresource $irodshome/test/foo1" );
runCmd( "ils -l $irodshome/test/foo1", "", "LIST", "$irodsdefresource" );
runCmd( "imeta add -d $irodshome/test/foo1 testmeta1 180 cm", "", "", "", "imeta rm -d $irodshome/test/foo1 testmeta1 180 cm" );
runCmd( "imeta ls -d $irodshome/test/foo1", "", "LIST", "testmeta1,180,cm" );
runCmd( "icp -K -R testresource $irodshome/test/foo1 $irodshome/test/foo2", "", "", "", "irm $irodshome/test/foo2" );
runCmd( "ils $irodshome/test/foo2", "", "LIST", "foo2" );
runCmd( "imv $irodshome/test/foo2 $irodshome/test/foo4" );
runCmd( "ils -l $irodshome/test/foo4", "", "LIST", "foo4" );
runCmd( "imv $irodshome/test/foo4 $irodshome/test/foo2" );
runCmd( "ils -l $irodshome/test/foo2", "", "LIST", "foo2" );
runCmd( "ichksum $irodshome/test/foo2", "", "LIST", "foo2" );
runCmd( "imeta add -d $irodshome/test/foo2 testmeta1 180 cm", "", "", "", "imeta rm -d $irodshome/test/foo2 testmeta1 180 cm" );
runCmd( "imeta add -d $irodshome/test/foo1 testmeta2 hello", "", "", "", "imeta rm -d $irodshome/test/foo1 testmeta2 hello"  );
runCmd( "imeta ls -d $irodshome/test/foo1", "", "LIST", "testmeta1,hello" );
runCmd( "imeta qu -d testmeta1 = 180", "", "LIST", "foo1" );
runCmd( "imeta qu -d testmeta2 = hello", "", "dataObj:", "foo1" );
runCmd( "iget -f -K $irodshome/test/foo2 $dir_w" );
runCmd( "ls -l $dir_w/foo2", "", "LIST", "foo2, $myssize");
unlink ( "$dir_w/foo2" );
# we have foo1 in $irodsdefresource and foo2 in testresource
# make a directory containing 20 small files
mksdir ();
runCmd( "irepl -B -R testresource $irodshome/test/foo1" );
runCmd( "iput -IkfR $irodsdefresource $sfile2 $irodshome/test/foo1" );
# show have 2 different copies
runCmd( "ils -l $irodshome/test/foo1", "", "LIST", "foo1, $myssize, $sfile2size" );
# update all old copies
runCmd( "irepl -U $irodshome/test/foo1" );
# make sure the old size is not there
runCmd( "ils -l $irodshome/test/foo1", "negtest", "LIST", "$myssize" );
runCmd( "itrim -S $irodsdefresource $irodshome/test/foo1" );
# bulk test
runCmd( "iput -bvPKr $mysdir $irodshome/test" );
# iput with a lot of options
my $rsfile = $dir_w . "/rsfile";
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "iput -PkIfTr -X $rsfile --retries 10  $mysdir $irodshome/testw",  "", "", "", "irm -rvf $irodshome/testw" );
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "iget -vIKPfr -X rsfile --retries 10 $irodshome/test $dir_w/testx", "", "", "", "rm -r $dir_w/testx" );
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "tar -chf $dir_w/testx.tar -C $dir_w/testx .", "", "", "", "rm $dir_w/testx.tar" );
my $phypath = $dir_w . '/' . 'testx.tar.' .  int(rand(10000000));
runCmd( "iput -p $phypath $dir_w/testx.tar $irodshome/testx.tar", "", "", "", "irm -f $irodshome/testx.tar" );
runCmd( "ibun -x $irodshome/testx.tar $irodshome/testx", "", "", "", "irm -rf $irodshome/testx" );
runCmd( "ils -lr $irodshome/testx", "", "LIST", "foo2, sfile10" );
runCmd( "ibun -cDtar $irodshome/testx1.tar $irodshome/testx", "", "", "", "irm -f $irodshome/testx1.tar" );
runCmd( "ils -l $irodshome/testx1.tar", "", "LIST", "testx1.tar" );
system ( "mkdir $dir_w/testx1" );
runCmd( "iget  $irodshome/testx1.tar $dir_w", "",  "", "", "rm $dir_w/testx1.tar" );
runCmd( "tar -xvf $dir_w/testx1.tar -C $dir_w/testx1", "", "", "", "rm -r $dir_w/testx1" );
runCmd( "diff -r $dir_w/testx $dir_w/testx1", "", "NOANSWER" );
system ( "mv $sfile2 /tmp/sfile2" );
runCmd( "ireg -KR testresource /tmp/sfile2  $irodshome/foo5", "", "", "", "irm -f foo5" );
runCmd( "iget -fK $irodshome/foo5 $dir_w/foo5", "", "", "", "rm $dir_w/foo5" );
runCmd( "diff /tmp/sfile2  $dir_w/foo5", "", "NOANSWER" );
runCmd( "ireg -KCR testresource $mysdir $irodshome/testa", "", "", "", "irm -vr $irodshome/testa" );
runCmd( "iget -fvrK $irodshome/testa $dir_w/testa" );
runCmd( "diff -r $mysdir $dir_w/testa", "", "NOANSWER" );
system ( "rm -r $dir_w/testa" );
runCmd( "imcoll -m link $irodshome/testa $irodshome/testb" );
runCmd( "iget -fvrK $irodshome/testb $dir_w/testb" );
runCmd( "diff -r $mysdir $dir_w/testb", "", "NOANSWER" );
runCmd( "imcoll -U $irodshome/testb" );
runCmd( "irm -rf $irodshome/testb" );
system ( "rm -r $dir_w/testb" );
runCmd( "imkdir $irodshome/testm" );
runCmd( "imcoll -m filesystem -R testresource $mysdir $irodshome/testm" );
runCmd( "iget -fvrK $irodshome/testa $dir_w/testm" );
runCmd( "diff -r $mysdir $dir_w/testm", "", "NOANSWER" );
runCmd( "imcoll -U $irodshome/testm" );
runCmd( "irm -rf $irodshome/testm" );
system ( "rm -r $dir_w/testm" );
system ( "rm -r $mysdir" );
runCmd( "imkdir $irodshome/testt" );
runCmd( "imcoll -m tar $irodshome/testx.tar $irodshome/testt" );
runCmd( "ils -lr $irodshome/testt", "", "LIST", "foo2, foo1" );
runCmd( "iget -vr $irodshome/testt  $dir_w/testt" );
runCmd( "diff -r  $dir_w/testx $dir_w/testt", "", "NOANSWER" );
runCmd( "imcoll -s $irodshome/testt" );
runCmd( "imcoll -p $irodshome/testt" );
runCmd( "imcoll -U $irodshome/testt" );
runCmd( "irm -rf $irodshome/testt" );
system ( "rm -r $dir_w/testt" );
# resource group test
runCmd( "iput -KR resgroup $progname $irodshome/test/foo6", "", "", "", "irm $irodshome/test/foo6" );
runCmd( "ils -l $irodshome/test/foo6", "", "LIST", "foo6, testresource" );
runCmd( "irepl -a $irodshome/test/foo6" );
runCmd( "ils -l $irodshome/test/foo6", "", "LIST", "compresource, testresource" );
runCmd( "itrim -S testresource -N1 $irodshome/test/foo6" );
runCmd( "ils -l $irodshome/test/foo6", "negtest", "LIST", "testresource" );
runCmd( "iget -f $irodshome/test/foo6 $dir_w/foo6" );
runCmd( "ils -l $irodshome/test/foo6", "", "LIST", "compresource, testresource" );
runCmd( "diff  $progname $dir_w/foo6", "", "NOANSWER" );
system ( "rm $dir_w/foo6" );



#-- Test a simple rule from the rule test file

$rc = makeRuleFile();
if ( $rc ) {
	print( "Problem with makeRuleFile. Rc = $rc\n" );
} else {
	runCmd( "irule -F $ruletestfile", "", "", "", "irm $irodshome/test/foo3" );
}

runCmd( "irsync $ruletestfile i:$irodshome/test/foo1" );
runCmd( "irsync i:$irodshome/test/foo1 $dir_w/foo1", "", "", "", "rm $dir_w/foo1" );
runCmd( "irsync i:$irodshome/test/foo1 i:$irodshome/test/foo2" );

if ( -e $ruletestfile ) { unlink( $ruletestfile ); }

# do the large files tests
mkldir ();
my $lrsfile = $dir_w . "/lrsfile";
if ( -e $lrsfile ) { unlink( $lrsfile ); }
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "iput -vbPKr --retries 10 -X $rsfile --lfrestart $lrsfile -N 2 $myldir $irodshome/test/testy" );
if ( -e $lrsfile ) { unlink( $lrsfile ); }
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "irepl -BvrPT -R testresource $irodshome/test/testy" );
runCmd( "itrim -vrS $irodsdefresource --dryrun --age 1 -N1 $irodshome/test/testy" );
runCmd( "itrim -vrS $irodsdefresource -N1 $irodshome/test/testy" );
runCmd( "icp -vKPTr -N2 $irodshome/test/testy $irodshome/test/testz" );
system ( "irm -vrf $irodshome/test/testy" );
runCmd( "iphymv -vrS $irodsdefresource -R testresource  $irodshome/test/testz" );

if ( -e $lrsfile ) { unlink( $lrsfile ); }
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "iget -vPKr --retries 10 -X $rsfile --lfrestart $lrsfile -N 2 $irodshome/test/testz $dir_w/testz" );
if ( -e $lrsfile ) { unlink( $lrsfile ); }
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "diff -r $dir_w/testz $myldir", "", "NOANSWER" );
system ( "rm -r $dir_w/testz" );
system ( "irm -vrf $irodshome/test/testz" );

# do the large files tests using RBUDP

runCmd( "iput -vQPKr --retries 10 -X $rsfile --lfrestart $lrsfile $myldir $irodshome/test/testy" );
if ( -e $lrsfile ) { unlink( $lrsfile ); }
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "irepl -BQvrPT -R testresource $irodshome/test/testy" );
runCmd( "itrim -vrS $irodsdefresource -N1 $irodshome/test/testy" );
runCmd( "icp -vQKPTr $irodshome/test/testy $irodshome/test/testz" );
system ( "irm -vrf $irodshome/test/testy" );
if ( -e $lrsfile ) { unlink( $lrsfile ); }
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "iget -vQPKr --retries 10 -X $rsfile --lfrestart $lrsfile $irodshome/test/testz $dir_w/testz" );
if ( -e $lrsfile ) { unlink( $lrsfile ); }
if ( -e $rsfile ) { unlink( $rsfile ); }
runCmd( "diff -r $dir_w/testz $myldir", "", "NOANSWER" );
system ( "rm -r $dir_w/testz" );
system ( "irm -vrf $irodshome/test/testz" );
system ( "rm -r $myldir" );

#-- Execute rollback commands

if ( $debug ) { print( "\nMAIN ########### Roll back ################\n" ); }

for ( $i = $#returnref; $i >= 0; $i-- ) {
	undef( @tmp_tab );
	$line     = $returnref[$i];
	@tmp_tab = @{$line};	
	runCmd( $tmp_tab[0], $tmp_tab[1], $tmp_tab[2], $tmp_tab[3] );
}

#-- Execute last commands before leaving

if ( $debug ) { print( "\nMAIN ########### Last ################\n" ); }

runCmd( "iadmin lg", "negtest", "LIST", "testgroup" );
runCmd( "iadmin lg", "negtest", "LIST", "resgroup" );
runCmd( "iadmin lr", "negtest", "LIST", "testresource" );
runCmd( "irmtrash" );
if ( ! $noprompt_flag ) {
    runCmd( "iexit full" );
}
`/bin/rm -rf /tmp/foo`;# remove the vault for the testresource; needed in case
                       # another unix login runs this test on this host
`/bin/rm -rf /tmp/comp`;
#-- print the result of the test into testSurvey.log

$nsuccess = @successlist;
$nfailure = @failureList;

open( FILE, "> $outputfile" ) or die "unable to open $outputfile for writing\n";
print( FILE "===========================================\n" );
print( FILE "number of successfull tested commands = $nsuccess\n" );
print( FILE "number of failed tested commands      = $nfailure\n" );
print( FILE "===========================================\n\n" );
print( FILE "Summary of the consecutive commands which have been tested:\n\n" );

$i = 1;
foreach $line ( @summarylist ) {
  print( FILE "$i - $line\n" );
  $i++;
}
print( FILE "\n\nList of successfull tests:\n\n" );
foreach $line ( @successlist ) { print( FILE "$line" ); }
print( FILE "\n\nList of failed tests:\n\n" );
foreach $line ( @failureList ) { print( FILE "$line" ); }
print( FILE "\n" );
close( FILE );
exit;

##########################################################################################################################
# runCmd needs at least 8 arguments: 
#   1- command name + arguments
#   2- specify if it is a negative test by providing the "negtest" value, ie it is successfull if the test fails (optional).
#   3- output line of interest (optional), if equal to "LIST" then match test the entire list of answers provided in 4-. if equal to "NOANSWER" then expect no answer.
#   4- expected list of results separeted by ',' (optional: must be given if second argument provided else it will fail).
#	5- command name to go back to first situation
#	6- same as 2 but for 5
#	7- same as 3 but for 5
#	8- same as 4 but for 5

sub runCmd {
 	my ( $cmd, $testtype, $stringToCheck, $expResult, $r_cmd, $r_testtype, $r_stringToCheck, $r_expResult ) = @_;

	my $rc = 0;
	my @returnList;
	my @words;
	my $line;

 	my $answer     = "";
 	my @list       = "";
 	my $numinlist  = 0;
 	my $numsuccess = 0;
 	my $tempFile   = "/tmp/iCommand.log";
 	my $negtest    = 0;
 	my $result     = 1; 		# used only in the case where the answer of the command has to be compared to an expected answer.

#-- Check inputs

	if ( ! $cmd ) {
		print( "No command given to runCmd; Exit\n" );
		exit;
	}
	if ( ! $testtype ) {
		$testtype = 0;
		$negtest  = 0;
	} else {
		if ( $testtype eq "negtest" ) {
			$negtest = 1;
		} else {
			$negtest = 0;
		}	
	}
	if ( ! $stringToCheck   ) { $stringToCheck = ""; }
	if ( ! $expResult       ) { $expResult = ""; }
	if ( ! $r_cmd           ) { $r_cmd = ""; }
	if ( ! $r_testtype      ) { $r_testtype = ""; }
	if ( ! $r_stringToCheck ) { $r_stringToCheck = ""; }
	if ( ! $r_expResult     ) { $r_expResult = ""; }

#-- Update counter

	$ntests++;

#-- Print debug
	
	if ( $debug ) { print( "\n" ); }
	printf( "%3d - cmd executed: $cmd\n", $ntests );
	if ( $debug ) { print( "DEBUG: input to runCMd: $cmd, $testtype, $stringToCheck, $expResult.\n" ); }

#-- Push return command in list

	undef( @returnList );
	if ( $r_cmd ){
		$returnList[0] = $r_cmd;
		$returnList[1] = $r_testtype;
		$returnList[2] = $r_stringToCheck;
		$returnList[3] = $r_expResult;
		push( @returnref, \@returnList );
		
		if ( $debug ) { print( "DEBUG: roll back:       $returnList[0], $returnList[1], $returnList[2], $returnList[3].\n" ); }
	} else {
		if ( $debug ) { print( "DEBUG: roll back:       no.\n" ); }		
	}
	
#-- Push icommand in @summarylist

	push( @summarylist, "$cmd" );

#-- Execute icommand

	$rc = system( "$cmd > $tempFile" );

#-- check that the list of answers is part of the result of the command.

	if (( $rc == 0 ) and $stringToCheck ) {
		@words     = split( ",", $expResult );
		$numinlist = @words;
		@list      = dumpFileContent( $tempFile );
		
		if ( $debug ) {
			print( "DEBUG: numinlist = $numinlist\n" );
			print( "DEBUG: list =\n@list\n" );
		}
				
#---- If LIST is given as 3rd element: compare output of icommand to list in 4th argument
		
		if ( $stringToCheck eq "LIST" ) {
			foreach $line ( @list ) {
				chomp( $line );
				$line =~ s/^\s+//;
				$line =~ s/\s+$//;
				$answer .= "$line ";
			}
			chomp( $answer );
			$answer =~ s/^\s+//;
			$answer =~ s/\s+$//;

			if ( $debug ) { print( "DEBUG: answer    = $answer\n" ); }

			foreach $entry ( @words ) {
				if ( $answer =~ /$entry/ ) { $numsuccess++; }
			}
			
			if ( $numsuccess == $numinlist ) {
				$result = 1;
			} else {
				$result = 0;
			}
		} elsif ( $stringToCheck eq "NOANSWER" ) {
			my $numanswer = @list;
			if ($numanswer == 0) {
				$result = 1;
                        } else {
                                $result = 0;
                        }
		} else {
			if ( $debug ) { print( "DEBUG: stringToCheck = $stringToCheck\n" ); }
			foreach $line ( @list ) {
				chomp( $line );
				if ( $debug ) { print( "DEBUG: line = $line\n" ); }
				if ( $line =~ /$stringToCheck/ ) {
					( $misc, $answer ) = split( /$stringToCheck/, $line );
					$answer =~ s/^\s+//;		# remove blanks
					$answer =~ s/\s+$//;		# remove blanks
					last;
				}
			}
			
			if ( $answer eq $words[0] ) {
				$result = 1;
			} else {
				$result = 0;
			}
		}
	}
	
	if ( $rc == 0 and ( $result ^ $negtest ) ) {
		push( @successlist, "$ntests - $cmd  ====> OK\n" );
		$result = 1;
	} else {
		push( @failureList, "$ntests - $cmd  ====> error code = $rc\n" );
		$result = 0;
	}

	if ( $debug ) { print( "DEBUG: result    = $result (1 is OK).\n" ); }			
	
	unlink( $tempFile );
	return();
}
##########################################################################################################################
# dumpFileContent: open a file in order to dump its content to an array

sub dumpFileContent {
	my $file = shift;
	my @filecontent;
	my $line;

	open( DUMP, $file ) or die "Unable to open the file $file in read mode.\n";
	foreach $line ( <DUMP> ) {
		$line =~ s/^\s+//;		# remove blanks
		$line =~ s/\s+$//;		# remove blanks
		push( @filecontent, $line );
	}
	close( DUMP );
	return( @filecontent );
}
##########################################################################################################################
# makeRuleFile: make a rule test file

sub makeRuleFile {
	my $rc;
	
	$rc = open( FILE2, ">$ruletestfile" );
	if ( ! $rc ) {
		print( "Impossible to open the file $ruletestfile in write mode.\n" );
		return( 1 );
	}

	print( FILE2 "# This is an example of an input for the irule command.\n" );
	print( FILE2 "# This first input line is the rule body.\n" );
	print( FILE2 "# The second input line is the input parameter in the format of:\n" );
	print( FILE2 "#   label=value. e.g., *A=$irodshome/test/foo1\n" );
	print( FILE2 "# Multiple inputs can be specified using the \'\%\' character as the seperator.\n" );
	print( FILE2 "# The third input line is the output description. Multiple outputs can be specified\n" );
	print( FILE2 "# using the \'\%\' character as the seperator.\n" );
	print( FILE2 "#\n" );
	print( FILE2 "myTestRule||msiDataObjOpen(*A,*S_FD)" );
	print( FILE2 "##msiDataObjCreate(*B,\'$irodsdefresource\',*D_FD)" );
	print( FILE2 "##msiDataObjLseek(*S_FD,10,\'SEEK_SET\',*junk1)" );
	print( FILE2 "##msiDataObjRead(*S_FD,10000,*R_BUF)" );
	print( FILE2 "##msiDataObjWrite(*D_FD,*R_BUF,*W_LEN)" );
	print( FILE2 "##msiDataObjClose(*S_FD,*junk2)" );
	print( FILE2 "##msiDataObjClose(*D_FD,*junk3)" );
	print( FILE2 "##msiDataObjCopy(*B,*C,\'$irodsdefresource\',*junk4)" );
#	print( FILE2 "##delayExec(msiDataObjRepl(*C,$irodsdefresource,*junk5),<A></A>)" );
	print( FILE2 "##msiDataObjUnlink(*B,*junk6)|null" );
	print( FILE2 "\n" );
	print( FILE2 "*A=\"$irodshome/test/foo1\"\%*B=\"$irodshome/test/foo4\"\%*C=\"$irodshome/test/foo3\"" );
	print( FILE2 "\n" );
	print( FILE2 "*R_BUF\%*W_LEN" );
	print( FILE2 "\n" );
	close( FILE2 );
	return( 0 );
}

# make a directory of 2 large files and 2 small fles
sub mkldir
{
    my $i;
    my $count = 5; 
    my $fcount = 2; 
    my $mylfile;
    my $mysfile;
    my $lfile = $dir_w . "/lfile";
    my $lfile1 = $dir_w . "/lfile1";
    system( "echo 012345678901234567890123456789012345678901234567890123456789012 > $lfile" );
    for ( $i = $count; $i >= 0; $i-- ) {
      system ( "cat $lfile $lfile $lfile $lfile $lfile $lfile $lfile $lfile $lfile > $lfile1" );
	rename ( $lfile1, $lfile );
    }
    $mylsize = stat ($lfile)->size;
    system ( "mkdir $myldir" );
    for ( $i = $fcount; $i > 0; $i-- ) {
        $mylfile = $myldir . '/' . 'lfile' . $i;
        $mysfile = $myldir . '/' . 'sfile' . $i;
	if ($i != 1) {
	    copy ( $lfile, $mylfile );
	} else { 
	    rename ( $lfile, $mylfile );
	}
	copy ( $progname, $mysfile );
    }
}

# make a directory of small files and $sfile2
sub mksdir
{
    my $i;
    my $count = 20;
    my $mysfile;
    system ( "mkdir $mysdir" );
    for ( $i = $count; $i > 0; $i-- ) {
        $mysfile = $mysdir . '/' . 'sfile' . $i;
	copy ( $progname, $mysfile );
    }
}

