ListAvailableMS {
  msiListEnabledMS(*KVPairs); 
  writeKeyValPairs("stdout", *KVPairs, ": ");
}
INPUT null
OUTPUT ruleExecOut
