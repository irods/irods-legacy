/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* xmsgTest.c - test the high level api */

#include "rodsClient.h" 

int
main(int argc, char **argv)
{
    rcComm_t *conn;
    rodsEnv myRodsEnv;
    int status;
    int mNum = 1;
    int tNum = 1;
    int sNum = 1;
    char myHostName[MAX_NAME_LEN];
    rErrMsg_t errMsg;
    getXmsgTicketInp_t getXmsgTicketInp;
    xmsgTicketInfo_t xmsgTicketInfo;
    xmsgTicketInfo_t *outXmsgTicketInfo;
    sendXmsgInp_t sendXmsgInp;
    rcvXmsgInp_t rcvXmsgInp;
    rcvXmsgOut_t *rcvXmsgOut = NULL;
    char  buf[4000];
    if (argc < 2 || !strcmp(argv[1], "-h")) {
      printf("usage: %s s [n] [m]\n" , argv[0]);
      printf("usage: %s r [n] [m]\n" , argv[0]);
      printf("usage: %s t \n" , argv[0]);
      printf("    s: send \n");
      printf("    r: receive \n");
      printf("    t: get ticket id \n");
      printf("  [n]: optional ticket number. default is 1\n");
      printf("  [m]: optional first message number. default is 1\n");
      exit(1);
    }

    status = getRodsEnv (&myRodsEnv);
    
    if (status < 0) {
	fprintf (stderr, "getRodsEnv error, status = %d\n", status);
	exit (1);
    }

    myHostName[0] = '\0';
    gethostname (myHostName, MAX_NAME_LEN);
    if (argc >= 4) 
      mNum = atoi(argv[3]);
    if (argc >= 3)
      tNum = atoi(argv[2]);
    memset (&xmsgTicketInfo, 0, sizeof (xmsgTicketInfo));
    
    if (!strcmp(argv[1], "s")) {
      memset (&sendXmsgInp, 0, sizeof (sendXmsgInp));
      xmsgTicketInfo.sendTicket = tNum;
      xmsgTicketInfo.rcvTicket = tNum;
      xmsgTicketInfo.flag = 1;
      sendXmsgInp.ticket = xmsgTicketInfo;
      sendXmsgInp.sendXmsgInfo.numRcv = 1;
      snprintf(sendXmsgInp.sendXmsgInfo.msgType, HEADER_TYPE_LEN, "%s:%i",
	       myHostName, getpid ());
      while (fgets (buf, 3999, stdin) != NULL) {
        if (strstr(buf,"/EOM") == buf)
	  exit(0);
	sendXmsgInp.sendXmsgInfo.msgNumber = mNum;
	if (mNum != 0) mNum++;
	sendXmsgInp.sendXmsgInfo.msg = buf;
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
	status = rcSendXmsg (conn, &sendXmsgInp);
	rcDisconnect(conn);
	if (status < 0) {
	  fprintf (stderr, "rsSendXmsg error. status = %d\n", status);
	  exit (9);
	}
      }
    }
    else if (!strcmp(argv[1], "r")) {
      memset (&rcvXmsgInp, 0, sizeof (rcvXmsgInp));
      rcvXmsgInp.rcvTicket = tNum;
      if (argc < 4)
	mNum = 0;
      rcvXmsgInp.msgNumber = mNum;
      if (mNum != 0) mNum++;
      while ( 1 ) {
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
	status = rcRcvXmsg (conn, &rcvXmsgInp, &rcvXmsgOut);
        rcDisconnect(conn);
 	if (status  >= 0) {
	  printf ("%s#%i:: %s", 
		  rcvXmsgOut->msgType, rcvXmsgOut->seqNumber, rcvXmsgOut->msg);
	  if (rcvXmsgOut->msg[strlen(rcvXmsgOut->msg)-1] != '\n')
	    printf("\n");
	  rcvXmsgInp.msgNumber = mNum;
	  if (mNum != 0) mNum++;
	  sNum = 1;
	}
	else {
	  sleep(sNum);
	  sNum = 2 * sNum;
	  if (sNum > 10) sNum = 10;
	}
      }
    }
    else if (!strcmp(argv[1], "t")) {
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
      free (outXmsgTicketInfo);
    }
    else {
      fprintf(stderr,"wrong option. Check with -h\n");
      rcDisconnect(conn);
      exit(9);
    }

    exit (0);
}
