/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* databaseObjectAdmin.h

   This client/server call is to manage the database-object (DBO) 
   objects, which are the SQL definitions that can be applied to DBOs.
   Operations are to add, remove, update, and get.
 */

#ifndef DATABASE_OBJECT_ADMIN_H
#define DATABASE_OBJECT_ADMIN_H

/* This is a metadata type API call */

#include "rods.h"
#include "rcMisc.h"
#include "procApiRequest.h"
#include "apiNumber.h"
#include "initServer.h"
#include "icatDefines.h"

/* Options: */
#define DBObjAdmin_Add    "add"
#define DBObjAdmin_Remove "rm"
#define DBObjAdmin_Update "update"
#define DBObjAdmin_Get    "get"

typedef struct {
   char *option;
   char *dbrName;
   char *dboName;
   rodsLong_t sql_id;
   char *sql;
   char *defaultParameters;
   char *description;
} databaseObjectAdminInp_t;
    
#define databaseObjectAdminInp_PI "str *option; str *dbrName; str *dboName; double sqlId; str *sql; str *defaultParameters; str *description;"

typedef struct {
   char *dbrName;
   char *dboName;
   rodsLong_t sqlId;
   char *sql;
   char *defaultParameters;
   char *description;
} databaseObjectAdminOut_t;

#define databaseObjectAdminOut_PI "str *dbrName; str *dboName; double sqlId; str *sql; str *defaultParameters; str *description;"

#if defined(RODS_SERVER)
#define RS_DATABASE_OBJECT_ADMIN rsDatabaseObjectAdmin
/* prototype for the server handler */
int
rsDatabaseObjectAdmin (rsComm_t *rsComm, 
		      databaseObjectAdminInp_t *databaseObjectAdminInp, 
		      databaseObjectAdminOut_t **databaseObjectAdminOut);
int
_rsDatabaseObjectAdmin (rsComm_t *rsComm,
		       databaseObjectAdminInp_t *databaseObjectAdminInp,
		       databaseObjectAdminOut_t **databaseObjectAdminOut);
#else
#define RS_DATABASE_OBJECT_ADMIN NULL
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* prototype for the client call */
int
rcDatabaseObjectAdmin (rcComm_t *conn, 
		      databaseObjectAdminInp_t *databaseObjectAdminInp, 
		      databaseObjectAdminOut_t **databaseObjectAdminOut);

#ifdef  __cplusplus
}
#endif

#endif	/* DATABASE_OBJECT_ADMIN_H */
