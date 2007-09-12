/* This is script-generated code.  */ 
/* See bunSubStat.h for a description of this API call.*/

#include "bunSubStat.h"

int
rcBunSubStat (rcComm_t *conn, subFile_t *subFile,
rodsStat_t **bunSubStatOut)
{
    int status;
    status = procApiRequest (conn, BUN_SUB_STAT_AN, subFile, NULL, 
        (void **) bunSubStatOut, NULL);

    return (status);
}
