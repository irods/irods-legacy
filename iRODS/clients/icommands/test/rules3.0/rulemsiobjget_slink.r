myTestRule {
# Input parameters are:
#   inRequestPath - the string sent to the remote iRODS data grid
#   inFileMode - the cache file creation mode
#   inFileFlags - the access modes for the cache file
#   inCacheFilename - the full path of the cache file
# No output parameters
# Output is the creation of a file on the local cache
  msiobjget_slink(*Request, *Mode, *Flags, *Path);
}
INPUT *Request ="//slink:/renci/home/rods/README.txt", *Mode = "w", *Flags = "O_RDWR", *Path = "/home/reagan/Vaulttest/home/rods/sub1/rodsfile"
OUTPUT ruleExecOut
