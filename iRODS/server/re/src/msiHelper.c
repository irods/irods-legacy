/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* reHelper.c */

#include "msiHelper.h"
int
msiGetStdoutInExecCmdOut (msParam_t *inpExecCmdOut, msParam_t *outStr,
ruleExecInfo_t *rei)
{
    char *strPtr;

    rei->status = getStdoutInExecCmdOut (inpExecCmdOut, &strPtr);

    if (rei->status < 0) return rei->status;

    fillStrInMsParam (outStr, strPtr);

    return rei->status;
}

int
msiGetStderrInExecCmdOut (msParam_t *inpExecCmdOut, msParam_t *outStr,
ruleExecInfo_t *rei)
{
    char *strPtr;

    rei->status = getStderrInExecCmdOut (inpExecCmdOut, &strPtr);

    if (rei->status < 0) return rei->status;

    fillStrInMsParam (outStr, strPtr);

    return rei->status;
}

/*
 * \fn msiWriteRodsLog
 * \author Jean-Yves Neif
 * \date   2009-06-15
 * \brief This micro-service can be used to write message into server rodsLog.
 * \note This call should only be used through the rcExecMyRule (irule) call
 *  i.e., rule execution initiated by clients and should not be called
 *  internally by the server since it interacts with the client through
 *  the normal client/server socket connection.
 * \param[in]
 *    irpParam1 - a STR_MS_T which specifies the message to log.
 * \param[out] a INT_MS_T containing the status.
 * \return integer
 * \retval 0 on success
 * \sa
 * \post
 * \pre
 * \bug  no known bugs
 */
int
msiWriteRodsLog (msParam_t *inpParam1,  msParam_t *outParam,
ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;

    RE_TEST_MACRO (" Calling msiWriteRodsLog")

    if (rei == NULL || rei->rsComm == NULL) {
        rodsLog (LOG_ERROR,
        "msiWriteRodsLog: input rei or rsComm is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    if ( inpParam1 == NULL ) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
        "msiWriteRodsLog: input Param1 is NULL");
        rei->status = USER__NULL_INPUT_ERR;
        return (rei->status);
    }

    if (strcmp (inpParam1->type, STR_MS_T) == 0) {
        rodsLog(LOG_NOTICE,
          "msiWriteRodsLog message: %s", inpParam1->inOutStruct);
    } else {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
        "msiWriteRodsLog: Unsupported input Param1 types %s",
        inpParam1->type);
        rei->status = UNKNOWN_PARAM_IN_RULE_ERR;
        return (rei->status);
    }

    rei->status = 0;

    fillIntInMsParam (outParam, rei->status);

    return (rei->status);
}

int
msiAddKeyValToMspStr (msParam_t *keyStr, msParam_t *valStr, 
msParam_t *msKeyValStr, ruleExecInfo_t *rei)
{
    RE_TEST_MACRO (" Calling msiAddKeyValToMspStr")

    if (rei == NULL) {
        rodsLog (LOG_ERROR,
          "msiAddKeyValToMspStr: input rei is NULL");
        rei->status = SYS_INTERNAL_NULL_INPUT_ERR;
        return (rei->status);
    }

    rei->status = addKeyValToMspStr (keyStr, valStr, msKeyValStr);


    return rei->status;
}

int
msiSplitPath (msParam_t *inpPath,  msParam_t *outParentColl, 
msParam_t *outChildName, ruleExecInfo_t *rei)
{
    char parent[MAX_NAME_LEN], child[MAX_NAME_LEN];

    RE_TEST_MACRO (" Calling msiSplitPath")

    if (rei == NULL) {
        rodsLog (LOG_ERROR,
          "msiSplitPath: input rei is NULL");
        rei->status = SYS_INTERNAL_NULL_INPUT_ERR;
        return (rei->status);
    }

    if ( inpPath == NULL ) {
        rodsLog (LOG_ERROR,
          "msiSplitPath: input inpPath is NULL");
        rei->status = USER__NULL_INPUT_ERR;
        return (rei->status);
    }

    if (strcmp (inpPath->type, STR_MS_T) == 0) {
        if ((rei->status = splitPathByKey ((char *) inpPath->inOutStruct,
          parent, child, '/')) < 0) {
            rodsLog (LOG_ERROR,
              "msiSplitPath: splitPathByKey for %s error, status = %d",
              (char *) inpPath->inOutStruct, rei->status);
        } else {
	    fillStrInMsParam (outParentColl, parent);
	    fillStrInMsParam (outChildName, child);
	}
    } else {
        rodsLog (LOG_ERROR,
        "msiSplitPath: Unsupported input inpPath types %s",
        inpPath->type);
        rei->status = UNKNOWN_PARAM_IN_RULE_ERR;
    }
    return (rei->status);
}
