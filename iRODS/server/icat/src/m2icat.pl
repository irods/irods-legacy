#!/usr/bin/perl
#
# This script helps convert an MCAT DB to an ICAT one.
#
# Currently, this is very preliminary, but I was able to insert an SRB
# file into the ICAT and get it via 'iget'.  Currently 
#
# If the Spull.log.* files don't exist, this script will run Spullmeta
# to create them.
#
# This script generates two files, one with iadmin commands and other
# with psql sql commands.  After these are created, this script will
# prompt and if confirmed will attempt to run iadmin and psql.  You
# might instead tho check the contents of these files and run the
# iadmin and psql steps my hand.
#
# You will need to change the following parameters for your
# particular MCAT and  ICATs and environment.

# 
$cv_srbZone = "A";            # Your SRB zone name
$cv_irodsZone = "tempZone";   # Your iRODS zone name
$SRB_BEGIN_DATE="2008-07-12"; # approximate date your SRB was installed; used to
                              # avoid some unneeded built-in items, etc.
$Spullmeta = "/scratch/slocal/srbtest/testc/SRB2_0_0rel/utilities/bin/Spullmeta";
                              # if in Path, but could also be just "Spullmeta"
$iadmin = "iadmin";           # Change this to be full path if not in PATH.
$psql = "psql";               # And this too.
@cv_srb_usernames=("srbAdmin"); # username conversion table:srb form
@cv_irods_usernames=("rods");   # irods form for matching srb forms
@cv_srb_userdomains=("demo");   # Used for user.domain situations

#------------
# You can, but don't need to, change these:
$logColl  = "Spull.log.coll";    # Spullmeta log file for collections
$logData = "Spull.log.data";     # Spullmeta log file for dataObjects
$logResc = "Spull.log.resc";     # Spullmeta log file for resources.
$sqlFile = "m2icat.cmds.sql"; # the output file: SQL (for psql)
$iadminFile = "m2icat.cmds.iadmin"; # the output file for iadmin commands
$showOne="0";            # A debug option, if "1";

#------------
@cv_srb_resources =();   # SRB resource(s) being converted (created on the fly)
@cv_irods_resources=();  # corresponding name of SRB resource(s) in iRODS.
@datatypeList=();        # filled and used dynamically


if (!-e $logResc) {
    runCmd(0, "$Spullmeta -F GET_CHANGED_PHYSICAL_RESOURCE_CORE_INFO $SRB_BEGIN_DATE > $logResc");
}
if (!-e $logColl) {
    runCmd(0, "$Spullmeta -F GET_CHANGED_COLL_CORE_INFO $SRB_BEGIN_DATE > $logColl");
}
if (!-e $logData) {
    runCmd(0, "$Spullmeta -F GET_CHANGED_DATA_CORE_INFO $SRB_BEGIN_DATE > $logData");
}

if ( open(  SQL_FILE, ">$sqlFile" ) == 0 ) {
    die("open failed on output file " . $sqlFile);
}
if ( open(  IADMIN_FILE, ">$iadminFile" ) == 0 ) {
    die("open failed on output file " . $iadminFile);
}

processLogFile($logResc);

processLogFile($logColl);

processLogFile($logData);

foreach $dataType (@datatypeList) {
    print( IADMIN_FILE "at data_type \'$dataType\'\n");
}
print( IADMIN_FILE "quit\n");

close( IADMIN_FILE );
close( SQL_FILE );

printf("Enter y or yes if you want this script to run the next steps now;\n");
printf("to run iadmin and psql with the files just generated:");
my $answer = <STDIN>;
chomp( $answer );	# remove trailing return
if ($answer eq "yes" || $answer eq "y") {
    runCmd (1, "iadmin < $iadminFile" );
    printf("Enter y or yes if you want to go ahead and run psql now:");
    my $answer = <STDIN>;
    chomp( $answer );	# remove trailing return
    if ($answer eq "yes" || $answer eq "y") {
	runCmd (0, "$psql ICAT < $sqlFile");
    }
}

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

