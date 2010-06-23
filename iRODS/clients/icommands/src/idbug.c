/*** Copyright (c), The University of North Carolina            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* idbug.c - debug rule execution with single stepping across micro-services */

#include "rodsClient.h" 

rcComm_t *conn;
rodsEnv myRodsEnv;
rErrMsg_t errMsg;

int sendIDebugCommand(sendXmsgInp_t *sendXmsgInp)
{
  int status;


  conn = rcConnectXmsg (&myRodsEnv, &errMsg);
  if (conn == NULL) {
    fprintf (stderr, "rcConnect error\n");
    exit (1);
  }
  status = clientLogin(conn);
  if (status != 0) {
    fprintf (stderr, "clientLogin error\n");
    rcDisconnect(conn);
    exit (7);
  }
  status = rcSendXmsg (conn, sendXmsgInp);
  rcDisconnect(conn);
  if (status < 0) {
    fprintf (stderr, "rsSendXmsg error. status = %d\n", status);
    exit (9);
  }
  return(0);
}

int getIDebugReply(rcvXmsgInp_t *rcvXmsgInp, rcvXmsgOut_t **rcvXmsgOut, int waitFlag)
{
  int status;
  int sleepNum = 1;

  while (1) {
    conn = rcConnectXmsg (&myRodsEnv, &errMsg);
    if (conn == NULL) {
      fprintf (stderr, "rcConnect error\n");
      exit (1);
    }
    status = clientLogin(conn);
    if (status != 0) {
      fprintf (stderr, "clientLogin error\n");
      rcDisconnect(conn);
      exit (7);
    }
    status = rcRcvXmsg (conn, rcvXmsgInp, rcvXmsgOut);
    rcDisconnect(conn);
    if (status >= 0 || waitFlag == 0)
      return(status);
    sleep(sleepNum);
    if (sleepNum < 10 )
      sleepNum++;
  }
  return(0);
}

