/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* rodsErrorTable.h - common header file for rods server and agents
 */

/* error code format:
 *
 *      -mmmmnnn
 *
 * where -mmmm000 is an error code defined in the rodsErrorTable.h to define
 * an error event. e.g.,
 *
 * #define SYS_SOCK_OPEN_ERR               -1000
 *
 * which define an error when a socket open call failed. Here mmmm = 1
 *
 * nnn is the errno associated with the socket open call. So, if the errno
 * is 34, then the error returned to the user is -1034. We use 3 figures
 * for nnn because the errno is less than 1000.
 */


#ifndef RODS_ERROR_TABLE_H
#define RODS_ERROR_TABLE_H

/* 1,000 - 299,000 - system type */
#define SYS_SOCK_OPEN_ERR		-1000
#define SYS_SOCK_BIND_ERR		-2000
#define SYS_SOCK_ACCEPT_ERR		-3000
#define SYS_HEADER_READ_LEN_ERR		-4000
#define SYS_HEADER_WRITE_LEN_ERR	-5000
#define SYS_HEADER_TPYE_LEN_ERR		-6000
#define SYS_CAUGHT_SIGNAL		-7000
#define SYS_GETSTARTUP_PACK_ERR		-8000
#define SYS_EXCEED_CONNECT_CNT		-9000
#define SYS_USER_NOT_ALLOWED_TO_CONN	-10000
#define SYS_READ_MSG_BODY_INPUT_ERR	-11000
#define SYS_UNMATCHED_API_NUM		-12000
#define SYS_NO_API_PRIV			-13000
#define SYS_API_INPUT_ERR		-14000
#define SYS_PACK_INSTRUCT_FORMAT_ERR	-15000
#define SYS_MALLOC_ERR			-16000
#define SYS_GET_HOSTNAME_ERR		-17000
#define SYS_OUT_OF_FILE_DESC		-18000
#define SYS_FILE_DESC_OUT_OF_RANGE	-19000
#define SYS_UNRECOGNIZED_REMOTE_FLAG	-20000
#define SYS_INVALID_SERVER_HOST		-21000
#define SYS_SVR_TO_SVR_CONNECT_FAILED	-22000
#define SYS_BAD_FILE_DESCRIPTOR		-23000
#define SYS_INTERNAL_NULL_INPUT_ERR	-24000
#define SYS_CONFIG_FILE_ERR		-25000
#define SYS_INVALID_ZONE_NAME		-26000
#define SYS_COPY_LEN_ERR		-27000
#define SYS_PORT_COOKIE_ERR		-28000
#define SYS_KEY_VAL_TABLE_ERR		-29000
#define SYS_INVALID_RESC_TYPE		-30000
#define SYS_INVALID_FILE_PATH		-31000
#define SYS_INVALID_RESC_INPUT		-32000
#define SYS_INVALID_PORTAL_OPR		-33000
#define SYS_PARA_OPR_NO_SUPPORT		-34000
#define SYS_INVALID_OPR_TYPE            -35000
#define SYS_NO_PATH_PERMISSION          -36000
#define SYS_NO_ICAT_SERVER_ERR          -37000
#define SYS_AGENT_INIT_ERR              -38000
#define SYS_PROXYUSER_NO_PRIV           -39000
#define SYS_NO_DATA_OBJ_PERMISSION      -40000
#define SYS_DELETE_DISALLOWED      	-41000
#define SYS_OPEN_REI_FILE_ERR           -42000
#define SYS_NO_RCAT_SERVER_ERR          -43000
#define SYS_UNMATCH_PACK_INSTRUCTI_NAME -44000
#define SYS_SVR_TO_CLI_MSI_NO_EXIST 	-45000
#define SYS_COPY_ALREADY_IN_RESC 	-46000
#define SYS_RECONN_OPR_MISMATCH 	-47000
#define SYS_INPUT_PERM_OUT_OF_RANGE 	-48000
#define SYS_FORK_ERROR 			-49000
#define SYS_PIPE_ERROR 			-50000
#define SYS_EXEC_CMD_STATUS_SZ_ERROR	-51000
#define SYS_PATH_IS_NOT_A_FILE		-52000
#define SYS_UNMATCHED_SPEC_COLL_TYPE	-53000
#define SYS_TOO_MANY_QUERY_RESULT	-54000
#define SYS_SPEC_COLL_NOT_IN_CACHE	-55000
#define SYS_SPEC_COLL_OBJ_NOT_EXIST	-56000
#define SYS_REG_OBJ_IN_SPEC_COLL	-57000
#define SYS_DEST_SPEC_COLL_SUB_EXIST	-58000
#define SYS_SRC_DEST_SPEC_COLL_CONFLICT -59000
#define SYS_UNKNOWN_SPEC_COLL_CLASS 	-60000
#define SYS_DUPLICATE_XMSG_TICKET 	-61000
#define SYS_UNMATCHED_XMSG_TICKET 	-62000
#define SYS_NO_XMSG_FOR_MSG_NUMBER 	-63000 
#define SYS_COLLINFO_2_FORMAT_ERR 	-64000 
#define SYS_CACHE_STRUCT_FILE_RESC_ERR 	-65000 
#define SYS_NOT_SUPPORTED		-66000
#define SYS_TAR_STRUCT_FILE_EXTRACT_ERR	-67000
#define SYS_STRUCT_FILE_DESC_ERR	-68000
#define SYS_TAR_OPEN_ERR		-69000
#define SYS_TAR_EXTRACT_ALL_ERR		-70000
#define SYS_TAR_CLOSE_ERR		-71000
#define SYS_STRUCT_FILE_PATH_ERR	-72000
#define SYS_MOUNT_MOUNTED_COLL_ERR	-73000
#define SYS_COLL_NOT_MOUNTED_ERR	-74000
#define SYS_STRUCT_FILE_BUSY_ERR	-75000
#define SYS_STRUCT_FILE_INMOUNTED_COLL	-76000
#define SYS_COPY_NOT_EXIST_IN_RESC	-77000
#define SYS_RESC_DOES_NOT_EXIST 	-78000
#define SYS_COLLECTION_NOT_EMPTY 	-79000
#define SYS_OBJ_TYPE_NOT_STRUCT_FILE 	-80000
#define SYS_WRONG_RESC_POLICY_FOR_BUN_OPR -81000
#define SYS_DIR_IN_VAULT_NOT_EMPTY 	-82000
#define SYS_OPR_FLAG_NOT_SUPPORT 	-83000
#define SYS_TAR_APPEND_ERR 		-84000
#define SYS_INVALID_PROTOCOL_TYPE 	-85000
#define SYS_UDP_CONNECT_ERR 		-86000
#define SYS_UDP_TRANSFER_ERR 		-89000
#define SYS_UDP_NO_SUPPORT_ERR 		-90000
#define SYS_READ_MSG_BODY_LEN_ERR	-91000
#define CROSS_ZONE_SOCK_CONNECT_ERR	-92000
#define SYS_NO_FREE_RE_THREAD		-93000
#define SYS_BAD_RE_THREAD_INX		-94000
#define SYS_CANT_CREATE_IN_STAGE_RESC	-95000
#define SYS_SRC_DEST_RESC_STAGING_TYPE	-96000
#define SYS_CACHE_RESC_NOT_ON_SAME_HOST	-97000

