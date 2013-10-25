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

#define JSON
#ifndef JSONMS_H
#define JSONMS_H
#define RE_JSON_ERROR RE_RUNTIME_ERROR 
#define JSON_MS_T "JSON_PI"
#include <string.h>
#include <jansson.h>
#include "apiHeaderAll.h"
#include "objMetaOpr.h"
#include "miscUtil.h"

json_t *parseMspForJson(msParam_t *inpParam);
Res *msiParseJSON(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r);
Res *msiFreeJSON(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r);
Res *msiJSONObjectGet(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r);
Res *msiJSONArraySize(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r);
Res *msiJSONArrayGet(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r);
Res *msiJSONStringValue(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r);
Res *msiJSONIntegerValue(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r);
Res *msiJSONObjectKeys(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r);
#endif

/* DEPRECATED */
int msiFtpGet(msParam_t *url, msParam_t *object, msParam_t *status, ruleExecInfo_t *rei);
int msiPostThis(msParam_t *url, msParam_t *data, msParam_t *status, ruleExecInfo_t *rei);


#endif	/* CURLMS_H */

