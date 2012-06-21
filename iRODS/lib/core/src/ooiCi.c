/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* ooiCi.c - OOI CI routines
 */
#include "ooiCi.h"
#include "msParam.h"

/* dictSetAttr - set a key/value pair. For non array, arrLen = 0 */ 
int
dictSetAttr (dictionary_t *dictionary, char *key, char *type_PI, void *valptr,
int arrLen)
{
    /* key and type_PI are replicated, but valptr is stolen */
    char **newKey;
    dictValue_t *newValue;
    int newLen;
    int i;

    if (dictionary == NULL) {
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    /* check if the keyword exists */

    for (i = 0; i < dictionary->len; i++) {
        if (strcmp (key, dictionary->key[i]) == 0) {
            free ( dictionary->value[i].ptr);
            dictionary->value[i].ptr = valptr;
            rstrcpy (dictionary->value[i].type_PI, type_PI, NAME_LEN);
            dictionary->value[i].len = arrLen;
            return (0);
        }
    }

    if ((dictionary->len % PTR_ARRAY_MALLOC_LEN) == 0) {
        newLen = dictionary->len + PTR_ARRAY_MALLOC_LEN;
        newKey = (char **) calloc (newLen, sizeof (newKey));
        newValue = (dictValue_t *) calloc (newLen,  sizeof (dictValue_t));
        for (i = 0; i < dictionary->len; i++) {
            newKey[i] = dictionary->key[i];
            newValue[i] = dictionary->value[i];
        }
        if (dictionary->key != NULL)
            free (dictionary->key);
        if (dictionary->value != NULL)
            free (dictionary->value);
        dictionary->key = newKey;
        dictionary->value = newValue;
    }

    dictionary->key[dictionary->len] = strdup (key);
    dictionary->value[dictionary->len].ptr = valptr;
    rstrcpy (dictionary->value[dictionary->len].type_PI, type_PI, NAME_LEN);
    dictionary->value[dictionary->len].len = arrLen;
    dictionary->len++;

    return (0);
}

dictValue_t *
dictGetAttr (dictionary_t *dictionary, char *key)
{
    int i;

    if (dictionary == NULL) {
        return (NULL);
    }

    for (i = 0; i < dictionary->len; i++) {
        if (strcmp (dictionary->key[i], key) == 0) {
            return (&dictionary->value[i]);
        }
    }
    return (NULL);
}

int
dictDelAttr (dictionary_t *dictionary, char *key) 
{
    int i, j;

    if (dictionary == NULL) {
        return (0);
    }

    for (i = 0; i < dictionary->len; i++) {
        if (dictionary->key[i] != NULL &&
          strcmp (dictionary->key[i], key) == 0) {
            free (dictionary->key[i]);
	    if (dictionary->value[i].ptr != NULL) {
                free (dictionary->value[i].ptr);
		dictionary->value[i].ptr = NULL;
            }
            dictionary->len--;
            for (j = i; j < dictionary->len; j++) {
                dictionary->key[j] = dictionary->key[j + 1];
                dictionary->value[j] = dictionary->value[j + 1];
            }
            if (dictionary->len <= 0) {
                free (dictionary->key);
                free (dictionary->value);
                dictionary->value = NULL;
                dictionary->key = NULL;
            }
            break;
        }
    }
    return (0);
}

int
clearDictionary (dictionary_t *dictionary)
{
    int i;

    if (dictionary == NULL || dictionary->len <= 0)
        return (0);

    for (i = 0; i < dictionary->len; i++) {
        free (dictionary->key[i]);
        free (dictionary->value[i].ptr);
    }

    free (dictionary->key);
    free (dictionary->value);
    bzero (dictionary, sizeof (keyValPair_t));
    return(0);
}

int
jsonPackDictionary (dictionary_t *dictionary, json_t **outObj)
{
    json_t *paramObj;
    int i, status;

    if (dictionary == NULL || outObj == NULL) return USER__NULL_INPUT_ERR;

    paramObj = json_object ();

    for (i = 0; i < dictionary->len; i++) {
	char *type_PI = dictionary->value[i].type_PI;

        if (strcmp (type_PI, STR_MS_T) == 0) {
            status = json_object_set_new (paramObj, dictionary->key[i],
              json_string ((char *) dictionary->value[i].ptr));
        } else if (strcmp (type_PI, INT_MS_T) == 0) {
#if JSON_INTEGER_IS_LONG_LONG
            rodsLong_t myInt = *(int *) dictionary->value[i].ptr;
#else
            int myInt = *(int *) dictionary->value[i].ptr;
#endif
            status = json_object_set_new (paramObj, dictionary->key[i],
              json_integer (myInt));
        } else if (strcmp (type_PI, FLOAT_MS_T) == 0) {
#if JSON_INTEGER_IS_LONG_LONG
            double myFloat = *(float *) dictionary->value[i].ptr;
#else
            float myFloat = *(float *) dictionary->value[i].ptr;
#endif
            status = json_object_set_new (paramObj, dictionary->key[i],
              json_real (myFloat));
        } else if (strcmp (type_PI, DOUBLE_MS_T) == 0) {
            /* DOUBLE_MS_T in iRODS is longlong */
#if JSON_INTEGER_IS_LONG_LONG
            rodsLong_t myInt = *(rodsLong_t *) dictionary->value[i].ptr;
#else
            int myInt = *(rodsLong_t *) dictionary->value[i].ptr;
#endif
            status = json_object_set_new (paramObj, dictionary->key[i],
              json_integer (myInt));
        } else if (strcmp (type_PI, BOOL_MS_T) == 0) {
            int myInt = *(int *) dictionary->value[i].ptr;
	    if (myInt == 0) {
                status = json_object_set_new (paramObj, dictionary->key[i],
                  json_false ());
            } else {
                status = json_object_set_new (paramObj, dictionary->key[i],
                  json_true ());
            }
        } else {
            rodsLog (LOG_ERROR, 
              "jsonPackDictionary: type_PI %s not supported", type_PI);
            json_decref (paramObj);
            return OOI_DICT_TYPE_NOT_SUPPORTED;
        }
        if (status != 0) {
            rodsLog (LOG_ERROR, 
              "jsonPackDictionary: son_object_set paramObj error");
            json_decref (paramObj);
            return OOI_JSON_OBJ_SET_ERR;
	}
    }
    *outObj = paramObj;

    return 0;
}

int
jsonPackOoiServReq (char *servName, char *servOpr, dictionary_t *params,
char **outStr)
{
    json_t *paramObj, *obj; 
    int status;

    if (servName == NULL || servOpr == NULL || params == NULL ||
      outStr == NULL) return USER__NULL_INPUT_ERR;

    status = jsonPackDictionary (params, &paramObj);

    if (status < 0) return status;

    obj = json_pack ("{s:{s:s,s:s,s:o}}",
                          SERVICE_REQUEST_STR,
                          SERVICE_NAME_STR, servName,
                          SERVICE_OP_STR, servOpr,
                          "params", paramObj);

    if (obj == NULL) {
        rodsLog (LOG_ERROR, "jsonPackOoiServReq: json_pack error");
        return OOI_JSON_PACK_ERR;
    }
    *outStr = json_dumps (obj, 0);
    json_decref (obj);
    if (*outStr == NULL) {
        rodsLog (LOG_ERROR, "jsonPackOoiServReq: json_dumps error");
        return OOI_JSON_DUMP_ERR;
    }
    return 0;
}

int
jsonPackOoiServReqForPost (char *servName, char *servOpr, dictionary_t *params,
char **outStr)
{
    char *tmpOutStr = NULL;
    int status, len;

    status = jsonPackOoiServReq (servName, servOpr, params, &tmpOutStr);

    if (status < 0) return status;

    len = strlen (tmpOutStr) + 20;
    *outStr = (char *)malloc (len);
    snprintf (*outStr, len, "payload=%s", tmpOutStr);
    free (tmpOutStr);
    return 0;
}

int
jsonUnpackOoiRespStr (void *buffer, char **outStr)
{
    json_t *root, *dataObj, *responseObj;
    json_error_t jerror;
    const char *responseStr;
    int status;

    root = json_loads((const char*) buffer, 0, &jerror);
    if (!root) {
        rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespStr: json_loads error. %s", jerror.text);
        return OOI_JSON_LOAD_ERR;
    }
    dataObj = json_object_get (root, OOI_DATA_TAG);
    if (!dataObj) {
       rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespStr: json_object_get data failed.");
        json_decref (root);
        return OOI_JSON_GET_ERR;
    }
    responseObj = json_object_get(dataObj, OOI_GATEWAY_RESPONSE_TAG);
    if (!responseObj) {
       rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespStr: json_object_get GatewayResponse failed.");
        json_decref (root);
        return OOI_JSON_GET_ERR;
    } 
    if (!json_is_string (responseObj)) {
	rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespStr: responseObj type %d is not JSON_STRING.",
          json_typeof (responseObj));
        json_decref (root);
        return OOI_JSON_TYPE_ERR;
    }

    responseStr = json_string_value (responseObj);

    if (responseStr != NULL) {
        *outStr = strdup (responseStr);
        status = 0;
    } else {
        status = OOI_JSON_NO_ANSWER_ERR;
    }
    json_decref (root);
    return status;
}

