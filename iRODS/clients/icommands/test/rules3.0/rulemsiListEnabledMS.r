myTestRule {
#Output
#  Buffer holding list of micro-services in form Key=Value
#Output from running the example is:
#    List of micro-services that are enabled
  msiListEnabledMS(*Buf);
  writeKeyValPairs("stdout",*Buf,":");
}
INPUT null
OUTPUT ruleExecOut
