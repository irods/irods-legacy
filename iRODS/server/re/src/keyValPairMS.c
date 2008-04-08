/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"


int msiGetValByKey(msParam_t* inKVPair,   msParam_t* inKey, msParam_t* outVal,  ruleExecInfo_t *rei)
{
   keyValPair_t *kvp;
   char *s, *k;
   int i;
   
   RE_TEST_MACRO ("msiGetValByKey");

   kvp = inKVPair->inOutStruct;
   k = inKey->inOutStruct;
   if (k == NULL)
     k = inKey->label;
   
   s = getValByKey(kvp,k);
   if (s == NULL)
     return(UNMATCHED_KEY_OR_INDEX);
   i = fillStrInMsParam (outVal,s);
   return(i);
}

int msiPrintKeyValPair(msParam_t* where, msParam_t* inkvpair, ruleExecInfo_t *rei)
{
  int i,l,m,j;
  keyValPair_t *k;
  char *s;
  msParam_t tms;

  RE_TEST_MACRO ("msiPrintKeyValPair");

  m = 0;
  s = NULL;
  k= inkvpair->inOutStruct;

  for (i = 0; i < k->len; i++) {
    l  =  strlen(k->keyWord[i]) + strlen(k->value[i]) + 10;
    if (l > m) {
      if (m > 0)
	free(s);
      s = malloc (l);
      m = l;
    }
    sprintf(s,"%s = %s\n", k->keyWord[i],k->value[i]);
    tms.inOutStruct =  s;
    j = writeString(where, &tms,rei);
    if (j < 0) {
      free(s);
      return(j);
    }
  }
  if (m > 0)
    free(s);
  return(0);
}

int msiString2KeyValPair(msParam_t *inBufferP, msParam_t* outKeyValPairP, ruleExecInfo_t *rei)
{
   keyValPair_t *kvp;
   strArray_t strArray;
   char *buf;
   char *value;
   int i,j;
   char *valPtr;
   char *tmpPtr;

   RE_TEST_MACRO ("msiString2KeyValPair");

   buf = (char *) inBufferP->inOutStruct;
   memset(&strArray,0,sizeof (strArray_t));
   i =  parseMultiStr (buf, &strArray);
   if (i < 0)
     return(i);
   value = strArray.value;

   kvp = mallocAndZero(sizeof(keyValPair_t));
   for (i = 0; i < strArray.len; i++) {
     valPtr = &value[i * strArray.size];
     if ((tmpPtr = strstr (valPtr, "=")) != NULL) {
       *tmpPtr = '\0';
       tmpPtr++;
       j = addKeyVal(kvp,valPtr, tmpPtr);
       if (j < 0) {
	 return(j);
       }
       *tmpPtr = '=';
     }
   }
   outKeyValPairP->inOutStruct = (void *) kvp;
   outKeyValPairP->type = (char *) strdup(KeyValPair_MS_T);
   
   return(0);
}


int msiStrArray2String(msParam_t* inSAParam, msParam_t* outStr, ruleExecInfo_t *rei)
{
  int i;
  strArray_t *strArr;
  char *s;
  char *val;

  RE_TEST_MACRO ("msiStrArray2String");

  strArr = (strArray_t *) inSAParam->inOutStruct;
  val = strArr->value;
  s = (char *) malloc(strArr->len * strArr->size);
  s[0] = '\0';
  
  strcat(s,val);
  for (i = 1; i <strArr->len; i++) {
    strcat(s,"%");
    strcat(s, &val[i * strArr->size]);
  }
  outStr->inOutStruct = (void *) strdup(s);
  outStr->type = (char *) strdup(STR_MS_T);
  free(s);
  return(0);
}
