bcExtractFeatureFilesRule {
# Command to be executed located in directory irods/server/bin/cmd/bulk_extractor
# This rule reads the disk image and generates a number of feature files in
# the specified output directory. It invokes the Bulk Extractor tool.
# Input Parameter is:
#   Image File path
# Output Parameter is: 
#   File Path for Feature Files
#  
# Example: 
# bulk_extractor
#  ~/Research/TestData/M57-Scenario/usbflashdrives/jo-work-usb-2009-12-11.aff
#  -o ~/Research/TestData/BEOutputs/jow-output
# Rule:
#  irule -F rulemsiBcExtractFeatureFiles "*image='/path/to/image.aff'" "outFeatDir='/path/to/outdir'"

    *Cmd="bulk_extractor";
    *timeStamp = double (time());

    # Query the metadata catalog
    # First get the DataID of the image. Fail if the image doesn't exist
    msiIsData(*image, *DataID, *Status);
    if (int(*DataID) == 0) {
        writeLine("stdout", "Please enter a filename");
        fail;
    }

    # Image exists

    msiSplitPath(*image, *Coll, *File);

    # Now Make a query to get the path to the image and the resource name 
    # DATA_PATH: Physical path name for digital object in resource
    # DATA_RESC_NAME: Logical name of storage resource
    *Query = select DATA_PATH, DATA_RESC_NAME where DATA_ID = '*DataID';
    foreach (*row in *Query) {
        *Path = *row.DATA_PATH;
        *Resource = *row.DATA_RESC_NAME;
        writeLine("stdout", "Path = *Path, Resource= *Resource");
 
    }

    # Make another query for IP Address of the resource
    # RESC_LOC: Resource IP Address
    # DATA_RESC_NAME: Logical name of storage resource
    *Query2 = select RESC_LOC where DATA_RESC_NAME = '*Resource';
    foreach (*row in *Query2) {
        *Addr = *row.RESC_LOC;
        writeLine("stdout", "ADDR = *Addr, Resource= *Resource");
    }

    *prefixStr = "*timeStamp$userNameClient";
    *tempStr = "/tmp/*prefixStr" ++ "outFeatDir";

    *Arg1 = execCmdArg(*Path);    # Image
    *Arg2 = execCmdArg("-o");
    *Arg3 = execCmdArg(*tempStr); # Output Feature Directory

    writeLine("stdout","Running Bulk Extractor Tool...");
    writeLine ("stdout","Command: *Cmd *Arg1 *Arg2 *Arg3");

    if (errorcode(msiExecCmd(*Cmd,"*Arg1 *Arg2 *Arg3","null","*image","null",*Result)) < 0) {
        if(errormsg(*Result,*msg)==0) { 
            msiGetStderrInExecCmdOut(*Result,*Out); 
            writeLine("stdout", "ERROR:*Out");
        } else {
            writeLine("stdout", "Result msg is empty");
        }
    } else {
        # Command executed successfully
        msiGetStdoutInExecCmdOut(*Result,*Out);
        writeLine("stdout", "Output is *Out ");

        # Clean up the temporary files
        cleanup(*Addr, *tempStr, *outFeatDir, *prefixStr, *status); 
    }
}

# Function: cleanup: Calls a script to remove the temporary files created
# in /tmp
cleanup: input string * input string * input string * input string * output integer -> integer 
cleanup(*Addr, *tempStr, *outFeatDir, *prefixStr, *status) {

       writeLine("stdout", "Cleanup: Moving *tempStr to *outFeatDir"); 
       remote(*Addr, "null") {
            *local = "localPath=*tempStr++++forceFlag="; #str(*options);
            writeLine("stdout", "cleanup: local: *local");
            writeLine("stdout", "cleanup: outFeatDir: *outFeatDir");
            writeLine("stdout", "cleanup: tempStr: *tempStr");

            msiDataObjPut(*outFeatDir, "null", *local, *status);
            *Arg1 =  execCmdArg(*prefixStr);          
            msiExecCmd("tmpCleanup", *Arg1, "null", "null", "null", *Result); 
       }
}

INPUT *image="/AstroZone/home/pixel/bcfiles/charlie-work-usb-2009-12-11.aff", *outFeatDir="/AstroZone/home/pixel/bcfiles/BeOutFeatDir"
OUTPUT ruleExecOut
