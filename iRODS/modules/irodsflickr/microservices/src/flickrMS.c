/*
 *	flickrMS.c
 *
 *  Created on: Feb 3, 2012
 *      Author: AdT
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "flickrMS.h"
#include "cJSON.h"

#include <oauth.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>


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
	char userID[NAME_LEN];
	char apiKey[LONG_NAME_LEN];
	char secret[LONG_NAME_LEN];
} flickrInfo_t;


typedef struct {
	char *callback;
	char *nonce;
	char *timestamp;
	char *consumer_key;
	char *consumer_secret;
	char *signature_method;
	char *signature;
	char *token;
	char *token_secret;
	char *verifier;
	char *version;
	string_t response;		/* CURL read buffer */
} oauth_t;




#if 0
/* Custom message handler callback that writes to iRODS log */
static void myErrorHandler(void *user_data, const char *message)
{
	rodsLog (LOG_ERROR, "cURL error: %s", message);
}


static size_t write_data(void *ptr, size_t size, size_t nmeb, void *stream)
{
	return fwrite(ptr, size, nmeb, (FILE*)stream);
}
#endif


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


/* custom callback function for the cURL handler, to write to an iRODS object */
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


static CURLcode mycurl(const char *url, string_t *string) {
	CURL *curl;							/* curl handler */
	CURLcode res;

	/* cURL easy handler init */
	curl = curl_easy_init();

	if(curl) {
		/* Set up curl easy handler */
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, string);

		/* When debugging */
 //		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* Magic! */
		res = curl_easy_perform(curl);

		/* Cleanup curl handler */
		curl_easy_cleanup(curl);
	}
	else {
		res = CURLE_FAILED_INIT;
	}

	/* Some error logging */
	if(res != CURLE_OK) {
		rodsLog (LOG_ERROR, "mycurl: cURL error: %s", curl_easy_strerror(res));
	}

	return res;
}


static int oauth_init(oauth_t *oauth) {
	oauth->callback = strdup("oob");

	oauth->timestamp = (char*)malloc(TIME_LEN);
	snprintf(oauth->timestamp, TIME_LEN, "%d", (uint)time(NULL));

	oauth->nonce = oauth_gen_nonce();

	oauth->signature_method = strdup("HMAC-SHA1");
	oauth->version = strdup("1.0");

	/* Init string */
	oauth->response.ptr = strdup("");
	oauth->response.len = 0;

	return 0;
}


// Only frees memory allocated by oauth_init()
static int oauth_cleanup(oauth_t *oauth) {
	if (oauth) {
		if (oauth->callback) free(oauth->callback);
		if (oauth->timestamp) free(oauth->timestamp);
		if (oauth->nonce) free(oauth->nonce);
		if (oauth->signature_method) free (oauth->signature_method);
		if (oauth->version) free (oauth->version);
		if (oauth->response.ptr) free (oauth->response.ptr);
	}

	return 0;
}


static int signRequestTokenRequest(oauth_t *oauth) {
	/* Will need to clean that up */
	char my_key[KEY_LEN];
	char *escaped_url, *escaped_params;
	char params[URL_LEN];
	char base_string[URL_LEN];
	/********/

	/* Prepare key */
	// Key = CONSUMER_SECRET + "&" + "" (empty token secret string)
	snprintf(my_key, KEY_LEN, "%s&", oauth->consumer_secret);

	/* Prepare base string */
	// Part 1 is just "GET"

	// Part 2 is escaped url
	escaped_url = oauth_url_escape(REQUEST_TOKEN_URL);

	// Part 3 is escaped parameters
	snprintf(params, URL_LEN,
			"oauth_callback=%s&oauth_consumer_key=%s&oauth_nonce=%s&oauth_signature_method=%s&oauth_timestamp=%s&oauth_version=%s",
			oauth->callback,
			oauth->consumer_key,
			oauth->nonce,
			oauth->signature_method,
			oauth->timestamp,
			oauth->version);
	escaped_params = oauth_url_escape((const char*)params);

	// Join part 1, 2 and 3 together and encode
	snprintf(base_string, URL_LEN, "GET&%s&%s", escaped_url, escaped_params);

	oauth->signature = oauth_sign_hmac_sha1((const char*)base_string, (const char*)my_key);

	// TEST
//	rodsLog (LOG_NOTICE, "base string: %s", base_string);
//	rodsLog (LOG_NOTICE, "oauth->signature: %s", oauth->signature);


	/* Cleanup */
	if (escaped_url) {
		free(escaped_url);
	}
	if (escaped_params) {
		free(escaped_params);
	}

	return 0;
}



