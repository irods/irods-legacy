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

/* definition for outType */
#define OOI_STR_TYPE		0
#define OOI_DICT_TYPE		1
#define OOI_ARRAY_TYPE		2       /* array of Value */
#if 0	/* just use OOI_ARRAY_TYPE */
#define OOI_DICT_ARRAY_TYPE	2	/* array of dict */
#define OOI_DICT_ARRAY_IN_ARRAY 3       /* An element in an array.
                                         * outInx is the inx in this array */
#endif
typedef struct {
    char servName[NAME_LEN];
    char servOpr[NAME_LEN];
    int outType;
    int outInx;
    dictionary_t params;
} ooiGenServReqInp_t;
   
#define OoiGenServReqInp_PI "str servName[NAME_LEN]; str servOpr[NAME_LEN]; int outType; int outInx; struct Dictionary_PI;"

typedef struct {
    char type_PI[NAME_LEN];   /* the packing instruction of the ptr */
    void *ptr;
} ooiGenServReqOut_t;

#define OoiGenServReqOut_PI "piStr outType_PI[NAME_LEN]; ?outType_PI *ptr;"

/* this struct is used for the post processing of the curl call */

typedef struct {
    int outType;
    int outInx;
    ooiGenServReqOut_t *ooiGenServReqOut;
} ooiGenServReqStruct_t;

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
ooiGenServReqFunc (void *buffer, size_t size, size_t nmemb, void *userp);
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
int
freeOoiGenServReqOut (ooiGenServReqOut_t **ooiGenServReqOut);

#ifdef  __cplusplus
}
#endif

#endif	/* OOI_GEN_SERV_REQ_H */