int
main(int argc, char **argv)
{
  int status, i;
    int mNum = 1;
    int tNum = 3;
    char myHostName[MAX_NAME_LEN];
    getXmsgTicketInp_t getXmsgTicketInp;
    xmsgTicketInfo_t xmsgTicketInfo;
    xmsgTicketInfo_t *outXmsgTicketInfo;
    sendXmsgInp_t sendXmsgInp;
    rcvXmsgInp_t rcvXmsgInp;
    rcvXmsgOut_t *rcvXmsgOut = NULL;
    char  ubuf[4000];
    if (argc < 2 || argc > 2 || !strcmp(argv[1], "-h")) {
      printf("usage: %s [n]\n" , argv[0]);
      printf("  [n]: optional ticket number. default is 3\n");
      printf("       if 0 it will create a new stream and give the stream number\n");
      printf("       which can be used in debugging icommands\n");
      exit(1);
    }

    status = getRodsEnv (&myRodsEnv);
    
    if (status < 0) {
	fprintf (stderr, "getRodsEnv error, status = %d\n", status);
	exit (1);
    }

    myHostName[0] = '\0';
    gethostname (myHostName, MAX_NAME_LEN);
    memset (&xmsgTicketInfo, 0, sizeof (xmsgTicketInfo));

    if (!strcmp(argv[1], "0")) {
      memset (&getXmsgTicketInp, 0, sizeof (getXmsgTicketInp));
      conn = rcConnectXmsg (&myRodsEnv, &errMsg);
      if (conn == NULL) {
        fprintf (stderr, "rcConnect error\n");
        exit (1);
      }
      status = clientLogin(conn);
      if (status != 0) {
        fprintf (stderr, "clientLogin error\n");
        rcDisconnect(conn);
        exit (7);
      }
      status = rcGetXmsgTicket (conn, &getXmsgTicketInp, &outXmsgTicketInfo);
      rcDisconnect(conn);
      if (status != 0) {
        fprintf (stderr, "rcGetXmsgTicket error. status = %d\n", status);
        exit (8);
      }
      printf("Send Ticket Number= %i\n",outXmsgTicketInfo->sendTicket);
      printf("Recv Ticket Number= %i\n",outXmsgTicketInfo->rcvTicket);
      printf("Ticket Expiry Time= %i\n",outXmsgTicketInfo->expireTime);
      printf("Ticket Flag       = %i\n",outXmsgTicketInfo->flag);
      tNum = outXmsgTicketInfo->sendTicket;
      free (outXmsgTicketInfo);
      rcDisconnect(conn);
    }
    memset (&sendXmsgInp, 0, sizeof (sendXmsgInp));
    memset (&rcvXmsgInp, 0, sizeof (rcvXmsgInp));
    xmsgTicketInfo.sendTicket = tNum;
    xmsgTicketInfo.rcvTicket = tNum;
    xmsgTicketInfo.flag = 1;
    sendXmsgInp.ticket = xmsgTicketInfo;
    sendXmsgInp.sendXmsgInfo.numRcv = 1;
    sendXmsgInp.sendXmsgInfo.msg = ubuf;
    rcvXmsgInp.rcvTicket = tNum;
    snprintf(sendXmsgInp.sendXmsgInfo.msgType, HEADER_TYPE_LEN, "idbug:%s",
	     myHostName);
    while (1) {
      /* wait for user */
      printf("> ");
      ubuf[0] ='\0';
      if (fgets (ubuf, 3999, stdin) == NULL) {
	printf("Exitting idbug\n");
	exit(0);
      }
      if (ubuf[0] == '\n'  || ubuf[0] == ' ') { /* see if any messages are there */
	/* get the response */
        rcvXmsgInp.msgNumber = mNum;
        status = getIDebugReply(&rcvXmsgInp, &rcvXmsgOut, 1);                         \

        if (status < 0) {
          fprintf (stderr, "rsRcvXmsg error. status = %d\n", status);
          exit (9);
        }
        printf ("%s#%i:: %s",
                rcvXmsgOut->msgType, rcvXmsgOut->seqNumber, rcvXmsgOut->msg);
        if (rcvXmsgOut->msg[strlen(rcvXmsgOut->msg)-1] != '\n')
          printf("\n");
      }
      else if (ubuf[0] == 'n' || ubuf[0] == 's' || ubuf[0] == 'w') { /* next or where */
        /* send a command to the rule engine */
	sendXmsgInp.sendXmsgInfo.msgNumber = mNum;
	sendIDebugCommand(&sendXmsgInp);
	mNum++;
	/* get the response */
	rcvXmsgInp.msgNumber = mNum;
	status = getIDebugReply(&rcvXmsgInp, &rcvXmsgOut, 1);				
	if (status < 0) {
	  fprintf (stderr, "rsRcvXmsg error. status = %d\n", status);
          exit (9);
	}
	printf ("%s#%i:: %s",
		rcvXmsgOut->msgType, rcvXmsgOut->seqNumber, rcvXmsgOut->msg);
	if (rcvXmsgOut->msg[strlen(rcvXmsgOut->msg)-1] != '\n')
	  printf("\n");
      }
      else if (ubuf[0] == 'e') { /* examine */
	i = 1;
	while (ubuf[i] == ' ') i++;
	if (ubuf[i] == '\0') {
	  printf("No attribute name is given\n");
	  continue;
	}
	sendXmsgInp.sendXmsgInfo.msgNumber = mNum;
        sendIDebugCommand(&sendXmsgInp);
        mNum++;
        /* get the response */
        rcvXmsgInp.msgNumber = mNum;
        status = getIDebugReply(&rcvXmsgInp, &rcvXmsgOut, 1);
        if (status < 0) {
          fprintf (stderr, "rsRcvXmsg error. status = %d\n", status);
          exit (9);
        }
        printf ("%s#%i:: %s",
                rcvXmsgOut->msgType, rcvXmsgOut->seqNumber, rcvXmsgOut->msg);
        if (rcvXmsgOut->msg[strlen(rcvXmsgOut->msg)-1] != '\n')
          printf("\n");
      }
      else if (ubuf[0] == 'c') { /* continue */
        /* send a command to the rule engine */
        sendXmsgInp.sendXmsgInfo.msgNumber = mNum;
        sendIDebugCommand(&sendXmsgInp);
        mNum++;
        /* get the response */
        rcvXmsgInp.msgNumber = mNum;
        status = getIDebugReply(&rcvXmsgInp, &rcvXmsgOut, 1);
        if (status < 0) {
          fprintf (stderr, "rsRcvXmsg error. status = %d\n", status);
          exit (9);
        }
        printf ("%s#%i:: %s",
                rcvXmsgOut->msgType, rcvXmsgOut->seqNumber, rcvXmsgOut->msg);
        if (rcvXmsgOut->msg[strlen(rcvXmsgOut->msg)-1] != '\n')
          printf("\n");
      }
      else if (ubuf[0] == 'l') { /* list rule (current is default) */
        /* send a command to the rule engine */
        sendXmsgInp.sendXmsgInfo.msgNumber = mNum;
        sendIDebugCommand(&sendXmsgInp);
        mNum++;
        /* get the response */
        rcvXmsgInp.msgNumber = mNum;
        status = getIDebugReply(&rcvXmsgInp, &rcvXmsgOut, 1);
        if (status < 0) {
          fprintf (stderr, "rsRcvXmsg error. status = %d\n", status);
          exit (9);
        }
        printf ("%s#%i:: %s",
                rcvXmsgOut->msgType, rcvXmsgOut->seqNumber, rcvXmsgOut->msg);
        if (rcvXmsgOut->msg[strlen(rcvXmsgOut->msg)-1] != '\n')
          printf("\n");
      }
      else if (ubuf[0] == 'b') { /* set break */
        i = 1;
        while (ubuf[i] == ' ') i++;
        if (ubuf[i] == '\0') {
          printf("No attribute name is given\n");
          continue;
        }
        sendXmsgInp.sendXmsgInfo.msgNumber = mNum;
        sendIDebugCommand(&sendXmsgInp);
        mNum++;
        /* get the response */
        rcvXmsgInp.msgNumber = mNum;
        status = getIDebugReply(&rcvXmsgInp, &rcvXmsgOut, 1);
        if (status < 0) {
          fprintf (stderr, "rsRcvXmsg error. status = %d\n", status);
          exit (9);
        }
        printf ("%s#%i:: %s",
                rcvXmsgOut->msgType, rcvXmsgOut->seqNumber, rcvXmsgOut->msg);
        if (rcvXmsgOut->msg[strlen(rcvXmsgOut->msg)-1] != '\n')
          printf("\n");
      }
      else if (ubuf[0] == 'h') { /* print help */
	printf("idbug supports the following command line options:\n");
	printf("n : perform the next micro-service\n");
        printf("l [rule-name] : show the current or named set of rules\n");
        printf("c : continue without stopping\n");
        printf("b [micro-service|rulename] : set break point\n");
        printf("e [$-variable|*-variable] :  examine value of variable\n");
        printf("h : print the help menu\n");
        printf("q : quit the debugger\n");
      }
      else if (ubuf[0] == 'q') { /* quit */
	printf("Exitting idbug\n");
	exit(0);
      }
      else { /* unknown */
	/* need to do step  */
	printf("Unknown command: %s\n",ubuf);
      }
    }
    exit (0);
}
