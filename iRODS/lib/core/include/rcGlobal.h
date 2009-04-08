/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* rcGlobal.h - global definition for client API */

#ifndef RC_GLOBAL_H
#define RC_GLOBAL_H

#include "rods.h"
#include "rodsPackTable.h"		/* globally declare RodsPackTable */
#include "apiPackTable.h"		/* globally declare apiPackTable */
#include "apiTable.h"   /* contains global RsApiTable */
#include "objInfo.h" 
#include "rodsGenQuery.h" 
#include "rodsGeneralUpdate.h" 

int ProcessType = CLIENT_PT;

char *dataObjCond[] = {
ALL_KW, 		/* operation done on all replica */
COPIES_KW, 		/* the number of copies */
EXEC_LOCALLY_KW, 	/* execute locally */
FORCE_FLAG_KW, 		/* force update */
CLI_IN_SVR_FIREWALL_KW, /* client behind same firewall */ 
REG_CHKSUM_KW, 		/* register checksum */
VERIFY_CHKSUM_KW, 	/* verify checksum */
OBJ_PATH_KW,		/* logical path of the object */ 
RESC_NAME_KW,		/* resource name */
DEST_RESC_NAME_KW,	/* destination resource name */
BACKUP_RESC_NAME_KW,	/* destination resource name */
DATA_TYPE_KW,		/* data type */
DATA_SIZE_KW,
CHKSUM_KW,
VERSION_KW,
FILE_PATH_KW,		/* the physical file path */
REPL_NUM_KW,		/* replica number */
REPL_STATUS_KW, 	/* status of the replica */
DATA_INCLUDED_KW,	/* data included in the input bytes buffer */
DATA_OWNER_KW,	
DATA_OWNER_ZONE_KW, 
DATA_EXPIRY_KW,
DATA_COMMENTS_KW,
DATA_CREATE_KW,
DATA_MODIFY_KW,
DATA_ACCESS_KW,
DATA_ACCESS_INX_KW,
NO_OPEN_FLAG_KW,
STREAMING_KW,
DATA_ID_KW,
COLL_ID_KW,
RESC_GROUP_NAME_KW,
STATUS_STRING_KW,
DATA_MAP_ID_KW,
"ENDOFLIST"};

char *compareOperator[]  = {
">","<","=",
"like", "between","LIKE", "BETWEEN",
"NOT LIKE", "NOT BETWEEN","NOT LIKE", "NOT BETWEEN",
"ENDOFLIST"};

char *rescCond[] = {
RESC_ZONE_KW,
RESC_NAME_KW,
RESC_LOC_KW,
RESC_TYPE_KW,
RESC_CLASS_KW,
RESC_VAULT_PATH_KW,
NUM_OPEN_PORTS_KW,
PARA_OPR_KW,
GATEWAY_ADDR_KW,
RESC_MAX_OBJ_SIZE_KW,
FREE_SPACE_KW,
FREE_SPACE_TIME_KW,
FREE_SPACE_TIMESTAMP_KW,
RESC_TYPE_INX_KW,
RESC_CLASS_INX_KW,
RESC_ID_KW,
RESC_INFO_KW,
RESC_COMMENTS_KW,
RESC_CREATE_KW,
RESC_MODIFY_KW,
"ENDOFLIST"};

char *userCond[] = {
USER_NAME_CLIENT_KW,
RODS_ZONE_CLIENT_KW,
HOST_CLIENT_KW,
USER_TYPE_CLIENT_KW,
AUTH_STR_CLIENT_KW,
USER_AUTH_SCHEME_CLIENT_KW,
USER_INFO_CLIENT_KW,
USER_COMMENT_CLIENT_KW,
USER_CREATE_CLIENT_KW,
USER_MODIFY_CLIENT_KW,
USER_NAME_PROXY_KW,
RODS_ZONE_PROXY_KW,
HOST_PROXY_KW,
USER_TYPE_PROXY_KW,
AUTH_STR_PROXY_KW,
USER_AUTH_SCHEME_PROXY_KW,
USER_INFO_PROXY_KW,
USER_COMMENT_PROXY_KW,
USER_CREATE_PROXY_KW,
USER_MODIFY_PROXY_KW,
"ENDOFLIST"};

char *collCond[] = {
COLL_NAME_KW,
COLL_PARENT_NAME_KW,
COLL_OWNER_NAME_KW,
COLL_OWNER_ZONE_KW,
COLL_MAP_ID_KW,
COLL_INHERITANCE_KW,
COLL_COMMENTS_KW,
COLL_EXPIRY_KW,
COLL_CREATE_KW,
COLL_MODIFY_KW,
COLL_ACCESS_KW,
COLL_ACCESS_INX_KW,
COLL_ID_KW,
"ENDOFLIST"};

rescTypeDef_t RescTypeDef[] = {
  {"unix",	FILE_CAT, UNIX_FILE_TYPE,  DO_CHK_PATH_PERM},
  {"hpss",	FILE_CAT, HPSS_FILE_TYPE,  DO_CHK_PATH_PERM},
  {"windows",   FILE_CAT, NT_FILE_TYPE,    DO_CHK_PATH_PERM},
  {"test stage",FILE_CAT, TEST_STAGE_FILE_TYPE, DO_CHK_PATH_PERM},
};

int NumRescTypeDef = sizeof (RescTypeDef) / sizeof (rescTypeDef_t);

rescClass_t RescClass[] = {
	{"generic",	CACHE_CL},
	{"cache",	CACHE_CL},
	{"volatile",	CACHE_CL},
	{"temporary",	CACHE_CL},
	{"permanent",	ARCHIVAL_CL},
	{"archive",	ARCHIVAL_CL},
	{"bundle",	BUNDLE_CL},
	{"compound",	COMPOUND_CL},
};

int NumRescClass = sizeof (RescClass) / sizeof (rescClass_t);

/* Note; all structFile name must contain the word structFile */ 

structFileTypeDef_t StructFileTypeDef[] = {
	{HAAW_STRUCT_FILE_STR, HAAW_STRUCT_FILE_T},
	{TAR_STRUCT_FILE_STR, TAR_STRUCT_FILE_T},
};

int NumStructFileType = sizeof (StructFileTypeDef) / sizeof (structFileTypeDef_t);

#endif	/* RC_GLOBAL_H */
