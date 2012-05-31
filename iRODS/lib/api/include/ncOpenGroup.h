/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* ncOpen.h
 */

#ifndef NC_OPEN_GROUP_H
#define NC_OPEN_GROUP_H

/* This is a NETCDF API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "dataObjInpOut.h"
#include "ncOpen.h"
#ifdef NETCDF_API
#include "netcdf.h"
#endif

#define NcOpenGroupInp_PI "str objPath[MAX_NAME_LEN]; int mode; int rootNcid; double intialsz; double bufrsizehint; struct KeyValPair_PI;"

#if defined(RODS_SERVER)
#define RS_NC_OPEN_GROUP rsNcOpenGroup
/* prototype for the server handler */
int
rsNcOpenGroup (rsComm_t *rsComm, ncOpenInp_t *ncOpenGroupInp, int **ncid);
#else
#define RS_NC_OPEN_GROUP NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* rcNcOpenGroup - netcdf open an iRODS data object (equivalent to nc_open.
 * Input - 
 *   rcComm_t *conn - The client connection handle.
 *   ncOpenInp_t *ncOpenGroupInp - generic nc open/create input. Relevant items are:
 *	objPath - the path of the data object.
 *      mode - the mode of the open - valid values are given in netcdf.h -
 *       NC_NOWRITE (0), NC_WRITE (1)
 *	condInput - condition input (not used).
 * OutPut - 
 *   int the ncid of the opened object - an integer descriptor.   
 */

/* prototype for the client call */
int
rcNcOpenGroup (rcComm_t *conn, ncOpenInp_t *ncOpenGroupInp, int *ncid);
int
_rcNcOpenGroup (rcComm_t *conn, ncOpenInp_t *ncOpenGroupInp, int **ncid);

#ifdef  __cplusplus
}
#endif

#endif	/* NC_OPEN_GROUP_H */
