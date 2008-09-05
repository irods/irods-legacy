/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* icCollUtil.h
 */

#ifndef _IC_COLL_UTIL
#define _IC_COLL_UTIL


#include "rods.h"
#include "rcMisc.h"
#include "rodsClient.h"
#include "rsApiHandler.h"
#include "objMetaOpr.h"

/*
#include "dataObjInpOut.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
*/

/* prototype for the server handler */
int msiListColl (msParam_t* mPin1, msParam_t* mPout1, ruleExecInfo_t* rei);
int icListColl (rsComm_t* rsComm, collInp_t *rmCollInp, collOprStat_t **collOprStat);
int _icListColl (rsComm_t* rsComm, collInp_t *rmCollInp, collOprStat_t **collOprStat);
int _icListCollRecur (rsComm_t* rsComm, collInp_t *rmCollInp, collOprStat_t **collOprStat);
int _icPhyListColl (rsComm_t* rsComm, collInp_t *rmCollInp, dataObjInfo_t *dataObjInfo, collOprStat_t **collOprStat);

//int svrUnregColl (rsComm_t *rsComm, collInp_t *rmCollInp);

#endif	/* _IC_COLL_UTIL */