int
jsonUnpackOoiRespDict (void *buffer, dictionary_t **outDict)
{
    json_t *root, *dataObj, *responseObj;
    json_error_t jerror;
    int status;

    root = json_loads((const char*) buffer, 0, &jerror);
    if (!root) {
        rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespDict: json_loads error. %s", jerror.text);
        return OOI_JSON_LOAD_ERR;
    }
    dataObj = json_object_get (root, OOI_DATA_TAG);
    if (!dataObj) {
       rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespDict: json_object_get data failed.");
        json_decref (root);
        return OOI_JSON_GET_ERR;
    }
    responseObj = json_object_get(dataObj, OOI_GATEWAY_RESPONSE_TAG);
    if (!responseObj) {
       rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespDict: json_object_get GatewayResponse failed.");
        json_decref (root);
        return OOI_JSON_GET_ERR;
    }
    if (!json_is_object (responseObj)) {
        rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespDict: responseObj type %d is not JSON_OBJECT.",
          json_typeof (responseObj));
        json_decref (root);
        return OOI_JSON_TYPE_ERR;
    }
    *outDict = (dictionary_t *) calloc (1, sizeof (dictionary_t));
    status = jsonUnpackDict (responseObj, *outDict);
    if (status < 0) free (*outDict);

    return status;
}


