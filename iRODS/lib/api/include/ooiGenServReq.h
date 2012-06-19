/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* ooiGenServReq.h
 */

#ifndef OOI_GEN_SERV_REQ_H
#define OOI_GEN_SERV_REQ_H

/* This is a General OOI service request  API call */

#include "rods.h"
#include "ooiCi.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"

#define ION_SERVICE                     "ion-service"

typedef struct {
    char servName[NAME_LEN];
    char servOpr[NAME_LEN];
    dictionary_t params;
} ooiGenServReqInp_t;
   
#define OoiGenServReqInp_PI "str servName[NAME_LEN]; str servOpr[NAME_LEN]; struct Dictionary_PI;"

typedef dictValue_t ooiGenServReqOut_t;

#if defined(RODS_SERVER)
#define RS_OOI_GEN_SERV_REQ rsOoiGenServReq
/* prototype for the server handler */
int
rsOoiGenServReq (rsComm_t *rsComm, ooiGenServReqInp_t *ooiGenServReqInp, 
ooiGenServReqOut_t **ooiGenServReqOut);
int
_rsOoiGenServReq (rsComm_t *rsComm, ooiGenServReqInp_t *ooiGenServReqInp,
ooiGenServReqOut_t **ooiGenServReqOut);
size_t
ooiGenServReqOutFunc (void *buffer, size_t size, size_t nmemb, void *userp);
#else
#define RS_OOI_GEN_SERV_REQ NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* rcOoiGenServReq - 
 * Input - 
 *   rcComm_t *conn - The client connection handle.
 *   ooiGenServReqInp_t struct:
 *     servName - OOI service name
 *     servOpr - which operation of the OOI service 
 *     params - input parameters in key/value dictionary 
 * OutPut - ncInqGrpOut_t.
 */
/* prototype for the client call */
int
rcOoiGenServReq (rcComm_t *conn, ooiGenServReqInp_t *ooiGenServReqInp, 
ooiGenServReqOut_t **ooiGenServReqOut);
#ifdef  __cplusplus
}
#endif

#endif	/* OOI_GEN_SERV_REQ_H */
