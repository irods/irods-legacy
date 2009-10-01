/**
 * @file	genQueryMS.c
 *
 */



/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "genQuery.h"
#include "reHelpers1.h"

int _makeQuery( char *sel, char *cond, char **sql);

/**
 * \fn msiExecStrCondQueryWithOptions(msParam_t* queryParam,
 *        msParam_t* zeroResultsIsOK,
 *        msParam_t* maxReturnedRowsParam, 
 *        msParam_t* genQueryOutParam, 
 *        ruleExecInfo_t *rei)
 *
 * \brief   This function takes a given condition string and options, creates an iCAT query, executes it and returns the values
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  Romain Guinot
 * \date    
 *
 * \remark Terrell Russell - msi documentation, 2009-09-10
 *
 * \note 
 *
 * \usage None
 *
 * \param[in] queryParam - a msParam of type GenQueryInp_MS_T
 * \param[in] zeroResultsIsOK - Optional - a msParam of type STR_MS_T - must equal "zeroOK"
 * \param[in] maxReturnedRowsParam - Optional - a msParam of type STR_MS_T - as integer
 * \param[out] genQueryOutParam - a msParam of type GenQueryOut_MS_T
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
int msiExecStrCondQueryWithOptions(msParam_t* queryParam,
				   msParam_t* zeroResultsIsOK,
				   msParam_t* maxReturnedRowsParam, 
				   msParam_t* genQueryOutParam, 
				   ruleExecInfo_t *rei)
{
    genQueryInp_t genQueryInp;
    int i;
    genQueryOut_t *genQueryOut = NULL;
    char *query;
    char *maxReturnedRowsStr;
    int maxReturnedRows;

    query = (char *) malloc(strlen(queryParam->inOutStruct) + 10 + MAX_COND_LEN * 8);
    strcpy(query, queryParam->inOutStruct);

    i  = replaceVariablesAndMsParams("",query, rei->msParamArray, rei);
    if (i < 0)
      return(i);
    memset (&genQueryInp, 0, sizeof (genQueryInp_t));
    i = fillGenQueryInpFromStrCond(query, &genQueryInp);
    if (i < 0)
      return(i);
    
    if(maxReturnedRowsParam != NULL)
      {
	maxReturnedRowsStr = (char *) maxReturnedRowsParam->inOutStruct;
	maxReturnedRows = atoi (maxReturnedRowsStr);
	genQueryInp.maxRows= maxReturnedRows;
      }
    else
      genQueryInp.maxRows= MAX_SQL_ROWS;
    
    genQueryInp.continueInx=0;
    
    i = rsGenQuery(rei->rsComm, &genQueryInp, &genQueryOut);
    //if (i < 0)
    if (zeroResultsIsOK !=NULL && strcmp(zeroResultsIsOK->inOutStruct, "zeroOK") == 0 )
      {
	if (i < 0 && i != CAT_NO_ROWS_FOUND)
	  return(i);
	else if (i == CAT_NO_ROWS_FOUND)
	  {
	    genQueryOutParam->type = strdup(STR_MS_T);
	    fillStrInMsParam (genQueryOutParam,"emptySet");
	    return(0);
	  }
	
      }
    else
      {
	if (i < 0)
          return(i);
      }
    
    genQueryOutParam->type = strdup(GenQueryOut_MS_T);
    genQueryOutParam->inOutStruct = genQueryOut;
    return(0);
}


/**
 * \fn msiExecStrCondQuery(msParam_t* queryParam, msParam_t* genQueryOutParam, ruleExecInfo_t *rei)
 *
 * \brief   This function takes a given condition string, creates an iCAT query, executes it and returns the values
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  
 * \date    2008
 *
 * \remark Terrell Russell - msi documentation, 2009-09-10
 *
 * \note 
 *
 * \usage
 * As seen in clients/icommands/test/ruleTest15.ir
 *
 * myTestRule||msiExecStrCondQuery(*A 'foo%' ,*B)##forEachExec(*B,msiPrintKeyValPair(stdout,*B)##writeLine(stdout,*K),nop)|nop
 * *A=SELECT DATA_NAME , DATA_REPL_NUM, DATA_CHECKSUM WHERE DATA_NAME LIKE  %*K=--------HAHAHAHAH-------------
 * *Action%*Condition%*A%*B%*C%*D%*E%ruleExecOut%*K
 *
 * \param[in] queryParam - a msParam of type GenQueryInp_MS_T
 * \param[out] genQueryOutParam - a msParam of type GenQueryOut_MS_T
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

/**
 * \fn msiExecGenQuery(msParam_t* genQueryInParam, msParam_t* genQueryOutParam, ruleExecInfo_t *rei)
 *
 * \brief   This function executes a given general query structure and returns results
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  
 * \date   2008
 *
 * \remark Jewel Ward - msi documentation, 2009-06-10
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-25
 *
 * \note Takes a SQL-like iRODS query (no FROM clause) and returns a table structure. Use #msiGetMoreRows to get all rows.
 *
 * \usage
 * 
 *  As seen in clients/icommands/test/queryContinueExple.ir
 * 
 * continue test||assign(*ContInx, 1)##msiMakeGenQuery("DATA_NAME, COLL_NAME",*Condition,*GenQInp)##msiExecGenQuery(*GenQInp, *GenQOut)##whileExec(*ContInx > 0,msiGetMoreRows(*GenQInp,*GenQOut,*ContInx)##forEachExec(*GenQOut,msiGetValByKey(*GenQOut, "COLL_NAME",*DataObj)##writeString(stdout,*DataObj)##writeString(stdout,"/")##msiGetValByKey(*GenQOut, "DATA_NAME",*DataObj)##writeString(stdout,*DataObj)##writeLine(stdout,""), nop), nop)|nop
 *
 * \param[in] genQueryInParam - a msParam of type GenQueryInp_MS_T
 * \param[out] genQueryOutParam - a msParam of type GenQueryOut_MS_T
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
 * \sa msiGetMoreRows and msiExecStrCondQuery
 * \bug  no known bugs
**/
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



