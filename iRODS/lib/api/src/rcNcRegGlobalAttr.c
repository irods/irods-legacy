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

int
clearRegGlobalAttrInp (ncRegGlobalAttrInp_t *ncRegGlobalAttrInp)
{
    int i;

    if (ncRegGlobalAttrInp == NULL || ncRegGlobalAttrInp->numAttrName <= 0 ||
      ncRegGlobalAttrInp->attrNameArray == NULL)
        return (0);

    for (i = 0; i < ncRegGlobalAttrInp->numAttrName; i++) {
        free (ncRegGlobalAttrInp->attrNameArray[i]);
    }
    free (ncRegGlobalAttrInp->attrNameArray);
    clearKeyVal (&ncRegGlobalAttrInp->condInput);
    memset (ncRegGlobalAttrInp, 0, sizeof (ncRegGlobalAttrInp_t));
    return(0);

}

