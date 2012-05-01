/**
 *  * @file  reNetcdf.c
 *   *
 *    */

/*** Copyright (c), The Regents of the University of California            ***
 *  *** For more information please refer to files in the COPYRIGHT directory ***/

#include "collection.h"
#include "reNetcdf.h"

/**
 * \fn msiNcOpen (msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, ruleExecInfo_t *rei)
 *
**/
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

/**
 * \fn msiNcCreate (msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *outParam, ruleExecInfo_t *rei)
 *
**/
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

/**
 * \fn msiNcClose (msParam_t *inpParam1, ruleExecInfo_t *rei)
 *
**/
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

/**
 * \fn msiNcInqId (msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *inpParam3, msParam_t *outParam, ruleExecInfo_t *rei)
 *
**/
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

/**
 * \fn msiNcInqWithId (msParam_t *inpParam1, msParam_t *inpParam2, msParam_t *inpParam3, msParam_t *outParam, ruleExecInfo_t *rei)
 *
**/
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

/**
 * \fn msiNcGetVarsByType (msParam_t *dataTypeParam, msParam_t *ncidParam,  msParam_t *varidParam, msParam_t *ndimParam, msParam_t *startParam,  msParam_t *countParam, msParam_t *strideParam, msParam_t *outParam, ruleExecInfo_t *rei)
 *
**/
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
/**
 * \fn msiNccfGetVara (msParam_t *ncidParam, msParam_t *varidParam, msParam_t *lvlIndexParam, msParam_t *timestepParam,  msParam_t *latRange0Param, msParam_t *latRange1Param, msParam_t *lonRange0Param, msParam_t *lonRange1Param, msParam_t *maxOutArrayLenParam, msParam_t *outParam, ruleExecInfo_t *rei)
 *
**/
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

