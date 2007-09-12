/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjTrim.h for a description of this API call.*/

#include "objStat.h"

int
rcObjStat (rcComm_t *conn, dataObjInp_t *dataObjInp,
rodsObjStat_t **rodsObjStatOut)
{
    int status;

    status = procApiRequest (conn, OBJ_STAT_AN,  dataObjInp, NULL, 
        (void **) rodsObjStatOut, NULL);

    return (status);
}

