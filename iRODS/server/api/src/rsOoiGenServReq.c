/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See ooiGenServReq.h for a description of this API call.*/
#include "ooiGenServReq.h"
#include "rodsLog.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "rsApiHandler.h"
#include "specColl.h"
#include "getRemoteZoneResc.h"

/* XXXX these will be defined in a OOI resource */
#define OOI_GATEWAY_URL "http://localhost"
#define OOI_GATEWAY_PORT                "5000"

int
rsOoiGenServReq (rsComm_t *rsComm, ooiGenServReqInp_t *ooiGenServReqInp,
ooiGenServReqOut_t **ooiGenServReqOut)
{
    int status;

    status = _rsOoiGenServReq (rsComm, ooiGenServReqInp, ooiGenServReqOut);

    return status;
}

int
_rsOoiGenServReq (rsComm_t *rsComm, ooiGenServReqInp_t *ooiGenServReqInp,
ooiGenServReqOut_t **ooiGenServReqOut)
{
    CURL *easyhandle;
    CURLcode res;
    char myUrl[MAX_NAME_LEN];
    int status;
    char *postStr = NULL;
    ooiGenServReqStruct_t ooiGenServReqStruct;

    easyhandle = curl_easy_init();
    if(!easyhandle) {
        rodsLog (LOG_ERROR, 
          "_rsOoiGenServReq: curl_easy_init error");
        return OOI_CURL_EASY_INIT_ERR;
    }
    status = jsonPackOoiServReqForPost (ooiGenServReqInp->servName,
                                        ooiGenServReqInp->servOpr,
                                        &ooiGenServReqInp->params, &postStr);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "_rsOoiGenServReq: jsonPackOoiServReq error");
        return status;
    }

    snprintf (myUrl, MAX_NAME_LEN, "%s:%s/%s/%s/%s",
      OOI_GATEWAY_URL, OOI_GATEWAY_PORT, ION_SERVICE_STR, 
        ooiGenServReqInp->servName, ooiGenServReqInp->servOpr);

    curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, postStr);
    curl_easy_setopt(easyhandle, CURLOPT_URL, myUrl);
    curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, ooiGenServReqFunc);
    bzero (&ooiGenServReqStruct, sizeof (ooiGenServReqStruct));
    ooiGenServReqStruct.outType = ooiGenServReqInp->outType;
    ooiGenServReqStruct.outInx = ooiGenServReqInp->outInx;

    curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &ooiGenServReqStruct);

    res = curl_easy_perform (easyhandle);
    free (postStr);

    if (res != CURLE_OK) {
	/* res is +ive for error */
        rodsLog (LOG_ERROR, 
          "_rsOoiGenServReq: curl_easy_perform error: %d", res);
	free (*ooiGenServReqOut);
        *ooiGenServReqOut = NULL;
        return OOI_CURL_EASY_PERFORM_ERR - res;
    }

    return 0;
}

size_t
ooiGenServReqFunc (void *buffer, size_t size, size_t nmemb, void *userp)
{
    char *type_PI;
    int status;
    void *ptr = NULL;
    ooiGenServReqStruct_t *ooiGenServReqStruct = 
      (ooiGenServReqStruct_t *) userp;

    switch (ooiGenServReqStruct->outType) {
      case OOI_STR_TYPE:
	type_PI = STR_MS_T;
        status = jsonUnpackOoiRespStr (buffer, (char **) &ptr);
        break;
      case OOI_DICT_TYPE:
	type_PI = Dictionary_MS_T;
        status = jsonUnpackOoiRespDict (buffer, (dictionary_t **) &ptr);
        break;
      case OOI_DICT_ARRAY_TYPE:
	type_PI = DictArray_MS_T;
        status = jsonUnpackOoiRespDictArray (buffer, (dictArray_t **) &ptr);
        break;
      case OOI_DICT_ARRAY_IN_ARRAY:
	type_PI = DictArray_MS_T;
       status = jsonUnpackOoiRespDictArrInArr (buffer, (dictArray_t **) &ptr,
         ooiGenServReqStruct->outInx);
        break;
      default:
        rodsLog (LOG_ERROR,
          "ooiGenServReqFunc: outType %d not supported", 
          ooiGenServReqStruct->outType);
        status = OOI_JSON_TYPE_ERR;
    }
    if (status < 0) return 0;

    ooiGenServReqStruct->ooiGenServReqOut =
      (ooiGenServReqOut_t *) calloc (1, sizeof (ooiGenServReqOut_t));

    rstrcpy (ooiGenServReqStruct->ooiGenServReqOut->type_PI, type_PI, NAME_LEN);
    ooiGenServReqStruct->ooiGenServReqOut->ptr = ptr;

    return nmemb*size;
}

