netcdfTest () {
	if (msiNcOpen (*ncTestPath, "0", *ncid) == 0) {
	    writeLine("stdout", "msiNcOpen success, ncid = *ncid");
	} else {
	    writeLine("stdout", "msiNcOpen failed");
	    fail;
	} 
	if (msiNcInqId ("longitude", 1, *ncid, *londimid) == 0) {
            writeLine("stdout", "msiNcInqId success, londimid = *londimid");
        } else {
            writeLine("stdout", "msiNcInqId failed");
            fail;
        } 
        if (msiNcInqWithId (*londimid, 1, *ncid, *inqOut) == 0) {
# inqOut is a struct.
            writeLine("stdout", "msiNcInqWithId londimid success");
        } else {
            writeLine("stdout", "msiNcInqWithId failed");
            fail;
        } 
        if (msiNcInqId ("latitude", 1, *ncid, *latdimid) == 0) {
            writeLine("stdout", "msiNcInqId success, latdimid = *latdimid");
        } else {
            writeLine("stdout", "msiNcInqId failed");
            fail;
        } 
# variables
        if (msiNcInqId ("pressure", 0, *ncid, *pressvarid) == 0) {
            writeLine("stdout", "msiNcInqId success, pressvarid = *pressvarid");
        } else {
            writeLine("stdout", "msiNcInqId failed");
            fail;
        }
        if (msiNcInqWithId (*pressvarid, 0, *ncid, *pressinqout) == 0) {
# inqOut is a struct.
            writeLine("stdout", "msiNcInqWithId pressvarid success");
        } else {
            writeLine("stdout", "msiNcInqWithId failed");
            fail;
        }
        if (msiNcGetVarsByType ("NC_FLOAT", *ncid, *pressvarid, "2", "0%0", "3%5", "1%1", *getVarsOut) == 0) {
# inqOut is a struct.
            writeLine("stdout", "msiNcGetVarsByType pressvarid success");
        } else {
            writeLine("stdout", "msiNcGetVarsByType pressvarid failed");
            fail;
        }

        if (msiNccfGetVara (*ncid, *pressvarid, "0", "0", "30.0", "41.0", "-120.0", "-96.0", 1000, *getVaraOut) == 0) {
# inqOut is a struct.
            writeLine("stdout", "msiNccfGetVara pressvarid success");
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
INPUT *ncTestPath="/wanZone/home/rods/netcdf/sfc_pres_temp.nc"
OUTPUT ruleExecOut