# Add item to the datatype array unless it's already there.
sub AddToDatatypeList($) {
    my($item) = @_;
    if ($item eq "generic") {return;}
    foreach $listItem (@datatypeList) {
        if ($listItem eq $item) {
            return;
        }
    }
    push(@datatypeList, $item);
}


sub processLogFile($) {
# First line in the log file is the run parameters, 
# 2nd is the column-names, the rest are data items.
    my($logFile) = @_;
    if ( open(  LOG_FILE, "<$logFile" ) == 0 ) {
	die("open failed on input file " . $logFile);
    }
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
	    if ($cmdArgs[0] eq "GET_CHANGED_PHYSICAL_RESOURCE_CORE_INFO") {
		$mode="RESC";
	    }
	}
	printf("MODE: $mode, i: $i\n");
	if ($i==2) {
	    @names = split('\|',$line);
	    print ("names[0]:" . $names[0] . " " . "\n");
	    if ($mode eq "") {
		die ("Unrecognized type of Spullmeta log file: $logFile");
	    }
	}
	if ($i>2) { # regular lines
	    @values = split('\|',$line);
	    $doDataInsert=0;
	    if ($mode eq "DATA") {
		$v_resc = $values[3];
		$k=0;
		foreach $resc (@cv_srb_resources) {
		    if ($resc eq $v_resc) {
			$newResource = $cv_irods_resources[$k];
			$doDataInsert=1;
		    }
		    $k++;
		}
	    }
	    if ($doDataInsert) {
		$v_dataName = $values[0];
		$v_dataTypeName = $values[1];
		$v_phyPath = $values[2];
		$v_size = $values[6];
		$v_owner = convertUser($values[9]);
		$v_create_time = convertTime($values[15]);
		$v_access_time = convertTime($values[16]);
		$v_collection = convertCollection($values[27]);
		print( SQL_FILE "begin;\n"); # begin/commit to make these 2 like 1
		print( SQL_FILE "insert into r_data_main (data_id, coll_id, data_name, data_repl_num, data_version, data_type_name, data_size, resc_name, data_path, data_owner_name, data_owner_zone, data_is_dirty, create_ts, modify_ts) values ((select nextval('R_ObjectID')), (select coll_id from r_coll_main where coll_name ='$v_collection'), '$v_dataName', '0', ' ', '$v_dataTypeName', '$v_size', '$newResource', '$v_phyPath', '$v_owner', '$cv_irodsZone', '1', '$v_create_time', '$v_access_time');\n");

		getNow();
		print( SQL_FILE "insert into r_objt_access ( object_id, user_id, access_type_id , create_ts,  modify_ts) values ( (select currval('R_ObjectID')), (select user_id from r_user_main where user_name = '$v_owner'), '1200', '$nowTime', '$nowTime');\n");

		print( SQL_FILE "commit;\n");

		AddToDatatypeList($v_dataTypeName);

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
		    print( IADMIN_FILE "mkdir $v_coll_name $v_coll_owner\n");
		    printf("mkdir $v_coll_name $v_coll_owner\n");
		}
	    }
	    if ($mode eq "RESC") {
		$v_resc_name=$values[0];
		$v_resc_type=$values[1];
		$v_resc_path=$values[2];
		$v_resc_physical_name=$values[3];
		$v_resc_location=$values[5];
		if ($v_resc_name eq $v_resc_physical_name &&
		    $v_resc_type eq "unix file system") {
		    $k = index($v_resc_path, "/?");
		    $newPath = substr($v_resc_path, 0, $k);
		    push(@cv_srb_resources, $v_resc_name);
		    $newName = "SRB-" . $v_resc_name;
		    push(@cv_irods_resources, $newName);
		    print( IADMIN_FILE "mkresc '$newName' '$v_resc_type' 'archive' '$v_resc_location' $newPath\n");
		}
	    }
	}
    }
    close( LOG_FILE );
}
