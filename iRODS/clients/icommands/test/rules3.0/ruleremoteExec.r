myTestRule {
#Input parameters are:
#  Host name where workflow is executed
#  Delaycondition for executing the workflow
#  Workflow that will be executed, listed in brackets
#  Recovery workflow
#Output from running the example written to server log:
#  writeLine: inString = local exec
#  writeLine: inString = remote exec
#Output from running the example written to standard out:
#  local exec
  writeLine("serverLog","local exec");
  remote("localhost", "null") {
    writeLine("serverLog","remote exec");
  }
  writeLine("stdout", "local exec");
}
INPUT null
OUTPUT ruleExecOut