/* 300,000 - 499,000 - user input type error */
#define USER_AUTH_SCHEME_ERR		-300000
#define USER_AUTH_STRING_EMPTY		-301000
#define USER_RODS_HOST_EMPTY		-302000
#define USER_RODS_HOSTNAME_ERR		-303000
#define USER_SOCK_OPEN_ERR		-304000
#define USER_SOCK_CONNECT_ERR		-305000
#define USER_STRLEN_TOOLONG		-306000
#define USER_API_INPUT_ERR		-307000
#define USER_PACKSTRUCT_INPUT_ERR	-308000
#define USER_NO_SUPPORT_ERR		-309000
#define USER_FILE_DOES_NOT_EXIST	-310000
#define USER_FILE_TOO_LARGE		-311000
#define OVERWITE_WITHOUT_FORCE_FLAG	-312000
#define UNMATCHED_KEY_OR_INDEX		-313000
#define USER_CHKSUM_MISMATCH		-314000
#define USER_BAD_KEYWORD_ERR		-315000
#define USER__NULL_INPUT_ERR		-316000
#define USER_INPUT_PATH_ERR		-317000
#define USER_INPUT_OPTION_ERR		-318000
#define USER_INVALID_USERNAME_FORMAT	-319000
#define USER_DIRECT_RESC_INPUT_ERR	-320000
#define USER_NO_RESC_INPUT_ERR		-321000
#define USER_PARAM_LABEL_ERR		-322000
#define USER_PARAM_TYPE_ERR		-323000
#define BASE64_BUFFER_OVERFLOW		-324000
#define BASE64_INVALID_PACKET		-325000
#define USER_MSG_TYPE_NO_SUPPORT	-326000
#define USER_RSYNC_NO_MODE_INPUT_ERR	-337000
#define USER_OPTION_INPUT_ERR		-338000
#define SAME_SRC_DEST_PATHS_ERR		-339000
#define USER_RESTART_FILE_INPUT_ERR	-340000
#define RESTART_OPR_FAILED		-341000
#define BAD_EXEC_CMD_PATH		-342000
#define EXEC_CMD_OUTPUT_TOO_LARGE	-343000
#define EXEC_CMD_ERROR			-344000
#define BAD_INPUT_DESC_INDEX		-345000
#define USER_PATH_EXCEEDS_MAX		-346000
#define USER_SOCK_CONNECT_TIMEDOUT	-347000
#define USER_API_VERSION_MISMATCH 	-348000
#define USER_INPUT_FORMAT_ERR		-349000

