#include "integritychecksMS.h"
#include "icutils.h"

int msiListCollACL (msParam_t* mPin1, msParam_t* mPout1, msParam_t* mPout2, ruleExecInfo_t *rei) {

	int i,j;
	char tmpstr[MAX_NAME_LEN];
	genQueryInp_t	gqin;
	genQueryOut_t*	gqout = NULL;
	char* collname=NULL;
	rsComm_t *rsComm;
	bytesBuf_t* mybuf=NULL;
	sqlResult_t* collUserName;
	sqlResult_t* collAccessName;

	RE_TEST_MACRO ("    Calling msiListCollACL")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiListCollACL: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}
	rsComm = rei->rsComm;

	/* init gqout, gqin & mybuf structs */
	mybuf = (bytesBuf_t *)malloc(sizeof(bytesBuf_t));
	memset (mybuf, 0, sizeof (bytesBuf_t));
	gqout = (genQueryOut_t *) malloc (sizeof (genQueryOut_t));
	memset (gqout, 0, sizeof (genQueryOut_t));
	clearGenQueryInp (&gqin);

	gqin.maxRows = MAX_SQL_ROWS;

	addInxIval (&gqin.selectInp, COL_COLL_USER_NAME, 1);
	addInxIval (&gqin.selectInp, COL_COLL_ACCESS_NAME, 1);

	/* Currently necessary since other namespaces exist in the token table */
	//snprintf (tmpstr, MAX_NAME_LEN, "='%s'", "access_type");
	//addInxVal (&gqin.sqlCondInp, COL_COLL_TOKEN_NAMESPACE, tmpStr);

	collname = (char*) strdup (mPin1->inOutStruct);
	if (collname == NULL) {
		return (USER__NULL_INPUT_ERR);
	} else {
		snprintf (tmpstr, MAX_NAME_LEN, " = '%s'", collname);
		addInxVal (&gqin.sqlCondInp, COL_COLL_NAME, tmpstr);
	}

	j = rsGenQuery (rsComm, &gqin, &gqout);
	
	if (j<0) {
		appendToByteBuf (mybuf, "Some genQuery error\n");
	}
	else if (j != CAT_NO_ROWS_FOUND) {

		//printGenQueryOut(stderr, NULL, NULL, gqout);
		collUserName = getSqlResultByInx (gqout, COL_COLL_USER_NAME);
		collAccessName = getSqlResultByInx (gqout, COL_COLL_ACCESS_NAME);

		for (i=0; i<gqout->rowCnt; i++) {
			sprintf (tmpstr, "Coll user name:%s\taccess name:%s\n", &collUserName->value[collUserName->len *i], &collAccessName->value[collAccessName->len *i]);
			appendToByteBuf (mybuf, tmpstr);
		}
		appendToByteBuf (mybuf, "Something was found\n");

	} else {

		appendToByteBuf (mybuf, "Some unknown error\n");
	}

	fillBufLenInMsParam (mPout1, mybuf->len, mybuf);
	fillIntInMsParam (mPout2, rei->status);
  
	return(rei->status);

	return(0);
	
}

/* Utility function for listing file object and an input parameter field */
int msiListFields (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPout1, msParam_t* mPout2, ruleExecInfo_t *rei) {

	genQueryInp_t gqin;
	genQueryOut_t *gqout = NULL;
	char condStr[MAX_NAME_LEN];
	char tmpstr[MAX_NAME_LEN];
	rsComm_t *rsComm;
	char* collname;
	char* fieldname;
	sqlResult_t *dataName;
	sqlResult_t *dataField;
	bytesBuf_t*	mybuf=NULL;
	int i,j;
	int	fieldid;

	RE_TEST_MACRO ("    Calling msiListFields")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiListFields: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;

	/* init stuff */
	memset (&gqin, 0, sizeof(genQueryInp_t));
	gqin.maxRows = MAX_SQL_ROWS;
    mybuf = (bytesBuf_t *)malloc(sizeof(bytesBuf_t));
    memset (mybuf, 0, sizeof (bytesBuf_t));

	/* construct an SQL query from the parameter list */
	collname = (char*) strdup (mPin1->inOutStruct);
	fieldname = (char*) strdup (mPin2->inOutStruct);
	fieldid = getAttrIdFromAttrName(fieldname);

	rodsLog (LOG_ERROR, "fieldname: %s\tfieldid:%d", fieldname,fieldid);

	/* this is the info we want returned from the query */
	addInxIval (&gqin.selectInp, COL_DATA_NAME, 1);
	addInxIval (&gqin.selectInp, fieldid, 1);
	snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&gqin.sqlCondInp, COL_COLL_NAME, condStr);

	rodsLog (LOG_ERROR, "got here 1");

	j = rsGenQuery (rsComm, &gqin, &gqout);

	if (j != CAT_NO_ROWS_FOUND) {

		rodsLog (LOG_ERROR, "got here 2");

		printGenQueryOut(stderr, NULL, NULL, gqout);

		dataName = getSqlResultByInx (gqout, COL_DATA_NAME);
		dataField = getSqlResultByInx (gqout, fieldid);

		rodsLog (LOG_ERROR, "got here 3 rowCnt=%d",gqout->rowCnt);

		for (i=0; i<gqout->rowCnt; i++) {
			sprintf (tmpstr, "Data object:%s\t%s:%s\n", &dataName->value[dataName->len *i], fieldname, &dataField->value[dataField->len *i]);
		rodsLog (LOG_ERROR, "got here 4");
			appendToByteBuf (mybuf, tmpstr);
		}

	} 

	fillBufLenInMsParam (mPout1, mybuf->len, mybuf);
	fillIntInMsParam (mPout2, rei->status);
  
	return(rei->status);

}


/* Silly hello world microservice */
int msiHiThere (ruleExecInfo_t *rei) {

	int i;

	RE_TEST_MACRO ("    Calling msiHiThere")

	i = hithere ();
	return(i);
}

