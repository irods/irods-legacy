#!/usr/bin/perl
#
# This script helps convert an MCAT DB to an ICAT one.
#
# Currently, this is very preliminary, but I was able to insert an SRB
# file into the ICAT and get it via 'iget'.
#
# Currently, the Spull.log file was gotten external to this
# script by running Spullmeta, something like:
#    Spullmeta -F GET_CHANGED_DATA_CORE_INFO 2002-12-12-12.12.12 > Spull.log
# and:
#    Spullmeta -F GET_CHANGED_COLL_CORE_INFO 2008-08-12 > Spull.log
#    (Using a recent date avoids built-in collections which we need to avoid)
#
# Other Spullmeta information is needed, data_types, etc.
#
# Users might want to adjust the following parameters for their
# particular MCAT and  ICATs:
@cv_srb_usernames=("srbAdmin", "another"); # username conversion table:srb form
@cv_irods_usernames=("rods", "huh");   # irods form for matching srb forms
@cv_srb_userdomains=("demo", "demo");  # Used for user.domain situations
$cv_srbZone = "A";
$cv_irodsZone = "tempZone";
$cv_resource = "srbResc";

$log1 = "Spull.log";      # copy of the Spullmeta log file
$sqlFile = "m2icat.cmds"; # the output file: SQL (for psql), or iadmin 
                          # commands (for sh)
$showOne="0";            # A debug option, if "1";



if ( open(  LOG_FILE, "<$log1" ) == 0 ) {
    die("open failed on input file " . $log1);
}
if ( open(  OUT_FILE, ">$sqlFile" ) == 0 ) {
    die("open failed on output file " . $sqlFile);
}

# First line in the log file is the run parameters, 
# 2nd is the column-names, the rest are data items.
$i = 0;
$mode = "";
foreach $line ( <LOG_FILE> ) {
    $i++;
    if ($i==1) {
	@cmdArgs = split('\|',$line);
	print ("cmdArgs[0]:" . $cmdArgs[0] . " " . "\n");
	if ($cmdArgs[0] eq "GET_CHANGED_COLL_CORE_INFO") {
	    $mode="COLL";
	}
	if ($cmdArgs[0] eq "GET_CHANGED_DATA_CORE_INFO") {
	    $mode="DATA";
	}
    }
    printf("MODE: $mode, i: $i\n");
    if ($i==2) {
	@names = split('\|',$line);
	print ("names[0]:" . $names[0] . " " . "\n");
	if ($mode eq "") {
	    die ("Unrecognized type of Spullmeta log file: $log1");
	}
    }
    if ($i>2) { # regular lines
	@values = split('\|',$line);
	if ($mode eq "DATA") {
#	    printf("values[2]= $values[2]\n");
	    $v_dataName = $values[0];
	    $v_dataTypeName = $values[1];
	    $v_phyPath = $values[2];
	    $v_resc = $values[3];
	    $v_size = $values[6];
	    $v_owner = convertUser($values[9]);
	    $v_create_time = convertTime($values[15]);
	    $v_access_time = convertTime($values[16]);
	    $v_collection = convertCollection($values[27]);
#   	    $t1 = convertTime ("2008-07-24-11.20.26");
#	    $t2 = convertTime ("2008-07-24-11.20.25");
#	    $t3 = convertTime ("2008-07-24-11.20.26");
	    print( OUT_FILE "insert into r_data_main (data_id, coll_id, data_name, data_repl_num, data_version, data_type_name, data_size, resc_name, data_path, data_owner_name, data_owner_zone, data_is_dirty, create_ts, modify_ts) values ((select nextval('R_ObjectID')), (select coll_id from r_coll_main where coll_name ='$v_collection'), '$v_dataName', '0', ' ', 'generic', '$v_size', '$cv_resource', '$v_phyPath', '$v_owner', '$cv_irodsZone', '1', '$v_create_time', '$v_access_time');\n");

	    getNow();
	    print( OUT_FILE "insert into r_objt_access ( object_id, user_id, access_type_id , create_ts,  modify_ts) values ( (select currval('R_ObjectID')), (select user_id from r_user_main where user_name = '$v_owner'), '1200', '$nowTime', '$nowTime');\n");

#	    print( OUT_FILE "$v_dataName $v_dataTypeName $v_phyPath $v_resc $v_size $v_owner $v_create_time $v_access_time $v_collection\n");

	    if ($i==3 && $showOne=="1") {
		$j = 0;
		foreach $value (@values) {
		    print ($names[$j] . " " . $value . "\n");
		    $j++;
		}
	    }
        }
	if ($mode eq "COLL") {
	    $v_coll_name=convertCollection($values[0]);
	    $v_parent_coll_name=convertCollection($values[1]);
	    $v_coll_owner=convertUser($values[2]);
	    $v_coll_zone=$values[6];
	    if ($v_coll_zone eq $cv_srbZone) { # make sure it's 
                                               # in the converting zone
		print( OUT_FILE "iadmin mkdir $v_coll_name $v_coll_owner\n");
		printf("iadmin mkdir $v_coll_name $v_coll_owner\n");
	    }
	}
    }
}
close( LOG_FILE );
close( OUT_FILE );


