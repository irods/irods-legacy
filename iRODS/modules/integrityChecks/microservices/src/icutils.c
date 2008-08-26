#include "integritychecksMS.h"
#include "icutils.h"

int msiListCollACL (msParam_t* mPin1, msParam_t* mPin2, msParam_t* mPin3, msParam_t* mPin4, msParam_t* mPout1, msParam_t* mPout2, ruleExecInfo_t *rei) {

	int i,j;
	int querytype=0;
	char tmpstr[MAX_NAME_LEN];
	genQueryInp_t	gqin;
	genQueryOut_t*	gqout = NULL;
	char* collname=NULL;
	char* collid=NULL;
	char* username=NULL;
	char* accessauth=NULL;
	char* notflag=NULL;
	rsComm_t *rsComm;
	bytesBuf_t* mybuf=NULL;
	sqlResult_t* collCollId;
	sqlResult_t* dataName;
	sqlResult_t* dataAccessName;
	sqlResult_t* dataAccessType;
	sqlResult_t* dataAccessDataId;
	sqlResult_t* dataAccessUserId;
	sqlResult_t* dataTokenNamespace;

	RE_TEST_MACRO ("    Calling msiListCollACL")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiListCollACL: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}
	rsComm = rei->rsComm;

	/* This function can perform three different checks, depending on flags set via input parameters:
		1. check that its ACL contains the same set of user-authorization pairs as others in its collection
		2. check that its ACL contains at least a given set of user-authorization pairs
		3. check that its ACL does not contain a given set of user-authorization pairs 

		We have four input parameters: Collection Name, User Name, Authorization Type & NOT flag
		For the above conditions, the following are examples of how to call the rule
		1. collname=/sdscZone/home/rods%*User=rods%*Auth=own  
		2. collname=/sdscZone/home/rods  
		3. collname=/sdscZone/home/rods%*User=rods%*Auth=own*Notflag=1  
	*/

	collname = (char*) strdup (mPin1->inOutStruct);
	if (collname == NULL) return (USER__NULL_INPUT_ERR);
	/* These next three don't necessarily have to be set */
	username = (char*) strdup (mPin2->inOutStruct);
	accessauth = (char*) strdup (mPin3->inOutStruct);
	notflag = (char*) strdup (mPin4->inOutStruct);
	
	if ((username==NULL) && (accessauth==NULL)) {
		querytype=1; /* Case #1. see above */
		notflag=NULL; /* we don't care about this variable in this case */
	} 
	else if (((username==NULL) && (accessauth!=NULL)) || ((username!=NULL) && (accessauth==NULL))) {
			return (USER__NULL_INPUT_ERR);
	} else {
		if (atoi(notflag)==0)
			querytype=2;
		else
			querytype=3;
	}
		
	
	/* init gqout, gqin & mybuf structs */
	gqout = (genQueryOut_t*) malloc (sizeof (genQueryOut_t));
	memset (gqout, 0, sizeof (genQueryOut_t));
	mybuf = (bytesBuf_t*) malloc(sizeof(bytesBuf_t));
	memset (mybuf, 0, sizeof (bytesBuf_t));
	gqin.maxRows = MAX_SQL_ROWS;


	/* First get the collection id from the collection name, then
		use the collection id field as a key to obtaining all the
		other nifty info we need */ 
	snprintf (tmpstr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&gqin.sqlCondInp, COL_COLL_NAME, tmpstr);
	addInxIval (&gqin.selectInp, COL_D_COLL_ID, 1);

	j = rsGenQuery (rsComm, &gqin, &gqout);
	
	if (j<0) {
		appendToByteBuf (mybuf, "Coll ID Not found\n");
	}
	else if (j != CAT_NO_ROWS_FOUND) {

		printGenQueryOut(stderr, NULL, NULL, gqout);

		/* RAJA don't we really just need the first element, since all the collection id's should be the same? */
		collCollId = getSqlResultByInx (gqout, COL_D_COLL_ID);
		collid = strdup (&collCollId->value[0]);
		sprintf (tmpstr, "Collection ID:%s\n", &collCollId->value[0]);
		appendToByteBuf (mybuf, tmpstr);

	} else {
		appendToByteBuf (mybuf, "RAJA - what unknown error\n");
		return (1);  /* what return value RAJA */
	}

	/* Now do another query to get all the interesting ACL information */
	memset (&gqin, 0, sizeof (genQueryInp_t));
	gqin.maxRows = MAX_SQL_ROWS;
	
	addInxIval (&gqin.selectInp, COL_DATA_NAME, 1);
	addInxIval (&gqin.selectInp, COL_DATA_ACCESS_NAME, 1);
	addInxIval (&gqin.selectInp, COL_DATA_ACCESS_TYPE, 1);
	addInxIval (&gqin.selectInp, COL_DATA_ACCESS_DATA_ID, 1);
	addInxIval (&gqin.selectInp, COL_DATA_ACCESS_USER_ID, 1);
	addInxIval (&gqin.selectInp, COL_DATA_TOKEN_NAMESPACE, 1);

	/* Currently necessary since other namespaces exist in the token table */
	snprintf (tmpstr, MAX_NAME_LEN, "='%s'", "access_type");
	addInxVal (&gqin.sqlCondInp, COL_COLL_TOKEN_NAMESPACE, tmpStr);

	snprintf (tmpstr, MAX_NAME_LEN, " = '%s'", collid);
	addInxVal (&gqin.sqlCondInp, COL_D_COLL_ID, tmpstr);

	j = rsGenQuery (rsComm, &gqin, &gqout);

	if (j<0) {
		 appendToByteBuf (mybuf, "Second gen query bad\n");
	} else if  (j != CAT_NO_ROWS_FOUND) {

		dataName = getSqlResultByInx (gqout, COL_DATA_NAME);
		dataAccessType = getSqlResultByInx (gqout, COL_DATA_ACCESS_TYPE);
		dataAccessName = getSqlResultByInx (gqout, COL_DATA_ACCESS_NAME);
		dataAccessDataId = getSqlResultByInx (gqout, COL_DATA_ACCESS_DATA_ID);
		dataAccessUserId = getSqlResultByInx (gqout, COL_DATA_ACCESS_USER_ID);
		dataTokenNamespace = getSqlResultByInx (gqout, COL_DATA_TOKEN_NAMESPACE);

		for (i=0; i<gqout->rowCnt; i++) {
			sprintf (tmpstr, "Data name:%s\tData Access Type:%s\tData Access Name:%s\tData Access Data Id:%s\t Data Access User ID:%s\tNamespace:%s\n",
				&dataName->value[dataName->len *i], 
				&dataAccessType->value[dataAccessType->len *i], 
				&dataAccessName->value[dataAccessName->len *i],
				&dataAccessDataId->value[dataAccessDataId->len *i],
				&dataAccessUserId->value[dataAccessUserId->len *i],
				&dataTokenNamespace->value[dataTokenNamespace->len *i]);
			appendToByteBuf (mybuf, tmpstr);
		}	

		printGenQueryOut(stderr, NULL, NULL, gqout);
	} else appendToByteBuf (mybuf, "something else gone bad RAJA");
	

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
int msiHiThere (msParam_t* mPout1, ruleExecInfo_t *rei) {

	char str[]="hi there\n";

	RE_TEST_MACRO ("    Calling msiHiThere")
	fillStrInMsParam (mPout1, str);

	return(rei->status);
	
}

