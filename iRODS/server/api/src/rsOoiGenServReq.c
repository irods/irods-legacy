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
    curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, ooiGenServReqOutFunc);
    *ooiGenServReqOut = (ooiGenServReqOut_t *)
      calloc (1, sizeof (ooiGenServReqOut_t));
    curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, *ooiGenServReqOut);

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
ooiGenServReqOutFunc (void *buffer, size_t size, size_t nmemb, void *userp)
{
    return nmemb*size;
}

