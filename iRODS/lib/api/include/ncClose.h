/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* dataObjLock.h
 */

#ifndef NC_CLOSE_H
#define NC_CLOSE_H

/* This is a NETCDF API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "dataObjInpOut.h"
#include "ncOpen.h"

#if defined(RODS_SERVER)
#define RS_NC_CLOSE rsNcClose
/* prototype for the server handler */
int
rsNcClose (rsComm_t *rsComm, int *ncid);
#else
#define RS_NC_CLOSE NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* rcNcClose - netcdf close an iRODS data object (equivalent to nc_close.
 * Input - 
 *   rcComm_t *conn - The client connection handle.
 *   int the ncid of the opened object - an integer descriptor.   
 * OutPut - 
 */
/* prototype for the client call */
int
rcNcClose (rcComm_t *conn, int ncid);


#ifdef  __cplusplus
}
#endif

#endif	/* NC_CLOSE_H */
