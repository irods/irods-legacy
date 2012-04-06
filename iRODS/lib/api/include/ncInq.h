/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* ncInq.h
 */

#ifndef NC_INQ_H
#define NC_INQ_H

/* This is a NETCDF API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "dataObjInpOut.h"
#include "ncOpen.h"
#include "ncInqId.h"

typedef struct {
    rodsLong_t arrayLen;
    int id;
    int myint;	/* not used */
    char name[LONG_NAME_LEN];
} ncGenDimOut_t;

#define NcGenDimOut_PI "double arrayLen; int id; int myint; str name[LONG_NAME_LEN];"

typedef struct {
    int dataType;
    int id;
    int length;
    int myint;  /* not used */
    char name[LONG_NAME_LEN];
} ncGenAttOut_t;

#define NcGenAttOut_PI "int dataType; int id; int length;  int myint; str name[LONG_NAME_LEN];"

typedef struct {
    int natts;
    int dataType;
    int id;
    int myint;  /* not used */
    char name[LONG_NAME_LEN];
    ncGenAttOut_t *att;		/* array of natts length */
} ncGenVarOut_t;

#define NcGenVarOut_PI "int natts; int dataType; int id; int myint; str name[LONG_NAME_LEN]; struct *NcGenAttOut_PI(natts);"

typedef struct {
    int ndims;
    int nvars;
    int ngatts;	/* number of global gttr */
    int unlimdimid;
    int format;		/* one of NC_FORMAT_CLASSIC, NC_FORMAT_64BIT, 
                         * NC_FORMAT_NETCDF4, NC_FORMAT_NETCDF4_CLASSIC */
    int myint;  /* not used */
    ncGenDimOut_t *dim;		/* array of ndims length */
    ncGenVarOut_t *var;		/* array of ngvars length */
    ncGenAttOut_t *gatt;	/* array of ngatts length */
} ncInqOut_t;

#define NcInqOut_PI "int ndims; int nvars; int ngatts; int unlimdimid; int format; int myint; struct *NcGenDimOut_PI(ndims); struct *NcGenVarOut_PI(nvars); struct *NcGenAttOut_PI(ngatts);"

#if defined(RODS_SERVER)
#define RS_NC_INQ rsNcInq
/* prototype for the server handler */
int
rsNcInq (rsComm_t *rsComm, ncInqIdInp_t *ncInqInp, ncInqOut_t **ncInqOut);
int
_rsNcInq (rsComm_t *rsComm, int ncid, ncInqOut_t **ncInqOut);
#else
#define RS_NC_INQ NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* rcNcInq - general netcdf inq for id (equivalent to nc_inq + nc_inq_format
 * Input - 
 *   rcComm_t *conn - The client connection handle.
 *   ncInqIdInp_t struct:
 *     ncid - the the ncid.   
 * OutPut - ncInqOut_t.
 */
/* prototype for the client call */
int
rcNcInq (rcComm_t *conn, ncInqIdInp_t *ncInqInp, ncInqOut_t **ncInqOut);

int
initNcInqOut (int ndims, int nvars, int ngatts, int unlimdimid, int format,
ncInqOut_t **ncInqOut);
int
clearNcInqOut (ncInqOut_t **ncInqOut);

#ifdef  __cplusplus
}
#endif

#endif	/* NC_INQ_H */