/* 500,000 to 800,000 - file driver error */
#define FILE_INDEX_LOOKUP_ERR		-500000 
#define UNIX_FILE_OPEN_ERR		-510000 
#define UNIX_FILE_CREATE_ERR		-511000 
#define UNIX_FILE_READ_ERR		-512000 
#define UNIX_FILE_WRITE_ERR		-513000 
#define UNIX_FILE_CLOSE_ERR		-514000 
#define UNIX_FILE_UNLINK_ERR		-515000 
#define UNIX_FILE_STAT_ERR		-516000 
#define UNIX_FILE_FSTAT_ERR		-517000 
#define UNIX_FILE_LSEEK_ERR		-518000 
#define UNIX_FILE_FSYNC_ERR		-519000 
#define UNIX_FILE_MKDIR_ERR		-520000 
#define UNIX_FILE_RMDIR_ERR		-521000 
#define UNIX_FILE_OPENDIR_ERR		-522000 
#define UNIX_FILE_CLOSEDIR_ERR		-523000 
#define UNIX_FILE_READDIR_ERR		-524000 
#define UNIX_FILE_STAGE_ERR		-525000 
#define UNIX_FILE_GET_FS_FREESPACE_ERR	-526000 
#define UNIX_FILE_CHMOD_ERR		-527000 
#define UNIX_FILE_RENAME_ERR		-528000 
#define UNIX_FILE_TRUNCATE_ERR		-529000 

/* 800,000 to 880,000 - Catalog library errors  */
#define CATALOG_NOT_CONNECTED           -801000
#define CAT_ENV_ERR                     -802000
#define CAT_CONNECT_ERR                 -803000
#define CAT_DISCONNECT_ERR              -804000
#define CAT_CLOSE_ENV_ERR               -805000
#define CAT_SQL_ERR                     -806000
#define CAT_GET_ROW_ERR                 -807000
#define CAT_NO_ROWS_FOUND               -808000
#define CATALOG_ALREADY_HAS_ITEM_BY_THAT_NAME -809000
#define CAT_INVALID_RESOURCE_TYPE       -810000
#define CAT_INVALID_RESOURCE_CLASS      -811000
#define CAT_INVALID_RESOURCE_NET_ADDR   -812000
#define CAT_INVALID_RESOURCE_VAULT_PATH -813000
#define CAT_UNKNOWN_COLLECTION          -814000
#define CAT_INVALID_DATA_TYPE           -815000
#define CAT_INVALID_ARGUMENT            -816000
#define CAT_UNKNOWN_FILE                -817000
#define CAT_NO_ACCESS_PERMISSION        -818000
#define CAT_SUCCESS_BUT_WITH_NO_INFO    -819000
#define CAT_INVALID_USER_TYPE           -820000
#define CAT_COLLECTION_NOT_EMPTY        -821000
#define CAT_TOO_MANY_TABLES             -822000
#define CAT_UNKNOWN_TABLE               -823000
#define CAT_NOT_OPEN                    -824000
#define CAT_FAILED_TO_LINK_TABLES       -825000
#define CAT_INVALID_AUTHENTICATION      -826000
#define CAT_INVALID_USER                -827000
#define CAT_INVALID_ZONE                -828000
#define CAT_INVALID_GROUP               -829000
#define CAT_INSUFFICIENT_PRIVILEGE_LEVEL -830000
#define CAT_INVALID_RESOURCE             -831000
#define CAT_INVALID_CLIENT_USER          -832000
#define CAT_NAME_EXISTS_AS_COLLECTION    -833000
#define CAT_NAME_EXISTS_AS_DATAOBJ       -834000
#define CAT_RESOURCE_NOT_EMPTY           -835000
#define CAT_NOT_A_DATAOBJ_AND_NOT_A_COLLECTION -836000
#define CAT_RECURSIVE_MOVE               -837000
#define CAT_LAST_REPLICA                 -838000
#define CAT_OCI_ERROR                    -839000
#define CAT_PASSWORD_EXPIRED             -840000

