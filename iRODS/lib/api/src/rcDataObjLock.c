/**
 * @file  rcDataObjLock.c
 *
 */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjLock.h for a description of this API call.*/

#include "dataObjLock.h"

/**
 * \fn rcDataObjLock (rcComm_t *conn, dataObjInp_t *dataObjInp)
 *
 * \brief Lock a data object.
 *
 * \user client
 *
 * \category data object operations
 *
 * \since 1.0
 *
 * \author  Mike Wan
 * \date    2007
 *
 * \remark none
 *
 * \note none
 *
 * \usage
 * Lock a data object /myZone/home/john/myfile in myRescource for write:
 * \n dataObjInp_t dataObjInp;
 * \n bzero (&dataObjInp, sizeof (dataObjInp));
 * \n rstrcpy (dataObjInp.objPath, "/myZone/home/john/myfile", MAX_NAME_LEN);
 * \n dataObjInp.openFlags = O_WRONLY;
 * \n addKeyVal (&dataObjInp.condInput, RESC_NAME_KW, "myRescource");
 * \n status = rcDataObjLock (conn, &dataObjInp);
 * \n if (status < 0) {
 * \n .... handle the error
 * \n }
 *
 * \param[in] conn - A rcComm_t connection handle to the server.
 * \param[in] dataObjInp - Elements of dataObjInp_t used :
 *    \li char \b objPath[MAX_NAME_LEN] - full path of the data object.
 *    \li keyValPair_t \b condInput - keyword/value pair input. Valid keywords:
 *
 * \return integer
 * \retval an opened and locked file descriptor on success

 * \sideeffect none
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/

int
rcDataObjLock (rcComm_t *conn, dataObjInp_t *dataObjInp)
{
    int status;
    status = procApiRequest (conn, DATA_OBJ_LOCK_AN,  dataObjInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}

