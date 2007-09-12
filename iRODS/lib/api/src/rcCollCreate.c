/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See collCreate.h for a description of this API call.*/

#include "collCreate.h"

int
rcCollCreate (rcComm_t *conn, collInp_t *collCreateInp)
{
    int status;
    status = procApiRequest (conn, COLL_CREATE_AN,  collCreateInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}
