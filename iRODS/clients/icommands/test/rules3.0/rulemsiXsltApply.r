myTestRule {
#Transform an XML file according to a provided XSL stylesheet
#
#This rule calls micro-service xsltApplyStylesheet from libsxlt to do the transformation
#
#Typical usage:
#  Transform an XML file ot the AVU format required for micro-service msiLoadMetadataFromXML
#
#Input parameters:
#  xsltObj	- the XSL stylesheet file (an iRODS object)
#  xmlObj	- the input XML file (an iRODS object)
#  note input parameters use the full iRODS path names
#Output parameters:
#  outBuf	- buffer containing the transformed XML data
#
#call the micro-service
  msiXsltApply(*xsltObj, *xmlObj, *outBuf);
#write the output buffer to stdout
  writeBytesBuf("stdout",*outBuf);
#write output file
  msiDataObjCreate(*Path,*OFlag,*D_FD);
  msiDataObjWrite(*D_FD,*outBuf,*W_len);
  msiDataObjClose(*D_FD,$Status1);
}
INPUT *xmlObj="/tempZone/home/rods/XML/sample.xml", *xsltObj-"/tempZone/home/rods/XML/sample.xsl", *Path="/tempZone/home/rods/XML/sample-processed.xml", *OFlag="destRescName=demoResc++++forceFlag="
OUTPUT ruleExecOut
