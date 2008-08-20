#include "integrityChecksMS.h"
#include "icutils.h"


/* For now our second parameter is the string "now" and just sees if anything has expired */
int msiVerifyExpiry (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPout1, msParam_t* mPout2, ruleExecInfo_t *rei) {

	genQueryInp_t gqin;
	genQueryOut_t *gqout = NULL;
	char condStr[MAX_NAME_LEN];
	char tmpstr[MAX_NAME_LEN];
	rsComm_t *rsComm;
	char* collname;
	sqlResult_t *dataName;
	sqlResult_t *dataExpiry;
	bytesBuf_t*	mybuf=NULL;
	int i,j;

	char* timestr;
	char* inputtime;

	RE_TEST_MACRO ("    Calling msiVerifyExpiry")

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

	/* 
		First get the local time = now
		foreach file's expiry date	
			if irodsfileunixtime < localtime
				print to buf as an expired file
	*/

	/* construct an SQL query from the parameter list */
	collname = (char*) strdup (mPin1->inOutStruct);
	inputtime = (char*) strdup (mPin2->inOutStruct);

	/* this is the info we want returned from the query */
	addInxIval (&gqin.selectInp, COL_DATA_NAME, 1);
	addInxIval (&gqin.selectInp, COL_D_EXPIRY , 1);
	snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&gqin.sqlCondInp, COL_COLL_NAME, condStr);

	j = rsGenQuery (rsComm, &gqin, &gqout);

	if (j != CAT_NO_ROWS_FOUND) {

		printGenQueryOut(stderr, NULL, NULL, gqout);

		getNowStr (timestr);
		rodsLog (LOG_ERROR, "timestr=%s", timestr);

		dataName = getSqlResultByInx (gqout, COL_DATA_NAME);
		dataExpiry = getSqlResultByInx (gqout, COL_D_EXPIRY);

		rodsLog (LOG_ERROR, "got here 3 rowCnt=%d",gqout->rowCnt);

		for (i=0; i<gqout->rowCnt; i++) {
			int dataobjexpiry, inputtimeexpiry;
			dataobjexpiry = atoi(&dataExpiry->value[dataExpiry->len*i]);
			inputtimeexpiry = atoi (inputtime);
			rodsLog (LOG_ERROR, "Data object:%s\tExpiry:%s\n", &dataName->value[dataName->len *i], &dataExpiry->value[dataExpiry->len *i]);
			sprintf (tmpstr, "Data object:%s\tExpiry:%s\n", &dataName->value[dataName->len *i], &dataExpiry->value[dataExpiry->len *i]);
			appendToByteBuf (mybuf, tmpstr);
		}
	} else appendToByteBuf (mybuf, "No rows found\n");

		rodsLog (LOG_ERROR, "got here 6: mybuf->len:%d",mybuf->len);
	fillBufLenInMsParam (mPout1, mybuf->len, mybuf);
		rodsLog (LOG_ERROR, "got here 7");
	fillIntInMsParam (mPout2, rei->status);
		rodsLog (LOG_ERROR, "got here 8");
  
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


