myTestRule {
# Input parameters are:
#   inRequestPath - the string sent to the remote URL
#   inFileMode - the cache file creation mode
#   inFileFlags - the access modes for the cache file
#   inCacheFilename - the full path of the cache file on the local system
# No output parameters
# Output is the creation of a file in the vault
#   Wrote local file /home/reagan/Vaulttest/webfile from request http://irods.org/pubs/iRODS_FACT_Sheet-0907c.pdf
  msiobjget_http(*Request, *Mode, *Flags, *Path);
  writeLine("stdout","Wrote local file *Path from request *Request");
}
INPUT *Request ="http://irods.org/pubs/iRODS_Fact_Sheet-0907c.pdf", *Mode = "w", *Flags = "O_RDWR", *Path = "/home/reagan/Vaulttest/webfile"
OUTPUT ruleExecOut
