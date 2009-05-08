/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "collInfoMS.h"
#include "eraUtil.h"



/**
 * \fn msiIsColl
 * \author  Antoine de Torcy
 * \date   2009-05-08
 * \brief Checks if an iRods path is a collection. For use in workflows.
 * \note This microservice takes an iRods path and returns the corresponding collection ID,
 *		or zero if the object is not a collection or does not exist.
 *		Avoid path names ending with '/' as they can be misparsed by lower level routines
 *		(eg: /tempZone/home instead of /tempZone/home/).
 * \param[in] 
 *    targetPath - A CollInp_MS_T or a STR_MS_T with the irods path.
 * \param[out] 
 *	  collId - an INT_MS_T containing the collection ID.
 *    status - an INT_MS_T containing the operation status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/
int
msiIsColl(msParam_t *targetPath, msParam_t *collId, msParam_t *status, ruleExecInfo_t *rei)
{
	collInp_t collInpCache, *collInp;				/* for parsing collection input param */
	rodsLong_t coll_id;								/* collection ID */
	
	
	
	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiIsColl")
	
	
	/* Sanity test */
	if (rei == NULL || rei->rsComm == NULL) {
			rodsLog (LOG_ERROR, "msiIsColl: input rei or rsComm is NULL.");
			return (SYS_INTERNAL_NULL_INPUT_ERR);
	}
	
	
	/* Parse collection input */
	rei->status = parseMspForCollInp (targetPath, &collInpCache, &collInp, 0);
	if (rei->status < 0) {
		rodsLog (LOG_ERROR, "msiIsColl: input targetPath error. status = %d", rei->status);
		return (rei->status);
	}


	/* Call isColl()*/
	rei->status = isColl (rei->rsComm, collInp->collName, &coll_id);
	
    
    /* Return 0 if no object was found */
    if (rei->status == CAT_NO_ROWS_FOUND)
    {
    	coll_id = 0;
    	rei->status = 0;
    }

	/* Return collection ID and operation status */
	fillIntInMsParam (collId, (int)coll_id);
	fillIntInMsParam (status, rei->status);

	/* Done */
	return rei->status;
}



