myTestRule {
# Input parameters are:
#   inRequestPath - the string sent to the remote database object
#   inFileMode - the cache file creation mode
#   inFileFlags - the access modes for the cache file
#   inCacheFilename - the full path of the cache file
# No output parameters
# Output is the creation of a file on the vault
  msiobjget_dbo(*Request, *Mode, *Flags, *Path);
}
INPUT *Request ="//dbo:dbr2:/tempZone/home/rods/dbotest/lt.pg", *Mode = "w", *Flags = "O_RDWR", *Path = "/tempZone/home/rods/sub1/rodsfile"
OUTPUT ruleExecOut