/**
 * \fn msiGetContInxFromGenQueryOut(msParam_t *genQueryOutParam, msParam_t *continueInx, ruleExecInfo_t *rei)
 *
 * \brief This microservice  gets conitnue index value from genQueryOut which is generated by msiExecGenQuery
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  Arcot Rajasekar
 * \date    2009-10
 *
 * \remark none
 *
 * \note This microservice  gets conitnue index value from genQueryOut which is generated by msiExecGenQuery
 *
 * \usage None
 *
 * \param[in] genQueryOut_msp - Required - of type GenQueryOut_MS_T.
 * \param[out] continueInx - a INT_MS_T containing the new continuation index (after the query).
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


int
msiGetContInxFromGenQueryOut(msParam_t* genQueryOutParam, msParam_t* continueInx, ruleExecInfo_t *rei)
{
  genQueryOut_t *genQueryOut;

  RE_TEST_MACRO ("    Calling msiGetContInxFromGenQueryOut")
    /* check for non null parameters */
    if ( !genQueryOutParam)
      {
	rodsLog (LOG_ERROR, "msiGetContInxFromGenQueryOut: Missing parameter(s)");
	return (USER__NULL_INPUT_ERR);
      }
  /* check for proper input types */
  if (strcmp(genQueryOutParam->type, GenQueryOut_MS_T))
    {
      rodsLog (LOG_ERROR, 
	       "msiGetContInxFromGenQueryOut: genQueryOutParam type is %s, should be GenQueryOut_MS_T", 
	       genQueryOutParam->type);
      return (USER_PARAM_TYPE_ERR);
    }
  
  genQueryOut = genQueryOutParam->inOutStruct;
  fillIntInMsParam(continueInx, genQueryOut->continueInx);
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

