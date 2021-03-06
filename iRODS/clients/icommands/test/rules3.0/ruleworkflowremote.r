myTestRule {
#Workflow operation to execute microservices at a remote location
#Input parameters are:
#  Host name where workflow is executed
#  Delaycondition for executing the workflow
#  Workflow ::: recovery-workflow  that will be executed, listed in brackets
#Output from running the example written to server log:
#  writeLine: inString = local exec
#  writeLine: inString = remote exec
#Output from running the example written to standard out:
#  local exec
  msiSleep("20","20");
  writeLine("serverLog","local exec");
  remote(*where, "null") {
    writeLine("serverLog","remote exec");
  }
  writeLine("stdout", "local exec");
}
INPUT *where=$"localhost"
OUTPUT ruleExecOut
