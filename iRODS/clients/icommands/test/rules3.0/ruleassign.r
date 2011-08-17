myTestRule {
#The assign micro-service is replaced with algebraic equations
#Output from running the example is:
#  Value assigned is assign
    *A = *B;
    writeLine("stdout", "Value assigned is *A");
}
INPUT *B="assign"
OUTPUT ruleExecOut

