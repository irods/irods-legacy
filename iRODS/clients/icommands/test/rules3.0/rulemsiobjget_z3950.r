myTestRule {
# Input parameters are:
#   inRequestPath - the string sent to the remote Z39.50 site
#   inFileMode - the cache file creation mode
#   inFileFlags - the access modes for the cache file
#   inCacheFilename - the full path of the cache file
# No output parameters
# Output is the name of the file that was created
  msiobjget_z3950(*Request, *Mode, *Flags, *Path);
}
INPUT *Request ="//z3950:z3950.loc.gov:7090/Voyager?query=@attr 1=1003 Marx&recordsyntax=USMARC", *Mode = "w", *Flags = "O_RDWR", *Path = "/home/reagan/Vaulttest/home/rods/sub1/rodsfile"
OUTPUT ruleExecOut
