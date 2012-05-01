netcdfTest () {
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

	if (msiNcGetNdimsInInqOut (*ncInqOut, "null", *ngdims) == 0) {
            writeLine("stdout", "Global number of dims = *ngdims");
        } else {
            writeLine("stdout", "msiNcGetNdimsInInqOut failed");
            fail;
	}
        if (msiNcGetNvarsInInqOut (*ncInqOut, *ngvars) == 0) {
            writeLine("stdout", "Global number of vars = *ngvars");
        } else {
            writeLine("stdout", "msiNcGetNvarsInInqOut failed");
            fail;
        }
        if (msiNcGetNattsInInqOut (*ncInqOut, "null", *ngatts) == 0) {
            writeLine("stdout", "Global number of atts = *ngatts");
        } else {
            writeLine("stdout", "msiNcGetNattsInInqOut failed");
            fail;
        }
        if (msiNcGetFormatInInqOut (*ncInqOut, *format) == 0) {
            writeLine("stdout", "Global format = *format");
        } else {
            writeLine("stdout", "msiNcGetFormatInInqOut failed");
            fail;
        }

        writeLine("stdout", "dimensions:");
        for(*I=0;*I<*ngdims;*I=*I+1) {
            if (msiNcGetDimNameInInqOut (*ncInqOut, *I, "null", *dimName) == 0) {
                writeLine("stdout", "    *I = *dimName");
            } else {
                writeLine("stdout", "msiNcGetDimNameInInqOut failed");
                fail;
            }
        }

        writeLine("stdout", "attributes:");
        for(*I=0;*I<*ngatts;*I=*I+1) {
            if (msiNcGetAttNameInInqOut (*ncInqOut, *I, "null", *attName) == 0) {
                writeLine("stdout", "    *I = *attName");
            } else {
                writeLine("stdout", "msiNcGetAttNameInInqOut failed");
                fail;
            }
        }

	writeLine("stdout", "variables:");
	for(*I=0;*I<*ngvars;*I=*I+1) {
	    if (msiNcGetVarNameInInqOut (*ncInqOut, *I, *varName) == 0) {
		*dimNameStr = "";
                if (msiNcGetNdimsInInqOut (*ncInqOut, *varName, *ndims) != 0) {
                    writeLine("stdout", "msiNcGetNdimsInInqOut *varName failed");
                    fail;
                }
		for(*J=0;*J<*ndims;*J=*J+1) {
		    if (msiNcGetDimNameInInqOut (*ncInqOut, *J, *varName, *dimName) == 0) {
		        msiStrCat (*dimNameStr, *dimName);
		        if (*J < (*ndims - 1)) {
			    msiStrCat (*dimNameStr, ",");
		        }
		    }
		}
                writeLine("stdout", "    *varName (*dimNameStr)");
		if (msiNcGetNattsInInqOut (*ncInqOut, *varName, *natts) == 0) {
		     writeLine("stdout", "      attributes:");
		    for(*J=0;*J<*natts;*J=*J+1) {
			if (msiNcGetAttNameInInqOut (*ncInqOut, *J, *varName, *attName) == 0) {
                            writeLine("stdout", "        *J = *attName");
                        } else {
                            writeLine("stdout", "msiNcGetAttNameInInqOut failed");
                            fail;
                        }
                    }
	        }
            } else {
                writeLine("stdout", "msiNcGetVarNameInInqOut failed");
                fail;
	    }
        }
        if (msiNcClose (*ncid) == 0) {
            writeLine("stdout", "msiNcClose success, ncid = *ncid");
        } else {
            writeLine("stdout", "msiNcClose failed");
            fail;
        }
}
INPUT *ncTestPath="/oneZone/home/rods/netcdf/pres_temp_4D.nc"
OUTPUT ruleExecOut,*ncInqOut
