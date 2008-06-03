/* This is script-generated code.  */ 
/* See collRepl.h for a description of this API call.*/

#include "collRepl.h"

int
rcCollRepl (rcComm_t *conn, dataObjInp_t *collReplInp)
{
    int status;
    transStat_t *transStat = NULL;

    memset (&conn->transStat, 0, sizeof (transStat_t));

    collReplInp->oprType = REPLICATE_OPR;

    status = procApiRequest (conn, COLL_REPL_AN, collReplInp, NULL, 
        (void **) &transStat, NULL);

    if (status >= 0 && transStat != NULL) {
        conn->transStat = *(transStat);
    }

    if (transStat != NULL) {
        free (transStat);
    }

    return (status);
}
