/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* curlMS.h - header file for curlMS.c
 */

#ifndef CURLMS_H
#define CURLMS_H

#include "apiHeaderAll.h"
#include "objMetaOpr.h"
#include "miscUtil.h"



int msiCurlGetStr(msParam_t *url, msParam_t *buffer, ruleExecInfo_t *rei);
int msiCurlGetObj(msParam_t *url, msParam_t *object, msParam_t *downloaded, ruleExecInfo_t *rei);
int msiCurlPost(msParam_t *url, msParam_t *postFields, msParam_t *response, ruleExecInfo_t *rei);
int msiTwitterPost(msParam_t *twittername, msParam_t *twitterpass, msParam_t *message, msParam_t *status, ruleExecInfo_t *rei);

/* DEPRECATED */
int msiFtpGet(msParam_t *url, msParam_t *object, msParam_t *status, ruleExecInfo_t *rei);
int msiPostThis(msParam_t *url, msParam_t *data, msParam_t *status, ruleExecInfo_t *rei);


#endif	/* CURLMS_H */

