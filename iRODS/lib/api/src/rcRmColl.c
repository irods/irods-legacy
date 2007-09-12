/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See rmColl.h for a description of this API call.*/

#include "rmColl.h"

int
rcRmColl (rcComm_t *conn, collInp_t *rmCollInp)
{
    int status;
    status = procApiRequest (conn, RM_COLL_AN, rmCollInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}
