/* This is script-generated code.  */ 
/* See regReplica.h for a description of this API call.*/

#include "regReplica.h"

int
rcRegReplica (rcComm_t *conn, regReplica_t *regReplicaInp)
{
    int status;
    status = procApiRequest (conn, REG_REPLICA_AN, regReplicaInp, NULL, 
        (void **) NULL, NULL);

    return (status);
}