/**
 * \fn msiNcGetArrayLen (msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei)
 *
**/
int
msiNcGetArrayLen (msParam_t *inpParam, msParam_t *outParam, 
ruleExecInfo_t *rei)
{
#if 0	/* should use rodsLong_t, but some problem with rule engine */
    rodsLong_t arrayLen;
#endif
    int arrayLen;

    RE_TEST_MACRO ("    Calling msiNcGetArrayLen")

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

/**
 * \fn msiNcGetNumDim (msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei)
 *
**/
int
msiNcGetNumDim (msParam_t *inpParam, msParam_t *outParam,
ruleExecInfo_t *rei)
{
    int ndim;

    RE_TEST_MACRO ("    Calling msiNcGetNumDim")

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

/**
 * \fn msiNcGetDataType (msParam_t *inpParam, msParam_t *outParam, ruleExecInfo_t *rei)
 *
**/
int
msiNcGetDataType (msParam_t *inpParam, msParam_t *outParam,
ruleExecInfo_t *rei)
{
    int dataType;

    RE_TEST_MACRO ("    Calling msiNcGetDataType")

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

/**
 * \fn msiNcGetElementInArray (msParam_t *arrayStructParam, msParam_t *indexParam, msParam_t *outParam, ruleExecInfo_t *rei)
 *
**/
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

    RE_TEST_MACRO ("    Calling msiNcGetElementInArray")

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

/**
 * \fn msiFloatToString (msParam_t *floatParam, msParam_t *stringParam, ruleExecInfo_t *rei)
 *
**/
int
msiFloatToString (msParam_t *floatParam, msParam_t *stringParam,
ruleExecInfo_t *rei)
{
    char floatStr[NAME_LEN];
    float *myfloat;

    RE_TEST_MACRO ("    Calling msiFloatToString")

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

/**
 * \fn msiNcInq (msParam_t *ncidParam, msParam_t *outParam, ruleExecInfo_t *rei)
 *
**/
int
msiNcInq (msParam_t *ncidParam, msParam_t *outParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    ncInqInp_t ncInqInp;
    ncInqOut_t *ncInqOut = NULL;;

    RE_TEST_MACRO ("    Calling msiNcInq")

    if (rei == NULL || rei->rsComm == NULL) {
      rodsLog (LOG_ERROR,
        "msiNcInq: input rei or rsComm is NULL");
      return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    rsComm = rei->rsComm;

    if (ncidParam == NULL) {
        rodsLog (LOG_ERROR,
          "msiNcInq: input ncidParam is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }
    bzero (&ncInqInp, sizeof (ncInqInp));

    ncInqInp.ncid = parseMspForPosInt (ncidParam);

    rei->status = rsNcInq (rsComm, &ncInqInp, &ncInqOut);

    clearKeyVal (&ncInqInp.condInput);
    if (rei->status >= 0) {
	fillMsParam (outParam, NULL, NcInqOut_MS_T, ncInqOut, NULL);
    } else {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
        "msiNcInq: rsNcInq failed for ncid %d, status = %d",
        ncInqInp.ncid, rei->status);
    }

    return (rei->status);
}

int
msiNcGetNdimsInInqOut (msParam_t *ncInqOutParam, msParam_t *varNameParam,
msParam_t *outParam, ruleExecInfo_t *rei)
{
    int ndims = -1;
    ncInqOut_t *ncInqOut;
    char *name;

    RE_TEST_MACRO ("    Calling msiNcGetNdimInInqOut")

    if (ncInqOutParam == NULL || varNameParam == NULL || outParam == NULL) 
        return USER__NULL_INPUT_ERR;

    if (strcmp (ncInqOutParam->type, NcInqOut_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "msiNcGetNdimsInInqOut: ncInqOutParam must be NcInqOut_MS_T. %s",
          ncInqOutParam->type);
        return (USER_PARAM_TYPE_ERR);
    } else {
	ncInqOut = (ncInqOut_t *) ncInqOutParam->inOutStruct;
    }
    if (strcmp (varNameParam->type, STR_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "msiNcGetNdimsInInqOut: varNameParam must be STR_MS_T. %s",
          varNameParam->type);
        return (USER_PARAM_TYPE_ERR);
    } else {
        name = (char*) varNameParam->inOutStruct;
    }

    if (strcmp (name, "null") == 0) {
	/* global ndims */
	ndims = ncInqOut->ndims;
    } else {
	int i;
	/* variable vndims */
	for (i = 0; i < ncInqOut->nvars; i++) {
	    if (strcmp (ncInqOut->var[i].name, name) == 0) {
		ndims = ncInqOut->var[i].nvdims;
		break;
	    }
	}
	if (ndims < 0) {
            rodsLog (LOG_ERROR,
              "msiNcGetNdimInInqOut: Unmatch variable name %s.", name);
	    return NETCDF_UNMATCHED_NAME_ERR;
	}
    }
    fillIntInMsParam (outParam, ndims);

    return 0;
}

int
msiNcGetNattsInInqOut (msParam_t *ncInqOutParam, msParam_t *varNameParam,
msParam_t *outParam, ruleExecInfo_t *rei)
{
    int natts = -1;
    ncInqOut_t *ncInqOut;
    char *name;

    RE_TEST_MACRO ("    Calling msiNcGetNattsInInqOut")

    if (ncInqOutParam == NULL || varNameParam == NULL || outParam == NULL) 
        return USER__NULL_INPUT_ERR;

    if (strcmp (ncInqOutParam->type, NcInqOut_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "msiNcGetNattsInInqOut: ncInqOutParam must be NcInqOut_MS_T. %s",
          ncInqOutParam->type);
        return (USER_PARAM_TYPE_ERR);
    } else {
	ncInqOut = (ncInqOut_t *) ncInqOutParam->inOutStruct;
    }
    if (strcmp (varNameParam->type, STR_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "msiNcGetNattsInInqOut: varNameParam must be STR_MS_T. %s",
          varNameParam->type);
        return (USER_PARAM_TYPE_ERR);
    } else {
        name = (char*) varNameParam->inOutStruct;
    }

    if (strcmp (name, "null") == 0) {
	/* global ndims */
	natts = ncInqOut->ngatts;
    } else {
	int i;
	/* variable vndims */
	for (i = 0; i < ncInqOut->nvars; i++) {
	    if (strcmp (ncInqOut->var[i].name, name) == 0) {
		natts = ncInqOut->var[i].natts;
		break;
	    }
	}
	if (natts < 0) {
            rodsLog (LOG_ERROR,
              "msiNcGetNdimInInqOut: Unmatch variable name %s.", name);
	    return NETCDF_UNMATCHED_NAME_ERR;
	}
    }
    fillIntInMsParam (outParam, natts);

    return 0;
}

int
msiNcGetNvarsInInqOut (msParam_t *ncInqOutParam, msParam_t *outParam, 
ruleExecInfo_t *rei)
{
    ncInqOut_t *ncInqOut;

    RE_TEST_MACRO ("    Calling msiNcGetNvarsInInqOut")

    if (ncInqOutParam == NULL || outParam == NULL) 
        return USER__NULL_INPUT_ERR;

    if (strcmp (ncInqOutParam->type, NcInqOut_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "msiNcGetNattsInInqOut: ncInqOutParam must be NcInqOut_MS_T. %s",
          ncInqOutParam->type);
        return (USER_PARAM_TYPE_ERR);
    } else {
	ncInqOut = (ncInqOut_t *) ncInqOutParam->inOutStruct;
    }

    /* global nvars */
    fillIntInMsParam (outParam, ncInqOut->nvars);

    return 0;
}

int
msiNcGetFormatInInqOut (msParam_t *ncInqOutParam, msParam_t *outParam, 
ruleExecInfo_t *rei)
{
    ncInqOut_t *ncInqOut;

    RE_TEST_MACRO ("    Calling msiNcGetFormatInInqOut")

    if (ncInqOutParam == NULL || outParam == NULL) 
        return USER__NULL_INPUT_ERR;

    if (strcmp (ncInqOutParam->type, NcInqOut_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "msiNcGetFormatInInqOut: ncInqOutParam must be NcInqOut_MS_T. %s",
          ncInqOutParam->type);
        return (USER_PARAM_TYPE_ERR);
    } else {
	ncInqOut = (ncInqOut_t *) ncInqOutParam->inOutStruct;
    }

    /* global nvars */
    fillIntInMsParam (outParam, ncInqOut->format);

    return 0;
}

int
msiNcGetVarNameInInqOut (msParam_t *ncInqOutParam, msParam_t *inxParam,
msParam_t *outParam, ruleExecInfo_t *rei)
{
    ncInqOut_t *ncInqOut;
    int inx;

    RE_TEST_MACRO ("    Calling msiNcGetVarNameInInqOut")

    if (ncInqOutParam == NULL || inxParam == NULL || outParam == NULL)
        return USER__NULL_INPUT_ERR;

    if (strcmp (ncInqOutParam->type, NcInqOut_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "msiNcGetVarNameInInqOut: ncInqOutParam must be NcInqOut_MS_T. %s",
          ncInqOutParam->type);
        return (USER_PARAM_TYPE_ERR);
    } else {
        ncInqOut = (ncInqOut_t *) ncInqOutParam->inOutStruct;
    }
    inx = parseMspForPosInt (inxParam);
    if (inx < 0 || inx >= ncInqOut->nvars) {
        rodsLog (LOG_ERROR,
          "msiNcGetVarNameInInqOut: input inx %d is out of range. nvars  = %d",
          inx, ncInqOut->nvars);
        return NETCDF_VAR_COUNT_OUT_OF_RANGE;
    }

    /* global nvars */
    fillStrInMsParam (outParam, ncInqOut->var[inx].name);

    return 0;
}

int
msiNcGetDimNameInInqOut (msParam_t *ncInqOutParam, msParam_t *inxParam,
msParam_t *varNameParam, msParam_t *outParam, ruleExecInfo_t *rei)
{
    ncInqOut_t *ncInqOut;
    int inx, i;
    char *name = NULL;

    RE_TEST_MACRO ("    Calling msiNcGetDimNameInInqOut")

    if (ncInqOutParam == NULL || inxParam == NULL || outParam == NULL)
        return USER__NULL_INPUT_ERR;

    if (strcmp (ncInqOutParam->type, NcInqOut_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "msiNcGetDimNameInInqOut: ncInqOutParam must be NcInqOut_MS_T. %s",
          ncInqOutParam->type);
        return (USER_PARAM_TYPE_ERR);
    } else {
        ncInqOut = (ncInqOut_t *) ncInqOutParam->inOutStruct;
    }
    inx = parseMspForPosInt (inxParam);
    if (inx < UNLIMITED_DIM_INX || inx >= ncInqOut->nvars) {
        rodsLog (LOG_ERROR,
          "msiNcGetDimNameInInqOut: input inx %d is out of range. nvars  = %d",
          inx, ncInqOut->nvars);
        return NETCDF_VAR_COUNT_OUT_OF_RANGE;
    }

    if (inx == UNLIMITED_DIM_INX) {
	/* get the name of unlimdim */
	if (ncInqOut->unlimdimid < 0) return NETCDF_NO_UNLIMITED_DIM;
	for (i = 0; i < ncInqOut->ndims; i++) {
	    if (ncInqOut->unlimdimid == ncInqOut->dim[i].id) {
		name = ncInqOut->dim[i].name;
		break;
	    }
	}
	if (name == NULL) {
            rodsLog (LOG_ERROR,
              "msiNcGetDimNameInInqOut: no match for unlimdimid %d",
              ncInqOut->unlimdimid);
            return NETCDF_NO_UNLIMITED_DIM;
	}
    } else {
	char *varName;
        if (varNameParam == NULL) return USER__NULL_INPUT_ERR;
        if (strcmp (varNameParam->type, STR_MS_T) != 0) {
            rodsLog (LOG_ERROR,
              "msiNcGetDimNameInInqOut: nameParam must be STR_MS_T. %s",
              varNameParam->type);
            return (USER_PARAM_TYPE_ERR);
        } else {
            varName = (char*) varNameParam->inOutStruct;
        }
	if (strcmp (varName, "null") == 0) {
	    /* use the global for inx */
	    name = ncInqOut->dim[inx].name;
	} else {
	    /* match the varName first */
	    for (i = 0; i < ncInqOut->nvars; i++) {
		int dimId, j;
		if (strcmp (varName, ncInqOut->var[i].name) == 0) { 
		    /* a match in var name */
		    dimId = ncInqOut->var[i].dimId[inx];
		    /* try to match dimId */
		    for (j = 0; j <  ncInqOut->ndims; j++) {
			if (ncInqOut->dim[j].id == dimId) {
			    name = ncInqOut->dim[j].name;
			    break;
			}
		    }
		}
	    }
	    if (name == NULL) {
                rodsLog (LOG_ERROR,
                  "msiNcGetDimNameInInqOut: unmatched varName %s and ix %d",
                  varName, inx);
                return NETCDF_UNMATCHED_NAME_ERR;
	    }
	}
    }
    fillStrInMsParam (outParam, name);

    return 0;
}

int
msiNcGetAttNameInInqOut (msParam_t *ncInqOutParam, msParam_t *inxParam,
msParam_t *varNameParam, msParam_t *outParam, ruleExecInfo_t *rei)
{
    ncInqOut_t *ncInqOut;
    int inx, i;
    char *varName;
    char *name = NULL;

    RE_TEST_MACRO ("    Calling msiNcGetAttNameInInqOut")

    if (ncInqOutParam == NULL || inxParam == NULL || outParam == NULL)
        return USER__NULL_INPUT_ERR;

    if (strcmp (ncInqOutParam->type, NcInqOut_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "msiNcGetAttNameInInqOut: ncInqOutParam must be NcInqOut_MS_T. %s",
          ncInqOutParam->type);
        return (USER_PARAM_TYPE_ERR);
    } else {
        ncInqOut = (ncInqOut_t *) ncInqOutParam->inOutStruct;
    }
    inx = parseMspForPosInt (inxParam);

    if (varNameParam == NULL) return USER__NULL_INPUT_ERR;

    if (strcmp (varNameParam->type, STR_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "msiNcGetAttNameInInqOut: nameParam must be STR_MS_T. %s",
          varNameParam->type);
        return (USER_PARAM_TYPE_ERR);
    } else {
        varName = (char*) varNameParam->inOutStruct;
    }

    if (strcmp (varName, "null") == 0) {
	/* use the global att */
	if (inx < 0 || inx >= ncInqOut->ngatts) {
            rodsLog (LOG_ERROR,
              "msiNcGetAttNameInInqOut: input inx %d out of range. ngatts = %d",
              inx, ncInqOut->ngatts);
            return NETCDF_VAR_COUNT_OUT_OF_RANGE;
        }
	name = ncInqOut->gatt[inx].name;
    } else {
	/* match the varName first */
	for (i = 0; i < ncInqOut->nvars; i++) {
	    if (strcmp (varName, ncInqOut->var[i].name) == 0) { 
		/* a match in var name */
		break;
	    }
        }
	if (i >= ncInqOut->nvars) {
            rodsLog (LOG_ERROR,
              "msiNcGetAttNameInInqOut: unmatched varName %s", varName);
            return NETCDF_UNMATCHED_NAME_ERR;
        }
        if (inx < 0 || inx >= ncInqOut->var[i].natts) {
            rodsLog (LOG_ERROR,
              "msiNcGetAttNameInInqOut: input inx %d out of range. natts = %d",
              inx, ncInqOut->var[i].natts);
            return NETCDF_VAR_COUNT_OUT_OF_RANGE;
        }
        name = ncInqOut->var[i].att[inx].name;
    }
    fillStrInMsParam (outParam, name);

    return 0;
}

int
msiNcGetAttValStrInInqOut (msParam_t *ncInqOutParam, msParam_t *whichAttParam,
msParam_t *varNameParam, msParam_t *outParam, ruleExecInfo_t *rei)
{
    int status;
    ncGetVarOut_t *value = NULL;
    char tempStr[NAME_LEN];
    void *bufPtr;

    RE_TEST_MACRO ("    Calling msiNcGetAttValStrInInqOut")
    status = _msiNcGetAttValInInqOut (ncInqOutParam, whichAttParam, 
      varNameParam, &value);

    if (status < 0) return status;

    bufPtr = value->dataArray->buf;
    status = ncValueToStr (value->dataArray->type, &bufPtr, tempStr);

    if (status < 0) return status;

    fillStrInMsParam (outParam, tempStr);

    return status;
}

int
_msiNcGetAttValInInqOut (msParam_t *ncInqOutParam, msParam_t *whichAttParam,
msParam_t *varNameParam, ncGetVarOut_t **ncGetVarOut)
{
    ncInqOut_t *ncInqOut;
    int i;
    int inx;
    char *varName, *attName;
    ncGetVarOut_t *value = NULL;

    if (ncInqOutParam == NULL || whichAttParam == NULL || ncGetVarOut == NULL)
        return USER__NULL_INPUT_ERR;

    *ncGetVarOut = NULL;

    if (strcmp (ncInqOutParam->type, NcInqOut_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "_msiNcGetAttValInInqOut: ncInqOutParam must be NcInqOut_MS_T. %s",
          ncInqOutParam->type);
        return (USER_PARAM_TYPE_ERR);
    } else {
        ncInqOut = (ncInqOut_t *) ncInqOutParam->inOutStruct;
    }
    /* whichAttParam can be inx or attName */
    if (strcmp (whichAttParam->type, STR_MS_T) == 0) {
	attName = (char *)whichAttParam->inOutStruct;
	inx = -1;
    } else if (strcmp (whichAttParam->type, INT_MS_T) == 0) {
	inx = *((int *) whichAttParam->inOutStruct);
	attName = NULL;
    } else {
        rodsLog (LOG_ERROR,
          "_msiNcGetAttValInInqOut:whichAttParam must be INT_MS_T/STR_MS_T. %s",
          whichAttParam->type);
        return (USER_PARAM_TYPE_ERR);
    }
    if (varNameParam == NULL) return USER__NULL_INPUT_ERR;

    if (strcmp (varNameParam->type, STR_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "_msiNcGetAttValInInqOut: varNameParam must be STR_MS_T. %s",
          varNameParam->type);
        return (USER_PARAM_TYPE_ERR);
    } else {
        varName = (char*) varNameParam->inOutStruct;
    }

    if (strcmp (varName, "null") == 0) {
	/* use the global att */
	if (attName == NULL) {
	    if (inx < 0 || inx >= ncInqOut->ngatts) {
                rodsLog (LOG_ERROR,
                  "_msiNcGetAttValInInqOut:inp inx %d out of range. ngatts=%d",
                  inx, ncInqOut->ngatts);
                return NETCDF_VAR_COUNT_OUT_OF_RANGE;
            }
	    value = &ncInqOut->gatt[inx].value;
	} else {
	    /* input is a att name */
	    for (i = 0; i < ncInqOut->ngatts; i++) {
		if (strcmp (attName, ncInqOut->gatt[i].name) == 0) {
		    value = &ncInqOut->gatt[i].value;
		    break;
		}
	    }
	    if (value == NULL) {
                rodsLog (LOG_ERROR,
                  "_msiNcGetAttValInInqOut: unmatched attName %s", attName);
                return NETCDF_UNMATCHED_NAME_ERR;
            }
	}
    } else {
	/* match the varName first */
	for (i = 0; i < ncInqOut->nvars; i++) {
	    if (strcmp (varName, ncInqOut->var[i].name) == 0) { 
		/* a match in var name */
		break;
	    }
        }
	if (i >= ncInqOut->nvars) {
            rodsLog (LOG_ERROR,
              "_msiNcGetAttValInInqOut: unmatched varName %s", varName);
            return NETCDF_UNMATCHED_NAME_ERR;
        }
        if (attName == NULL) {
            if (inx < 0 || inx >= ncInqOut->var[i].natts) {
                rodsLog (LOG_ERROR,
                  "_msiNcGetAttNameInInqOut:inp inx %d out of range. natts=%d",
                  inx, ncInqOut->var[i].natts);
                return NETCDF_VAR_COUNT_OUT_OF_RANGE;
            }
            value = &ncInqOut->var[i].att[inx].value;
        } else {
            /* input is a att name */
	    int j;
            for (j = 0; j < ncInqOut->ngatts; j++) {
                if (strcmp (attName, ncInqOut->var[i].att[j].name) == 0) {
                    value = &ncInqOut->var[i].att[j].value;
                    break;
                }
            }
            if (value == NULL) {
                rodsLog (LOG_ERROR,
                  "_msiNcGetAttValInInqOut: unmatched attName %s", attName);
                return NETCDF_UNMATCHED_NAME_ERR;
            }
	}
    }
    *ncGetVarOut = value;

    return 0;
}

int
msiNcGetVarTypeInInqOut (msParam_t *ncInqOutParam, msParam_t *varNameParam, 
msParam_t *outParam, ruleExecInfo_t *rei)
{
    ncInqOut_t *ncInqOut;
    int i;
    char *varName;

    RE_TEST_MACRO ("    Calling msiNcGetVarTypeInInqOut")

    if (ncInqOutParam == NULL || outParam == NULL || varNameParam == NULL)
        return USER__NULL_INPUT_ERR;

    if (strcmp (ncInqOutParam->type, NcInqOut_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "msiNcGetVarTypeInInqOut: ncInqOutParam must be NcInqOut_MS_T. %s",
          ncInqOutParam->type);
        return (USER_PARAM_TYPE_ERR);
    } else {
        ncInqOut = (ncInqOut_t *) ncInqOutParam->inOutStruct;
    }

    if (strcmp (varNameParam->type, STR_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "msiNcGetAttNameInInqOut: nameParam must be STR_MS_T. %s",
          varNameParam->type);
        return (USER_PARAM_TYPE_ERR);
    } else {
        varName = (char*) varNameParam->inOutStruct;
    }

    /* match the varName */
    for (i = 0; i < ncInqOut->nvars; i++) {
        if (strcmp (varName, ncInqOut->var[i].name) == 0) { 
            /* a match in var name */
	    break;
	}
    }
    if (i >= ncInqOut->nvars) {
        rodsLog (LOG_ERROR,
          "msiNcGetAttNameInInqOut: unmatched varName %s", varName);
        return NETCDF_UNMATCHED_NAME_ERR;
    }
    fillIntInMsParam (outParam, ncInqOut->var[i].dataType);

    return 0;
}

int
msiNcIntDataTypeToStr (msParam_t *dataTypeParam, msParam_t *outParam, 
ruleExecInfo_t *rei)
{
    int dataType;
    char dataTypeStr[NAME_LEN];
    int status;

    RE_TEST_MACRO ("    Calling msiNcIntDataTypeToStr")

    if (dataTypeParam == NULL || outParam == NULL) return USER__NULL_INPUT_ERR;

    if (strcmp (dataTypeParam->type, INT_MS_T) != 0) {
        rodsLog (LOG_ERROR,
          "msiNcIntDataTypeToStr: Unsupported input dataTypeParam type %s",
          dataTypeParam->type);
        return (USER_PARAM_TYPE_ERR);
    }

    dataType = *((int *) dataTypeParam->inOutStruct);

    if ((status = getNcTypeStr (dataType, dataTypeStr)) < 0) return status; 

    fillStrInMsParam (outParam, dataTypeStr);

    return 0;
}

int
msiAddToNcArray (msParam_t *elementParam, msParam_t *ncArrayParam,
ruleExecInfo_t *rei)
{
    ncGetVarOut_t *ncArray;

    RE_TEST_MACRO ("    Calling msiAddToNcArray")

    if (elementParam == NULL || ncArrayParam == NULL) 
        return USER__NULL_INPUT_ERR;

    if (strcmp (elementParam->type, INT_MS_T) == 0) {
	int *intArray;
	int len;
	if (ncArrayParam->inOutStruct == NULL) {
	    /* first time */
	    ncArray = (ncGetVarOut_t *) calloc (1, sizeof (ncGetVarOut_t));
	    ncArray->dataArray = (dataArray_t *) 
	        calloc (1, sizeof (dataArray_t));
	    ncArray->dataArray->type = NC_INT;
	    rstrcpy (ncArray->dataType_PI, "intDataArray_PI", NAME_LEN);
	    ncArray->dataArray->buf  = calloc (1, sizeof (int) * NC_MAX_DIMS);
	    fillMsParam (ncArrayParam, NULL, NcGetVarOut_MS_T, ncArray, NULL);
	} else {
	    ncArray = (ncGetVarOut_t *) ncArrayParam->inOutStruct;
	    if (strcmp (ncArray->dataType_PI, "intDataArray_PI") != 0) {
                rodsLog (LOG_ERROR, 
                  "msiAddToNcArray: wrong dataType_PI for INT_MS_T %s",
                  ncArray->dataType_PI);
                return (USER_PARAM_TYPE_ERR);
	    }
        }
	intArray = (int *) ncArray->dataArray->buf;
	len = ncArray->dataArray->len;
	if (len >= NC_MAX_DIMS) return NETCDF_VAR_COUNT_OUT_OF_RANGE;
	intArray[len] = *((int *) elementParam->inOutStruct);
	ncArray->dataArray->len++;
    } else {
	/* only do INT_MS_T for now */
        rodsLog (LOG_ERROR,
          "msiAddToNcArray: Unsupported input dataTypeParam type %s",
          elementParam->type);
        return (USER_PARAM_TYPE_ERR);
    }
    return 0;
}