int
jsonUnpackDict (json_t *dictObj, dictionary_t *outDict) 
{
    void *iter;
    void *tmpOut;
    int *tmpInt;
    float *tmpFloat;
    const char *key;
    json_t *value;
    int status;

    if (dictObj == NULL || outDict == NULL) {
        rodsLog (LOG_ERROR,
          "jsonUnpackDict: NULL input");
        return USER__NULL_INPUT_ERR;
    }
    bzero (outDict, sizeof (dictionary_t));
    iter = json_object_iter(dictObj);
    while (iter) {
        json_type myType;

        key = json_object_iter_key (iter);
        value = json_object_iter_value (iter);
        myType = json_typeof (value);
        switch (myType) {
          case JSON_STRING:
            tmpOut = strdup (json_string_value (value));
            status = dictSetAttr (outDict, (char *) key, STR_MS_T, 
              tmpOut, 0);
            break;
          case JSON_INTEGER:
	    tmpInt = (int *) calloc (1, sizeof (int));
            *tmpInt = (int) json_integer_value (value);
            status = dictSetAttr (outDict, (char *) key, INT_MS_T,
              (void *) tmpInt, 0);
            break;
          case JSON_REAL:
	    tmpFloat = (float *) calloc (1, sizeof (float));
            *tmpFloat = (float) json_real_value (value);
            status = dictSetAttr (outDict, (char *) key, FLOAT_MS_T,
              (void *) tmpFloat, 0);
            break;
          case JSON_TRUE:
	    tmpInt = (int *) calloc (1, sizeof (int));
            *tmpInt = 1;
            status = dictSetAttr (outDict, (char *) key, BOOL_MS_T,
              (void *) tmpInt, 0);
            break;
          case JSON_FALSE:
	    tmpInt = (int *) calloc (1, sizeof (int));
            *tmpInt = 0;
            status = dictSetAttr (outDict, (char *) key, BOOL_MS_T,
              (void *) tmpInt, 0);
            break;
          default:
            rodsLog (LOG_ERROR,
              "ooiGenServReqFunc: myType %d not supported", myType);
	    status = OOI_JSON_TYPE_ERR;
        }
        iter = json_object_iter_next(dictObj, iter);
    }
    if (status < 0) clearDictionary (outDict);

    return status;
}

