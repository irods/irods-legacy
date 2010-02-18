/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* See genQuery.h for a description of this API call.*/

#include "getRescQuota.h"
#include "miscUtil.h"
#include "genQuery.h"

int
rsGetRescQuota (rsComm_t *rsComm, getRescQuotaInp_t *getRescQuotaInp,
rescQuota_t **rescQuota)
{
    rodsServerHost_t *rodsServerHost;
    int status = 0;

    status = getAndConnRcatHost (rsComm, SLAVE_RCAT, 
      getRescQuotaInp->zoneHint, &rodsServerHost);

    if (status < 0) return(status);

    if (rodsServerHost->localFlag == LOCAL_HOST) {
	status = _rsGetRescQuota (rsComm, getRescQuotaInp, rescQuota);
    } else {
	status = rcGetRescQuota (rodsServerHost->conn, getRescQuotaInp, 
	  rescQuota);
    }

    return status;
}

int
setRescQuota (rsComm_t *rsComm, char *zoneHint, 
rescGrpInfo_t **rescGrpInfoHead)
{
    rescGrpInfo_t *tmpRescGrpInfo;
    int needInit = 0;
    int status = 0;

    /* XXXXXX check the rule first */
    if (rescGrpInfoHead == NULL) return USER__NULL_INPUT_ERR;
    tmpRescGrpInfo = *rescGrpInfoHead;
    while (tmpRescGrpInfo != NULL) {
	if (tmpRescGrpInfo->rescInfo->quotaLimit == -1) {
	    needInit = 1;
	    break;
	}
	tmpRescGrpInfo = tmpRescGrpInfo->next;
    }
    return status;
}

int
_rsGetRescQuota (rsComm_t *rsComm, getRescQuotaInp_t *getRescQuotaInp,
rescQuota_t **rescQuota)
{
    int status = 0;
    genQueryOut_t *genQueryOut = NULL;

    if (rescQuota == NULL) return USER__NULL_INPUT_ERR;

    *rescQuota = NULL;

    /* assume it is a resource */

    status = getQuotaByResc (rsComm, getRescQuotaInp->userName,
      getRescQuotaInp->rescName, &genQueryOut);

#if 0
    if (status > = 0) {
        queRescQuota (rescQuota, genQueryOut);
        return status;
    }
#endif
    return 0;
}

/* getQuotaByResc - get the quoto for an individual resource. The code is
 * mostly from doTest7 of iTestGenQuery.c
 */

int
getQuotaByResc (rsComm_t *rsComm, char *userName, char *rescName,
genQueryOut_t **genQueryOut) 
{
    int status;
    genQueryInp_t genQueryInp;
    char condition1[MAX_NAME_LEN];
    char condition2[MAX_NAME_LEN];

    if (genQueryOut == NULL) return USER__NULL_INPUT_ERR;

    *genQueryOut = NULL;
    memset (&genQueryInp, 0, sizeof (genQueryInp));

    genQueryInp.options = QUOTA_QUERY;

    snprintf (condition1, MAX_NAME_LEN, "%s",
             userName);
    addInxVal (&genQueryInp.sqlCondInp, COL_USER_NAME, condition1);

    if (rescName != NULL && strlen(rescName)>0) {
       snprintf (condition2, MAX_NAME_LEN, "%s",
                rescName);
       addInxVal (&genQueryInp.sqlCondInp, COL_R_RESC_NAME, condition2);
    }

    genQueryInp.maxRows = MAX_SQL_ROWS;

    status = rsGenQuery (rsComm, &genQueryInp, genQueryOut);

    clearGenQueryInp(&genQueryInp);

    return (status);
}

