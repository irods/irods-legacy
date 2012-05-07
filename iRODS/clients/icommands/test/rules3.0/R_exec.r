###############################################################################################
###		R_exec.r: Runs an R program on a designated node (resource)
###		Assumes a link to R in <iRODS_DIR>/server/bin/cmd/ on that resource
###		AdT, 04/30/12
###############################################################################################  

R_exec 
{
	### Check if object exists on R-enabled resource
	isObjOnResc(*dataIdCount);

	### Replicate obj to target resc if needed
	if (int(*dataIdCount) == 0)
	then
	{
		msiDataObjRepl(*inputObj, *rescName, *foo);
	}
	
	### Get file path and resource address
	getFileInfo(*rescAddr, *filePath);
	
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
	
	### Extract metadata from R obj and adds it to result obj
	extractMetadataFromObj(*outputObj);
	
	### Trim newly created replica
	if (int(*dataIdCount) == 0)
	then
	{
		msiDataObjTrim(*inputObj, *rescName, "null", "1", "null", *foo2);
	}
	
	#Print stuff out if needed
	#writeLine("stdout", *stdoutStr);
}


isObjOnResc(*dataIdCount)
{
	### Split collection and object name
	msiSplitPath(*inputObj, *collection, *object);
	
	### Set up query
	# Add select fields
	msiAddSelectFieldToGenQuery("DATA_ID", "COUNT", *genQInp0);
	# Add conditions
	msiAddConditionToGenQuery("DATA_NAME", "=", *object, *genQInp0);
	msiAddConditionToGenQuery("COLL_NAME", "=", *collection, *genQInp0);
	msiAddConditionToGenQuery("DATA_RESC_NAME", "=", *rescName, *genQInp0);
	
	### Run query
	msiExecGenQuery(*genQInp0, *genQOut0);
	
	### Extract data ID count
  	foreach (*genQOut0)
	{
		msiGetValByKey(*genQOut0, "DATA_ID", *dataIdCount);
	}
}


getFileInfo(*rescAddr, *filePath)
{
	### Split collection and object name
	msiSplitPath(*inputObj, *collection, *object);

	### Set up query
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
}


extractMetadataFromObj(*destObj)
{
	# Set read buffer length
	*r_len=524288;	# 512KB
	
	# Read input object
	*openArgs="objPath="++*inputObj++"++++rescName="++*rescName
	msiDataObjOpen(*openArgs, *FD1);
	msiDataObjRead(*FD1, *r_len, *r_buf);
	msiDataObjClose(*FD1, *status);
	
	# Read object that contains metadata extraction tags
	msiDataObjOpen(*tagObj, *FD2);
	msiDataObjRead(*FD2, *r_len, *r_buf2);
	msiReadMDTemplateIntoTagStruct(*r_buf2, *tags);
	msiDataObjClose(*FD2, *status);
	
	# Extract metadata from input object using tags
	msiExtractTemplateMDFromBuf(*r_buf, *tags, *KVP);
	
	# Add metadata to the object
	msiGetObjType(*destObj, *objType);
	msiAssociateKeyValuePairsToObj(*KVP, *destObj, *objType);
	
	# Write out extracted metadata
	writeKeyValPairs("stdout", *KVP, " : ");
}  


INPUT *inputObj="/renci/home/antoine/R/Negative Binomial test 1 (with MD).R", *rescName="renci-vault1", *tagObj="/renci/home/antoine/R/R_meta.tag"
OUTPUT ruleExecOut