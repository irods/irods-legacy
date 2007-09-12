#!/usr/bin/perl
#
# Copyright (c), The Regents of the University of California            ***
# For more information please refer to files in the COPYRIGHT directory ***
#
# This is a simple perl script that replaces text lines in a file.
# It is used by the configure script and the install.pl script.
#
# It replaces lines in the file arg1 that contain the string arg2 with
# the line arg3.  It will save a copy of the original file in file.orig 
# if it does not exist.
#

($InputFile, $testLine, $replaceLine)=@ARGV;

$OutputFile =$InputFile . ".tmp.1234323";
$InputFileOrig = $InputFile . ".orig";
open(FileIn, $InputFile) || die("Can't open input file " . "$InputFile");
open(FileOut, ">".$OutputFile) || die("Can't open output file "."$OutputFile");
while($line = <FileIn>) {
    $lineNum++;
    $where = index( $line, $testLine);
    if ($where >= 0) {
	$line = $replaceLine . "\n";
    }
    printf FileOut $line;
}
close(FileIn);
close(FileOut);

if ( -e $InputFileOrig ) {
    unlink($InputFile);
}
else {
    rename($InputFile, $InputFileOrig);
}
rename($OutputFile, $InputFile);
