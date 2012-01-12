/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* ncInqId.h
 */

#ifndef NC_INQ_ID_H
#define NC_INQ_ID_H

/* This is a NETCDF API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "dataObjInpOut.h"
#include "ncOpen.h"

/* definition for paramType */
#define NC_VAR_T		0	/* nc variable */
#define NC_DIM_T		1	/* nc dimension */
typedef struct {
    int paramType;
    int ncid;
    char name[NAME_LEN];
    keyValPair_t condInput;
} ncInqIdInp_t;
    
#define NcInqIdInp_PI "int paramType; int ncid; str name[NAME_LEN]; struct KeyValPair_PI;"
#if defined(RODS_SERVER)
#define RS_NC_INQ_ID rsNcInqId
/* prototype for the server handler */
int
rsNcInqId (rsComm_t *rsComm, ncInqIdInp_t *ncInqIdInp, int **outId);
int
_rsNcInqId (int type, int ncid, char *name, int *outId);
#else
#define RS_NC_INQ_ID NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* rcNcInqId - general netcdf inq for id (equivalent to nc_inq_dimid,
 *    nc_inq_varid, ....
 * Input - 
 *   rcComm_t *conn - The client connection handle.
 *   ncInqIdInp_t struct:
 *     type - parameter type - NC_VAR_T, NC_DIM_T, ....
 *   int the the nc id for the type.   
 * OutPut - 
 */
/* prototype for the client call */
int
rcNcInqId (rcComm_t *conn, ncInqIdInp_t *ncInqIdInp, int *outId);

#ifdef  __cplusplus
}
#endif

#endif	/* NC_INQ_ID_H */
