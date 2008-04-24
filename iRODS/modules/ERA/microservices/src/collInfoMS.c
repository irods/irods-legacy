/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "collInfoMS.h"
#include "eraUtil.h"



/*
 * msiGetCollectionContentsReport() - STUB
 *
 */
int
msiGetCollectionContentsReport(msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, ruleExecInfo_t *rei)
{
	genQueryInp_t genQueryInp;		/* for query inputs */
	char condStr[MAX_NAME_LEN];		/* for query condition */
	genQueryOut_t *genQueryOut;		/* for query results */


	RE_TEST_MACRO ("    Calling msiGetCollectionContentsReport")
	
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiGetCollectionContentsReport: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}


	/* How many data types are there? Count all token IDs whose namespace is data_type */
	memset (&genQueryInp, 0, sizeof (genQueryInp));
	addInxIval (&genQueryInp.selectInp, COL_TOKEN_ID, 1);


	/* conditions and options*/
	snprintf (condStr, MAX_NAME_LEN, "= 'data_type'");
	addInxVal (&genQueryInp.sqlCondInp,  COL_TOKEN_NAMESPACE, condStr);

	genQueryInp.options = RETURN_TOTAL_ROW_COUNT;
	genQueryInp.maxRows = -1;


	/* Query */
	rei->status  = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);


	/* return operation status through outParam */
// 	fillIntInMsParam (outParam, rei->status);
	fillIntInMsParam (outParam, genQueryOut->totalRowCount);

	return (rei->status);
}




