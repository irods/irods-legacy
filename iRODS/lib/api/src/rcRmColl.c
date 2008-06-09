/* This is script-generated code.  */ 
/* See rmCollRecur.h for a description of this API call.*/

#include "rmColl.h"

int
_rcRmColl (rcComm_t *conn, collInp_t *rmCollInp, 
collOprStat_t **collOprStat)
{
    int status;
    status = procApiRequest (conn, RM_COLL_AN, rmCollInp, NULL, 
        (void **) collOprStat, NULL);

    return (status);
}

int
rcRmColl (rcComm_t *conn, collInp_t *rmCollInp, int vFlag)
{
    int status;
    collOprStat_t *collOprStat = NULL;

    status = _rcRmColl (conn, rmCollInp, &collOprStat);

    status = cliGetCollOprStat (conn, collOprStat, vFlag);

    return (status);
}

