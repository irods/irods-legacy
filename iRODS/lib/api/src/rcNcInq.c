/* This is script-generated code.  */ 
/* See ncInq.h for a description of this API call.*/

#include "ncInq.h"

int
rcNcInq (rcComm_t *conn, ncInqIdInp_t *ncInqInp, ncInqOut_t **ncInqOut)
{
    int status;
    status = procApiRequest (conn, NC_INQ_AN, ncInqInp, NULL, 
        (void **) ncInqOut, NULL);

    return (status);
}

int
initNcInqOut (int ndims, int nvars, int ngatts, int unlimdimid, int format,
ncInqOut_t **ncInqOut)
{
    *ncInqOut = (ncInqOut_t *) calloc (1, sizeof (ncInqOut_t));
    (*ncInqOut)->ndims = ndims;
    (*ncInqOut)->nvars = nvars;
    (*ncInqOut)->ngatts = ngatts;
    (*ncInqOut)->unlimdimid = unlimdimid;
    (*ncInqOut)->format = format;

    if (ndims > 0)
        (*ncInqOut)->dim = (ncGenDimOut_t *) 
	  calloc (ndims, sizeof (ncGenDimOut_t));
    if (nvars > 0) 
        (*ncInqOut)->var = (ncGenVarOut_t *) 
	  calloc (nvars, sizeof (ncGenVarOut_t));
    if (ngatts > 0)
        (*ncInqOut)->gatt = (ncGenAttOut_t *) 
	  calloc (ngatts, sizeof (ncGenAttOut_t));

    return 0;
}

int
freeNcInqOut (ncInqOut_t **ncInqOut)
{
    int i;

    if (ncInqOut == NULL || *ncInqOut == NULL) return USER__NULL_INPUT_ERR;

    if ((*ncInqOut)->dim != NULL) free ((*ncInqOut)->dim);
    if ((*ncInqOut)->gatt != NULL) free ((*ncInqOut)->gatt);
    if ((*ncInqOut)->var != NULL) {
	for (i = 0; i < (*ncInqOut)->nvars; i++) {
	    if ((*ncInqOut)->var[i].att != NULL) {
		clearNcGetVarOut (&(*ncInqOut)->var[i].att->value);
		free ((*ncInqOut)->var[i].att);
	    }
	}
	free ((*ncInqOut)->var);
    }
    free (*ncInqOut);
    *ncInqOut = NULL;

    return 0;
}

