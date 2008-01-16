#!/usr/bin/perl
#
# Copyright (c), The Regents of the University of California            ***
# For more information please refer to files in the COPYRIGHT directory ***
#
# Simple script to remove a few files and make a tar file for a
# release; to be run after a clean checkout from CVS.  If the top
# directory is still RODS, rename it to iRODS.
#
# It removes the doc/* files, then makes a tar file and then removes
# all the CVS directories and CVS files, and this script from the tar file.
# It also removes any .o and .a files (in case there are any) from the
# tar file, but not the executables so you should run this from a new
# cvs co directory.

$tarFileName="irods.tar";
$tmpFileName="tmpFile1";

print "cd ../..\n";
chdir("../..");

runCmd(1, 1, "rm -rf iRODS/doc");

runCmd(1, 1, "rm -rf iRODS/server/src/rcat");

runCmd(1, 1, "find iRODS -name Root");
$rootList=$cmdStdout;
chomp($rootList);

runCmd(1, 1, "find iRODS -name Entries");
$entriesList=$cmdStdout;
chomp($entriesList);

runCmd(1, 1, "find iRODS -name Repository");
$repList=$cmdStdout;
chomp($repList);

runCmd(1, 1, "find iRODS -name CVS");
$cvsList=$cmdStdout;
chomp($cvsList);

runCmd(1, 1, "find iRODS -name '*.o'");
$objList=$cmdStdout;
chomp($objList);

runCmd(1, 1, "find iRODS -name '*.a'");
$libList=$cmdStdout;
chomp($libList);

runCmd(1, 1, "tar cf $tarFileName iRODS");

print "unlinking $tmpFileName\n";
unlink('$tmpFileName');
printf "running: echo [rootList] > $tmpFileName\n";
runCmd(1, 0, "echo '$rootList' > $tmpFileName");
printf "running: echo [entriesList] >> $tmpFileName\n";
runCmd(1, 0, "echo '$entriesList' >> $tmpFileName");
printf "running: echo [repList] >> $tmpFileName\n";
runCmd(1, 0, "echo '$repList' >> $tmpFileName");
printf "running: echo [cvsList] >> $tmpFileName\n";
runCmd(1, 0, "echo '$cvsList' >> $tmpFileName");
if ($objList) {
    printf "running: echo [objList] >> $tmpFileName\n";
    runCmd(1, 0, "echo '$objList' >> $tmpFileName");
}
if ($libList) {
    printf "running: echo [libList] >> $tmpFileName\n";
    runCmd(1, 0, "echo '$libList' >> $tmpFileName");
}

runCmd(1, 1, "echo 'iRODS/install/prepRelease.pl' >> $tmpFileName");

runCmd(1, 1, "tar f $tarFileName --delete  --files-from=$tmpFileName");

# run a command;
# if chk is 1 (normal), check the exit code 
# if prt is 1 (normal), print the command
sub runCmd {
    my($chk, $prt, $cmd) = @_;
    if ($prt == 1) {
	print "running: $cmd \n";
    }
    $cmdStdout=`$cmd`;
    $cmdStat=$?;
    if ($chk == 1) {
	if ($cmdStat!=0) {
	    print "The following command failed:";
	    print "$cmd \n";
	    print "Exit code= $cmdStat \n";
	    die("command failed");
	}
    }
}
