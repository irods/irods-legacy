/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/*

  This routine is called to set up the tables needed by
  chlGeneralQuery.

  A series of sTable calls are made to define the table names.  When
  the 2nd argument is two strings, it's an alias and will be of the
  form sTable("name1", "name2 name1", n) and name1 will be the alias
  for name2.  The From portion of the SQL with have "name2 name1" and
  name1 will be used for the table name elsewhere.  This provides a
  more meaningful name to the client and allows the Where clause to do
  multiple comparisons of fields in the same table in different ways.

  The last argument to sTable is 0 or 1 to indicate if the table
  is a 'cycler'.  When 1, the the sql generation function will stop
  at this table to avoid cycles in the graph.  We had some set but
  currently all are 0 because they are natural terminators; that is,
  each table has either links that can be used or no further links.

  A series for sColumns calls are made to map the #define values to
  tables and columns.

  And a series of sFklink's are called to initialize the Foreign
  Key table specifying how tables are related, how foreign keys can be
  used to tie them together.  Arguments are: table1, table2, and sql
  text to link them together.  This means that table1 and table2 can
  be related together via the sql text string.  This creates a tree
  structure describing the ICAT schema.

*/
#include "rodsClient.h"
#include "icatHighLevelRoutines.h"

void
icatGeneralQuerySetup() {

  sTableInit();   /* initialize */

  sTable( "r_user_password",  "r_user_password", 0);
  sTable( "r_user_session_key",  "r_user_session_key", 0);
  sTable( "r_tokn_main",  "r_tokn_main", 0);
  sTable( "r_resc_group",  "r_resc_group", 0);
  sTable( "r_zone_main",  "r_zone_main", 0);
  sTable( "r_resc_main",  "r_resc_main", 0);
  sTable( "r_coll_main",  "r_coll_main", 0);
  sTable( "r_data_main",  "r_data_main", 0);

  sTable( "r_met2_main", "r_meta_main r_met2_main" ,0);
  sTable( "r_meta_main", "r_meta_main" , 0);

  sTable( "r_rule_main",  "r_rule_main", 0);
  sTable( "r_user_main",  "r_user_main", 0);
  sTable( "r_resc_access", "r_objt_access r_resc_access", 0);
  sTable( "r_coll_access", "r_objt_access r_coll_access", 0);
  sTable( "r_data_access", "r_objt_access r_data_access", 0);
  sTable( "r_met2_access", "r_objt_access r_met2_access", 0);
  sTable( "r_rule_access", "r_objt_access r_rule_access", 0);
  sTable( "r_resc_audit", "r_objt_audit r_resc_audit", 0);
  sTable( "r_coll_audit", "r_objt_audit r_coll_audit", 0);
  sTable( "r_data_audit", "r_objt_audit r_data_audit", 0);
  sTable( "r_met2_audit", "r_objt_audit r_met2_audit", 0);
  sTable( "r_rule_audit", "r_objt_audit r_rule_audit", 0);
  sTable( "r_resc_deny_access", "r_objt_deny_access r_resc_deny_access", 0);
  sTable( "r_coll_deny_access", "r_objt_deny_access r_coll_deny_access", 0);
  sTable( "r_data_deny_access", "r_objt_deny_access r_data_deny_access", 0);
  sTable( "r_met2_deny_access", "r_objt_deny_access r_met2_deny_access", 0);
  sTable( "r_rule_deny_access", "r_objt_deny_access r_rule_deny_access", 0);
  sTable( "r_resc_metamap", "r_objt_metamap r_resc_metamap", 0);
  sTable( "r_coll_metamap", "r_objt_metamap r_coll_metamap", 0);
  sTable( "r_data_metamap", "r_objt_metamap r_data_metamap", 0);
  sTable( "r_met2_metamap", "r_objt_metamap r_met2_metamap", 0);
  sTable( "r_rule_metamap", "r_objt_metamap r_rule_metamap", 0);
  sTable( "r_user_metamap", "r_objt_metamap r_user_metamap", 0);
  sTable( "r_resc_user_group", "r_user_group r_resc_user_group", 0);
  sTable( "r_coll_user_group", "r_user_group r_coll_user_group", 0);
  sTable( "r_data_user_group", "r_user_group r_data_user_group", 0);
  sTable( "r_met2_user_group", "r_user_group r_met2_user_group", 0);
  sTable( "r_rule_user_group", "r_user_group r_rule_user_group", 0);
  sTable( "r_resc_da_user_group", "r_user_group r_resc_da_user_group", 0);
  sTable( "r_coll_da_user_group", "r_user_group r_coll_da_user_group", 0);
  sTable( "r_data_da_user_group", "r_user_group r_data_da_user_group", 0);
  sTable( "r_met2_da_user_group", "r_user_group r_met2_da_user_group", 0);
  sTable( "r_rule_da_user_group", "r_user_group r_rule_da_user_group", 0);

  sTable( "r_resc_au_user_group", "r_user_group r_resc_au_user_group", 0);
  sTable( "r_coll_au_user_group", "r_user_group r_coll_au_user_group", 0);
  sTable( "r_data_au_user_group", "r_user_group r_data_au_user_group", 0);
  sTable( "r_met2_au_user_group", "r_user_group r_met2_au_user_group", 0);
  sTable( "r_rule_au_user_group", "r_user_group r_rule_au_user_group", 0);
  sTable( "r_resc_user_main", "r_user_main r_resc_user_main", 0);
  sTable( "r_coll_user_main", "r_user_main r_coll_user_main", 0);
  sTable( "r_data_user_main", "r_user_main r_data_user_main", 0);
  sTable( "r_met2_user_main", "r_user_main r_met2_user_main", 0);
  sTable( "r_rule_user_main", "r_user_main r_rule_user_main", 0);
  sTable( "r_resc_da_user_main", "r_user_main r_resc_da_user_main", 0);
  sTable( "r_coll_da_user_main", "r_user_main r_coll_da_user_main", 0);
  sTable( "r_data_da_user_main", "r_user_main r_data_da_user_main", 0);
  sTable( "r_met2_da_user_main", "r_user_main r_met2_da_user_main", 0);
  sTable( "r_rule_da_user_main", "r_user_main r_rule_da_user_main", 0);
  sTable( "r_resc_au_user_main", "r_user_main r_resc_au_user_main", 0);
  sTable( "r_coll_au_user_main", "r_user_main r_coll_au_user_main", 0);
  sTable( "r_data_au_user_main", "r_user_main r_data_au_user_main", 0);
  sTable( "r_met2_au_user_main", "r_user_main r_met2_au_user_main", 0);
  sTable( "r_rule_au_user_main", "r_user_main r_rule_au_user_main", 0);

  sTable( "r_resc_tokn_accs", "r_tokn_main r_resc_tokn_accs", 0);
  sTable( "r_coll_tokn_accs", "r_tokn_main r_coll_tokn_accs", 0);
  sTable( "r_data_tokn_accs", "r_tokn_main r_data_tokn_accs", 0);

  sTable( "r_rule_tokn_accs", "r_tokn_main r_rule_tokn_accs", 0);
  sTable( "r_met2_tokn_accs", "r_tokn_main r_met2_tokn_accs", 0);
  sTable( "r_resc_tokn_deny_accs", "r_tokn_main r_resc_tokn_deny_accs", 0);
  sTable( "r_coll_tokn_deny_accs", "r_tokn_main r_coll_tokn_deny_accs", 0);
  sTable( "r_data_tokn_deny_accs", "r_tokn_main r_data_tokn_deny_accs", 0);
  sTable( "r_rule_tokn_deny_accs", "r_tokn_main r_rule_tokn_deny_accs", 0);
  sTable( "r_met2_tokn_deny_accs", "r_tokn_main r_met2_tokn_deny_accs", 0);
  sTable( "r_resc_tokn_audit", "r_tokn_main r_resc_tokn_audit", 0);
  sTable( "r_coll_tokn_audit", "r_tokn_main r_coll_tokn_audit", 0);
  sTable( "r_data_tokn_audit", "r_tokn_main r_data_tokn_audit", 0);
  sTable( "r_rule_tokn_audit", "r_tokn_main r_rule_tokn_audit", 0);
  sTable( "r_met2_tokn_audit", "r_tokn_main r_met2_tokn_audit", 0);
  sTable( "r_resc_meta_main", "r_meta_main r_resc_meta_main", 0);
  sTable( "r_coll_meta_main", "r_meta_main r_coll_meta_main", 0);
  sTable( "r_data_meta_main", "r_meta_main r_data_meta_main", 0);
  sTable( "r_rule_meta_main", "r_meta_main r_rule_meta_main", 0);
  sTable( "r_user_meta_main", "r_meta_main r_user_meta_main", 0);
  sTable( "r_met2_meta_main", "r_meta_main r_met2_meta_main", 0);

  sTable( "r_user_group",  "r_user_group", 0);
  sTable( "r_group_main", "r_user_main r_group_main", 0);

  sTable( "r_rule_exec", "r_rule_exec", 0);

  sTable( "r_objt_audit", "r_objt_audit", 0);

  sTable( "r_server_load", "r_server_load", 0);

  sTable( "r_server_load_digest", "r_server_load_digest", 0);

  /* Map the #define values to tables and columns */

  sColumn( COL_ZONE_ID, "r_zone_main", "zone_id");
  sColumn( COL_ZONE_NAME, "r_zone_main", "zone_name");
  sColumn( COL_ZONE_TYPE, "r_zone_main", "zone_type_name");
  sColumn( COL_ZONE_CONNECTION, "r_zone_main", "zone_conn_string");
  sColumn( COL_ZONE_COMMENT, "r_zone_main", "r_comment");

  sColumn( COL_USER_ID,   "r_user_main", "user_id");
  sColumn( COL_USER_NAME, "r_user_main", "user_name");
  sColumn( COL_USER_TYPE, "r_user_main", "user_type_name");
  sColumn( COL_USER_ZONE, "r_user_main", "zone_name");
  sColumn( COL_USER_DN,   "r_user_main", "user_distin_name");
  sColumn( COL_USER_INFO, "r_user_main", "user_info");
  sColumn( COL_USER_COMMENT,     "r_user_main", "r_comment");
  sColumn( COL_USER_CREATE_TIME, "r_user_main", "create_ts");
  sColumn( COL_USER_MODIFY_TIME, "r_user_main", "modify_ts");

  sColumn( COL_R_RESC_ID, "r_resc_main", "resc_id");
  sColumn( COL_R_RESC_NAME, "r_resc_main", "resc_name");
  sColumn( COL_R_ZONE_NAME, "r_resc_main", "zone_name");
  sColumn( COL_R_TYPE_NAME, "r_resc_main", "resc_type_name");
  sColumn( COL_R_CLASS_NAME, "r_resc_main", "resc_class_name");
  sColumn( COL_R_LOC, "r_resc_main", "resc_net");
  sColumn( COL_R_VAULT_PATH, "r_resc_main", "resc_def_path ");
  sColumn( COL_R_FREE_SPACE, "r_resc_main", "free_space");
  sColumn( COL_R_RESC_INFO, "r_resc_main", "resc_info");
  sColumn( COL_R_RESC_COMMENT, "r_resc_main", "r_comment");
  sColumn( COL_R_CREATE_TIME, "r_resc_main", "create_ts");
  sColumn( COL_R_MODIFY_TIME, "r_resc_main", "modify_ts ");

  sColumn( COL_D_DATA_ID, "r_data_main", "data_id");
  sColumn( COL_D_COLL_ID, "r_data_main", "coll_id");
  sColumn( COL_DATA_NAME, "r_data_main", "data_name");
  sColumn( COL_DATA_REPL_NUM, "r_data_main", "data_repl_num");
  sColumn( COL_DATA_VERSION, "r_data_main", "data_version");
  sColumn( COL_DATA_TYPE_NAME, "r_data_main", "data_type_name");
  sColumn( COL_DATA_SIZE, "r_data_main", "data_size");
  sColumn( COL_D_RESC_GROUP_NAME, "r_data_main", "resc_group_name");
  sColumn( COL_D_RESC_NAME, "r_data_main", "resc_name");
  sColumn( COL_D_DATA_PATH, "r_data_main", "data_path");
  sColumn( COL_D_OWNER_NAME, "r_data_main", "data_owner_name");
  sColumn( COL_D_OWNER_ZONE, "r_data_main", "data_owner_zone");
  sColumn( COL_D_REPL_STATUS, "r_data_main", "data_is_dirty");
  sColumn( COL_D_DATA_STATUS, "r_data_main", "data_status");
  sColumn( COL_D_DATA_CHECKSUM, "r_data_main", "data_checksum");
  sColumn( COL_D_EXPIRY, "r_data_main", "data_expiry_ts");
  sColumn( COL_D_MAP_ID, "r_data_main", "data_map_id");
  sColumn( COL_D_COMMENTS, "r_data_main", "r_comment");
  sColumn( COL_D_CREATE_TIME, "r_data_main", "create_ts");
  sColumn( COL_D_MODIFY_TIME, "r_data_main", "modify_ts");
  sColumn( COL_DATA_MODE, "r_data_main", "data_mode");


  sColumn( COL_DATA_ACCESS_TYPE, "r_data_access", "access_type_id");
  sColumn( COL_DATA_ACCESS_NAME, "r_data_tokn_accs", "token_name");
  sColumn( COL_DATA_TOKEN_NAMESPACE, "r_data_tokn_accs", "token_namespace");
  sColumn( COL_DATA_ACCESS_USER_ID, "r_data_access", "user_id");
  sColumn( COL_DATA_ACCESS_DATA_ID, "r_data_access", "object_id");

  sColumn( COL_COLL_ACCESS_TYPE, "r_coll_access", "access_type_id");
  sColumn( COL_COLL_ACCESS_NAME, "r_coll_tokn_accs", "token_name");
  sColumn( COL_COLL_TOKEN_NAMESPACE, "r_coll_tokn_accs", "token_namespace");
  sColumn( COL_COLL_ACCESS_USER_ID, "r_coll_access", "user_id");
  sColumn( COL_COLL_ACCESS_COLL_ID, "r_coll_access", "object_id");

  sColumn( COL_COLL_ID, "r_coll_main", "coll_id");
  sColumn( COL_COLL_NAME, "r_coll_main", "coll_name");
  sColumn( COL_COLL_PARENT_NAME, "r_coll_main", "parent_coll_name");
  sColumn( COL_COLL_OWNER_NAME, "r_coll_main", "coll_owner_name");
  sColumn( COL_COLL_OWNER_ZONE, "r_coll_main", "coll_owner_zone");
  sColumn( COL_COLL_MAP_ID, "r_coll_main", "coll_map_id");
  sColumn( COL_COLL_INHERITANCE, "r_coll_main", "coll_inheritance");
  sColumn( COL_COLL_COMMENTS, "r_coll_main", "r_comment");
  sColumn( COL_COLL_CREATE_TIME, "r_coll_main", "create_ts");
  sColumn( COL_COLL_MODIFY_TIME, "r_coll_main", "modify_ts");
  sColumn( COL_COLL_TYPE, "r_coll_main", "coll_type");
  sColumn( COL_COLL_INFO1, "r_coll_main", "coll_info1");
  sColumn( COL_COLL_INFO2, "r_coll_main", "coll_info2");

  sColumn( COL_META_DATA_ATTR_NAME, "r_data_meta_main", "meta_attr_name");
  sColumn( COL_META_DATA_ATTR_VALUE, "r_data_meta_main", "meta_attr_value");
  sColumn( COL_META_DATA_ATTR_UNITS, "r_data_meta_main", "meta_attr_unit");
  sColumn( COL_META_DATA_ATTR_ID,   "r_data_meta_main", "meta_id");
 
  sColumn( COL_META_COLL_ATTR_NAME, "r_coll_meta_main", "meta_attr_name");
  sColumn( COL_META_COLL_ATTR_VALUE, "r_coll_meta_main", "meta_attr_value");
  sColumn( COL_META_COLL_ATTR_UNITS, "r_coll_meta_main", "meta_attr_unit");
  sColumn( COL_META_COLL_ATTR_ID,   "r_coll_meta_main", "meta_id");

  sColumn( COL_META_NAMESPACE_COLL, "r_coll_meta_main", "meta_namespace");
  sColumn( COL_META_NAMESPACE_DATA, "r_data_meta_main", "meta_namespace");
  sColumn( COL_META_NAMESPACE_RESC, "r_resc_meta_main", "meta_namespace");
  sColumn( COL_META_NAMESPACE_USER, "r_user_meta_main", "meta_namespace");

  sColumn( COL_META_RESC_ATTR_NAME, "r_resc_meta_main", "meta_attr_name");
  sColumn( COL_META_RESC_ATTR_VALUE, "r_resc_meta_main", "meta_attr_value");
  sColumn( COL_META_RESC_ATTR_UNITS, "r_resc_meta_main", "meta_attr_unit");
  sColumn( COL_META_RESC_ATTR_ID,   "r_resc_meta_main", "meta_id");

  sColumn( COL_META_USER_ATTR_NAME, "r_user_meta_main", "meta_attr_name");
  sColumn( COL_META_USER_ATTR_VALUE, "r_user_meta_main", "meta_attr_value");
  sColumn( COL_META_USER_ATTR_UNITS, "r_user_meta_main", "meta_attr_unit");
  sColumn( COL_META_USER_ATTR_ID,   "r_user_meta_main", "meta_id");

  sColumn( COL_RESC_GROUP_RESC_ID, "r_resc_group", "resc_id");
  sColumn( COL_RESC_GROUP_NAME, "r_resc_group", "resc_group_name");

  sColumn( COL_USER_GROUP_ID, "r_user_group",  "group_user_id");
  sColumn( COL_USER_GROUP_NAME, "r_group_main", "user_name");

  sColumn( COL_RULE_EXEC_ID, "r_rule_exec", "rule_exec_id");
  sColumn( COL_RULE_EXEC_NAME, "r_rule_exec", "rule_name");
  sColumn( COL_RULE_EXEC_REI_FILE_PATH, "r_rule_exec", "rei_file_path");
  sColumn( COL_RULE_EXEC_USER_NAME, "r_rule_exec", "user_name");
  sColumn( COL_RULE_EXEC_ADDRESS, "r_rule_exec", "exe_address");
  sColumn( COL_RULE_EXEC_TIME, "r_rule_exec", "exe_time");
  sColumn( COL_RULE_EXEC_FREQUENCY, "r_rule_exec", "exe_frequency");
  sColumn( COL_RULE_EXEC_PRIORITY, "r_rule_exec", "priority");
  sColumn( COL_RULE_EXEC_ESTIMATED_EXE_TIME,"r_rule_exec","estimated_exe_time");
  sColumn( COL_RULE_EXEC_NOTIFICATION_ADDR,"r_rule_exec", "notification_addr");
  sColumn( COL_RULE_EXEC_LAST_EXE_TIME,"r_rule_exec", "last_exe_time");
  sColumn( COL_RULE_EXEC_STATUS,"r_rule_exec", "exe_status");

  sColumn(COL_TOKEN_NAMESPACE, "r_tokn_main", "token_namespace");
  sColumn(COL_TOKEN_ID, "r_tokn_main", "token_id");
  sColumn(COL_TOKEN_NAME, "r_tokn_main", "token_name");
  sColumn(COL_TOKEN_VALUE, "r_tokn_main", "token_value");
  sColumn(COL_TOKEN_VALUE2, "r_tokn_main", "token_value2");
  sColumn(COL_TOKEN_VALUE3, "r_tokn_main", "token_value3");
  sColumn(COL_TOKEN_COMMENT, "r_tokn_main", "r_comment");

  sColumn(COL_AUDIT_OBJ_ID,      "r_objt_audit", "object_id");
  sColumn(COL_AUDIT_USER_ID,     "r_objt_audit", "user_id");
  sColumn(COL_AUDIT_ACTION_ID,   "r_objt_audit", "action_id");
  sColumn(COL_AUDIT_COMMENT,     "r_objt_audit", "r_comment");
  sColumn(COL_AUDIT_CREATE_TIME, "r_objt_audit", "create_ts");
  sColumn(COL_AUDIT_MODIFY_TIME, "r_objt_audit", "modify_ts");

  sColumn(COL_COLL_USER_NAME, "r_coll_user_main", "user_name");
  sColumn(COL_COLL_USER_ZONE, "r_coll_user_main", "zone_name");

  sColumn(COL_SL_HOST_NAME, "r_server_load", "resc_host");
  sColumn(COL_SL_RESC_NAME, "r_server_load", "resc_name");
  sColumn(COL_SL_CPU_USED, "r_server_load", "cpu_used");
  sColumn(COL_SL_MEM_USED, "r_server_load", "mem_used");
  sColumn(COL_SL_SWAP_USED, "r_server_load", "swap_used");
  sColumn(COL_SL_RUNQ_LOAD, "r_server_load", "runq_load");
  sColumn(COL_SL_DISK_SPACE, "r_server_load", "disk_space");
  sColumn(COL_SL_NET_INPUT, "r_server_load", "net_input");
  sColumn(COL_SL_NET_OUTPUT, "r_server_load", "net_output");
  sColumn(COL_SL_CREATE_TIME, "r_server_load", "create_ts");

  sColumn(COL_SLD_RESC_NAME, "r_server_load_digest", "resc_name");
  sColumn(COL_SLD_LOAD_FACTOR, "r_server_load_digest", "load_factor");
  sColumn(COL_SLD_CREATE_TIME, "r_server_load_digest", "create_ts");

  /* Define the Foreign Key links between tables */
  sFklink("r_coll_main", "r_data_main", "r_coll_main.coll_id = r_data_main.coll_id");
  sFklink("r_resc_group", "r_resc_main", "r_resc_group.resc_id = r_resc_main.resc_id");
  sFklink("r_resc_main", "r_resc_metamap", "r_resc_main.resc_id = r_resc_metamap.object_id");
  sFklink("r_resc_main", "r_data_main", "r_resc_main.resc_name = r_data_main.resc_name");
  sFklink("r_coll_main", "r_coll_metamap", "r_coll_main.coll_id = r_coll_metamap.object_id");
  sFklink("r_data_main", "r_data_metamap", "r_data_main.data_id = r_data_metamap.object_id");
  sFklink("r_rule_main", "r_rule_metamap", "r_rule_main.rule_id = r_rule_metamap.object_id");
  sFklink("r_user_main", "r_user_metamap", "r_user_main.user_id = r_user_metamap.object_id");
  sFklink("r_met2_main", "r_met2_metamap", "r_met2_main.meta_id = r_met2_metamap.object_id");
  sFklink("r_resc_main", "r_resc_access", "r_resc_main.resc_id = r_resc_access..object_id");
  sFklink("r_coll_main", "r_coll_access", "r_coll_main.coll_id = r_coll_access.object_id");
  sFklink("r_data_main", "r_data_access", "r_data_main.data_id = r_data_access.object_id");
  sFklink("r_rule_main", "r_rule_access", "r_rule_main.rule_id = r_rule_access.object_id");
  sFklink("r_met2_main", "r_met2_access", "r_met2_main.meta_id = r_met2_access.object_id");
  sFklink("r_resc_main", "r_resc_deny_access", "r_resc_main.resc_id = r_resc_deny_access.object_id");
  sFklink("r_coll_main", "r_coll_deny_access", "r_coll_main.coll_id = r_coll_deny_access.object_id");
  sFklink("r_data_main", "r_data_deny_access", "r_data_main.data_id = r_data_deny_access.object_id");
  sFklink("r_rule_main", "r_rule_deny_access", "r_rule_main.rule_id = r_rule_deny_access.object_id");
  sFklink("r_met2_main", "r_met2_deny_access", "r_met2_main.meta_id = r_met2_deny_access.object_id");
  sFklink("r_resc_main", "r_resc_audit", "r_resc_main.resc_id = r_resc_audit.object_id");
  sFklink("r_coll_main", "r_coll_audit", "r_coll_main.coll_id = r_coll_audit.object_id");
  sFklink("r_data_main", "r_data_audit", "r_data_main.data_id = r_data_audit.object_id");
  sFklink("r_rule_main", "r_rule_audit", "r_rule_main.rule_id = r_rule_audit.object_id");
  sFklink("r_met2_main", "r_met2_audit", "r_met2_main.meta_id = r_met2_audit.object_id.meta_id");
  sFklink("r_resc_metamap", "r_resc_meta_main",  "r_resc_metamap.meta_id = r_resc_meta_main.meta_id");
  sFklink("r_coll_metamap", "r_coll_meta_main",  "r_coll_metamap.meta_id = r_coll_meta_main.meta_id");
  sFklink("r_data_metamap", "r_data_meta_main",  "r_data_metamap.meta_id = r_data_meta_main.meta_id");
  sFklink("r_rule_metamap", "r_rule_meta_main",  "r_rule_metamap.meta_id = r_rule_meta_main.meta_id");
  sFklink("r_user_metamap", "r_user_meta_main",  "r_user_metamap.meta_id = r_user_meta_main.meta_id");
  sFklink("r_met2_metamap", "r_met2_meta_main",  "r_met2_metamap.object_id = r_met2_meta_main.meta_id");
  sFklink("r_resc_access", "r_resc_tokn_accs",  "r_resc_access.access_typ_id = r_resc_tokn_accs.token_id");
  sFklink("r_resc_access", "r_resc_user_group", "r_resc_access.user_id = r_resc_user_group.group_user_id");
  sFklink("r_coll_access", "r_coll_tokn_accs",  "r_coll_access.access_type_id = r_coll_tokn_accs.token_id");
  sFklink("r_coll_access", "r_coll_user_group", "r_coll_access.user_id = r_coll_user_group.group_user_id");
  sFklink("r_data_access", "r_data_tokn_accs",  "r_data_access.access_type_id = r_data_tokn_accs.token_id");
  sFklink("r_data_access", "r_data_user_group", "r_data_access.user_id = r_data_user_group.group_user_id");
  sFklink("r_rule_access", "r_rule_tokn_accs",  "r_rule_access.access_typ_id = r_rule_tokn_accs.token_id");
  sFklink("r_rule_access", "r_rule_user_group", "r_rule_access.user_id = r_rule_user_group.group_user_id");
  sFklink("r_met2_access", "r_met2_tokn_accs",  "r_met2_access.access_typ_id = r_met2_tokn_accs.token_id");
  sFklink("r_met2_access", "r_met2_user_group", "r_met2_access.user_id = r_met2_user_group.group_user_id");
  sFklink("r_resc_deny_access", "r_resc_tokn_deny_accs", "r_resc_deny_access.access_typ_id = r_resc_tokn_deny_accs.token_id");
  sFklink("r_resc_deny_access", "r_resc_da_user_group",     "r_resc_deny_access.user_id = r_resc_da_user_group.group_user_id");
  sFklink("r_coll_deny_access", "r_coll_tokn_deny_accs", "r_coll_deny_access.access_typ_id = r_coll_tokn_deny_accs.token_id");
  sFklink("r_coll_deny_access", "r_coll_da_user_group",     "r_coll_deny_access.user_id = r_coll_da_user_group.group_user_id");
  sFklink("r_data_deny_access", "r_data_tokn_deny_accs", "r_data_deny_access.access_typ_id = r_data_tokn_deny_accs.token_id");
  sFklink("r_data_deny_access", "r_data_da_user_group",     "r_data_deny_access.user_id = r_data_da_user_group.group_user_id");
  sFklink("r_rule_deny_access", "r_rule_tokn_deny_accs", "r_rule_deny_access.access_typ_id = r_rule_tokn_deny_accs.token_id");
  sFklink("r_rule_deny_access", "r_rule_da_user_group",     "r_rule_deny_access.user_id = r_rule_da_user_group.group_user_id");
  sFklink("r_met2_deny_access", "r_met2_tokn_deny_accs", "r_met2_deny_access.access_typ_id = r_met2_tokn_deny_accs.token_id");
  sFklink("r_met2_deny_access", "r_met2_da_user_group",     "r_met2_deny_access.user_id = r_met2_da_user_group.group_user_id");
  sFklink("r_resc_audit", "r_resc_tokn_audit", "r_resc_audit.action_id = r_resc_tokn_audit.token_id");
  sFklink("r_resc_audit", "r_resc_au_user_group", "r_resc_audit.user_id = r_resc_au_user_group.group_user_id");
  sFklink("r_coll_audit", "r_coll_tokn_audit", "r_coll_audit.action_id = r_coll_tokn_audit.token_id");
  sFklink("r_coll_audit", "r_coll_au_user_group", "r_coll_audit.user_id = r_coll_au_user_group.group_user_id");
  sFklink("r_data_audit", "r_data_tokn_audit", "r_data_audit.action_id = r_data_tokn_audit.token_id");
  sFklink("r_data_audit", "r_data_au_user_group", "r_data_audit.user_id = r_data_au_user_group.group_user_id");
  sFklink("r_rule_audit", "r_rule_tokn_audit", "r_rule_audit.action_id = r_rule_tokn_audit.token_id");
  sFklink("r_rule_audit", "r_rule_au_user_group", "r_rule_audit.user_id = r_rule_au_user_group.group_user_id");
  sFklink("r_met2_audit", "r_met2_tokn_audit", "r_met2_audit.action_id = r_met2_tokn_audit.token_id");
  sFklink("r_met2_audit", "r_met2_au_user_group", "r_met2_audit.user_id = r_met2_au_user_group.group_user_id");
  sFklink("r_resc_user_group", "r_resc_user_main", "r_resc_user_group.user_id = r_resc_user_main.user_id");
  sFklink("r_coll_user_group", "r_coll_user_main", "r_coll_user_group.user_id = r_coll_user_main.user_id");
  sFklink("r_data_user_group", "r_data_user_main", "r_data_user_group.user_id = r_data_user_main.user_id");
  sFklink("r_rule_user_group", "r_rule_user_main", "r_rule_user_group.user_id = r_rule_user_main.user_id");
  sFklink("r_met2_user_group", "r_met2_user_main", "r_met2_user_group.user_id = r_met2_user_main.user_id");
  sFklink("r_resc_da_user_group", "r_resc_da_user_main", "r_resc_da_user_group.user_id = r_resc_da_user_main.user_id");
  sFklink("r_coll_da_user_group", "r_coll_da_user_main", "r_coll_da_user_group.user_id = r_coll_da_user_main.user_id");
  sFklink("r_data_da_user_group", "r_data_da_user_main", "r_data_da_user_group.user_id = r_data_da_user_main.user_id");
  sFklink("r_rule_da_user_group", "r_rule_da_user_main", "r_rule_da_user_group.user_id = r_rule_da_user_main.user_id");
  sFklink("r_met2_da_user_group", "r_met2_da_user_main", "r_met2_da_user_group.user_id = r_met2_da_user_main.user_id");
  sFklink("r_resc_au_user_group", "r_resc_au_user_main", "r_resc_au_user_group.user_id = r_resc_au_user_main.user_id");
  sFklink("r_coll_au_user_group", "r_coll_au_user_main", "r_coll_au_user_group.user_id = r_coll_au_user_main.user_id");
  sFklink("r_data_au_user_group", "r_data_au_user_main", "r_data_au_user_group.user_id = r_data_au_user_main.user_id");
  sFklink("r_rule_au_user_group", "r_rule_au_user_main", "r_rule_au_user_group.user_id = r_rule_au_user_main.user_id");
  sFklink("r_met2_au_user_group", "r_met2_au_user_main", "r_met2_au_user_group.user_id = r_met2_au_user_main.user_id");
  sFklink("r_user_main", "r_user_password", "r_user_main.user_id = r_user_password.user_id");
  sFklink("r_user_main", "r_user_session_key", "r_user_main.user_id = r_user_session_key.user_id");
  sFklink("r_user_main", "r_data_access", "r_user_main.user_id = r_data_access.user_id");
  sFklink("r_user_main", "r_user_group", "r_user_main.user_id = r_user_group.user_id");
  sFklink("r_user_group", "r_group_main", "r_user_group.group_user_id = r_group_main.user_id");
  sFklink("r_user_main", "r_objt_audit", "r_user_main.user_id = r_objt_audit.user_id");
}
