#include "integrityChecksMS.h"
#include "icutils.h"


/**
 * \fn msiVerifyExpiry
 * \module	integrityChecks
 * \author	Susan Lindsey
 * \date	September 2008
 * \brief	Checks whether files in a collection have expired
 * \note	none 
 * \param[in]	 mPin1=string=collection name, mPin2=string=date, mPin3=string=type{EXPIRED|NOTEXPIRED}
 * \param[out]	 none
 * \DolVarDependence	none
 * \DolVarModified		none
 * \iCatAtrDependence	none
 * \iCatAttrModified	none
 * \sideeffect
 * \return integer
 * \retval rei->status
 * \bug no known bugs
**/
int msiVerifyExpiry (msParam_t *mPin1, msParam_t *mPin2, msParam_t* mPin3, msParam_t *mPout1, msParam_t* mPout2, ruleExecInfo_t *rei) {

	rsComm_t *rsComm;
	genQueryInp_t gqin;
	genQueryOut_t *gqout = NULL;
	char condStr[MAX_NAME_LEN];
	char tmpstr[MAX_NAME_LEN];
	char* collname;
	sqlResult_t *dataName;
	sqlResult_t *dataExpiry;
	bytesBuf_t*	mybuf=NULL;
	int i,j,status;

	char* inputtime;
	char* querytype;
	int	checkExpiredFlag=0;
	char inputtimestr[TIME_LEN];

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
    mybuf = (bytesBuf_t *) malloc (sizeof (bytesBuf_t));
    memset (mybuf, 0, sizeof (bytesBuf_t));
	gqout = (genQueryOut_t*) malloc (sizeof (genQueryOut_t));
	memset (gqout, 0, sizeof (genQueryOut_t));


	/* construct an SQL query from the parameter list */
	collname = (char*) strdup (mPin1->inOutStruct);
	inputtime = (char*) strdup (mPin2->inOutStruct);
	querytype = (char*) strdup (mPin3->inOutStruct);

	/* 
		We have a couple of rule query possibilities:
		1. If 'inputtime' is valid & querytype == EXPIRED then list files which have an expiration date equal or past the input time
		2. If 'inputtime' is valid & querytype == NOTEXPIRED then list files which have yet to expire
	*/

	/* gotta at least have a collection name input */
	if (collname==NULL) return (USER_PARAM_TYPE_ERR);
	
	/* convert inputtime to unixtime */
	/* SUSAN we should make an option that inputtime = "now" */
	rstrcpy (inputtimestr, inputtime, TIME_LEN);
	status = checkDateFormat (inputtimestr);
	if (status < 0) return (DATE_FORMAT_ERR);

	/* now figure out what kind of query to perform */
	if (!strcmp(querytype, "EXPIRED")) {
		checkExpiredFlag = 1;
	} else if (!strcmp(querytype, "NOTEXPIRED")) {
		checkExpiredFlag = 0;
	} else return (USER_PARAM_TYPE_ERR); 
	

	/* this is the info we want returned from the query */
	addInxIval (&gqin.selectInp, COL_DATA_NAME, 1);
	addInxIval (&gqin.selectInp, COL_D_EXPIRY , 1);
	snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&gqin.sqlCondInp, COL_COLL_NAME, condStr);

	j = rsGenQuery (rsComm, &gqin, &gqout);

	if (j<0) {

		appendToByteBuf (mybuf, "General Query was bad");

	} else if (j != CAT_NO_ROWS_FOUND) {

		int dataobjexpiry, inputtimeexpiry;

		dataName = getSqlResultByInx (gqout, COL_DATA_NAME);
		dataExpiry = getSqlResultByInx (gqout, COL_D_EXPIRY);
		inputtimeexpiry = atoi (inputtimestr);

		for (i=0; i<gqout->rowCnt; i++) {
			dataobjexpiry = atoi(&dataExpiry->value[dataExpiry->len*i]);

			/* check for expired files */
			if (checkExpiredFlag) { 
				if (dataobjexpiry < inputtimeexpiry)   {
					sprintf (tmpstr, "Data object:%s\twith Expiration date:%s has expired\n", 
						&dataName->value[dataName->len *i], &dataExpiry->value[dataExpiry->len *i]);
					appendToByteBuf (mybuf, tmpstr);
				}
			} else { 
				if (dataobjexpiry >= inputtimeexpiry) {
					sprintf (tmpstr, "Data object:%s\twith Expiration date:%s is expiring\n", 
						&dataName->value[dataName->len *i], &dataExpiry->value[dataExpiry->len *i]);
					appendToByteBuf (mybuf, tmpstr);
				}
			} 
				
		}
	} else appendToByteBuf (mybuf, "No rows found\n");

	fillBufLenInMsParam (mPout1, mybuf->len, mybuf);
	fillIntInMsParam (mPout2, rei->status);
  
	return(rei->status);

}

