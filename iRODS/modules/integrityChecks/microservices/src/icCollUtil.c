/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* icCollUtil.c
 */

#include "icCollUtil.h"

int	msiListColl (msParam_t* collectionname, msParam_t* buf, ruleExecInfo_t* rei) {

	rsComm_t* rsComm;
	sqlResult_t *collectionName;
	sqlResult_t *collectionID;
	bytesBuf_t* mybuf=NULL;
	genQueryInp_t gqin;
	genQueryOut_t* gqout=NULL;
	int i,status;
	char condStr[MAX_NAME_LEN];
	char* collname;

	RE_TEST_MACRO ("    Calling msiListColl")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiListColl: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;

	/* need to turn parameter 1 into a collInp_t struct */
	collname = strdup (collectionname->inOutStruct);
	
	/* init stuff */
	memset (&gqin, 0, sizeof(genQueryInp_t));
	gqin.maxRows = MAX_SQL_ROWS;
	gqout = (genQueryOut_t*) malloc (sizeof (genQueryOut_t));
	memset (gqout, 0, sizeof (genQueryOut_t));
	mybuf = (bytesBuf_t *) malloc (sizeof (bytesBuf_t));
	memset (mybuf, 0, sizeof (bytesBuf_t));

	/* Generate a query - we only want subcollection data objects */
    addInxIval (&gqin.selectInp, COL_COLL_NAME, 1);
    addInxIval (&gqin.selectInp, COL_COLL_ID, 1);
	genAllInCollQCond (collname, condStr);
    addInxVal (&gqin.sqlCondInp, COL_COLL_NAME, condStr);

	/* This is effectively a recursive query */
    status = rsGenQuery (rsComm, &gqin, &gqout);

	if (status < 0) {
		return (status);
	} else if (status != CAT_NO_ROWS_FOUND)  {

		collectionName = getSqlResultByInx (gqout, COL_COLL_NAME);
		collectionID = getSqlResultByInx (gqout, COL_COLL_ID);
		for (i=1; i<gqout->rowCnt; i++) {

			char* subcoll;
			char tmpstr[MAX_NAME_LEN];

			subcoll = &collectionName->value[collectionName->len*i];
			sprintf (tmpstr, "%s\n", subcoll );
			appendToByteBuf (mybuf, tmpstr);
		}

	} else {
		return (status); /* something bad happened */
	}

    fillBufLenInMsParam (buf, mybuf->len, mybuf);	
	return (rei->status);

}
	
