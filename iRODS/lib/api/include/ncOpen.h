/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* dataObjLock.h
 */

#ifndef NC_OPEN_H
#define NC_OPEN_H

/* This is a NETCDF API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "dataObjInpOut.h"
#ifdef NETCDF_API
#include "netcdf.h"
#endif

typedef struct {
    char objPath[MAX_NAME_LEN];
    int mode;
    int flags;		/* not used */
    rodsULong_t	intialsz;	/* used for nc__open, nc__create */
    rodsULong_t bufrsizehint;	/* used for nc__open, nc__create */
    keyValPair_t condInput;	/* not used */
} ncOpenInp_t;

#define NcOpenInp_PI "str objPath[MAX_NAME_LEN]; int mode; int flags; double intialsz; double bufrsizehint; struct *SpecColl_PI; struct KeyValPair_PI;"

#if defined(RODS_SERVER)
#define RS_NC_OPEN rsNcOpen
/* prototype for the server handler */
int
rsNcOpen (rsComm_t *rsComm, ncOpenInp_t *ncOpenInp, int **ncid);
#else
#define RS_NC_OPEN NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* prototype for the client call */
/* rcNcOpen - netcdf open an iRODS data object (equivalent to nc_open.
 * Input - 
 *   rcComm_t *conn - The client connection handle.
 *   ncOpenInp_t *ncOpenInp - generic nc open/create input. Relevant items are:
 *	objPath - the path of the data object.
 *      mode - the mode of the open - valid values are given in netcdf.h -
 *       NC_NOWRITE, NC_WRITE
 *	condInput - condition input (not used).
 * OutPut - 
 *   int the ncid of the opened object - an integer descriptor.   
 */

int
rcNcOpen (rcComm_t *conn, ncOpenInp_t *ncOpenInp, int *ncid);
int
_rcNcOpen (rcComm_t *conn, ncOpenInp_t *ncOpenInp, int **ncid);

#ifdef  __cplusplus
}
#endif

#endif	/* NC_OPEN_H */
