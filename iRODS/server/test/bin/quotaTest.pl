#!/usr/bin/perl
#
# Copyright (c), The Regents of the University of California            ***
# For more information please refer to files in the COPYRIGHT directory ***
#
# This is a test script that runs various tests on the ICAT quota
# functions.
#
# It will print "Success" at the end and have a 0 exit code, if all
# everything works properly.
#
# Some of the icommands print error messages in various cases, but
# that's OK, sometimes they should or sometimes it doesn't matter.
# The important checks are being made.  If "Success" is printed at the
# end, then everything was OK.
#

$F1="TestFile1";
$F2="TestFile2";
$F3="TestFile3";

# Quota Users:
$QU2="qu2";
$QU1="qu1";

$Resc="demoResc";
$Resc2="Resc2";

$G1="quotaGroup1";

# run a command
# if option is 0 (normal), check the exit code and fail if non-0
# if 1, don't care
# if 2, should get a non-zero result, exit if not
sub runCmd {
    my($option, $cmd, $stdoutVal) = @_;
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
    if ($option == 2) {
	if ($cmdStat==0) {
	    print "The following command should have failed:";
	    print "$cmd \n";
	    print "Exit code= $cmdStat \n";
	    die("command failed to fail");
	}
    }
    if ($stdoutVal != "") {
	if ($cmdStdout != $stdoutVal) {
	    print "stdout should have been: $stdoutVal\n";
	    print "but was: $cmdStdout";
	    die("incorrect stdout");
	}
    }
}


# get our zone name
runCmd(0, "ienv | grep irodsZone | tail -1");
chomp($cmdStdout);
$ix = index($cmdStdout,"=");
$myZone=substr($cmdStdout, $ix+1);

# Move/rename tests
`ls -l > $F1`;
$F1a = "$F1" . "a";
$F1b = "$F1" . "b";

# Basic tests
$tmpPwFile="/tmp/testPwFile.5678956";

$DIR = "/$myZone/home/$QU1";
$ENV{'irodsAuthFileName'}=$tmpPwFile;
$ENV{'irodsUserName'}=$QU1;
runCmd(1, "echo 123 | irm -f $DIR/$F1");
runCmd(1, "echo 123 | irmtrash");
delete $ENV{'irodsUserName'};
delete $ENV{'irodsAuthFileName'};

runCmd(1, "iadmin rmuser $QU1");
runCmd(0, "iadmin mkuser $QU1 rodsuser");
runCmd(0, "iadmin moduser $QU1 password 123");

runCmd(0, "iadmin cu");               # make sure it's initialized

runCmd(1, "iadmin suq $QU1 $Resc 0"); # unset quota for this user, if any
runCmd(0, "iadmin cu");               # make sure above is all set up
runCmd(0, "test_chl checkquota $QU1 $Resc 0 0");
runCmd(0, "iadmin suq $QU1 $Resc 100");
runCmd(0, "test_chl checkquota $QU1 $Resc 0 1"); # before cu (not exactly right)
runCmd(0, "iadmin cu");
runCmd(0, "test_chl checkquota $QU1 $Resc m100 1"); # m100 is -100

runCmd(0, "iadmin moduser $QU1 password 123");
unlink($F1);
runCmd(0, "head -c50 /etc/passwd > $F1");
$ENV{'irodsAuthFileName'}=$tmpPwFile;
$ENV{'irodsUserName'}=$QU1;
runCmd(0, "echo 123 | iput $F1 $DIR");
delete $ENV{'irodsUserName'};
delete $ENV{'irodsAuthFileName'};

runCmd(0, "test_chl checkquota $QU1 $Resc m100 1");
runCmd(0, "iadmin cu");
runCmd(0, "test_chl checkquota $QU1 $Resc m50 1");

runCmd(0, "iadmin suq $QU1 $Resc 40");
runCmd(0, "iadmin cu");
runCmd(0, "test_chl checkquota $QU1 $Resc 10 1");


runCmd(0, "iadmin suq $QU1 $Resc 40000000000000");
runCmd(0, "iadmin cu");
sleep(1);
runCmd(0, "test_chl checkquota $QU1 $Resc m39999999999950 1");

$ENV{'irodsAuthFileName'}=$tmpPwFile;
$ENV{'irodsUserName'}=$QU1;
runCmd(0, "echo 123 | irm -f $DIR/$F1");
runCmd(0, "echo 123 | irmtrash");
delete $ENV{'irodsUserName'};
delete $ENV{'irodsAuthFileName'};

runCmd(0, "iadmin cu");
sleep(1);
runCmd(0, "test_chl checkquota $QU1 $Resc m40000000000000 1");
runCmd(0, "iadmin suq $QU1 $Resc 40");
runCmd(0, "iadmin cu");
runCmd(0, "test_chl checkquota $QU1 $Resc m40 1");

# clean up
runCmd(0, "iadmin suq $QU1 $Resc 0"); # unset quota for this user
runCmd(0, "iadmin rmuser $QU1");

printf("Success\n");
