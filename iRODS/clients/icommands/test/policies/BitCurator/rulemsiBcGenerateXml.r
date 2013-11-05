bcGenerateFiwalkRule {
# Command to be executed is located in directory irods/server/bin/cmd/fiwalk
# This rule Invokes the Fiwalk tool to generate the XML output of the given 
# disk image.
#
# Input Parameter is:
#   Image File path
# Output Parameter is: 
#   XML File path 
#  
# Example: 
#  irule -F rulemsiBcGenerateXml.r "*outXmlFile='/Path/to/xmlfile'" "*image='/path/to/image.aff'"
    *Cmd="fiwalk";
    *timeStamp = double (time());

    # Query the metadata catalog to get the Data ID for "image". 
    # First validate the image
    msiIsData(*image, *DataID, *Status);
    if (int(*DataID) == 0) {
        writeLine("stdout", "Please enter a filename");
        fail;
    }

    # Image exists

    msiSplitPath(*image, *Coll, *File);

    # Now Make a query to get the absolute path to the image and the resource name
    *Query = select DATA_PATH, DATA_RESC_NAME where DATA_ID = '*DataID';
    foreach (*row in *Query) {
        *Path = *row.DATA_PATH;
        *Resource = *row.DATA_RESC_NAME;
        writeLine("stdout", "PAth = *Path, Resource= *Resource");
 
    }

    # Make another query for IP Address of the resource
    # RESC_LOC: Resource IP Address
    # DATA_RESC_NAME: Logical name of storage resource
    *Query2 = select RESC_LOC where DATA_RESC_NAME = '*Resource';
    foreach (*row in *Query2) {
        *Addr = *row.RESC_LOC;
        writeLine("stdout", "ADDR = *Addr, Resource= *Resource");
    }

    *Arg1 = execCmdArg("-f");
    *Arg2 = execCmdArg("-X");
    *prefixStr = "*timeStamp$userNameClient";
    *tempStr = "/tmp/*prefixStr" ++ "outXmlFile";

    # Shellscript will remove /tmp/*timeStamp$userNameClient*

    *Arg3 = execCmdArg(*tempStr);
    *Arg4 = execCmdArg(*Path);

    writeLine("stdout","Running Fiwalk Command...");
    writeLine ("stdout","Command: *Cmd *Arg1 *Arg2 *Arg3 *Arg4");

    if (errorcode(msiExecCmd(*Cmd,"*Arg1 *Arg2 *Arg3 *Arg4","null","*image","null",*Result)) < 0) {
        if(errormsg(*Result,*msg)==0) { 
            msiGetStderrInExecCmdOut(*Result,*Out); 
            writeLine("stdout", "ERROR: *Out");
        } else {
            writeLine("stdout", "Result msg is empty");
        }
    } else {
        # Command executed successfully
        msiGetStdoutInExecCmdOut(*Result,*Out);
        writeLine("stdout", "Output is *Out ");

        # Clean up the temporary files
        cleanup(*Addr, *tempStr, *outXmlFile, *prefixStr, *status); 
    }
} 

# Function: cleanup: Calls a script to remove the temporary files created
# in /tmp
cleanup: input string * input string * input string * input string * output integer -> integer 
cleanup(*Addr, *tempStr, *outXmlFile, *prefixStr, *status) {
       remote(*Addr, "null") {
            *local = "localPath=*tempStr++++forceFlag="; #str(*options);
            writeLine("stdout", "cleanup: local: *local");
            writeLine("stdout", "cleanup: outXmlFile: *outXmlFile");
            writeLine("stdout", "cleanup: tempStr: *tempStr");
            
            msiDataObjPut(*outXmlFile, "null", *local, *status);
            *Arg1 =  execCmdArg(*prefixStr);          
            msiExecCmd("tmpCleanup", *Arg1, "null", "null", "null", *Result); 
       }
}
INPUT *outXmlFile="/AstroZone/home/pixel/bcfiles/xmlfile", *image="/AstroZone/home/pixel/bcfiles/charlie-work-usb-2009-12-11.aff"
OUTPUT ruleExecOut