/**
 * \fn msiMakeQuery(msParam_t* selectListParam, msParam_t* conditionsParam, msParam_t* queryOutParam, ruleExecInfo_t *rei)
 * 
 * \brief Creates sql query from parameter list and conditions.
 *
 * \module core
 *
 * \since 
 *
 * \author  
 * \date    
 *
 * \remark Terrell Russell - msi documentation, 2009-06-23
 *
 * \note
 *
 * \usage None
 *
 * \param[in] selectListParam - a STR_MS_T containing the parameters.
 * \param[in] conditionsParam - a STR_MS_T containing the conditions.
 * \param[out] queryOutParam - a STR_MS_T containing the parameters and conditions as sql.
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



/**
 * \fn msiGetMoreRows(msParam_t *genQueryInp_msp, msParam_t *genQueryOut_msp, msParam_t *continueInx, ruleExecInfo_t *rei)
 *
 * \brief This microservice continues an unfinished query.
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  Antoine de Torcy
 * \date    2008-09-18
 *
 * \remark Terrell Russell - msi documentation, 2009-06-17
 *
 * \note This microservice gets the next batch of rows for an open iCAT query. Likely to follow msiMakeGenQuery and msiExecGenQuery.
 *
 * \usage None
 *
 * \param[in] genQueryInp_msp - Required - a GenQueryInp_MS_T containing the query parameters and conditions.
 * \param[in] genQueryOut_msp - Required - a GenQueryOut_MS_T to write results to. If its continuation index is 0 the query will be closed.
 * \param[out] continueInx - a INT_MS_T containing the new continuation index (after the query).
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
int
msiGetMoreRows(msParam_t *genQueryInp_msp, msParam_t *genQueryOut_msp, msParam_t *continueInx, ruleExecInfo_t *rei)
{
	genQueryInp_t *genQueryInp;
	genQueryOut_t *genQueryOut;


	RE_TEST_MACRO ("    Calling msiGetMoreRows")
	
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiGetMoreRows: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}


	/* check for non null parameters */
	if (!genQueryInp_msp || !genQueryOut_msp)
	{
		rodsLog (LOG_ERROR, "msiGetMoreRows: Missing parameter(s)");
		return (USER__NULL_INPUT_ERR);
	}
	

	/* check for proper input types */
	if (strcmp(genQueryOut_msp->type, GenQueryOut_MS_T))
	{
		rodsLog (LOG_ERROR, "msiGetMoreRows: genQueryOut_msp type is %s, should be GenQueryOut_MS_T", genQueryOut_msp->type);
		return (USER_PARAM_TYPE_ERR);
	}

	if (strcmp(genQueryInp_msp->type, GenQueryInp_MS_T))
	{
		rodsLog (LOG_ERROR, "msiGetMoreRows: query_msp type is %s, should be GenQueryInp_MS_T", genQueryInp_msp->type);
		return (USER_PARAM_TYPE_ERR);
	}


	/* retrieve genQueryXXX data structures */
	genQueryOut = genQueryOut_msp->inOutStruct;
	genQueryInp = genQueryInp_msp->inOutStruct;


	/* match continuation indexes */
	genQueryInp->continueInx = genQueryOut->continueInx;

	if (genQueryInp->continueInx > 0)
	{
		/* get the next batch */
		genQueryInp->maxRows = MAX_SQL_ROWS;
	}
	else
	{
		/* close query */
		genQueryInp->maxRows = -1;
	}


	/* free memory allocated for previous results */
	freeGenQueryOut (&genQueryOut);


	/* query */
	rei->status = rsGenQuery(rei->rsComm, genQueryInp, &genQueryOut);


	if (rei->status == 0)
	{
		/* return query results */
		genQueryOut_msp->inOutStruct = genQueryOut;

		/* return continuation index separately in case it is needed in conditional expressions */
		resetMsParam(continueInx);
		fillIntInMsParam(continueInx, genQueryOut->continueInx);
	}

	return (rei->status);
}



