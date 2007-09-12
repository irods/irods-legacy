/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See dataObjWrite.h for a description of this API call.*/

#include "dataObjWrite.h"
#include "rodsLog.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "bunSubRead.h"  /* XXXXX can be taken out when bundle api done */

int
rsDataObjWrite (rsComm_t *rsComm, dataObjWriteInp_t *dataObjWriteInp, 
bytesBuf_t *dataObjWriteInpBBuf)
{
    int bytesWritten;

    int l1descInx = dataObjWriteInp->l1descInx;

    if (l1descInx < 2 || l1descInx >= NUM_L1_DESC) {
	rodsLog (LOG_NOTICE,
	  "rsDataObjWrite: l1descInx %d out of range",
	  l1descInx);
	return (SYS_FILE_DESC_OUT_OF_RANGE);
    }

    bytesWritten = l3Write (rsComm, l1descInx, dataObjWriteInp->len,
     dataObjWriteInpBBuf);

    return (bytesWritten);
}

int
l3Write (rsComm_t *rsComm, int l1descInx, int len,
bytesBuf_t *dataObjWriteInpBBuf)
{
    int rescTypeInx;
    fileWriteInp_t fileWriteInp;
    int bytesWritten;

    dataObjInfo_t *dataObjInfo;
    dataObjInfo = L1desc[l1descInx].dataObjInfo;

    if (getBunType (dataObjInfo->specColl) >= 0) {
        bunSubFdOprInp_t bunSubWriteInp;
        memset (&bunSubWriteInp, 0, sizeof (bunSubWriteInp));
        bunSubWriteInp.type = dataObjInfo->specColl->type;
        bunSubWriteInp.fd = L1desc[l1descInx].l3descInx;
        bunSubWriteInp.len = len;
        rstrcpy (bunSubWriteInp.addr.hostAddr, dataObjInfo->rescInfo->rescLoc,
          NAME_LEN);
        bytesWritten = rsBunSubWrite (rsComm, &bunSubWriteInp, 
	  dataObjWriteInpBBuf);
    } else {
        rescTypeInx = L1desc[l1descInx].dataObjInfo->rescInfo->rescTypeInx;

        switch (RescTypeDef[rescTypeInx].rescCat) {
          case FILE_CAT:
	    memset (&fileWriteInp, 0, sizeof (fileWriteInp));
	    fileWriteInp.fileInx = L1desc[l1descInx].l3descInx;
	    fileWriteInp.len = len;
	    bytesWritten = rsFileWrite (rsComm, &fileWriteInp, 
	      dataObjWriteInpBBuf);
	    if (bytesWritten > 0) {
	        L1desc[l1descInx].bytesWritten+=bytesWritten;
	    }
	    break;

          default:
            rodsLog (LOG_NOTICE,
              "l3Write: rescCat type %d is not recognized",
              RescTypeDef[rescTypeInx].rescCat);
            bytesWritten = SYS_INVALID_RESC_TYPE;
            break;
	}
    }
    return (bytesWritten);
}

int
_l3Write (rsComm_t *rsComm, int rescTypeInx, int l3descInx, 
void *buf, int len)
{
    fileWriteInp_t fileWriteInp;
    bytesBuf_t dataObjWriteInpBBuf;
    int bytesWritten;

    dataObjWriteInpBBuf,len = len;
    dataObjWriteInpBBuf.buf = buf;

    switch (RescTypeDef[rescTypeInx].rescCat) {
      case FILE_CAT:
	memset (&fileWriteInp, 0, sizeof (fileWriteInp));
	fileWriteInp.fileInx = l3descInx;
	fileWriteInp.len = len;
	bytesWritten = rsFileWrite (rsComm, &fileWriteInp, 
	  &dataObjWriteInpBBuf);
	break;
      default:
        rodsLog (LOG_NOTICE,
          "_l3Write: rescCat type %d is not recognized",
          RescTypeDef[rescTypeInx].rescCat);
        bytesWritten = SYS_INVALID_RESC_TYPE;
        break;
    }
    return (bytesWritten);
}

