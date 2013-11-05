bcGenerateReportsGuiRule {
# Command to be executed located in file irods/server/bin/cmd/bc_reports_tab
# Parameters are:
#  Output: None
#  Input:  None 
# Example: 
#  irule -F rulemsiBcGenerateReportsGui.r 

    *Cmd="bc_reports_tab";

    writeLine("stdout","Running Generate Reports command...");
    writeLine ("stdout","Command: *Cmd ");

    msiExecCmd(*Cmd,"null","null","null","null",*Result);
    if (errorcode(msiExecCmd(*Cmd,"null","null","null","null",*Result)) < 0) {
        if(errormsg(*Result,*msg)==0) {
            msiGetStderrInExecCmdOut(*Result,*Out);
            writeLine("stdout", "ERROR: *Out");
        } else {
            writeLine("stdout", "Result msg is empty");
        }
        
    }
    
    msiGetStdoutInExecCmdOut(*Result,*Out);
    
}
INPUT null
OUTPUT ruleExecOut
