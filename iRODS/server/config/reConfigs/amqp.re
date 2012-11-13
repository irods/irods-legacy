# rules for amqp
# requires python and pika
# send a msg
amqpSend: string * string * ? -> integer
amqpSend(*Host, *Queue, *Msg) {
	*HostArg = execCmdArg(*Host);
	*QueueArg = execCmdArg(*Queue);
	*MsgArg = execCmdArg(*Msg);
	msiExecCmd("amqpsend.py", "*Host *Queue *Msg", "null", "null", "null", *Out);
}

# receive a msg
amqpRecv: string * string * output boolean * output string -> integer
amqpRecv(*Host, *Queue, *Emp, *Msg) {
    *HostArg = execCmdArg(*Host);
    *QueueArg = execCmdArg(*Queue);
	msiExecCmd("amqprecv.py", "*Host *Queue", "null", "null", "null", *Out);
    msiGetStdoutInExecCmdOut(*Out, *Msg);
    *Emp = strlen(*Msg) == 0;
    if(!*Emp) {
        *Msg = trimr(*Msg, "\n");        
    }
}

# Xmsg to AMQP bridge
# Messages are of the format "Host:Queue:Msg", assuming that there is no ":" in Host or Queue
startXmsgAmqpBridge(*Tic, *Log) {
	delay("<EF>30s</EF>") {
		*Found = false;
		foreach(*A in listcorerules()) {
			if(*A == "ampqSend") {
				*Found = true;
				break;
			}
		}
		foreach(*A in listapprules()) {
			if(*A == "ampqSend") {
				*Found = true;
				break;
			}
		}
		if(!*Found) {
			msiAdmAddAppRuleStruct("amqp", "", "");
		}
		# msiXmsgServerConnect(*Conn);
		while(true) {
			if(*Log) {
				writeLine("serverLog", "waiting for message with ticket *Tic");
			}
			*ErrorCode = errorcode(readXMsg(str(*Tic), "", *MNum, *SNum, *MHdr, *XMsg, *MUser, *MAddr));
			if(*ErrorCode < 0) {
				if(*ErrorCode == -63000) {
					writeLine("serverLog", "no more xmessages");
					break;
				} else {
					fail(*ErrorCode);
				}
			} else {
				if(*Log) {
					writeLine("serverLog", "received xmessage '*XMsg'");
				}
	
				*QueueMsg = triml(*XMsg, ":");
				*Host = substr(*XMsg, 0, strlen(*XMsg) - strlen(*QueueMsg) - 1);
				*Msg = triml(*QueueMsg, ":");
				*Queue = substr(*QueueMsg, 0, strlen(*QueueMsg) - strlen(*Msg) - 1);
				if(*Log) {
					writeLine("serverLog", "sending amqp message '*Host:*Queue:*Msg'");
				}
				amqpSend(*Host, *Queue, *Msg);
			}
		}
		# msiXmsgServerDisConnect(*Conn);
	}
}

# AMQP to Xmsg bridge
# Messages are read from *Queue on *Host, and written to stream with ticket *Tic
startAmqpXmsgBridge(*Host, *Queue, *Tic, *Log) {
	delay("<EF>30s</EF>") {
		*Found = false;
		foreach(*A in listcorerules()) {
			if(*A == "ampqRecv") {
				*Found = true;
				break;
			}
		}
		foreach(*A in listapprules()) {
			if(*A == "ampqRecv") {
				*Found = true;
				break;
			}
		}
		if(!*Found) {
			msiAdmAddAppRuleStruct("amqp", "", "");
		}

		while(true) {
			if(*Log) {
				writeLine("serverLog", "waiting for message from *Host:*Queue");
			}
			*ErrorCode = errorcode(amqpRecv(*Host, *Queue, *Emp, *Msg));
			if(*ErrorCode < 0) {
				writeLine("serverLog", "AMQP receive error");
				fail(*ErrorCode);
			} else if(*Emp) {
				if(*Log) {
					writeLine("serverLog", "no AMQP message");
				}
				break;
			} else {
				if(*Log) {
					writeLine("serverLog", "received AMQP message '*Msg'");
				}
	
				if(*Log) {
					writeLine("serverLog", "sending Xmessage '*Msg' with ticket *Tic");
				}
				writeXMsg(str(*Tic), *Queue, *Msg);
			}
		}
		# msiXmsgServerDisConnect(*Conn);
	}
}
