/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"
#include "rodsXmsg.h"
#include "getXmsgTicket.h"
#include "sendXmsg.h"
#include "rcvXmsg.h"


int msiXmsgServerConnect(msParam_t* outConnParam, ruleExecInfo_t *rei)
{

  rcComm_t *conn;
  rodsEnv myRodsEnv;
  rErrMsg_t errMsg;
  int status;
  
  RE_TEST_MACRO ("    Calling msiXmsgServerConnect");

  status = getRodsEnv (&myRodsEnv);
  if (status < 0) {
    rodsLog (LOG_ERROR, "msiXmsgServerConnect: getRodsEnv failed:%i", status);
    return(status);
  }
  conn = rcConnectXmsg (&myRodsEnv, &errMsg);
    if (conn == NULL) {
    rodsLog (LOG_ERROR,
	     "msiXmsgServerConnect: rcConnectXmsg failed:%i :%s\n", errMsg.status, errMsg.msg);
    return(errMsg.status);
  }
  status = clientLogin(conn);
  if (status != 0) {
    rodsLog (LOG_ERROR, "msiXmsgServerConnect: clientLogin failed:%i", status);
    return(status);
  }

  outConnParam->inOutStruct = (void *) conn;
  outConnParam->type = (char *) strdup("RcComm_MS_T");

  return(0);

}

int msiXmsgCreateStream(msParam_t* inConnParam, 
			msParam_t* inGgetXmsgTicketInpParam, 
			msParam_t* outXmsgTicketInfoParam, 
			ruleExecInfo_t *rei)
{
  
  rcComm_t *conn;
  getXmsgTicketInp_t *getXmsgTicketInp;
  xmsgTicketInfo_t *outXmsgTicketInfo = NULL;
  int status;
  int allocFlag = 0;
  
  RE_TEST_MACRO ("    Calling msiXmsgCreateStream");

  if (inConnParam->inOutStruct == NULL) {
    rodsLog (LOG_ERROR,
	     "msiXmsgCreateStream: input inConnParam is NULL");
    return (SYS_INTERNAL_NULL_INPUT_ERR);
  }
  conn = (rcComm_t *) inConnParam->inOutStruct;

  if (inGgetXmsgTicketInpParam->inOutStruct != NULL)
    getXmsgTicketInp = (getXmsgTicketInp_t *) inGgetXmsgTicketInpParam->inOutStruct;
  else {
    getXmsgTicketInp = (getXmsgTicketInp_t *)  malloc(sizeof (getXmsgTicketInp_t));
    memset (getXmsgTicketInp, 0, sizeof (getXmsgTicketInp_t));
    allocFlag = 1;
  }

  status = rcGetXmsgTicket (conn, getXmsgTicketInp, &outXmsgTicketInfo);
  if (status != 0) {
    rodsLog (LOG_ERROR, "msiXmsgCreateStream: rcGetXmsgTicket failed:%i", status);
    return(status);
  }

  outXmsgTicketInfoParam->inOutStruct = (void *) outXmsgTicketInfo;
  outXmsgTicketInfoParam->type = (char *) strdup(XmsgTicketInfo_MS_T);
  if (allocFlag == 1)
    free(getXmsgTicketInp);
  return(0);

}

