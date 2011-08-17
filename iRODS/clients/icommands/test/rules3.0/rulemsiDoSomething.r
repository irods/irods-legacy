myTestRule {
#Placeholder for creating a rule for a new micro-service
  msiDoSomething("", *keyValOut);
  writeKeyValPairs("stdout", *keyValOut, " : ");
}
INPUT null
OUTPUT ruleExecOut
