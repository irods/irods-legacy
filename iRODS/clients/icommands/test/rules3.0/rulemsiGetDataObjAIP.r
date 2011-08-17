myTestRule {
#Input parameter is:
#  Path of file
#Output parameter is:
#  Buffer listing object metadata in XML format
  msiGetDataObjAIP(*Path,*Buf);
  writeBytesBuf("stdout",*Buf);
}
INPUT *Path="/tempZone/home/rods/sub1/foo1"
OUTPUT ruleExecOut
