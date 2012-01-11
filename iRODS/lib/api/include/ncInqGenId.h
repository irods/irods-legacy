/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* ncInqGenId.h
 */

#ifndef NC_INQ_GEN_ID_H
#define NC_INQ_GEN_ID_H

/* This is a NETCDF API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "dataObjInpOut.h"
#include "ncOpen.h"

/* definition for type */
#define NC_VAR_T		0	/* nc variable */
#define NC_DIM_T		1	/* nc dimension */
typedef struct {
    int type;
    int ncid;
    char name[NAME_LEN];
    keyValPair_t condInput;     /* not used */
} ncInqGenIdInp_t;
    
#define NcInqGenIdInput "int type; int ncid; str name[NAME_LEN]; struct KeyValPair_PI;"
#if defined(RODS_SERVER)
#define RS_NC_INQ_GEN_ID rsNcInqGenId
/* prototype for the server handler */
int
rsNcInqGenId (rsComm_t *rsComm, ncInqGenIdInp_t *ncInqGenIdInp_t, int **ncid);
#else
#define RS_NC_INQ_GEN_ID NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* rcNcInqGenId - general netcdf inq for id (equivalent to nc_inq_dimid,
 *    nc_inq_varid, ....
 * Input - 
 *   rcComm_t *conn - The client connection handle.
 *   ncInqGenIdInp_t struct:
 *     type - parameter type - NC_VAR_T, NC_DIM_T, ....
 *   int the the nc id for the type.   
 * OutPut - 
 */
/* prototype for the client call */
int
rcNcInqGenId (rcComm_t *conn, ncInqGenIdInp_t *ncInqGenIdInp_t, int **ncid);

#ifdef  __cplusplus
}
#endif

#endif	/* NC_INQ_GEN_ID_H */
