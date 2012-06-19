/* This is script-generated code.  */ 
/* See ooiGenServReq.h for a description of this API call.*/

#include "ooiGenServReq.h"

int
rcOoiGenServReq (rcComm_t *conn, ooiGenServReqInp_t *ooiGenServReqInp, 
ooiGenServReqOut_t **ooiGenServReqOut)
{
    int status;
    status = procApiRequest (conn, OOI_GEN_SERV_REQ_AN, ooiGenServReqInp, NULL, 
        (void **) ooiGenServReqOut, NULL);

    return (status);
}
