myTestRule {
#Input parameter is:
#  Argument
  writeLine("stdout","Execute command to print out hello");
  print_hello_arg("");
}
INPUT *Arg="iRODS"
OUTPUT ruleExecOut