int
jsonUnpackOoiRespDictArray (void *buffer, dictArray_t **outDictArray)
{
    json_t *root, *dataObj, *responseObj, *dictObj;
    json_error_t jerror;
    int status, i;
    int arrayLen;
    dictionary_t *dictArray;

    root = json_loads((const char*) buffer, 0, &jerror);
    if (!root) {
        rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespDictArray: json_loads error. %s", jerror.text);
        return OOI_JSON_LOAD_ERR;
    }
    dataObj = json_object_get (root, OOI_DATA_TAG);
    if (!dataObj) {
       rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespDictArray: json_object_get data failed.");
        json_decref (root);
        return OOI_JSON_GET_ERR;
    }
    responseObj = json_object_get(dataObj, OOI_GATEWAY_RESPONSE_TAG);
    if (!responseObj) {
       rodsLog (LOG_ERROR,
         "jsonUnpackOoiRespDictArray: json_object_get GatewayResponse failed.");
        json_decref (root);
        return OOI_JSON_GET_ERR;
    }
    if (!json_is_array (responseObj)) {
        rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespDictArray: responseObj type %d is not JSON_ARRAY.",
          json_typeof (responseObj));
        json_decref (root);
        return OOI_JSON_TYPE_ERR;
    }
    arrayLen = (int ) json_array_size(responseObj);

    if (arrayLen <= 0) return OOI_JSON_NO_ANSWER_ERR;

    dictArray = (dictionary_t *) calloc (arrayLen, sizeof (dictionary_t));
    for(i = 0; i < arrayLen; i++) {
        dictObj = json_array_get(responseObj, i);
        if(!json_is_object(dictObj)) {
            rodsLog (LOG_ERROR,
              "jsonUnpackOoiRespDictArray: Obj type %d not an object",
              json_typeof (dictObj));
            json_decref (root);
            clearDictArray (dictArray, arrayLen);
            free (dictArray);
            return OOI_JSON_TYPE_ERR;
        }
        status = jsonUnpackDict (dictObj, &dictArray[i]);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "jsonUnpackOoiRespDictArray: jsonUnpackDict failed");
            json_decref (root);
            clearDictArray (dictArray, arrayLen);
            free (dictArray);
            return status;
        }
    }
    json_decref (root);
    *outDictArray = (dictArray_t *) calloc (1, sizeof (dictArray_t));
    (*outDictArray)->len = arrayLen;
    (*outDictArray)->dictionary = dictArray;

    return 0;
}

int
jsonUnpackOoiRespDictArrInArr (void *buffer, dictArray_t **outDictArray,
int outInx)
{
    json_t *root, *dataObj, *responseObj, *myDictArrayObj, *dictObj;
    json_error_t jerror;
    int status, i;
    int arrayLen;
    dictionary_t *dictArray;

    root = json_loads((const char*) buffer, 0, &jerror);
    if (!root) {
        rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespDictArrInArr: json_loads error. %s", jerror.text);
        return OOI_JSON_LOAD_ERR;
    }
    dataObj = json_object_get (root, OOI_DATA_TAG);
    if (!dataObj) {
       rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespDictArrInArr: json_object_get data failed");
        json_decref (root);
        return OOI_JSON_GET_ERR;
    }
    responseObj = json_object_get(dataObj, OOI_GATEWAY_RESPONSE_TAG);
    if (!responseObj) {
       rodsLog (LOG_ERROR,
        "jsonUnpackOoiRespDictArrInArr:json_object_get GatewayResponse failed");
        json_decref (root);
        return OOI_JSON_GET_ERR;
    }
    if (!json_is_array (responseObj)) {
        rodsLog (LOG_ERROR,
         "jsonUnpackOoiRespDictArrInArr:responseObj type %d is not JSON_ARRAY",
          json_typeof (responseObj));
        json_decref (root);
        return OOI_JSON_TYPE_ERR;
    }
    arrayLen = (int ) json_array_size(responseObj);

    if (arrayLen <= 0) return OOI_JSON_NO_ANSWER_ERR;

    if (outInx >= arrayLen) {
        rodsLog (LOG_ERROR,
         "jsonUnpackOoiRespDictArrInArr: outInx %d >= arrayLen %d",
          outInx, arrayLen);
        json_decref (root);
        return OOI_JSON_INX_OUT_OF_RANGE;
    }

    myDictArrayObj = json_array_get (responseObj, outInx);
    arrayLen = (int ) json_array_size(myDictArrayObj);

    if (arrayLen <= 0) return OOI_JSON_NO_ANSWER_ERR;


    dictArray = (dictionary_t *) calloc (arrayLen, sizeof (dictionary_t));
    for(i = 0; i < arrayLen; i++) {
        dictObj = json_array_get(myDictArrayObj, i);
        if(!json_is_object(dictObj)) {
            rodsLog (LOG_ERROR,
              "jsonUnpackOoiRespDictArrInArr: Obj type %d not an object",
            json_typeof (dictObj));
            json_decref (root);
            clearDictArray (dictArray, arrayLen);
            free (dictArray);
            return OOI_JSON_TYPE_ERR;
        }
        status = jsonUnpackDict (dictObj, &dictArray[i]);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "jsonUnpackOoiRespDictArrInArr: jsonUnpackDict failed");
            json_decref (root);
            clearDictArray (dictArray, arrayLen);
            free (dictArray);
            return status;
        }
    }
    json_decref (root);
    *outDictArray = (dictArray_t *) calloc (1, sizeof (dictArray_t));
    (*outDictArray)->len = arrayLen;
    (*outDictArray)->dictionary = dictArray;

    return 0;
}

int 
clearDictArray (dictionary_t *dictArray, int len)
{
    int i;

    if (dictArray == NULL) return 0;

    for (i = 0; i < len; i++) {
        clearDictionary (&dictArray[i]);
    }
    return 0;
}