static int signAccessTokenRequest(oauth_t *oauth) {
	char my_key[KEY_LEN];
	char *escaped_url, *escaped_params;
	char params[URL_LEN];
	char base_string[URL_LEN];

	/* Prepare key */
	// Key = CONSUMER_SECRET + "&" + REQUEST_TOKEN_SECRET
	snprintf(my_key, KEY_LEN, "%s&%s", oauth->consumer_secret, oauth->token_secret);

	/* Prepare base string */
	// Part 1 is just "GET"

	// Part 2 is escaped url
	escaped_url = oauth_url_escape(ACCESS_TOKEN_URL);

	// Part 3 is escaped parameters
	snprintf(params, URL_LEN,
			"oauth_callback=%s&oauth_consumer_key=%s&oauth_nonce=%s&oauth_signature_method=%s&oauth_timestamp=%s&oauth_token=%s&oauth_verifier=%s&oauth_version=%s",
			oauth->callback,
			oauth->consumer_key,
			oauth->nonce,
			oauth->signature_method,
			oauth->timestamp,
			oauth->token,
			oauth->verifier,
			oauth->version);
	escaped_params = oauth_url_escape((const char*)params);

	// Join part 1, 2 and 3 together and encode
	snprintf(base_string, URL_LEN, "GET&%s&%s", escaped_url, escaped_params);

	oauth->signature = oauth_sign_hmac_sha1((const char*)base_string, (const char*)my_key);

	// TEST
//	rodsLog (LOG_NOTICE, "base string: %s", base_string);
//	rodsLog (LOG_NOTICE, "oauth->signature: %s", oauth->signature);

	/* Cleanup */
	if (escaped_url) {
		free(escaped_url);
	}
	if (escaped_params) {
		free(escaped_params);
	}

	return 0;
}


// Need to add support for arguments (mix with oauth params and alpha sort)
static int signAuthenticatedRequest(oauth_t *oauth, const char *method) {
	char my_key[KEY_LEN];
	char *escaped_url, *escaped_params;
	char params[URL_LEN];
	char base_string[URL_LEN];

	/* Prepare key */
	// Key = CONSUMER_SECRET + "&" + ACCESS_TOKEN_SECRET
	snprintf(my_key, KEY_LEN, "%s&%s", oauth->consumer_secret, oauth->token_secret);

	/* Prepare base string */
	// Part 1 is just "GET"

	// Part 2 is escaped url
	escaped_url = oauth_url_escape(BASE_API_URL);

	// Part 3 is escaped parameters
	snprintf(params, URL_LEN,
			"format=json&method=%s&nojsoncallback=1&oauth_consumer_key=%s&oauth_nonce=%s&oauth_signature_method=%s&oauth_timestamp=%s&oauth_token=%s&oauth_version=%s",
			method,
			oauth->consumer_key,
			oauth->nonce,
			oauth->signature_method,
			oauth->timestamp,
			oauth->token,
			oauth->version);
	escaped_params = oauth_url_escape((const char*)params);

	// Join part 1, 2 and 3 together and encode
	snprintf(base_string, URL_LEN, "GET&%s&%s", escaped_url, escaped_params);

	oauth->signature = oauth_sign_hmac_sha1((const char*)base_string, (const char*)my_key);

	// TEST
//	rodsLog (LOG_NOTICE, "base string: %s", base_string);
//	rodsLog (LOG_NOTICE, "oauth->signature: %s", oauth->signature);

	/* Cleanup */
	if (escaped_url) {
		free(escaped_url);
	}
	if (escaped_params) {
		free(escaped_params);
	}

	return 0;
}


static int getRequestToken(oauth_t *oauth) {
	char url[URL_LEN];


	/* Put full URL together */
	snprintf(url, URL_LEN,
			"%s?oauth_nonce=%s&oauth_timestamp=%s&oauth_consumer_key=%s&oauth_signature_method=%s&oauth_version=%s&oauth_signature=%s&oauth_callback=%s",
			REQUEST_TOKEN_URL,
			oauth->nonce,
			oauth->timestamp,
			oauth->consumer_key,
			oauth->signature_method,
			oauth->version,
			oauth->signature,
			oauth->callback);

	/* CURL call */
	return (int) mycurl(url, &oauth->response);
}


static int getAccessToken(oauth_t *oauth) {
	char url[URL_LEN];


	/* Put full URL together */
	snprintf(url, URL_LEN,
			"%s?oauth_nonce=%s&oauth_timestamp=%s&oauth_token=%s&oauth_verifier=%s&oauth_consumer_key=%s&oauth_signature_method=%s&oauth_version=%s&oauth_signature=%s&oauth_callback=%s",
			ACCESS_TOKEN_URL,
			oauth->nonce,
			oauth->timestamp,
			oauth->token,
			oauth->verifier,
			oauth->consumer_key,
			oauth->signature_method,
			oauth->version,
			oauth->signature,
			oauth->callback);

	/* CURL call */
	return (int) mycurl(url, &oauth->response);
}


