/*** Copyright (c), The Unregents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* unregDataObj.c
 */

#include "modDataObjMeta.h"
#include "icatHighLevelRoutines.h"

int
rsModDataObjMeta (rsComm_t *rsComm, modDataObjMeta_t *modDataObjMetaInp)
{
    int status;
    rodsServerHost_t *rodsServerHost = NULL;
    dataObjInfo_t *dataObjInfo;
    keyValPair_t *regParam;

    regParam = modDataObjMetaInp->regParam;
    dataObjInfo = modDataObjMetaInp->dataObjInfo;

    status = getAndConnRcatHost (rsComm, MASTER_RCAT, dataObjInfo->objPath,
      &rodsServerHost);
    if (status < 0) {
       return(status);
    }
    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
        status = _rsModDataObjMeta (rsComm, modDataObjMetaInp);
#else
        status = SYS_NO_RCAT_SERVER_ERR;
#endif
    } else {
        status = rcModDataObjMeta (rodsServerHost->conn, modDataObjMetaInp);
    }

    return (status);
}

int
_rsModDataObjMeta (rsComm_t *rsComm, modDataObjMeta_t *modDataObjMetaInp)
{
#ifdef RODS_CAT
    int status;
    dataObjInfo_t *dataObjInfo;
    keyValPair_t *regParam;

    regParam = modDataObjMetaInp->regParam;
    dataObjInfo = modDataObjMetaInp->dataObjInfo;

    if (regParam->len == 0) {
        return (0);
    }

    status = chlModDataObjMeta (rsComm, dataObjInfo, regParam);
    return (status);
#else
    return (SYS_NO_RCAT_SERVER_ERR);
#endif

}


