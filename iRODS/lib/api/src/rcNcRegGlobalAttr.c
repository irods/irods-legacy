/* This is script-generated code.  */ 
/* See ncRegGlobalAttr.h for a description of this API call.*/

#include "ncRegGlobalAttr.h"

int
rcNcRegGlobalAttr (rcComm_t *conn, ncRegGlobalAttrInp_t *ncRegGlobalAttrInp)
{
    int status;
    status = procApiRequest (conn, NC_REG_GLOBAL_ATTR_AN, ncRegGlobalAttrInp, 
        NULL, (void **) NULL, NULL);

    return (status);
}