static int sendAuthenticatedRequest(oauth_t *oauth, const char *method) {
	char url[URL_LEN];


	/* Put full URL together */
	snprintf(url, URL_LEN,
			"%s?method=%s&oauth_nonce=%s&oauth_timestamp=%s&oauth_token=%s&oauth_consumer_key=%s&oauth_signature_method=%s&oauth_version=%s&oauth_signature=%s&format=json&nojsoncallback=1",
			BASE_API_URL,
			method,
			oauth->nonce,
			oauth->timestamp,
			oauth->token,
			oauth->consumer_key,
			oauth->signature_method,
			oauth->version,
			oauth->signature);

	/* CURL call */
	return (int) mycurl(url, &oauth->response);
}



static int addAVUfromJSON(cJSON *item, char *objType, char *objPath, char *attrName, rsComm_t *rsComm) {
	modAVUMetadataInp_t modAVUMetadataInp;		/* for new AVU creation */
	int status;

	if (!item) {
		rodsLog (LOG_ERROR, "addAVUfromJSON: NULL cJSON input.");
		return SYS_INTERNAL_NULL_INPUT_ERR;
	}

	if (!(item->type == cJSON_String || item->type == cJSON_Number)) {
		rodsLog (LOG_ERROR, "addAVUfromJSON: Invalid cJSON input type: %d", item->type);
		return SYS_INVALID_INPUT_PARAM;
	}

	/* Init modAVU input */
	memset (&modAVUMetadataInp, 0, sizeof(modAVUMetadataInp_t));
	modAVUMetadataInp.arg0 = "add";
	modAVUMetadataInp.arg1 = objType;	// "-d" or "-C"
	modAVUMetadataInp.arg2 = objPath;
	modAVUMetadataInp.arg3 = attrName;
	modAVUMetadataInp.arg5 = "";

	/* Print content of cJSON item. Must free. */
	modAVUMetadataInp.arg4 = cJSON_Print(item);

	/* Invoke rsModAVUMetadata() */
	status = rsModAVUMetadata (rsComm, &modAVUMetadataInp);

	/* Log errors (rsModAVUMetadata() doesn't) */
	if (status < 0) {
		rodsLog (LOG_ERROR,
				"addAVUtoObj: rsModAVUMetadata failed for path: %s, attribute: %s, value: %s. Status = %d",
				objPath, attrName, modAVUMetadataInp.arg4, status);
	}

	/* Cleanup and return */
	if (modAVUMetadataInp.arg4) free(modAVUMetadataInp.arg4);
	return status;
}


// entry is photoset item
static int storePhotosetDescription(cJSON *entry, char *collection, rsComm_t *rsComm) {
	cJSON *description, *_content, *id;
	dataObjInp_t dataObjInp;
	openedDataObjInp_t openedDataObjInp;
	bytesBuf_t writeBBuf;
	int status;


	if (!entry) {
		rodsLog (LOG_ERROR, "storePhotosetDescription: NULL cJSON input.");
		return SYS_INTERNAL_NULL_INPUT_ERR;
	}

	/* Extract photoset description */
	description = cJSON_GetObjectItem(entry, "description");

	if (!description) {
		rodsLog (LOG_ERROR, "storePhotosetDescription: No description item found.");
		return SYS_INVALID_INPUT_PARAM;
	}

	_content = cJSON_GetObjectItem(description, "_content");

	/* If empty don't bother */
	if (!_content->valuestring || !strlen(_content->valuestring)) {
		rodsLog (LOG_NOTICE, "storePhotosetDescription: Empty description found for %s", collection);
		return 0;
	}


	/* Init inputs */
	memset(&dataObjInp, 0, sizeof(dataObjInp_t));
	memset(&openedDataObjInp, 0, sizeof(openedDataObjInp_t));

	snprintf(dataObjInp.objPath, MAX_NAME_LEN, "%s/DESCRIPTION.txt", collection);

	/* Create new object */
	status = rsDataObjCreate (rsComm, &dataObjInp);

	if (status < 0) {
		rodsLog (LOG_ERROR, "storePhotosetDescription: rsDataObjCreate failed for %s, status = %d",
				dataObjInp.objPath, status);
		return status;
	}

	openedDataObjInp.l1descInx = status;

	/* Write to object and close */
	writeBBuf.buf = _content->valuestring;
	openedDataObjInp.len = writeBBuf.len = strlen(_content->valuestring);

	status = rsDataObjWrite (rsComm, &openedDataObjInp, &writeBBuf);

	if (status < 0) {
		rodsLog (LOG_ERROR, "storePhotosetDescription: rsDataObjWrite failed for %s, status = %d",
				dataObjInp.objPath, status);
		return status;
	}

	rodsLog (LOG_NOTICE, "storePhotosetDescription: wrote %d bytes to %s", status, dataObjInp.objPath);

	status = rsDataObjClose (rsComm, &openedDataObjInp);

	if (status < 0) {
		rodsLog (LOG_ERROR, "storePhotosetDescription: rsDataObjClose failed for %s, status = %d",
				dataObjInp.objPath, status);
	}


	/* Add photoset id as metadata to new object */
	id = cJSON_GetObjectItem(entry,"id");
	if (id || id->valuestring) {
		addAVUfromJSON(id, "-d", dataObjInp.objPath, "photoset id", rsComm);
	}

	return 0;
}


