/**
 *  * @file  reNetcdf.c
 *   *
 *    */

/*** Copyright (c), The Regents of the University of California            ***
 *  *** For more information please refer to files in the COPYRIGHT directory ***/

#include "collection.h"
#include "reNetcdf.h"

int
msiNcOpen (msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, 
ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    ncOpenInp_t ncOpenInp;
    int *ncid;

    RE_TEST_MACRO ("    Calling msiNcOpen")

    if (rei == NULL || rei->rsComm == NULL) {
      rodsLog (LOG_ERROR,
        "msiNcOpen: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;
    if (inpParam1 == NULL) {
        rodsLog (LOG_ERROR,
          "msiNcOpen: input inpParam1 is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (strcmp (inpParam1->type, STR_MS_T) == 0) {
        /* str input */
	bzero (&ncOpenInp, sizeof (ncOpenInp));
	rstrcpy (ncOpenInp.objPath, (char*)inpParam1->inOutStruct, 
	  MAX_NAME_LEN);
    } else  if (strcmp (inpParam1->type, NcOpenInp_MS_T) == 0) {
	ncOpenInp = *((ncOpenInp_t *) inpParam1->inOutStruct);
	replKeyVal (&((ncOpenInp_t *) inpParam1->inOutStruct)->condInput,
	  &ncOpenInp.condInput);
    } else {
        rodsLog (LOG_ERROR,
          "msiNcOpen: Unsupported input Param1 type %s",
          inpParam1->type);
        return (USER_PARAM_TYPE_ERR);
    }
    if (inpParam2 != NULL) {
	/* parse for mode */
	ncOpenInp.mode = parseMspForPosInt (inpParam2);
	if (ncOpenInp.mode < 0) return (ncOpenInp.mode);
    }

    rei->status = rsNcOpen (rsComm, &ncOpenInp, &ncid);
    clearKeyVal (&ncOpenInp.condInput);
    if (rei->status >= 0) {
        fillIntInMsParam (outParam, *ncid);
        free (ncid);
    } else {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
        "msiNcOpen: rsNcOpen failed for %s, status = %d",
        ncOpenInp.objPath, rei->status);
    }

    return (rei->status);
}

int
msiNcCreate (msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, 
ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    ncOpenInp_t ncOpenInp;
    int *ncid;

    RE_TEST_MACRO ("    Calling msiNcCreate")

    if (rei == NULL || rei->rsComm == NULL) {
      rodsLog (LOG_ERROR,
        "msiNcCreate: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;
    if (inpParam1 == NULL) {
        rodsLog (LOG_ERROR,
          "msiNcCreate: input inpParam1 is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (strcmp (inpParam1->type, STR_MS_T) == 0) {
        /* str input */
	bzero (&ncOpenInp, sizeof (ncOpenInp));
	rstrcpy (ncOpenInp.objPath, (char*)inpParam1->inOutStruct, 
	  MAX_NAME_LEN);
    } else  if (strcmp (inpParam1->type, NcOpenInp_MS_T) == 0) {
	ncOpenInp = *((ncOpenInp_t *) inpParam1->inOutStruct);
	replKeyVal (&((ncOpenInp_t *) inpParam1->inOutStruct)->condInput,
	  &ncOpenInp.condInput);
    } else {
        rodsLog (LOG_ERROR,
          "msiNcOpen: Unsupported input Param1 type %s",
          inpParam1->type);
        return (USER_PARAM_TYPE_ERR);
    }
    if (inpParam2 != NULL) {
	/* parse for mode */
	ncOpenInp.mode = parseMspForPosInt (inpParam2);
	if (ncOpenInp.mode < 0) return (ncOpenInp.mode);
    }

    rei->status = rsNcCreate (rsComm, &ncOpenInp, &ncid);
    clearKeyVal (&ncOpenInp.condInput);
    if (rei->status >= 0) {
        fillIntInMsParam (outParam, *ncid);
        free (ncid);
    } else {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
        "msiNcCreate: rsNcCreate failed for %s, status = %d",
        ncOpenInp.objPath, rei->status);
    }

    return (rei->status);
}

int
msiNcClose (msParam_t *inpParam1, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    ncCloseInp_t ncCloseInp;

    RE_TEST_MACRO ("    Calling msiNcClose")

    if (rei == NULL || rei->rsComm == NULL) {
      rodsLog (LOG_ERROR,
        "msiNcClose: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    rsComm = rei->rsComm;
    if (inpParam1 == NULL) {
        rodsLog (LOG_ERROR,
          "msiNcClose: input inpParam1 is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (strcmp (inpParam1->type, INT_MS_T) == 0) {
        /* str input */
	bzero (&ncCloseInp, sizeof (ncCloseInp));
	ncCloseInp.ncid = *((int*) inpParam1->inOutStruct); 
    } else  if (strcmp (inpParam1->type, NcCloseInp_MS_T) == 0) {
	ncCloseInp = *((ncCloseInp_t *) inpParam1->inOutStruct);
	replKeyVal (&((ncCloseInp_t *) inpParam1->inOutStruct)->condInput,
	  &ncCloseInp.condInput);
    } else {
        rodsLog (LOG_ERROR,
          "msiNcClose: Unsupported input Param1 type %s",
          inpParam1->type);
        return (USER_PARAM_TYPE_ERR);
    }

    rei->status = rsNcClose (rsComm, &ncCloseInp);
    clearKeyVal (&ncCloseInp.condInput);
    if (rei->status < 0) {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
        "msiNcClose: rsNcClose failed for %d, status = %d",
        ncCloseInp.ncid, rei->status);
    }

    return (rei->status);
}

int
msiNcInqId (msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *inpParam3,
msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    ncInqIdInp_t ncInqIdInp;
    int *outId;

    RE_TEST_MACRO ("    Calling msiNcInqId")

    if (rei == NULL || rei->rsComm == NULL) {
      rodsLog (LOG_ERROR,
        "msiNcInqId: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    rsComm = rei->rsComm;

    if (inpParam1 == NULL) {
        rodsLog (LOG_ERROR,
          "msiNcInqId: input inpParam1 is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    /* parse for name or ncInqWithIdInp_t */
    rei->status = parseMspForNcInqIdInpName (inpParam1, &ncInqIdInp);

    if (rei->status < 0) return rei->status;

    if (inpParam2 != NULL) {
	/* parse for paramType */
	ncInqIdInp.paramType = parseMspForPosInt (inpParam2);
	if (ncInqIdInp.paramType != NC_VAR_T && 
	  ncInqIdInp.paramType != NC_DIM_T) {
            rodsLog (LOG_ERROR,
              "msiNcInqId: Unknow paramType %d for %s ", 
	      ncInqIdInp.paramType, ncInqIdInp.name);
            return (NETCDF_INVALID_PARAM_TYPE);
	}
    }

    if (inpParam3 != NULL) {
        /* parse for ncid */
        ncInqIdInp.ncid = parseMspForPosInt (inpParam3);
    }

    rei->status = rsNcInqId (rsComm, &ncInqIdInp, &outId);
    clearKeyVal (&ncInqIdInp.condInput);
    if (rei->status >= 0) {
        fillIntInMsParam (outParam, *outId);
        free (outId);
    } else {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
        "msiNcInqId: rsNcInqId failed for %s, status = %d",
        ncInqIdInp.name, rei->status);
    }

    return (rei->status);
}

int
msiNcInqWithId (msParam_t *inpParam1, msParam_t *inpParam2, 
msParam_t *inpParam3, msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    ncInqIdInp_t ncInqWithIdInp;
    ncInqWithIdOut_t *ncInqWithIdOut = NULL;

    RE_TEST_MACRO ("    Calling msiNcInqWithId")

    if (rei == NULL || rei->rsComm == NULL) {
      rodsLog (LOG_ERROR,
        "msiNcInqWithId: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    rsComm = rei->rsComm;

    if (inpParam1 == NULL) {
        rodsLog (LOG_ERROR,
          "msiNcInqWithId: input inpParam1 is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    /* parse for myid or ncInqWithIdInp_t */
    rei->status = parseMspForNcInqIdInpId (inpParam1, &ncInqWithIdInp);

    if (rei->status < 0) return rei->status;

    if (inpParam2 != NULL) {
	/* parse for paramType */
	ncInqWithIdInp.paramType = parseMspForPosInt (inpParam2);
	if (ncInqWithIdInp.paramType != NC_VAR_T && 
	  ncInqWithIdInp.paramType != NC_DIM_T) {
            rodsLog (LOG_ERROR,
              "msiNcInqWithId: Unknow paramType %d for %s ", 
	      ncInqWithIdInp.paramType, ncInqWithIdInp.name);
            return (NETCDF_INVALID_PARAM_TYPE);
	}
    }

    if (inpParam3 != NULL) {
        /* parse for ncid */
        ncInqWithIdInp.ncid = parseMspForPosInt (inpParam3);
    }

    rei->status = rsNcInqWithId (rsComm, &ncInqWithIdInp, &ncInqWithIdOut);
    clearKeyVal (&ncInqWithIdInp.condInput);
    if (rei->status >= 0) {
	fillMsParam (outParam, NULL, NcInqWithIdOut_MS_T, ncInqWithIdOut, NULL);
    } else {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
        "msiNcInqWithId: rsNcInqWithId failed for %s, status = %d",
        ncInqWithIdInp.name, rei->status);
    }

    return (rei->status);
}

int
msiNcGetVarsByType (msParam_t *dataTypeParam, msParam_t *ncidParam, 
msParam_t *varidParam, msParam_t *ndimParam, msParam_t *startParam, 
msParam_t *countParam, msParam_t *strideParam,
msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    ncGetVarInp_t ncGetVarInp;
    ncGetVarOut_t *ncGetVarOut = NULL;
    int ndimOut;

    RE_TEST_MACRO ("    Calling msiNcGetVarsByType")

    if (rei == NULL || rei->rsComm == NULL) {
      rodsLog (LOG_ERROR,
        "msiNcGetVarsByType: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    rsComm = rei->rsComm;

    if (dataTypeParam == NULL) {
        rodsLog (LOG_ERROR,
          "msiNcGetVarsByType: input dataTypeParam is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    /* parse for dataType or ncGetVarInp_t */
    rei->status = parseMspForNcGetVarInp (dataTypeParam, &ncGetVarInp);

    if (rei->status < 0) return rei->status;

    if (ncidParam != NULL) {
	/* parse for ncid */
	ncGetVarInp.ncid = parseMspForPosInt (ncidParam);
	if (ncGetVarInp.ncid < 0) return ncGetVarInp.ncid;
    }

    if (varidParam != NULL) { 
        /* parse for varid */ 
        ncGetVarInp.varid = parseMspForPosInt (varidParam);
        if (ncGetVarInp.varid < 0) return ncGetVarInp.varid;
    }

    if (ndimParam != NULL) { 
        /* parse for ndim */
        ncGetVarInp.ndim = parseMspForPosInt (ndimParam);
        if (ncGetVarInp.ndim < 0) return ncGetVarInp.ndim;
    }

    if (startParam != NULL) {
        /* parse for start */
        rei->status = parseStrMspForLongArray (startParam, &ndimOut, 
	  &ncGetVarInp.start);
        if (rei->status < 0) return rei->status;
	if (ndimOut != ncGetVarInp.ndim) {
	    rei->status = NETCDF_DIM_MISMATCH_ERR;
            rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
              "msiNcGetVarsByType: start dim = %d, input ndim = %d",
	      ndimOut, ncGetVarInp.ndim);
	    return NETCDF_DIM_MISMATCH_ERR;
	}
    }

    if (countParam != NULL) {
        /* parse for count */
        rei->status = parseStrMspForLongArray (countParam, &ndimOut,
          &ncGetVarInp.count);
        if (rei->status < 0) return rei->status;
        if (ndimOut != ncGetVarInp.ndim) {
            rei->status = NETCDF_DIM_MISMATCH_ERR;
            rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
              "msiNcGetVarsByType: count dim = %d, input ndim = %d",
              ndimOut, ncGetVarInp.ndim);
            return NETCDF_DIM_MISMATCH_ERR;
        }
    }

    if (strideParam != NULL) {
        /* parse for stride */
        rei->status = parseStrMspForLongArray (strideParam, &ndimOut,
          &ncGetVarInp.stride);
        if (rei->status < 0) return rei->status;
        if (ndimOut != ncGetVarInp.ndim) {
            rei->status = NETCDF_DIM_MISMATCH_ERR;
            rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
              "msiNcGetVarsByType: stride dim = %d, input ndim = %d",
              ndimOut, ncGetVarInp.ndim);
            return NETCDF_DIM_MISMATCH_ERR;
        }
    }

    rei->status = rsNcGetVarsByType (rsComm, &ncGetVarInp, &ncGetVarOut);
    clearNcGetVarInp (&ncGetVarInp);
    if (rei->status >= 0) {
	fillMsParam (outParam, NULL, NcGetVarOut_MS_T, ncGetVarOut, NULL);
    } else {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
        "msiNcGetVarsByType: rsNcGetVarsByType failed, status = %d",
        rei->status);
    }

    return (rei->status);
}

#ifdef LIB_CF
int
msiNccfGetVara (msParam_t *ncidParam, msParam_t *varidParam, 
msParam_t *lvlIndexParam, msParam_t *timestepParam, 
msParam_t *latRange0Param, msParam_t *latRange1Param,
msParam_t *lonRange0Param, msParam_t *lonRange1Param,
msParam_t *maxOutArrayLenParam, msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    nccfGetVarInp_t nccfGetVarInp;
    nccfGetVarOut_t *nccfGetVarOut = NULL;

    RE_TEST_MACRO ("    Calling msiNccfGetVara")

    if (rei == NULL || rei->rsComm == NULL) {
      rodsLog (LOG_ERROR,
        "msiNccfGetVara: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    rsComm = rei->rsComm;

    if (ncidParam == NULL) {
        rodsLog (LOG_ERROR,
          "msiNccfGetVara: input ncidParam is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    /* parse for dataType or nccfGetVarInp_t */
    rei->status = parseMspForNccfGetVarInp (ncidParam, &nccfGetVarInp);

    if (rei->status < 0) return rei->status;

    if (varidParam != NULL) { 
        /* parse for varid */ 
        nccfGetVarInp.varid = parseMspForPosInt (varidParam);
        if (nccfGetVarInp.varid < 0) return nccfGetVarInp.varid;
    }

    if (lvlIndexParam != NULL) { 
        /* parse for ndim */
        nccfGetVarInp.lvlIndex = parseMspForPosInt (lvlIndexParam);
        if (nccfGetVarInp.lvlIndex < 0) return nccfGetVarInp.lvlIndex;
    }

    if (timestepParam != NULL) { 
        /* parse for ndim */
        nccfGetVarInp.timestep = parseMspForPosInt (timestepParam);
        if (nccfGetVarInp.timestep < 0) return nccfGetVarInp.timestep;
    }

    if (latRange0Param != NULL) {
        rei->status = parseMspForFloat (latRange0Param, 
	  &nccfGetVarInp.latRange[0]);
	if (rei->status < 0) return rei->status;
    }

    if (latRange1Param != NULL) {
	rei->status = parseMspForFloat (latRange1Param,
          &nccfGetVarInp.latRange[1]); 
	if (rei->status < 0) return rei->status;
    }

    if (lonRange0Param != NULL) {
	rei->status  = parseMspForFloat (lonRange0Param,
          &nccfGetVarInp.lonRange[0]); 
	if (rei->status < 0) return rei->status;
    }

    if (lonRange1Param != NULL) {
	rei->status = parseMspForFloat (lonRange1Param,
          &nccfGetVarInp.lonRange[1]);
	if (rei->status < 0) return rei->status;
    }

    if (maxOutArrayLenParam != NULL) {
        /* parse for maxOutArrayLen */
        nccfGetVarInp.maxOutArrayLen = parseMspForPosInt (maxOutArrayLenParam);
        if (nccfGetVarInp.maxOutArrayLen < 0) 
	  return nccfGetVarInp.maxOutArrayLen;
    }

    rei->status = rsNccfGetVara (rsComm, &nccfGetVarInp, &nccfGetVarOut);
    clearKeyVal (&nccfGetVarInp.condInput);
    if (rei->status >= 0) {
	fillMsParam (outParam, NULL, NccfGetVarOut_MS_T, nccfGetVarOut, NULL);
    } else {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
        "msiNccfGetVara: rsNccfGetVara failed, status = %d",
        rei->status);
    }

    return (rei->status);
}
#endif	

int
msiNcGetArrayLen (msParam_t *inpParam, msParam_t *outParam, 
ruleExecInfo_t *rei)
{
#if 0	/* should use rodsLong_t, but some problem with rule engine */
    rodsLong_t arrayLen;
#endif
    int arrayLen;

    if (inpParam == NULL || outParam == NULL) return USER__NULL_INPUT_ERR;

    if (strcmp (inpParam->type, NcInqWithIdOut_MS_T) == 0) {
	ncInqWithIdOut_t *ncInqWithIdOut;
        ncInqWithIdOut = (ncInqWithIdOut_t *) inpParam->inOutStruct;
	arrayLen = ncInqWithIdOut->mylong;
    } else if (strcmp (inpParam->type, NcGetVarOut_MS_T) == 0) {
        ncGetVarOut_t *ncGetVarOut;
        ncGetVarOut = (ncGetVarOut_t *) inpParam->inOutStruct;
        if (ncGetVarOut == NULL || ncGetVarOut->dataArray == NULL)
            return USER__NULL_INPUT_ERR;
        arrayLen = ncGetVarOut->dataArray->len;
    } else if (strcmp (inpParam->type, NccfGetVarOut_MS_T) == 0) {
        nccfGetVarOut_t *nccfGetVarOut;
        nccfGetVarOut = (nccfGetVarOut_t *) inpParam->inOutStruct;
        if (nccfGetVarOut == NULL || nccfGetVarOut->dataArray == NULL)
            return USER__NULL_INPUT_ERR;
        arrayLen = nccfGetVarOut->dataArray->len;
    } else {
        rodsLog (LOG_ERROR, 
          "msiNcGetArrayLen: Unsupported input Param type %s",
          inpParam->type);
        return (USER_PARAM_TYPE_ERR);
    }
    fillIntInMsParam (outParam, arrayLen);
    return 0;
}

int
msiNcGetNumDim (msParam_t *inpParam, msParam_t *outParam,
ruleExecInfo_t *rei)
{
    int ndim;

    if (inpParam == NULL || outParam == NULL) return USER__NULL_INPUT_ERR;

    if (strcmp (inpParam->type, NcInqWithIdOut_MS_T) == 0) {
        ncInqWithIdOut_t *ncInqWithIdOut;
        ncInqWithIdOut = (ncInqWithIdOut_t *) inpParam->inOutStruct;
        ndim = ncInqWithIdOut->ndim;
    } else if (strcmp (inpParam->type, NcGetVarInp_MS_T) == 0) {
        ncGetVarInp_t *ncGetVarInp;
        ncGetVarInp = (ncGetVarInp_t *) inpParam->inOutStruct;
        ndim = ncGetVarInp->ndim;
    } else {
        rodsLog (LOG_ERROR,
          "msiNcGetNumDim: Unsupported input Param type %s",
          inpParam->type);
        return (USER_PARAM_TYPE_ERR);
    }
    fillIntInMsParam (outParam, ndim);
    return 0;
}

int
msiNcGetDataType (msParam_t *inpParam, msParam_t *outParam,
ruleExecInfo_t *rei)
{
    int dataType;

    if (inpParam == NULL || outParam == NULL) return USER__NULL_INPUT_ERR;

    if (strcmp (inpParam->type, NcInqWithIdOut_MS_T) == 0) {
        ncInqWithIdOut_t *ncInqWithIdOut;
        ncInqWithIdOut = (ncInqWithIdOut_t *) inpParam->inOutStruct;
        dataType = ncInqWithIdOut->dataType;
    } else if (strcmp (inpParam->type, NcGetVarInp_MS_T) == 0) {
        ncGetVarInp_t *ncGetVarInp;
        ncGetVarInp = (ncGetVarInp_t *) inpParam->inOutStruct;
        dataType = ncGetVarInp->dataType;
    } else if (strcmp (inpParam->type, NcGetVarOut_MS_T) == 0) {
        ncGetVarOut_t *ncGetVarOut;
        ncGetVarOut = (ncGetVarOut_t *) inpParam->inOutStruct;
	if (ncGetVarOut == NULL || ncGetVarOut->dataArray == NULL)
	    return USER__NULL_INPUT_ERR;
        dataType = ncGetVarOut->dataArray->type;
    } else if (strcmp (inpParam->type, NccfGetVarOut_MS_T) == 0) {
        nccfGetVarOut_t *nccfGetVarOut;
        nccfGetVarOut = (nccfGetVarOut_t *) inpParam->inOutStruct;
        if (nccfGetVarOut == NULL || nccfGetVarOut->dataArray == NULL)
            return USER__NULL_INPUT_ERR;
        dataType = nccfGetVarOut->dataArray->type;
    } else {
        rodsLog (LOG_ERROR,
          "msiNcGetNumDim: Unsupported input Param type %s",
          inpParam->type);
        return (USER_PARAM_TYPE_ERR);
    }
    fillIntInMsParam (outParam, dataType);
    return 0;
}

int
msiNcGetElementInArray (msParam_t *arrayStructParam, msParam_t *indexParam,
msParam_t *outParam, ruleExecInfo_t *rei)
{
    int myindex;
    void *myarray;
    int dataType;
    int arrayLen;
    char *charArray;
    int *intArray;
    float *floatArray;
    rodsLong_t *longArray;
    char **strArray;

    if (arrayStructParam == NULL || indexParam == NULL ||
      outParam == NULL) return USER__NULL_INPUT_ERR;

    if (strcmp (arrayStructParam->type, NcInqWithIdOut_MS_T) == 0) {
	/* intArray for the id array */
        ncInqWithIdOut_t *ncInqWithIdOut;
        ncInqWithIdOut = (ncInqWithIdOut_t *) arrayStructParam->inOutStruct;
        dataType = NC_INT;
        myarray = (void *) ncInqWithIdOut->intArray;
	arrayLen = ncInqWithIdOut->ndim;
    } else if (strcmp (arrayStructParam->type, NcGetVarOut_MS_T) == 0) {
        ncGetVarOut_t *ncGetVarOut;
        ncGetVarOut = (ncGetVarOut_t *) arrayStructParam->inOutStruct;
        if (ncGetVarOut == NULL || ncGetVarOut->dataArray == NULL)
            return USER__NULL_INPUT_ERR;
        dataType = ncGetVarOut->dataArray->type;
        myarray = ncGetVarOut->dataArray->buf;
	arrayLen = ncGetVarOut->dataArray->len;
    } else if (strcmp (arrayStructParam->type, NccfGetVarOut_MS_T) == 0) {
        nccfGetVarOut_t *nccfGetVarOut;
        nccfGetVarOut = (nccfGetVarOut_t *) arrayStructParam->inOutStruct;
        if (nccfGetVarOut == NULL || nccfGetVarOut->dataArray == NULL)
            return USER__NULL_INPUT_ERR;
        dataType = nccfGetVarOut->dataArray->type;
        myarray = nccfGetVarOut->dataArray->buf;
	arrayLen = nccfGetVarOut->dataArray->len;
    } else {
        rodsLog (LOG_ERROR,
          "msiNcGetNumDim: Unsupported input Param type %s",
          arrayStructParam->type);
        return (USER_PARAM_TYPE_ERR);
    }
    myindex = parseMspForPosInt (indexParam);
    if (myindex < 0 || myindex >= arrayLen) {
        rodsLog (LOG_ERROR,
          "msiNcGetElementInArray: input index %d out of range. arrayLen = %d",
          myindex, arrayLen);
        return (NETCDF_DIM_MISMATCH_ERR);
    }

    switch (dataType) {
      case NC_CHAR:
      case NC_BYTE:
      case NC_UBYTE:
	charArray = (char *) myarray;
	fillCharInMsParam (outParam, charArray[myindex]);
	break;
      case NC_STRING:
	strArray = (char **) myarray;
	fillStrInMsParam (outParam, strArray[myindex]);
	break;
      case NC_INT:
      case NC_UINT:
	intArray = (int *) myarray;
        fillIntInMsParam (outParam, intArray[myindex]);
        break;
      case NC_FLOAT:
	floatArray = (float *) myarray;
        fillFloatInMsParam (outParam, floatArray[myindex]);
        break;
      case NC_INT64:
      case NC_UINT64:
      case NC_DOUBLE:
	longArray = (rodsLong_t *) myarray;
        fillDoubleInMsParam (outParam, longArray[myindex]);
        break;
      default:
        rodsLog (LOG_ERROR,
          "msiNcGetElementInArray: Unknow dataType %d", dataType);
        return (NETCDF_INVALID_DATA_TYPE);
    }
    return 0;
}

int
msiFloatToString (msParam_t *floatParam, msParam_t *stringParam,
ruleExecInfo_t *rei)
{
    char floatStr[NAME_LEN];
    float *myfloat;
    if (floatParam == NULL || stringParam == NULL) return USER__NULL_INPUT_ERR;

    if (strcmp (floatParam->type, FLOAT_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "msiFloatToString: floatParam type %s error", floatParam->type);
	return USER_PARAM_TYPE_ERR;
    }
    myfloat = (float *) floatParam->inOutStruct;
    snprintf (floatStr, NAME_LEN, "%f", *myfloat);
    fillStrInMsParam (stringParam, floatStr);

    return 0;
}
