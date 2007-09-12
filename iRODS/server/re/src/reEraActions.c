/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "reEraActions.h"
#include "reEraHelpers.h"
#include "apiHeaderAll.h"

#define BIG_STR 200


/*
 * Copies metadata AVUs from one iRODS object to another one.
 * Both the source and destination object can be either a file,
 * a collection, a user or a resource independently of each other.
 * inpParam1 is treated as the destination object.
 * inpParam2 is treated as the source object.
 *
 */
int
msiCopyAVUMetadata(msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, ruleExecInfo_t *rei)
{
	rsComm_t *rsComm;

	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiCopyAVUMetadata")


	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiCopyAVUMetadata: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}
	
	rsComm = rei->rsComm;


	/* Check for proper input */
	if ((parseMspForStr(inpParam1) == NULL)) {
		rodsLog (LOG_ERROR, "msiCopyAVUMetadata: input inpParam1 is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	if ((parseMspForStr (inpParam2) == NULL)) {
		rodsLog (LOG_ERROR, "msiCopyAVUMetadata: input inpParam2 is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}


	/* Call copyAVUMetadata() */
	rei->status = copyAVUMetadata(inpParam1->inOutStruct, inpParam2->inOutStruct, rsComm);

	return (rei->status);
}



/*
 * Writes the contents of inpParam2->inpOutBuf->buf to a local file.
 * The local file is opened for appending.
 */
int
msiSaveBufferToLocalFile(msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, ruleExecInfo_t *rei)
{
	rsComm_t *rsComm;
	char *localPath;
	FILE *destFile;
	int myInt;

	RE_TEST_MACRO ("    Calling msiSaveBufferToLocalFile")

	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiSaveBufferToLocalFile: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;

	/* Sanity check for inpParam1 (string) */
	if ((localPath = parseMspForStr (inpParam1)) == NULL) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			"msiSaveBufferToLocalFile: parseMspForStr error for param 1.");
		return (rei->status);
	}

	/* Sanity check for inParam2 (buffer) */
	if (inpParam2 != NULL) {
		myInt = parseMspForPosInt (inpParam2);
		if ((myInt < 0)  && (myInt != SYS_NULL_INPUT)) {
			rei->status = myInt;
			rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
				"msiSaveBufferToLocalFile: parseMspForPosInt error for param 2.");
			return (rei->status);
		}
	}

	/* Write to local file */
	if ((destFile = fopen(localPath, "a")) == NULL) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			"msiSaveBufferToLocalFile: Unable to open local file %s for appending.", localPath);
		return (rei->status);
	}

	rei->status = fprintf(destFile, "%s", inpParam2->inpOutBuf->buf);
	fclose(destFile);

	return (rei->status);
}



/*
 * Recursively exports user defined metadata attributes (AVUs) for
 * a collection and all collections and data objects in this collection.
 * The output is written to a bytesBuf_t buffer in pipe separated format, one
 * line per attribute as in the example below:
 * /tempZone/home/testuser/myFile|myAttr|myVal|myUnits
 */
