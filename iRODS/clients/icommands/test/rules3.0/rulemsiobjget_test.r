myTestRule {
# Input parameters are:
#   inRequestPath - the string that specifies a test of the microservice object framework
#   inFileMode - the cache file creation mode
#   inFileFlags - the access modes for the cache file
#   inCacheFilename - the full path of the cache file
# No output parameters
# Output
#  The specified file contains "PID is pid-number. This is a test"
#  You will need to set the access permission on the file, chmod a+r filename
  msiobjget_test(*Request, *Mode, *Flags, *Path);
}
INPUT *Request ="//test:Test string", *Mode = "r+", *Flags = "O_RDWR", *Path = "/home/reagan/Vault/home/rods/rodsfile"
OUTPUT ruleExecOut