static int getPhoto(cJSON *entry, char *collection, rsComm_t *rsComm) {
	cJSON *url;

	CURL *curl;								/* curl handler */
	CURLcode res;

	rodsLong_t data_id;						/* data ID, to check for existing objects */

	writeDataInp_t writeDataInp;			/* custom file descriptor for our callback function */
	openedDataObjInp_t openedDataObjInp;	/* to close iRODS object after writing */

	char *filename;
	int status;


	if (!entry) {
		rodsLog (LOG_ERROR, "getPhoto: NULL entry.");
		return SYS_INTERNAL_NULL_INPUT_ERR;
	}

	if (!collection) {
		rodsLog (LOG_ERROR, "getPhoto: NULL collection.");
		return SYS_INTERNAL_NULL_INPUT_ERR;
	}


	/*** Get photo file ***/

	/* Extract photo URL */
	/* If url_o is not available try for url_l */
	if ( !(url = cJSON_GetObjectItem(entry, "url_o") )) {
		url = cJSON_GetObjectItem(entry, "url_l");
	}

	if (!url || !url->valuestring) {
		rodsLog (LOG_ERROR, "getPhoto: No download URL found.");
		return SYS_INVALID_INPUT_PARAM;
	}

	/* Extract file name from URL */
	filename = strrchr(url->valuestring, '/');
	if (!filename) {
		rodsLog (LOG_ERROR, "getPhoto: Unable to get filename from URL %s", url->valuestring);
		return SYS_INVALID_INPUT_PARAM;
	}

	filename++;

	/* pad data structures with null chars */
	memset(&writeDataInp, 0, sizeof(writeDataInp_t));
	memset(&openedDataObjInp, 0, sizeof(openedDataObjInp_t));


	/* set up writeDataInp */
	snprintf(writeDataInp.objPath, MAX_NAME_LEN, "%s/%s", collection, filename);


	/* Skip that file if we already have it */
	if (isData(rsComm, writeDataInp.objPath, &data_id) >= 0)
	{
		rodsLog (LOG_ERROR, "getPhoto: %s already exists.", writeDataInp.objPath);
		return CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME;
	}


	writeDataInp.l1descInx = 0; /* the object is yet to be created */
	writeDataInp.rsComm = rsComm;


	/* curl easy handler init */
	curl = curl_easy_init();

	if(curl)
	{
		/* Set up curl easy handler */
		curl_easy_setopt(curl, CURLOPT_URL, url->valuestring);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, createAndWriteToDataObj);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeDataInp);

		/* For debugging only */
// 		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* let curl do its thing */
		res = curl_easy_perform(curl);

		/* cleanup curl handler */
		curl_easy_cleanup(curl);

		/* how did it go? */
		if(res != CURLE_OK)
		{
			rodsLog (LOG_ERROR, "getPhoto: libcurl error: %d", res);
			status = ACTION_FAILED_ERR;
		}
	} else {
		rodsLog (LOG_ERROR, "getPhoto: curl_easy_init() error");
		status = ACTION_FAILED_ERR;
	}

	if (status < 0) {
		return status;
	}


	rodsLog (LOG_NOTICE, "Created %s, status = %d", writeDataInp.objPath, status);


	/* close iRODS object */
	if (writeDataInp.l1descInx)
	{
		openedDataObjInp.l1descInx = writeDataInp.l1descInx;
		status = rsDataObjClose(rsComm, &openedDataObjInp);
		if (status < 0) {
			rodsLog (LOG_ERROR, "getPhoto: rsDataObjClose failed for %s, status = %d",
					writeDataInp.objPath, status);
		}
	}


	/*** Extract and add metadata to photo object ***/

	addAVUfromJSON(cJSON_GetObjectItem(entry,"id"), "-d", writeDataInp.objPath, "id", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"title"), "-d", writeDataInp.objPath, "title", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"isprimary"), "-d", writeDataInp.objPath, "isprimary", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"datetaken"), "-d", writeDataInp.objPath, "datetaken", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"datetakengranularity"), "-d", writeDataInp.objPath, "datetakengranularity", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"ownername"), "-d", writeDataInp.objPath, "ownername", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"originalformat"), "-d", writeDataInp.objPath, "originalformat", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"latitude"), "-d", writeDataInp.objPath, "latitude", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"longitude"), "-d", writeDataInp.objPath, "longitude", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"accuracy"), "-d", writeDataInp.objPath, "accuracy", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"context"), "-d", writeDataInp.objPath, "context", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"tags"), "-d", writeDataInp.objPath, "tags", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"machine_tags"), "-d", writeDataInp.objPath, "machine_tags", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"o_width"), "-d", writeDataInp.objPath, "o_width", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"o_height"), "-d", writeDataInp.objPath, "o_height", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"media"), "-d", writeDataInp.objPath, "media", rsComm);
	addAVUfromJSON(cJSON_GetObjectItem(entry,"media_status"), "-d", writeDataInp.objPath, "media_status", rsComm);


	/*** Done ***/

	return 0;
}



