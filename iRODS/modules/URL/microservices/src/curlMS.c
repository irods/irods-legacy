/**
 * @file curlMS.c
 *
 */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "curlMS.h"

#ifdef  OAUTH
extern "C" {
#include <oauth.h>
}
#endif

#include <stdio.h>

#include <curl/curl.h>
#include <curl/easy.h>

/* types.h is not included in newer versions of libcurl */ 
#if LIBCURL_VERSION_NUM < 0x071503
#include <curl/types.h>
#endif

#define MAX_DL_STR_LEN	1048576		// 1MB

/* Keep structure definitions local */
typedef struct {
  char *ptr;
  size_t len;		/* not counting terminating null char */
} string_t;


typedef struct {
	char objPath[MAX_NAME_LEN];
	int l1descInx;
	rsComm_t *rsComm;
} writeDataInp_t;


typedef struct {
	size_t downloaded;
	size_t cutoff;	/* 0 means unlimited */
} curlProgress_t;



/* Callback progress function for the curl handler */
static int progress(void *p, double dltotal, double dlnow, double ultotal, double ulnow) {
	curlProgress_t *prog = (curlProgress_t *)p;

	/* Update total so far */
	prog->downloaded = (size_t)dlnow;

	/* Abort if next transfer could exceed cutoff */
	if (prog->cutoff && (dlnow + CURL_MAX_WRITE_SIZE > prog->cutoff)) {
		rodsLog(LOG_NOTICE, "progress(): Aborting curl download, max size is %d bytes", prog->cutoff);
		return -1;
	}

	return 0;
}



/* Custom callback function for the curl handler, to write to an iRODS object */
static size_t createAndWriteToDataObj(void *buffer, size_t size, size_t nmemb, void *stream)
{
	writeDataInp_t *writeDataInp;			/* the "file descriptor" for our destination object */
	dataObjInp_t dataObjInp;				/* input structure for rsDataObjCreate */
	openedDataObjInp_t openedDataObjInp;	/* input structure for rsDataObjWrite */
	bytesBuf_t bytesBuf;					/* input buffer for rsDataObjWrite */
	size_t written;							/* output value */


	/* retrieve writeDataInp_t input */
	writeDataInp = (writeDataInp_t *)stream;


	/* to avoid unpleasant surprises */
	memset(&dataObjInp, 0, sizeof(dataObjInp_t));
	memset(&openedDataObjInp, 0, sizeof(openedDataObjInp_t));


	/* If this is the first call we need to create our data object before writing to it */
	if (writeDataInp && !writeDataInp->l1descInx)
	{
		strcpy(dataObjInp.objPath, writeDataInp->objPath);
		writeDataInp->l1descInx = rsDataObjCreate(writeDataInp->rsComm, &dataObjInp);

		/* problem? */
		if (writeDataInp->l1descInx <= 2)
		{
			rodsLog (LOG_ERROR, "createAndWriteToDataObj: rsDataObjCreate failed for %s, status = %d", dataObjInp.objPath, writeDataInp->l1descInx);
			return (writeDataInp->l1descInx);
		}
	}


	/* set up input buffer for rsDataObjWrite */
	bytesBuf.len = (int)(size * nmemb);
	bytesBuf.buf = buffer;


	/* set up input data structure for rsDataObjWrite */
	openedDataObjInp.l1descInx = writeDataInp->l1descInx;
	openedDataObjInp.len = bytesBuf.len;


	/* write to data object */
	written = rsDataObjWrite(writeDataInp->rsComm, &openedDataObjInp, &bytesBuf);

	return (written);
}



/* Another curl write callback function, this time to a string */
static size_t write_string(void *ptr, size_t size, size_t nmeb, void *stream)
{
	size_t newLen;
	string_t *string;
	void *tmpPtr;

	if (!stream) {
		rodsLog (LOG_ERROR, "%s", "write_string() error. NULL destination stream.");
		return 0;
	}

	string = (string_t *)stream;

	newLen = string->len + size*nmeb;

	/* Reallocate memory with space for new content */
	/* Add an extra byte for terminating null char */
	tmpPtr = realloc(string->ptr, newLen + 1);
	if (!tmpPtr) {
		rodsLog(LOG_ERROR, "%s", "write_string(): realloc failed.");
		return -1;
	}
	string->ptr = (char*)tmpPtr;


	/* Append new content to string and terminating '\0' */
	memcpy(string->ptr + string->len, ptr, size*nmeb);
	string->ptr[newLen] = '\0';
	string->len = newLen;

	return size*nmeb;

}



