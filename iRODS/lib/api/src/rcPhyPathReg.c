/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See phyPathReg.h for a description of this API call.*/

#include "phyPathReg.h"

int
rcPhyPathReg (rcComm_t *conn, dataObjInp_t *phyPathRegInp)
{
    int status;
    status = procApiRequest (conn, PHY_PATH_REG_AN,  phyPathRegInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}
