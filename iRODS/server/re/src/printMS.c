/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"


int writeLine(msParam_t* where, msParam_t* inString, ruleExecInfo_t *rei)
{
  int i;
  char tmp[3];
  char *ptr;
  i = writeString(where, inString,rei);
  if (i < 0)
    return(i);
  ptr = inString->inOutStruct;
  sprintf(tmp,"%s\n","");
  inString->inOutStruct =  tmp;
  i = writeString(where, inString,rei);
  inString->inOutStruct = ptr;
  return(i);
  
}
int writeString(msParam_t* where, msParam_t* inString, ruleExecInfo_t *rei)
{
  int i;
  char *writeId;
  char *writeStr;

  if (where->inOutStruct == NULL)
    writeId = where->label;
  else
    writeId = where->inOutStruct;

  if (inString->inOutStruct == NULL) {
    writeStr = (char *) malloc(strlen(inString->label) + MAX_COND_LEN);
    strcpy(writeStr , inString->label);
  }
  else {
    writeStr = (char *) malloc(strlen(inString->inOutStruct) + MAX_COND_LEN);
    strcpy(writeStr , inString->inOutStruct);
  }
  i = _writeString(writeId, writeStr, rei);

  free(writeStr);
  return(i);
}

int _writeString(char *writeId, char *writeStr, ruleExecInfo_t *rei)
{
  msParamArray_t *inMsParamArray;
  msParam_t *mP;
  execCmdOut_t *myExecCmdOut;
    
  mP = NULL;
  inMsParamArray = rei->msParamArray;
  if (((mP = getMsParamByLabel (inMsParamArray, "ruleExecOut")) != NULL) &&
      (mP->inOutStruct != NULL)) {
    if (!strcmp(mP->type,STR_MS_T)) {
      myExecCmdOut =  malloc (sizeof (execCmdOut_t));
      memset (myExecCmdOut, 0, sizeof (execCmdOut_t));
      mP->inOutStruct = myExecCmdOut;
      mP->type = strdup(ExecCmdOut_MS_T);
    }
    else
      myExecCmdOut = mP->inOutStruct;
  }
  else {
    myExecCmdOut =  malloc (sizeof (execCmdOut_t));
    memset (myExecCmdOut, 0, sizeof (execCmdOut_t));
    if (mP == NULL)
      addMsParam(inMsParamArray,"ruleExecOut", ExecCmdOut_MS_T,myExecCmdOut,NULL);
    else {
      mP->inOutStruct = myExecCmdOut;
      mP->type = strdup(ExecCmdOut_MS_T);
    }
  }

  /***** Jun 27, 2007
  i  = replaceVariablesAndMsParams("",writeStr, rei->msParamArray, rei);
  if (i < 0)
    return(i);
  ****/

  if (!strcmp(writeId,"stdout")) 
    appendToByteBuf(&(myExecCmdOut->stdoutBuf),(char *) writeStr);
  else if (!strcmp(writeId,"stderr")) 
    appendToByteBuf(&(myExecCmdOut->stderrBuf),(char *) writeStr);



  return(0);
}


int writePosInt(msParam_t* where, msParam_t* inInt, ruleExecInfo_t *rei)
{
	char *writeId;
	char writeStr[LONG_NAME_LEN];
	int status;

	if (where->inOutStruct != NULL) {
		writeId = where->inOutStruct;	
	}
	else {
		writeId = where->label;
	}

	if (inInt->inOutStruct != NULL) {
		sprintf(writeStr, "%d", parseMspForPosInt (inInt));
	}
	else {
		snprintf(writeStr, LONG_NAME_LEN, "%s", inInt->label);
	}

	status = _writeString(writeId, writeStr, rei);

	return (status);
}


int writeBytesBuf(msParam_t* where, msParam_t* inBuf, ruleExecInfo_t *rei)
{
	char *writeId;
	char *writeStr;
	int status;
	
	if (where->inOutStruct != NULL) {
		writeId = where->inOutStruct;	
	}
	else {
		writeId = where->label;
	}
	
	if (inBuf->inpOutBuf != NULL) {
		writeStr = (char *) malloc(strlen(inBuf->inpOutBuf->buf) + MAX_COND_LEN);
		strcpy(writeStr , inBuf->inpOutBuf->buf);
	}
	else {
		writeStr = (char *) malloc(strlen(inBuf->label) + MAX_COND_LEN);
		strcpy(writeStr , inBuf->label);
	}

	status = _writeString(writeId, writeStr, rei);
	
	if (writeStr != NULL) {
		free(writeStr);
	}

	return (status);
}





