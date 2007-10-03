/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "genQuery.h"
#include "reHelpers1.h"

int _makeQuery( char *sel, char *cond, char **sql);

int msiExecStrCondQuery(msParam_t* queryParam, msParam_t* genQueryOutParam, ruleExecInfo_t *rei)
{
  genQueryInp_t genQueryInp;
  int i;
  genQueryOut_t *genQueryOut = NULL;
  char *query;

  query = (char *) malloc(strlen(queryParam->inOutStruct) + 10 + MAX_COND_LEN * 8);
  
  strcpy(query, queryParam->inOutStruct);

  /**** Jun 27, 2007
  if (queryParam->inOutStruct == NULL) {
    query = (char *) malloc(strlen(queryParam->label) + MAX_COND_LEN);
    strcpy(query , queryParam->label);
  }
  else {
    query = (char *) malloc(strlen(queryParam->inOutStruct) + MAX_COND_LEN);
    strcpy(query , queryParam->inOutStruct);
  }
  i  = replaceVariablesAndMsParams("",query, rei->msParamArray, rei);
  if (i < 0)
    return(i);
  ***/
  i  = replaceVariablesAndMsParams("",query, rei->msParamArray, rei);
  if (i < 0)
    return(i);
  memset (&genQueryInp, 0, sizeof (genQueryInp_t));
  i = fillGenQueryInpFromStrCond(query, &genQueryInp);
  if (i < 0)
    return(i);
  genQueryInp.maxRows= MAX_SQL_ROWS;
  genQueryInp.continueInx=0;

  i = rsGenQuery(rei->rsComm, &genQueryInp, &genQueryOut);
  if (i < 0)
    return(i);
  genQueryOutParam->type = strdup(GenQueryOut_MS_T);
  genQueryOutParam->inOutStruct = genQueryOut;
  return(0);
}

int msiExecGenQuery(msParam_t* genQueryInParam, msParam_t* genQueryOutParam, ruleExecInfo_t *rei)
{
  genQueryInp_t *genQueryInp;
  int i;
  genQueryOut_t *genQueryOut = NULL;


  genQueryInp = genQueryInParam->inOutStruct;

  i = rsGenQuery(rei->rsComm, genQueryInp, &genQueryOut);
  if (i < 0)
    return(i);
  genQueryOutParam->type = strdup(GenQueryOut_MS_T);
  genQueryOutParam->inOutStruct = genQueryOut;
  return(0);
}

int
_makeQuery( char *sel, char *cond, char **sql)
{
  *sql = (char *) malloc(strlen(sel) + strlen(cond) + 20);
  if (strlen(cond) >  0) 
    sprintf(*sql, "SELECT %s WHERE %s", sel, cond);
  else
    sprintf(*sql, "SELECT %s ", sel);
  return(0);
}

int
msiMakeQuery(msParam_t* selectListParam, msParam_t* conditionsParam, 
	     msParam_t* queryOutParam, ruleExecInfo_t *rei)
{
  char *sql, *sel, *cond;
  int i;
  sel = (char *) selectListParam->inOutStruct;
  cond = (char *) conditionsParam->inOutStruct;
  i = _makeQuery(sel,cond,&sql);
  queryOutParam->type = strdup(STR_MS_T);
  queryOutParam->inOutStruct = sql;
  return(i);
}
