/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"


int msiGetValByKey(msParam_t* inKVPair,   msParam_t* inKey, msParam_t* outVal,  ruleExecInfo_t *rei)
{
   keyValPair_t *kvp;
   char *s, *k;
   int i;

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
