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



int writeKeyValPairs(msParam_t *where, msParam_t *inKVPair, msParam_t *separator, ruleExecInfo_t *rei)
{
	keyValPair_t *KVPairs;
	char *writeId;
	char *writeStr;
	char *sepStr;
	int i;
	size_t size;


	RE_TEST_MACRO ("    Calling writeKeyValPairs")

	
	/* sanity checks */
	if (!rei ) {
		rodsLog (LOG_ERROR, "writeKeyValPairs: input rei is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}
	
	if (!where) {
		rodsLog (LOG_ERROR, "writeKeyValPairs: No destination provided for writing.");
		return (USER__NULL_INPUT_ERR);
	}

	
	/* check for proper input type and get keyValPair input */
	if (strcmp(inKVPair->type, KeyValPair_MS_T)) {
		rodsLog (LOG_ERROR, "writeKeyValPairs: input parameter is not of KeyValPair_MS_T type.");
		return(USER_PARAM_TYPE_ERR);
	}
	KVPairs = (keyValPair_t *)inKVPair->inOutStruct;


	/* where are we writing to? */
	if (where->inOutStruct != NULL) {
		writeId = where->inOutStruct;
	}
	else {
		writeId = where->label;
	}


	/* get separator string or use default */
	if ((sepStr = parseMspForStr(separator)) == NULL)  {
		sepStr = "\t|\t";
	}


	/* find out how much memory is needed for writeStr */
	size = 0;
	for (i=0; i < KVPairs->len; i++) {
		size += strlen(KVPairs->keyWord[i]) + strlen(sepStr) + strlen(KVPairs->value[i]) + strlen("\n");
	}

	/* allocate memory for writeStr and pad with null chars */
	writeStr = (char *)malloc(size + MAX_COND_LEN);
	memset(writeStr, '\0', size + MAX_COND_LEN);


	/* print each key-value pair to writeStr */
	for (i=0; i < KVPairs->len; i++)  {
		strcat(writeStr, KVPairs->keyWord[i]);
		strcat(writeStr, sepStr);
		strcat(writeStr, KVPairs->value[i]);
		strcat(writeStr, "\n");
	}


	/* call _writeString() routine */
	rei->status = _writeString(writeId, writeStr, rei);

	
	/* free writeStr since its content has been copied somewhere else */
	if (writeStr != NULL) {
		free(writeStr);
	}

	return (rei->status);
}