/* 880,000 to 900,000  RDA errors  */
#define RDA_NOT_COMPILED_IN             -880000
#define RDA_NOT_CONNECTED               -881000
#define RDA_ENV_ERR                     -882000
#define RDA_CONNECT_ERR                 -883000
#define RDA_DISCONNECT_ERR              -884000
#define RDA_CLOSE_ENV_ERR               -885000
#define RDA_SQL_ERR                     -886000
#define RDA_CONFIG_FILE_ERR             -887000
#define RDA_ACCESS_PROHIBITED           -888000
#define RDA_NAME_NOT_FOUND              -889000

/* 900,000 to 920,000 - Misc errors (used by obf library, etc)  */
#define FILE_OPEN_ERR                    -900000
#define FILE_READ_ERR                    -901000
#define FILE_WRITE_ERR                   -902000
#define PASSWORD_EXCEEDS_MAX_SIZE        -903000
#define ENVIRONMENT_VAR_HOME_NOT_DEFINED -904000
#define UNABLE_TO_STAT_FILE              -905000
#define AUTH_FILE_NOT_ENCRYPTED          -906000
#define AUTH_FILE_DOES_NOT_EXIST         -907000
#define UNLINK_FAILED                    -908000
#define NO_PASSWORD_ENTERED              -909000
#define REMOTE_SERVER_AUTHENTICATION_FAILURE -910000
#define REMOTE_SERVER_AUTH_NOT_PROVIDED  -911000
#define REMOTE_SERVER_AUTH_EMPTY         -912000
#define REMOTE_SERVER_SID_NOT_DEFINED    -913000

/* 921,000 to 999,000 - GSI errors  */
#define GSI_NOT_COMPILED_IN              -921000
#define GSI_NOT_BUILT_INTO_CLIENT        -922000
#define GSI_NOT_BUILT_INTO_SERVER        -923000
#define GSI_ERROR_IMPORT_NAME            -924000
#define GSI_ERROR_INIT_SECURITY_CONTEXT  -925000
#define GSI_ERROR_SENDING_TOKEN_LENGTH   -926000
#define GSI_ERROR_READING_TOKEN_LENGTH   -927000
#define GSI_ERROR_TOKEN_TOO_LARGE        -928000
#define GSI_ERROR_BAD_TOKEN_RCVED        -929000
#define GSI_SOCKET_READ_ERROR            -930000
#define GSI_PARTIAL_TOKEN_READ           -931000
#define GSI_SOCKET_WRITE_ERROR           -932000
#define GSI_ERROR_FROM_GSI_LIBRARY       -933000
#define GSI_ERROR_IMPORTING_NAME         -934000
#define GSI_ERROR_ACQUIRING_CREDS        -935000
#define GSI_ACCEPT_SEC_CONTEXT_ERROR     -936000
#define GSI_ERROR_DISPLAYING_NAME        -937000
#define GSI_ERROR_RELEASING_NAME         -938000
#define GSI_DN_DOES_NOT_MATCH_USER       -939000
#define GSI_QUERY_INTERNAL_ERROR         -940000

