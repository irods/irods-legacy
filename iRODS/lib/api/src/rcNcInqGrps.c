/* This is script-generated code.  */ 
/* See ncInqGrps.h for a description of this API call.*/

#include "ncInqGrps.h"

int
rcNcInqGrps (rcComm_t *conn, ncInqGrpsInp_t *ncInqGrpsInp,
ncInqGrpsOut_t **ncInqGrpsOut)
{
    int status;
    status = procApiRequest (conn, NC_INQ_GRPS_AN, ncInqGrpsInp, NULL, 
        (void **) ncInqGrpsOut, NULL);

    return (status);
}

int
freeNcInqGrpsOut (ncInqGrpsOut_t **ncInqGrpsOut)
{
    ncInqGrpsOut_t *myNInqGrpsOut;
    int i;

    if (ncInqGrpsOut == NULL || *ncInqGrpsOut == NULL) return 0;

    myNInqGrpsOut = *ncInqGrpsOut;
    for (i = 0; i < myNInqGrpsOut->ngrps; i++) {
	free (myNInqGrpsOut->grpName[i]);
    }
    if (myNInqGrpsOut->grpName != NULL) free (myNInqGrpsOut->grpName);
    free (myNInqGrpsOut);
    *ncInqGrpsOut = NULL;
    return 0;
}

