# netcdfTest3 - Test the subsetting msi - msiNcSubsetVar and max/min/ave
# computation msi - msiNcVarStat 
netcdfTest1 () {
# open the netcdf file
	if (msiNcOpen (*ncTestPath, "0", *ncid) == 0) {
	    writeLine("stdout", "msiNcOpen success, ncid = *ncid");
	} else {
	    writeLine("stdout", "msiNcOpen failed");
	    fail;
	} 
# do the big inquery
        if (msiNcInq (*ncid, *ncInqOut) == 0) {
            writeLine("stdout", "msiNcInq success");
        } else {
            writeLine("stdout", "msiNcInq failed");
            fail;
        }
# get the number of variables - *ngvars from the inquery output
        if (msiNcGetNvarsInInqOut (*ncInqOut, *ngvars) == 0) {
            writeLine("stdout", "Global number of vars = *ngvars");
        } else {
            writeLine("stdout", "msiNcGetNvarsInInqOut failed");
            fail;
        }

	writeLine("stdout", "Data:");
# loop over the *ngvars for subsettting operation
        for(*I=0;*I<*ngvars;*I=*I+1) {
	    if (msiNcGetVarNameInInqOut (*ncInqOut, *I, *varName) != 0) {
               writeLine("stdout", "msiNcGetVarNameInInqOut failed");
               fail;
            }

	    if (msiNcGetVarTypeInInqOut (*ncInqOut, *varName, *dataType) != 0) {
	        writeLine("stdout", "msiNcGetVarTypeInInqOut failed");
                fail;
            }

	    if (msiNcIntDataTypeToStr (*dataType, *dataTypeStr) != 0) {
                writeLine("stdout", "msiNcIntDataTypeToStr failed");
                    fail;
            }
            writeLine("stdout", "    *dataTypeStr *varName ;");
#  subsetting operation
            if (msiNcSubsetVar(*varName, *ncid, *ncInqOut, *subsetStr, *getVarsOut) != 0) {
               writeLine("stdout", "msiNcSubsetVar failed");
               fail;
            }

	    if (msiNcGetArrayLen (*getVarsOut, *varArrayLen) != 0) {
	        writeLine("stdout", "msiNcGetArrayLen failed");
            }
# loop over the result of subsetting and print out each value
            for(*K=0;*K<*varArrayLen;*K=*K+1) {
                msiNcGetElementInArray (*getVarsOut, *K, *element);
                if (*dataType == 5) {
                    msiFloatToString (*element, *floatStr);
                    writeLine("stdout", "      *K: *floatStr");
                } else {
                    writeLine("stdout", "      *K: *element");
                }
            }
# compute max/min/ave of the subsetting results
            if (msiNcVarStat(*getVarsOut, *statOutStr) != 0) {
                writeLine("stdout", "msiNcVarStat failed");
            } else {
               writeLine("stdout", "*statOutStr");
            }
        msiFreeNcStruct (*getVarsOut);
	}
}
INPUT *ncTestPath="/dfctest/home/rodsAdmin/pydap/oceanModel/hawModel1.nc", *subsetStr="time[10:1:12] depth[3:1:3] lat[20:1:21] lon[30:1:34]"
OUTPUT ruleExecOut,*ncInqOut
