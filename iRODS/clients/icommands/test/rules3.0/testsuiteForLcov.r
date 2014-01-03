rule {
	testSuite;
}

# assert rules
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
		writeLine("stdout", *RES);
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

# tests

intFunc : integer -> integer
intFunc(*x) = *x

fraction = 1.0

bool = false

pathFunc:path->path
pathFunc(*p) = *p

path2 = /$rodsZoneClient/home/$userNameClient

testDynamicCoercionFromUninitializedValue(*RES) {
	assertError(``unspeced==0``, *RES);
}

testFractionToInt(*RES) {
	assertError(``intFunc(fraction)``, *RES);
} 

testBoolToInt(*RES) {
	assert(``intFunc(bool)==0``, *RES);
}

testStringToPath(*RES) {
	assert(``pathFunc("/tempZone/home/rods")``,*RES);
}

testCollectionSpiderForeach(*RES) {
#	writeLine("stdout", path2);
	foreach(*obj in path2) {
#		writeLine("stdout", *obj);
	}
	assert(``true``, *RES);
}

testGenQueryForeach(*RES) {
	*A = select DATA_NAME;
	foreach(*obj in *A) {
		writeLine("stdout", *obj.DATA_NAME);
	}
	assert(``true``, *RES);
}



INPUT null
OUTPUT ruleExecOut	
