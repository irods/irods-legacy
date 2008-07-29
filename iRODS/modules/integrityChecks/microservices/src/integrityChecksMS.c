#include "rsApiHandler.h"
#include "integrityChecksMS.h"


int msiCheckFilesizeRange (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPin3, msParam_t *mPout1, ruleExecInfo_t *rei) {

	genQueryInp_t genQueryInp;
	genQueryOut_t *genQueryOut = NULL;
	char condStr[MAX_NAME_LEN];
	rsComm_t *rsComm;
	char collname[200];
	char maxfilesize[100]; 
	char minfilesize[100];
	bytesBuf_t	*mybuf;
	int i,j;
	sqlResult_t *dataName;
	sqlResult_t *dataSize;
	sqlResult_t *collName;

	sqlResult_t	*v[MAX_SQL_ATTR];
	char* colname[MAX_SQL_ATTR];
	keyValPair_t	*results;	
	char* key;
	char* value;
	

	char* attrs[]={"filename", "filesize", "collname"};

	RE_TEST_MACRO ("    Calling msiCheckCollectionFilesizeRange")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiCheckFilesizeRange: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;

	/* construct an SQL query from the parameter list */
	strcpy (collname,  (char*) mPin1->inOutStruct);
	strcpy (maxfilesize, (char*) mPin2->inOutStruct);
	strcpy (minfilesize, (char*) mPin3->inOutStruct);

	// initialize results to 0; AddKeyVal does all our malloc-ing
	results = (keyValPair_t*) malloc (sizeof(keyValPair_t));
	memset (results, 0, sizeof(keyValPair_t));

	memset (&genQueryInp, 0, sizeof(genQueryInp_t));
	genQueryInp.maxRows = MAX_SQL_ROWS;

	/* this is the info we want returned from the query */
	addInxIval (&genQueryInp.selectInp, COL_DATA_NAME, 1);
	addInxIval (&genQueryInp.selectInp, COL_DATA_SIZE, 1);
	addInxIval (&genQueryInp.selectInp, COL_COLL_NAME, 1);

	rodsLog (LOG_ERROR, "0 stuff: ");

	/* make the condition */
	snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, condStr); 
	snprintf (condStr, MAX_NAME_LEN, " < '%s'", minfilesize);
	addInxVal (&genQueryInp.sqlCondInp, COL_DATA_SIZE, condStr); 
	snprintf (condStr, MAX_NAME_LEN, " > '%s'", maxfilesize);
	addInxVal (&genQueryInp.sqlCondInp, COL_DATA_SIZE, condStr); 
	
	rodsLog (LOG_ERROR, "1 s tuff: ");

	fprintf (stderr,"\n");
	printGenQI(&genQueryInp);  
	fprintf (stderr,"\n");
	rodsLog (LOG_ERROR, "2 s tuff: ");

	j = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
	rodsLog (LOG_ERROR, "3 s tuff: j=%d",j);

	if (j != CAT_NO_ROWS_FOUND) {

		/* we got results - do something cool */
		/* just print out the GenQueryOut struct sheesh */
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

		dataName = getSqlResultByInx (genQueryOut, COL_DATA_NAME);
		dataSize = getSqlResultByInx (genQueryOut, COL_DATA_SIZE);
		for (i=0; i<genQueryOut->rowCnt; i++) {
			fprintf (stderr, "dataName[%d]:%s\n",i,&dataName->value[dataName->len * i]);
			key = strdup (&dataName->value[dataName->len *i]);
			fprintf (stderr, "dataSize[%d]:%s\n",i,&dataSize->value[dataSize->len * i]);
			value = strdup (&dataSize->value[dataSize->len * i]);
			addKeyVal (results, key, value);
		}

		rodsLog (LOG_ERROR, "4.8 stuff: ");

/*
		collName = getSqlResultByInx (genQueryOut, COL_COLL_NAME);
		for (i=0; i<genQueryOut->rowCnt; i++) {
			fprintf (stderr, "collName[%d]:%s\n",i,&collName->value[collName->len * i]);
		}
*/

	}

	rodsLog (LOG_ERROR, "6 stuff: ");
	printGenQueryOut(stderr, NULL, NULL, genQueryOut);

	/* fill in the return value struct - but how?*/
	// we'll use the KeyValPair_t struct - but is this optimal?
	// OK traverse the GenQueryOut struct again - this time I'm going to do it all in my head!
/*
	for (i=0; i<genQueryOut->attriCnt; i++) {
		v[i]=&genQueryOut->sqlResult[i];	
		cname[i] = getAttrNameFromAttrId(v[i]->attriInx);
	}

	for (i=0; i<genQueryOut->rowCnt; i++) {
		for (j=0; j<genQueryOut->attriCnt; j++) {
			fprintf (fd,"%s = %s\n", cname[j], &v[j]->value[v[j]->len * i]);
			key = strdup ();
		}
	}
*/

	/* print out KeyVal struct */
	rodsLog (LOG_ERROR, "6.5 stuff: ");
	fprintf (stderr, "results.len: %d\n", results->len);
	rodsLog (LOG_ERROR, "6.6 stuff: ");
	for (i=0; i<results->len; i++) {
		fprintf (stderr, "key: %s = value: %s\n", results->keyWord[i], results->value[i]);
	}

	fillMsParam (mPin3, NULL, KeyValPair_MS_T, results, NULL);
	rodsLog (LOG_ERROR, "7 s tuff: ");
	fillIntInMsParam (mPout1, rei->status);
	rodsLog (LOG_ERROR, "8 s tuff: ");
  
	return(rei->status);

}


int msiHiThere (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPout1, msParam_t *mPout2, ruleExecInfo_t *rei) {

  char *in1,  out1;
  int i, in2, out2;
   
  RE_TEST_MACRO ("    Calling susan's hithere")
  /* the above line is needed for loop back testing using irule -i option */
  
/*
  in1  = (char *) mPin1->inOutStruct;
  in2  = (int)    mPin2->inOutStruct;
  out1 = (char *) mPout1->inOutStruct;
  out2 = (int)    mPout2->inOutStruct;
*/
  
  i = hithere (in1, in2, out1, &out2);

	fillIntInMsParam (mPout2, i);

/*  mPout2->inOutStruct = (int) i; */


  
  return(i);
}