/**
 * \fn msiGetCollectionContentsReport
 * \author  Antoine de Torcy
 * \date   2008-03-17
 * \brief Returns the number of objects in a collection by data type
 * \note This microservice returns the number of objects for each known data type in a collection, recursively.
 *	The results are written to a KeyValPair_MS_T, with a keyword for each data type plus "unknown".
 * \param[in] 
 *    inpParam1 - A CollInp_MS_T or a STR_MS_T with the irods path of the target collection.
 * \param[out] 
 *    inpParam2 (poorly named...) - A KeyValPair_MS_T containing the results.
 *    outParam - an INT_MS_T containing the status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/
int
msiGetCollectionContentsReport(msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, ruleExecInfo_t *rei)
{
	collInp_t collInpCache, *outCollInp;	/* for parsing collection input param */

	keyValPair_t *contents;			/* for passing out results */

	char collQCond[2*MAX_NAME_LEN];		/* for condition in rsGenquery() */
	genQueryInp_t genQueryInp;		/* for query inputs */
	genQueryOut_t *genQueryOut;		/* for query results */

	char *resultStringToken;		/* for parsing key-value pairs from genQueryOut */
	char *oldValStr, newValStr[21];		/* for parsing key-value pairs from genQueryOut */
	rodsLong_t newVal;			/* for parsing key-value pairs from genQueryOut */
	sqlResult_t *sqlResult;			/* for parsing key-value pairs from genQueryOut */
	int i;					/* for parsing key-value pairs from genQueryOut */


	RE_TEST_MACRO ("    Calling msiGetCollectionContentsReport")
	
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiGetCollectionContentsReport: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}


	/* parse inpParam1: our target collection */
	rei->status = parseMspForCollInp (inpParam1, &collInpCache, &outCollInp, 0);
	
	if (rei->status < 0) {
		rodsLog (LOG_ERROR, "msiGetCollectionContentsReport: input inpParam1 error. status = %d", rei->status);
		return (rei->status);
	}


	/* allocate memory for our result struct */
	contents = (keyValPair_t*)malloc(sizeof(keyValPair_t));
	memset (contents, 0, sizeof (keyValPair_t));


	/* Wanted fields. We use coll_id to do a join query on r_data_main and r_coll_main */
	memset (&genQueryInp, 0, sizeof (genQueryInp));
	addInxIval (&genQueryInp.selectInp, COL_DATA_TYPE_NAME, 1);
	addInxIval (&genQueryInp.selectInp, COL_D_DATA_ID, 1);
	addInxIval (&genQueryInp.selectInp, COL_COLL_ID, 1);


	/* Make condition for getting all objects under a collection */
	genAllInCollQCond (outCollInp->collName, collQCond);
	addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, collQCond);
	genQueryInp.maxRows = MAX_SQL_ROWS;
	/* genQueryInp.options = RETURN_TOTAL_ROW_COUNT; */


	/* Query */
	rei->status  = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);


	/* Parse results */
	if (rei->status == 0) {

		/* for each row */
		for (i=0;i<genQueryOut->rowCnt;i++) {
	
			/* get COL_DATA_TYPE_NAME result */
			sqlResult = getSqlResultByInx (genQueryOut, COL_DATA_TYPE_NAME);

			/* retrieve value for this row */
			resultStringToken = sqlResult->value + i*sqlResult->len;

			/* have we found this data type before? */
			oldValStr = getValByKey (contents, resultStringToken);
			if (oldValStr) {
				newVal = atoll(oldValStr) + 1;
			}
			else {
				newVal = 1;
			}
		
			/* add data type name along with its total number of occurrences */
			snprintf(newValStr, 21, "%lld", newVal);
			addKeyVal(contents, resultStringToken, newValStr);
		}

		/* not done? */
		while (rei->status==0 && genQueryOut->continueInx > 0) {
			genQueryInp.continueInx=genQueryOut->continueInx;
			rei->status = rsGenQuery(rei->rsComm, &genQueryInp, &genQueryOut);

			/* for each row */
			for (i=0;i<genQueryOut->rowCnt;i++) {
		
				/* get COL_DATA_TYPE_NAME result */
				sqlResult = getSqlResultByInx (genQueryOut, COL_DATA_TYPE_NAME);
	
				/* retrieve value for this row */
				resultStringToken = sqlResult->value + i*sqlResult->len;
	
				/* have we found this data type before? */
				oldValStr = getValByKey (contents, resultStringToken);
				if (oldValStr) {
					newVal = atoll(oldValStr) + 1;
				}
				else {
					newVal = 1;
				}
			
				/* add data type name along with its total number of occurrences */
				snprintf(newValStr, 21, "%lld", newVal);
				addKeyVal(contents, resultStringToken, newValStr);
			}
		}
	}


	/* send results out to inpParam2 */
	fillMsParam (inpParam2, NULL, KeyValPair_MS_T, contents, NULL);


	/* Return operation status through outParam */
	fillIntInMsParam (outParam, rei->status);
	/* fillIntInMsParam (outParam, genQueryOut->totalRowCount); */

	return (rei->status);
}



