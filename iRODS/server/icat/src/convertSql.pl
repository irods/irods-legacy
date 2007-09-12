#!/usr/bin/perl
#
# Copyright (c), The Regents of the University of California            ***
# For more information please refer to files in the COPYRIGHT directory ***
#
# Script to convert the .sql files between Oracle and Postgres form.
#

# PostgresQL and Oracle forms for the table creation, etc, performed
# by the *.sql files in this directory, are mostly the same, so rather
# than maintain two nearly identical copies, this script converts
# icatSysTables.sql and icatCoreTables.sql back and forth.  The only
# difference is that for Postgres we use 'bigint's and for Oracle
# 'integer's.  When Postgresql's smaller integer is OK (rather than
# bigint), the .sql files have use upper case INTEGER which is kept
# the same in both versions.

$tmpFile="convertSql.TempFile1";
($arg1, $arg2)=@ARGV;

if (!$arg1) {
    print "Usage postgresql|oracle\n";
    die("Invalid (null) argument");
}

if ($arg1 eq "oracle" || $arg1 eq "postgresql") {
}
else {
    print "Usage postgresql|oracle\n";
    die("Invalid argument");
}

if ($arg1 eq "oracle") {
    print ("oracle\n");

    unlink($tmpFile);
    runCmd("cat icatSysTables.sql | sed s/bigint/integer/g > $tmpFile");
    print "Moving $tmpFile to icatSysTables.sql\n";
    unlink("icatSysTables.sql");
    rename($tmpFile, "icatSysTables.sql");

    unlink($tmpFile);
    runCmd("cat icatCoreTables.sql | sed s/bigint/integer/g > $tmpFile");
    print "Moving $tmpFile to icatCoreTables.sql\n";
    unlink("icatCoreTables.sql");
    rename($tmpFile, "icatCoreTables.sql");

    exit(0);
}

if ($arg1 eq "postgresql") {
    print ("postgresql\n");

    unlink($tmpFile);
    runCmd("cat icatSysTables.sql | sed s/integer/bigint/g > $tmpFile");
    print "Moving $tmpFile to icatSysTables.sql\n";
    unlink("icatSysTables.sql");
    rename($tmpFile, "icatSysTables.sql");

    unlink($tmpFile);
    runCmd("cat icatCoreTables.sql | sed s/integer/bigint/g > $tmpFile");
    print "Moving $tmpFile to icatCoreTables.sql\n";
    unlink("icatCoreTables.sql");
    rename($tmpFile, "icatCoreTables.sql");

    exit(0);
}

exit(0);

# run a command and print a message and exit if it fails.
sub runCmd {
    my($cmd) = @_;
    print "running: $cmd \n";
    $cmdStdout=`$cmd`;
    $cmdStat=$?;
    if ($cmdStat!=0) {
	print "The following command failed:";
	print "$cmd \n";
	print "Exit code= $cmdStat \n";
	die("command failed");
    }
}

