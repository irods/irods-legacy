/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"

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
