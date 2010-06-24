/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* databaseObjInfo.h
 */

#ifndef DATABASE_OBJ_INFO_H
#define DATABASE_OBJ_INFO_H

/* This is a metadata type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "icatDefines.h"

typedef struct {
   char *dbrName;
   int objDesc;  /* if positive, object descriptor (index) of open DBO */
   char *option; /* option to perform */
   char *optionArg; /* A value associated with the particular optin */
} databaseObjInfoInp_t;
    
#define databaseObjInfoInp_PI "str *dbrName; int objDesc; str *option; str *optionArg;"

typedef struct {
   char *outBuf;
} databaseObjInfoOut_t;

#define databaseObjInfoOut_PI "str *outBuf;"

#if defined(RODS_SERVER)
#define RS_DATABASE_OBJ_INFO rsDatabaseObjInfo
/* prototype for the server handler */
int
rsDatabaseObjInfo (rsComm_t *rsComm, 
		      databaseObjInfoInp_t *databaseObjInfoInp, 
		      databaseObjInfoOut_t **databaseObjInfoOut);
int
_rsDatabaseObjInfo (rsComm_t *rsComm,
		       databaseObjInfoInp_t *databaseObjInfoInp,
		       databaseObjInfoOut_t **databaseObjInfoOut);
#else
#define RS_DATABASE_OBJ_INFO NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* prototype for the client call */
int
rcDatabaseObjInfo (rcComm_t *conn, 
		      databaseObjInfoInp_t *databaseObjInfoInp, 
		      databaseObjInfoOut_t **databaseObjInfoOut);

#ifdef  __cplusplus
}
#endif

#endif	/* DATABASE_OBJ_INFO_H */
