myTestRule {
# Input parameters are:
#   inRequestPath - the string sent to the remote Storage Resource Broker data grid
#   inFileMode - the cache file creation mode
#   inFileFlags - the access modes for the cache file
#   inCacheFilename - the full path of the cache file
# No output parameters
# Output is the creation of a file on the local vault
  msiobjget_srb(*Request, *Mode, *Flags, *Path);
}
INPUT *Request ="//srb:srbbrick11.sdsc.edu:7676:testuser@sdsc:TESTUSER/UCHRI/home/srbAdmin.uchri/testdir/testFile", *Mode = "w", *Flags = "O_RDWR", *Path = "/home/reagan/Vaulttest/home/rods/sub1/rodsfile"
OUTPUT ruleExecOut
