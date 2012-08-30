# netcdfCfTest - test the libcf subseting msi msiNccfGetVara
# The netcdf test files pres_temp_4D.nc and sfc_pres_temp.nc can be found
# in the netcdf directory 

netcdfCfTest () {
	if (msiNcOpen (*ncTestPath, "0", *ncid) == 0) {
	    writeLine("stdout", "msiNcOpen success, ncid = *ncid");
	} else {
	    writeLine("stdout", "msiNcOpen failed");
	    fail;
	} 

# msiNccfGetVara test

#        if (msiNcGetVarIdInInqOut (*ncInqOut, "temperature", *tempvarid) != 0) {
#            writeLine("stdout", "msiNcGetVarIdInInqOut failed");
#            fail;
#        }
# Need to do msiNcInqId for msiNccfGetVara to succeed
       if (msiNcInqId ("temperature", 0, *ncid, *tempvarid) == 0) {
            writeLine("stdout", "msiNcInqId success, tempvarid = *tempvarid");
        } else {
            writeLine("stdout", "msiNcInqId failed");
            fail;
        }
       if (msiNcInqId ("latitude", 1, *ncid, *latid) == 0) {
            writeLine("stdout", "msiNcInqId success, latid = *latid");
        } else {
            writeLine("stdout", "msiNcInqId failed");
            fail;
        }
       if (msiNcInqId ("longitude", 1, *ncid, *lonid) == 0) {
            writeLine("stdout", "msiNcInqId success, lonid = *lonid");
        } else {
            writeLine("stdout", "msiNcInqId failed");
            fail;
        }
       if (msiNcInqId ("latitude", 0, *ncid, *latvid) == 0) {
            writeLine("stdout", "msiNcInqId success, latvid = *latvid");
        } else {
            writeLine("stdout", "msiNcInqId failed");
            fail;
        }
       if (msiNcInqId ("longitude", 0, *ncid, *lonvid) == 0) {
            writeLine("stdout", "msiNcInqId success, lonvid = *lonvid");
        } else {
            writeLine("stdout", "msiNcInqId failed");
            fail;
        }


        if (msiNccfGetVara (*ncid, *tempvarid, "0", "0", "30.0", "41.0", "-120.0", "-96.0", 1000, *tempVaraOut) == 0) {
# inqOut is a struct.
            writeLine("stdout", "msiNccfGetVara tempvarid success");
            if (msiNcGetArrayLen (*tempVaraOut, *tempArrayLen) == 0) {
                writeLine ("stdout", "tempArrayLen = *tempArrayLen");
                if (msiNcGetDataType (*tempVaraOut, *tempDataType) == 0) {
                    writeLine("stdout", "tempDataType = *tempDataType");
                } else {
                    writeLine("stdout", "msiNcGetDataType temp failed");
                    fail;
               }
               for(*I=0;*I<*tempArrayLen;*I=*I+1) {
                    msiNcGetElementInArray (*tempVaraOut, *I, *element);
                    if (*tempDataType == 5) {
# float. writeLine cannot handle float yet.
                        msiFloatToString (*element, *floatStr);
                        writeLine("stdout", "pressure *I: *floatStr");
                    } else {
                        writeLine("stdout", "pressure *I: *element");
                    }
                }
            } else {
                writeLine("stdout", "msiNcGetArrayLen failed");
                fail;
            }
        } else {
            writeLine("stdout", "msiNccfGetVara pressvarid failed");
            fail;
        }
        if (msiNcClose (*ncid) == 0) {
            writeLine("stdout", "msiNcClose success, ncid = *ncid");
        } else {
            writeLine("stdout", "msiNcClose failed");
            fail;
        }
}
# INPUT *ncTestPath="/wanZone/home/rods/netcdf/pres_temp_4D.nc"
INPUT *ncTestPath="/wanZone/home/rods/netcdf/sfc_pres_temp.nc"
OUTPUT ruleExecOut,*ncInqOut
