/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See ncInq.h for a description of this API call.*/
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
    char postStr[MAX_NAME_LEN];
    int status;

    easyhandle = curl_easy_init();
    if(!easyhandle) {
        rodsLog (LOG_ERROR, 
          "_rsOoiGenServReq: curl_easy_init error");
        return OOI_CURL_EASY_INIT_ERR;
    }
    return 0;
}

