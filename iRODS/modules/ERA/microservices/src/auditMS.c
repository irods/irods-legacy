/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "auditMS.h"
#include "auditUtil.h"


/*
 * Retrieves Audit Trail information for a user ID
 *
 */
int
msiGetAuditTrailInfoByUserID(msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, ruleExecInfo_t *rei)
{
	rsComm_t *rsComm; 
	char *userID;
	bytesBuf_t *mybuf;
	
	
	RE_TEST_MACRO ("    Calling msiGetAuditTrailInfoByUserID")
	
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiGetAuditTrailInfoByUserID: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}
	
	rsComm = rei->rsComm;
	
	/* buffer init */
	mybuf = (bytesBuf_t *)malloc(sizeof(bytesBuf_t));
	memset (mybuf, 0, sizeof (bytesBuf_t));


	/* parse inpParam1 (user ID input string) */
	if ((userID = parseMspForStr (inpParam1)) == NULL) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			"msiGetAuditTrailInfoByUserID: parseMspForStr error for param 1.");
		return (rei->status);
	}

	
	/* call getAuditTrailInfoByUserID() */
	rei->status = getAuditTrailInfoByUserID(userID, mybuf, rsComm);

	/* failure? */
	if (rei->status < 0) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			"msiGetAuditTrailInfoByUserID: getAuditTrailInfoByUserID failed for user ID %s, status = %d", userID, rei->status);
	}


	/* return operation status through outParam */
	fillIntInMsParam (outParam, rei->status);


	/* send result buffer, even if length is 0, to inParam2 */
	if (!mybuf->buf) {
		mybuf->buf = strdup("");
	}
	fillBufLenInMsParam (inpParam2, strlen(mybuf->buf), mybuf);

	
	return (rei->status);

}



/*
 * Retrieves Audit Trail information for an object ID
 *
 */
int
msiGetAuditTrailInfoByObjectID(msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, ruleExecInfo_t *rei)
{
	rsComm_t *rsComm; 
	char *objectID;
	bytesBuf_t *mybuf;
	
	
	RE_TEST_MACRO ("    Calling msiGetAuditTrailInfoByObjectID")
	
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiGetAuditTrailInfoByObjectID: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}
	
	rsComm = rei->rsComm;
	
	/* buffer init */
	mybuf = (bytesBuf_t *)malloc(sizeof(bytesBuf_t));
	memset (mybuf, 0, sizeof (bytesBuf_t));


	/* parse inpParam1 (object ID input string) */
	if ((objectID = parseMspForStr (inpParam1)) == NULL) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			"msiGetAuditTrailInfoByObjectID: parseMspForStr error for param 1.");
		return (rei->status);
	}

	
	/* call getAuditTrailInfoByObjectID() */
	rei->status = getAuditTrailInfoByObjectID(objectID, mybuf, rsComm);

	/* failure? */
	if (rei->status < 0) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			"msiGetAuditTrailInfoByObjectID: getAuditTrailInfoByObjectID failed for object ID %s, status = %d", objectID, rei->status);
	}


	/* return operation status through outParam */
	fillIntInMsParam (outParam, rei->status);


	/* send result buffer, even if length is 0, to inParam2 */
	if (!mybuf->buf) {
		mybuf->buf = strdup("");
	}
	fillBufLenInMsParam (inpParam2, strlen(mybuf->buf), mybuf);

	
	return (rei->status);

}



/*
 * Retrieves Audit Trail information for an action ID
 *
 */
int
msiGetAuditTrailInfoByActionID(msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, ruleExecInfo_t *rei)
{
	rsComm_t *rsComm; 
	char *actionID;
	bytesBuf_t *mybuf;
	
	
	RE_TEST_MACRO ("    Calling msiGetAuditTrailInfoByActionID")
	
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiGetAuditTrailInfoByActionID: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}
	
	rsComm = rei->rsComm;
	
	/* buffer init */
	mybuf = (bytesBuf_t *)malloc(sizeof(bytesBuf_t));
	memset (mybuf, 0, sizeof (bytesBuf_t));


	/* parse inpParam1 (action ID input string) */
	if ((actionID = parseMspForStr (inpParam1)) == NULL) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			"msiGetAuditTrailInfoByActionID: parseMspForStr error for param 1.");
		return (rei->status);
	}

	
	/* call getAuditTrailInfoByActionID() */
	rei->status = getAuditTrailInfoByActionID(actionID, mybuf, rsComm);

	/* failure? */
	if (rei->status < 0) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			"msiGetAuditTrailInfoByActionID: getAuditTrailInfoByActionID failed for action ID %s, status = %d", actionID, rei->status);
	}


	/* return operation status through outParam */
	fillIntInMsParam (outParam, rei->status);


	/* send result buffer, even if length is 0, to inParam2 */
	if (!mybuf->buf) {
		mybuf->buf = strdup("");
	}
	fillBufLenInMsParam (inpParam2, strlen(mybuf->buf), mybuf);

	
	return (rei->status);

}



