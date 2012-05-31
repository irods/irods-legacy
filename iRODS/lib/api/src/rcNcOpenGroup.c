/* This is script-generated code.  */ 
/* See ncOpenGroup.h for a description of this API call.*/

#include "ncOpenGroup.h"

int
rcNcOpenGroup (rcComm_t *conn, ncOpenInp_t *ncOpenGroupInp, int *ncid)
{
    int status;
   int *myncid = NULL;

    status = procApiRequest (conn, NC_OPEN_GROUP_AN, ncOpenGroupInp, NULL, 
        (void **) &myncid, NULL);

    if (myncid != NULL) {
        *ncid = *myncid;
        free (myncid);
    }

    return (status);
}

int
_rcNcOpenGroup (rcComm_t *conn, ncOpenInp_t *ncOpenGroupInp, int **ncid)
{
    int status;
    status = procApiRequest (conn, NC_OPEN_GROUP_AN,  ncOpenGroupInp, NULL,
        (void **) ncid, NULL);

    return (status);
}

