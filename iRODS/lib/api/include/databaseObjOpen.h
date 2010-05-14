/*** Copyright (c), The Regents of the University of California            ***
 *** For more openrmation please refer to files in the COPYRIGHT directory ***/
/* databaseObjOpen.h
 */

#ifndef DATABASE_OBJ_OPEN_H
#define DATABASE_OBJ_OPEN_H

/* This is a metadata type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "icatDefines.h"

typedef struct {
   char *dbrName; /* database resource name */
   char *dboName; /* database obj name */
} databaseObjOpenInp_t;
    
#define databaseObjOpenInp_PI "str *dbrName; str *dboName;"

#if defined(RODS_SERVER)
#define RS_DATABASE_OBJ_OPEN rsDatabaseObjOpen
/* prototype for the server handler */
int
rsDatabaseObjOpen (rsComm_t *rsComm, 
		      databaseObjOpenInp_t *databaseObjOpenInp);
int
_rsDatabaseObjOpen (rsComm_t *rsComm,
		       databaseObjOpenInp_t *databaseObjOpenInp);
#else
#define RS_DATABASE_OBJ_OPEN NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* prototype for the client call */
int
rcDatabaseObjOpen (rcComm_t *conn, 
		      databaseObjOpenInp_t *databaseObjOpenInp);

#ifdef  __cplusplus
}
#endif

#endif	/* DATABASE_OBJ_OPEN_H */