// collection is photoset collection
// id is photoset ID
static int getPhotosInSet(oauth_t *oauth, cJSON *set_id, char *collection, rsComm_t *rsComm) {
	char *method = "flickr.photosets.getPhotos";
	char *extras = "date_taken, owner_name, original_format, geo, tags, machine_tags, o_dims, media, url_l, url_o";
	char *escaped_extras = NULL;
	char *photoset_id;

	/* for signing and making call */
	char my_key[KEY_LEN];
	char *escaped_url, *escaped_params;
	char params[URL_LEN];
	char base_string[URL_LEN];
	char url[URL_LEN];

	/* for processing photos */
	cJSON *json_root = NULL;
	cJSON *photoset, *photo, *entry;
	int i, status, pic_count = 0;


	/* Check input */
	if (!set_id || !set_id->valuestring) {
		return SYS_INVALID_INPUT_PARAM;
	}

	photoset_id = set_id->valuestring;

	oauth_cleanup(oauth);
	oauth_init(oauth);


	/*** Sign call ***/
	/* Prepare key */
	// Key = CONSUMER_SECRET + "&" + REQUEST_TOKEN_SECRET
	snprintf(my_key, KEY_LEN, "%s&%s", oauth->consumer_secret, oauth->token_secret);

	/* Prepare base string */
	// Part 1 is just "GET"

	// Part 2 is escaped url
	escaped_url = oauth_url_escape(BASE_API_URL);

	// Part 3 is escaped parameters
	snprintf(params, URL_LEN,
			"extras=%s&format=json&method=%s&nojsoncallback=1&oauth_consumer_key=%s&oauth_nonce=%s&oauth_signature_method=%s&oauth_timestamp=%s&oauth_token=%s&oauth_version=%s&photoset_id=%s",
			extras,
			method,
			oauth->consumer_key,
			oauth->nonce,
			oauth->signature_method,
			oauth->timestamp,
			oauth->token,
			oauth->version,
			photoset_id);
	escaped_params = oauth_url_escape((const char*)params);

	// Join part 1, 2 and 3 together and encode
	snprintf(base_string, URL_LEN, "GET&%s&%s", escaped_url, escaped_params);

	oauth->signature = oauth_sign_hmac_sha1((const char*)base_string, (const char*)my_key);

	/* Cleanup */
	if (escaped_url) free(escaped_url);
	if (escaped_params) free(escaped_params);


	/*** Make call ***/
	/* Put full URL together */
	escaped_extras = oauth_url_escape(extras);
	snprintf(url, URL_LEN,
			"%s?method=%s&photoset_id=%s&extras=%s&oauth_nonce=%s&oauth_timestamp=%s&oauth_token=%s&oauth_consumer_key=%s&oauth_signature_method=%s&oauth_version=%s&oauth_signature=%s&format=json&nojsoncallback=1",
			BASE_API_URL,
			method,
			photoset_id,
			escaped_extras,
			oauth->nonce,
			oauth->timestamp,
			oauth->token,
			oauth->consumer_key,
			oauth->signature_method,
			oauth->version,
			oauth->signature);

	/* CURL call */
	mycurl(url, &oauth->response);

	if (escaped_extras) free(escaped_extras);


	/*** Process photo list ***/
	/* parse response as JSON */
	json_root = cJSON_Parse(oauth->response.ptr);

	if (!json_root) {
		rodsLog (LOG_ERROR, "getPhotosInSet: Unable to get photos in set %s. cJSON_Parse error.", photoset_id);
		return ACTION_FAILED_ERR;
	}

	photoset = cJSON_GetObjectItem(json_root,"photoset");
	if (!photoset) {
		return 0;
	}

	photo = cJSON_GetObjectItem(photoset,"photo");
	if (!photo) {
		return 0;
	}

//	rodsLog (LOG_NOTICE, "Found %d photos in set %s", cJSON_GetArraySize(photo), photoset_id);

	/* Extract nested JSON elements */
	for (i=0; i<cJSON_GetArraySize(photo); i++) {
		entry = cJSON_GetArrayItem(photo,i);
		status = getPhoto(entry, collection, rsComm);
		if (!status) {
			pic_count++;
		}
	}

	// Until paging is properly handled
	if (i>500) {
		rodsLog (LOG_ERROR, "getPhotosInSet: Set %s larger than 500 pics, likely incomplete", photoset_id);
	}


	return pic_count;
}