/* 1,000,000 to 1,500,000  - Rule Engine errors */
#define  OBJPATH_EMPTY_IN_STRUCT_ERR          -1000000
#define  RESCNAME_EMPTY_IN_STRUCT_ERR         -1001000
#define  DATATYPE_EMPTY_IN_STRUCT_ERR         -1002000
#define  DATASIZE_EMPTY_IN_STRUCT_ERR         -1003000
#define  CHKSUM_EMPTY_IN_STRUCT_ERR           -1004000
#define  VERSION_EMPTY_IN_STRUCT_ERR          -1005000
#define  FILEPATH_EMPTY_IN_STRUCT_ERR         -1006000
#define  REPLNUM_EMPTY_IN_STRUCT_ERR          -1007000
#define  REPLSTATUS_EMPTY_IN_STRUCT_ERR       -1008000
#define  DATAOWNER_EMPTY_IN_STRUCT_ERR        -1009000
#define  DATAOWNERZONE_EMPTY_IN_STRUCT_ERR    -1010000
#define  DATAEXPIRY_EMPTY_IN_STRUCT_ERR       -1011000
#define  DATACOMMENTS_EMPTY_IN_STRUCT_ERR     -1012000
#define  DATACREATE_EMPTY_IN_STRUCT_ERR       -1013000
#define  DATAMODIFY_EMPTY_IN_STRUCT_ERR       -1014000
#define  DATAACCESS_EMPTY_IN_STRUCT_ERR       -1015000
#define  DATAACCESSINX_EMPTY_IN_STRUCT_ERR    -1016000
#define  NO_RULE_FOUND_ERR                    -1017000
#define  NO_MORE_RULES_ERR                    -1018000
#define  UNMATCHED_ACTION_ERR                 -1019000
#define  RULES_FILE_READ_ERROR                -1020000
#define  ACTION_ARG_COUNT_MISMATCH            -1021000
#define  MAX_NUM_OF_ARGS_IN_ACTION_EXCEEDED   -1022000
#define  UNKNOWN_PARAM_IN_RULE_ERR            -1023000
#define  DESTRESCNAME_EMPTY_IN_STRUCT_ERR     -1024000
#define  BACKUPRESCNAME_EMPTY_IN_STRUCT_ERR   -1025000
#define DATAID_EMPTY_IN_STRUCT_ERR            -1026000
#define COLLID_EMPTY_IN_STRUCT_ERR            -1027000
#define RESCGROUPNAME_EMPTY_IN_STRUCT_ERR     -1028000
#define STATUSSTRING_EMPTY_IN_STRUCT_ERR      -1029000
#define DATAMAPID_EMPTY_IN_STRUCT_ERR         -1030000
#define USERNAMECLIENT_EMPTY_IN_STRUCT_ERR    -1031000
#define RODSZONECLIENT_EMPTY_IN_STRUCT_ERR    -1032000
#define USERTYPECLIENT_EMPTY_IN_STRUCT_ERR    -1033000
#define HOSTCLIENT_EMPTY_IN_STRUCT_ERR        -1034000
#define AUTHSTRCLIENT_EMPTY_IN_STRUCT_ERR     -1035000
#define USERAUTHSCHEMECLIENT_EMPTY_IN_STRUCT_ERR  -1036000
#define USERINFOCLIENT_EMPTY_IN_STRUCT_ERR    -1037000
#define USERCOMMENTCLIENT_EMPTY_IN_STRUCT_ERR -1038000
#define USERCREATECLIENT_EMPTY_IN_STRUCT_ERR  -1039000
#define USERMODIFYCLIENT_EMPTY_IN_STRUCT_ERR  -1040000
#define USERNAMEPROXY_EMPTY_IN_STRUCT_ERR     -1041000
#define RODSZONEPROXY_EMPTY_IN_STRUCT_ERR     -1042000
#define USERTYPEPROXY_EMPTY_IN_STRUCT_ERR     -1043000
#define HOSTPROXY_EMPTY_IN_STRUCT_ERR         -1044000
#define AUTHSTRPROXY_EMPTY_IN_STRUCT_ERR      -1045000
#define USERAUTHSCHEMEPROXY_EMPTY_IN_STRUCT_ERR    -1046000
#define USERINFOPROXY_EMPTY_IN_STRUCT_ERR     -1047000
#define USERCOMMENTPROXY_EMPTY_IN_STRUCT_ERR  -1048000
#define USERCREATEPROXY_EMPTY_IN_STRUCT_ERR   -1049000
#define USERMODIFYPROXY_EMPTY_IN_STRUCT_ERR   -1050000
#define COLLNAME_EMPTY_IN_STRUCT_ERR          -1051000
#define COLLPARENTNAME_EMPTY_IN_STRUCT_ERR    -1052000
#define COLLOWNERNAME_EMPTY_IN_STRUCT_ERR     -1053000
#define COLLOWNERZONE_EMPTY_IN_STRUCT_ERR     -1054000
#define COLLEXPIRY_EMPTY_IN_STRUCT_ERR        -1055000
#define COLLCOMMENTS_EMPTY_IN_STRUCT_ERR      -1056000
#define COLLCREATE_EMPTY_IN_STRUCT_ERR        -1057000
#define COLLMODIFY_EMPTY_IN_STRUCT_ERR        -1058000
#define COLLACCESS_EMPTY_IN_STRUCT_ERR        -1059000
#define COLLACCESSINX_EMPTY_IN_STRUCT_ERR     -1060000
#define COLLMAPID_EMPTY_IN_STRUCT_ERR         -1062000
#define COLLINHERITANCE_EMPTY_IN_STRUCT_ERR   -1063000
#define RESCZONE_EMPTY_IN_STRUCT_ERR          -1065000
#define RESCLOC_EMPTY_IN_STRUCT_ERR           -1066000
#define RESCTYPE_EMPTY_IN_STRUCT_ERR          -1067000
#define RESCTYPEINX_EMPTY_IN_STRUCT_ERR       -1068000
#define RESCCLASS_EMPTY_IN_STRUCT_ERR         -1069000
#define RESCCLASSINX_EMPTY_IN_STRUCT_ERR      -1070000
#define RESCVAULTPATH_EMPTY_IN_STRUCT_ERR     -1071000
#define NUMOPEN_ORTS_EMPTY_IN_STRUCT_ERR      -1072000
#define PARAOPR_EMPTY_IN_STRUCT_ERR           -1073000
#define RESCID_EMPTY_IN_STRUCT_ERR            -1074000
#define GATEWAYADDR_EMPTY_IN_STRUCT_ERR       -1075000
#define RESCMAX_BJSIZE_EMPTY_IN_STRUCT_ERR    -1076000
#define FREESPACE_EMPTY_IN_STRUCT_ERR         -1077000
#define FREESPACETIME_EMPTY_IN_STRUCT_ERR     -1078000
#define FREESPACETIMESTAMP_EMPTY_IN_STRUCT_ERR     -1079000
#define RESCINFO_EMPTY_IN_STRUCT_ERR          -1080000
#define RESCCOMMENTS_EMPTY_IN_STRUCT_ERR      -1081000
#define RESCCREATE_EMPTY_IN_STRUCT_ERR        -1082000
#define RESCMODIFY_EMPTY_IN_STRUCT_ERR        -1083000
#define INPUT_ARG_NOT_WELL_FORMED_ERR         -1084000
#define INPUT_ARG_OUT_OF_ARGC_RANGE_ERR       -1085000
#define INSUFFICIENT_INPUT_ARG_ERR            -1086000
#define INPUT_ARG_DOES_NOT_MATCH_ERR          -1087000
#define RETRY_WITHOUT_RECOVERY_ERR            -1088000
#define CUT_ACTION_PROCESSED_ERR              -1089000
#define ACTION_FAILED_ERR                     -1090000
#define FAIL_ACTION_ENCOUNTERED_ERR           -1091000
#define VARIABLE_NAME_TOO_LONG_ERR            -1092000
#define UNKNOWN_VARIABLE_MAP_ERR              -1093000
#define UNDEFINED_VARIABLE_MAP_ERR            -1094000
#define NULL_VALUE_ERR                        -1095000
#define DVARMAP_FILE_READ_ERROR               -1096000
#define NO_RULE_OR_MSI_FUNCTION_FOUND_ERR     -1097000
#define FILE_CREATE_ERROR                     -1098000
#define FMAP_FILE_READ_ERROR                  -1099000
#define DATE_FORMAT_ERR                       -1100000
#define RULE_FAILED_ERR                       -1101000
#define NO_MICROSERVICE_FOUND_ERR             -1102000
#define INVALID_REGEXP                        -1103000
#define INVALID_OBJECT_NAME                   -1104000
#define INVALID_OBJECT_TYPE                   -1105000
#define NO_VALUES_FOUND                       -1106000
#define NO_COLUMN_NAME_FOUND                  -1107000
#define BREAK_ACTION_ENCOUNTERED_ERR          -1108000

/* 1,600,000 to 1,700,000  - PHP scripting error */
#define PHP_EXEC_SCRIPT_ERR                   -1600000
#define PHP_REQUEST_STARTUP_ERR               -1601000
#define PHP_OPEN_SCRIPT_FILE_ERR	      -1602000

/* The following are handler protocol type msg. These are not real error */
#define SYS_NULL_INPUT				-99999996
#define SYS_HANDLER_DONE_WITH_ERROR		-99999997
#define SYS_HANDLER_DONE_NO_ERROR		-99999998
#define SYS_NO_HANDLER_REPLY_MSG		-99999999

#endif	/* RODS_ERROR_TABLE_H */
