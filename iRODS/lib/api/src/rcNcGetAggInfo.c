/* This is script-generated code.  */ 
/* See ncGetAggInfo.h for a description of this API call.*/

#include "ncGetAggInfo.h"

int
rcNcGetAggInfo (rcComm_t *conn, ncOpenInp_t *ncOpenInp, 
ncAggInfo_t **ncAggInfo)
{
    int status;
    status = procApiRequest (conn, NC_GET_AGG_INFO_AN, ncOpenInp, NULL, 
        (void **) ncAggInfo, NULL);

    return (status);
}

int
addNcAggElement (ncAggElement_t *ncAggElement, ncAggInfo_t *ncAggInfo)
{
    int newNumFiles, i, j;
    ncAggElement_t *newElement;

    if (ncAggInfo == NULL || ncAggElement == NULL) return USER__NULL_INPUT_ERR;

    if ((ncAggInfo->numFiles % PTR_ARRAY_MALLOC_LEN) == 0) {
        newNumFiles =  ncAggInfo->numFiles + PTR_ARRAY_MALLOC_LEN;
        newElement = (ncAggElement_t *) calloc (newNumFiles,
          sizeof (ncAggElement_t));
        if (ncAggInfo->numFiles > 0) {
            if (ncAggInfo->ncAggElement == NULL) {
                rodsLog (LOG_ERROR,
                  "addNcAggElement: numFiles > 0 but cAggElement == NULL");
                return (NETCDF_VAR_COUNT_OUT_OF_RANGE);
            }
            memcpy (newElement, ncAggInfo->ncAggElement,
              ncAggInfo->numFiles * sizeof (newElement));
            free (ncAggInfo->ncAggElement);
        }
        ncAggInfo->ncAggElement = newElement;
        ncAggInfo->ncAggElement = newElement;
    }

    if (ncAggInfo->numFiles <= 0) {
        ncAggInfo->ncAggElement[0] = *ncAggElement;
    } else {
        for (i = 0; i < ncAggInfo->numFiles; i++) {
            ncAggElement_t *myElement = &ncAggInfo->ncAggElement[i];
            if (ncAggElement->startTime < myElement->startTime ||
              (ncAggElement->startTime == myElement->startTime &&
              ncAggElement->endTime < myElement->endTime)) {
                /* insert */
                for (j = ncAggInfo->numFiles; j > i; j--) {
                    ncAggInfo->ncAggElement[j] = ncAggInfo->ncAggElement[j-1];
                }
                ncAggInfo->ncAggElement[i] = *ncAggElement;
                break;
            }
        }
        if (i >= ncAggInfo->numFiles) {
            /* not inserted yet. put it at the end */
            ncAggInfo->ncAggElement[i] = *ncAggElement;
        }
    }
    ncAggInfo->numFiles++;
    return 0;
}