static int getPhotoset(oauth_t *oauth, cJSON *entry, char *flickrRootCollection, rsComm_t *rsComm) {
	cJSON *title, *_content = NULL, *id, *primary, *photos, *videos;
	collInp_t collCreateInp;
	int status;


	if (!entry) {
		return SYS_INTERNAL_NULL_INPUT_ERR;
	}


	/* Init collCreateInp */
	memset(&collCreateInp, 0, sizeof(collInp_t));


	/********************* Make collection *********************/

	/* Use ID for collection name to avoid invalid chars */
	id = cJSON_GetObjectItem(entry,"id");
	if (!id || !id->valuestring) {
		return SYS_INVALID_INPUT_PARAM;
	}

	snprintf(collCreateInp.collName, MAX_NAME_LEN, "%s/%s", flickrRootCollection, id->valuestring);

	/* Test if collection already exists */
	if (isColl(rsComm, collCreateInp.collName, NULL) != CAT_NO_ROWS_FOUND) {
		rodsLog (LOG_NOTICE, "makePhotosetCollection: skipping existing collection %s", collCreateInp.collName);
		return 0;
	}

	/* Create intermediate collections if needed */
	addKeyVal (&collCreateInp.condInput, RECURSIVE_OPR__KW, "");

	/* Collection creation */
	status = rsCollCreate (rsComm, &collCreateInp);
	if (status < 0) {
		rodsLog (LOG_ERROR, "makePhotosetCollection: Unable to create collection %s, status = %d", collCreateInp.collName, status);
		return ACTION_FAILED_ERR;
	}


	/********************* Add Metadata *********************/

	/* Extract photoset title */
	title = cJSON_GetObjectItem(entry, "title");
	if (title) {
		_content = cJSON_GetObjectItem(title, "_content");
	}


	/* Extract and add metadata */
	id = cJSON_GetObjectItem(entry,"id");
	primary = cJSON_GetObjectItem(entry,"primary");
	photos = cJSON_GetObjectItem(entry,"photos");
	videos = cJSON_GetObjectItem(entry,"videos");


	addAVUfromJSON(_content, "-C", collCreateInp.collName, "photoset title", rsComm);
	addAVUfromJSON(id, "-C", collCreateInp.collName, "photoset id", rsComm);
	addAVUfromJSON(primary, "-C", collCreateInp.collName, "primary photo id", rsComm);
	addAVUfromJSON(photos, "-C", collCreateInp.collName, "photos", rsComm);
	addAVUfromJSON(videos, "-C", collCreateInp.collName, "videos", rsComm);

//	rodsLog (LOG_NOTICE, "done %s", collCreateInp.collName);


	/********************* Store description in separate text file *********************/

	storePhotosetDescription(entry, collCreateInp.collName, rsComm);


	/********************* Get photos in photoset *********************/

	// TEST: get contents of only one set
//	if (strcmp(id->valuestring, "72157631882531637")) {
//		return 0;
//	}
	// TEST

	status = getPhotosInSet(oauth, id, collCreateInp.collName, rsComm);



	return status;

}


static int getAllPhotosets(oauth_t *oauth, char *flickrRootCollection, rsComm_t *rsComm) {
	cJSON *json_root = NULL;
	cJSON *photosets, *photoset, *entry;
	int i, status, pic_count = 0;


	/* Sign and send API call */
	signAuthenticatedRequest(oauth, "flickr.photosets.getList");
	status = sendAuthenticatedRequest(oauth, "flickr.photosets.getList");

	if (status) {		/* CURL Error if non zero */
		rodsLog (LOG_ERROR, "%s", "getPhotosets: Flickr call failed.");
		return ACTION_FAILED_ERR;
	}

	/* parse response as JSON (original string can be freed) */
	json_root = cJSON_Parse(oauth->response.ptr);

	if (!json_root) {
		rodsLog (LOG_ERROR, "%s", "getPhotosets: Unable to get photosets list. cJSON_Parse error.");
		return ACTION_FAILED_ERR;
	}

	/* Now parse photosets list from JSON object */
	photosets = cJSON_GetObjectItem(json_root,"photosets");
	if (!photosets) {
		return 0;
	}

	photoset = cJSON_GetObjectItem(photosets,"photoset");
	if (!photoset) {
		return 0;
	}

	/* Extract nested JSON elements */
	for (i=0; i<cJSON_GetArraySize(photoset); i++) {
		//rodsLog (LOG_NOTICE, "%s", cJSON_Print(cJSON_GetArrayItem(photoset,i)));
		entry = cJSON_GetArrayItem(photoset,i);

		status = getPhotoset(oauth, entry, flickrRootCollection, rsComm);
		if (status > 0) pic_count+=status;

	}

	return pic_count;

}