int
msiExportRecursiveCollMeta(msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei)
{
	collInp_t collInpCache, *outCollInp;
	genQueryInp_t genQueryInp;
	genQueryOut_t *genQueryOut;
	char collQCond[MAX_NAME_LEN];
	char objPath[MAX_NAME_LEN];
	rsComm_t *rsComm;
	bytesBuf_t *mybuf;
	char *tResult;	
	int i, j;	
	
	
	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiExportRecursiveCollMeta")
	
	rsComm = rei->rsComm;

	
	/* parse inpParam1 */
	rei->status = parseMspForCollInp (inpParam, &collInpCache, &outCollInp, 0);
	
	if (rei->status < 0) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
				    "msiExportRecursiveCollMeta: input inpParam1 error. status = %d", rei->status);
		return (rei->status);
	}
	
	
	/* Make sure input is a collection */
	if (isColl(rei->rsComm, outCollInp->collName, NULL) < 0) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
				    "msiExportRecursiveCollMeta: Invalid input in inpParam1: %s. No such collection.", outCollInp->collName);
		return (rei->status);
	}	
	
	
	/* Allocate 1KB for our buffer */
	mybuf=(bytesBuf_t *)malloc(sizeof(bytesBuf_t));
	mybuf->len=1024;
	mybuf->buf=(char *)malloc(mybuf->len);
	memset(mybuf->buf, '\0', mybuf->len);
	
	
	/* Get all collections (recursively) under our input collection */
	/* Prepare query */
	memset (&genQueryInp, 0, sizeof (genQueryInp_t));
	genAllInCollQCond (outCollInp->collName, collQCond);
	
	addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, collQCond);
	addInxIval (&genQueryInp.selectInp, COL_COLL_NAME, 1);
	
	genQueryInp.maxRows = MAX_SQL_ROWS;
	
	/* ICAT query for subcollections */
	rei->status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);	

	if (rei->status != CAT_NO_ROWS_FOUND) {
		
		for (i=0;i<genQueryOut->rowCnt;i++) {
			
			for (j=0;j<genQueryOut->attriCnt;j++) {
				tResult = genQueryOut->sqlResult[j].value;
				tResult += i*genQueryOut->sqlResult[j].len;
				
				/* get metadata for this collection */
				getCollectionPSmeta(tResult, mybuf, rsComm);
			}
		}

		while (rei->status==0 && genQueryOut->continueInx > 0) {
			genQueryInp.continueInx=genQueryOut->continueInx;
			rei->status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);

			for (i=0;i<genQueryOut->rowCnt;i++) {
			
				for (j=0;j<genQueryOut->attriCnt;j++) {
					tResult = genQueryOut->sqlResult[j].value;
					tResult += i*genQueryOut->sqlResult[j].len;
					
					/* get metadata for this collection */
					getCollectionPSmeta(tResult, mybuf, rsComm);
				}
			}
		}
	}

	
	/* Same thing now for all files (recursively) under our input collection */
	/* Prepare query */
	memset (&genQueryInp, 0, sizeof (genQueryInp_t));
	genAllInCollQCond (outCollInp->collName, collQCond);
	
	addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, collQCond);
	addInxIval (&genQueryInp.selectInp, COL_COLL_NAME, 1);
	addInxIval (&genQueryInp.selectInp, COL_DATA_NAME, 1);
	
	genQueryInp.maxRows = MAX_SQL_ROWS;	
	
	/* ICAT query for data objects */
	rei->status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);	

	if (rei->status != CAT_NO_ROWS_FOUND) {
		
		for (i=0;i<genQueryOut->rowCnt;i++) {

			/* First attribute is parent collection */
			tResult = genQueryOut->sqlResult[0].value;
			tResult += i*genQueryOut->sqlResult[0].len;
			snprintf(objPath, MAX_NAME_LEN, "%s/", tResult);
			
			/* Second attribute is the filename */
			tResult = genQueryOut->sqlResult[1].value;
			tResult += i*genQueryOut->sqlResult[1].len;
			strcat(objPath, tResult);
			
			/* Get metadata for this file */
			getDataObjPSmeta(objPath, mybuf, rsComm);
		}

		while (rei->status==0 && genQueryOut->continueInx > 0) {
			genQueryInp.continueInx=genQueryOut->continueInx;
			rei->status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);

			for (i=0;i<genQueryOut->rowCnt;i++) {
			
				/* First attribute is parent collection */
				tResult = genQueryOut->sqlResult[0].value;
				tResult += i*genQueryOut->sqlResult[0].len;
				snprintf(objPath, MAX_NAME_LEN, "%s/", tResult);
			
				/* Second attribute is the filename */
				tResult = genQueryOut->sqlResult[1].value;
				tResult += i*genQueryOut->sqlResult[1].len;
				strcat(objPath, tResult);
			
				/* Get metadata for this file */
				getDataObjPSmeta(objPath, mybuf, rsComm);
			}
		}
	}
	
	/* send results out to outParam */
	fillBufLenInMsParam (outParam, strlen(mybuf->buf), mybuf);

	return 0;
}



