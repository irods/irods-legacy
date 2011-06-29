doSomething {
	msiDoSomething("", *keyValOut);
	writeKeyValPairs("stdout", *keyValOut, ": ");
}
INPUT null
OUTPUT ruleExecOut