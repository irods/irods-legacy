/**
 * @file	mailMS.c
 *
 */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"

/**
 * \fn msiSendMail(msParam_t* xtoAddr, msParam_t* xsubjectLine, msParam_t* xbody, ruleExecInfo_t *rei)
 *
 * \brief Send email
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  Arcot Rajasekar
 * \date    2008-05
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-19
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note   This microservice sends e-mail using the mail command in the unix system. The first argument is the e-mail address of the receiver. The 
 *         second argument is the subject string and the third argument is the body of the e-mail. No attachments are supported. The sender of the e-mail 
 *         is the unix uxr-id running the irodsServer.
 *
 * \usage
 *
 * As seen in clients/icommands/test/ruleInp5
 *
 * myTestRule||msiSendMail(moore@sdsc.edu,irods test,mail sent by an msi.did you get this)|nop
 *
 * \param[in] xtoAddr - a msParam of type STR_MS_T which is an address of the receiver.
 * \param[in] xsubjectLine - a msParam of type STR_MS_T which is a subject of the message.
 * \param[in] xbody - a msParam of type STR_MS_T which is a body of the message.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none 
 * \sideeffect An e-mail sent to the specified recipient.
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int msiSendMail(msParam_t* xtoAddr, msParam_t* xsubjectLine, msParam_t* xbody, ruleExecInfo_t *rei)
{
  char *mailStr;
  char fName[100];
  char *t1, *t2;
  FILE *fd;
  char *toAddr;
  char *subjectLine;
  char *body;

  toAddr = (char *) xtoAddr->inOutStruct;
  subjectLine = (char *) xsubjectLine->inOutStruct;
  body = (char *) xbody->inOutStruct;


    if (reTestFlag > 0 ) {
      if (reTestFlag == COMMAND_TEST_1) {
	fprintf(stdout,"  Sending Email\n     To:%s\n     Subject:%s\n     Body:%s\n",
		toAddr,subjectLine,body);
      }
      else if (reTestFlag == HTML_TEST_1) {
	fprintf(stdout,"Sending Email\n<UL>\n");
	fprintf(stdout,"<LI>To: %s\n",toAddr);
	fprintf(stdout,"<LI>subjectLine: %s\n",subjectLine);
	fprintf(stdout,"<LI>Body: %s\n",body); 
	fprintf(stdout,"</UL>\n");
      }
      else if (reTestFlag == LOG_TEST_1) 
	  rodsLog (LOG_NOTICE,"   Calling msiSendMail To:%s Subject %s\n",
		   toAddr,subjectLine);
      if (reLoopBackFlag > 0)
	return(0);
  }
    sprintf(fName,"mailFile%d.ml",getpid());
    fd = fopen(fName,"w");
    if (fd == NULL)
      return(FILE_CREATE_ERROR);
    t1 = body;
#ifdef solaris_platform
    if (subjectLine != NULL && strlen(subjectLine) > 0)
      fprintf(fd,"Subject:%s\n\n",subjectLine);
#endif
    while (t1 != NULL) {
      if ((t2 = strstr(t1,"\\n")) != NULL)
	*t2 = '\0';
      fprintf(fd,"%s\n",t1);
      if (t2 != NULL) {
	*t2 = '\\';
	t1 = t2+2;
      }
      else 
	t1 = NULL;
    }
    fclose(fd);
    mailStr = malloc (strlen(toAddr)+100);

#ifdef solaris_platform
    sprintf(mailStr,"cat %s| mail  %s",fName,toAddr);
#else /* tested for linux - not sure how other platforms operate for subject */
    if (subjectLine != NULL && strlen(subjectLine) > 0)
      sprintf(mailStr,"cat %s| mail -s '%s'  %s",fName,subjectLine, toAddr);
    else
      sprintf(mailStr,"cat %s| mail  %s",fName,toAddr);
#endif
    system(mailStr);
    sprintf(mailStr,"rm %s",fName);
    system(mailStr);
    free(mailStr);
    return(0);
}


/**
 * \fn msiSendStdoutAsEmail(msParam_t* xtoAddr, msParam_t* xsubjectLine, ruleExecInfo_t *rei)
 *
 * \brief sends the current buffer content in rei->ruleExecOut->stdoutBuf.buf as email. 
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  Arcot Rajasekar
 * \date    2008-05
 * 
 * \remark Ketan Palshikar - created msi documentation, 2009-06-19
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note   This microservice sends contents of the buffer rei->ruleExecOut->stdoutBuf.buf as email. It is a microservice which, given a toAddr parameter 
 *         (an e-mail address) and a subjectLine parameter, sends out the stdout buffer as the body of the e-mail.
 *
 * \usage
 *
 * As seen in clients/icommands/test/purgeCollAndEmail.ir
 *
 * purgeRule(*Condition,*MailTo)||acPurgeFiles(*Condition)##msiSendStdoutAsEmail(*MailTo,Purge Results)|nop##nop
 * *Condition= COLL_NAME =  '/tempZone/home/rods/loopTest'%*MailTo=sekar@sdsc.edu
 * *Condition%ruleExecOut
 *
 * \param[in] xtoAddr - a msParam of type STR_MS_T which is the address of the receiver.
 * \param[in] xsubjectLine - a msParam of type STR_MS_T which is the subject of the message.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none 
 * \sideeffect none
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa writeLine, writeString
 * \bug  no known bugs
**/
int msiSendStdoutAsEmail(msParam_t* xtoAddr, msParam_t* xsubjectLine, ruleExecInfo_t *rei)
{
  int i;
  msParam_t *mP;
  char tmpVarName[MAX_ACTION_SIZE];
  execCmdOut_t *myExecCmdOut;
  if ((mP = getMsParamByLabel (rei->msParamArray, "ruleExecOut")) == NULL) 
    return(NO_VALUES_FOUND);
  myExecCmdOut = mP->inOutStruct;
  getNewVarName(tmpVarName,rei->msParamArray);
  addMsParam(rei->msParamArray, tmpVarName,  STR_MS_T,myExecCmdOut->stdoutBuf.buf , NULL);
  mP = getMsParamByLabel (rei->msParamArray, tmpVarName);
  i = msiSendMail(xtoAddr, xsubjectLine, mP, rei);
  rmMsParamByLabel (rei->msParamArray, tmpVarName,1);
  return(i);

}