/**
 * \fn	msiCheckFilesizeRange
 * \module	integrityChecks
 * \author	Susan Lindsey
 * \date	December 2006
 * \brief	Check to see if file sizes are NOT within a certain range
 * \note
 * \param[in] mPin1=string=collection name, mPin2=lower limit on filesize, mPin3=upper limit on filesize
 * \param[out]
 * \DolVarDependence	none
 * \DolVarModified		none
 * \iCatAtrDependence	none
 * \iCatAttrModified	none
 * \sideeffect
 * \return	integer
 * \retval	0 on success
 * \bug		no known bugs
**/
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

	/* build the condition:
		collection name AND (filesize < minfilesize or filesize > maxfilesize) */

	snprintf (condStr, MAX_NAME_LEN, " < '%s' || > '%s'", minfilesize, maxfilesize);
	addInxVal (&genQueryInp.sqlCondInp, COL_DATA_SIZE, condStr); 
	snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, condStr); 

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

	} else {
		fillIntInMsParam (mPout1, rei->status);
		return (rei->status);  //ack this is ugly
	}

	//printGenQueryOut(stderr, NULL, NULL, genQueryOut);

	fillMsParam (mPin3, NULL, KeyValPair_MS_T, results, NULL);
	fillIntInMsParam (mPout1, rei->status);
  
	return(rei->status);

}


/* See if all files in a collection match a given AVU */
/**
 * \fn	msiVerifyAVU
 * \module	integrityChecks
 * \author	Susan Lindsey
 * \date	December 2006
 * \brief	Performs operations on the AVU metadata on files in a given collection
 * \note
 * \param[in]
 * \param[out]
 * \DolVarDependence	none
 * \DolVarModified		none
 * \iCatAtrDependence	none
 * \iCatAttrModified	none
 * \sideeffect
 * \return	integer
 * \retval	0 on success
 * \bug		no known bugs
**/
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

	/* init structs */
	gqin.maxRows = MAX_SQL_ROWS;
    mybuf = (bytesBuf_t *)malloc(sizeof(bytesBuf_t));
    memset (mybuf, 0, sizeof (bytesBuf_t));
	gqout1 = (genQueryOut_t*) malloc (sizeof (genQueryOut_t));
	memset (gqout1, 0, sizeof (genQueryOut_t));
	gqout2 = (genQueryOut_t*) malloc (sizeof (genQueryOut_t));
	memset (gqout2, 0, sizeof (genQueryOut_t));
 

	/* construct an SQL query from the parameter list */
	collname = (char*) strdup (mPin1->inOutStruct);

	/* Return just the name and id of all the files in the collection for our first query */
	addInxIval (&gqin.selectInp, COL_DATA_NAME, 1);
	addInxIval (&gqin.selectInp, COL_D_DATA_ID, 1);
	snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&gqin.sqlCondInp, COL_COLL_NAME, condStr);

	j = rsGenQuery (rsComm, &gqin, &gqout1);

	if (j<0)
		return (-1); /*RAJA badness what's the correct return error message */
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
/**
 * \fn	msiVerifyOwner
 * \module	integrityChecks
 * \author	Susan Lindsey
 * \date	December 2006
 * \brief	Check if files in a given collection have a consistent owner
 * \note
 * \param[in]
 * \param[out]
 * \DolVarDependence	none
 * \DolVarModified		none
 * \iCatAtrDependence	none
 * \iCatAttrModified	none
 * \sideeffect
 * \return	integer
 * \retval	0 on success
 * \bug		no known bugs
**/
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

	j = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

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
					matchflag=0;
					break;
				}
			}

			appendToByteBuf (stuff,"let's get this party started\n");

			if (matchflag) /* owner field was consistent across all data objects */
				appendToByteBuf (stuff, "Owner is consistent across this collection.\n");
			else
				appendToByteBuf (stuff, "Owner is not consistent across this collection.\n");
		}	
	} 

	fillBufLenInMsParam (mPout1, stuff->len, stuff);
	fillIntInMsParam (mPout1, rei->status);
  
	return(rei->status);

}


/**
 * \fn	msiCheckFileDatatypes
 * \module	integrityChecks
 * \author	Susan Lindsey
 * \date	December 2006
 * \brief	Check if files in a collection belong to a given data type
 * \note
 * \param[in]
 * \param[out]
 * \DolVarDependence	none
 * \DolVarModified		none
 * \iCatAtrDependence	none
 * \iCatAttrModified	none
 * \sideeffect
 * \return	integer
 * \retval	0 on success
 * \bug		no known bugs
**/
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
	

		j = rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

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

