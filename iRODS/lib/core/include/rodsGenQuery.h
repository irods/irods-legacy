/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* rodsGenQuery.h - common header file for the generalized query input
 * and results
 */



#ifndef RODS_GEN_QUERY_H
#define RODS_GEN_QUERY_H

#include "objInfo.h"

#define MAX_SQL_ATTR	50
#define MAX_SQL_ROWS	500

/* In genQueryInp_t, selectInp is a int index, int value pair. The index
 * represents the attribute index. 
 * sqlCondInp is a int index, string value pair. The index
 * represents the attribute index. 
 */

typedef struct GenQueryInp {
    int maxRows;	     /* max number of rows to return, if 0 
                                close out the SQL statement call (i.e. instead
                                of getting more rows until it is finished). */
    int continueInx;         /* if non-zero, this is the value returned in
                                the genQueryOut structure and the current
                                call is to get more rows.  In this case, the
                                selectInp & sqlCondInp arguments are ignored.*/
    int rowOffset;           /* if positive, return rows starting with
                                this index (skip earlier ones), 0-origin  */
    int options;             /* Bits for special options, currently:
			        If RETURN_TOTAL_ROW_COUNT is set, the total
                                number of available rows will be returned
                                in totalRowCount (causes a little overhead
                                so only request it if needed).  If rowOffset
                                is also used, totalRowCount will include
                                the skipped rows. */
    keyValPair_t condInput;
    inxIvalPair_t selectInp; /* 1st int array is columns to return (select),
				2nd int array has bits for special options:
                                currently ORDER_BY and ORDER_BY_DESC */
    inxValPair_t sqlCondInp; /* 1st array is columns for conditions (where),
				2nd array has strings for the conditions. */
} genQueryInp_t;


typedef struct SqlResult {
    int attriInx;	/* attribute index */
    int len;		/* strlen of each attribute */
    char *value;	/* char array of [rowCnt][len] */
} sqlResult_t;

typedef struct GenQueryOut {
    int rowCnt;
    int attriCnt;
    int continueInx;
    int totalRowCount; 
    sqlResult_t sqlResult[MAX_SQL_ATTR]; 
} genQueryOut_t; 

/* 
Bits to set in the value array (genQueryInp.selectInp.value[i]) to
order the results by that column, either ascending or decending.  This
is done in the order of the value array, so the first one will be the
primary ordering column.
*/
#define ORDER_BY 0x400
#define ORDER_BY_DESC 0x800

/*
 */
#define RETURN_TOTAL_ROW_COUNT 0x20


/* 
  These are some operations (functions) that can be applied to columns
  being returned (selected) by setting the input array,
  genQueryInp.selectInp.value[i], to these values.  Values 0 and 1 (or
  any not defined here) will just return the column content (i.e. the
  normal, default case).  The ORDER_BY and ORDER_BY_DESC bits are also
  stored in the same input array but are not normally used together
  with these since these will return one row.
*/
#define SELECT_MIN 2
#define SELECT_MAX 3
#define SELECT_SUM 4
#define SELECT_AVG 5
#define SELECT_COUNT 6


/* 
  These are the Table Column names used with the GenQuery.  Also see
  the rcatGeneralQuerySetup routine which associates these values with
  tables and columns.
*/

/* R_ZONE_MAIN: */
#define COL_ZONE_ID 101
#define COL_ZONE_NAME 102
#define COL_ZONE_TYPE 103
#define COL_ZONE_CONNECTION 104
#define COL_ZONE_COMMENT 105

/* R_USER_MAIN: */
#define COL_USER_ID 201
#define COL_USER_NAME 202
#define COL_USER_TYPE 203
#define COL_USER_ZONE 204
#define COL_USER_DN 205
#define COL_USER_INFO 206
#define COL_USER_COMMENT 207
#define COL_USER_CREATE_TIME 208
#define COL_USER_MODIFY_TIME 209

/* R_RESC_MAIN: */
#define COL_R_RESC_ID 301
#define COL_R_RESC_NAME 302
#define COL_R_ZONE_NAME 303
#define COL_R_TYPE_NAME 304
#define COL_R_CLASS_NAME 305
#define COL_R_LOC 306
#define COL_R_VAULT_PATH 307
#define COL_R_FREE_SPACE 308
#define COL_R_RESC_INFO  309
#define COL_R_RESC_COMMENT 310
#define COL_R_CREATE_TIME 311
#define COL_R_MODIFY_TIME 312

/* R_DATA_MAIN: */
#define COL_D_DATA_ID 401
#define COL_D_COLL_ID 402
#define COL_DATA_NAME 403
#define COL_DATA_REPL_NUM 404
#define COL_DATA_VERSION 405
#define COL_DATA_TYPE_NAME 406
#define COL_DATA_SIZE 407
#define COL_D_RESC_GROUP_NAME 408
#define COL_D_RESC_NAME 409
#define COL_D_DATA_PATH 410
#define COL_D_OWNER_NAME 411
#define COL_D_OWNER_ZONE 412
#define COL_D_REPL_STATUS 413 /* isDirty */
#define COL_D_DATA_STATUS 414
#define COL_D_DATA_CHECKSUM 415
#define COL_D_EXPIRY 416
#define COL_D_MAP_ID 417
#define COL_D_COMMENTS 418
#define COL_D_CREATE_TIME 419
#define COL_D_MODIFY_TIME 420
#define COL_DATA_MODE 421

