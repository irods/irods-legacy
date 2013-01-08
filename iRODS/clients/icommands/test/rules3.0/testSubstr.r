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
testSubstr1(*RES) {
        assert(``substr("str",0,0)==""``, *RES);
}
testSubstr2(*RES) {
        assert(``substr("str",3,3)==""``, *RES);
}
testSubstr3(*RES) {
	assert(``substr("str",0,3)=="str"``, *RES);
}
testSubstr4(*RES) {
	assert(``substr("str",1,2)=="t"``, *RES);
}
testSubstrError1(*RES) {
	assertError(``substr("str",-1,3)``, *RES);
}
testSubstrError2(*RES) {
	assertError(``substr("str",0,4)``, *RES);
}
testSubstrError3(*RES) {
	assertError(``substr("str",3,0)``, *RES);
}
testSubstrError4(*RES) {
	assertError(``substr("str",0,-1)``, *RES);
}
testSubstrError5(*RES) {
	assertError(``substr("str",4,3)``, *RES);
}
testSubstrError6(*RES) {
	assertError(``substr("str",-2,-1)``, *RES);
}
testSubstrError7(*RES) {
	assertError(``substr("str",4,5)``, *RES);
}
input null
output ruleExecOut
