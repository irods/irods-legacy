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
		if (msiNcGetVarTypeInInqOut (*ncInqOut, *varName, *dataType) != 0) {
		    writeLine("stdout", "msiNcGetVarTypeInInqOut failed");
                    fail;
                }

		if (msiNcIntDataTypeToStr (*dataType, *dataTypeStr) != 0) {
                    writeLine("stdout", "msiNcIntDataTypeToStr failed");
                    fail;
                }
                writeLine("stdout", "    *dataTypeStr *varName (*dimNameStr) ;");
		if (msiNcGetNattsInInqOut (*ncInqOut, *varName, *natts) == 0) {
		    for(*J=0;*J<*natts;*J=*J+1) {
			if (msiNcGetAttNameInInqOut (*ncInqOut, *J, *varName, *attName) != 0) {
                            writeLine("stdout", "msiNcGetAttNameInInqOut failed");
                            fail;
                        }
			if (msiNcGetAttValStrInInqOut (*ncInqOut, *J, *varName, *attValStr) != 0) {
                            writeLine("stdout", "msiNcGetAttValStrInInqOut failed");
                            fail;
                        }

                        writeLine("stdout", "        *varName:*attName = *attValStr");
                    }
	        }
            } else {
                writeLine("stdout", "msiNcGetVarNameInInqOut failed");
                fail;
	    }
        }
	writeLine("stdout", "Data:");
        for(*I=0;*I<*ngvars;*I=*I+1) {
            msiNcGetVarTypeInInqOut (*ncInqOut, *varName, *dataType);
            msiNcGetVarNameInInqOut (*ncInqOut, *I, *varName); 
            if (msiNcGetVarIdInInqOut (*ncInqOut, *varName, *varId) != 0) {
                writeLine("stdout", "msiNcGetVarIdInInqOut failed");
                fail;
            }
	    writeLine("stdout", "  *varName =");
            msiNcGetNdimsInInqOut (*ncInqOut, *varName, *ndims);
            for(*J=0;*J<*ndims;*J=*J+1) {
                if (msiNcGetDimLenInInqOut (*ncInqOut, *J, *varName, *dimLen) != 0) {
                    writeLine("stdout", "msiNcGetDimLenInInqOut failed");
                    fail;
		}
                msiAddToNcArray (0, *J, *startArray);
                msiAddToNcArray (*dimLen, *J, *countArray);
                msiAddToNcArray (1, *J, *strideArray);
            }
            if (msiNcGetVarsByType (*dataType, *ncid, *varId, *ndims, *startArray, *countArray, *strideArray, *getVarsOut) != 0) {
                writeLine("stdout", "msiNcGetVarsByType failed");
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
	    msiFreeNcStruct (*startArray);
	    msiFreeNcStruct (*countArray);
	    msiFreeNcStruct (*strideArray);
	}
}
INPUT *ncTestPath="/wanZone/home/rods/netcdf/pres_temp_4D.nc"
OUTPUT ruleExecOut,*ncInqOut
