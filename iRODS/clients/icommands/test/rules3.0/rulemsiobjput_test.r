myTestRule {
# Input parameters are:
#   inMSOPath - the string that specifies a test of the microservice object framework
#   inCacheFilename - the full path of the cache file in the local storage vault
#   inFileSize - the size of the cache file, found from a ls command on the storage vault
# Output from the rule is located in the rodsLog file:
#   MSO_TEST file contains: This is a test
  msiobjput_test(*Request, *Path, *Size);
}
INPUT *Request ="//test:Test string", *Path = "/home/reagan/Vault/home/rods/rodsfile", *Size="15"
OUTPUT ruleExecOut
