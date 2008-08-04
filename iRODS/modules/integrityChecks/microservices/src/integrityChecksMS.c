#include "integrityChecksMS.h"


int msiCheckFileDatatypes (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPout1, ruleExecInfo_t *rei) {

	genQueryInp_t genQueryInp;
	genQueryOut_t *genQueryOut = NULL;
	char condStr[MAX_NAME_LEN];
	rsComm_t *rsComm;
	char collname[200];
	char datatypeparam[200];
	int i,j;
	sqlResult_t *dataName;
	sqlResult_t *dataType;
	char delims[]=",";
	char* word;

	keyValPair_t	*results;	
	char* key;
	char* value;
	
	RE_TEST_MACRO ("    Calling msiCheckDatatypes")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiCheckFileDatatypes: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;

	//rodsLog (LOG_ERROR, "msiCheckFileDatatypes: got here 0");

	/* construct an SQL query from the parameter list */
	strcpy (collname,  (char*) mPin1->inOutStruct);
	strcpy (datatypeparam, (char*) mPin2->inOutStruct);

	fprintf (stderr, "datatypeparam: %s\n", datatypeparam);

	// initialize results to 0; AddKeyVal does all our malloc-ing
	results = (keyValPair_t*) malloc (sizeof(keyValPair_t));
	memset (results, 0, sizeof(keyValPair_t));


	/* Parse the comma-delimited datatype list & make a separate query for each datatype*/
	for (word=strtok(datatypeparam, delims); word; word=strtok(NULL, delims)) {

		fprintf (stderr, "word: %s\n", word);

		memset (&genQueryInp, 0, sizeof(genQueryInp_t));
		genQueryInp.maxRows = MAX_SQL_ROWS;

		/* this is the info we want returned from the query */
		addInxIval (&genQueryInp.selectInp, COL_DATA_NAME, 1);
		addInxIval (&genQueryInp.selectInp, COL_DATA_TYPE_NAME, 1);
		snprintf (condStr, MAX_NAME_LEN, " = '%s'", word);
		addInxVal (&genQueryInp.sqlCondInp, COL_DATA_TYPE_NAME, condStr); 
	
		rodsLog (LOG_ERROR, "msiCheckFileDatatypes: got here 2");

		j = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

		rodsLog (LOG_ERROR, "msiCheckFileDatatypes: got here 3");

		if (j != CAT_NO_ROWS_FOUND) {

			fprintf (stderr, "word: %s\trows:%d\n", word, genQueryOut->rowCnt);

			/* we got results - do something cool */
			dataName = getSqlResultByInx (genQueryOut, COL_DATA_NAME);
			dataType = getSqlResultByInx (genQueryOut, COL_DATA_TYPE_NAME);

			for (i=0; i<genQueryOut->rowCnt; i++) {
				key = strdup (&dataName->value[dataName->len *i]);
				value = strdup (&dataType->value[dataType->len * i]);
				addKeyVal (results, key, value);
			}	

			printGenQueryOut(stderr, NULL, NULL, genQueryOut);

		} else continue; 

	}

	fillMsParam (mPin2, NULL, KeyValPair_MS_T, results, NULL);
	//rodsLog (LOG_ERROR, "7 s tuff: ");
	fillIntInMsParam (mPout1, rei->status);
	//rodsLog (LOG_ERROR, "8 s tuff: ");
  
	return(rei->status);

}

int msiCheckFilesizeRange (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPin3, msParam_t *mPout1, ruleExecInfo_t *rei) {

	genQueryInp_t genQueryInp;
	genQueryOut_t *genQueryOut = NULL;
	char condStr[MAX_NAME_LEN];
	rsComm_t *rsComm;
	char collname[200];
	char maxfilesize[100]; 
	char minfilesize[100];
	int i,j;
	sqlResult_t *dataName;
	sqlResult_t *dataSize;

	keyValPair_t	*results;	
	char* key;
	char* value;
	
	RE_TEST_MACRO ("    Calling msiCheckFilesizeRange")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiCheckFilesizeRange: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;

	/* construct an SQL query from the input parameter list */
	strcpy (collname,  (char*) mPin1->inOutStruct);
	strcpy (minfilesize, (char*) mPin2->inOutStruct);
	strcpy (maxfilesize, (char*) mPin3->inOutStruct);

	/* But first, make sure our size range is valid */
	if (atoi(minfilesize) >= atoi(maxfilesize)) {
		return (USER_PARAM_TYPE_ERR);
	}

	// initialize results to 0; AddKeyVal does all our malloc-ing
	results = (keyValPair_t*) malloc (sizeof(keyValPair_t));
	memset (results, 0, sizeof(keyValPair_t));

	memset (&genQueryInp, 0, sizeof(genQueryInp_t));
	genQueryInp.maxRows = MAX_SQL_ROWS;

	/* this is the info we want returned from the query */
	addInxIval (&genQueryInp.selectInp, COL_DATA_NAME, 1);
	addInxIval (&genQueryInp.selectInp, COL_DATA_SIZE, 1);
	addInxIval (&genQueryInp.selectInp, COL_COLL_NAME, 1);

	rodsLog (LOG_ERROR, "2 stuff: ");

	/* build the condition:
		collection name AND (filesize < minfilesize or filesize > maxfilesize) */

	snprintf (condStr, MAX_NAME_LEN, " < '%s' || > '%s'", minfilesize, maxfilesize);
	addInxVal (&genQueryInp.sqlCondInp, COL_DATA_SIZE, condStr); 
	snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, condStr); 

	rodsLog (LOG_ERROR, "3 stuff: ");
	
	j = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

	if (j != CAT_NO_ROWS_FOUND) {

		/* we got results - do something cool */
		dataName = getSqlResultByInx (genQueryOut, COL_DATA_NAME);
		dataSize = getSqlResultByInx (genQueryOut, COL_DATA_SIZE);
		for (i=0; i<genQueryOut->rowCnt; i++) {
			// fprintf (stderr, "dataName[%d]:%s\n",i,&dataName->value[dataName->len * i]);
			key = strdup (&dataName->value[dataName->len *i]);
			// fprintf (stderr, "dataSize[%d]:%s\n",i,&dataSize->value[dataSize->len * i]);
			value = strdup (&dataSize->value[dataSize->len * i]);
			addKeyVal (results, key, value);
		}

		// rodsLog (LOG_ERROR, "4.8 stuff: ");

	} else {
		fillIntInMsParam (mPout1, rei->status);
		return (rei->status);  //ack this is ugly
	}

	rodsLog (LOG_ERROR, "6 stuff: ");
	printGenQueryOut(stderr, NULL, NULL, genQueryOut);

	fillMsParam (mPin3, NULL, KeyValPair_MS_T, results, NULL);
	//rodsLog (LOG_ERROR, "7 s tuff: ");
	fillIntInMsParam (mPout1, rei->status);
	//rodsLog (LOG_ERROR, "8 s tuff: ");
  
	return(rei->status);

}


