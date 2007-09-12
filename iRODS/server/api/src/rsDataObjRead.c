/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See dataObjRead.h for a description of this API call.*/

#include "dataObjRead.h"
#include "rodsLog.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "bunSubRead.h"  /* XXXXX can be taken out when bundle api done */


int
rsDataObjRead (rsComm_t *rsComm, dataObjReadInp_t *dataObjReadInp, 
bytesBuf_t *dataObjReadOutBBuf)
{
    int bytesRead;

    int l1descInx = dataObjReadInp->l1descInx;

    if (l1descInx < 2 || l1descInx >= NUM_L1_DESC) {
        rodsLog (LOG_NOTICE,
          "rsDataObjRead: l1descInx %d out of range",
          l1descInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }
    bytesRead = l3Read (rsComm, l1descInx, dataObjReadInp->len,
      dataObjReadOutBBuf);

    return (bytesRead);
}

int
l3Read (rsComm_t *rsComm, int l1descInx, int len,
bytesBuf_t *dataObjReadOutBBuf)
{
    int rescTypeInx;
    int bytesRead;

    dataObjInfo_t *dataObjInfo;
    dataObjInfo = L1desc[l1descInx].dataObjInfo;

    if (getBunType (dataObjInfo->specColl) >= 0) {
	bunSubFdOprInp_t bunSubReadInp;
        memset (&bunSubReadInp, 0, sizeof (bunSubReadInp));
        bunSubReadInp.type = dataObjInfo->specColl->type;
        bunSubReadInp.fd = L1desc[l1descInx].l3descInx;
        bunSubReadInp.len = len;
        rstrcpy (bunSubReadInp.addr.hostAddr, dataObjInfo->rescInfo->rescLoc,
          NAME_LEN);
        bytesRead = rsBunSubRead (rsComm, &bunSubReadInp, dataObjReadOutBBuf);
    } else {
        fileReadInp_t fileReadInp;

        rescTypeInx = L1desc[l1descInx].dataObjInfo->rescInfo->rescTypeInx;

        switch (RescTypeDef[rescTypeInx].rescCat) {
          case FILE_CAT:
            memset (&fileReadInp, 0, sizeof (fileReadInp));
            fileReadInp.fileInx = L1desc[l1descInx].l3descInx;
            fileReadInp.len = len;
            bytesRead = rsFileRead (rsComm, &fileReadInp, dataObjReadOutBBuf);
            break;

          default:
            rodsLog (LOG_NOTICE,
              "l3Read: rescCat type %d is not recognized",
              RescTypeDef[rescTypeInx].rescCat);
            bytesRead = SYS_INVALID_RESC_TYPE;
            break;
	}
    }
    return (bytesRead);
}

int
_l3Read (rsComm_t *rsComm, int rescTypeInx, int l3descInx,
void *buf, int len)
{
    fileReadInp_t fileReadInp;
    bytesBuf_t dataObjReadInpBBuf;
    int bytesRead;

    dataObjReadInpBBuf,len = len;
    dataObjReadInpBBuf.buf = buf;

    switch (RescTypeDef[rescTypeInx].rescCat) {
      case FILE_CAT:
        memset (&fileReadInp, 0, sizeof (fileReadInp));
        fileReadInp.fileInx = l3descInx;
        fileReadInp.len = len;
        bytesRead = rsFileRead (rsComm, &fileReadInp,
          &dataObjReadInpBBuf);
        break;
      default:
        rodsLog (LOG_NOTICE,
          "_l3Read: rescCat type %d is not recognized",
          RescTypeDef[rescTypeInx].rescCat);
        bytesRead = SYS_INVALID_RESC_TYPE;
        break;
    }
    return (bytesRead);
}

