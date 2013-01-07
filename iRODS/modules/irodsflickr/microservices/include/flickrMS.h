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



int msiFlickrOAuthExample1(msParam_t *input, msParam_t *output, ruleExecInfo_t *rei);
int msiFlickrOAuthExample2(msParam_t *input, msParam_t *output, ruleExecInfo_t *rei);
int msiFlickHarvester(msParam_t *input, msParam_t *output, ruleExecInfo_t *rei);

#endif /* FLICKRMS_H_ */
