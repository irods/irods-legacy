rule {
	testSuite;
}

assert(*B) {
	assert(*B, *RES);
        writeLine("stdout", "*RES");
}

assertFalse(*B) {
	assertFalse(*B, *RES);
        writeLine("stdout", "*RES");
}

assertError(*B, *RES) {
	assert(*B, *RES);
        if(*RES like "[error]*") {
                *RES="[passed]";
        } else {
                *RES="[failed]";
        }

}

assert(*B,*RES) {
        if (errormsg(*RET = eval(*B),*E)==0) {
            if (*RET) {
                *RES="[passed]";
            } else {
                *RES="[failed]";
            }
        } else {
            *RES="[error]\n*E\n";
        }
	writeLine("stdout", "*B");
}

assertFalse(*B, *RES) {
    assert("*B == false", *RES);
}

testSuite {
    runTests("test.*");
}
runTests(*Pattern) {
    *Start = time();
    *A = listextrules;
    *P=0;
    *F=0;
    *E=0;
    *O=0;
    *U=0;
    *FN="";
    *EN="";
    *C=0;
    foreach(*A) {
        if(*A like regex *Pattern && *A != "testSuite") then {
		writeLine("stdout", "*C: *A");
		if(arity(*A) == 1) then {
			eval("*A(\*RES)");
			writeLine("stdout", *RES);
			if(*RES like "[passed]*") then {*P=*P+1;} 
			else if(*RES like "[failed]*") then {*F=*F+1;*FN="*FN*A\n"} 
			else if(*RES like "[error]*") then {*E=*E+1;*EN="*EN*A\n"} 
			else { *O=*O+1; }
		} else {
			*U=*U+1;
			eval(*A);
		}
		*C=*C+1;
	}
    }
    writeLine("stdout", "\nsummary *P passed *F failed *E error *O other *U undefined");
    if(*F!=0) {
        writeLine("stdout", "failed: *FN");
    }
    if(*E!=0) {
        writeLine("stdout", "error: *EN");
    }
    *Finish = time();
    *Time = double(*Finish) - double(*Start);
    writeLine("stdout", "time: *Time s");
}
testKV1(*RES) {
	
        assert(``{*kv.A = "a";*kv.B = "b";*kv.A=="a"}``, *RES);
}
testKV11(*RES) {
	
        assert(``{*kv.A = "a";*kv.A = "b";*kv.A=="b"}``, *RES);
}
testKV12(*RES) {
	
        assert(``{*kv.A = "a";*kv.B = "b";*kv.B="c";*kv.B=="c"}``, *RES);
}
testKV2(*RES) {
	assert(``{*kv.A = "a";*kv.B = "b";*kv.B=="b"}``, *RES);
}
testKV3(*RES) {
	assertError(``{*kv.A = 1;*kv.B = 2}``, *RES);
}
testKV4(*RES) {
	assertError(``{*kv=1;*kv.A = "a"}``, *RES);
}
testKV5(*RES) {
	assertError(``{*kv.*A = "a"}``, *RES);
}
testKV6(*RES) {
	assertError(``{*kv.A("a") = "a"}``, *RES);
}
testKV7(*RES) {
	
        assert(``{*kv."A" = "a";*kv."B" = "b";*kv."A"=="a"}``, *RES);
}
testKV8(*RES) {
	
        assert(``{*kv."A B" = "a";*kv.A = "b";*kv."A B"=="a"}``, *RES);
}
testKV9(*RES) {
	
        assert(``{*kv."A B"= "a";*kv.B = "b";*kv.B="c";msiGetValByKey(*kv, "A B", *val);*val=="a"}``, *RES);
}
testKV10(*RES) {
	assert(``{msiAddKeyVal(*kv,"A B","a");*kv.B = "b";*kv."A B"=="a"}``, *RES);
}

input null
output ruleExecOut
