/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjRename.h for a description of this API call.*/

#include "dataObjRename.h"

int
rcDataObjRename (rcComm_t *conn, dataObjCopyInp_t *dataObjRenameInp)
{
    int status;


    status = procApiRequest (conn, DATA_OBJ_RENAME_AN,  dataObjRenameInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}