/**
 * \fn msiGetCollectionSize
 * \author  Antoine de Torcy
 * \date   2008-10-31
 * \brief Returns the object count and total disk usage of a collection
 * \note This microservice returns the object count and total disk usage for all objects in a collection, recursively.
 *	The results are written to a KeyValPair_MS_T whose keyword strings are "Size" and "Object Count".
 *	Might be merged with msiGetCollectionContentsReport()...
 * \param[in] 
 *    collPath - A CollInp_MS_T or a STR_MS_T with the irods path of the target collection.
 * \param[out] 
 *    outKVPairs - A KeyValPair_MS_T containing the results.
 *    status - an INT_MS_T containing the status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/
int
msiGetCollectionSize(msParam_t *collPath, msParam_t *outKVPairs, msParam_t *status, ruleExecInfo_t *rei)
{
	collInp_t collInpCache, *outCollInp;	/* for parsing collection input param */

	keyValPair_t *res;			/* for passing out results */

	char collQCond[2*MAX_NAME_LEN];		/* for condition in rsGenquery() */
	genQueryInp_t genQueryInp;		/* for query inputs */
	genQueryOut_t *genQueryOut;		/* for query results */

	rodsLong_t size, objCount;		/* to store total size and file count */
	char tmpStr[21];			/* to store total size and file count */
	
	char *resultStringToken;		/* for parsing key-value pairs from genQueryOut */
	sqlResult_t *sqlResult;			/* for parsing key-value pairs from genQueryOut */
	int i;					/* for parsing key-value pairs from genQueryOut */


	RE_TEST_MACRO ("    Calling msiGetCollectionSize")
	
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiGetCollectionSize: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}


	/* parse inpParam1: our target collection */
	rei->status = parseMspForCollInp (collPath, &collInpCache, &outCollInp, 0);
	
	if (rei->status < 0) {
		rodsLog (LOG_ERROR, "msiGetCollectionSize: input collPath error. status = %d", rei->status);
		return (rei->status);
	}


	/* allocate memory for our result struct */
	res = (keyValPair_t*)malloc(sizeof(keyValPair_t));
	memset (res, 0, sizeof (keyValPair_t));


	/* Wanted fields. We use coll_id to do a join query on r_data_main and r_coll_main */
	memset (&genQueryInp, 0, sizeof (genQueryInp));
	addInxIval (&genQueryInp.selectInp, COL_DATA_SIZE, 1);
	addInxIval (&genQueryInp.selectInp, COL_D_DATA_ID, 1);
	addInxIval (&genQueryInp.selectInp, COL_COLL_ID, 1);


	/* Make condition for getting all objects under a collection */
	genAllInCollQCond (outCollInp->collName, collQCond);
	addInxVal (&genQueryInp.sqlCondInp, COL_COLL_NAME, collQCond);
	genQueryInp.maxRows = MAX_SQL_ROWS;
	genQueryInp.options = RETURN_TOTAL_ROW_COUNT;


	/* Query */
	rei->status  = rsGenQuery (rei->rsComm, &genQueryInp, &genQueryOut);


	/* intit counters */
	size = 0;
	objCount = 0;


	/* Parse results */
	if (rei->status == 0) {

		/* add row count to previous total */
		objCount += genQueryOut->totalRowCount;

		/* for each row */
		for (i=0;i<genQueryOut->rowCnt;i++) {
	
			/* get COL_DATA_SIZE result */
			sqlResult = getSqlResultByInx (genQueryOut, COL_DATA_SIZE);

			/* retrieve value for this row */
			resultStringToken = sqlResult->value + i*sqlResult->len;

			/* add to previous total */
			size += atoll(resultStringToken);
		}

		/* not done? */
		while (rei->status==0 && genQueryOut->continueInx > 0) {
			genQueryInp.continueInx=genQueryOut->continueInx;
			rei->status = rsGenQuery(rei->rsComm, &genQueryInp, &genQueryOut);
	
			/* add row count to previous total */
			objCount += genQueryOut->totalRowCount;

			/* for each row */
			for (i=0;i<genQueryOut->rowCnt;i++) {
			
				/* get COL_DATA_SIZE result */
				sqlResult = getSqlResultByInx (genQueryOut, COL_DATA_SIZE);
	
				/* retrieve value for this row */
				resultStringToken = sqlResult->value + i*sqlResult->len;
	
				/* add to previous total */
				size += atoll(resultStringToken);
			}
		}
	}


	/* store results in keyValPair_t*/
	snprintf(tmpStr, 21, "%lld", size);
	addKeyVal(res, "Size", tmpStr);

	snprintf(tmpStr, 21, "%lld", objCount);
	addKeyVal(res, "Object Count", tmpStr);


	/* send results out to outKVPairs */
	fillMsParam (outKVPairs, NULL, KeyValPair_MS_T, res, NULL);


	/* Return operation status through outParam */
	fillIntInMsParam (status, rei->status);

	return (rei->status);
}