int msiCurlGetStr(msParam_t *url, msParam_t *buffer, ruleExecInfo_t *rei) {
	CURL *curl;							/* curl handler */
	CURLcode res;
	char *my_url;
	string_t string;

	curlProgress_t prog;				/* for progress and cutoff */


	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiCurlGetStr")

	/* Sanity checks */
	if (!rei || !rei->rsComm) {
		rodsLog (LOG_ERROR, "msiCurlGetStr: Input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	/* Check url input */
	my_url = parseMspForStr(url);
	if (!my_url || !strlen(my_url)) {
		rodsLog (LOG_ERROR, "msiCurlGetStr: Null or empty url input.");
		return (USER_INPUT_STRING_ERR);
	}

	/* Init string */
	string.ptr = strdup("");
	string.len = 0;


	/* Progress struct init */
	prog.downloaded = 0;
	prog.cutoff = MAX_DL_STR_LEN;


	/* CURL easy handler init */
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if(curl) {
		/* Set up easy handler */
		curl_easy_setopt(curl, CURLOPT_URL, my_url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &string);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		/* Progress settings */
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &prog);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

		/* CURL call */
		res = curl_easy_perform(curl);

		/* Cleanup curl handler */
		curl_easy_cleanup(curl);
	}
	else {
		res = CURLE_FAILED_INIT;
	}

	/* Some error logging */
	if(res != CURLE_OK) {
		rodsLog (LOG_ERROR, "msiCurlGetStr: cURL error: %s", curl_easy_strerror(res));
	}

	/* CURL cleanup before returning */
	curl_global_cleanup();

	/* This should not happen, but just in case... */
	if (!string.ptr) {
		rodsLog (LOG_ERROR, "msiCurlGetStr: string.ptr is NULL");
		return ACTION_FAILED_ERR;
	}

	/* Return string */
	fillStrInMsParam(buffer, string.ptr); // does an strdup

	/* Cleanup and done */
	free (string.ptr);
	return 0;
}



int msiCurlGetObj(msParam_t *url, msParam_t *object, msParam_t *downloaded, ruleExecInfo_t *rei) {
	CURL *curl;									/* curl handler */
	CURLcode res;
	char *my_url;

	dataObjInp_t destObjInp, *myDestObjInp;		/* for parsing input object */

	writeDataInp_t writeDataInp;				/* custom file descriptor for our callback function */
	openedDataObjInp_t openedDataObjInp;		/* to close iRODS object after writing */

	curlProgress_t prog;						/* for progress and cutoff */


	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiCurlGetObj")

	/* Sanity checks */
	if (!rei || !rei->rsComm) {
		rodsLog (LOG_ERROR, "msiCurlGetObj: Input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}


	/* Pad data structures with null chars */
	memset(&writeDataInp, 0, sizeof(writeDataInp_t));
	memset(&openedDataObjInp, 0, sizeof(openedDataObjInp_t));


	/* Check url input */
	my_url = parseMspForStr(url);
	if (!my_url || !strlen(my_url)) {
		rodsLog (LOG_ERROR, "msiCurlGetObj: Null or empty url input.");
		return (USER_INPUT_STRING_ERR);
	}


	/* Get path of destination object */
	rei->status = parseMspForDataObjInp (object, &destObjInp, &myDestObjInp, 0);
	if (rei->status < 0)
	{
		rodsLog (LOG_ERROR, "msiCurlGetObj: Input object error. status = %d", rei->status);
		return (rei->status);
	}


	/* Set up writeDataInp */
	strcpy(writeDataInp.objPath, destObjInp.objPath);
	writeDataInp.l1descInx = 0; /* the object is yet to be created */
	writeDataInp.rsComm = rei->rsComm;


	/* Progress struct init */
	prog.downloaded = 0;
	prog.cutoff = 0;


	/* CURL easy handler init */
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if(curl) {
		/* Set up easy handler */
		curl_easy_setopt(curl, CURLOPT_URL, my_url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, createAndWriteToDataObj);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeDataInp);

		/* Progress settings */
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &prog);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

		/* CURL call */
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		res = curl_easy_perform(curl);

		/* Cleanup curl handler */
		curl_easy_cleanup(curl);
	}
	else {
		res = CURLE_FAILED_INIT;
	}

	/* Some error logging */
	if(res != CURLE_OK) {
		rodsLog (LOG_ERROR, "msiCurlGetObj: cURL error: %s", curl_easy_strerror(res));
	}

	/* CURL cleanup before returning */
	curl_global_cleanup();

	/* close iRODS object */
	if (writeDataInp.l1descInx)
	{
		openedDataObjInp.l1descInx = writeDataInp.l1descInx;
		rei->status = rsDataObjClose(rei->rsComm, &openedDataObjInp);
		if (rei->status < 0) {
			rodsLog (LOG_ERROR, "msiCurlGetObj: rsDataObjClose failed for %s, status = %d",
					writeDataInp.objPath, rei->status);
		}
	}

	/* Return bytes read/written */
	fillIntInMsParam(downloaded, prog.downloaded);

	/* Cleanup and done */
	return 0;
}



int msiCurlPost(msParam_t *url, msParam_t *postFields, msParam_t *response, ruleExecInfo_t *rei) {
	CURL *curl;							/* curl handler */
	CURLcode res;

	struct curl_slist *headers = NULL;

	char *my_url, *my_headers, *data;	/* input for POST request */
	char *encoded_data = NULL;

	string_t string;					/* server response */

	int must_encode = 0; // for the time being...

	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiCurlPost")

	/* Sanity checks */
	if (!rei || !rei->rsComm) {
		rodsLog (LOG_ERROR, "msiCurlPost: Input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	/* Parse url input */
	my_url = parseMspForStr(url);
	if (!my_url || !strlen(my_url)) {
		rodsLog (LOG_ERROR, "msiCurlPost: Null or empty url input.");
		return (USER_INPUT_STRING_ERR);
	}


	/* Check for proper input type */
	if (!postFields->type || strcmp(postFields->type, KeyValPair_MS_T)) {
		rodsLog (LOG_ERROR, "msiCurlPost: postFields must be KeyValPair_MS_T.");
		return (USER_PARAM_TYPE_ERR);
	}


	/* Parse POST fields */
	data = getValByKey((keyValPair_t *)postFields->inOutStruct, "data");
	my_headers = getValByKey((keyValPair_t *)postFields->inOutStruct, "headers");


	/* Init string */
	string.ptr = strdup("");
	string.len = 0;

	/* CURL easy handler init */
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if(curl) {
		/* URL-encode data */
		if (must_encode && data) {
			encoded_data = curl_easy_escape(curl, data, 0);
		}

		/* Set headers */
		if (my_headers && strlen(my_headers)) {
			headers = curl_slist_append(headers, my_headers);
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		}

		/* Set up easy handler */
		curl_easy_setopt(curl, CURLOPT_URL, my_url);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &string);

		/* CURL call */
		res = curl_easy_perform(curl);

		/* Cleanup */
		if (headers) curl_slist_free_all(headers);
		if (encoded_data) curl_free(encoded_data);
		curl_easy_cleanup(curl);
	}
	else {
		res = CURLE_FAILED_INIT;
	}

	/* Some error logging */
	if(res != CURLE_OK) {
		rodsLog (LOG_ERROR, "msiCurlPost: cURL error: %s", curl_easy_strerror(res));
	}

	/* CURL cleanup before returning */
	curl_global_cleanup();

	/* Return string and bytes read */
	fillStrInMsParam(response, string.ptr); // does an strdup

	/* Cleanup and done */
	free (string.ptr);

	return res;
}



/**
 * \fn msiTwitterPost(msParam_t *twittername, msParam_t *twitterpass, msParam_t *message, msParam_t *status, ruleExecInfo_t *rei)
 *
 * \brief Posts a message to twitter.com
 *
 * \module URL
 *
 * \since 2.3
 *
 * \author  Antoine de Torcy
 * \date    2009-07-23
 *
 * \note This microservice posts a message on twitter.com, aka a "tweet". 
 *       A valid twitter account name and password must be provided. 
 *       Special characters in the message can affect parsing of the POST form and 
 *       create unexpected results. Avoid if possible, or use quotes.
 *       This is intended for fun and for use in demos. Since your twitter password is
 *       passed unencrypted here, do not use this with a twitter account you do not
 *       wish to be compromised. Or if you do, change your password afterwards. 
 *
 * \usage See clients/icommands/test/rules3.0/
 *
 * \param[in] twittername - Required - a STR_MS_T containing the twitter username.
 * \param[in] twitterpass - Required - a STR_MS_T containing the twitter password.
 * \param[in] message - Required - a STR_MS_T containing the message to post.
 * \param[out] status - An INT_MS_T containing the status.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence None
 * \DolVarModified None
 * \iCatAttrDependence None
 * \iCatAttrModified None
 * \sideeffect None
 *
 * \return integer
 * \retval 0 on success
 * \pre None
 * \post None
 * \sa None
**/
#ifndef OAUTH
int msiTwitterPost(msParam_t *twittername, msParam_t *twitterpass, msParam_t *message, msParam_t *status, ruleExecInfo_t *rei)
{
	CURL *curl;								/* curl handler */
	CURLcode res;

	char *username, *passwd, *tweet;		/* twitter account and msg. */
	
	char userpwd[LONG_NAME_LEN];			/* input for POST request */
	char form_msg[160];

	int isCurlErr;							/* transfer success/failure boolean */



	/*************************************  INIT **********************************/
	
	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiTwitterPost")
	
	/* Sanity checks */
	if (rei == NULL || rei->rsComm == NULL) 
	{
		rodsLog (LOG_ERROR, "msiTwitterPost: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}


	/********************************** PARAM PARSING  *********************************/

	/* parse twitter name */
	if ((username = parseMspForStr(twittername)) == NULL)
	{
		rodsLog (LOG_ERROR, "msiTwitterPost: input twittername is NULL.");
		return (USER__NULL_INPUT_ERR);
	}
	
	/* parse twitter password */
	if ((passwd = parseMspForStr(twitterpass)) == NULL)
	{
		rodsLog (LOG_ERROR, "msiTwitterPost: input twitterpass is NULL.");
		return (USER__NULL_INPUT_ERR);
	}

	/* parse message */
	if ((tweet = parseMspForStr(message)) == NULL)
	{
		rodsLog (LOG_ERROR, "msiTwitterPost: input message is NULL.");
		return (USER__NULL_INPUT_ERR);
	}


	/* prepare form and truncate tweet to 140 chars */
	strcpy(form_msg, "status=");
	strncat(form_msg, tweet, 140);

	/* Prepare userpwd */
	snprintf(userpwd, LONG_NAME_LEN, "%s:%s", username, passwd);


	/************************** SET UP AND INVOKE CURL HANDLER **************************/

	/* curl easy handler init */
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	isCurlErr = 0;

	if(curl) 
	{
		/* Set up curl easy handler */
		curl_easy_setopt(curl, CURLOPT_URL, "http://twitter.com/statuses/update.xml");
		curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, form_msg);
		
		/* For debugging */
// 		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		
		/* let curl do its thing */
		res = curl_easy_perform(curl);
		
		/* cleanup curl handler */
		curl_easy_cleanup(curl);
		
		/* how did it go? */
		if(CURLE_OK != res)
		{
			rodsLog (LOG_ERROR, "msiTwitterPost: libcurl error: %d", res);
			isCurlErr = 1;
		}
	}


	/*********************************** DONE ********************************/

	/* cleanup and return status */
	curl_global_cleanup();

	if (isCurlErr)
	{
		/* -1 for now. should add an error code for this */
		rei->status = -1;
	}

	fillIntInMsParam (status, rei->status);
	
	return (rei->status);
}
#else
/* UPDATED TO SUPPORT OAUTH. THIS PART IS A WORK IN PROGRESS... */
int msiTwitterPost(msParam_t *twittername, msParam_t *twitterpass, msParam_t *message, msParam_t *status, ruleExecInfo_t *rei)
{
	char status_update[160], *tweet;
	char *url="http://twitter.com/statuses/update.json";

	const char *c_key         = "CONSUMER_KEY_HERE"; // consumer key
	const char *c_secret      = "CONSMER_SECRET_HERE"; // consumer secret
	char *t_key               = "ACCESS_TOKEN_KEY_HERE"; // access token key
	char *t_secret            = "ACCESS_TOKEN_SECRET_HERE"; // access token secret

	char *postarg = NULL;
	char *req_url = NULL;
	char *reply   = NULL;
	char *bh;
	char *uh;
	char *sig_url;



	/* parse message */
	if ((tweet = parseMspForStr(message)) == NULL)
	{
		rodsLog (LOG_ERROR, "msiTwitterPost: input message is NULL.");
		return (USER__NULL_INPUT_ERR);
	}


	/* prepare form and truncate tweet to 140 chars */
	strcpy(status_update, "status=");
	strncat(status_update, tweet, 140);

//	bh=oauth_body_hash_data(strlen(data), data);
//	uh = oauth_catenc(2, url, bh);
	uh = oauth_catenc(2, url, status_update);
	req_url = oauth_sign_url2(uh, &postarg, OA_HMAC, NULL, c_key, c_secret, t_key, t_secret);


	rodsLog(LOG_NOTICE, "---------------------------------");

	rodsLog(LOG_NOTICE, "req_url: %s", req_url);
	rodsLog(LOG_NOTICE, "postarg: %s", postarg);



	rodsLog(LOG_NOTICE, "msiTwitterPost - POST: %s?%s", req_url, postarg);

	if (uh) free(uh);

	sig_url = (char *)malloc(2+strlen(req_url)+strlen(postarg));
	sprintf(sig_url,"%s?%s",req_url, postarg);
	reply = oauth_post_data(sig_url, status_update, strlen(status_update), "Content-Type: application/json");

	if(sig_url) free(sig_url);

	rodsLog(LOG_NOTICE, "msiTwitterPost - REPLY: %s", reply);


	rodsLog(LOG_NOTICE, "---------------------------------");


	fillStrInMsParam (status, reply);

//	if(reply) free(reply);

	return 0;

}

#endif

#ifdef JSON
#include "parser.h"
json_t *parseMspForJson(msParam_t *inpParam) {
    if (inpParam == NULL || inpParam->inOutStruct == NULL) {
        return (NULL);
    }

    if (strcmp (inpParam->type, JSON_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "parseMspForJson: inpParam type %s is not JSON_MS_T",
          inpParam->type);
    }

    return (json_t *)(inpParam->inOutStruct);
}

/* input string -> `JSON_PI` */
Res *msiParseJSON(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

	json_t *root;
	json_error_t error;
	char *text;

	text = subtrees[0]->text;

	root = json_loads(text, 0, &error);

	if(root == NULL) {
		char err[ERR_MSG_LEN];
		snprintf(err, ERR_MSG_LEN, "error: on line %d: %s\n", error.line, error.text);
	    	generateAndAddErrMsg(err, node, RE_JSON_ERROR, errmsg);
    		return newErrorRes(r, RE_JSON_ERROR);
	}
	
	return newUninterpretedRes(r, JSON_MS_T, root, NULL);
}

/* input `JSON_PI` -> integer */
Res* msiFreeJSON(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
	json_t *root;
	
	root = (json_t *) RES_UNINTER_STRUCT(subtrees[0]);

	json_decref(root);

	return newIntRes(r, 0);
}

/* input `JSON_PI` * input string -> `JSON_PI` */
Res* msiJSONObjectGet(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
	json_t *root, *val;
	char *text;
	
	root = (json_t *) RES_UNINTER_STRUCT(subtrees[0]);
	text = subtrees[1]->text;

	val = json_object_get(root, text);

	if(val == NULL) {
		char err[ERR_MSG_LEN];
		snprintf(err, ERR_MSG_LEN, "msiJSONObjectGet: cannot get value for key %s.", text);
	    	generateAndAddErrMsg(err, node, RE_JSON_ERROR, errmsg);
    		return newErrorRes(r, RE_JSON_ERROR);
	}

	return newUninterpretedRes(r, JSON_MS_T, val, NULL);
}

/* input `JSON_PI` -> integer */
Res* msiJSONArraySize(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
	json_t *root;
	
	root = (json_t *) RES_UNINTER_STRUCT(subtrees[0]);

	unsigned int arraySize;
	
	arraySize = json_array_size(root);

	return newIntRes(r, arraySize);
}

/* input `JSON_PI` * input integer -> `JSON_PI` */
Res* msiJSONArrayGet(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
	json_t *root, *val;
	int index;
	
	root = (json_t *) RES_UNINTER_STRUCT(subtrees[0]);
	index = RES_INT_VAL(subtrees[1]);

	val = json_array_get(root, index);

	if(val == NULL) {
		char err[ERR_MSG_LEN];
		snprintf(err, ERR_MSG_LEN, "msiJSONArrayGet: cannot get value for index %d.", index);
	    	generateAndAddErrMsg(err, node, RE_JSON_ERROR, errmsg);
    		return newErrorRes(r, RE_JSON_ERROR);
	}

	return newUninterpretedRes(r, JSON_MS_T, val, NULL);
}

/* input `JSON_PI` -> string */
Res* msiJSONStringValue(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
	json_t *root;
	char *val;

	root = (json_t *) RES_UNINTER_STRUCT(subtrees[0]);

	val = (char *) json_string_value(root);

	if(val == NULL) {
		char err[ERR_MSG_LEN];
		snprintf(err, ERR_MSG_LEN, "msiJSONStringValue: cannot get value.");
	    	generateAndAddErrMsg(err, node, RE_JSON_ERROR, errmsg);
    		return newErrorRes(r, RE_JSON_ERROR);
	}

	return newStringRes(r, val);
}

/* input `JSON_PI` -> integer */
Res* msiJSONIntegerValue(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
	json_t *root;
	int val;

	root = (json_t *) RES_UNINTER_STRUCT(subtrees[0]);

	val = json_integer_value(root);

	return newIntRes(r, val);
}

/* input `JSON_PI` -> integer */
Res* msiJSONObjectKeys(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
	json_t *root;
	int s = 1024;
	int i = 0;
	char **arr = (char **) malloc(n * sizeof(char *));

	root = (json_t *) RES_UNINTER_STRUCT(subtrees[0]);

	const char *key;
	json_t *value;

	json_object_foreach(root, key, value) {
    		/* block of code that uses key and value */
		arr[i++] = (char *) key;
		if(i == s) {
			s *= 2;
			arr = (char **) realloc(arr, s * sizeof(char *));
		}
	}

	Res *res = newCollRes(i, newSimpType(T_STRING, r), r);

	int c;
        for(c = 0; c < i; c++) {
		res->subtrees[c] = newStringRes(r, arr[c]);
	}

	free(arr);
	return res;
}


#endif

/**
 *
 * \deprecated Use msiCurlGetObj instead
 *
 * \fn msiFtpGet(msParam_t *target, msParam_t *destObj, msParam_t *status, ruleExecInfo_t *rei)
 *
 * \brief This microservice gets a remote file using FTP and writes it to an iRODS object.
 *
 * \module URL
 *
 * \since pre-2.1
 *
 * \author  Antoine de Torcy
 * \date    2008-09-24
 *
 * \note This microservice uses libcurl to open an ftp session with a remote server and read from a remote file.
 *    The results are written to a newly created iRODS object, one block at a time until the whole file is read.
 *
 * \usage See clients/icommands/test/rules3.0/
 *
 * \param[in] url - Required - a STR_MS_T containing the remote URL.
 * \param[in] destObj - Required - a DataObjInp_MS_T or a STR_MS_T which would be taken as the object's path.
 * \param[out] status - a INT_MS_T containing the status.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence None
 * \DolVarModified None
 * \iCatAttrDependence None
 * \iCatAttrModified None
 * \sideeffect None
 *
 * \return integer
 * \retval 0 on success
 * \pre None
 * \post None
 * \sa None
**/
int msiFtpGet(msParam_t *url, msParam_t *object, msParam_t *status, ruleExecInfo_t *rei)
{
	CURL *curl;					/* curl handler */
	CURLcode res;

	writeDataInp_t writeDataInp;			/* custom file descriptor for our callback function */

	dataObjInp_t destObjInp, *myDestObjInp;		/* for parsing input params */
	openedDataObjInp_t openedDataObjInp;
	char *my_url;

	int isCurlErr;					/* transfer success/failure boolean */



	/*************************************  INIT **********************************/

	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiFtpGet")

	/* Sanity checks */
	if (rei == NULL || rei->rsComm == NULL)
	{
		rodsLog (LOG_ERROR, "msiFtpGet: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	/* pad data structures with null chars */
	memset(&writeDataInp, 0, sizeof(writeDataInp_t));
	memset(&destObjInp, 0, sizeof(dataObjInp_t));
	memset(&openedDataObjInp, 0, sizeof(openedDataObjInp_t));



	/********************************** PARAM PARSING  *********************************/

	/* parse target URL */
	if ((my_url = parseMspForStr(url)) == NULL)
	{
		rodsLog (LOG_ERROR, "msiFtpGet: input url is NULL.");
		return (USER__NULL_INPUT_ERR);
	}


	/* Get path of destination object */
	rei->status = parseMspForDataObjInp (object, &destObjInp, &myDestObjInp, 0);
	if (rei->status < 0)
	{
		rodsLog (LOG_ERROR, "msiFtpGet: input object error. status = %d", rei->status);
		return (rei->status);
	}



	/************************** SET UP AND INVOKE CURL HANDLER **************************/

	/* set up dataObjFtpInp */
	strcpy(writeDataInp.objPath, destObjInp.objPath);
	writeDataInp.l1descInx = 0; /* the object is yet to be created */
	writeDataInp.rsComm = rei->rsComm;


	/* curl easy handler init */
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	isCurlErr = 0;

	if(curl)
	{
		/* Set up curl easy handler */
		curl_easy_setopt(curl, CURLOPT_URL, my_url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, createAndWriteToDataObj);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeDataInp);

		/* For debugging only */
// 		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* let curl do its thing */
		res = curl_easy_perform(curl);

		/* cleanup curl handler */
		curl_easy_cleanup(curl);

		/* how did it go? */
		if(CURLE_OK != res)
		{
			rodsLog (LOG_ERROR, "msiFtpGet: libcurl error: %d", res);
			isCurlErr = 1;
		}
	}


	/* close irods object */
	if (writeDataInp.l1descInx)
	{
		openedDataObjInp.l1descInx = writeDataInp.l1descInx;
		rei->status = rsDataObjClose(rei->rsComm, &openedDataObjInp);
	}



	/*********************************** DONE ********************************/

	/* cleanup and return status */
	curl_global_cleanup();

	if (isCurlErr)
	{
		rei->status = ACTION_FAILED_ERR;
	}

	fillIntInMsParam (status, rei->status);

	return (rei->status);
}





/**
 * \deprecated Use msiCurlPost instead
 *
**/
int msiPostThis(msParam_t *url, msParam_t *data, msParam_t *status, ruleExecInfo_t *rei)
{
	CURL *curl;								/* curl handler */
	CURLcode res;

	char *myurl, *mydata;					/* input for POST request */


	/*************************************  INIT **********************************/

	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiPostThis")

	/* Sanity checks */
	if (rei == NULL || rei->rsComm == NULL)
	{
		rodsLog (LOG_ERROR, "msiPostThis: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}


	/********************************** PARAM PARSING  *********************************/

	/* Parse url */
	if ((myurl = parseMspForStr(url)) == NULL)
	{
		rodsLog (LOG_ERROR, "msiPostThis: input url is NULL.");
		return (USER__NULL_INPUT_ERR);
	}

	/* Parse data, which can be NULL */
	mydata = parseMspForStr(data);


	/************************** SET UP AND INVOKE CURL HANDLER **************************/

	/* cURL easy handler init */
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if(curl)
	{
		/* Set up curl easy handler */
		curl_easy_setopt(curl, CURLOPT_URL, myurl);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, mydata);

		/* For debugging */
// 		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* Let curl do its thing */
		res = curl_easy_perform(curl);

		/* Cleanup curl handler */
		curl_easy_cleanup(curl);

		/* How did it go? */
		if(res != CURLE_OK)
		{
			rodsLog (LOG_ERROR, "msiPostThis: cURL error: %s", curl_easy_strerror(res));
			rei->status = SYS_HANDLER_DONE_WITH_ERROR; /* For lack of anything better */
		}
		else
		{
			rei->status = 0;
		}
	}


	/*********************************** DONE ********************************/

	/* Cleanup and return CURLcode */
	curl_global_cleanup();

	fillIntInMsParam (status, rei->status);
	return (rei->status);
}


#if 0
int msiDABOpenSearchQuery(msParam_t *inKVP, msParam_t *outKVP, ruleExecInfo_t *rei) {
	CURL *curl;								/* curl handler */
	CURLcode res;

	char url[MAX_NAME_LEN];
	char tmpStr[10];

	FILE *file;


	/* OpenSearch Parameters */
	char *searchTerms;

	/* for xml parsing */
	xmlDocPtr doc;

	/* for XPath evaluation */
	xmlXPathContextPtr xpathCtx;
	xmlXPathObjectPtr xpathObj;
	xmlChar xpathExpr[] = "//atom:entry";
	xmlNodeSetPtr nodes;
	int entryNbr, i;

	/* for output */
	keyValPair_t *newKVP;


	/*************************************  INIT **********************************/

	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiDABOpenSearchQuery")

	/* Sanity checks */
	if (rei == NULL || rei->rsComm == NULL)
	{
		rodsLog (LOG_ERROR, "msiDABOpenSearchQuery: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}


	/********************************** PARAM PARSING  *********************************/

	/* Check for wrong parameter type */
	 if (inKVP->type && strcmp(inKVP->type, KeyValPair_MS_T))
	 {
		rodsLog (LOG_ERROR, "msiDABOpenSearchQuery: inKVP is not of type KeyValPair_MS_T.");
		return (USER_PARAM_TYPE_ERR);
	 }


	 /* Parse query params */

	searchTerms = getValByKey((keyValPair_t *)inKVP->inOutStruct, "searchTerms");
	snprintf(url, MAX_NAME_LEN,
			"http://rio2012.eurogeoss-broker.eu/hackathondemo/services/opensearchgeo?si=&ct=25&st=%s&bbox=&rel=&loc=&ts=&te=&outputFormat=application/atom+xml",
			searchTerms);


	/* Set up outKVP */
	newKVP = (keyValPair_t*)malloc(sizeof(keyValPair_t));
	memset(newKVP, 0, sizeof(keyValPair_t));
	outKVP->inOutStruct = (void*)newKVP;
	outKVP->type = strdup(KeyValPair_MS_T);



	file = tmpfile();
	if(!file)
	{
		rei->status = -1;
		return(rei->status);
	}


	/************************** SET UP AND INVOKE CURL HANDLER **************************/

	/* cURL easy handler init */
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if(curl)
	{
		/* Set up curl easy handler */
		curl_easy_setopt(curl, CURLOPT_URL, url);
//		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, mydata);

		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl,CURLOPT_WRITEDATA, file);

		/* For debugging */
// 		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* Let curl do its thing */
		res = curl_easy_perform(curl);

		/* Cleanup curl handler */
		curl_easy_cleanup(curl);

		/* How did it go? */
		if(res != CURLE_OK)
		{
			rodsLog (LOG_ERROR, "msiDABOpenSearchQuery: cURL error: %s", curl_easy_strerror(res));
			rei->status = SYS_HANDLER_DONE_WITH_ERROR; /* For lack of anything better */
		}
		else
		{
			rei->status = 0;
		}
	}


	/***************************** XML PARSING ********************************/

	/* Init libxml */
	xmlInitParser();
	LIBXML_TEST_VERSION

	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue = 1;

	/* Parse and close temp file */
	fseek(file, 0, SEEK_SET);
	doc = xmlReadFd(fileno(file), NULL, "UTF-8", 0);
	fclose(file);


	/* Create xpath evaluation context */
	xpathCtx = xmlXPathNewContext(doc);
	if(xpathCtx == NULL)
	{
		rodsLog (LOG_ERROR, "msiDABOpenSearchQuery: Unable to create new XPath context.");
		xmlFreeDoc(doc);
		return(-1);
	}

	/* Register namespace */
	if(xmlXPathRegisterNs(xpathCtx, (const xmlChar *)"atom", (const xmlChar *)"http://www.w3.org/2005/Atom") != 0) {
		rodsLog (LOG_ERROR, "msiDABOpenSearchQuery: Unable to register namespace.");
		xmlFreeDoc(doc);
	    return(-1);
	}


	/* Evaluate xpath expression */
	xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
	if(xpathObj == NULL)
	{
		rodsLog (LOG_ERROR, "msiDABOpenSearchQuery: Unable to evaluate XPath expression \"%s\".", xpathExpr);
		xmlXPathFreeContext(xpathCtx);
		xmlFreeDoc(doc);
		return(-1);
	}

	/* How many entries did we get? */
	if ((nodes = xpathObj->nodesetval) != NULL)
	{
		entryNbr = nodes->nodeNr;
	}
	else
	{
		entryNbr = 0;
	}


//	rodsLog (LOG_NOTICE, "msiDABOpenSearchQuery: entryNbr = %d.", entryNbr);

	for (i = 0; i < entryNbr; i++) {
		if (nodes->nodeTab[i]) {
//			rodsLog (LOG_NOTICE, "msiDABOpenSearchQuery: i = %d.", i);
		}
	}

	/***************************** Set up output *****************************/

	snprintf(tmpStr, 10, "%d", entryNbr);
	addKeyVal((keyValPair_t*)newKVP, "Number of matching results", tmpStr);


	/*********************************** DONE ********************************/

	/* cleanup of xml doc */
	xmlFreeDoc(doc);


	/* Cleanup and return CURLcode */
	curl_global_cleanup();


	/* libxml2 cleanup. Always do that last */
	xmlCleanupParser();

	return 0;
}
#endif



