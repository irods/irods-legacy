#!/usr/bin/perl 

# Simple script that takes a ticket as input and gets the associated
# irods file (if any).  It does an iquest query to get the collection
# and name and then uses that to 'iget' the file.

($arg1, $arg2, $arg3)=@ARGV;

my $ticket = $arg1;

if ($arg1 eq "") {
    printf("Usage: igetbyticket.pl ticket-string\n");
    printf("Type 'igetbyticket.pl help' for more\n");
    exit();
}

if ($arg1 eq "help") {
    printf("This script gets an iRODS file using a provided ticket.\n");
    printf("It does this by running iquest to map the ticket to the file\n");
    printf("and then iget to retrieve it.  For more about tickets see\n");
    printf("https://www.irods.org/index.php/Ticket-based_Access .\n");
    exit();
}

my $verbose = '0';
my $iquest = "iquest";  # just assuming it's in the path for now
my $iget = "iget";      # just assuming it's in the path for now

my $command = "$iquest \"%s/%s\" \"select TICKET_DATA_COLL_NAME, TICKET_DATA_NAME where TICKET_STRING = \'$ticket\'\" ";
if ($verbose==1) { print "running: $command \n"; }
my $output = `$command`;
my $cmdStat=$?;
chomp($output);
if ($verbose==1) { print "out:" . $output . ":\n"; }
my $fullFilePath=$output;
if ($cmdStat != 0) {
    printf("Not found\n");
    exit();
}
my $command = "$iget -t $ticket $fullFilePath";
print "getting: $fullFilePath\n";
if ($verbose==1) { print "running: $command \n"; }
my $output = `$command`;
my $cmdStat=$?;
