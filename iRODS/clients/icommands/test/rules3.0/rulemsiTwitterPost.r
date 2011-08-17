myTestRule {
#Input parameters are:
#  Twitter name
#  Twitter password
#  Message
#Output parameter is:
#  Status
  msiTwitterPost(*Username, *Passwd, *Msg, *Status);
  writePosInt("stdout", *Status);
  writeLine("stdout", " is the status for twitter post");
}
INPUT *Username="rods", *Passwd="password", *Msg="Electronic Records Summer Camp is now open"
OUTPUT ruleExecOut