# run a command
# if option is 0 (normal), check the exit code and fail if non-0
# if 1, don't care
# if 2, should get a non-zero result, exit if not
sub runCmd {
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
    if ($option == 2) {
	if ($cmdStat==0) {
	    print "The following command should have failed:";
	    print "$cmd \n";
	    print "Exit code= $cmdStat \n";
	    die("command failed to fail");
	}
    }
}

# Using an iadmin option, convert srb to irods time values;
# also keep a cache list to avoid running the subprocess so often.
sub convertTime($)
{
    my ($inTime) = @_;
    $lastOutTime;
    $lastOutTimeConverted;
    $maxLastOutTime=10;

#   printf("inTime =%s\n",$inTime);
    $outTime = substr($inTime,0,10) . "." . substr($inTime, 11, 2) .
	":" . substr($inTime, 14, 2) . 	":" . substr($inTime, 17, 2);
#   printf("outTime=%s\n",$outTime);
    for ($i=0; $i<$maxLastOutTime; $i++) {
	if ($lastOutTime[$i] eq $outTime) {
	    return($lastOutTimeConverted[$i]);
	}
    }

    runCmd(0, "iadmin ctime str " . $outTime);
    $i = index($cmdStdout, "time: ");
    $outTimeConvert = substr($cmdStdout, $i+6);
    chomp($outTimeConvert);

    $lastOutTime[$ixLastOutTime] = $outTime;
    $lastOutTimeConverted[$ixLastOutTime] = $outTimeConvert;
    $ixLastOutTime++;
    if ($ixLastOutTime >= $maxLastOutTime) {
	$ixLastOutTime=0;
    }

    return($outTimeConvert);
}

# Using an iadmin option, get the current time in irods format
sub getNow() {
    runCmd(0, "iadmin ctime now");
    $i = index($cmdStdout, "time: ");
    $nowTime = substr($cmdStdout, $i+6);
    chomp($nowTime);
}

# Using the defined arrays at the top, possibly convert a user name
sub convertUser($)
{
    my ($inUser) = @_;
    $k=0;
    foreach $user (@cv_srb_usernames) {
	if ($user eq $inUser) {
	    return ($cv_irods_usernames[$k]);
	}
	$k++;
    }
    return($inUser);
}

# Using the defined arrays at the top, possibly convert collection name
sub convertCollection($)
{
    my ($inColl) = @_;
    my $outColl = $inColl;

    $outColl =~ s\/$cv_srbZone/\/$cv_irodsZone/\g; 
    $k=0;
    foreach $user (@cv_srb_usernames) {
	$tmp = "/" . $user . "." . $cv_srb_userdomains[$k];
	$tmp2 = "/" . $cv_irods_usernames[$k];
	$outColl =~ s\$tmp\$tmp2\g; 
	$k++;
    }
    return($outColl);
}
