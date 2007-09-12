/* rsOprComplete.c */
#include "oprComplete.h"
#include "dataObjClose.h"
#include "rsGlobalExtern.h"

int rsOprComplete (rsComm_t *rsComm, int *retval)
{
    dataObjCloseInp_t dataObjCloseInp;
    int status;

    if (*retval >= 2) {
        memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
        dataObjCloseInp.l1descInx = *retval;
	if (L1desc[*retval].oprType == PUT_OPR) {
	    dataObjCloseInp.bytesWritten = L1desc[*retval].dataSize;
	}
        *retval = rsDataObjClose (rsComm, &dataObjCloseInp);
    }

    if (*retval >= 0) {
	return (SYS_HANDLER_DONE_NO_ERROR);
    } else {
        return (*retval);
    }
}

