/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* ncInqId.h
 */

#ifndef NC_GET_VARS_BY_TYPE_H
#define NC_GET_VARS_BY_TYPE_H

/* This is a NETCDF API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "dataObjInpOut.h"
#include "ncInqId.h"

/* valid dataType are:
NC_BYTE         signed 1 byte integer 
NC_CHAR         ISO/ASCII character   - uchar, text
NC_SHORT        signed 2 byte integer
NC_INT          signed 4 byte integer 
NC_FLOAT        single precision floating point number 
NC_DOUBLE       double precision floating point number
NC_UBYTE        unsigned 1 byte int
NC_USHORT       unsigned 2-byte int
NC_UINT         unsigned 4-byte int
NC_INT64        signed 8-byte int
NC_UINT64       unsigned 8-byte int
NC_STRING       string - only for netcdf librarysupports HDF
*/

typedef struct {
    int dataType;
    int ncid;
    int varid;	
    int ndim;		/* no. of dimension */
    rodsLong_t *start;  /* array of ndim length */
    rodsLong_t *count;  /* array of ndim length */
    rodsLong_t *stride; /* array of ndim length */
    keyValPair_t condInput;
} ncGetVarInp_t;
    
#define NcGetVarInp_PI "int varType; int ncid; int myint; int ndim; double *start(ndim); double *count(ndim); double *stride(ndim); struct KeyValPair_PI;"

typedef struct {
    char dataType_PI[NAME_LEN];	  /* the packing instruction of the dataType */
    bytesBuf_t *dataArray;
} ncGetVarOut_t;

#define NcGetVarOut_PI "piStr dataType_PI[NAME_LEN]; ?dataType_PI *dataArray;"

#if defined(RODS_SERVER)
#define RS_NC_GET_VARS_BY_TYPE rsNcGetVarsByType
/* prototype for the server handler */
int
rsNcGetVarsByType (rsComm_t *rsComm, ncGetVarInp_t *ncGetVarInp, 
ncGetVarOut_t **ncGetVarOut);
int
_rsNcGetVarsByType (int ncid, ncGetVarInp_t *ncGetVarInp, 
ncGetVarOut_t **ncGetVarOut);
#else
#define RS_NC_GET_VARS_BY_TYPE NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* rcNcGetVarsByType - general netcdf subsetting function nc_get_vara_type
 *    ....
 * Input - 
 *   rcComm_t *conn - The client connection handle.
 *   ncGetVarInp_t struct:
 *    int dataType;
 *   int ncid;
 *   int varid;
 *   int ndim;           no. of dimension 
 *   rodsLong_t *start;  array of ndim length
 *   rodsLong_t *count;  array of ndim length
 *   rodsLong_t *stride; array of ndim length 
 * OutPut - ncGetVarOut_t
 *   int varid;
 *   int dataLen;        array length of the data
 *   char dataType_PI;   the packing instruction of the dataType 
 *   void *data;         data array of dataLen length
 */
/* prototype for the client call */
int
rcNcGetVarsByType (rcComm_t *conn,  ncGetVarInp_t *ncGetVarInp, 
ncGetVarOut_t **ncGetVarOut);
int
freeNcGetVarOut (ncGetVarOut_t **ncGetVarOut);
#ifdef  __cplusplus
}
#endif

#endif	/* NC_GET_VARS_BY_TYPE_H */
