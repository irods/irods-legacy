serverBackup {
	msiServerBackup("", *keyValOut);
	writeKeyValPairs("stdout", *keyValOut, ": ");
}
INPUT null
OUTPUT ruleExecOut