/**
 * \fn msiStructFileBundle
 * \author  Antoine de Torcy
 * \date   2009-04-21
 * \brief Bundles a collection for export
 * \note This microservice creates a bundle from an iRods collection on a target resource.
 *		Files in the collection are first replicated onto the target resource. If no resource
 *		is given the default resource will be used.
 * \param[in] 
 *    collection - A CollInp_MS_T or a STR_MS_T with the irods path of the collection to bundle.
 *	  bundleObj - a DataObjInp_MS_T or a STR_MS_T with the bundle object's path.
 *    resource - Optional - a STR_MS_T which specifies the target resource.
 * \param[out] 
 *    status - an INT_MS_T containing the operation status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/
int
msiStructFileBundle(msParam_t *collection, msParam_t *bundleObj, msParam_t *resource, msParam_t *status, ruleExecInfo_t *rei)
{
	collInp_t collInpCache, *collInp;				/* for parsing collection input param */
	dataObjInp_t destObjInpCache, *destObjInp;		/* for parsing bundle object inp. param */
	structFileExtAndRegInp_t *structFileBundleInp; 	/* input for rsStructfileBundle */
	
	
	
	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiStructfileBundle")
	
	
	/* Sanity test */
	if (rei == NULL || rei->rsComm == NULL) {
			rodsLog (LOG_ERROR, "msistructFileBundle: input rei or rsComm is NULL.");
			return (SYS_INTERNAL_NULL_INPUT_ERR);
	}
	
	
	/* Parse collection input */
	rei->status = parseMspForCollInp (collection, &collInpCache, &collInp, 0);
	
	if (rei->status < 0) {
		rodsLog (LOG_ERROR, "msiStructFileBundle: input collection error. status = %d", rei->status);
		return (rei->status);
	}


	/* Get path of destination bundle object */
	rei->status = parseMspForDataObjInp (bundleObj, &destObjInpCache, &destObjInp, 0);
	
	if (rei->status < 0)
	{
		rodsLog (LOG_ERROR, "msiStructFileBundle: input bundleObj error. status = %d", rei->status);
		return (rei->status);
	}

	
	/* Parse resource input */
	rei->status = parseMspForCondInp (resource, &collInp->condInput, DEST_RESC_NAME_KW);
      
    if (rei->status < 0)  {
        rodsLog (LOG_ERROR, "msistructFileBundle: input resource error. status = %d", rei->status);
        return (rei->status);
    }
    
    
    /* Replicate collection to target resource */
    rei->status = rsCollRepl (rei->rsComm, collInp, NULL);
    
    
    /* Set up input for rsStructFileBundle */
    structFileBundleInp = (structFileExtAndRegInp_t *) malloc (sizeof(structFileExtAndRegInp_t));
    memset (structFileBundleInp, 0, sizeof (structFileExtAndRegInp_t));
    rstrcpy (structFileBundleInp->objPath, destObjInp->objPath, MAX_NAME_LEN);
    rstrcpy (structFileBundleInp->collection, collInp->collName, MAX_NAME_LEN);
    
    /* Add resource info to structFileBundleInp */
    replKeyVal (&collInp->condInput, &structFileBundleInp->condInput);
    
    /* Set data type of target object to tar file (required by rsStructFileBundle) */
    addKeyVal (&structFileBundleInp->condInput, DATA_TYPE_KW, "tar file");
    
    
    /* And now... let's see what happens */
    rei->status = rsStructFileBundle (rei->rsComm, structFileBundleInp);


	/* Return operation status */
	fillIntInMsParam (status, rei->status);


	return 0;
}






