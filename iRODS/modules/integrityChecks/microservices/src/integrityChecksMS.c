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



/* Check and see if the owner is in a comma separated list */
int msiVerifyOwner (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPout1, msParam_t* mPout2, ruleExecInfo_t *rei) {

	genQueryInp_t genQueryInp;
	genQueryOut_t *genQueryOut = NULL;
	char condStr[MAX_NAME_LEN];
	rsComm_t *rsComm;
	char* collname;
	char* ownerlist;
	int i,j;
	sqlResult_t *dataName;
	sqlResult_t *dataOwner;
	char delims[]=",";
	char* word;
	char** olist=NULL;
	bytesBuf_t*	stuff=NULL;
	
	RE_TEST_MACRO ("    Calling msiVerifyOwner")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiVerifyOwner: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;

	memset (&genQueryInp, 0, sizeof(genQueryInp_t));
	genQueryInp.maxRows = MAX_SQL_ROWS;

   /* buffer init */
    stuff = (bytesBuf_t *)malloc(sizeof(bytesBuf_t));
    memset (stuff, 0, sizeof (bytesBuf_t));

	/* construct an SQL query from the parameter list */
	collname = (char*) strdup (mPin1->inOutStruct);

	/* this is the info we want returned from the query */
	addInxIval (&genQueryInp.selectInp, COL_DATA_NAME, 1);
	addInxIval (&genQueryInp.selectInp, COL_D_OWNER_NAME, 1);
	snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, condStr);

	rodsLog (LOG_ERROR, "msiVerifyOwner: got here 0");
	
	j = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

	rodsLog (LOG_ERROR, "msiVerifyOwner: got here 1");

	/* Now for each file retrieved in the query, determine if it's owner is in our list */

	if (j != CAT_NO_ROWS_FOUND) {

		printGenQueryOut(stderr, NULL, NULL, genQueryOut);

		dataName = getSqlResultByInx (genQueryOut, COL_DATA_NAME);
		dataOwner = getSqlResultByInx (genQueryOut, COL_D_OWNER_NAME);


		ownerlist = (char*) strdup (mPin2->inOutStruct);
		fprintf(stderr, "ownerlist: %s\n", ownerlist);

		if (strlen(ownerlist)>0) { /* our rule contains a list of owners we want to compare against */
			int ownercount=0;

			rodsLog (LOG_ERROR, "msiVerifyOwner: ownerlist!=NULL");

			/* Construct a list of owners*/
			for (word=strtok(ownerlist, delims); word; word=strtok(NULL, delims)) {
				olist = (char**) realloc (olist, sizeof (char*) * (ownercount));  
				olist[ownercount] = strdup (word);
				ownercount++;
			}

			/* Now compare each file's owner with our list */
			for (i=0; i<genQueryOut->rowCnt; i++) {
				int foundflag=0;
				for (j=0; j<ownercount; j++) {
					char* thisowner = strdup(&dataOwner->value[dataOwner->len*i]);

					fprintf(stderr, "comparing %s and %s\n", thisowner, olist[j]);
					if (!(strcmp(thisowner, olist[j]))) {
						/* We only care about the ones that DON'T match */
						foundflag=1;
						break;
					}
				}

				if (!foundflag) {
					char tmpstr[80];
					sprintf (tmpstr, "File: %s with owner: %s does not match input list\n", 
						&dataName->value[dataName->len *i], &dataOwner->value[dataOwner->len * i]);
					appendToByteBuf (stuff, tmpstr);
				}
			}
			appendToByteBuf(stuff,"here's an answer for ya");

		} else { /* input parameter for owner is not set */
			
			rodsLog (LOG_ERROR, "msiVerifyOwner: ownerlist is NULL");

			/* just check if the owner name (whatever it is) is consistent across all files */
			/* We'll just compare using the first file's owner - this can probably be done several ways */
			char* firstowner = strdup(&dataOwner->value[0]); 

			int matchflag=1;  /* Start off assuming all owners match */

			for (i=1; i<genQueryOut->rowCnt; i++) {
				char* thisowner = strdup( &dataOwner->value[dataOwner->len*i]);
				if (strcmp(firstowner, thisowner)) { /* the two strings are not equal */
					appendToByteBuf (stuff, "Owner is not consistent across this collection");
					rodsLog (LOG_ERROR, "got here 5");
					matchflag=0;
					break;
				}
			}

			appendToByteBuf (stuff,"let's get this party started\n");

			rodsLog (LOG_ERROR, "got here 6");
			if (matchflag) /* owner field was consistent across all data objects */
				appendToByteBuf (stuff, "Owner is consistent across this collection.\n");
			else
				appendToByteBuf (stuff, "Owner is not consistent across this collection.\n");
			rodsLog (LOG_ERROR, "got here 6.5");
		}	
	} 

	rodsLog (LOG_ERROR, "got here 7");
	fillBufLenInMsParam (mPout1, stuff->len, stuff);
	rodsLog (LOG_ERROR, "got here 8");