/* Return and list all contents of a collection */
int msiListCollection (msParam_t *mPin1, msParam_t *mPout1, ruleExecInfo_t *rei) {

	genQueryInp_t genQueryInp;
	genQueryOut_t *genQueryOut = NULL;
	char condStr[MAX_NAME_LEN];
	rsComm_t *rsComm;
	char collname[200];
	int i,j;
	sqlResult_t *dataName;
	sqlResult_t *dataType;

	keyValPair_t	*results;	
	char* key;
	char* value;
	
	RE_TEST_MACRO ("    Calling msiListCollection")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiCheckFilesizeRange: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;

	/* construct an SQL query from the input parameter list */
	strcpy (collname,  (char*) mPin1->inOutStruct);

	// initialize results to 0; AddKeyVal does all our malloc-ing
	results = (keyValPair_t*) malloc (sizeof(keyValPair_t));
	memset (results, 0, sizeof(keyValPair_t));

	memset (&genQueryInp, 0, sizeof(genQueryInp_t));
	genQueryInp.maxRows = MAX_SQL_ROWS;

	/* this is the info we want returned from the query */
	addInxIval (&genQueryInp.selectInp, COL_DATA_NAME, 1);
	addInxIval (&genQueryInp.selectInp, COL_DATA_TYPE_NAME, 1);
	addInxIval (&genQueryInp.selectInp, COL_COLL_NAME, 1);

	rodsLog (LOG_ERROR, "2 stuff: ");

	/* build the condition:
		collection name AND (filesize < minfilesize or filesize > maxfilesize) */

	snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, condStr); 

	rodsLog (LOG_ERROR, "3 stuff: ");
	
	j = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

	if (j != CAT_NO_ROWS_FOUND) {

		/* we got results - do something cool */
/*
		fprintf (stderr, "GenQO->rowCnt: %d\n", genQueryOut->rowCnt);
		fprintf (stderr, "GenQO->totalRowCount: %d\n", genQueryOut->totalRowCount);
		fprintf (stderr, "GenQO->attriCnt: %d\n", genQueryOut->attriCnt);

		rodsLog (LOG_ERROR, "4 stuff: ");

		for (i=0; i<genQueryOut->rowCnt; i++) {
			fprintf (stderr, "genQO->sqlResult[%d].attriInx= %d\n", i,genQueryOut->sqlResult[i].attriInx);
			fprintf (stderr, "genQO->sqlResult[%d].len= %d\n", i,genQueryOut->sqlResult[i].len);
			fprintf (stderr, "genQO->sqlResult[%d].value= %s\n", i,genQueryOut->sqlResult[i].value);
		}

		rodsLog (LOG_ERROR, "4.5 stuff: ");
*/

		dataName = getSqlResultByInx (genQueryOut, COL_DATA_NAME);
		dataType = getSqlResultByInx (genQueryOut, COL_DATA_TYPE_NAME);
		for (i=0; i<genQueryOut->rowCnt; i++) {
			// fprintf (stderr, "dataName[%d]:%s\n",i,&dataName->value[dataName->len * i]);
			key = strdup (&dataName->value[dataName->len *i]);
			// fprintf (stderr, "dataSize[%d]:%s\n",i,&dataSize->value[dataSize->len * i]);
			value = strdup (&dataType->value[dataType->len * i]);
			addKeyVal (results, key, value);
		}

		// rodsLog (LOG_ERROR, "4.8 stuff: ");

	} else {
		fillIntInMsParam (mPout1, rei->status);
		return (rei->status);  //ack this is ugly
	}

	rodsLog (LOG_ERROR, "6 stuff: ");
	printGenQueryOut(stderr, NULL, NULL, genQueryOut);

	fillMsParam (mPin1, NULL, KeyValPair_MS_T, results, NULL);
	//rodsLog (LOG_ERROR, "7 s tuff: ");
	fillIntInMsParam (mPout1, rei->status);
	//rodsLog (LOG_ERROR, "8 s tuff: ");
  
	return(rei->status);

}

/* Silly hello world microservice */
int msiHiThere (ruleExecInfo_t *rei) {

	int i;

	RE_TEST_MACRO ("    Calling msiHiThere")

	i = hithere ();
	return(i);
}

