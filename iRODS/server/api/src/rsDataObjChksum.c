#include "reGlobalsExtern.h"
#include "dataObjChksum.h"
#include "objMetaOpr.h"
#include "dataObjOpr.h"
#include "rsApiHandler.h"
#include "modDataObjMeta.h"

int
rsDataObjChksum (rsComm_t *rsComm, dataObjInp_t *dataObjChksumInp,
char **outChksum)
{
    int status;
    dataObjInfo_t *dataObjInfoHead;

    status = _rsDataObjChksum (rsComm, dataObjChksumInp, outChksum,
      &dataObjInfoHead);

    freeAllDataObjInfo (dataObjInfoHead);
    return (status);
}

int
_rsDataObjChksum (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
char **outChksumStr, dataObjInfo_t **dataObjInfoHead)
{
    int status;
    dataObjInfo_t *tmpDataObjInfo;
    int allFlag;
    int verifyFlag; 
    int forceFlag;

    if (getValByKey (&dataObjInp->condInput, CHKSUM_ALL_KW) != NULL) {
	allFlag = 1;
    } else {
	allFlag = 0;
    }

    if (getValByKey (&dataObjInp->condInput, VERIFY_CHKSUM_KW) != NULL) {
        verifyFlag = 1;
    } else {
        verifyFlag = 0;
    }

    if (getValByKey (&dataObjInp->condInput, FORCE_CHKSUM_KW) != NULL) {
        forceFlag = 1;
    } else {
        forceFlag = 0;
    }

    *dataObjInfoHead = NULL;
    *outChksumStr = NULL;
#if 0
    status = getDataObjInfo (rsComm, dataObjInp, dataObjInfoHead,
      ACCESS_READ_OBJECT, 0);
#endif
    status = getDataObjInfoIncSpecColl (rsComm, dataObjInp, dataObjInfoHead);

    if (status < 0) {
        return status;
    } else if (allFlag == 0) {
        /* screen out any stale copies */
        sortObjInfoForOpen (dataObjInfoHead, &dataObjInp->condInput, 0);
        tmpDataObjInfo = *dataObjInfoHead;
        if (tmpDataObjInfo->next == NULL) {
            /* the only copy */
            if (strlen (tmpDataObjInfo->chksum) > 0) {
		if (verifyFlag == 0 && forceFlag == 0) { 
                    *outChksumStr = strdup (tmpDataObjInfo->chksum);
                    return (0);
		}
            }
        } else {
            while (tmpDataObjInfo != NULL) {
                if (tmpDataObjInfo->replStatus > 0 &&
                  strlen (tmpDataObjInfo->chksum) > 0) {
		    if (verifyFlag == 0 && forceFlag == 0) {
                        *outChksumStr = strdup (tmpDataObjInfo->chksum);
                        return (0);
		    } else {
			break;
		    }
                }
                tmpDataObjInfo = tmpDataObjInfo->next;
            }
        }
	/* need to compute the chksum */
	if (tmpDataObjInfo == NULL) {
	    tmpDataObjInfo = *dataObjInfoHead;
	}
	if (verifyFlag > 0 && strlen (tmpDataObjInfo->chksum) > 0) {
	    status = verifyDatObjChksum (rsComm, tmpDataObjInfo, 
	      outChksumStr);
	} else {
	    status = dataObjChksumAndRegInfo (rsComm, tmpDataObjInfo,
              outChksumStr);
	}
	return (status);
    }

    /* allFlag == 1 */
    tmpDataObjInfo = *dataObjInfoHead;
    while (tmpDataObjInfo != NULL) {
	char *tmpChksumStr;

	if (strlen (tmpDataObjInfo->chksum) == 0) {
	    /* need to chksum no matter what */ 
	    status = dataObjChksumAndRegInfo (rsComm, tmpDataObjInfo,
              &tmpChksumStr);
        } else if (verifyFlag > 0) {
            status = verifyDatObjChksum (rsComm, tmpDataObjInfo,
              &tmpChksumStr);
        } else if (forceFlag > 0) {
            status = dataObjChksumAndRegInfo (rsComm, tmpDataObjInfo,
              &tmpChksumStr);
        } else {
	    tmpChksumStr = strdup (tmpDataObjInfo->chksum);
	    status = 0;
        }

	if (status < 0) {
	    return (status);
	}
	if (tmpDataObjInfo->replStatus > 0 && *outChksumStr == NULL) {
	    *outChksumStr = tmpChksumStr;
	} else {
	    /* save it */
	    if (strlen (tmpDataObjInfo->chksum) == 0) {
	        rstrcpy (tmpDataObjInfo->chksum, tmpChksumStr, NAME_LEN);
	    }	
	    free (tmpChksumStr);
	}
	tmpDataObjInfo = tmpDataObjInfo->next;
    }
    if (*outChksumStr == NULL) {
	*outChksumStr = strdup ((*dataObjInfoHead)->chksum);
    }

    return (status);
}

int
dataObjChksumAndRegInfo (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo,
char **outChksumStr)
{
    modDataObjMeta_t modDataObjMetaInp;
     keyValPair_t regParam;
    int status;

    status = _dataObjChksum (rsComm, dataObjInfo, outChksumStr);

    if (status < 0) {
        rodsLog (LOG_ERROR,
          "dataObjChksumAndRegInfo: _dataObjChksum error for %s, status = %d",
          dataObjInfo->objPath, status);
    }

    if (dataObjInfo->specColl != NULL) {
	return (status);
    }
    memset (&regParam, 0, sizeof (regParam));
    addKeyVal (&regParam, CHKSUM_KW, *outChksumStr);
    modDataObjMetaInp.dataObjInfo = dataObjInfo;
    modDataObjMetaInp.regParam = &regParam;
    status = rsModDataObjMeta (rsComm, &modDataObjMetaInp);
    clearKeyVal (&regParam);

    return (status);
}

int
verifyDatObjChksum (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo, 
char **outChksumStr)
{
    int status;

    status = _dataObjChksum (rsComm, dataObjInfo, outChksumStr);
    if (status < 0) {
        rodsLog (LOG_ERROR,
           "verifyDatObjChksum:_dataObjChksum error for %s, stat=%d",
          dataObjInfo->objPath, status);
        return (status);
    }

    if (strcmp (*outChksumStr, dataObjInfo->chksum) != 0) {
        rodsLog (LOG_ERROR,
          "verifyDatObjChksum: computed chksum %s != icat value %s for %s",
          *outChksumStr, dataObjInfo->chksum, dataObjInfo->objPath);
	return (USER_CHKSUM_MISMATCH);
    } else {
	return status;
    }
}


