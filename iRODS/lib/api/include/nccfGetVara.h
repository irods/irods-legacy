/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* nccfGetVara.h
 */

#ifndef NCCF_GET_VARA_H
#define NCCF_GET_VARA_H

/* This is a NETCDF API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "dataObjInpOut.h"
#ifdef NETCDF_API
#include "netcdf.h"
#ifdef LIB_CF
#include "libcf.h"
#endif
#endif

typedef struct {
    int ncid;
    int varid;	
    int lvlIndex;
    int timestep;
    float latRange[2];
    float lonRange[2];
    keyValPair_t condInput;
} nccfGetVarInp_t;
    
#define NccfGetVarInp_PI "int ncid; int varid; int lvlIndex; int timestep; int latRange[2]; int lonRange[2]; struct KeyValPair_PI;"

typedef struct {
    int nlat;
    int nlon;
    char dataType_PI[NAME_LEN];	  /* the packing instruction of the dataType */
    dataArray_t *dataArray;
} nccfGetVarOut_t;

#define NccfGetVarOut_PI "int nlat; int nlon; piStr dataType_PI[NAME_LEN]; ?dataType_PI *dataArray;"

#if defined(RODS_SERVER)
#define RS_NCCF_GET_VARA rsNccfGetVara
/* prototype for the server handler */
int
rsNccfGetVara (rsComm_t *rsComm,  nccfGetVarInp_t * nccfGetVarInp, 
nccfGetVarOut_t ** nccfGetVarOut);
int
_rsNccfGetVara (int ncid,  nccfGetVarInp_t * nccfGetVarInp, 
nccfGetVarOut_t ** nccfGetVarOut);
#else
#define RS_NCCF_GET_VARA NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* rcNccfGetVara - libcf subsetting function nccf_get_vara implementation
 *    ....
 * Input - 
 *   rcComm_t *conn - The client connection handle.
 *    nccfGetVarInp_t struct:
 *   int ncid;
 *   int varid;
 *   int lvlIndex - A zero-based index number for the verticle level of 
 *     interest (Ignored if data has no vertical axis).
 *   int timestep - A zero-based index number for the timestep of interest  
 *     (Ignored if data has no time axis).
 *   float latRange[2] - holds the latitude start and stop values for the 
 *     range of interest.
 *   float lonRange[2] - holds the longitude start and stop values for the 
 *     range of interest (Wrapping around the dateline is not allowed!).
 * OutPut -  nccfGetVarOut_t
 *   int nlat;		 the number of latitude values which fall within the 
 *                        range.
 *   int nlon;		 the number of longitude values which fall within the 
 *                        range.
 *   char dataType_PI;   the packing instruction of the dataType 
 *   int type;           the type of data
 *   int len;            array length of the data
 *   void *data;         data array of len length
 */
/* prototype for the client call */
int
rcNccfGetVara (rcComm_t *conn,   nccfGetVarInp_t *nccfGetVarInp, 
 nccfGetVarOut_t ** nccfGetVarOut);
#ifdef  __cplusplus
}
#endif

#endif	/* NCCF_GET_VARA_H */
