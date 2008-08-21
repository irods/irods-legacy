#include "integritychecksMS.h"
#include "icutils.h"

int msiListCollACL (msParam_t* mPin1, msParam_t* mPout1, msParam_t* mPout2, ruleExecInfo_t *rei) {

	int i,j;
	char tmpstr[MAX_NAME_LEN];
	genQueryInp_t	gqin;
	genQueryOut_t*	gqout = NULL;
	char* collname=NULL;
	char* collid=NULL;
	rsComm_t *rsComm;
	bytesBuf_t* mybuf=NULL;
	sqlResult_t* collAccessType;;
	sqlResult_t* collCollId;
	sqlResult_t* collAccessName;
	sqlResult_t* collDataName;

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
	memset (&gqin, 0, sizeof (genQueryInp_t));
	//clearGenQueryInp (&gqin);
	gqin.maxRows = MAX_SQL_ROWS;

	/* Currently necessary since other namespaces exist in the token table */
	//snprintf (tmpstr, MAX_NAME_LEN, "='%s'", "access_type");
	//addInxVal (&gqin.sqlCondInp, COL_COLL_TOKEN_NAMESPACE, tmpStr);

	/* First get the collection id from the collection name - then using 
		that info - and in the future chaining together these microservices -
		we can get all the data objects access types */

	collname = (char*) strdup (mPin1->inOutStruct);
	if (collname == NULL) {
		return (USER__NULL_INPUT_ERR);
	} else {
		snprintf (tmpstr, MAX_NAME_LEN, " = '%s'", collname);
		addInxVal (&gqin.sqlCondInp, COL_COLL_NAME, tmpstr);
	}
	addInxIval (&gqin.selectInp, COL_D_COLL_ID, 1);

	j = rsGenQuery (rsComm, &gqin, &gqout);
	
	if (j<0) {
		appendToByteBuf (mybuf, "Coll ID Not found\n");
	}
	else if (j != CAT_NO_ROWS_FOUND) {

		printGenQueryOut(stderr, NULL, NULL, gqout);

		/* don't we really just need the first element, since all the collection id's should be the same? */
		collCollId = getSqlResultByInx (gqout, COL_D_COLL_ID);
		collid = strdup (&collCollId->value[0]);
		sprintf (tmpstr, "Collection ID:%s\n", &collCollId->value[0]);
		appendToByteBuf (mybuf, tmpstr);

	} else {

		appendToByteBuf (mybuf, "Some unknown error\n");
	}

	/* Now do the query to get all the interesting information re: ACLS */
	memset (&gqin, 0, sizeof (genQueryInp_t));
	gqin.maxRows = MAX_SQL_ROWS;
	
	addInxIval (&gqin.selectInp, COL_DATA_NAME, 1);
	addInxIval (&gqin.selectInp, COL_DATA_ACCESS_NAME, 1);
	addInxIval (&gqin.selectInp, COL_DATA_ACCESS_TYPE, 1);
	snprintf (tmpstr, MAX_NAME_LEN, " = '%s'", collid);
	addInxVal (&gqin.sqlCondInp, COL_D_COLL_ID, tmpstr);

	j = rsGenQuery (rsComm, &gqin, &gqout);

	if (j<0) {
		 appendToByteBuf (mybuf, "Second gen query bad\n");
	} else if  (j != CAT_NO_ROWS_FOUND) {

		collDataName = getSqlResultByInx (gqout, COL_DATA_NAME);
		collAccessType = getSqlResultByInx (gqout, COL_DATA_ACCESS_TYPE);
		collAccessName = getSqlResultByInx (gqout, COL_DATA_ACCESS_NAME);
		fprintf (stderr, "rowCnt:%d\n", gqout->rowCnt);
		for (i=0; i<gqout->rowCnt; i++) {
			sprintf (tmpstr, "Data name:%s\tUser name:%s\tAccess Name:%s\n", 
				&collDataName->value[collDataName->len *i], 
				&collAccessType->value[collAccessType->len *i], 
				&collAccessName->value[collAccessName->len *i]);
			appendToByteBuf (mybuf, tmpstr);
		}	

		printGenQueryOut(stderr, NULL, NULL, gqout);
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
	int debug=1;

	RE_TEST_MACRO ("    Calling msiListFields")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiListFields: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;

	/* init gqout, gqin & mybuf structs */
	mybuf = (bytesBuf_t *) malloc(sizeof(bytesBuf_t));
	memset (mybuf, 0, sizeof (bytesBuf_t));
	gqout = (genQueryOut_t *) malloc (sizeof (genQueryOut_t));
	memset (gqout, 0, sizeof (genQueryOut_t));
	memset (&gqin, 0, sizeof (genQueryInp_t));

	gqin.maxRows = MAX_SQL_ROWS;

	/* construct an SQL query from the parameter list */
	collname = (char*) strdup (mPin1->inOutStruct);
	fieldname = (char*) strdup (mPin2->inOutStruct);
	if ((fieldid = getAttrIdFromAttrName(fieldname))==NO_COLUMN_NAME_FOUND) {
		sprintf (tmpstr, "Field: %s not found in database", fieldname);
		appendToByteBuf (mybuf, tmpstr);
		return (-1);
	}

	if (debug) rodsLog (LOG_NOTICE, "fieldname: %s\tfieldid:%d", fieldname,fieldid);

	/* this is the info we want returned from the query */
	addInxIval (&gqin.selectInp, COL_DATA_NAME, 1);
	addInxIval (&gqin.selectInp, fieldid, 1);
	snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&gqin.sqlCondInp, COL_COLL_NAME, condStr);

	j = rsGenQuery (rsComm, &gqin, &gqout);

	if (j<0) {
		appendToByteBuf (mybuf, "Gen Query Error returned badness");
	} else if (j != CAT_NO_ROWS_FOUND) {

		printGenQueryOut(stderr, NULL, NULL, gqout);

		dataName = getSqlResultByInx (gqout, COL_DATA_NAME);
		dataField = getSqlResultByInx (gqout, fieldid);

		rodsLog (LOG_ERROR, "got here 3 rowCnt=%d",gqout->rowCnt);

		for (i=0; i<gqout->rowCnt; i++) {
			sprintf (tmpstr, "Data object:%s\t%s:%s\n", &dataName->value[dataName->len *i], fieldname, &dataField->value[dataField->len *i]);
		rodsLog (LOG_ERROR, "got here 4");
			appendToByteBuf (mybuf, tmpstr);
		}

	} else appendToByteBuf (mybuf, "No matching rows found");

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

