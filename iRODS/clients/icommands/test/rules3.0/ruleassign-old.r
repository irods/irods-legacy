myTestRule {
#The assign micro-service is replaced with algebraic equations
#Input parameters are:
#  Result variable
#  Input variable
#Output from running the example is:
#  assign
    assign(*A,*B);
    writeLine("stdout", *A);
}
INPUT *B="assign"
OUTPUT ruleExecOut

