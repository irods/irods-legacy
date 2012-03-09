###############################################################################################
###		getFileMTime.r: Simple example of retrieving file stat info and storing it
###		as user defined metadata. Can be used as a template for similar rules.
###		Assumes a link to /usr/bin/stat in <iRODS_DIR>/server/bin/cmd/
###		AdT, 03/08/12
###############################################################################################
getFileMTime {

### Command settings
	*command="stat";
#	*options="-f %m";		## OSX
	*options="-c %Y";		## Linux

### Get resource and file path of original replica
# Split collection and object name for query
	msiSplitPath(*objPath, *collection, *object);
# Add select fields
	msiAddSelectFieldToGenQuery("DATA_PATH", "null", *genQInp);
	msiAddSelectFieldToGenQuery("RESC_LOC", "null", *genQInp);
# Add conditions
	msiAddConditionToGenQuery("DATA_NAME", "=", *object, *genQInp);
	msiAddConditionToGenQuery("COLL_NAME", "=", *collection, *genQInp);
	msiAddConditionToGenQuery("DATA_REPL_NUM", "=", "0", *genQInp);
# Run query
	msiExecGenQuery(*genQInp, *genQOut);
# Extract file path and resource address
  	foreach (*genQOut)
	{
		msiGetValByKey(*genQOut, "DATA_PATH", *filePath);
		msiGetValByKey(*genQOut, "RESC_LOC", *resource);
	}

### Run stat on resource
# Watch for pathnames with spaces
	*args=*options++" "++"'"++*filePath++"'";
	msiExecCmd(*command, *args, *resource, "null", "null", *cmdOut);
# Get stdout from stat call
	msiGetStdoutInExecCmdOut(*cmdOut, *stdoutStr);

### Add mtime as user defined metadata to the object
# Create new key-value pair
# Add preceding 0 for consistency with iRODS timestamps (optional)
#	msiAddKeyVal(*KVP, "st_mtime", *stdoutStr);
	msiAddKeyVal(*KVP, "st_mtime", "0"++*stdoutStr);
# Add AVU
	msiAssociateKeyValuePairsToObj(*KVP, *objPath, "-d");
	
# Print stuff out if needed
#	writeLine("stdout", *stdoutStr);
}
INPUT *objPath = $1
OUTPUT ruleExecOut