/*
 * Gets metadata AVUs for a given file.
 * Writes results to outParam in XML format.
 *
 */
int
msiGetDataObjAVUs(msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei)
{
   rsComm_t *rsComm; 
   dataObjInp_t dataObjInp, *myDataObjInp;
   bytesBuf_t *mybuf;

   genQueryInp_t genQueryInp;
   genQueryOut_t *genQueryOut;
   int i1a[10];
   int i1b[10];
   int i2a[10];
   char *condVal[10];
   char v1[BIG_STR];
   char v2[BIG_STR];
   char fullName[MAX_NAME_LEN];
   char myDirName[MAX_NAME_LEN];
   char myFileName[MAX_NAME_LEN];
   int printCount=0;

   char *tags[]={"AVU", "attribute", "value", "units"};


   RE_TEST_MACRO ("    Calling msiGetDataObjAVUs")

   if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiGetDataObjAVUs: input rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
   }

   rsComm = rei->rsComm;

   /* parse inpParam1 */
   rei->status = parseMspForDataObjInp (inpParam, &dataObjInp, &myDataObjInp, 0);

   if (rei->status < 0) {
	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiGetDataObjAVUs: input inpParam1 error. status = %d", rei->status);
	return (rei->status);
   }


   memset (&genQueryInp, 0, sizeof (genQueryInp_t));

   i1a[0]=COL_META_DATA_ATTR_NAME;
   i1b[0]=0; /* currently unused */
   i1a[1]=COL_META_DATA_ATTR_VALUE;
   i1b[1]=0; /* currently unused */
   i1a[2]=COL_META_DATA_ATTR_UNITS;
   i1b[2]=0; /* currently unused */
   genQueryInp.selectInp.inx = i1a;
   genQueryInp.selectInp.value = i1b;
   genQueryInp.selectInp.len = 3;

   /* Extract cwd name and object name */
   strncpy(fullName, myDataObjInp->objPath, MAX_NAME_LEN);
   rei->status = splitPathByKey(fullName, myDirName, myFileName, '/');

   i2a[0]=COL_COLL_NAME;
   sprintf(v1,"='%s'",myDirName);
   condVal[0]=v1;

   i2a[1]=COL_DATA_NAME;
   sprintf(v2,"='%s'",myFileName);
   condVal[1]=v2;


   genQueryInp.sqlCondInp.inx = i2a;
   genQueryInp.sqlCondInp.value = condVal;
   genQueryInp.sqlCondInp.len=2;

   genQueryInp.maxRows=10;
   genQueryInp.continueInx=0;
   genQueryInp.condInput.len=0;


   /* Actual query happens here */
   rei->status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);

   /* Allocate 1KB for our buffer */
   mybuf=(bytesBuf_t *)malloc(sizeof(bytesBuf_t));
   mybuf->len=1024;
   mybuf->buf=(char *)malloc(mybuf->len);
   memset(mybuf->buf, '\0', mybuf->len);


   /* Prepare XML output */
   appendStrToBBuf(mybuf, 50, "<?xml version='1.0' encoding='utf-8'?>\n");
   appendStrToBBuf(mybuf, 70, "<!-- Generated by SDSC iRODS (http://www.irods.org) -->\n");
   appendStrToBBuf(mybuf, 20, "<metadata>\n");


   if (rei->status == CAT_NO_ROWS_FOUND) {
      i1a[0]=COL_D_DATA_PATH;
      genQueryInp.selectInp.len = 1;
      rei->status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
      if (rei->status==0) {
	 printf("None\n");
	 return(0);
      }
      if (rei->status == CAT_NO_ROWS_FOUND) {

	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiGetDataObjAVUs: DataObject %s not found. status = %d", fullName, rei->status);
	return (rei->status);
      }
      printCount+=genQueryOutToXML(rsComm, rei->status, genQueryOut, mybuf, tags);
   }
   else {
      printCount+=genQueryOutToXML(rsComm, rei->status, genQueryOut, mybuf, tags);
   }

   while (rei->status==0 && genQueryOut->continueInx > 0) {
      genQueryInp.continueInx=genQueryOut->continueInx;
      rei->status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
      printCount+= genQueryOutToXML(rsComm, rei->status, genQueryOut, mybuf, tags);
   }

  /* Closing tag */
  appendStrToBBuf(mybuf, 20, "</metadata>\n");


  fillBufLenInMsParam (outParam, strlen(mybuf->buf), mybuf);

  return (rei->status);

}