//	fillIntInMsParam (mPout1, rei->status);
//	rodsLog (LOG_ERROR, "got here 9");
  
	return(rei->status);

}

/* Performs two functions: if mPin2 is not NULL, then check and see if every file in the collection
matches the given ACL, if it is NULL, just see that all ACLs are identical */
int msiVerifyACL (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPin3, msParam_t* mPin4, 
	msParam_t* mPout1, msParam_t* mPout2, ruleExecInfo_t *rei) {

	genQueryInp_t genQueryInp;
	genQueryOut_t *genQueryOut = NULL;
	char condStr[MAX_NAME_LEN];
	rsComm_t *rsComm;
	char *collname, *aclname, *aclvalue, *aclattr;
	int i,j;
	sqlResult_t *dataName;
	sqlResult_t *dataMetaName;
	sqlResult_t *dataMetaValue;
	sqlResult_t *dataMetaUnits;

	bytesBuf_t*	mybuf;
	
	RE_TEST_MACRO ("    Calling msiVerifyACL")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiVerifyACL: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	/* initialize return buffer structure */
	mybuf = (bytesBuf_t*) malloc (sizeof(bytesBuf_t));
	memset (mybuf, 0, sizeof (bytesBuf_t));

	rsComm = rei->rsComm;

	/* construct an SQL query from the input parameter list */
	/* Some of these may or may not be NULL */
	collname = (char*) strdup (mPin1->inOutStruct);
	aclname = (char*) strdup (mPin2->inOutStruct);
	aclvalue = (char*) strdup (mPin3->inOutStruct);
	aclattr = (char*) strdup (mPin4->inOutStruct);

	memset (&genQueryInp, 0, sizeof(genQueryInp_t));
	genQueryInp.maxRows = MAX_SQL_ROWS;

	/* this is the info we want returned from the query */
	addInxIval (&genQueryInp.selectInp, COL_DATA_NAME, 1);
	addInxIval (&genQueryInp.selectInp, COL_META_DATA_ATTR_NAME, 1);
	addInxIval (&genQueryInp.selectInp, COL_META_DATA_ATTR_VALUE, 1);
	addInxIval (&genQueryInp.selectInp, COL_META_DATA_ATTR_UNITS, 1);

	snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, condStr); 

	j = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
	rodsLog (LOG_ERROR, "after query j=%d",j);

	if (j != CAT_NO_ROWS_FOUND) {
		printGenQueryOut(stderr, NULL, NULL, genQueryOut);

		/* pointers to returned columns */
		dataName = getSqlResultByInx (genQueryOut, COL_DATA_NAME);
		dataMetaName = getSqlResultByInx (genQueryOut, COL_META_DATA_ATTR_NAME);
		dataMetaValue = getSqlResultByInx (genQueryOut, COL_META_DATA_ATTR_VALUE);
		dataMetaUnits = getSqlResultByInx (genQueryOut, COL_META_DATA_ATTR_UNITS);

		for (i=0; i<genQueryOut->rowCnt; i++) {
			appendToByteBuf (mybuf, "data object: ");
			appendToByteBuf (mybuf, &dataName->value[dataName->len * i]);
			appendToByteBuf (mybuf, "\n");
			appendToByteBuf (mybuf, "data meta name: ");
			appendToByteBuf (mybuf, &dataMetaName->value[dataMetaName->len * i]);
			appendToByteBuf (mybuf, "\n");
			appendToByteBuf (mybuf, "data meta value: ");
			appendToByteBuf (mybuf, &dataMetaValue->value[dataMetaValue->len * i]);
			appendToByteBuf (mybuf, "\n");
			appendToByteBuf (mybuf, "data meta units: ");
			appendToByteBuf (mybuf, &dataMetaUnits->value[dataMetaUnits->len * i]);
			appendToByteBuf (mybuf, "\n");
			
		}

		/* fill in the return buffer */
		fillBufLenInMsParam (mPout1, mybuf->len, mybuf);

	} else {
		appendToByteBuf (mybuf, "No rows found\n");
	}

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

