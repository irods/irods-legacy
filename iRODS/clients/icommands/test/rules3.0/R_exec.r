###############################################################################################
###		R_exec.r: Runs an R program on a designated node (resource)
###		Assumes a link to R in <iRODS_DIR>/server/bin/cmd/ on that resource
###		AdT, 04/30/12
###############################################################################################
R_exec 
{
	### Split collection and object name, for queries
	msiSplitPath(*inputObj, *collection, *object);
	

	### 1st query to see if obj exists on R-enabled resc
	# Add select fields
	msiAddSelectFieldToGenQuery("DATA_ID", "COUNT", *genQInp0);
	# Add conditions
	msiAddConditionToGenQuery("DATA_NAME", "=", *object, *genQInp0);
	msiAddConditionToGenQuery("COLL_NAME", "=", *collection, *genQInp0);
	msiAddConditionToGenQuery("DATA_RESC_NAME", "=", *rescName, *genQInp0);
	# Run query
	msiExecGenQuery(*genQInp0, *genQOut0);
	# Extract file path and resource address
  	foreach (*genQOut0)
	{
		msiGetValByKey(*genQOut0, "DATA_ID", *dataIdCount);
	}
	

	### Replicate obj to target resc if needed
	if (int(*dataIdCount) == 0)
	then
	{
		msiDataObjRepl(*inputObj, *rescName, *foo);
	}
	
	
	### Get resource address and file path
	# Add select fields
	msiAddSelectFieldToGenQuery("DATA_PATH", "null", *genQInp);
	msiAddSelectFieldToGenQuery("RESC_LOC", "null", *genQInp);
	# Add conditions
	msiAddConditionToGenQuery("DATA_NAME", "=", *object, *genQInp);
	msiAddConditionToGenQuery("COLL_NAME", "=", *collection, *genQInp);
	msiAddConditionToGenQuery("DATA_RESC_NAME", "=", *rescName, *genQInp);
	# Run query
	msiExecGenQuery(*genQInp, *genQOut);
	# Extract file path and resource address
  	foreach (*genQOut)
	{
		msiGetValByKey(*genQOut, "DATA_PATH", *filePath);
		msiGetValByKey(*genQOut, "RESC_LOC", *rescAddr);
	}
	
	
	### Invoke R on resource
	# Watch for pathnames with spaces
	*args="-f '*filePath'";
	msiExecCmd("R", *args, *rescAddr, "null", "null", *cmdOut);
	# Get stdout
	msiGetStdoutInExecCmdOut(*cmdOut, *stdoutStr);
	
	
	### Save result in new object
	# Create path with timestamp
	msiGetSystemTime(*Time, "human");
	*outputObj = *inputObj++".OUT."++*Time++".txt";
	# Write to new object
	msiStrlen(*stdoutStr, *len);
	msiDataObjCreate(*outputObj, "", *W_FD);
	msiDataObjWrite(*W_FD, *stdoutStr, *len);
	msiDataObjClose(*W_FD, *foo1);
	
	
	### Trim newly created replica
	if (int(*dataIdCount) == 0)
	then
	{
		msiDataObjTrim(*inputObj, *rescName, "null", "1", "null", *foo2);
	}
	
	
	#Print stuff out if needed
	#writeLine("stdout", *stdoutStr);
	
}  
INPUT *inputObj="/renci/home/antoine/R/Negative Binomial test 1.R", *rescName="renci-vault1"
OUTPUT ruleExecOut