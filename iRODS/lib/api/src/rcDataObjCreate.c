/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjCreate.h for a description of this API call.*/

#include "dataObjCreate.h"

/**
 * \fn rcDataObjCreate (rcComm_t *conn, dataObjInp_t *dataObjInp)
 *
 * \brief Creates a data object in the iCAT.
 *
 * \module dataObjOpr
 *
 * \since 1.0
 *
 * \author  Mike Wan
 * \date    2007
 *
 * \remark Mike Wan - C API documentation, 2011-09-12
 *
 * \note none
 *
 * \usage
 *
 *
 * \param[in] conn - A rcComm_t connection handle to the server.
 * \param[in] dataObjInp - A dataObjInp_t data object input type. 
 *   Elements of dataObjInp_t used :
 *    \li "objPath" - the full path of the data objection.
 *    \li "forceFlag" - overwrite existing copy. This keyWd has
 *        no value. But the '=' character is still needed
 *    \li "createMode" - the file mode of the data object.
 *    \li "dataType" - the data type of the data object.
 *    \li "dataSize" - the size of the data object. This input is optional.
 * \param[out] outParam - a INT_MS_T containing the descriptor of the create.
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence none
 * \DolVarModified none
 * \iCatAttrDependence none
 * \iCatAttrModified none
 * \sideeffect none
 *
 * \return integer
 * \retval 0 on success
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/

int
rcDataObjCreate (rcComm_t *conn, dataObjInp_t *dataObjInp)
{
    int status;
    status = procApiRequest (conn, DATA_OBJ_CREATE_AN,  dataObjInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}