/*
 * Retrieves Audit Trail information by keywords in the comment field
 *
 */
int
msiGetAuditTrailInfoByKeywords(msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, ruleExecInfo_t *rei)
{
	rsComm_t *rsComm; 
	char *commentStr;
	bytesBuf_t *mybuf;
	
	
	RE_TEST_MACRO ("    Calling msiGetAuditTrailInfoByKeywords")
	
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiGetAuditTrailInfoByKeywords: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}
	
	rsComm = rei->rsComm;
	
	/* buffer init */
	mybuf = (bytesBuf_t *)malloc(sizeof(bytesBuf_t));
	memset (mybuf, 0, sizeof (bytesBuf_t));


	/* parse inpParam1 (comment input string) */
	if ((commentStr = parseMspForStr (inpParam1)) == NULL) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			"msiGetAuditTrailInfoByKeywords: parseMspForStr error for param 1.");
		return (rei->status);
	}

	
	/* call getAuditTrailInfoByKeywords() */
	rei->status = getAuditTrailInfoByKeywords(commentStr, mybuf, rsComm);

	/* failure? */
	if (rei->status < 0) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			"msiGetAuditTrailInfoByKeywords: getAuditTrailInfoByKeywords failed for comment string %s, status = %d", commentStr, rei->status);
	}


	/* return operation status through outParam */
	fillIntInMsParam (outParam, rei->status);


	/* send result buffer, even if length is 0, to inParam2 */
	if (!mybuf->buf) {
		mybuf->buf = strdup("");
	}
	fillBufLenInMsParam (inpParam2, strlen(mybuf->buf), mybuf);

	
	return (rei->status);

}



/*
 * Retrieves Audit Trail information by timestamp
 * 
 *
 */
int
msiGetAuditTrailInfoByTimeStamp(msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *inpParam3, msParam_t *outParam, ruleExecInfo_t *rei)
{
	rsComm_t *rsComm; 
	char *begTS;
	char *endTS;
	bytesBuf_t *mybuf;
	
	
	RE_TEST_MACRO ("    Calling msiGetAuditTrailInfoByTimeStamp")
	
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiGetAuditTrailInfoByTimeStamp: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}
	
	rsComm = rei->rsComm;
	
	/* buffer init */
	mybuf = (bytesBuf_t *)malloc(sizeof(bytesBuf_t));
	memset (mybuf, 0, sizeof (bytesBuf_t));


	/* NULL values for timestamps don't generate errors */
	/* parse inpParam1 (beginning timestamp) */
	if ((begTS = parseMspForStr (inpParam1)) == NULL) {
		begTS = strdup("");
	}

	/* parse inpParam2 (end timestamp) */
	if ((endTS = parseMspForStr (inpParam2)) == NULL) {
		endTS = strdup("");
	}

	
	/* call getAuditTrailInfoByTimeStamp() */
	rei->status = getAuditTrailInfoByTimeStamp(begTS, endTS, mybuf, rsComm);

	/* failure? */
	if (rei->status < 0) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
			"msiGetAuditTrailInfoByTimeStamp: getAuditTrailInfoByTimeStamp failed for values between %s and %s, status = %d", begTS, endTS, rei->status);
	}


	/* return operation status through outParam */
	fillIntInMsParam (outParam, rei->status);

	/* send result buffer, even if length is 0, to inParam3 */
	if (!mybuf->buf) {
		mybuf->buf = strdup("");
	}
	fillBufLenInMsParam (inpParam3, strlen(mybuf->buf), mybuf);
	
	return (rei->status);

}








