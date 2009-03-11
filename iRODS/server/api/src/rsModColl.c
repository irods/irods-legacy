/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* modColl.c
 */

#include "modColl.h"
#include "icatHighLevelRoutines.h"

int
rsModColl (rsComm_t *rsComm, collInp_t *modCollInp)
{
    int status;
    rodsServerHost_t *rodsServerHost = NULL;

    status = getAndConnRcatHost (rsComm, MASTER_RCAT, modCollInp->collName,
                                &rodsServerHost);
    if (status < 0) {
       return(status);
    }
    if (rodsServerHost->localFlag == LOCAL_HOST) {
#ifdef RODS_CAT
        status = _rsModColl (rsComm, modCollInp);
#else
        status = SYS_NO_RCAT_SERVER_ERR;
#endif
    } else {
        status = rcModColl (rodsServerHost->conn, modCollInp);
    }

    return (status);
}

int
_rsModColl (rsComm_t *rsComm, collInp_t *modCollInp)
{
#ifdef RODS_CAT
    int status;
    collInfo_t collInfo;
    char *tmpStr;

    memset (&collInfo, 0, sizeof (collInfo));

    rstrcpy (collInfo.collName, modCollInp->collName, MAX_NAME_LEN);

    if ((tmpStr = getValByKey (&modCollInp->condInput,
      COLLECTION_TYPE_KW)) != NULL) {
        rstrcpy (collInfo.collType, tmpStr, NAME_LEN);
    }
    if ((tmpStr = getValByKey (&modCollInp->condInput,
      COLLECTION_INFO1_KW)) != NULL) {
        rstrcpy (collInfo.collInfo1, tmpStr, MAX_NAME_LEN);
    }
    if ((tmpStr = getValByKey (&modCollInp->condInput,
      COLLECTION_INFO2_KW)) != NULL) {
        rstrcpy (collInfo.collInfo2, tmpStr, MAX_NAME_LEN);
    }

    status = chlModColl (rsComm, &collInfo);
    /* XXXX need to commit */
    if (status >= 0) status = chlCommit(rsComm);

    return (status);
#else
    return (SYS_NO_RCAT_SERVER_ERR);
#endif
}

#ifdef COMPAT_201
int
rsModColl201 (rsComm_t *rsComm, collInp201_t *modCollInp)
{
    collInp_t collInp;
    int status; 

    collInp201ToCollInp (modCollInp, &collInp);

    status = rsModColl (rsComm, &collInp);

    return status;
}
#endif

