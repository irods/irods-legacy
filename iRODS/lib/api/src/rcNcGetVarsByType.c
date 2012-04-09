/* This is script-generated code.  */ 
/* See ncGetVarsByType.h for a description of this API call.*/

#include "ncGetVarsByType.h"

int
rcNcGetVarsByType (rcComm_t *conn,  ncGetVarInp_t *ncGetVarInp,
ncGetVarOut_t **ncGetVarOut)
{
    int status;
    status = procApiRequest (conn, NC_GET_VARS_BY_TYPE_AN, ncGetVarInp, NULL, 
        (void **) ncGetVarOut, NULL);

    return (status);
}

int
freeNcGetVarOut (ncGetVarOut_t **ncGetVarOut)
{
    if (ncGetVarOut == NULL || *ncGetVarOut == NULL) return 
      USER__NULL_INPUT_ERR;

    clearNcGetVarOut (*ncGetVarOut);
    free (*ncGetVarOut);
    *ncGetVarOut = NULL;
    return 0;
}

int
clearNcGetVarOut (ncGetVarOut_t *ncGetVarOut)
{
    if (ncGetVarOut == NULL) return USER__NULL_INPUT_ERR;

    if ((ncGetVarOut)->dataArray != NULL) {
        if ((ncGetVarOut)->dataArray->buf != NULL) {
            free ((ncGetVarOut)->dataArray->buf);
        }
        free ((ncGetVarOut)->dataArray);
    }
    return 0;
}

int
clearNcGetVarInp (ncGetVarInp_t *ncGetVarInp)
{
    if (ncGetVarInp == NULL) return USER__NULL_INPUT_ERR;
    if (ncGetVarInp->start != NULL) {
	free (ncGetVarInp->start);
	ncGetVarInp->start = NULL;
    }
    if (ncGetVarInp->count != NULL) {
        free (ncGetVarInp->count);
        ncGetVarInp->count = NULL;
    }
    if (ncGetVarInp->stride != NULL) {
        free (ncGetVarInp->stride);
        ncGetVarInp->stride = NULL;
    }
    clearKeyVal (&ncGetVarInp->condInput);
    return 0;
}

