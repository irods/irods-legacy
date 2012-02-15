/**
 *  * @file  reNetcdf.c
 *   *
 *    */

/*** Copyright (c), The Regents of the University of California            ***
 *  *** For more information please refer to files in the COPYRIGHT directory ***/

#if 0
#include "reNetcdf.h"
#endif
#include "apiHeaderAll.h"
#include "rsApiHandler.h"
#include "collection.h"

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

    if (inpParam1 == NULL) {
        rodsLog (LOG_ERROR,
          "msiNcInqId: input inpParam1 is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    /* parse for myid or ncInqWithIdInp_t */
    rei->status = parseMspForNcInqIdInp (inpParam1, &ncInqIdInp);

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

    if (inpParam1 == NULL) {
        rodsLog (LOG_ERROR,
          "msiNcInqWithId: input inpParam1 is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    /* parse for myid or ncInqWithIdInp_t */
    rei->status = parseMspForNcInqIdInp (inpParam1, &ncInqWithIdInp);

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
	fillMsParam (outParam, NULL, NcInqWithIdInp_PI, ncInqWithIdOut, NULL);
    } else {
      rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
        "msiNcInqWithId: rsNcInqWithId failed for %s, status = %d",
        ncInqWithIdInp.name, rei->status);
    }

    return (rei->status);
}