int msiCreateXmsgInp(msParam_t* inMsgNumber,
		      msParam_t* inMsgType,
		      msParam_t* inNumberOfReceivers,
		      msParam_t* inMsg,
		      msParam_t* inNumberOfDeliverySites,
		      msParam_t* inDeliveryAddressList,
		      msParam_t* inDeliveryPortList,
		      msParam_t* inMiscInfo,
		      msParam_t* inXmsgTicketInfoParam,
		      msParam_t* outSendXmsgInpParam,
		      ruleExecInfo_t *rei)
{


  sendXmsgInp_t  *sendXmsgInp;
  xmsgTicketInfo_t *xmsgTicketInfo;

  if (inXmsgTicketInfoParam == NULL) {
    rodsLog (LOG_ERROR, "msiSendXmsg: input inXmsgTicketInfoParam is NULL");
    return (SYS_INTERNAL_NULL_INPUT_ERR);
  }
  xmsgTicketInfo = (xmsgTicketInfo_t *) inXmsgTicketInfoParam->inOutStruct;

  sendXmsgInp =  (sendXmsgInp_t  *) malloc(sizeof (sendXmsgInp_t));
  /*  memset (&sendXmsgInp, 0, sizeof (sendXmsgInp_t));*/

  sendXmsgInp->ticket.sendTicket =  xmsgTicketInfo->sendTicket;
  sendXmsgInp->ticket.rcvTicket =  xmsgTicketInfo->rcvTicket;
  sendXmsgInp->ticket.expireTime =  xmsgTicketInfo->expireTime;
  sendXmsgInp->ticket.flag = xmsgTicketInfo->flag;
  if (!strcmp(inMsgNumber->type,STR_MS_T))
    sendXmsgInp->sendXmsgInfo.msgNumber = (uint) atoi(inMsgNumber->inOutStruct);
  else
    sendXmsgInp->sendXmsgInfo.msgNumber = (uint) inMsgNumber->inOutStruct;
  strcpy(sendXmsgInp->sendXmsgInfo.msgType, inMsgType->inOutStruct);
  if (!strcmp(inNumberOfReceivers->type,STR_MS_T))
    sendXmsgInp->sendXmsgInfo.numRcv = (uint) atoi(inNumberOfReceivers->inOutStruct);
  else
    sendXmsgInp->sendXmsgInfo.numRcv = (uint) inNumberOfReceivers->inOutStruct;
  sendXmsgInp->sendXmsgInfo.msg = (char *) inMsg->inOutStruct;
  if (!strcmp(inNumberOfDeliverySites->type,STR_MS_T))
    sendXmsgInp->sendXmsgInfo.numDeli = (int) atoi(inNumberOfDeliverySites->inOutStruct);
  else
    sendXmsgInp->sendXmsgInfo.numDeli = (int) inNumberOfDeliverySites->inOutStruct;
  if (sendXmsgInp->sendXmsgInfo.numDeli == 0) {
    sendXmsgInp->sendXmsgInfo.deliAddress = NULL;
    sendXmsgInp->sendXmsgInfo.deliPort = NULL;
  }
  else {
    sendXmsgInp->sendXmsgInfo.deliAddress = inDeliveryAddressList->inOutStruct;
    sendXmsgInp->sendXmsgInfo.deliPort = inDeliveryPortList->inOutStruct;
  }
  sendXmsgInp->sendXmsgInfo.miscInfo = (char *) inMiscInfo->inOutStruct;

  outSendXmsgInpParam->inOutStruct = (void *) sendXmsgInp;
  outSendXmsgInpParam->type = (char *) strdup(SendXmsgInp_MS_T);
  return(0);

  
}
int msiSendXmsg(msParam_t* inConnParam, 
		msParam_t* inSendXmsgInpParam,
		ruleExecInfo_t *rei)
{
  
  rcComm_t *conn;
  sendXmsgInp_t  *sendXmsgInp;
  int status;



  RE_TEST_MACRO ("    Calling msiSendXmsg");

  if (inConnParam->inOutStruct == NULL) {
    rodsLog (LOG_ERROR, "msiSendXmsg: input inConnParam is NULL");
    return (SYS_INTERNAL_NULL_INPUT_ERR);
  }
  conn = (rcComm_t *) inConnParam->inOutStruct;

  if (inSendXmsgInpParam == NULL) {
    rodsLog (LOG_ERROR, "msiSendXmsg: input inSendXmsgInpParam is NULL");
    return (SYS_INTERNAL_NULL_INPUT_ERR);
  }
  sendXmsgInp = (sendXmsgInp_t *) inSendXmsgInpParam->inOutStruct;

  
  status = rcSendXmsg (conn, sendXmsgInp);
  if (status < 0) {
    rodsLog (LOG_ERROR, "msiSendXmsg: rcSendXmsg failed:%i", status);
    return(status);
  }
  return(status);
}

  
int msiRcvXmsg(msParam_t* inConnParam, 
	       msParam_t* inTicketNumber,
	       msParam_t* inMsgNumber,
	       msParam_t* outMsgType,
	       msParam_t* outMsg,
	       msParam_t* outSendUser,
	       ruleExecInfo_t *rei)
{
  
  rcComm_t *conn;
  rcvXmsgInp_t rcvXmsgInp;
  rcvXmsgOut_t *rcvXmsgOut = NULL;
  xmsgTicketInfo_t *xmsgTicketInfo = NULL;
  int status;


  RE_TEST_MACRO ("    Calling msiRcvXmsg");

  if (inConnParam->inOutStruct == NULL) {
    rodsLog (LOG_ERROR, "msiRcvXmsg: input inConnParam is NULL");
    return (SYS_INTERNAL_NULL_INPUT_ERR);
  }
  conn = (rcComm_t *) inConnParam->inOutStruct;

  memset (&rcvXmsgInp, 0, sizeof (rcvXmsgInp));
  if (!strcmp(inTicketNumber->type,XmsgTicketInfo_MS_T)) {
    xmsgTicketInfo = (xmsgTicketInfo_t *) inTicketNumber->inOutStruct;
    rcvXmsgInp.rcvTicket = xmsgTicketInfo->rcvTicket;
  }
  else if (!strcmp(inTicketNumber->type,STR_MS_T)) {
    rcvXmsgInp.rcvTicket = (uint) atoi(inTicketNumber->inOutStruct);
  }
  else
    rcvXmsgInp.rcvTicket = (uint) inTicketNumber->inOutStruct;
  if (!strcmp(inMsgNumber->type,STR_MS_T)) 
    rcvXmsgInp.msgNumber = (uint) atoi(inMsgNumber->inOutStruct);
  else
    rcvXmsgInp.msgNumber = (uint) inMsgNumber->inOutStruct;
  
  status = rcRcvXmsg (conn, &rcvXmsgInp, &rcvXmsgOut);
  if (status < 0) {
    rodsLog (LOG_ERROR, "msiRcvXmsg: rcRcvXmsg failed:%i", status);
    return(status);
  }

  outMsgType->inOutStruct = (void *) strdup(rcvXmsgOut->msgType);
  outMsgType->type = strdup(STR_MS_T);
  outMsg->inOutStruct = (void *) rcvXmsgOut->msg;
  outMsg->type = strdup(STR_MS_T);
  outSendUser->inOutStruct = (void *) strdup(rcvXmsgOut->sendUserName);
  outSendUser->type = strdup(STR_MS_T);
  return(status);
}
  
int msiXmsgServerDisConnect(msParam_t* inConnParam, ruleExecInfo_t *rei)
{

  rcComm_t *conn;
  int status;
  
  RE_TEST_MACRO ("    Calling msiXmsgServerDisConnect");

  if (inConnParam->inOutStruct == NULL) {
    rodsLog (LOG_ERROR, "msiSendXmsg: input inConnParam is NULL");
    return (SYS_INTERNAL_NULL_INPUT_ERR);
  }
  conn = (rcComm_t *) inConnParam->inOutStruct;
  status = rcDisconnect(conn);
  if (status == 0)
    inConnParam->inOutStruct = NULL;
  return(status);

}