/**
 * \fn msiMakeGenQuery(msParam_t* selectListStr, msParam_t* condStr, msParam_t* genQueryInpParam, ruleExecInfo_t *rei)
 *
 * \brief This microservice sets up a GenQueryInp_MS_T from a list of parameters and conditions
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  Antoine de Torcy
 * \date    2008-09-19
 *
 * \remark Terrell Russell - msi documentation, 2009-06-17
 *
 * \note This microservice sets up a genQueryInp_t data structure needed by calls to rsGenQuery().
 *    To be used before msiExecGenQuery and msiGetMoreRows.
 *
 * \usage None
 *
 * \param[in] selectListStr - Required - a STR_MS_T containing the parameters.
 * \param[in] condStr - Required - a STR_MS_T containing the conditions 
 * \param[out] genQueryInpParam - a GenQueryInp_MS_T containing the parameters and conditions.
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
int
msiMakeGenQuery(msParam_t* selectListStr, msParam_t* condStr, msParam_t* genQueryInpParam, ruleExecInfo_t *rei)
{
	genQueryInp_t *genQueryInp;
	char *sel, *cond, *rawQuery, *query;


	RE_TEST_MACRO ("    Calling msiMakeGenQuery")
	
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiMakeGenQuery: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}


	/* parse selectListStr */
	if ((sel = parseMspForStr(selectListStr)) == NULL)
	{
		rodsLog (LOG_ERROR, "msiMakeGenQuery: input selectListStr is NULL.");
		return (USER__NULL_INPUT_ERR);
	}


	/* parse condStr */
	if ((cond = parseMspForStr(condStr)) == NULL)
	{
		rodsLog (LOG_ERROR, "msiMakeGenQuery: input condStr is NULL.");
		return (USER__NULL_INPUT_ERR);
	}


	/* The code below is partly taken from msiMakeQuery and msiExecStrCondQuery. There may be a better way to do this. */

	/* Generate raw SQL query string */
	rei->status = _makeQuery(sel, cond, &rawQuery);

	/* allocate more memory for query string with expanded variable names */
	query = (char *)malloc(strlen(rawQuery) + 10 + MAX_COND_LEN * 8);
	strcpy(query, rawQuery);

	/* parse variables and replace them with their value */
	rei->status  = replaceVariablesAndMsParams("", query, rei->msParamArray, rei);
	if (rei->status < 0)
	{
		rodsLog (LOG_ERROR, "msiMakeGenQuery: replaceVariablesAndMsParams failed.");
		return(rei->status);
	}

	/* allocate memory for genQueryInp */
	genQueryInp = (genQueryInp_t*)malloc(sizeof(genQueryInp_t));
	memset (genQueryInp, 0, sizeof (genQueryInp_t));

	/* set up GenQueryInp */
	genQueryInp->maxRows = MAX_SQL_ROWS;
	genQueryInp->continueInx = 0;
	rei->status = fillGenQueryInpFromStrCond(query, genQueryInp);
	if (rei->status < 0)
	{
		rodsLog (LOG_ERROR, "msiMakeGenQuery: fillGenQueryInpFromStrCond failed.");
		return(rei->status);
	}


	/* return genQueryInp through GenQueryInpParam */
	genQueryInpParam->type = strdup(GenQueryInp_MS_T);
	genQueryInpParam->inOutStruct = genQueryInp;


	/* cleanup */
	free(rawQuery);
	free(query);

	return(rei->status);
}




int
msiPrintGenQueryInp( msParam_t *where, msParam_t* genQueryInpParam, ruleExecInfo_t *rei)
{
  genQueryInp_t *genQueryInp;
  int i, j;
  char *writeId;
  char writeStr[MAX_NAME_LEN * 2];
  int len;
  int *ip1, *ip2;
  char *cp;
  char **cpp;

  RE_TEST_MACRO ("    Calling msiPrintGenQueryInp");
    
  if (rei == NULL || rei->rsComm == NULL) {
    rodsLog (LOG_ERROR, "msiPrintGenQueryInp: input rei or rsComm is NULL.");
    return (SYS_INTERNAL_NULL_INPUT_ERR);
  }
  if (!where) {
    rodsLog (LOG_ERROR, "msiPrintGenQueryInp: No destination provided for writing.");
    return (USER__NULL_INPUT_ERR);
  }
  /* where are we writing to? */
  if (where->inOutStruct != NULL) {
    writeId = where->inOutStruct;
  }
  else {
    writeId = where->label;
  }

  genQueryInp = (genQueryInp_t *)  strtol((char *)genQueryInpParam->inOutStruct,
					  (char **) NULL,0);


  /* print each selection  pair to writeStr */
  len = genQueryInp->selectInp.len;
  ip1 = genQueryInp->selectInp.inx;
  ip2 = genQueryInp->selectInp.value;
  for (i=0;i<len;i++) {
    sprintf(writeStr,"Selected Column %d With Option %d\n",*ip1, *ip2);
    j = _writeString(writeId, writeStr, rei);
    if (j < 0)
      return(j);
    ip1++;
    ip2++;
  }

  len = genQueryInp->sqlCondInp.len;
  ip1 = genQueryInp->sqlCondInp.inx;
  cpp = genQueryInp->sqlCondInp.value;
  cp = *cpp;
  for (i=0;i<len;i++) {
    sprintf(writeStr,"Condition Column %d %s\n", *ip1, cp);
    j = _writeString(writeId, writeStr, rei);
    if (j < 0)
      return(j);
    ip1++;
    cpp++;
    cp = *cpp;
  }
  return(0);
}
