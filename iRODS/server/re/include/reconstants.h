/* For copyright information please refer to files in the COPYRIGHT directory
 */


#ifndef RECONSTANTS_H
#define RECONSTANTS_H
#include "debug.h"

#define MAX_PARAMS_LEN 100
#define MAX_RULE_LEN (1024 * 64)
#define MAX_NUM_DISJUNCTS 100

/* parser error -1203000 */
#define PARSER_ERROR -1203000
#define UNPARSED_SUFFIX -1203001
#define POINTER_ERROR -1203002
/* runtime error -1205000 */
#define RUNTIME_ERROR -1205000
#define DIVISION_BY_ZERO -1205001
#define BUFFER_OVERFLOW -1205002
#define UNSUPPORTED_OP_OR_TYPE -1205003
#define UNSUPPORTED_SESSION_VAR -1205004
#define UNABLE_TO_WRITE_LOCAL_VAR -1205005
#define UNABLE_TO_READ_LOCAL_VAR -1205006
#define UNABLE_TO_WRITE_SESSION_VAR -1205007
#define UNABLE_TO_READ_SESSION_VAR -1205008
#define UNABLE_TO_WRITE_VAR -1205009
#define UNABLE_TO_READ_VAR -1205010
#define PATTERN_NOT_MATCHED -1205011
#define STRING_OVERFLOW -1205012
/* system error -1207000 */
#define UNKNOWN_ERROR -1207000
#define OUT_OF_MEMORY -1207001
#define SHM_UNLINK_ERROR -1207002
#define FILE_STAT_ERROR -1207003
/* type error -1209000 */
#define TYPE_ERROR -1209000
#define FUNCTION_REDEFINITION -12090001

#define DATETIME_MS_T "DATETIME_MS_T"

#define LIST "[]"
#define TUPLE "<>"
#define APPLICATION "()"
#define META_DATA "@()"
#define AVU "avu"
#define ST_TUPLE "()"
#define FUNC "->"
#define ERR_MSG_SEP "=========="

/* todo change text of dynamically allocated array */
#define MAX_TOKEN_TEXT_LEN 1023

/* #define PARSER_LAZY 0 */
#define MAX_FUNC_PARAMS 20
#define MAX_NUM_RULES 50000
#define CORE_RULE_INDEX_OFF 30000
#define APP_RULE_INDEX_OFF 10000

#define MAX_PREC 20
#define MIN_PREC 0

#define POINTER_BUF_SIZE (16*1024)

#define RULE_ENGINE_IRODS_SERVER 0
#define RULE_ENGINE_RE_SERVER 1
#define RULE_ENGINE_XMSG_SERVER 2
#define RULE_ENGINE_IRODS_AGENT 3

#endif
