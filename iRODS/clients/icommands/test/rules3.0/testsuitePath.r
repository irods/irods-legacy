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

testPath1(*RES) {
	*Path = /tempZone/home/rods/foo;
	msiCollCreate(*Path, "0", *Status) ::: msiRmColl(*Path, "", *Status);
	*Counter = 0;
	foreach(*DataObj in *Path) {
		*Counter = *Counter + 1; 
	}
	msiRmColl(*Path, "", *Status);
	assert("*Counter == 0", *RES);
}

testPath2(*RES) {
	*Path = /tempZone/home/rods/foo;
	msiCollCreate(*Path, "0", *Status) ::: msiRmColl(*Path, "", *Status);
	msiDataObjCreate(/*Path/bar, "", *Status) ::: msiDataObjUnlink(/*Path/bar, *Status);
	msiDataObjCreate(/*Path/baz, "", *Status) ::: msiDataObjUnlink(/*Path/baz, *Status);
	*Counter = 0;
	foreach(*DataObj in *Path) {
		*Counter = *Counter + 1; 
	}
	msiRmColl(*Path, "", *Status);
	assert("*Counter == 2", *RES);
}
testPath3(*RES) {
	*Path = /tempZone/home/rods/foo;
	msiCollCreate(*Path, "0", *Status) ::: msiRmColl(*Path, "", *Status);
	msiDataObjCreate(/*Path/bar, "", *Status) ::: msiDataObjUnlink(/*Path/bar, *Status);
	msiDataObjCreate(/*Path/baz, "", *Status) ::: msiDataObjUnlink(/*Path/baz, *Status);
	*Counter = 0;
	foreach(*DataObj in *Path) {
		*Counter = *Counter + 1;
		break; 
	}
	msiRmColl(*Path, "", *Status);
	assert("*Counter == 1", *RES);
}
INPUT null
OUTPUT ruleExecOut	