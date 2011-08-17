myTestRule {
#Input parameters are:
#  Optional format flag - human
#  Optional printf formatting for human format
#Output parameter is:
#Local system time
  msiGetFormattedSystemTime(*Out,"null","null");
  writeLine("stdout",*Out);
}
INPUT null 
OUTPUT ruleExecOut
