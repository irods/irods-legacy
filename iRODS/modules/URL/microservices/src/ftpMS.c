/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

#include "ftpMS.h"


#include <stdio.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>




/* custom callback function for the cURL handler */
static size_t createAndWriteToDataObj(void *buffer, size_t size, size_t nmemb, void *stream)
{
	dataObjFtpInp_t *dataObjFtpInp;		/* the "file descriptor" for our destination object */
	dataObjInp_t dataObjInp;		/* input structure for rsDataObjCreate */
	dataObjWriteInp_t dataObjWriteInp;	/* input structure for rsDataObjWrite */
	bytesBuf_t bytesBuf;			/* input buffer for rsDataObjWrite */
	size_t written;				/* output value */


	/* retrieve dataObjFtpInp_t input */
	dataObjFtpInp = (dataObjFtpInp_t *)stream;


	/* to avoid unpleasant surprises */
	memset(&dataObjInp, 0, sizeof(dataObjInp_t));
	memset(&dataObjWriteInp, 0, sizeof(dataObjWriteInp_t));


	/* If this is the first call we need to create our data object before writing to it */
	if (dataObjFtpInp && !dataObjFtpInp->l1descInx)
	{
		strcpy(dataObjInp.objPath, dataObjFtpInp->objPath);
		dataObjFtpInp->l1descInx = rsDataObjCreate(dataObjFtpInp->rsComm, &dataObjInp);
		
		/* problem? */
		if (dataObjFtpInp->l1descInx <= 2)
		{
			rodsLog (LOG_ERROR, "createAndWriteToDataObj: rsDataObjCreate failed for %s, status = %d", dataObjInp.objPath, dataObjFtpInp->l1descInx);
			return (dataObjFtpInp->l1descInx);
		}
	}


	/* set up input buffer for rsDataObjWrite */
	bytesBuf.len = (int)(size * nmemb);
	bytesBuf.buf = buffer;


	/* set up input data structure for rsDataObjWrite */
	dataObjWriteInp.l1descInx = dataObjFtpInp->l1descInx;
	dataObjWriteInp.len = bytesBuf.len;


	/* write to data object */
	written = rsDataObjWrite(dataObjFtpInp->rsComm, &dataObjWriteInp, &bytesBuf);

	return (written);
}





/**
 * \fn msiFtpGet
 * \author  Antoine de Torcy
 * \date   2008-09-24
 * \brief Gets a remote file using FTP and writes it to an iRODS object
 * \note This microservice uses libcurl to open an ftp session with a remote server and read from a remote file.
 *	The results are written to a newly created iRODS object, one block at a time until the whole file is read.
 * \param[in] 
 *    target - Required - a STR_MS_T containing the remote URL.
 * \param[in] 
 *    destObj - Required - a DataObjInp_MS_T or a STR_MS_T which would be taken as the object's path.
 * \param[out] 
 *    status - a INT_MS_T containing the status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
**/
int msiFtpGet(msParam_t *target, msParam_t *destObj, msParam_t *status, ruleExecInfo_t *rei)
{
	CURL *curl;					/* curl handler */
	CURLcode res;

	dataObjFtpInp_t dataObjFtpInp;			/* custom file descriptor for our callback function */

	dataObjInp_t destObjInp, *myDestObjInp;		/* for parsing input params */
	dataObjCloseInp_t dataObjCloseInp;
	char *target_str;

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
	memset(&dataObjFtpInp, 0, sizeof(dataObjFtpInp_t));
	memset(&destObjInp, 0, sizeof(dataObjInp_t));
	memset(&dataObjCloseInp, 0, sizeof(dataObjCloseInp_t));



	/********************************** PARAM PARSING  *********************************/

	/* parse target URL */
	if ((target_str = parseMspForStr(target)) == NULL)
	{
		rodsLog (LOG_ERROR, "msiFtpGet: input target is NULL.");
		return (USER__NULL_INPUT_ERR);
	}


	/* Get path of destination object */
	rei->status = parseMspForDataObjInp (destObj, &destObjInp, &myDestObjInp, 0);
	if (rei->status < 0)
	{
		rodsLog (LOG_ERROR, "msiFtpGet: input destObj error. status = %d", rei->status);
		return (rei->status);
	}



	/************************** SET UP AND INVOKE CURL HANDLER **************************/

	/* set up dataObjFtpInp */
	strcpy(dataObjFtpInp.objPath, destObjInp.objPath);
	dataObjFtpInp.l1descInx = 0; /* the object is yet to be created */
	dataObjFtpInp.rsComm = rei->rsComm;


	/* curl easy handler init */
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	isCurlErr = 0;

	if(curl) 
	{
		/* Set up curl easy handler */
		curl_easy_setopt(curl, CURLOPT_URL, target_str);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, createAndWriteToDataObj);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dataObjFtpInp);
		
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
	if (dataObjFtpInp.l1descInx)
	{
		dataObjCloseInp.l1descInx = dataObjFtpInp.l1descInx;
		rei->status = rsDataObjClose(rei->rsComm, &dataObjCloseInp);
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





