/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* ooici.h - header file for general Obj Info
 */


#ifndef OOI_CI_H
#define OOI_CI_H

#include "rods.h"
#include <jansson.h>
#include <curl/curl.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define SERVICE_REQUEST_STR	"serviceRequest"
#define SERVICE_NAME_STR	"serviceName"
#define SERVICE_OP_STR		"serviceOp"
#define PARAMS_STR		"params"
#define ION_SERVICE_STR		"ion-service"
#define OOI_DATA_TAG            "data"  /* tag for HTTP response */
#define OOI_GATEWAY_RESPONSE_TAG "GatewayResponse" /* tag for HTTP response */
#define OOI_GATEWAY_ERROR_TAG "GatewayError" /* tag for HTTP error */


typedef struct DictValue {
    char type_PI[NAME_LEN];   /* the packing instruction of the ptr */
#if 0
    int len;		      /* > 0 if ptr is an array */
    int flags;		      /* not used */
#endif
    void *ptr;
} dictValue_t;

#if 0
#define DictValue_PI "piStr type_PI[NAME_LEN]; int arrLen; int flags; ?type_PI *ptr(arrLen);"
#else
#define DictValue_PI "piStr type_PI[NAME_LEN]; ?type_PI *ptr;"
#endif

typedef struct Dictionary {
    int len;
    char **key;     		/* array of keyword */
    dictValue_t *value;        /* pointer to an array of values */
} dictionary_t;

#define Dictionary_PI "int dictLen; str *key[dictLen]; struct *DictValue_PI(dictLen);" 

/* array of dictionary */
typedef struct DictArray {
    int len;
    int flags;		/* not used */
    dictionary_t *dictionary;
} dictArray_t;

#define DictArray_PI "int dictArrayLen; int flags; struct *Dictionary_PI(dictArrayLen);" 

int
dictSetAttr (dictionary_t *dictionary, char *key, char *type_PI, void *valptr);
dictValue_t *
dictGetAttr (dictionary_t *dictionary, char *key);
int
dictDelAttr (dictionary_t *dictionary, char *key);
int
clearDictionary (dictionary_t *dictionary);
int
jsonPackDictionary (dictionary_t *dictionary, json_t **outObj);
int
jsonPackOoiServReq (char *servName, char *servOpr, dictionary_t *param,
char **outStr);
int
jsonPackOoiServReqForPost (char *servName, char *servOpr, dictionary_t *params,
char **outStr);
int
jsonUnpackOoiRespStr (json_t *responseObj, char **outStr);
int
jsonUnpackOoiRespDict (json_t *responseObj, dictionary_t **outDict);
int
jsonUnpackOoiRespList (json_t *responseObj, dictionary_t **outDict);
int
jsonUnpackDict (json_t *dictObj, dictionary_t *outDict);
int
clearDictArray (dictArray_t *dictArray);
int
_clearDictArray (dictionary_t *dictArray, int len);
int
jsonUnpackOoiRespDictArray (json_t *responseObj, dictArray_t **outDictArray);
int
jsonUnpackOoiRespDictArrInArr (json_t *responseObj, dictArray_t **outDictArray,
int outInx);
int
printDictArray (dictArray_t *dictArray);
int
printDict (dictionary_t *dictionary);
int
printList (dictionary_t *dictionary);
int
jsonUnpackList (json_t *listObj, dictionary_t *outList);
int
getStrByType_PI (char *type_PI, void *valuePtr, char *valueStr);
int
getRevIdFromList (dictionary_t *dictionary, char *objectId, char *outRevId);
#ifdef  __cplusplus
}
#endif

#endif	/* OOI_CI_H */

