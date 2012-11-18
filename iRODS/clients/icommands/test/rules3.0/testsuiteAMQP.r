# test amqp client
# python, pika 0.9.5, and rabbitmq 2.8.7 server running on localhost:5672
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

testAmqp1(*RES) {
	msiAdmAddAppRuleStruct("amqp","","");
	*Message = "Message";
	*Host = "localhost";
	*Queue = "queue";
	amqpSend(*Host, *Queue, *Message);
	amqpRecv(*Host, *Queue, *Emp, *Message2);
	assert("``*Message`` == ``*Message2``", *RES);
}

# this test rule requires starting Xmsg server
testAmqp2(*RES) {
	msiAdmAddAppRuleStruct("amqp","","");
	*Message = "Message";
	*Host = "localhost";
	*Queue = "queue";
	
	msiXmsgServerConnect(*Conn);
    msiXmsgCreateStream(*Conn,*A,*Tic);
    msiCreateXmsgInp("1","0","1","*Host:*Queue:*Message","0","" ,"" ,"" ,*Tic,*MParam);
    msiSendXmsg(*Conn,*MParam);
    msiXmsgServerDisConnect(*Conn);
    
    startXmsgAmqpBridge(*Tic, 1);
	
    *Retries = 0;
    *Emp = true;
    while(*Emp && *Retries < 10) {
		*Retries=*Retries+1;
		msiSleep("5", "0");
		amqpRecv(*Host, *Queue, *Emp, *Message2);
	}
	assert("``*Message`` == ``*Message2``", *RES);
}
INPUT null
OUTPUT ruleExecOut	
