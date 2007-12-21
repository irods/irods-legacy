/* This is script-generated code.  */ 
/* See bunSubStat.h for a description of this API call.*/

#include "subStructFileStat.h"

int
rcBunSubStat (rcComm_t *conn, subFile_t *subFile,
rodsStat_t **bunSubStatOut)
{
    int status;
    status = procApiRequest (conn, SUB_STRUCT_FILE_STAT_AN, subFile, NULL, 
        (void **) bunSubStatOut, NULL);

    return (status);
}
