/*
 *	flickrMS.h
 *
 *  Created on: Feb 3, 2012
 *      Author: AdT
 */

#ifndef FLICKRMS_H_
#define FLICKRMS_H_

#include "apiHeaderAll.h"
#include "objMetaOpr.h"
#include "miscUtil.h"


#define REQUEST_TOKEN_URL	"http://www.flickr.com/services/oauth/request_token"
#define ACCESS_TOKEN_URL	"http://www.flickr.com/services/oauth/access_token"
#define AUTHORIZE_URL		"http://www.flickr.com/services/oauth/authorize"
#define BASE_API_URL		"http://api.flickr.com/services/rest"
#define KEY_LEN				256
#define URL_LEN				1024
#define OUT_MSG_LEN			1024*5



typedef struct {
	char objPath[MAX_NAME_LEN];
	int l1descInx;
	rsComm_t *rsComm;
} writeDataInp_t;


typedef struct {
  char *ptr;
  size_t len;		/* not counting terminating null char */
} string_t;


typedef struct {
	char userID[NAME_LEN];
	char apiKey[LONG_NAME_LEN];
	char secret[LONG_NAME_LEN];
} flickrInfo_t;


//typedef struct {
//	char *token;
//	char *secret;
//} oauth_token_t;


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



int msiFlickrOAuthExample1(msParam_t *input, msParam_t *output, ruleExecInfo_t *rei);
int msiFlickrOAuthExample2(msParam_t *input, msParam_t *output, ruleExecInfo_t *rei);
int msiFlickHarvester(msParam_t *input, msParam_t *output, ruleExecInfo_t *rei);

#endif /* FLICKRMS_H_ */