// Request a request token from Flickr
// Get keys from input keyValPair
int msiFlickrOAuthExample1(msParam_t *input, msParam_t *output, ruleExecInfo_t *rei) {
	oauth_t oauth;						/* OAuth params */
	keyValPair_t *KVP;					/* will be output->inOutStruct */
	char authorization_url[URL_LEN];
	char *tokenPtr, *secretPtr, *sep;
	int status;


	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiFlickrOAuthExample1")

	/* Sanity checks */
	if (!rei || !rei->rsComm)
	{
		rodsLog (LOG_ERROR, "msiFlickrOAuthExample1: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	/* Check for proper input type */
	if (!input->type || strcmp(input->type, KeyValPair_MS_T)) {
		rodsLog (LOG_ERROR, "msiFlickrOAuthExample1: input must be KeyValPair_MS_T.");
		return (USER_PARAM_TYPE_ERR);
	}


	/* Init oauth params */
	oauth_init(&oauth);

	/* Get API key and secret from input params */
	oauth.consumer_key = getValByKey((keyValPair_t *)input->inOutStruct, "consumer_key");
	oauth.consumer_secret = getValByKey((keyValPair_t *)input->inOutStruct, "consumer_secret");


	/*** GENERATE SIGNATURE ***/
	signRequestTokenRequest(&oauth);


	/*** ASK FOR REQUEST TOKEN ***/
	status = getRequestToken(&oauth);

	if (status) {		// CURL Error if non zero
		oauth_cleanup(&oauth);
		return ACTION_FAILED_ERR;
	}


	/* Prepare output struct */
	KVP = (keyValPair_t*)malloc(sizeof(keyValPair_t));
	memset(KVP, 0, sizeof(keyValPair_t));

	/* Return copy of response string before modifying it  */
	addKeyVal(KVP, "request token response", oauth.response.ptr);


	/* Extract token and secret from response string */
	tokenPtr = strstr(oauth.response.ptr, "oauth_token=");
	secretPtr = strstr(oauth.response.ptr, "oauth_token_secret=");

	if (tokenPtr && secretPtr) {
		/* Replace separators with null chars */
		if ((sep = strstr(tokenPtr, "&"))) {
			sep[0]='\0';
		}
		if ((sep = strstr(secretPtr, "&"))) {
			sep[0]='\0';
		}

		tokenPtr+=strlen("oauth_token=");
		secretPtr+=strlen("oauth_token_secret=");

		addKeyVal(KVP, "token", tokenPtr);
		addKeyVal(KVP, "token_secret", secretPtr);

		/* Also send out authorization URL for convenience */
		snprintf(authorization_url, URL_LEN, "%s?oauth_token=%s",
				AUTHORIZE_URL, tokenPtr);
		addKeyVal(KVP, "authorization_url", authorization_url);
	}


	/* Pass KVP out */
	output->type = strdup(KeyValPair_MS_T);
	output->inOutStruct = (void*)KVP;

	/* Cleanup and done */
	oauth_cleanup(&oauth);
	return 0;
}



// Exchange a request token for an access token from Flickr
int msiFlickrOAuthExample2(msParam_t *input, msParam_t *output, ruleExecInfo_t *rei) {
	oauth_t oauth;						/* OAuth params */
	keyValPair_t *KVP;					/* will be output->inOutStruct */
	char *tokenPtr, *secretPtr, *sep;
	int status;


	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiFlickrOAuthExample2")

	/* Sanity checks */
	if (!rei || !rei->rsComm)
	{
		rodsLog (LOG_ERROR, "msiFlickrOAuthExample2: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	/* Check for proper input type */
	if (!input->type || strcmp(input->type, KeyValPair_MS_T)) {
		rodsLog (LOG_ERROR, "msiFlickrOAuthExample2: input must be KeyValPair_MS_T.");
		return (USER_PARAM_TYPE_ERR);
	}


	/* Init oauth */
	oauth_init(&oauth);

	/* Get API key and secret from input params */
	oauth.consumer_key = getValByKey((keyValPair_t *)input->inOutStruct, "consumer_key");
	oauth.consumer_secret = getValByKey((keyValPair_t *)input->inOutStruct, "consumer_secret");

	/* Get request token, request token secret, and verifier from input params */
	oauth.token = getValByKey((keyValPair_t *)input->inOutStruct, "token");
	oauth.token_secret = getValByKey((keyValPair_t *)input->inOutStruct, "token_secret");
	oauth.verifier = getValByKey((keyValPair_t *)input->inOutStruct, "verifier");


	/*** GENERATE SIGNATURE ***/
	signAccessTokenRequest(&oauth);


	/*** ASK FOR ACCESS TOKEN ***/
	status = getAccessToken(&oauth);

	if (status) {		// CURL Error if non zero
		oauth_cleanup(&oauth);
		return ACTION_FAILED_ERR;
	}


	/* Prepare output struct */
	KVP = (keyValPair_t*)malloc(sizeof(keyValPair_t));
	memset(KVP, 0, sizeof(keyValPair_t));

	/* Return copy of response string before modifying it  */
	addKeyVal(KVP, "request token response", oauth.response.ptr);


	/* Extract token and secret from response string */
	tokenPtr = strstr(oauth.response.ptr, "oauth_token=");
	secretPtr = strstr(oauth.response.ptr, "oauth_token_secret=");

	if (tokenPtr && secretPtr) {
		/* Replace separators with null chars */
		if ((sep = strstr(tokenPtr, "&"))) {
			sep[0]='\0';
		}
		if ((sep = strstr(secretPtr, "&"))) {
			sep[0]='\0';
		}

		tokenPtr+=strlen("oauth_token=");
		secretPtr+=strlen("oauth_token_secret=");

		addKeyVal(KVP, "token", tokenPtr);
		addKeyVal(KVP, "token_secret", secretPtr);
	}


	/* Pass KVP out */
	output->type = strdup(KeyValPair_MS_T);
	output->inOutStruct = (void*)KVP;

	/* Cleanup and done */
	oauth_cleanup(&oauth);
	return 0;

}


int msiFlickHarvester(msParam_t *input, msParam_t *output, ruleExecInfo_t *rei) {
	oauth_t oauth;						/* OAuth params */
	keyValPair_t *KVP;					/* will be output->inOutStruct */
	char *flickrRootCollection, zoneInPath[NAME_LEN+2], mystr[20];
	int status;

	/* For testing mode when used with irule --test */
	RE_TEST_MACRO ("    Calling msiFlickHarvester")

	/* Sanity checks */
	if (!rei || !rei->rsComm)
	{
		rodsLog (LOG_ERROR, "msiFlickHarvester: input rei or rsComm is NULL.");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	/* Check for proper input type */
	if (!input->type || strcmp(input->type, KeyValPair_MS_T)) {
		rodsLog (LOG_ERROR, "msiFlickHarvester: input must be KeyValPair_MS_T.");
		return (USER_PARAM_TYPE_ERR);
	}


	/* Get target collection */
	flickrRootCollection = getValByKey((keyValPair_t *)input->inOutStruct, "target_collection");
	if (!flickrRootCollection || strlen(flickrRootCollection) > MAX_NAME_LEN) {
		rodsLog (LOG_ERROR, "msiFlickHarvester: invalid target_collection input.");
		return (USER_INPUT_STRING_ERR);
	}

	/* Additional sanity test on target collection */
	snprintf(zoneInPath, NAME_LEN+2, "/%s/", rei->rsComm->myEnv.rodsZone);
	if (strstr(flickrRootCollection, zoneInPath) != flickrRootCollection) {
		rodsLog (LOG_ERROR, "msiFlickHarvester: invalid zone in target_collection.");
		return (USER_INPUT_PATH_ERR);
	}


	/* Init oauth */
	oauth_init(&oauth);


	/* Get API key and secret from input */
	oauth.consumer_key = getValByKey((keyValPair_t *)input->inOutStruct, "consumer_key");
	oauth.consumer_secret = getValByKey((keyValPair_t *)input->inOutStruct, "consumer_secret");

	/* Get access token and access token secret from input */
	oauth.token = getValByKey((keyValPair_t *)input->inOutStruct, "token");
	oauth.token_secret = getValByKey((keyValPair_t *)input->inOutStruct, "token_secret");


	/************************* GET PHOTOSETS *************************/

	// A positive return value is nbr of pics downloaded
	status = getAllPhotosets(&oauth, flickrRootCollection, rei->rsComm);

	if (status < 0) {
		oauth_cleanup(&oauth);
		rodsLog (LOG_ERROR, "%s", "msiFlickHarvester: getPhotosets() error.");
		return ACTION_FAILED_ERR;
	}


	/************************* RETURN STUFF *************************/

	// No need to be too verbose here since rule will probably be run in the background
	// and subroutines are writing enough to the log already...

	/* Prepare output struct */
	KVP = (keyValPair_t*)malloc(sizeof(keyValPair_t));
	memset(KVP, 0, sizeof(keyValPair_t));

	/* Return pic count */
	snprintf(mystr, 20, "%d", status);
	addKeyVal(KVP, "Photos downloaded", mystr);

	/* Pass KVP out */
	output->type = strdup(KeyValPair_MS_T);
	output->inOutStruct = (void*)KVP;


	/* Cleanup and done */
	oauth_cleanup(&oauth);
	return 0;
}