/*
 * Gets metadata AVUs for a data object (file).
 * Writes results to outParam in pipe-separated (Chien-Yi's) format.
 *
 */
int
msiGetDataObjPSmeta(msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei)
{
	rsComm_t *rsComm; 
	dataObjInp_t dataObjInp, *myDataObjInp;
	bytesBuf_t *mybuf;
	
	
	RE_TEST_MACRO ("    Calling msiGetDataObjPSmeta")
	
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR,
			"msiGetDataObjPSmeta: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}
	
	rsComm = rei->rsComm;
	
	
	/* parse inpParam1 */
	rei->status = parseMspForDataObjInp (inpParam, &dataObjInp, &myDataObjInp, 0);
	
	if (rei->status < 0) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			"msiGetDataObjPSmeta: input inpParam1 error. status = %d", rei->status);
		return (rei->status);
	}

	
	/* Allocate 1KB for our buffer */
	mybuf=(bytesBuf_t *)malloc(sizeof(bytesBuf_t));
	mybuf->len=1024;
	mybuf->buf=(char *)malloc(mybuf->len);
	memset(mybuf->buf, '\0', mybuf->len);

	
	/* call getDataObjPSmeta() */
	getDataObjPSmeta(myDataObjInp->objPath, mybuf, rsComm);
	

	/* send results out to outParam */
	fillBufLenInMsParam (outParam, strlen(mybuf->buf), mybuf);
	
	return (rei->status);

}


/*
 * Gets metadata AVUs for a collection.
 * Writes results to outParam in pipe-separated (Chien-Yi's) format.
 */
int
msiGetCollectionPSmeta(msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei)
{
	rsComm_t *rsComm; 
	collInp_t collInpCache, *outCollInp;
	bytesBuf_t *mybuf;
	
	
	RE_TEST_MACRO ("    Calling msiGetCollectionPSmeta")
	
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR,
			 "msiGetCollectionPSmeta: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}
	
	rsComm = rei->rsComm;
	
	
	/* parse inpParam1 */
	rei->status = parseMspForCollInp (inpParam, &collInpCache, &outCollInp, 0);	

	if (rei->status < 0) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			"msiGetCollectionPSmeta: input inpParam1 error. status = %d", rei->status);
		return (rei->status);
	}


	/* Allocate 1KB for our buffer */
	mybuf=(bytesBuf_t *)malloc(sizeof(bytesBuf_t));
	mybuf->len=1024;
	mybuf->buf=(char *)malloc(mybuf->len);
	memset(mybuf->buf, '\0', mybuf->len);


	/* call getCollectionPSmeta() */
	getCollectionPSmeta(outCollInp->collName, mybuf, rsComm);


	/* send results out to outParam */
	fillBufLenInMsParam (outParam, strlen(mybuf->buf), mybuf);

	return (rei->status);

}



/*
 * msiLoadMetadataFromFile() - Looks for an .mdf file in iRODS, parses it
 * and adds metadata to files accordingly
 */
