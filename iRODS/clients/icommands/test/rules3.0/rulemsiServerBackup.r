myTestRule {
#Input parameter is:
#  Options - currently none are specified for controlling server backup
#Output parameter is:
#  Result - a keyvalpair structure holding number of files and size
#
#  This will take a while to run.
#
  msiServerBackup(*Opt,*Result);
  writeKeyValPairs("stdout",*Result, " : ");
}
INPUT *Opt=""
OUTPUT ruleExecOut
