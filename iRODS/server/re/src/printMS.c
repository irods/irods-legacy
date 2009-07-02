/**
 * @file	printMS.c
 *
 */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"

/**
 * \fn writeLine(msParam_t* where, msParam_t* inString, ruleExecInfo_t *rei)
 *
 * \brief  This microservice writes a given string followed by a new-line character into the target buffer in ruleExecOut Parameter.
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  Arcot Rajasekar
 * \date    2008
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-24
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note  
 *
 * \usage
 *
 * As seen in clients/icommands/test/ruleInp5
 *
 * myTestRule||writeLine(stdout,alpha beta gamma)##writeLine(stdout,alpha beta gamma)##writeLine(stderr,Error:blah)|nop
 *
 * Also:
 *
 * myTestRule||assign(*A,0)##whileExec(*A < 20, writeLine(stdout, *A)##assign(*A, *A + 4), nop)|nop##nop
 * null
 * ruleExecOut
 *
 * \param[in] where - a msParam of type STR_MS_T which is the buffer name in ruleExecOut. Currently stdout and stderr.
 * \param[in] inString - a msParam of type STR_MS_T which is a string to be written into buffer.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect ruleExecOut structure in msParamArray gets modified
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
int writeLine(msParam_t* where, msParam_t* inString, ruleExecInfo_t *rei)
{
  int i;
  char tmp[3];
  char *ptr;
  char *writeId = (char *) where->inOutStruct;

  if (writeId != NULL && strcmp (writeId, "serverLog") == 0 &&
   inString->inOutStruct != NULL) {
    rodsLog (LOG_NOTICE, "writeLine: inString = %s\n", inString->inOutStruct);
    return 0;
  }

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

/**
 * \fn writeString(msParam_t* where, msParam_t* inString, ruleExecInfo_t *rei)
 *
 * \brief  This microservice writes a given string into the target buffer in ruleExecOut parameter
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  Arcot Rajasekar
 * \date    2008
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-24
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note  
 *
 * \usage
 *
 * As seen in clients/icommands/test/ruleInp5
 *
 * myTestRule||writeString(stdout,alpha beta gamma)##writeString(stdout,alpha beta gamma)##writeString(stderr,Error:blah)|nop
 *
 * \param[in] where - where is a msParam of type STR_MS_T which is the buffer name in ruleExecOut. Currently stdout and stderr.
 * \param[in] inString - inString is a msParam of type STR_MS_T which is a string to be written into buffer
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect ruleExecOut structure in msParamArray gets modified.
 *
 * \return integer
 * \retval
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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
    
  if (writeId != NULL && strcmp (writeId, "serverLog") == 0) {
    rodsLog (LOG_NOTICE, "writeString: inString = %s", writeStr);
    return 0;
  }
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


/**
 * \fn writePosInt(msParam_t* where, msParam_t* inInt, ruleExecInfo_t *rei)
 *
 * \brief  This microservice writes a positive integer into a buffer.
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  
 * \date
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-24
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note  
 *
 * \usage
 *
 * As seen in modules/integrityChecks/test/testwriteposint.ir
 *
 * msiTestWritePosInt||msiTestWritePosInt(*A)##writePosInt(stdout, *A)##writeLine(stdout,"xxx")|nop
 * null
 * ruleExecOut
 *
 * \param[in] where - a msParam of type STR_MS_T which is the buffer name in ruleExecOut.
 * \param[in] inInt - the integer to write
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence 
 * \DolVarModified 
 * \iCatAttrDependence 
 * \iCatAttrModified 
 * \sideeffect 
 *
 * \return integer
 * \retval
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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


/**
 * \fn writeBytesBuf(msParam_t* where, msParam_t* inBuf, ruleExecInfo_t *rei)
 *
 * \brief  This microservice writes the buffer in an inOutStruct to stdout or stderr.
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  
 * \date 
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-24
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note  
 *
 * \usage
 *
 * As seen in modules/integrityChecks/test/listACL.ir
 *
 * msiListCollACL||msiListCollACL(*Collection,*BUF,*STATUS)##writeBytesBuf(stdout,*BUF)##writeLine(stdout,"")|nop
 * *Collection=/homeZone/home/rods/bigcollection
 * ruleExecOut%*STATUS
 *
 * \param[in] where - a msParam of type STR_MS_T which is the buffer name in ruleExecOut. It can be stdout or stderr.
 * \param[in] inBuf - a msParam of type STR_MS_T - related to the status output
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence 
 * \DolVarModified 
 * \iCatAttrDependence 
 * \iCatAttrModified 
 * \sideeffect 
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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

/**
 * \fn writeKeyValPairs(msParam_t *where, msParam_t *inKVPair, msParam_t *separator, ruleExecInfo_t *rei)
 *
 * \brief  This microservice writes keyword value pairs to stdout or stderr, using the given separator.
 * 
 * \module core
 * 
 * \since pre-2.1
 * 
 * \author  
 * \date 
 * 
 * \remark Ketan Palshikar - msi documentation, 2009-06-24
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-30
 * 
 * \note  
 *
 * \usage
 *
 * As seen in modules/integrityChecks/test/verifyDataType.ir
 *
 * msiCheckFileDatatypes||msiCheckFileDatatypes(*Collection,*Datatype,*Status)##writeKeyValPairs(stdout, *B, ": ")##writeLine(stdout,"")|nop
 * *A=/homeZone/home/rods%*B=generic,xml,kitten,mmCIF, ,
 * ruleExecOut%*STATUS
 *
 * \param[in] where - a msParam of type STR_MS_T which is the buffer name in ruleExecOut. It can be stdout or stderr.
 * \param[in] inKVPair - a msParam of type KeyValPair_MS_T
 * \param[in] separator - Optional - a msParam of type STR_MS_T, the desired parameter
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence 
 * \DolVarModified 
 * \iCatAttrDependence 
 * \iCatAttrModified 
 * \sideeffect 
 *
 * \return integer
 * \retval 0 on success
 * \pre
 * \post
 * \sa
 * \bug  no known bugs
**/
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

