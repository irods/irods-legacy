myTestRule {
#Input parameters are:
#  Delay condition composed from tags
#    EA    - host where the execution if performed
#    ET    - Absolute time whe execution is done
#    PLUSET - Relative time for execution
#    EF    - Execution frequency
#    Workflow specified within brackets
#Output from running the example is:
#  exec
#Output written to the reLog.2011.6.1 log file:
# writeLine: inString = Delayed exec 
  delay("<PLUSET>30s</PLUSET><EF>30s</EF>") {
    writeLine("serverLog","Delayed exec");
    }
  writeLine("stdout","exec");
}
INPUT null
OUTPUT ruleExecOut
