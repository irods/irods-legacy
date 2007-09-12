#!/usr/bin/perl
#
# Copyright (c), The Regents of the University of California            ***
# For more information please refer to files in the COPYRIGHT directory ***
#
# This script parses a module's 'info.txt' file and returns the requested
# entry.
#

# Get the arguments
($filename, $keyword, $case)=@ARGV;

# Read the file
open(  File, $filename) || die("Can't open input file " . "$filename");
chomp( @data = <File> );
close( File );

# Find the desired line and print it out
foreach $line ( @data )
{
	if ( (($val) = ($line =~ /^$keyword\s*:\s*(.*)$/i)) )
	{
		if ( $case =~ "lowercase" )
		{
			$val = lc( $val );
		}
		print "$val\n";
		exit( 0 );
	}
}
exit( 1 );