/* See if all files in a collection match a given AVU */
/* xxxxxxxxx */
int msiVerifyAVU (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPin3, msParam_t *mPin4, msParam_t *mPout1, msParam_t* mPout2, ruleExecInfo_t *rei) {

	genQueryInp_t gqin;;
	genQueryOut_t *gqout1 = NULL;
	genQueryOut_t *gqout2 = NULL;
	char condStr[MAX_NAME_LEN];
	char tmpstr[MAX_NAME_LEN];
	rsComm_t *rsComm;
	int i,j;
	sqlResult_t *dataName;
	sqlResult_t *dataID1; /* points to the data ID column in our first query */
	sqlResult_t *dataID2; /* points to the data ID column in our second query */
	sqlResult_t *dataAttrName;
	sqlResult_t *dataAttrValue;
	sqlResult_t *dataAttrUnits;
	bytesBuf_t*	mybuf=NULL;
	char* collname;
	char* inputattrname;
	char* inputattrvalue;
	char* inputattrunits;
	
	RE_TEST_MACRO ("    Calling msiVerifyAVU")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiVerifyAVU: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	/*
		Query 1 - just return the files in the collection
		Query 2 - return the files and their AVU triplet
	
		Note that Query 2 might be a subset of Query 1 since Query 2 will not return files that have no AVU listed.
		
		Foreach file in Query1 - see if the same file exists in Query2
			if so, see if the AVU triplet matches the input parameter
				if the AVU triplet doesn't match
					append error message to buf
			if not, append error message to buf
	*/

	rsComm = rei->rsComm;

	/* General Query Input init */
	memset (&gqin, 0, sizeof(genQueryInp_t));
	gqin.maxRows = MAX_SQL_ROWS;

   /* buffer init */
    mybuf = (bytesBuf_t *)malloc(sizeof(bytesBuf_t));
    memset (mybuf, 0, sizeof (bytesBuf_t));

	/* construct an SQL query from the parameter list */
	collname = (char*) strdup (mPin1->inOutStruct);

	/* Return just the name and id of all the files in the collection for our first query */
	addInxIval (&gqin.selectInp, COL_DATA_NAME, 1);
	addInxIval (&gqin.selectInp, COL_D_DATA_ID, 1);
	snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&gqin.sqlCondInp, COL_COLL_NAME, condStr);

	j = rsGenQuery (rsComm, &gqin, &gqout1);

	if (j != CAT_NO_ROWS_FOUND) {

		printGenQueryOut(stderr, NULL, NULL, gqout1);

		/* Now construct our second query, get the AVU information */
		clearGenQueryInp (&gqin);
		gqin.maxRows = MAX_SQL_ROWS;

		addInxIval (&gqin.selectInp, COL_DATA_NAME, 1);
		addInxIval (&gqin.selectInp, COL_D_DATA_ID, 1);
		addInxIval (&gqin.selectInp, COL_META_DATA_ATTR_NAME, 1);
		addInxIval (&gqin.selectInp, COL_META_DATA_ATTR_VALUE, 1);
		addInxIval (&gqin.selectInp, COL_META_DATA_ATTR_UNITS, 1);
		snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
		addInxVal (&gqin.sqlCondInp, COL_COLL_NAME, condStr);

		j = rsGenQuery (rsComm, &gqin, &gqout2);

		if (j != CAT_NO_ROWS_FOUND) { /* Second query results */

			int q1rows = gqout1->rowCnt;
			int q2rows = gqout2->rowCnt;
			int q1thisdataid, q2thisdataid;
			char* q1thisdataname;
			char* thisdataattrname;
			char* thisdataattrvalue;
			char* thisdataattrunits; 

			/* OK now we have the results of two queries - time to compare
				data objects and their AVU's */
			dataName = getSqlResultByInx (gqout1, COL_DATA_NAME);
			dataID1 = getSqlResultByInx (gqout1, COL_D_DATA_ID);  /* we'll use this field for indexing into the second query */
			dataID2 = getSqlResultByInx (gqout2, COL_D_DATA_ID);  
			dataAttrName = getSqlResultByInx (gqout2, COL_META_DATA_ATTR_NAME);  
			dataAttrValue = getSqlResultByInx (gqout2, COL_META_DATA_ATTR_VALUE);  
			dataAttrUnits = getSqlResultByInx (gqout2, COL_META_DATA_ATTR_UNITS);  

			/* Assigning the inputted AVU values */
			inputattrname = (char*) strdup (mPin2->inOutStruct); 
			inputattrvalue = (char*) strdup (mPin3->inOutStruct); 
			inputattrunits = (char*) strdup (mPin4->inOutStruct); 

			/* Ok now for each data object id returned in Query 1, see if there's a matching ID returned in Query 2 */
			for (i=0; i<q1rows; i++) {
				int avufoundflag=0;
				q1thisdataname = strdup((&dataName->value[dataName->len*i]));
				q1thisdataid = atoi(&dataID1->value[dataID1->len*i]);
				for (j=0; j<q2rows; j++) {
					q2thisdataid = atoi(&dataID2->value[dataID2->len*j]);
					if (q1thisdataid == q2thisdataid) { /* means this data object has an AVU triplet set*/
						avufoundflag=1;
						/* now see if the AVU matches our input */
						thisdataattrname = strdup((&dataAttrName->value[dataAttrName->len*j]));
						thisdataattrvalue = strdup((&dataAttrValue->value[dataAttrValue->len*j]));
						thisdataattrunits = strdup((&dataAttrUnits->value[dataAttrUnits->len*j]));
						if (strcmp(thisdataattrname, inputattrname)) { /* no match */
							sprintf (tmpstr, "Data object:%s with AVU:%s,%s,%s does not match input\n",
								q1thisdataname, thisdataattrname, thisdataattrvalue, thisdataattrunits);	
							appendToByteBuf (mybuf, tmpstr);
						}
						if (strcmp(thisdataattrvalue, inputattrvalue)) { /* no match */
							sprintf (tmpstr, "Data object:%s with AVU:%s,%s,%s does not match input\n",
								q1thisdataname, thisdataattrname, thisdataattrvalue, thisdataattrunits);	
							appendToByteBuf (mybuf, tmpstr);
						}
						if (strcmp(thisdataattrunits, inputattrunits)) { /* no match */
							sprintf (tmpstr, "Data object:%s with AVU:%s,%s,%s does not match input\n",
								q1thisdataname, thisdataattrname, thisdataattrvalue, thisdataattrunits);	
							appendToByteBuf (mybuf, tmpstr);
						}
						break;
					} 
					
				}
				if (!avufoundflag) { /* this data object has no AVU associated with it */
					sprintf (tmpstr, "Data object:%s has no AVU triplet set\n", q1thisdataname);	
					appendToByteBuf (mybuf, tmpstr);
				}
				
			}
			

		} else {
			/* Query 2 returned no results */
		}
	} else {
		/* Query 1 returned no results */
	}

	fillBufLenInMsParam (mPout1, mybuf->len, mybuf);
	fillIntInMsParam (mPout2, rei->status);
  
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

	fillBufLenInMsParam (mPout1, stuff->len, stuff);
	fillIntInMsParam (mPout1, rei->status);
  
	return(rei->status);

}


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
	fillIntInMsParam (mPout1, rei->status);
  
	return(rei->status);

}