/* R_COLL_MAIN */
#define COL_COLL_ID 500
#define COL_COLL_NAME 501
#define COL_COLL_PARENT_NAME 502
#define COL_COLL_OWNER_NAME 503
#define COL_COLL_OWNER_ZONE 504
#define COL_COLL_MAP_ID 505
#define COL_COLL_INHERITANCE 506
#define COL_COLL_COMMENTS 507
#define COL_COLL_CREATE_TIME 508
#define COL_COLL_MODIFY_TIME 509
#define COL_COLL_TYPE 510
#define COL_COLL_INFO1 511
#define COL_COLL_INFO2 512

/* R_META_MAIN */
#define COL_META_DATA_ATTR_NAME 600
#define COL_META_DATA_ATTR_VALUE 601
#define COL_META_DATA_ATTR_UNITS 602
#define COL_META_DATA_ATTR_ID 603

#define COL_META_COLL_ATTR_NAME 610
#define COL_META_COLL_ATTR_VALUE 611
#define COL_META_COLL_ATTR_UNITS 612
#define COL_META_COLL_ATTR_ID 613

#define COL_META_NAMESPACE_COLL 620
#define COL_META_NAMESPACE_DATA 621
#define COL_META_NAMESPACE_RESC 622
#define COL_META_NAMESPACE_USER 623

#define COL_META_RESC_ATTR_NAME 630
#define COL_META_RESC_ATTR_VALUE 631
#define COL_META_RESC_ATTR_UNITS 632
#define COL_META_RESC_ATTR_ID 633

#define COL_META_USER_ATTR_NAME 640
#define COL_META_USER_ATTR_VALUE 641
#define COL_META_USER_ATTR_UNITS 642
#define COL_META_USER_ATTR_ID 643

/* R_OBJT_ACCESS */
#define COL_DATA_ACCESS_TYPE 700
#define COL_DATA_ACCESS_NAME 701
#define COL_DATA_TOKEN_NAMESPACE 702
#define COL_DATA_ACCESS_USER_ID 703
#define COL_DATA_ACCESS_DATA_ID 704

#define COL_COLL_ACCESS_TYPE 710
#define COL_COLL_ACCESS_NAME 711
#define COL_COLL_TOKEN_NAMESPACE 712
#define COL_COLL_ACCESS_USER_ID 713
#define COL_COLL_ACCESS_COLL_ID 714

/* R_RESC_GROUP */
#define COL_RESC_GROUP_RESC_ID 800
#define COL_RESC_GROUP_NAME 801

/* R_USER_GROUP / USER */
#define COL_USER_GROUP_ID 900
#define COL_USER_GROUP_NAME 901

/* R_RULE_EXEC */
#define COL_RULE_EXEC_ID 1000
#define COL_RULE_EXEC_NAME 1001
#define COL_RULE_EXEC_REI_FILE_PATH 1002
#define COL_RULE_EXEC_USER_NAME   1003
#define COL_RULE_EXEC_ADDRESS 1004
#define COL_RULE_EXEC_TIME    1005
#define COL_RULE_EXEC_FREQUENCY 1006
#define COL_RULE_EXEC_PRIORITY 1007
#define COL_RULE_EXEC_ESTIMATED_EXE_TIME 1008
#define COL_RULE_EXEC_NOTIFICATION_ADDR 1009
#define COL_RULE_EXEC_LAST_EXE_TIME 1010
#define COL_RULE_EXEC_STATUS 1011

/* R_TOKN_MAIN */
#define COL_TOKEN_NAMESPACE 1100
#define COL_TOKEN_ID 1101
#define COL_TOKEN_NAME 1102
#define COL_TOKEN_VALUE 1103
#define COL_TOKEN_VALUE2 1104
#define COL_TOKEN_VALUE3 1105
#define COL_TOKEN_COMMENT 1106

/* R_OBJT_AUDIT */
#define COL_AUDIT_OBJ_ID      1200
#define COL_AUDIT_USER_ID     1201
#define COL_AUDIT_ACTION_ID   1202
#define COL_AUDIT_COMMENT     1203
#define COL_AUDIT_CREATE_TIME 1204
#define COL_AUDIT_MODIFY_TIME 1205

/* Range of the Audit columns; used sometimes to restrict access */
#define COL_AUDIT_RANGE_START 1200
#define COL_AUDIT_RANGE_END   1299

/* R_COLL_USER_MAIN (r_user_main for Collection information) */
#define COL_COLL_USER_NAME    1300
#define COL_COLL_USER_ZONE    1301

/* R_SERVER_LOAD */
#define COL_SL_HOST_NAME      1400
#define COL_SL_RESC_NAME      1401
#define COL_SL_CPU_USED       1402
#define COL_SL_MEM_USED       1403
#define COL_SL_SWAP_USED      1404
#define COL_SL_RUNQ_LOAD      1405
#define COL_SL_DISK_SPACE     1406
#define COL_SL_NET_INPUT      1407
#define COL_SL_NET_OUTPUT     1408
#define COL_SL_NET_OUTPUT     1408
#define COL_SL_CREATE_TIME    1409

/* R_SERVER_LOAD_DIGEST */
#define COL_SLD_RESC_NAME     1500
#define COL_SLD_LOAD_FACTOR   1501
#define COL_SLD_CREATE_TIME   1502

#endif	/* RODS_GEN_QUERY_H */
