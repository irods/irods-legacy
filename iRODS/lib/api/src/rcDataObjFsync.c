#include "dataObjFsync.h"

int
rcDataObjFsync (rcComm_t *conn, openedDataObjInp_t *dataObjFsyncInp)
{
   int status;
   rodsLog (LOG_NOTICE, "rcDataObjFsync calling procApiRequest");
   status = procApiRequest (conn, DATA_OBJ_FSYNC_AN, dataObjFsyncInp, NULL,
                            NULL, NULL);

   return (status);
}