int
msiLoadMetadataFromFile(msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei)
{
	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiLoadMetadataFromFile")


	/* Sanity checks */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiLoadMetadataFromFile: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	if ((parseMspForStr(inpParam) == NULL)) {
		rodsLog (LOG_ERROR, "msiLoadMetadataFromFile: input inpParam is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}


	/* Call loadMetadataFromFile() */
	loadMetadataFromFile(rei->rsComm, inpParam->inOutStruct);

	return 0;
}


/*
 * msiGetDataObjAIP() - Gets the Archival Information Package of a given file.
 * Writes results to outParam in XML format. This may later
 * be broken down to separate microservices: one for
 * getting system metadata and the other one for user defined MD.
 */
int
msiGetDataObjAIP(msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei)
{
   rsComm_t *rsComm; 
   dataObjInp_t dataObjInp, *myDataObjInp;
   bytesBuf_t *mybuf;

   genQueryInp_t genQueryInp;
   genQueryOut_t *genQueryOut;
   int i1a[30];
   int i1b[30];
   int i2a[30];
   char *condVal[30];
   char v1[BIG_STR];
   char v2[BIG_STR];
   char fullName[MAX_NAME_LEN];
   char myDirName[MAX_NAME_LEN];
   char myFileName[MAX_NAME_LEN];
   int printCount=0, i;
   char *tags[30];

   char tmpStr[MAX_NAME_LEN];
   rodsObjStat_t *rodsObjStatOut;

   RE_TEST_MACRO ("    Calling GetDataObjAIP")

   if (rei == NULL || rei->rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "msiGetDataObjAIP: input rei or rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
   }

   rsComm = rei->rsComm;

   /* parse inpParam1 */
   rei->status = parseMspForDataObjInp (inpParam, &dataObjInp, &myDataObjInp, 0);

   if (rei->status < 0) {
	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiGetDataObjAIP: input inpParam1 error. status = %d", rei->status);
	return (rei->status);
   }

   /* Extract cwd name and object name */
   strncpy(fullName, myDataObjInp->objPath, MAX_NAME_LEN);
   rei->status = splitPathByKey(fullName, myDirName, myFileName, '/');


   /* Allocate 1KB for our buffer */
   mybuf=(bytesBuf_t *)malloc(sizeof(bytesBuf_t));
   mybuf->len=1024;
   mybuf->buf=(char *)malloc(mybuf->len);
   memset(mybuf->buf, '\0', mybuf->len);


   /* Prepare XML output */
   appendStrToBBuf(mybuf, 50, "<?xml version='1.0' encoding='utf-8'?>\n");
   appendStrToBBuf(mybuf, 70, "<!-- Generated by SDSC iRODS (http://www.irods.org) -->\n");
   appendStrToBBuf(mybuf, 20, "<AIP>\n");


   /* First we're going to query the ICAT for system metadata */
   tags[0]=strdup("");
   tags[1]=strdup("D_DATA_ID");
   tags[2]=strdup("D_COLL_ID");
   tags[3]=strdup("DATA_NAME");
   tags[4]=strdup("DATA_REPL_NUM");
   tags[5]=strdup("DATA_VERSION");
   tags[6]=strdup("DATA_TYPE_NAME");
   tags[7]=strdup("DATA_SIZE");
   tags[8]=strdup("D_RESC_GROUP_NAME");
   tags[9]=strdup("D_RESC_NAME");
   tags[10]=strdup("D_DATA_PATH");
   tags[11]=strdup("D_OWNER_NAME");
   tags[12]=strdup("D_OWNER_ZONE");
   tags[13]=strdup("D_REPL_STATUS");
   tags[14]=strdup("D_DATA_STATUS");
   tags[15]=strdup("D_DATA_CHECKSUM");
   tags[16]=strdup("D_EXPIRY");
   tags[17]=strdup("D_MAP_ID");
   tags[18]=strdup("D_COMMENTS");
   tags[19]=strdup("D_CREATE_TIME");
   tags[20]=strdup("D_MODIFY_TIME");


   memset (&genQueryInp, 0, sizeof (genQueryInp_t));

   i1a[0]=COL_D_DATA_ID;
   i1a[1]=COL_D_COLL_ID;
   i1a[2]=COL_DATA_NAME;
   i1a[3]=COL_DATA_REPL_NUM;
   i1a[4]=COL_DATA_VERSION;
   i1a[5]=COL_DATA_TYPE_NAME;
   i1a[6]=COL_DATA_SIZE;
   i1a[7]=COL_D_RESC_GROUP_NAME;
   i1a[8]=COL_D_RESC_NAME;
   i1a[9]=COL_D_DATA_PATH;
   i1a[10]=COL_D_OWNER_NAME;
   i1a[11]=COL_D_OWNER_ZONE;
   i1a[12]=COL_D_REPL_STATUS;
   i1a[13]=COL_D_DATA_STATUS;
   i1a[14]=COL_D_DATA_CHECKSUM;
   i1a[15]=COL_D_EXPIRY;
   i1a[16]=COL_D_MAP_ID;
   i1a[17]=COL_D_COMMENTS;
   i1a[18]=COL_D_CREATE_TIME;
   i1a[19]=COL_D_MODIFY_TIME;

   for (i=0; i<20; i++) {
   i1b[i]=0; /* currently unused */
   }

   genQueryInp.selectInp.inx = i1a;
   genQueryInp.selectInp.value = i1b;
   genQueryInp.selectInp.len = 20;


   i2a[0]=COL_COLL_NAME;
   sprintf(v1,"='%s'",myDirName);
   condVal[0]=v1;

   i2a[1]=COL_DATA_NAME;
   sprintf(v2,"='%s'",myFileName);
   condVal[1]=v2;

   genQueryInp.sqlCondInp.inx = i2a;
   genQueryInp.sqlCondInp.value = condVal;
   genQueryInp.sqlCondInp.len=2;

   genQueryInp.maxRows=30;
   genQueryInp.continueInx=0;
   genQueryInp.condInput.len=0;


   /* First rsGenQuery() call for system metadata */
   rei->status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);




   /* Parse and print out results */
   if (rei->status == CAT_NO_ROWS_FOUND) {
      i1a[0]=COL_D_DATA_PATH;
      genQueryInp.selectInp.len = 1;
      rei->status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
      if (rei->status==0) {
	 printf("None\n");
	 return(0);
      }
      if (rei->status == CAT_NO_ROWS_FOUND) {

	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiGetDataObjAIP: DataObject %s not found. status = %d", fullName, rei->status);
	return (rei->status);
      }
      printCount+=genQueryOutToXML(rsComm, rei->status, genQueryOut, mybuf, tags);
   }
   else {
      printCount+=genQueryOutToXML(rsComm, rei->status, genQueryOut, mybuf, tags);
   }

   while (rei->status==0 && genQueryOut->continueInx > 0) {
      genQueryInp.continueInx=genQueryOut->continueInx;
      rei->status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
      printCount+= genQueryOutToXML(rsComm, rei->status, genQueryOut, mybuf, tags);
   }


  /* Now we're going to query the ICAT for user defined metadata. One hit = one AVU triplet */
   tags[0]=strdup("AVU");
   tags[1]=strdup("attribute");
   tags[2]=strdup("value");
   tags[3]=strdup("units");

   memset (&genQueryInp, 0, sizeof (genQueryInp_t));

   i1a[0]=COL_META_DATA_ATTR_NAME;
   i1b[0]=0; /* currently unused */
   i1a[1]=COL_META_DATA_ATTR_VALUE;
   i1b[1]=0; /* currently unused */
   i1a[2]=COL_META_DATA_ATTR_UNITS;
   i1b[2]=0; /* currently unused */
   genQueryInp.selectInp.inx = i1a;
   genQueryInp.selectInp.value = i1b;
   genQueryInp.selectInp.len = 3;

   /* Extract cwd name and object name */
   strncpy(fullName, myDataObjInp->objPath, MAX_NAME_LEN);
   rei->status = splitPathByKey(fullName, myDirName, myFileName, '/');

   i2a[0]=COL_COLL_NAME;
   sprintf(v1,"='%s'",myDirName);
   condVal[0]=v1;

   i2a[1]=COL_DATA_NAME;
   sprintf(v2,"='%s'",myFileName);
   condVal[1]=v2;


   genQueryInp.sqlCondInp.inx = i2a;
   genQueryInp.sqlCondInp.value = condVal;
   genQueryInp.sqlCondInp.len=2;

   genQueryInp.maxRows=10;
   genQueryInp.continueInx=0;
   genQueryInp.condInput.len=0;

   /* rsGenQuery() call for user defined metadata */
   rei->status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);

   /* Parse and print out results */
   if (rei->status == CAT_NO_ROWS_FOUND) {
      i1a[0]=COL_D_DATA_PATH;
      genQueryInp.selectInp.len = 1;
      rei->status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
      if (rei->status==0) {
	 printf("None\n");
	 return(0);
      }
      if (rei->status == CAT_NO_ROWS_FOUND) {

	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiGetDataObjAVUs: DataObject %s not found. status = %d", fullName, rei->status);
	return (rei->status);
      }
      printCount+=genQueryOutToXML(rsComm, rei->status, genQueryOut, mybuf, tags);
   }
   else {
      printCount+=genQueryOutToXML(rsComm, rei->status, genQueryOut, mybuf, tags);
   }

   while (rei->status==0 && genQueryOut->continueInx > 0) {
      genQueryInp.continueInx=genQueryOut->continueInx;
      rei->status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
      printCount+= genQueryOutToXML(rsComm, rei->status, genQueryOut, mybuf, tags);
   }

    /* Same thing again, but this time for user access */
    tags[0]=strdup("ACL");
    tags[1]=strdup("COL_DATA_ACCESS_TYPE");
    tags[2]=strdup("COL_DATA_ACCESS_NAME");
    tags[3]=strdup("COL_DATA_TOKEN_NAMESPACE");
    tags[4]=strdup("COL_DATA_ACCESS_USER_IDM");
    tags[5]=strdup("COL_DATA_ACCESS_DATA_ID");

    memset (&genQueryInp, 0, sizeof (genQueryInp_t));

    rei->status = rsObjStat(rsComm, &dataObjInp, &rodsObjStatOut);

    addInxIval (&genQueryInp.selectInp, COL_USER_NAME, 1);
    addInxIval (&genQueryInp.selectInp, COL_DATA_ACCESS_NAME, 1);

    snprintf (tmpStr, MAX_NAME_LEN, " = '%s'", rodsObjStatOut->dataId);

    addInxVal (&genQueryInp.sqlCondInp, COL_DATA_ACCESS_DATA_ID, tmpStr);

    snprintf (tmpStr, MAX_NAME_LEN, "='%s'", "access_type");

    /* Currently necessary since other namespaces exist in the token table */
    addInxVal (&genQueryInp.sqlCondInp, COL_DATA_TOKEN_NAMESPACE, tmpStr);

    genQueryInp.maxRows = MAX_SQL_ROWS;

    rei->status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);

   /* Parse and print out results */
   if (rei->status == CAT_NO_ROWS_FOUND) {
      i1a[0]=COL_D_DATA_PATH;
      genQueryInp.selectInp.len = 1;
      rei->status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
      if (rei->status==0) {
	 printf("None\n");
	 return(0);
      }
      if (rei->status == CAT_NO_ROWS_FOUND) {

	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiGetDataObjAIP: DataObject %s not found. status = %d", fullName, rei->status);
	return (rei->status);
      }
      printCount+=genQueryOutToXML(rsComm, rei->status, genQueryOut, mybuf, tags);
   }
   else {
      printCount+=genQueryOutToXML(rsComm, rei->status, genQueryOut, mybuf, tags);
   }

   while (rei->status==0 && genQueryOut->continueInx > 0) {
      genQueryInp.continueInx=genQueryOut->continueInx;
      rei->status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
      printCount+= genQueryOutToXML(rsComm, rei->status, genQueryOut, mybuf, tags);
   }


  /* Closing tag */
  appendStrToBBuf(mybuf, 20, "</AIP>\n");


  fillBufLenInMsParam (outParam, strlen(mybuf->buf), mybuf);

  return (rei->status);

}





