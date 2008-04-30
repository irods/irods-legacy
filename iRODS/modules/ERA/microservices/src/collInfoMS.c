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
	char *resultStringToken;
	char *oldValStr, newValStr[21];
	rodsLong_t newVal; 
	sqlResult_t *sqlResult;
	int i;
	keyValPair_t contents;

	collInp_t collInpCache, *outCollInp;
	char collQCond[2*MAX_NAME_LEN];		/* for condition in rsGenquery() */
	bytesBuf_t *mybuf;

	genQueryInp_t genQueryInp;		/* for query inputs */
	genQueryOut_t *genQueryOut;		/* for query results */


	RE_TEST_MACRO ("    Calling msiGetCollectionContentsReport")
	
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiGetCollectionContentsReport: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}


	/* buffer init */
	mybuf = (bytesBuf_t *)malloc(sizeof(bytesBuf_t));
	memset (mybuf, 0, sizeof (bytesBuf_t));

	memset (&contents, 0, sizeof (contents));


	/* parse inpParam1 */
	rei->status = parseMspForCollInp (inpParam1, &collInpCache, &outCollInp, 0);
	
	if (rei->status < 0) {
		rodsLog (LOG_ERROR, "msiGetCollectionContentsReport: input inpParam1 error. status = %d", rei->status);
		return (rei->status);
	}


	/* How many data types are there? Count all token IDs whose namespace is data_type */
	memset (&genQueryInp, 0, sizeof (genQueryInp));
	addInxIval (&genQueryInp.selectInp, COL_DATA_TYPE_NAME, 1);
	addInxIval (&genQueryInp.selectInp, COL_D_DATA_ID, 1);
	addInxIval (&genQueryInp.selectInp, COL_COLL_ID, 1);


	/* Conditions and options */
	/* make the condition */
	genAllInCollQCond (outCollInp->collName, collQCond);
	
	addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, collQCond);

	genQueryInp.maxRows = MAX_SQL_ROWS;

	genQueryInp.options = RETURN_TOTAL_ROW_COUNT;


	/* Query */
	rei->status  = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);


	/**********************************/

	/* print results out to buffer */
	if (rei->status == 0) {

		for (i=0;i<genQueryOut->rowCnt;i++) {
	

			sqlResult = getSqlResultByInx (genQueryOut, COL_DATA_TYPE_NAME);

			/* retrive value for this row */
			resultStringToken = sqlResult->value + i*sqlResult->len;

			oldValStr = getValByKey (&contents, resultStringToken);
			if (oldValStr) {
				newVal = atoll(oldValStr) + 1;
			}
			else {
				newVal = 1;
			}
		
			snprintf(newValStr, 21, "%lld", newVal);
			addKeyVal(&contents, resultStringToken, newValStr);

		}



		/* not done? */
		while (rei->status==0 && genQueryOut->continueInx > 0) {
			genQueryInp.continueInx=genQueryOut->continueInx;
			rei->status = rsGenQuery(rei->rsComm, &genQueryInp, &genQueryOut);

			for (i=0;i<genQueryOut->rowCnt;i++) {
		
	
				sqlResult = getSqlResultByInx (genQueryOut, COL_DATA_TYPE_NAME);
	
				/* retrive value for this row */
				resultStringToken = sqlResult->value + i*sqlResult->len;
	
				oldValStr = getValByKey (&contents, resultStringToken);
				if (oldValStr) {
					newVal = atoll(oldValStr) + 1;
				}
				else {
					newVal = 1;
				}
			
				snprintf(newValStr, 21, "%lld", newVal);
				addKeyVal(&contents, resultStringToken, newValStr);
	
			}
		}
	}



	/* print contents of keyvalpair to buffer */
	for (i=0; i<contents.len; i++) {
		appendStrToBBuf(mybuf, contents.keyWord[i]);
		appendStrToBBuf(mybuf, ": ");
		appendStrToBBuf(mybuf, contents.value[i]);
		appendStrToBBuf(mybuf, "\n");
	}



	/* send results out to outParam */
	fillBufLenInMsParam (inpParam2, strlen(mybuf->buf), mybuf);






	/* Return operation status through outParam */
// 	fillIntInMsParam (outParam, rei->status);
	fillIntInMsParam (outParam, genQueryOut->totalRowCount);




	return (rei->status);
}




