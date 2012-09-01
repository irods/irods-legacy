# netcdfTest1 - Test various netcdf msi. Use the comprehensive inquery msi - 
# msiNcInq for inquery instead of inquery of individual variable used in
# netcdfTest2
# The netcdf test files pres_temp_4D.nc and sfc_pres_temp.nc can be found
# in the netcdf directory 
netcdfTest1 () {
	if (msiNcOpen (*ncTestPath, "0", *ncid) == 0) {
	    writeLine("stdout", "msiNcOpen success, ncid = *ncid");
	} else {
	    writeLine("stdout", "msiNcOpen failed");
	    fail;
	} 
# do the big inq
        if (msiNcInq (*ncid, *ncInqOut) == 0) {
            writeLine("stdout", "msiNcInq success");
        } else {
            writeLine("stdout", "msiNcInq failed");
            fail;
        }

        if (msiNcGetNvarsInInqOut (*ncInqOut, *ngvars) == 0) {
            writeLine("stdout", "Global number of vars = *ngvars");
        } else {
            writeLine("stdout", "msiNcGetNvarsInInqOut failed");
            fail;
        }

	writeLine("stdout", "Data:");
	for(*I=0;*I<*ngvars;*I=*I+1) {
	    if (msiNcGetVarNameInInqOut (*ncInqOut, *I, *varName) == 0) {
		if (msiNcGetVarTypeInInqOut (*ncInqOut, *varName, *dataType) != 0) {
		    writeLine("stdout", "msiNcGetVarTypeInInqOut failed");
                    fail;
                }

		if (msiNcIntDataTypeToStr (*dataType, *dataTypeStr) != 0) {
                    writeLine("stdout", "msiNcIntDataTypeToStr failed");
                    fail;
                }
                writeLine("stdout", "    *dataTypeStr *varName ;");
                if (msiNcSubsetVar(*varName, *ncid, *ncInqOut, *subsetStr, *getVarsOut) != 0) {
                   writeLine("stdout", "msiNcSubsetVar failed");
               }

	       if (msiNcGetArrayLen (*getVarsOut, *varArrayLen) != 0) {
		   writeLine("stdout", "msiNcGetArrayLen failed");
               }
               for(*K=0;*K<*varArrayLen;*K=*K+1) {
                   msiNcGetElementInArray (*getVarsOut, *K, *element);
                   if (*dataType == 5) {
# float. writeLine cannot handle float yet.
                       msiFloatToString (*element, *floatStr);
                       writeLine("stdout", "      *K: *floatStr");
                   } else {
                       writeLine("stdout", "      *K: *element");
                   }
               }
	       msiFreeNcStruct (*getVarsOut);
            } else {
	       writeLine("stdout", "msiNcGetVarNameInInqOut failed");
            }
	}
}
INPUT *ncTestPath="/wanZone/home/rods/pydap/oceanModel/hawModel1.nc", *subsetStr="time[10:1:12] depth[3:1:3] lat[20:1:21] lon[30:1:34]"
OUTPUT ruleExecOut,*ncInqOut
